/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2009 The Android Open Source Project
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

package com.android.gallery3d.gadget;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.MediaStore.Images;
import android.provider.MediaStore.Images.ImageColumns;
import android.widget.Toast;

import com.android.gallery3d.R;
import com.android.gallery3d.app.GalleryActivity;
import com.mediatek.gallery3d.util.Log;

/// M: [BUG.MARK] @{
/*// M: comment out for MTK UX issue
 //import com.android.gallery3d.app.PhotoPage;*/
/// @}
import com.android.gallery3d.common.ApiHelper;
import com.mediatek.gallery3d.util.PermissionHelper;

public class WidgetClickHandler extends Activity {
    private static final String TAG = "Gallery2/WidgetClickHandler";

    private boolean isValidDataUri(Uri dataUri) {
        if (dataUri == null) return false;
        /// M: [BUG.ADD] scheme is content @{
        String scheme = dataUri.getScheme();
        if (scheme == null || !ContentResolver.SCHEME_CONTENT.equals(scheme)) {
            return false;
        }
        /// @}
        try {
            AssetFileDescriptor f = getContentResolver()
                    .openAssetFileDescriptor(dataUri, "r");
            f.close();
            return true;
        } catch (Throwable e) {
            Log.w(TAG, "cannot open uri: " + dataUri, e);
            return false;
        }
    }

