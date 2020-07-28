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
import android.appwidget.AppWidgetManager;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.widget.RemoteViews;
import android.widget.RemoteViewsService;

import com.android.gallery3d.R;
import com.android.gallery3d.app.GalleryApp;
import com.android.gallery3d.app.GalleryAppImpl;
import com.android.gallery3d.common.ApiHelper;
import com.android.gallery3d.data.ContentListener;
import com.android.gallery3d.data.ImageCacheService;
import com.android.gallery3d.data.MediaItem;
import com.mediatek.gallery3d.adapter.PhotoPlayFacade;
import com.mediatek.gallery3d.util.Log;
import com.mediatek.gallery3d.util.PermissionHelper;
import com.mediatek.gallerybasic.base.MediaFilter;
import com.mediatek.gallerybasic.base.MediaFilterSetting;
import com.mediatek.omadrm.OmaDrmStore;

@TargetApi(ApiHelper.VERSION_CODES.HONEYCOMB)
public class WidgetService extends RemoteViewsService {

    @SuppressWarnings("unused")
    private static final String TAG = "Gallery2/GalleryAppWidgetService";

    public static final String EXTRA_WIDGET_TYPE = "widget-type";
    public static final String EXTRA_ALBUM_PATH = "album-path";

    @Override
    public RemoteViewsFactory onGetViewFactory(Intent intent) {
        int id = intent.getIntExtra(AppWidgetManager.EXTRA_APPWIDGET_ID,
                AppWidgetManager.INVALID_APPWIDGET_ID);
        int type = intent.getIntExtra(EXTRA_WIDGET_TYPE, 0);
        String albumPath = intent.getStringExtra(EXTRA_ALBUM_PATH);

        return new PhotoRVFactory((GalleryApp) getApplicationContext(),
                id, type, albumPath);
    }

    /// M: [FEATURE.MODIFY] @{
    /*private static class PhotoRVFactory implements
              RemoteViewsService.RemoteViewsFactory, ContentListener {*/
    private class PhotoRVFactory implements
            RemoteViewsService.RemoteViewsFactory, ContentListener {
    /// @}

        private final int mAppWidgetId;
        private final int mType;
        private final String mAlbumPath;
        private final GalleryApp mApp;

        private WidgetSource mSource;

        public PhotoRVFactory(GalleryApp app, int id, int type, String albumPath) {
            mApp = app;
            mAppWidgetId = id;
            mType = type;
            mAlbumPath = albumPath;
        }

        @Override
        public void onCreate() {
            /// M: [FEATURE.ADD] @{
            Log.d(TAG, "<PhotoRVFactory.onCreate>");
            /// create new cache for gallery widget for avoid synchronizations cache file issue.
            ImageCacheService.setCacheName("imagecacheforwidget");
            PhotoPlayFacade.initialize((GalleryAppImpl) WidgetService.this.getApplication(),
                    MediaItem.getTargetSize(MediaItem.TYPE_MICROTHUMBNAIL),
                    MediaItem.getTargetSize(MediaItem.TYPE_THUMBNAIL),
                    MediaItem.getTargetSize(MediaItem.TYPE_HIGHQUALITYTHUMBNAIL));
            PhotoPlayFacade.registerWidgetMedias(mApp.getAndroidContext());
            MediaFilter filter = new MediaFilter();
            Intent widgetRequestIntent = new Intent();
            widgetRequestIntent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                    OmaDrmStore.DrmIntentExtra.LEVEL_FL);
            filter.setFlagFromIntent(widgetRequestIntent);
            MediaFilterSetting.setCurrentFilter(WidgetService.this, filter);
            /// @}
            /// M: [BUG.ADD] using a HandlerThread to do some operations @{
            // initialize HandlerThread and Handler when onCreate
            initHandler();
            /// @}
            if (mType == WidgetDatabaseHelper.TYPE_ALBUM) {
                mSource = new MediaSetSource(mApp.getDataManager(), mAlbumPath);
            } else {
                mSource = new LocalPhotoSource(mApp.getAndroidContext());
            }
            mSource.setContentListener(this);
            /// M: [BUG.MODIFY] @{
            // Since mSource.reload has been moved to run in HandlerThread when MSG_CONTENT_DIRTY,
            // not in onDataSetChanged, so do not notifyAppWidgetViewDataChanged here,
            // but call onContentDirty to trigger reload for the first time.
            /*AppWidgetManager.getInstance(mApp.getAndroidContext())
             .notifyAppWidgetViewDataChanged(
             mAppWidgetId, R.id.appwidget_stack_view);*/
            onContentDirty();
            /// @}
        }

