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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.android.camera.v2.uimanager;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.support.v4.content.LocalBroadcastManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;

import com.android.camera.R;
import com.android.camera.v2.Thumbnail;
import com.android.camera.v2.app.AppController;
import com.android.camera.v2.ui.RotateImageView;
import com.android.camera.v2.uimanager.ThumbnailAnimation.AnimationListener;
import com.android.camera.v2.util.CameraUtil;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import com.mediatek.camera.v2.services.storage.IStorageService;
import java.io.File;

public class ThumbnailManager extends AbstractUiManager {
    private static final Tag TAG = new Tag(ThumbnailManager.class.getSimpleName());
    private static final String                     ACTION_UPDATE_PICTURE
            = "com.android.gallery3d.action.UPDATE_PICTURE";
    

    private static final int                        MSG_SAVE_THUMBNAIL = 0;
    private static final int                        MSG_CHECK_THUMBNAIL = 1;
    private IStorageService mStorageService;
    private Activity                                mActivity;
    private ContentResolver                         mContentResolver;
    private RotateImageView                         mThumbnailView;
    private RotateImageView                         mPreviewThumb;
    private Handler                                 mMaiHandler;
    private ThumbnailCreatorHandler                 mHandler;
    private Thumbnail                               mThumbnail;
    private ThumbnailAnimation                      mThumbnailAnimation;
    private OnThumbnailClickListener                mOnThumbnailClickListener;
    private AsyncTask<Void, Void, Thumbnail>        mLoadThumbnailTask;
    private Intent                                  mIntent;
    // mShownByIntent is used to judge thumbnail should show or not by intent
    // action.
    private boolean                                 mShownByIntent = true;
    private boolean                                 mResumed = false;
    private boolean mIsSecureCamera = false;
    private boolean mNeedShowSecureCamera = true;

    public interface OnThumbnailClickListener {
        public void onThumbnailClick();
    }