    @Override
    @TargetApi(ApiHelper.VERSION_CODES.HONEYCOMB)
    protected void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        /// M: [FEATURE.MODIFY] [Runtime permission] @{
        /// Behavior change for empty widget.
        /// Still can launch gallery when click empty widget.
        /*
         // The behavior is changed in JB, refer to b/6384492 for more details
         boolean tediousBack = Build.VERSION.SDK_INT >= ApiHelper.VERSION_CODES.JELLY_BEAN;
         Uri uri = getIntent().getData();
         Intent intent;
         if (isValidDataUri(uri)) {
         intent = new Intent(Intent.ACTION_VIEW, uri);
         if (tediousBack) {
         intent.putExtra(PhotoPage.KEY_TREAT_BACK_AS_UP, true);
         }
         } else {
         Toast.makeText(this,
         R.string.no_such_item, Toast.LENGTH_LONG).show();
         intent = new Intent(this, GalleryActivity.class);
         }
         if (tediousBack) {
         intent.setFlags(
         Intent.FLAG_ACTIVITY_NEW_TASK |
         Intent.FLAG_ACTIVITY_CLEAR_TASK |
         Intent.FLAG_ACTIVITY_TASK_ON_HOME);
         }
         startActivity(intent);
         finish();
        */
        mLaunchFromEmptyView = getIntent().getBooleanExtra(FLAG_FROM_EMPTY_VIEW, false)
                && (getIntent().getData() == null);
        Log.d(TAG, "<onCreate> FLAG_FROM_EMPTY_VIEW is "
                        + getIntent().getBooleanExtra(FLAG_FROM_EMPTY_VIEW, false)
                        + ", data = "
                        + getIntent().getData());
        if (PermissionHelper.checkAndRequestForWidget(this)) {
            if (mLaunchFromEmptyView) {
                permissionGrantedWhenLaunchFromEmpty();
            }
            startToViewImage();
            return;
        }
        /// @}
    }

    /// M: [BUG.ADD] @{
    // The URI(/storage/sdcard1/) is absolute path, so transform the path to
    // URI(content://media/external/images/media/id)
    public static Uri getContentUri(Uri uri,
            Context context) {
        if (uri == null) {
            return null;
        }
        String absolutePath = uri.toString();
        Log.d(TAG, "<getContentUri> Single Photo mode absolutePath = "
                + absolutePath);
        Cursor cursor = null;
        int id;
        try {
            cursor = context.getContentResolver().query(
                    Images.Media.EXTERNAL_CONTENT_URI,
                    new String[] { ImageColumns._ID },
                    ImageColumns.DATA + " = ?", new String[] { absolutePath },
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                id = cursor.getInt(0);
                Log.d(TAG, " <getContentUri> " + Images.Media.EXTERNAL_CONTENT_URI);
                uri = ContentUris.withAppendedId(
                        Images.Media.EXTERNAL_CONTENT_URI, id);
                Log.d(TAG, "<getContentUri> Single Photo mode : The URI base of absolte path = "
                                        + uri);
            } else {
                uri = null;
            }
        } catch (final SQLiteException e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return uri;
    }
    /// @}

    /// M: [FEATURE.ADD] [Runtime permission] @{
    public static final String FLAG_FROM_EMPTY_VIEW = "on_click_from_empty_view";
    public static final String FLAG_WIDGET_ID = "widget_id";

    private boolean mLaunchFromEmptyView = false;

    // Remove the initialize operation in onCreate to startToViewImage.
    // When not launch from empty view and permission is granted,
    // do initialize, and start activity to view image.
    private void startToViewImage() {
        // The behavior is changed in JB, refer to b/6384492 for more details
        boolean tediousBack = Build.VERSION.SDK_INT >= ApiHelper.VERSION_CODES.JELLY_BEAN;
        Uri uri = getIntent().getData();

        /// M: [FEATURE.MODIFY] change BuckID to Uri@{
        //Intent intent;
        Intent intent = null;
        /// @}
        /// M: [BEHAVIOR.ADD] Transform absolute path to URI
        //(//content://media/external/images/media/id)@{
        //if (isValidDataUri(uri)) {
        if ((isValidDataUri(uri)
                 || (uri = getContentUri(uri, this.getBaseContext())) != null)) {
        /// @}
            intent = new Intent(Intent.ACTION_VIEW, uri);
            /// M: [BUG.MARK] commented out  for MTK UX issues.@{
            /*if (tediousBack) {
                intent.putExtra(PhotoPage.KEY_TREAT_BACK_AS_UP, true);
            }*/
            /// @}
        } else {
            Toast.makeText(this,
                    R.string.no_such_item, Toast.LENGTH_LONG).show();
            intent = new Intent(this, GalleryActivity.class);
        }
        if (tediousBack) {
            intent.setFlags(
                    Intent.FLAG_ACTIVITY_NEW_TASK |
                    Intent.FLAG_ACTIVITY_CLEAR_TASK |
                    Intent.FLAG_ACTIVITY_TASK_ON_HOME);
        }
        /// M: [BUG.ADD] for widget flash issue.@{
        intent.putExtra(GalleryActivity.EXTRA_FROM_WIDGET, true);
        /// @}
        startActivity(intent);
        finish();
    }

    // An activity without a UI must call finish() before onResume() completes.
    // When permission is not granted, current activity is not finished when onResume.
    // In order to avoid IllegalStateException when performResume, not use GalleryNoDisplay any more
    @Override
    protected void onStart() {
        super.onStart();
        if (!isFinishing()) {
            setVisible(true);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
            String[] permissions, int[] grantResults) {
        if (PermissionHelper.isAllPermissionsGranted(permissions, grantResults)) {
            Log.d(TAG, "<onRequestPermissionsResult> all permission granted");
            if (mLaunchFromEmptyView) {
                permissionGrantedWhenLaunchFromEmpty();
            } else {
                startToViewImage();
            }
            return;
        } else {
            Log.d(TAG, "<onRequestPermissionsResult> permission denied, finish");
            PermissionHelper.showDeniedPrompt(this);
            finish();
        }
    }

    private void permissionGrantedWhenLaunchFromEmpty() {
        Log.d(TAG, "<permissionGrantedWhenLaunchFromEmpty>");
        // when gallery permssion changed, notify all widgets to update
        WidgetUtils.notifyAllWidgetViewChanged();
        finish();
    }
    /// @}
}
