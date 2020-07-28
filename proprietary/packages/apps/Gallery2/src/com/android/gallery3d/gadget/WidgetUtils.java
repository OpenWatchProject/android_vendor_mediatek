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

import android.appwidget.AppWidgetManager;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Paint;

import com.android.gallery3d.R;
import com.android.gallery3d.data.MediaItem;
import com.android.gallery3d.gadget.WidgetDatabaseHelper.Entry;
import com.android.gallery3d.util.ThreadPool;
import com.mediatek.gallery3d.util.Log;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class WidgetUtils {

    private static final String TAG = "Gallery2/WidgetUtils";

    private static int sStackPhotoWidth = 220;
    private static int sStackPhotoHeight = 170;

    /// M: [BUG.ADD] get context in widget.@{
    public static Context sContext;
    /// @}

    private WidgetUtils() {
    }

    public static void initialize(Context context) {
        /// M: [BUG.ADD] get context in widget.@{
        sContext = context;
        /// @}
        Resources r = context.getResources();
        sStackPhotoWidth = r.getDimensionPixelSize(R.dimen.stack_photo_width);
        sStackPhotoHeight = r.getDimensionPixelSize(R.dimen.stack_photo_height);
    }

    public static Bitmap createWidgetBitmap(MediaItem image) {
        /// M: [BUG.ADD] @{
        if (image == null) {
            Log.d(TAG, "<createWidgetBitmap> image == null, return null");
            return null;
        }
        Log.d(TAG, "<createWidgetBitmap> decode image path = " + image.getFilePath());
        /// @}
        Bitmap bitmap = image.requestImage(MediaItem.TYPE_THUMBNAIL)
               .run(ThreadPool.JOB_CONTEXT_STUB);
        if (bitmap == null) {
            Log.w(TAG, "fail to get image of " + image.toString());
            return null;
        }
        return createWidgetBitmap(bitmap, image.getRotation());
    }

    public static Bitmap createWidgetBitmap(Bitmap bitmap, int rotation) {
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();

        float scale;
        if (((rotation / 90) & 1) == 0) {
            scale = Math.max((float) sStackPhotoWidth / w,
                    (float) sStackPhotoHeight / h);
        } else {
            scale = Math.max((float) sStackPhotoWidth / h,
                    (float) sStackPhotoHeight / w);
        }

        Bitmap target = Bitmap.createBitmap(
                sStackPhotoWidth, sStackPhotoHeight, Config.ARGB_8888);
        Canvas canvas = new Canvas(target);
        canvas.translate(sStackPhotoWidth / 2, sStackPhotoHeight / 2);
        canvas.rotate(rotation);
        canvas.scale(scale, scale);
        Paint paint = new Paint(Paint.FILTER_BITMAP_FLAG | Paint.DITHER_FLAG);
        canvas.drawBitmap(bitmap, -w / 2, -h / 2, paint);
        return target;
    }

    public static void notifyAllWidgetViewChanged() {
        List<Integer> ids = getAllWidgetId();
        Iterator<Integer> itr = ids.iterator();
        while (itr.hasNext()) {
            AppWidgetManager.getInstance(sContext).notifyAppWidgetViewDataChanged(itr.next(),
                    R.id.appwidget_stack_view);
        }
    }

    private static List<Integer> getAllWidgetId() {
        WidgetDatabaseHelper dbHelper = new WidgetDatabaseHelper(sContext);
        List<Integer> ids = new ArrayList<Integer>();
        List<Entry> entry1 = dbHelper.getEntries(WidgetDatabaseHelper.TYPE_SINGLE_PHOTO);
        List<Entry> entry2 = dbHelper.getEntries(WidgetDatabaseHelper.TYPE_SHUFFLE);
        List<Entry> entry3 = dbHelper.getEntries(WidgetDatabaseHelper.TYPE_ALBUM);
        putIdsToList(entry1, ids);
        putIdsToList(entry2, ids);
        putIdsToList(entry3, ids);
        return ids;
    }

    private static void putIdsToList(List<Entry> entry, List<Integer> ids) {
        Iterator<Entry> itr = entry.iterator();
        while (itr.hasNext()) {
            ids.add(itr.next().widgetId);
        }
    }
}
