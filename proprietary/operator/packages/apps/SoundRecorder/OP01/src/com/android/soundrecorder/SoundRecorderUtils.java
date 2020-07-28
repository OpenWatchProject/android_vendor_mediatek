/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.android.soundrecorder;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.sqlite.SQLiteFullException;
import android.net.Uri;
import android.provider.MediaStore;
import android.widget.Toast;

/**
 * Utils class for sound recorder.
 */
public class SoundRecorderUtils {
    private static final String TAG = "SR/SoundRecorderUtils";
    private static Toast sToast;

    /**
     * send Broadcast to stop music.
     *
     * @param context
     *            the context that call this function
     */
    public static void sendBroadcastToStopMusic(Context context) {
        if (null == context) {
            LogUtils.e(TAG, "<sendBroadcastToStopMusic> context is null");
            return;
        }
        final String commandString = "command";
        Intent i = new Intent("com.android.music.musicservicecommand");
        i.putExtra(commandString, "pause");
        context.sendBroadcast(i);
    }

    /**
     * delete file from Media database.
     *
     * @param context
     *            the context that call this function
     * @param filePath
     *            the file path to to deleted
     * @return delete result
     */
    public static boolean deleteFileFromMediaDB(Context context, String filePath) {
        LogUtils.i(TAG, "<deleteFileFromMediaDB> begin");
        if (null == context) {
            LogUtils.e(TAG, "<deleteFileFromMediaDB> context is null");
            return false;
        }
        if (null == filePath) {
            LogUtils.i(TAG, "<deleteFileFromMediaDB> filePath is null");
            return false;
        }
        ContentResolver resolver = context.getContentResolver();
        Uri base = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
        final String[] ids = new String[] { MediaStore.Audio.Media._ID };
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append(MediaStore.Audio.Media.DATA);
        stringBuilder.append(" LIKE '%");
        stringBuilder.append(filePath.replaceFirst("file:///", ""));
        stringBuilder.append("'");
        final String where = stringBuilder.toString();
        Cursor cursor = query(context, base, ids, where, null, null);
        boolean res = false;
        try {
            if ((null != cursor) && (cursor.getCount() > 0)) {
                int deleteNum = resolver.delete(base, where, null);
                LogUtils.i(TAG, "<deleteFileFromMediaDB> delete " + deleteNum + " items in db");
                res = (deleteNum != 0);
            } else {
                if (cursor == null) {
                    LogUtils.e(TAG, "<deleteFileFromMediaDB>, cursor is null");
                } else {
                    LogUtils.e(TAG, "<deleteFileFromMediaDB>, cursor is:" + cursor
                            + "; cursor.getCount() is:" + cursor.getCount());
                }
            }
        } catch (IllegalStateException e) {
            LogUtils.e(TAG, "<deleteFileFromMediaDB> " + e.getMessage());
            res = false;
        } catch (SQLiteFullException e) {
            LogUtils.e(TAG, "<deleteFileFromMediaDB> " + e.getMessage());
            res = false;
        } finally {
            if (null != cursor) {
                cursor.close();
            }
        }
        LogUtils.i(TAG, "<deleteFileFromMediaDB> end");
        return res;
    }

    /**
     * A simple utility to do a query into the databases.
     *
     * @param context
     *            the context that call this function
     * @param uri
     *            data URI
     * @param projection
     *            column collection
     * @param selection
     *            the rule of select
     * @param selectionArgs
     *            the args of select
     * @param sortOrder
     *            sort order
     * @return the cursor returned by resolver.query
     */
    public static Cursor query(Context context, Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        if (null == context) {
            LogUtils.e(TAG, "<query> context is null");
            return null;
        }
        try {
            ContentResolver resolver = context.getContentResolver();
            if (null == resolver) {
                LogUtils.e(TAG, "<query> resolver is null");
                return null;
            }
            return resolver.query(uri, projection, selection, selectionArgs, sortOrder);
        } catch (UnsupportedOperationException ex) {
            LogUtils.e(TAG, ex.getMessage());
            return null;
        }
    }

    /**
     * show a toast.
     * @param context the context
     * @param resId the content that to be display
     */
    public static void getToast(Context context, int resId) {
        if (null == sToast) {
            sToast = Toast.makeText(context, resId, Toast.LENGTH_SHORT);
        }
        sToast.setText(resId);
        sToast.show();
    }
}