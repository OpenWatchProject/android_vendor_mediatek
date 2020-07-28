/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
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

import android.app.Activity;
import android.appwidget.AppWidgetManager;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore.Images.ImageColumns;
import android.widget.RemoteViews;
import android.widget.Toast;

import com.android.gallery3d.R;
import com.android.gallery3d.app.AlbumPicker;
import com.android.gallery3d.app.DialogPicker;
import com.android.gallery3d.app.GalleryActivity;
import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.data.DataManager;
import com.android.gallery3d.data.LocalAlbum;
import com.android.gallery3d.data.MediaSet;
import com.android.gallery3d.data.Path;
import com.android.gallery3d.filtershow.crop.CropActivity;
import com.android.gallery3d.filtershow.crop.CropExtras;
import com.mediatek.gallery3d.adapter.PhotoPlayFacade;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallery3d.util.PermissionHelper;
import com.mediatek.omadrm.OmaDrmStore;

public class WidgetConfigure extends Activity {
    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/WidgetConfigure";

    public static final String KEY_WIDGET_TYPE = "widget-type";
    private static final String KEY_PICKED_ITEM = "picked-item";

    private static final int REQUEST_WIDGET_TYPE = 1;
    private static final int REQUEST_CHOOSE_ALBUM = 2;
    private static final int REQUEST_CROP_IMAGE = 3;
    private static final int REQUEST_GET_PHOTO = 4;

    public static final int RESULT_ERROR = RESULT_FIRST_USER;

    // Scale up the widget size since we only specified the minimized
    // size of the gadget. The real size could be larger.
    // Note: There is also a limit on the size of data that can be
    // passed in Binder's transaction.
    private static float WIDGET_SCALE_FACTOR = 1.5f;
    private static int MAX_WIDGET_SIDE = 360;

    private int mAppWidgetId = -1;
    private Uri mPickedItem;

