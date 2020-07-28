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
package com.android.camera.manager;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.media.CameraProfile;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.support.v4.content.LocalBroadcastManager;
import android.view.View;
import android.view.View.OnClickListener;

import com.android.camera.CameraActivity;
import com.android.camera.FileSaver;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.SaveRequest;
import com.android.camera.Storage;
import com.android.camera.Thumbnail;
import com.android.camera.Util;
import com.android.camera.ui.RotateImageView;
import com.mediatek.camera.util.CameraAnimation;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

//TODO: we should do decoupling to move it into mediatek/ui
public class ThumbnailViewManager extends ViewManager implements OnClickListener,
        FileSaver.FileSaverListener, CameraActivity.Resumable {
    private static final String TAG = "ThumbnailViewManager";

    private static final int MSG_SAVE_THUMBNAIL = 0;
    private static final int MSG_UPDATE_THUMBNAIL = 1;
    private static final int MSG_SAVE_THUMBNAIL_WITH_YUV = 2;
    private static final int MSG_RELEASE_URI = 3;

    // TODO:dependency with Google packages,should change the code when do
    // decoupling
    // An image view showing the last captured picture thumbnail.
    private RotateImageView mThumbnailView;
    public RotateImageView mPreviewThumb;
    private SaveRequest mCurrentSaveRequest;
    private Thumbnail mThumbnail;
    //
    private AsyncTask<Void, Void, Thumbnail> mLoadThumbnailTask;
    private CameraActivity mActivity;
    private WorkerHandler mWorkerHandler;
    private CameraAnimation mCameraAnimation;
    private Object mLock = new Object();

    private long mRefreshInterval = 0;
    private long mLastRefreshTime;

    private boolean mIsSavingThumbnail;
    private boolean mIsUpdatingThumbnail;

    // add for update thumbnail with yuv data from post view callback.
    private byte[] mYuvData;
    private int mYuvCount;
    private int mYuvWidth;
    private int mYuvHeight;
    private int mYuvOrientation;
    private int mYuvImageFormat;

    //this interface just used for when animation end,will call updatethumbnail view
    public interface AnimationEndListener {
        void onAnianmationEnd();
    }

    public AnimationEndListener mListener = new AnimationEndListener() {
        @Override
        public void onAnianmationEnd() {
            mIsUpdatingThumbnail = false;
            updateThumbnailView();
        }
    };

    public ThumbnailViewManager(CameraActivity context) {
        super(context);
        mActivity = context;
        setFileter(false);
        context.addResumable(this);
        mCameraAnimation = new CameraAnimation();
    }

    @Override
    public void begin() {
        if (mWorkerHandler == null) {
            HandlerThread t = new HandlerThread("thumbnail-creation-thread");
            t.start();
            mWorkerHandler = new WorkerHandler(t.getLooper());
        }
    }

    @Override
    public void resume() {
    }

    @Override
    public void pause() {
        mWorkerHandler.sendEmptyMessage(MSG_RELEASE_URI);
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        if (mThumbnailView != null) {
            mThumbnailView.setEnabled(enabled);
            mThumbnailView.setClickable(enabled);
        }
    }

    @Override
    public void finish() {
        if (mWorkerHandler != null) {
            mWorkerHandler.getLooper().quit();
        }
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.thumbnail);
        mThumbnailView = (RotateImageView) view.findViewById(R.id.thumbnail);
        mThumbnailView.setOnClickListener(this);
        mPreviewThumb = (RotateImageView) view.findViewById(R.id.preview_thumb);
        return view;
    }

    @Override
    protected void onRefresh() {
        updateThumbnailView();
    }

    @Override
    public void onFileSaved(SaveRequest request) {
        Log.d(TAG, "[onFileSaved]... mYuvCount = " + mYuvCount);
        if (request.isIgnoreThumbnail()) {
            return;
        }
        // If current URI is not valid, don't create thumbnail.
        mCurrentSaveRequest = request;
        if (mYuvCount == 0 && request.getUri() != null) {
            Log.d(TAG, "[onFileSaved],send MSG_SAVE_THUMBNAIL.");
            cancelLoadThumbnail();
            mWorkerHandler.removeMessages(MSG_SAVE_THUMBNAIL);
            mWorkerHandler.sendEmptyMessage(MSG_SAVE_THUMBNAIL);
        }
        if (mYuvCount > 0) {
            --mYuvCount;
        }
    }

    @Override
    public void onClick(View v) {
        if (getContext().isCameraIdle() && mThumbnail != null && getThumbnailUri() != null) {
            Log.d(TAG, "[onClick]call gotoGallery.");
            getContext().gotoGallery();
        }
    }
    public void forceUpdate() {
        Log.d(TAG, "[forceUpdate]...");
        // when MediaScanner Scan done, we should get thumbnail form Media Store
        getLastThumbnailUncached();
    }

    public Uri getThumbnailUri() {
        Uri uri = mThumbnail.getUri();
        if (uri == null && mCurrentSaveRequest != null && mCurrentSaveRequest.getUri() != null) {
            uri = mCurrentSaveRequest.getUri();
        }
        Log.d(TAG, "getThumbnailUri = " + uri);
        return uri;
    }

    public String getThumbnailMimeType() {
        String mimeType = mActivity.getContentResolver().getType(getThumbnailUri());
        Log.i(TAG, "getThumbnailMimeType mimeType = " + mimeType);
        return mimeType;
    }

    /**
     * the interface to update thumbnail view, if the data is YUV from post view
     * callback.
     * @param yuvData the YUV data from post view callback.
     * @param yuvWidth the width of YUV image.
     * @param yuvHeight the height of YUV image.
     * @param orientation the phone orientation set to native.
     * @param imageFormat the image format of YUV image.
     */
    public void updateThumbnailViewWithYuv(byte[] yuvData, int yuvWidth,
            int yuvHeight, int orientation, int imageFormat) {
        if (isNeedDumpYuv()) {
            dumpYuv("/sdcard/postView.yuv", yuvData);
        }
        ++mYuvCount;
        Log.d(TAG, "[updateThumbnailViewWithYuv] yuvData = " + yuvData
                + ", yuvWidth = " + yuvWidth + ", yuvHeight = " + yuvHeight
                + ", orientation = " + orientation + ", imageFormat = "
                + imageFormat + ", mYuvCount = " + mYuvCount);
        mYuvData = yuvData;
        mYuvWidth = yuvWidth;
        mYuvHeight = yuvHeight;
        mYuvOrientation = orientation;
        mYuvImageFormat = imageFormat;
        cancelLoadThumbnail();
        mWorkerHandler.removeMessages(MSG_SAVE_THUMBNAIL_WITH_YUV);
        mWorkerHandler.sendEmptyMessage(MSG_SAVE_THUMBNAIL_WITH_YUV);
    }

    public void addFileSaver(FileSaver saver) {
        if (saver != null) {
            saver.addListener(this);
        }
    }

    public void removeFileSaver(FileSaver saver) {
        if (saver != null) {
            saver.removeListener(this);
        }
    }

    public void setRefreshInterval(int ms) {
        mRefreshInterval = ms;
        mLastRefreshTime = System.currentTimeMillis();
    }

    public void updateThumbnailView() {
        if (mThumbnailView != null && !mIsUpdatingThumbnail) {
            if (super.isShowing()) {
                Log.d(TAG, "[updateThumbnailView]showing is true");
                if (mThumbnail != null && mThumbnail.getBitmap() != null) {
                    // here set bitmap null to avoid show last thumbnail in a
                    // moment.
                    Log.d(TAG, "[updateThumbnailView]showing is true,set VISIBLE.");
                    mThumbnailView.setBitmap(null);
                    mThumbnailView.setBitmap(mThumbnail.getBitmap());
                    mThumbnailView.setVisibility(View.VISIBLE);
                } else {
                    Log.d(TAG, "[updateThumbnailView]thumbnail is null,set INVISIBLE!");
                    mThumbnailView.setBitmap(null);
                    mThumbnailView.setVisibility(View.INVISIBLE);
                }
            } else {
                Log.d(TAG, "[updateThumbnailView]showing is false,set INVISIBLE.");
                mThumbnailView.setVisibility(View.INVISIBLE);
            }
        }
    };

    private class LoadThumbnailTask extends AsyncTask<Void, Void, Thumbnail> {

        public LoadThumbnailTask() {
        }

        @Override
        protected Thumbnail doInBackground(Void... params) {
            // Load the thumbnail from the file.
            try {
                ContentResolver resolver = getContext().getContentResolver();
                Thumbnail t = null;
                if (isCancelled()) {
                    Log.w(TAG, "[doInBackground]task is cancel,return.");
                    return null;
                }
                if (t == null && Storage.isStorageReady()) {
                    Thumbnail result[] = new Thumbnail[1];
                    // Load the thumbnail from the media provider.
                    int code = Thumbnail.getLastThumbnailFromContentResolver(
                            resolver, result, mThumbnail);
                    Log.d(TAG, "getLastThumbnailFromContentResolver code = "
                            + code);
                    switch (code) {
                    case Thumbnail.THUMBNAIL_FOUND:
                        return result[0];
                    case Thumbnail.THUMBNAIL_NOT_FOUND:
                        return null;
                    case Thumbnail.THUMBNAIL_DELETED:
                        // in secure camera, if getContext().getSecureAlbumCount() <= 0,
                        // should continuous to do onPostExecute().
                        if (getContext().isSecureCamera()
                                && getContext().getSecureAlbumCount() <= 0) {
                            return null;
                        }
                        cancel(true);
                        return null;
                    default:
                        return null;
                    }
                }
                return t;
            } catch (Exception ex) {
                ex.printStackTrace();
                return null;
            }
        }

        @Override
        protected void onPostExecute(Thumbnail thumbnail) {
            Log.d(TAG, "[onPostExecute]isCancelled()="
                    + isCancelled());
            if (isCancelled()) {
                return;
            }
            // in secure camera, if getContext().getMediaItemCount() <= 0,
            // there is no need to get thumbnail and should invisible thumbnail
            // view
            if (getContext().isSecureCamera() && getContext().getSecureAlbumCount() <= 0) {
                mThumbnail = null;
            } else {
                mThumbnail = thumbnail;
            }
            updateThumbnailView();
        }
    }

    private void getLastThumbnailUncached() {
        Log.d(TAG, "[getLastThumbnailUncached]...");
        cancelLoadThumbnail();
        synchronized (mLock) {
            mLoadThumbnailTask = new LoadThumbnailTask().execute();
        }
    }

    private void sendUpdateThumbnail() {
        Log.d(TAG, "[sendUpdateThumbnail]...");
        mIsUpdatingThumbnail = true;
        mMainHandler.removeMessages(MSG_UPDATE_THUMBNAIL);
        Message msg = mMainHandler.obtainMessage(MSG_UPDATE_THUMBNAIL, mThumbnail);
        msg.sendToTarget();
    }

    private class WorkerHandler extends Handler {
        public WorkerHandler(Looper looper) {
            super(looper);
            Log.d(TAG, "[WorkerHandler]new...");
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage]WorkerHandler,msg.what = " + msg.what);
            long now = System.currentTimeMillis();
            switch (msg.what) {
            case MSG_SAVE_THUMBNAIL:
                mIsSavingThumbnail = true;
                SaveRequest curRequest = mCurrentSaveRequest;
                // M: initialize the ThumbnaiView to create thumbnail when
                // camera
                // launched by 3rd apps.@{
                if (mThumbnailView == null) {
                    getView();
                }
                // @}
                if (mThumbnailView != null) {
                    if (mRefreshInterval != 0 && (now - mLastRefreshTime < mRefreshInterval)) {
                        Log.d(TAG, "[handleMessage]WorkerHandler, sendEmptyMessageDelayed.");
                        long delay = mRefreshInterval - (now - mLastRefreshTime);
                        sendEmptyMessageDelayed(MSG_SAVE_THUMBNAIL, delay);
                    } else {
                        mLastRefreshTime = now;
                        int thumbnailWidth = mThumbnailView.getLayoutParams().width;
                        Thumbnail thumb = curRequest.createThumbnail(thumbnailWidth);
                        if (thumb != null) { // just update when thumbnail valid
                            mThumbnail = thumb;
                        } else {
                            Log.w(TAG, "[handleMessage]WorkerHandler,thumb is null!");
                        }
                        sendUpdateThumbnail();
                    }
                }
                mIsSavingThumbnail = false;
                break;

            case MSG_SAVE_THUMBNAIL_WITH_YUV:
                mIsSavingThumbnail = true;
                // M: initialize the ThumbnaiView to create thumbnail when
                // camera
                // launched by 3rd apps.@{
                if (mThumbnailView == null) {
                    getView();
                }
                // @}
                if (mThumbnailView != null) {
                    int thumbnailWidth = mThumbnailView.getLayoutParams().width;
                    Thumbnail thumb = createThumbnailWithYuv(mYuvData,
                            thumbnailWidth, mYuvWidth, mYuvHeight,
                            mYuvOrientation, mYuvImageFormat);
                    if (thumb != null) { // just update when thumbnail valid
                        mThumbnail = thumb;
                        sendUpdateThumbnail();
                    } else {
                        Log.w(TAG, "[handleMessage]WorkerHandler,thumb is null!");
                    }
                }
                mIsSavingThumbnail = false;
                break;
            case MSG_RELEASE_URI:
                if (mCurrentSaveRequest != null) {
                    mCurrentSaveRequest.releaseUri();
                }
                break;
            default:
                break;
            }
        }
    }

    private void cancelLoadThumbnail() {
        synchronized (mLock) {
            if (mLoadThumbnailTask != null) {
                Log.d(TAG, "[cancelLoadThumbnail]...");
                mLoadThumbnailTask.cancel(true);
                mLoadThumbnailTask = null;
            }
        }
    }

    private Handler mMainHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (mThumbnail == null) {
                Log.w(TAG, "[handleMessage]mMainHandler,mThumbnail is null,return!");
                return;
            }
            switch (msg.what) {
            case MSG_UPDATE_THUMBNAIL:
                // here set bitmap null to avoid show last thumbnail in a
                // moment.
                // M:Add for CMCC capture performance test case
                Log.d(TAG, "[CMCC Performance test][Camera][Camera] camera capture end ["
                                + System.currentTimeMillis() + "]");
                mPreviewThumb.setBitmap(null);
                mPreviewThumb.setBitmap(mThumbnail.getBitmap());
                mCameraAnimation.doCaptureAnimation(mPreviewThumb,
                        getContext(), mListener);
                break;
            default:
                break;
            }
        }
    };

    private Thumbnail createThumbnailWithYuv(byte[] yuvData,
            int thumbnailWidth, int yuvWidth, int yuvHeight, int orientation, int imageFormat) {
        Thumbnail thumb = null;
        if (yuvData != null) {
            Log.d(TAG, "[createThumbnailWithYuv]...");
            // Create a thumbnail whose width is equal or bigger than
            // that of the thumbnail view.
            int ratio = (int) Math.ceil((double) yuvWidth / thumbnailWidth);
            int inSampleSize = Integer.highestOneBit(ratio);
            thumb = Thumbnail.createThumbnail(
                    covertYuvDataToJpeg(yuvData, yuvWidth, yuvHeight, imageFormat),
                    orientation, inSampleSize, null, null);
        }
        return thumb;
    }

    // Encode YUV to jpeg, and crop it
    private byte[] covertYuvDataToJpeg(byte[] data, int yuvWidth, int yuvHeight, int imageFormat) {
        byte[] jpeg;
        Rect rect = new Rect(0, 0, yuvWidth, yuvHeight);
        // TODO: the yuv data from native must be NV21 or YUY2.
        YuvImage yuvImg = new YuvImage(data, imageFormat, yuvWidth, yuvHeight, null);
        ByteArrayOutputStream outputstream = new ByteArrayOutputStream();
        int jpegQuality = CameraProfile
                .getJpegEncodingQualityParameter(CameraProfile.QUALITY_HIGH);
        yuvImg.compressToJpeg(rect, jpegQuality, outputstream);
        jpeg = outputstream.toByteArray();
        return jpeg;
    }

    private boolean isNeedDumpYuv() {
        boolean enable = SystemProperties.getInt("vendor.debug.thumbnailFromYuv.enable",
                0) == 1 ? true : false;
        Log.d(TAG, "[isNeedDumpYuv] return :" + enable);
        return enable;
    }

    private void dumpYuv(String filePath, byte[] data) {
        FileOutputStream out = null;
        try {
            Log.d(TAG, "[dumpYuv] begin");
            out = new FileOutputStream(filePath);
            out.write(data);
            out.close();
        } catch (IOException e) {
            Log.e(TAG, "[dumpYuv]Failed to write image,ex:", e);
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    Log.e(TAG, "[dumpYuv]IOException:", e);
                }
            }
        }
        Log.d(TAG, "[dumpYuv] end");
    }
}