        @Override
        public void onDestroy() {
            /// M: [FEATURE.ADD] @{
            Log.d(TAG, "<PhotoRVFactory.onDestroy>");
            MediaFilterSetting.removeFilter(WidgetService.this);
            /// @}
            /// M: [BUG.ADD] using a HandlerThread to do some operations @{
            // destroy HandlerThread and Handler when onDestroy
            closeHandler();
            /// @}
            mSource.close();
            mSource = null;
        }

        @Override
        public int getCount() {
            /// M: [BUG.ADD] getCount maybe called in another binder call thread
            // after onDestory which lead to the null pointer exception
            if (mSource == null) {
                Log.d(TAG, "<PhotoRVFactory.getCount> mSource is null, return 0");
                return 0;
            }
            /// @}
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<PhotoRVFactory.getCount> count=" + mSource.size());
            /// @}
            return mSource.size();
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getViewTypeCount() {
            /// M: [BUG.MODIFY] @{
            // Gallery support 2 layout:
            // R.layout.appwidget_loading_item and R.layout.appwidget_photo_item
            // so need return 2 to avoid RemoteView Error
            return 2;
            /// @}
        }

        @Override
        public boolean hasStableIds() {
            return true;
        }

        @Override
        public RemoteViews getLoadingView() {
            RemoteViews rv = new RemoteViews(
                    mApp.getAndroidContext().getPackageName(),
                    R.layout.appwidget_loading_item);
            rv.setProgressBar(R.id.appwidget_loading_item, 0, 0, true);
            return rv;
        }

        @Override
        public RemoteViews getViewAt(int position) {
            /// M: [BUG.ADD] if any exception happens here, it most probably means
            // that onDestroy has been called and mSource is not available anymore.@{
            Log.d(TAG, "<PhotoRVFactory.getViewAt> " + position);
            Uri uri = null;
            uri = mSource.getContentUri(position);
            if (null == uri) {
                Log.d(TAG, "<PhotoRVFactory.getViewAt> uri is null, return loading view");
                return getLoadingView();
            }
            /// @}
            Bitmap bitmap = mSource.getImage(position);
            /// M: [DEBUG.MODIFY] @{
            /*if (bitmap == null) return getLoadingView();*/
            if (bitmap == null) {
                Log.d(TAG, "<PhotoRVFactory.getViewAt> bitmap is null, return loading view");
                return getLoadingView();
            }
            /// @}
            RemoteViews views = new RemoteViews(
                    mApp.getAndroidContext().getPackageName(),
                    R.layout.appwidget_photo_item);
            views.setImageViewBitmap(R.id.appwidget_photo_item, bitmap);
            views.setOnClickFillInIntent(R.id.appwidget_photo_item, new Intent()
                    /// M: [BUG.MODIFY] fix bug: pop up permission dialog twice. @{
                    // .setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                    .setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK)
                    /// @}
                    /// M: [BUG.MODIFY] use defined uri.@{
                    .setData(uri));
                    //.setData(mSource.getContentUri(position)));
                    /// @}
            return views;
        }

        @Override
        public void onDataSetChanged() {
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<onDataSetChanged>");
            /// @}
            /// M: [BUG.MARK] run WidgetSource.reload when onContentDirty @{
            /*mSource.reload();*/
            /// @}
            /// M: [FEATURE.ADD] [Runtime permission] @{
            boolean currentPermissionStatus = PermissionHelper
                    .checkStoragePermission(WidgetService.this);
            if (mLastPermissionStatus != currentPermissionStatus && mSource != null) {
                Log.d(TAG, "<onDataSetChanged> forceNotifyDirty");
                mSource.forceNotifyDirty();
            }
            mLastPermissionStatus = currentPermissionStatus;
            /// @}
        }

        @Override
        public void onContentDirty() {
            /// M: [DEBUG.ADD] @{
            Log.d(TAG, "<onContentDirty> send MSG_CONTENT_DIRTY");
            /// @}
            /// M: [BUG.MODIFY] do onContentDirty in another thread @{
            /*AppWidgetManager.getInstance(mApp.getAndroidContext())
                    .notifyAppWidgetViewDataChanged(
                    mAppWidgetId, R.id.appwidget_stack_view);*/
            mHandler.sendEmptyMessage(MSG_CONTENT_DIRTY);
            /// @}
        }

        //********************************************************************
        //*                              MTK                                 *
        //********************************************************************
        // [Runtime permission] @{
        private boolean mLastPermissionStatus = true;
        // @}
        // Because notifyAppWidgetViewDataChanged and onDataSetChanged is not one
        // to one relationship, mSource.reload is not guaranteed.
        // In order to refresh mSource correctly, abandon old flow:
        // [1]onContentDirty -> [2]notifyAppWidgetViewDataChanged
        // -> [3]onDataSetChanged -> [4]mSource.reload
        // New flow:
        // [1]onContentDirty -> [2]send MSG_CONTENT_DIRTY -> [3]handleMessage
        // -> [4]mSource.reload -> [5]notifyAppWidgetViewDataChanged
        private HandlerThread mHandlerThread = null;
        private Handler mHandler = null;
        private static final int INTERVALTIME = 1000;
        private long mStartTime = -1;
        private static final int MSG_CONTENT_DIRTY = 1;

        private void initHandler() {
            mHandlerThread = new HandlerThread("WidgetService-HandlerThread",
                    android.os.Process.THREAD_PRIORITY_BACKGROUND);
            mHandlerThread.start();
            mHandler = new ContentDirtyHandler(mHandlerThread.getLooper());
        }

        private void closeHandler() {
            if (mHandlerThread != null) {
                mHandlerThread.quit();
                mHandlerThread = null;
            }
            mHandler = null;
        }

        private class ContentDirtyHandler extends Handler {
            private static final int RETRY_LAST_ABANDON_MSG = -1;
            private long mLastAbandonMsgId = 0;
            private long mLastReceiveMsgId = 0;
            private Message mLastAbandonMsg;

            public ContentDirtyHandler(Looper looper) {
                super(looper);
            }

            @Override
            public void dispatchMessage(Message message) {
                if (message.what == RETRY_LAST_ABANDON_MSG) {
                    if (mLastAbandonMsgId == mLastReceiveMsgId && mLastAbandonMsg != null) {
                        this.sendMessage(mLastAbandonMsg);
                        mLastAbandonMsg = null;
                    }
                } else {
                    long time = System.currentTimeMillis();
                    long intervalTime = time - mStartTime;
                    mLastReceiveMsgId++;
                    if (intervalTime < INTERVALTIME) {
                        mLastAbandonMsgId = mLastReceiveMsgId;
                        mLastAbandonMsg = Message.obtain(message);
                        Message msg = Message.obtain(this, RETRY_LAST_ABANDON_MSG);
                        // To avoid missing update message, if last message has
                        // been abandon,
                        // we need retry it after INTERVALTIME
                        sendMessageDelayed(msg, INTERVALTIME);
                        return;
                    }
                    mStartTime = time;
                    super.dispatchMessage(message);
                }
            }

            public void handleMessage(Message msg) {
                if (msg.what == MSG_CONTENT_DIRTY) {
                    Log.d(TAG, "<handleMessage> mSource = " + mSource + ", reload");
                    if (mSource != null) {
                        mSource.reload();
                    }
                    Log.d(TAG, "<handleMessage> notifyAppWidgetViewDataChanged");
                    AppWidgetManager.getInstance(mApp.getAndroidContext())
                                    .notifyAppWidgetViewDataChanged(mAppWidgetId,
                                            R.id.appwidget_stack_view);
                }
            }
        }
    }
}