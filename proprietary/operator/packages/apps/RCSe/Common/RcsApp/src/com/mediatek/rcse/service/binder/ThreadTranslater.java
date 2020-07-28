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
package com.mediatek.rcse.service.binder;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.SQLException;
import android.net.Uri;
import android.text.TextUtils;
import android.util.LruCache;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IntegratedMessagingData;
import com.mediatek.rcse.service.MediatekFactory;

import java.util.Map;

/**
 * This class will cache a tag and translate a remote string to be a cached tag.
 */
public final class ThreadTranslater {
    /**
     * The Constant TAG.
     */
    private static final String TAG = ThreadTranslater.class
            .getSimpleName();
    /**
     * The Constant MAX_CACHE_SIZE.
     */
    private static final int MAX_CACHE_SIZE = 200;
    /**
     * The Constant THREAD_CACHE.
     */
    private static final LruCache<Long, String> THREAD_CACHE =
            new LruCache<Long, String>(
            MAX_CACHE_SIZE);

    /**
     * Save a threadId and relevant tag into cache.
     *
     * @param threadId            The threadId to be cached
     * @param tag            The tag to be cached
     */
    public static void saveThreadandTag(Long threadId, String tag) {
        Logger.d(TAG, "saveThreadandTag() entry threadId is "
                + threadId + " tag is " + tag);
        THREAD_CACHE.put(threadId, tag);
        putTagIntoDB(threadId, tag);
    }
    /**
     * Put tag into db.
     *
     * @param threadId the thread id
     * @param tag the tag
     */
    public static void putTagIntoDB(Long threadId, String tag) {
        ContentValues values = new ContentValues();
        values.put(
                IntegratedMessagingData.KEY_INTEGRATED_MODE_THREAD_ID,
                threadId);
        values.put(IntegratedMessagingData.KEY_INTEGRATED_MODE_TAG,
                tag);
        try {
            Uri uri = MediatekFactory
                    .getApplicationContext()
                    .getContentResolver()
                    .insert(IntegratedMessagingData.CONTENT_URI_INTEGRATED_TAG,
                            values);
        } catch (SQLException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /**
     * Gets the tag by id.
     *
     * @param threadID the thread id
     * @return the tag by id
     */
    private static String getTagByID(long threadID) {
        Cursor cursor = null;
        String tag = "";
        try {
            cursor = MediatekFactory
                    .getApplicationContext()
                    .getContentResolver()
                    .query(IntegratedMessagingData.CONTENT_URI_INTEGRATED_TAG,
                            null,
                            ""
                                    + IntegratedMessagingData.KEY_INTEGRATED_MODE_THREAD_ID
                                    + " = " + threadID + " ", null,
                            null);
            if (cursor != null && cursor.moveToFirst()) {
                tag = cursor
                        .getString(cursor
                                .getColumnIndex(IntegratedMessagingData.KEY_INTEGRATED_MODE_TAG));
            }
        } catch (SQLException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
                cursor = null;
            }
        }
        return tag;
    }
    /**
     * Translate a threadId into a real tag.
     *
     * @param threadId            The threadId to be translated
     * @return The real tag
     */
    public static String translateThreadId(Long threadId) {
        Logger.d(TAG, "translateThreadId() entry threadId is "
                + threadId);
        String result = THREAD_CACHE.get(threadId);
        Logger.d(TAG, "translateThreadId() result is " + result);
        if (result == null || result.equals("")) {
            result = getTagByID(threadId);
            Logger.d(TAG, "translateThreadId() resultDB is " + result);
            THREAD_CACHE.put(threadId, result);
        }
        return result;
    }
    /**
     * Translate tag.
     *
     * @param tag the tag
     * @return the long
     */
    public static Long translateTag(String tag) {
        Logger.d(TAG, "translateThreadId() entry threadId is " + tag);
        Long threadID = 0L;
        Map<Long, String> map = THREAD_CACHE.snapshot();
        for (Map.Entry<Long, String> e : map.entrySet()) {
            Long key = e.getKey();
            String value = e.getValue();
            if (value.equals(tag)) {
                threadID = key;
            }
        }
        return threadID;
    }
    /**
     * check if the tag is in the cache.
     *
     * @param tag            The tag relevant to group
     * @return boolean if exit
     */
    public static boolean tagExistInCache(String tag) {
        Logger.d(TAG, "tagExistInCache() entry tag is " + tag);
        if (TextUtils.isEmpty(tag)) {
            Logger.d(TAG, "tagExistInCache() tag is null ");
            return false;
        }
        Map<Long, String> map = THREAD_CACHE.snapshot();
        boolean exist = map.containsValue(tag);
        Logger.d(TAG, "tagExistInCache() exist is " + exist);
        return exist;
    }
}