    @Override
    protected void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        mAppWidgetId = getIntent().getIntExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, -1);

        if (mAppWidgetId == -1) {
            setResult(Activity.RESULT_CANCELED);
            finish();
            return;
        }
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<onCreate> widget id=" + mAppWidgetId);
        /// @}

        if (savedState == null) {
            /// M: [FEATURE.ADD] [Runtime permission] @{
            if (PermissionHelper.checkAndRequestForWidget(this)) {
            /// @}
                if (ApiHelper.HAS_REMOTE_VIEWS_SERVICE) {
                    Intent intent = new Intent(this, WidgetTypeChooser.class);
                    startActivityForResult(intent, REQUEST_WIDGET_TYPE);
                } else { // Choose the photo type widget
                    setWidgetType(new Intent()
                            .putExtra(KEY_WIDGET_TYPE, R.id.widget_type_photo));
                }
            /// M: [FEATURE.ADD] [Runtime permission] @{
            }
            /// @}
        } else {
            mPickedItem = savedState.getParcelable(KEY_PICKED_ITEM);
        }
    }

    protected void onSaveInstanceStates(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putParcelable(KEY_PICKED_ITEM, mPickedItem);
    }

    private void updateWidgetAndFinish(WidgetDatabaseHelper.Entry entry) {
        AppWidgetManager manager = AppWidgetManager.getInstance(this);
        RemoteViews views = PhotoAppWidgetProvider.buildWidget(this, mAppWidgetId, entry);
        manager.updateAppWidget(mAppWidgetId, views);
        setResult(RESULT_OK, new Intent().putExtra(
                AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetId));
        /// M: [BUG.ADD] @{
        // In order to keep service alive, start service by gallery self,
        // or else widget process may be killed for there is no android
        // component running on it.
        startService(new Intent(getApplicationContext(), WidgetService.class));
        /// @}
        finish();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode != RESULT_OK) {
            setResult(resultCode, new Intent().putExtra(
                    AppWidgetManager.EXTRA_APPWIDGET_ID, mAppWidgetId));
            finish();
            return;
        }

        /// M: [FEATURE.ADD] @{
        PhotoPlayFacade.registerWidgetMedias(this);
        /// @}
        if (requestCode == REQUEST_WIDGET_TYPE) {
            setWidgetType(data);
        } else if (requestCode == REQUEST_CHOOSE_ALBUM) {
            setChoosenAlbum(data);
        } else if (requestCode == REQUEST_GET_PHOTO) {
            setChoosenPhoto(data);
        } else if (requestCode == REQUEST_CROP_IMAGE) {
            setPhotoWidget(data);
        } else {
            throw new AssertionError("unknown request: " + requestCode);
        }
    }

    private void setPhotoWidget(Intent data) {
        // Store the cropped photo in our database
        /// M: [BUG.MODIFY] @{
        // The return data is changed to byte array, we need to decode bitmap from byte array
        /* Bitmap bitmap = (Bitmap) data.getParcelableExtra("data");*/
        Bitmap bitmap = null;
        byte[] byteArray = (byte[]) data.getByteArrayExtra(CropExtras.KEY_DATA_COMPRESS);
        if (byteArray != null) {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;
            options.inDither = true;
            options.inPreferQualityOverSpeed = true;
            bitmap = BitmapFactory.decodeByteArray(byteArray, 0, byteArray.length, options);
        }
        /// @}
        WidgetDatabaseHelper helper = new WidgetDatabaseHelper(this);
        try {
            /// M: [BUG.MODIFY]  @{
            // we need to check each step for possible failure,
            // e.g. low memory caused gallery process being killed during CropImage
            /*
            helper.setPhoto(mAppWidgetId, mPickedItem, bitmap);
            updateWidgetAndFinish(helper.getEntry(mAppWidgetId));
            */
            if (mPickedItem == null) {
                // M: since we've changed default pick->crop flow,
                // we should look into intent data for a possible picked item uri
                /// M: The URI(content://media/external/images/media/id)
                // maybe change, after unmount and mount operation. So Use absolute path replace
                // URI(content://media/external/images/media/id)
                String absolutePath = getAbsloutePath(data.getData());
                if (absolutePath != null) {
                    mPickedItem = Uri.parse(absolutePath);
                    Log.d(TAG, "<setPhotoWidget> use absloute path =" + mPickedItem);
                } else {
                    mPickedItem = data.getData();
                }
            }
            boolean setPhotoSucceeded = helper.setPhoto(mAppWidgetId, mPickedItem, bitmap);
            if (!setPhotoSucceeded) {
                Log.e(TAG, "<setPhotoWidget> setPhoto for widget #" + mAppWidgetId + " uri["
                        + mPickedItem + "] failed!!");
                Toast.makeText(this, R.string.widget_load_failed, Toast.LENGTH_SHORT).show();
                setResult(RESULT_CANCELED);
                finish();
                return;
            }
            WidgetDatabaseHelper.Entry widgetEntry = helper.getEntry(mAppWidgetId);
            if (widgetEntry != null) {
                updateWidgetAndFinish(widgetEntry);
            } else {
                Log.e(TAG, "<setPhotoWidget> getEntry(" + mAppWidgetId + ") failed!!");
                Toast.makeText(this, R.string.widget_load_failed, Toast.LENGTH_SHORT).show();
                setResult(RESULT_CANCELED);
                finish();
            }
            /// @}
        } finally {
            helper.close();
        }
    }

    private void setChoosenPhoto(Intent data) {
        Resources res = getResources();

        float width = res.getDimension(R.dimen.appwidget_width);
        float height = res.getDimension(R.dimen.appwidget_height);

        // We try to crop a larger image (by scale factor), but there is still
        // a bound on the binder limit.
        float scale = Math.min(WIDGET_SCALE_FACTOR,
                MAX_WIDGET_SIDE / Math.max(width, height));

        int widgetWidth = Math.round(width * scale);
        int widgetHeight = Math.round(height * scale);

        mPickedItem = data.getData();
        Intent request = new Intent(CropActivity.CROP_ACTION, mPickedItem)
                .putExtra(CropExtras.KEY_OUTPUT_X, widgetWidth)
                .putExtra(CropExtras.KEY_OUTPUT_Y, widgetHeight)
                .putExtra(CropExtras.KEY_ASPECT_X, widgetWidth)
                .putExtra(CropExtras.KEY_ASPECT_Y, widgetHeight)
                .putExtra(CropExtras.KEY_SCALE_UP_IF_NEEDED, true)
                .putExtra(CropExtras.KEY_SCALE, true)
                .putExtra(CropExtras.KEY_RETURN_DATA, true);
        startActivityForResult(request, REQUEST_CROP_IMAGE);
    }

    private void setChoosenAlbum(Intent data) {
        String albumPath = data.getStringExtra(AlbumPicker.KEY_ALBUM_PATH);
        WidgetDatabaseHelper helper = new WidgetDatabaseHelper(this);
        try {
            String relativePath = null;
            GalleryApp galleryApp = (GalleryApp) getApplicationContext();
            DataManager manager = galleryApp.getDataManager();
            Path path = Path.fromString(albumPath);
            MediaSet mediaSet = (MediaSet) manager.getMediaObject(path);
            if (mediaSet instanceof LocalAlbum) {
                /// M: [BUG.MARK] @{
                /*int bucketId = Integer.parseInt(path.getSuffix());*/
                /// @}
                // If the chosen album is a local album, find relative path
                // Otherwise, leave the relative path field empty
                /// M: [BUG.MODIFY] @{
                /*relativePath = LocalAlbum.getRelativePath(bucketId);*/
                relativePath = ((LocalAlbum) mediaSet).getRelativePath();
                /// @}
                Log.i(TAG, "<setChoosenAlbum> Setting widget, album path: " + albumPath
                        + ", relative path: " + relativePath);
            }
            helper.setWidget(mAppWidgetId,
                    WidgetDatabaseHelper.TYPE_ALBUM, albumPath, relativePath);
            updateWidgetAndFinish(helper.getEntry(mAppWidgetId));
        } finally {
            helper.close();
        }
    }

    private void setWidgetType(Intent data) {
        int widgetType = data.getIntExtra(KEY_WIDGET_TYPE, R.id.widget_type_shuffle);
        if (widgetType == R.id.widget_type_album) {
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<setWidgetType> setWidgetType: type=album");
            /// @}
            Intent intent = new Intent(this, AlbumPicker.class);
            ///M: [FEATURE.ADD] Only show FL drm files @{
            intent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                    OmaDrmStore.DrmIntentExtra.LEVEL_FL);
            /// @}
            startActivityForResult(intent, REQUEST_CHOOSE_ALBUM);
        } else if (widgetType == R.id.widget_type_shuffle) {
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<setWidgetType> setWidgetType: type=shuffle");
            /// @}
            WidgetDatabaseHelper helper = new WidgetDatabaseHelper(this);
            try {
                helper.setWidget(mAppWidgetId, WidgetDatabaseHelper.TYPE_SHUFFLE, null, null);
                updateWidgetAndFinish(helper.getEntry(mAppWidgetId));
            } finally {
                helper.close();
            }
        } else {
            /// M: [BUG.ADD] start Crop Flow if needed.@{
            Log.d(TAG, "<setWidgetType> type=photo");
            if (startMtkCropFlow()) {
                return;
            }
            /// @}
            // Explicitly send the intent to the DialogPhotoPicker
            Intent request = new Intent(this, DialogPicker.class)
                    .setAction(Intent.ACTION_GET_CONTENT)
                    .setType("image/*");
            ///M: [FEATURE.ADD] Only show FL drm files @{
            request.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                    OmaDrmStore.DrmIntentExtra.LEVEL_FL);
            /// @}
            startActivityForResult(request, REQUEST_GET_PHOTO);
        }
    }

    /// M: [BUG.ADD]added for MTK-specific pick-and-crop flow;
    // which allows user to navigate back to AlbumPage
    // and pick another image to crop @{
    private boolean startMtkCropFlow() {
        // Explicitly send the intent to the DialogPhotoPicker
        Intent request = new Intent(this, DialogPicker.class)
                .setAction(Intent.ACTION_GET_CONTENT)
                .setType("image/*");

        // M: calculate and fill in crop related extras
        Resources res = getResources();

        float width = res.getDimension(R.dimen.appwidget_width);
        float height = res.getDimension(R.dimen.appwidget_height);

        // We try to crop a larger image (by scale factor), but there is still
        // a bound on the binder limit.
        float scale = Math.min(WIDGET_SCALE_FACTOR,
                MAX_WIDGET_SIDE / Math.max(width, height));

        int widgetWidth = Math.round(width * scale);
        int widgetHeight = Math.round(height * scale);

        request.putExtra(CropExtras.KEY_OUTPUT_X, widgetWidth)
                .putExtra(CropExtras.KEY_OUTPUT_Y, widgetHeight)
                .putExtra(CropExtras.KEY_ASPECT_X, widgetWidth)
                .putExtra(CropExtras.KEY_ASPECT_Y, widgetHeight)
                .putExtra(CropExtras.KEY_SCALE_UP_IF_NEEDED, true)
                .putExtra(CropExtras.KEY_SCALE, true)
                // request to return compress data
                .putExtra(CropExtras.KEY_RETURN_DATA_COMPRESS, true);
        request.putExtra(GalleryActivity.EXTRA_CROP, "crop");
        ///M: [FEATURE.ADD] Only show FL drm files @{
        request.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                OmaDrmStore.DrmIntentExtra.LEVEL_FL);
        /// @}
        startActivityForResult(request, REQUEST_CROP_IMAGE);
        return true;
    }

    /// @}
    /// M: [BUG.ADD] The URI(//conten://media/external/images/media/id)
    // maybe change, after mount and remount operation. So Use absolute path replace
    // URI(//conten://media/external/images/media/id)  @{
    public String getAbsloutePath(Uri imageUri) {
        Log.d(TAG, "<getAbsloutePath> Single Photo mode :imageUri=" + imageUri);
        if (imageUri == null) {
            return null;
        }
        String absloutePath = null;
        Cursor cursor = null;
        try {
            cursor = getContentResolver().query(
                    imageUri,
                    new String[]{ImageColumns.DATA},
                    null,
                    null,
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                absloutePath = cursor.getString(0);
                Log.d(TAG, "<getAbsloutePath> get absolute path =" + absloutePath);
            }
        } catch (final SQLiteException e) {
            e.printStackTrace();
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return absloutePath;
    }
    /// @}

    /// M: [FEATURE.ADD] [Runtime permission] @{
    @Override
    public void onRequestPermissionsResult(int requestCode,
            String[] permissions, int[] grantResults) {
        if (PermissionHelper.isAllPermissionsGranted(permissions, grantResults)) {
            Log.d(TAG, "<onRequestPermissionsResult> all permission granted");
            if (ApiHelper.HAS_REMOTE_VIEWS_SERVICE) {
                Intent intent = new Intent(this, WidgetTypeChooser.class);
                startActivityForResult(intent, REQUEST_WIDGET_TYPE);
            } else { // Choose the photo type widget
                setWidgetType(new Intent()
                        .putExtra(KEY_WIDGET_TYPE, R.id.widget_type_photo));
            }
        } else {
            Log.d(TAG, "<onRequestPermissionsResult> permission denied, finish");
            PermissionHelper.showDeniedPrompt(this);
            finish();
        }
    }
    /// @}
}
