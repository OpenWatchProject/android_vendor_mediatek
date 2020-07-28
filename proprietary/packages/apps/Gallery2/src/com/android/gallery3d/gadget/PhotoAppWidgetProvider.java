/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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
import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.widget.RemoteViews;

import com.android.gallery3d.R;
import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.gadget.WidgetDatabaseHelper.Entry;
import com.android.gallery3d.onetimeinitializer.GalleryWidgetMigrator;
import com.mediatek.gallery3d.util.Log;

public class PhotoAppWidgetProvider extends AppWidgetProvider {

    private static final String TAG = "Gallery2/PhotoAppWidgetProvider";

    static RemoteViews buildWidget(Context context, int id, Entry entry) {

        switch (entry.type) {
            case WidgetDatabaseHelper.TYPE_ALBUM:
            case WidgetDatabaseHelper.TYPE_SHUFFLE:
                return buildStackWidget(context, id, entry);
            case WidgetDatabaseHelper.TYPE_SINGLE_PHOTO:
                return buildFrameWidget(context, id, entry);
        }
        throw new RuntimeException("invalid type - " + entry.type);
    }

    @Override
    public void onUpdate(Context context,
            AppWidgetManager appWidgetManager, int[] appWidgetIds) {

        if (ApiHelper.HAS_REMOTE_VIEWS_SERVICE) {
            // migrate gallery widgets from pre-JB releases to JB due to bucket ID change
            GalleryWidgetMigrator.migrateGalleryWidgets(context);
        }

        WidgetDatabaseHelper helper = new WidgetDatabaseHelper(context);
        try {
            for (int id : appWidgetIds) {
                Entry entry = helper.getEntry(id);
                /// M: [DEBUG.ADD] @{
                Log.d(TAG, " <onUpdate>: entry for id[" + id + "]="
                        + (entry == null ? "null" : ("(" + entry.type + ", " + entry.imageUri
                                + ", " + entry.albumPath + ", " + entry.imageData + ")")));
                /// @}
                if (entry != null) {
                    RemoteViews views = buildWidget(context, id, entry);
                    appWidgetManager.updateAppWidget(id, views);
                } else {
                    RemoteViews views = new RemoteViews(context.getPackageName(),
                            R.layout.appwidget_main);
                    views.setEmptyView(R.id.appwidget_stack_view, R.id.appwidget_empty_view);
                    appWidgetManager.updateAppWidget(id, views);
                    Log.e(TAG, "<onUpdate>cannot load widget: " + id);
                }
            }
        } finally {
            helper.close();
        }
        super.onUpdate(context, appWidgetManager, appWidgetIds);
    }

    @SuppressWarnings("deprecation")
    @TargetApi(ApiHelper.VERSION_CODES.HONEYCOMB)
    private static RemoteViews buildStackWidget(Context context, int widgetId, Entry entry) {
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<buildStackWidget> for id=" + widgetId + ", entry=(" + entry.type + ", "
                + entry.imageUri + ", " + entry.albumPath + ", " + entry.imageData + ")");
        /// @}
        RemoteViews views = new RemoteViews(context.getPackageName(), R.layout.appwidget_main);

        Intent intent = new Intent(context, WidgetService.class);
        intent.putExtra(AppWidgetManager.EXTRA_APPWIDGET_ID, widgetId);
        intent.putExtra(WidgetService.EXTRA_WIDGET_TYPE, entry.type);
        intent.putExtra(WidgetService.EXTRA_ALBUM_PATH, entry.albumPath);
        intent.setData(Uri.parse("widget://gallery/" + widgetId));

        // We use the deprecated API for backward compatibility
        // The new API is available in ICE_CREAM_SANDWICH (15)
        views.setRemoteAdapter(widgetId, R.id.appwidget_stack_view, intent);

        views.setEmptyView(R.id.appwidget_stack_view, R.id.appwidget_empty_view);

        Intent clickIntent = new Intent(context, WidgetClickHandler.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(
                context, 0, clickIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        views.setPendingIntentTemplate(R.id.appwidget_stack_view, pendingIntent);

/*
        /// M: [FEATURE.ADD] [Runtime permission] @{
        Intent clickIntentForEmpty = new Intent(context, WidgetClickHandler.class);
        clickIntentForEmpty.putExtra(WidgetClickHandler.FLAG_FROM_EMPTY_VIEW, true);
        clickIntentForEmpty.putExtra(WidgetClickHandler.FLAG_WIDGET_ID, widgetId);
        PendingIntent pendingIntentForEmpty = PendingIntent.getActivity(context, 0,
                clickIntentForEmpty, PendingIntent.FLAG_UPDATE_CURRENT);
        views.setOnClickPendingIntent(R.id.appwidget_empty_view, pendingIntentForEmpty);
        /// @}
*/
        return views;
    }

    static RemoteViews buildFrameWidget(Context context, int appWidgetId, Entry entry) {
        /// M: [DEBUG.ADD] @{
        Log.d(TAG, "<buildFrameWidget> for id=" + appWidgetId + ", entry=(" + entry.type + ", "
                + entry.imageUri + ", " + entry.albumPath + ", " + entry.imageData + ")");
        /// @}
        RemoteViews views = new RemoteViews(
                context.getPackageName(), R.layout.photo_frame);
        try {
            byte[] data = entry.imageData;
            /// M: [BUG.ADD] make this bitmap mutable for editing@{
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inMutable = true;
            /// @}
            /// M: [BUG.MODIFY] @{
            /*  Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);*/
            Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length, options);
            /// @}
            views.setImageViewBitmap(R.id.photo, bitmap);
        } catch (Throwable t) {
            Log.w(TAG, "<buildFrameWidget> cannot load widget image: " + appWidgetId, t);
        }

        if (entry.imageUri != null) {
            try {
                Uri uri = Uri.parse(entry.imageUri);
                Intent clickIntent = new Intent(context, WidgetClickHandler.class)
                        .setData(uri);
                /// M: [BUG.ADD] fix bug that permission dialog pop up twice when click deny. @{
                clickIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
                Log.d(TAG, "<buildFrameWidget> set FLAG_ACTIVITY_CLEAR_TASK..");
                /// @}
                /// M: [BUG.MODIFY] @{
                // After add many frame widgets for one same image,
                // old PendingIntent will be canceled because of PendingIntent.FLAG_CANCEL_CURRENT
                /*PendingIntent pendingClickIntent = PendingIntent.getActivity(context, 0,
                      clickIntent, PendingIntent.FLAG_CANCEL_CURRENT);*/
                PendingIntent pendingClickIntent = PendingIntent.getActivity(context, 0,
                        clickIntent, PendingIntent.FLAG_UPDATE_CURRENT);
                /// @}
                views.setOnClickPendingIntent(R.id.photo, pendingClickIntent);
            } catch (Throwable t) {
                Log.w(TAG, "<buildFrameWidget>cannot load widget uri: " + appWidgetId, t);
            }
        }
        return views;
    }

    @Override
    public void onDeleted(Context context, int[] appWidgetIds) {
        // Clean deleted photos out of our database
        WidgetDatabaseHelper helper = new WidgetDatabaseHelper(context);
        for (int appWidgetId : appWidgetIds) {
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<onDeleted>onDelete: id=" + appWidgetId);
            /// @}
            helper.deleteEntry(appWidgetId);
        }
        helper.close();
    }
}
