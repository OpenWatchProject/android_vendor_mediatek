/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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

package com.mediatek.wappush.ui;

import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;

import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.ui.MessageUtils;

import com.mediatek.wappush.WapPushMessagingNotification;
import com.mediatek.wappush.ui.WPMessageListAdapter.WPColumnsMap;

import java.util.regex.Pattern;

import mediatek.telephony.MtkTelephony.WapPush;

public class WPMessageItem {

    private static final String WP_TAG = "Mms/WapPush";

    final Context mContext;
    final int mType;
    final long mMsgId;
    boolean mLocked;            // locked to prevent auto-deletion
    long mDate;
    String mTimestamp;
    String mAddress;
    String mContact;
    String mText;
    String mURL;
    long mCreate;
    long mExpirationLong;
    String mExpiration;
    Pattern mHighlight; // portion of message to highlight (from search)
    int mIsExpired;   //if already be expired, the value is based on ERROR column
    int mAction;    //Priority
    int mSubId;
    String mBody;

    WPMessageItem(Context context, int type, Cursor cursor,
            WPColumnsMap wpColumnsMap, Pattern highlight) {
        mContext = context;
        mMsgId = cursor.getLong(wpColumnsMap.mColumnMsgId);
        mHighlight = highlight;
        mType = type;

        mDate = cursor.getLong(wpColumnsMap.mColumnWpmsDate);
        mTimestamp = MessageUtils.formatTimeStampString(context, mDate);
        mAddress = cursor.getString(wpColumnsMap.mColumnWpmsAddr);
        if (!TextUtils.isEmpty(mAddress)) {
            mContact = Contact.get(mAddress, false).getName();
        } else {
            mContact = "";
        }

        StringBuilder sBuilder = new StringBuilder();
        sBuilder.toString();

        mText = cursor.getString(wpColumnsMap.mColumnWpmsText);
        if (null != mText && !"".equals(mText)) {
            //MmsLog.i(WP_TAG, "WPMessageItem: " + "mText is : " + mText + "test");
            sBuilder.append(mText);
        }

        mURL = cursor.getString(wpColumnsMap.mColumnWpmsURL);
        if (null != mURL && !"".equals(mURL)) {
//            sBuilder.append("<br>");
            sBuilder.append("\n");
            sBuilder.append(mURL);
        }
        mBody = sBuilder.toString();

        mCreate = cursor.getLong(wpColumnsMap.mColumnWpmsCreate) * 1000;

        mExpirationLong = cursor.getLong(wpColumnsMap.mColumnWpmsExpiration) * 1000;
        if (0 != mExpirationLong) {
            mExpiration = String.format(context.getString(R.string.wp_msg_expiration_label),
                    MessageUtils.formatTimeStampString(context, mExpirationLong));
        }

        mIsExpired = cursor.getInt(wpColumnsMap.mColumnWpmsError);
        mAction = cursor.getInt(wpColumnsMap.mColumnWpmsAction);
        mSubId = cursor.getInt(wpColumnsMap.mColumnWpmsSubId);
        mLocked = cursor.getInt(wpColumnsMap.mColumnWpmsLocked) != 0;
        // mBody = mText + mURL + mCreate + mExpiration;
        //MmsLog.i(WP_TAG, toString());
        setIsUnread(cursor.getInt(wpColumnsMap.mColumnWpmsRead) == 0);
    }

    public int getSubId() {
        return mSubId;
    }

    @Override
    public String toString() {
        return "type: " + mType +
            " sim: " + mSubId +
            " text: " + mText +
            " url: " + mURL +
            " time: " + mTimestamp +
            " address: " + mAddress +
            " contact: " + mContact +
            " create: " + mCreate +
            " expiration: " + mExpiration +
            " action: " + mAction;
    }
    private boolean mIsChecked;
    private boolean mIsUnread;
    public boolean isChecked() {
        return mIsChecked;
    }

    public void setIsChecked(boolean isChecked) {
        mIsChecked = isChecked;
    }

    public boolean isUnread() {
        return mIsUnread;
    }

    public void setIsUnread(boolean isUnread) {
        mIsUnread = isUnread;
    }

    public void markAsRead() {
        final Uri wpMsgUri = ContentUris.withAppendedId(WapPush.CONTENT_URI,
                mMsgId);
        new Thread(new Runnable() {
            public void run() {
                if (wpMsgUri != null) {
                    buildReadContentValues();

                    /**
                     * M: Check the read flag first. It's much faster to do a
                     * query than to do an update. Timing this function show
                     * it's about 10x faster to do the query compared to the
                     * update, even when there's nothing to update.
                     */
                    boolean needUpdate = true;

                    Cursor c = mContext.getContentResolver().query(wpMsgUri,
                            UNREAD_PROJECTION, UNREAD_SELECTION, null, null);
                    if (c != null) {
                        try {
                            needUpdate = c.getCount() > 0;
                        } finally {
                            c.close();
                        }
                    }

                    if (needUpdate) {
                        mContext.getContentResolver().update(wpMsgUri,
                                sReadContentValues, UNREAD_SELECTION, null);
                        mIsUnread = false;
                    }
                }
                WapPushMessagingNotification
                        .nonBlockingUpdateNewMessageIndicator(mContext,
                                WapPushMessagingNotification.THREAD_NONE);
            }
        }).start();
    }

    private static ContentValues sReadContentValues;
    private static final String UNREAD_SELECTION = "(read=0 OR seen=0)";
    public static final String[] UNREAD_PROJECTION = {
        WapPush._ID,
        WapPush.READ
    };
    private void buildReadContentValues() {
        if (sReadContentValues == null) {
            sReadContentValues = new ContentValues(2);
            sReadContentValues.put("read", 1);
            sReadContentValues.put("seen", 1);
        }
    }
}