    IntentFilter mUpdatePictureFilter = new IntentFilter(ACTION_UPDATE_PICTURE);
    private BroadcastReceiver mUpdatePictureReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            LogHelper.i(TAG, "mDeletePictureReceiver.onReceive(" + intent + ")");
            // why need add :getContext().isActivityOnpause() ?
            // CS -> onPause()->entry gallery to delete the CS picture,
            // so this time ,isShowing() & isFullScreen() is true,
            // but this time we don't need update the Thumbnail
            if (isShowing()) {
                getLastThumbnailUncached();
            } else {
                //mUpdateThumbnailDelayed = true;
            }
        }
    };

    
    /**
     * Create a thumbnail manager controller.
     * @param appcontroller controller used to get service for storage.
     * @param activity the current activity.
     * @param parent view group.
     * @param secureCamera whether the current camera is secure camera or not.
     */
    public ThumbnailManager(AppController appcontroller, Activity activity,
            ViewGroup parent, boolean secureCamera) {
        super(activity, parent);
        mIsSecureCamera = secureCamera;
        setFilterEnable(false);
        mStorageService = appcontroller.getAppControllerAdapter().getServices().getStorageService();
        mActivity = activity;
        mContentResolver = activity.getContentResolver();
        mMaiHandler = new Handler(activity.getMainLooper());
        HandlerThread t = new HandlerThread("thumbnail-creation-thread");
        t.start();
        mHandler = new ThumbnailCreatorHandler(t.getLooper());
        mThumbnailAnimation = new ThumbnailAnimation();

        LocalBroadcastManager manager = LocalBroadcastManager.getInstance(mActivity);
        manager.registerReceiver(mUpdatePictureReceiver, mUpdatePictureFilter);
        

        mIntent = activity.getIntent();
        String action = null;
        if (mIntent != null) {
            action = mIntent.getAction();
        }
        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
                || MediaStore.ACTION_VIDEO_CAPTURE.equals(action)
                || CameraUtil.ACTION_STEREO3D.equals(action)) {
            mShownByIntent = false;
        }
    }

    @Override
    public void show() {
        LogHelper.i(TAG, "[show], mShownByIntent:" + mShownByIntent);
        if (mShownByIntent) {
            super.show();
        }
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.thumbnail_v2);
        mThumbnailView = (RotateImageView) view.findViewById(R.id.thumbnail);
        mThumbnailView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mOnThumbnailClickListener != null) {
                    mOnThumbnailClickListener.onThumbnailClick();
                }
            }
        });
        mPreviewThumb = (RotateImageView) view.findViewById(R.id.preview_thumb);
        return view;
    }

    @Override
    protected void onRefresh() {
        LogHelper.i(TAG, "[onRefresh]...");
        updateThumbnailView();
    }

    @Override
    public void setEnable(boolean enable) {
        super.setEnable(enable);
        if (mThumbnailView != null) {
            mThumbnailView.setEnabled(enable);
            mThumbnailView.setClickable(enable);
        }
    }

    public Uri getThumbnailUri() {
        if (mThumbnail != null) {
            return Uri.parse("file://" + mThumbnail.getFilePath());
        }
        LogHelper.i(TAG, "[getThumbnailUri], null");
        return null;
    }

    public String getThumbnailMimeType() {
        if (mThumbnail != null) {
            return getMimeType(mThumbnail.getFilePath());
        }
        LogHelper.i(TAG, "[getThumbnailMimeType], null");
        return null;
    }

    private String getMimeType(String filePath) {
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        String mime = "image/jpeg";
        if (filePath != null) {
            try {
                retriever.setDataSource(filePath);
                mime = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_MIMETYPE);
            } catch (IllegalStateException e) {
                return mime;
            } catch (IllegalArgumentException e) {
                return mime;
            } catch (RuntimeException e) {
                return mime;
            }
        }
        LogHelper.i(TAG, "[getMimeType] mime = " + mime);
        return mime;
    }

    public void setOnThumbnailClickListener(OnThumbnailClickListener listener) {
        mOnThumbnailClickListener = listener;
    }

    public void onCreate() {

    }

    /**
     * Used to load latest thumbnail when camera activity onResume.
     */
    public void onResume() {
        LogHelper.i(TAG, "[onResume] mShownByIntent = " + mShownByIntent);
        mResumed = true;
        if (mShownByIntent) {
            mLoadThumbnailTask = new LoadThumbnailTask(false).execute();
        }
    }

    /**
     * Used to save current thumbnail to file when camera activity onPause.
     */
    public void onPause() {
        LogHelper.i(TAG, "[onPause]...");
        mResumed = false;
        cancelLoadThumbnail();
        saveThumbnailToFile();
    }

    /**
     * Used to quit ThumbnailCreatorHandler when camera activity onDestroy.
     */
    public void onDestroy() {
        LogHelper.i(TAG, "[onDestroy]...");
        if (mHandler != null) {
            mHandler.getLooper().quit();
        }
    }

    /**
     * Update thumbnail form Media Store when MediaScanner Scan done.
     */
    public void forceUpdate() {
        getLastThumbnailUncached();
    }

    /**
     * Save thumbnail when received the fileSaved notify.
     * @param uri the fileSaved uri.
     */
    public void notifyFileSaved(Uri uri) {
        LogHelper.i(TAG, "[notifyFileSaved], uri:" + uri + ", mShownByIntent:" + mShownByIntent);
        if (uri == null) {
            return;
        }
        cancelLoadThumbnail();
        mHandler.sendEmptyMessage(MSG_SAVE_THUMBNAIL);
    }

    public void updateNeedShowThumbnail(boolean isNeedShow) {
        LogHelper.i(TAG, "[updateNeedShowThumbnail] isNeedShow " + isNeedShow);
        mNeedShowSecureCamera = isNeedShow;
    }

    private void updateThumbnailView() {
        LogHelper.i(TAG, "[updateThumbnailView]this = " + this);
        if (mThumbnailView != null) {
            if (isShowing()) {
                if (mThumbnail != null && mThumbnail.getBitmap() != null) {
                    // here set bitmap null to avoid show last thumbnail in a
                    // moment.
                    LogHelper.i(TAG, "[updateThumbnailView]showing is true,set VISIBLE.");
                    mThumbnailView.setBitmap(null);
                    mThumbnailView.setBitmap(mThumbnail.getBitmap());
                    mThumbnailView.setVisibility(View.VISIBLE);
                } else {
                    LogHelper.i(TAG, "[updateThumbnailView]showing is true," +
                            "but thumbnail is null,set INVISIBLE!");
                    mThumbnailView.setBitmap(null);
                    mThumbnailView.setVisibility(View.INVISIBLE);
                }
            }
        }
    };

    private Thumbnail getLastThumbnailFromContentResolver(IStorageService storageService,
            ContentResolver resolver) {
        Thumbnail result[] = new Thumbnail[1];
        // Load the thumbnail from the media provider.
        int code = Thumbnail.getLastThumbnailFromContentResolver(storageService.getFileDirectory(),
                resolver,
                result);
        LogHelper.d(TAG, "getLastThumbnailFromContentResolver code = " + code);
        switch (code) {
        case Thumbnail.THUMBNAIL_FOUND:
            return result[0];
        case Thumbnail.THUMBNAIL_NOT_FOUND:
            return null;
        case Thumbnail.THUMBNAIL_DELETED:
            return null;
        default:
            return null;
        }
    }

    private void updateThumbnailViewWithAnimation() {
        if (mThumbnail != null && mPreviewThumb != null) {
            mPreviewThumb.setBitmap(null);
            mPreviewThumb.setBitmap(mThumbnail.getBitmap());

            mThumbnailAnimation.doCaptureAnimation(mPreviewThumb, mActivity,
                    new AnimationListener() {
                @Override
                public void onAnimationEnd() {
                    updateThumbnailView();
                }
            });
        }
    }

    private void saveThumbnailToFile() {
        LogHelper.d(TAG, "[saveThumbnailToFile], mThumbnail:" + mThumbnail);
        if (mThumbnail != null && !mThumbnail.fromFile()) {
            LogHelper.d(TAG, "[saveThumbnailToFile]execute...");
            new SaveThumbnailTask().execute(mThumbnail);
        }
    }


    private void getLastThumbnailUncached() {
        LogHelper.d(TAG, "[getLastThumbnailUncached]...");
        cancelLoadThumbnail();
        mLoadThumbnailTask = new LoadThumbnailTask(false)
                .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    private void cancelLoadThumbnail() {
        if (mLoadThumbnailTask != null) {
            LogHelper.d(TAG, "[cancelLoadThumbnail]...");
            mLoadThumbnailTask.cancel(true);
            mLoadThumbnailTask = null;
        }
    }

    private class ThumbnailCreatorHandler extends Handler {
        public ThumbnailCreatorHandler(Looper looper) {
            super(looper);
            LogHelper.i(TAG, "[ThumbnailCreatorHandler]new...");
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.i(TAG, "[handleMessage]ThumbnailCreatorHandler,msg:" + msg);
            long now = System.currentTimeMillis();
            switch (msg.what) {
            case MSG_SAVE_THUMBNAIL:
                mThumbnail = getLastThumbnailFromContentResolver(
                        mStorageService, mContentResolver);
                // may be receiver new media uri when camera is paused,
                // but it still need to save this
                // new thumbnail to file.
                if (!mResumed) {
                    saveThumbnailToFile();
                }
                if (mMaiHandler != null) {
                    mMaiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            updateThumbnailViewWithAnimation();
                        }
                    });
                }
                break;

            case MSG_CHECK_THUMBNAIL:

                break;

            default:
                break;
            }
        }
    }

    private class LoadThumbnailTask extends AsyncTask<Void, Void, Thumbnail> {
        private boolean mLookAtCache;
        protected LoadThumbnailTask(boolean lookAtCache) {
            mLookAtCache = lookAtCache;
        }

        @Override
        protected Thumbnail doInBackground(Void... params) {
            LogHelper.i(TAG, "[doInBackground]begin.mLookAtCache = " + mLookAtCache);
            // Load the thumbnail from the file.
            Thumbnail t = null;
            if (mLookAtCache) {
                t = Thumbnail.getLastThumbnailFromFile(mStorageService.getFileDirectory(),
                        mActivity.getFilesDir(), mContentResolver);
            }
            if (isCancelled()) {
                LogHelper.w(TAG, "[doInBackground]task is cancel,return.");
                return null;
            }

            if (t == null && mStorageService.isStorageReady()) {
                t = getLastThumbnailFromContentResolver(mStorageService, mContentResolver);
            }
            return t;
        }

        @Override
        protected void onPostExecute(Thumbnail thumbnail) {
            LogHelper.d(TAG, "[onPostExecute]isCancelled()="
                    + isCancelled() + ",mIsSecureCamera = " + mIsSecureCamera +
                    ",mNeedShowSecureCamera = " + mNeedShowSecureCamera);
            if (isCancelled()) {
                return;
            }
            // in secure camera, if getContext().getMediaItemCount() <= 0,
            // there is no need to get thumbnail and should invisible thumbnail
            // view
            if (mIsSecureCamera && !mNeedShowSecureCamera) {
                mThumbnail = null;
            } else {
                mThumbnail = thumbnail;
            }
            updateThumbnailView();
        }
    }

    private class SaveThumbnailTask extends AsyncTask<Thumbnail, Void, Void> {
        @Override
        protected Void doInBackground(Thumbnail... params) {
            final int n = params.length;
            LogHelper.d(TAG, "[doInBackground]length = " + n);
            final File filesDir = mActivity.getFilesDir();
            for (int i = 0; i < n; i++) {
                params[i].saveLastThumbnailToFile(filesDir);
            }
            return null;
        }
    }
}
