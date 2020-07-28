/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera;

import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.provider.MediaStore.Images.Media;
import android.view.Window;
import android.view.WindowManager;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.util.ArrayList;
import java.util.regex.PatternSyntaxException;

/**
 * Use a different activity for secure camera only. So it can have a different
 * task affinity from others. This makes sure non-secure camera activity is not
 * started in secure lock screen.
 */
public class SecureCameraActivity extends CameraActivity {
    private static final Tag TAG = new Tag(SecureCameraActivity.class.getSimpleName());
    private static final String INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE =
            "android.media.action.STILL_IMAGE_CAMERA_SECURE";
    private static final String ACTION_IMAGE_CAPTURE_SECURE =
            "android.media.action.IMAGE_CAPTURE_SECURE";
    private static final String REVIEW_ACTION = "com.android.camera.action.REVIEW";
    private static final String IS_SECURE_CAMERA = "isSecureCamera";
    private static final String SECURE_ALBUM = "secureAlbum";
    private static final String SECURE_PATH = "securePath";
    // The intent extra for camera from secure lock screen. True if the gallery
    // should only show newly captured pictures. sSecureAlbumId does not
    // increment. This is used when switching between camera, camcorder, and
    // panorama. If the extra is not set, it is in the normal camera mode.
    private static final String SECURE_CAMERA_EXTRA = "secure_camera";

    // Secure album id. This should be incremented every time the camera is
    // launched from the secure lock screen. The id should be the same when
    // switching between camera mode.
    private static int sSecureAlbumId;
    // True if the camera is started from secure lock screen.
    private boolean mSecureCamera;

    private ArrayList<String> mSecureArray = new ArrayList<String>();
    private String mPath = null;

    @Override
    protected void onCreateTasks(Bundle icicle) {
        // Check if this is in the secure camera mode.
        Intent intent = getIntent();
        String action = intent.getAction();
        if (INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE.equals(action)) {
            mSecureCamera = true;
            // Use a new album when this is started from the lock screen.
            sSecureAlbumId++;
        } else if (ACTION_IMAGE_CAPTURE_SECURE.equals(action)) {
            mSecureCamera = true;
        } else {
            mSecureCamera = intent.getBooleanExtra(SECURE_CAMERA_EXTRA, false);
        }
        if (mSecureCamera) {
            setScreenFlags();
            mPath = "/secure/all/" + sSecureAlbumId;
            IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
            registerReceiver(mScreenOffReceiver, filter);
        }
        super.onCreateTasks(icicle);
        if (mSecureCamera) {
            getAppUi().updateThumbnail(null);
        }
    }

    @Override
    protected void onResumeTasks() {
        if (mSecureCamera) {
            if (mSecureArray.isEmpty()) {
                getAppUi().updateThumbnail(null);
            } else {
                if (!checkSecureAlbumLive()) {
                    getAppUi().updateThumbnail(null);
                    mSecureArray.clear();
                }
            }
        }
        super.onResumeTasks();
    }

    @Override
    protected void onPauseTasks() {
        super.onPauseTasks();
    }

    @Override
    protected void onDestroyTasks() {
        if (mSecureCamera) {
            unregisterReceiver(mScreenOffReceiver);
        }
        mSecureArray.clear();
        super.onDestroyTasks();
    }

    @Override
    public void notifyNewMedia(Uri uri, boolean needNotify) {
        super.notifyNewMedia(uri, needNotify);
        if (needNotify) {
            addSecureAlbumItem(uri);
        }
    }

    @Override
    protected void goToGallery(Uri uri) {
        if (uri == null) {
            LogHelper.d(TAG, "uri is null, can not go to gallery");
            return;
        }
        String mimeType = getContentResolver().getType(uri);
        LogHelper.d(TAG, "[goToGallery] uri: " + uri + ", mimeType = " + mimeType);
        Intent intent = new Intent(REVIEW_ACTION);
        intent.setDataAndType(uri, mimeType);
        intent.putExtra(IS_SECURE_CAMERA, true);
        intent.putExtra(SECURE_ALBUM, mSecureArray);
        intent.putExtra(SECURE_PATH, mPath);
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException ex) {
            LogHelper.e(TAG, "[startGalleryActivity] Couldn't view ", ex);
        }
    }

    // close activity when screen turns off
    private BroadcastReceiver mScreenOffReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            finish();
            LogHelper.d(TAG, "mScreenOffReceiver receive");
        }
    };

    private void setScreenFlags() {
        final Window win = getWindow();
        final WindowManager.LayoutParams params = win.getAttributes();
        params.flags |= WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
        win.setAttributes(params);
    }

    private void addSecureAlbumItem(Uri uri) {
        LogHelper.d(TAG, "addSecureAlbumItem uri = " + uri + ", mSecureCamera = " + mSecureCamera);
        if (uri != null && mSecureCamera) {
            int id = Integer.parseInt(uri.getLastPathSegment());
            String mimeType = getContentResolver().getType(uri);
            if (mimeType == null) {
                LogHelper.e(TAG, "addSecureAlbumItem uri = " + uri +
                        ", mSecureCamera = " + mSecureCamera);
                return;
            }
            boolean isVideo = mimeType.startsWith("video/") ? true : false;
            // Notify Gallery the secure albums through String format
            // such as "4321+true", means file's id = 4321 and is video
            String videoIndex = isVideo ? "+true" : "+false";
            String secureAlbum = String.valueOf(id) + videoIndex;
            mSecureArray.add(secureAlbum);
        }
    }

    // Check file whether exist in provider by id.
    private boolean isSecureUriLive(int id) {
        Cursor cursor = null;
        try {
            // for file kinds of uri, query media database
            cursor = Media.query(getContentResolver(), MediaStore.Files
                    .getContentUri("external"),
                     null, "_id=(" + id + ")", null, null);
            if (null != cursor) {
                LogHelper.w(TAG, "<isSecureUriLive> cursor " + cursor.getCount());
                return cursor.getCount() > 0;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }
        return true;
    }
    //check all files in Secure array whether exit in provider
    private boolean checkSecureAlbumLive() {
        int albumCount = mSecureArray.size();
        LogHelper.d(TAG, "<checkSecureAlbumLive> albumCount " + albumCount);
        for (int i = 0; i < albumCount; i++) {
            try {
                String[] albumItem = mSecureArray.get(i).split("\\+");
                int albumItemSize = albumItem.length;
                LogHelper.d(TAG, "<checkSecureAlbumLive> albumItemSize "
                        + albumItemSize);
                if (albumItemSize == 2) {
                    int id = Integer.parseInt(albumItem[0].trim());
                    boolean isVideo = Boolean.parseBoolean(albumItem[1]
                            .trim());
                    LogHelper.d(TAG, "<checkSecureAlbumLive> secure item : id " + id
                            + ", isVideo " + isVideo);
                    if (isSecureUriLive(id)) {
                        return true;
                    }
                }
            } catch (NullPointerException ex) {
                LogHelper.e(TAG, "<checkSecureAlbumLive> NullPointerException " + ex);
            } catch (PatternSyntaxException ex) {
                LogHelper.e(TAG, "<checkSecureAlbumLive> PatternSyntaxException " + ex);
            } catch (NumberFormatException ex) {
                LogHelper.e(TAG, "<checkSecureAlbumLive> NumberFormatException " + ex);
            }
        }
        return false;
    }
}
