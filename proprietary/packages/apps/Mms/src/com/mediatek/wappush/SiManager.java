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

package com.mediatek.wappush;

import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;

import com.android.mms.util.MmsLog;

import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.pushparser.ParsedMessage;
import com.mediatek.pushparser.si.SiMessage;

import mediatek.telephony.MtkTelephony.WapPush;


public class SiManager extends WapPushManager {

    public final static String TAG = "Mms/WapPush";

    public SiManager(Context context) {
        super(context);
    }

    @Override
    public void handleIncoming(ParsedMessage message) {

        if (message == null) {
            MmsLog.e(TAG, "SiManager handleIncoming: null message");
            return;
        }


        SiMessage siMsg = null;
        try {
             siMsg = (SiMessage) message;
        } catch (Exception e) {
            MmsLog.e(TAG, "SiManager SiMessage error");
        }

        //Prepare for query
        ContentResolver resolver = mContext.getContentResolver();
        Cursor cursor = null;


        int currentTime = (int) (System.currentTimeMillis() / 1000);

        //If there is no Created time, use the local device time instead (convert to UTC).
        if (siMsg.create == 0) {
            siMsg.create = (int) (System.currentTimeMillis() / 1000);
        }
        /*
         * Handle SI Message
         * Reference:WAP-167-ServiceInd-20010731-a
         */

        //if si-id is null , si-id will be set the url
        if (siMsg.siid == null) {
            siMsg.siid = siMsg.url;
        }

        //1,Dicard Expired Message

        if (siMsg.expiration > 0 && siMsg.expiration < currentTime) {
            MmsLog.wpi(TAG, "SiManager:Expired Message! " + siMsg.url);
            return;
        }

        //5,Discard SI has action = none
        if (siMsg.action == SiMessage.ACTION_NONE) {
            MmsLog.wpi(TAG, "SiManager:Discard None Message! " + siMsg.url);
            return;
        }

        // M: fix bug ALPS00354663
        /*
         * Query to find Message with the same si-id and received by the same card.
         * Single card id is -1, and Gemini Cards number > 0
         *
         */
         String []projection = {WapPush._ID, WapPush.SIID, WapPush.URL, WapPush.CREATE,
                         WapPush.ADDR, WapPush.TEXT, WapPush.SUBSCRIPTION_ID};
         String selection = WapPush.SIID + "=?" + " AND " + WapPush.SUBSCRIPTION_ID + "=?";
         String []selectionArgs = {siMsg.siid, Long.toString(siMsg.getSubId())};

        //url && siid will be the same; if siid is null, no need to query database
        if (siMsg.siid != null) {
            cursor = resolver.query(WapPush.CONTENT_URI_SI,
                    projection, selection, selectionArgs, null);
        }

        boolean onlyUpdate = false;
        long rowId = 0;
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                    do {
                        long messageId = cursor.getLong(0);
                        String siid = cursor.getString(1);
                        int createdTime = cursor.getInt(3);
                        String address = cursor.getString(4);
                        String text = cursor.getString(5);

                        if (siid.equals(siMsg.siid)) {
                            // 2,Received SI older than other SI with
                            // identical si-id, discard the older one
                            if (siMsg.create > 0 && siMsg.create < createdTime) {
                                MmsLog.wpi(TAG, "SiManager:Out of order Message! " + siMsg.url);
                                return;
                            } else if (siMsg.create >= createdTime) {
                                // update to the new message, the old one is replaced.
                                onlyUpdate = true;
                                rowId = messageId;
                            }
                        }
                    } while(cursor.moveToNext());

                }
            } finally {
                cursor.close();
            }
        }

        //4,SI has action=delete, delete
        if (siMsg.action == SiMessage.ACTION_DELETE) {
            mContext.getContentResolver()
                .delete(ContentUris.withAppendedId(WapPush.CONTENT_URI, rowId), null, null);
            WapPushMessagingNotification.blockingUpdateNewMessageIndicator(mContext,
                    WapPushMessagingNotification.THREAD_NONE);
            MmsLog.ipi(TAG, "SiManager:Discard delete Message! " + siMsg.url);
            return;
        }

        //store in db
        ContentValues values = new ContentValues();
        values.put(WapPush.ADDR, siMsg.getSenderAddr());
        values.put(WapPush.SERVICE_ADDR, siMsg.getServiceCenterAddr());
        values.put(WapPush.SUBSCRIPTION_ID, siMsg.getSubId());
        values.put(WapPush.URL, siMsg.url);
        values.put(WapPush.SIID, siMsg.siid);
        values.put(WapPush.ACTION, siMsg.action);
        values.put(WapPush.CREATE, siMsg.create);
        values.put(WapPush.EXPIRATION, siMsg.expiration);
        values.put(WapPush.TEXT, siMsg.text);
        values.put(WapPush.DATE, System.currentTimeMillis());

        Uri uri = null;
        if (!onlyUpdate) {
            uri = mContext.getContentResolver().insert(WapPush.CONTENT_URI_SI, values);
        } else {
            // it is update to a new mesage, make it as a new one.
            values.put(WapPush.READ, 0);
            values.put(WapPush.SEEN, 0);
            int count = mContext.getContentResolver().update(WapPush.CONTENT_URI_SI,
                    values, selection, selectionArgs);
            uri = ContentUris.withAppendedId(WapPush.CONTENT_URI, rowId);
            MmsLog.d(TAG, "update a si message.count:" + count);
        }

        //notification
        if (uri != null) {
            //called off of the UI thread so ok to block
            long threadId = WapPushMessagingNotification.getWapPushThreadId(mContext, uri);
            MmsLog.ipi(TAG, "SiManager:Store msg! " + siMsg.url + " uri:" + uri.toString()
                    + " threaId:" + threadId);
            WapPushMessagingNotification.blockingUpdateNewMessageIndicator(mContext, threadId);
        }
    }

}
