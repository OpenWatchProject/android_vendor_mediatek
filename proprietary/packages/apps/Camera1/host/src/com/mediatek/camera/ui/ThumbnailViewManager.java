/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.ui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.AsyncTask;
import android.os.Build;
import android.support.v4.graphics.drawable.RoundedBitmapDrawable;
import android.support.v4.graphics.drawable.RoundedBitmapDrawableFactory;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.animation.AccelerateInterpolator;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUiListener.OnThumbnailClickedListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.common.widget.RotateImageView;
import com.mediatek.camera.portability.storage.StorageManagerExt;

import java.lang.reflect.Method;

/**
 * A manager for ThumbnailView.
 */
class ThumbnailViewManager extends AbstractViewManager {
    private static final Tag TAG = new Tag(
            ThumbnailViewManager.class.getSimpleName());
    private static final String INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE =
            "android.media.action.STILL_IMAGE_CAMERA_SECURE";
    private static final int THUMB_COLOR = 0XFF303030;
    private static final int BORDER_COLOR = 0x4D000000;
    private static final int ANIMATION_COLOR = 0XFFFFFFFF;
    private static final int ANIMATION_DURATION = 0;
    private static final float ANIMATION_ALPHA_START = 0.0f;
    private static final float ANIMATION_ALPHA_END = 1.0f;
    private final Context mContext;
    private Bitmap mRoundedBitmap = null;
    private RotateImageView mThumbnailView;
    private RotateImageView mAnimationView;
    private RoundedBitmapDrawable mRoundDrawable;
    private RoundedBitmapDrawable mAnimationDrawable;

    private Object mLock = new Object();
    private AsyncTask<Void, Void, Bitmap> mLoadBitmapTask;
    private OnThumbnailClickedListener mOnClickListener;

    private int mThumbnailViewWidth;
    private boolean mIsNeedQueryDB = true;

    /**
     * constructor of ThumbnailViewManager.
     * @param app The {@link IApp} implementer.
     * @param parentView the root view of ui.
     */
    public ThumbnailViewManager(IApp app, ViewGroup parentView) {
        super(app, parentView);
        mContext = app.getActivity();
    }

    @Override
    protected View getView() {
        LogHelper.d(TAG, "[getView]...");
        mThumbnailView = (RotateImageView) mParentView
                .findViewById(R.id.thumbnail);
        mAnimationView = (RotateImageView) mParentView
                .findViewById(R.id.thumbnail_animation);
        mRoundDrawable = createRoundDrawable(null, THUMB_COLOR);
        mAnimationDrawable = createRoundDrawable(null, ANIMATION_COLOR);
        mThumbnailView.setImageDrawable(mRoundDrawable);
        mAnimationView.setImageDrawable(mAnimationDrawable);
        mThumbnailView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mOnClickListener != null) {
                    mOnClickListener.onThumbnailClicked();
                }
            }
        });
        mThumbnailViewWidth = Math.min(
                mThumbnailView.getLayoutParams().width,
                mThumbnailView.getLayoutParams().height);

        return mThumbnailView;
    }

    public void setThumbnailClickedListener(OnThumbnailClickedListener listener) {
        mOnClickListener = listener;
    }

    @Override
    public void onResume() {
        LogHelper.d(TAG, "[onResume]");
        super.onResume();
        if (isExtendStorageCanUsed()) {
            registerIntentFilter();
            sMountPoint = StorageManagerExt.getDefaultPath();
        }
        if (mIsNeedQueryDB) {
            getLastThumbnail();
        }
        mIsNeedQueryDB = true;
    }

    @Override
    public void onPause() {
        LogHelper.d(TAG, "[onPause]");
        super.onPause();
        if (isExtendStorageCanUsed()) {
            unregisterIntentFilter();
        }
//        if (mRoundedBitmap != null) {
 //           mRoundedBitmap.recycle();
 //           mRoundedBitmap = null;
 //       }
    }

    @Override
    public void setEnabled(boolean enabled) {
        LogHelper.d(TAG, "[setEnabled] enabled = " + enabled);
        if (mThumbnailView != null) {
            mThumbnailView.setClickable(enabled);
        }
    }

    /**
     * Get get the width of thumbnail view.
     * @return The min value of width and height of thumbnail view.
     */
    public int getThumbnailViewWidth() {
        return mThumbnailViewWidth;
    }


    /**
     * update thumbnailView.
     * @param bitmap
     *            the bitmap matched with the picture or video, such as
     *            orientation, content. suggest thumbnail view size.
     */
    public void updateThumbnail(Bitmap bitmap) {
        updateThumbnailView(bitmap);
        if (bitmap != null) {
            doAnimation(mAnimationView);
        } else {
            // for security camera not query db.
            mThumbnailView.setImageDrawable(mRoundDrawable);
            mIsNeedQueryDB = false;
        }
    }

    private RoundedBitmapDrawable createRoundDrawable(Bitmap bitmap, final int color) {
        int thumbWidth = mThumbnailView.getLayoutParams().width;
        int thumbHeight = mThumbnailView.getLayoutParams().height;
        //setContentDescription is added for testing
        mThumbnailView.setContentDescription("Has Content");
        // crop a square bitmap according minimum edge.
        if (bitmap == null) {
            bitmap = Bitmap.createBitmap(thumbWidth, thumbHeight,
                    Bitmap.Config.ARGB_8888);
            mThumbnailView.setContentDescription("No Content");
        }
        int bmpWidth = bitmap.getWidth();
        int bmpHeight = bitmap.getHeight();
        Bitmap squareBitmap;
        if (bmpHeight > bmpWidth) {
            squareBitmap = Bitmap.createBitmap(bitmap, 0,
                    (bmpHeight - bmpWidth) / 2, bmpWidth, bmpWidth);
        } else if (bmpHeight < bmpWidth) {
            squareBitmap = Bitmap.createBitmap(bitmap,
                    (bmpWidth - bmpHeight) / 2, 0, bmpHeight, bmpHeight);
        } else {
            squareBitmap = bitmap;
        }
        // center scale to the size of thumbnail view.
        Bitmap scaledBitmap = Bitmap.createScaledBitmap(squareBitmap,
                thumbWidth, thumbHeight, true);
        // Calculate the bitmap radius
        int borderWidthHalf = 2;
        int bitmapWidth = scaledBitmap.getWidth();
        int bitmapHeight = scaledBitmap.getHeight();
        int bitmapRadius = Math.min(bitmapWidth, bitmapHeight) / 2;
        int bitmapSquareWidth = Math.min(bitmapWidth, bitmapHeight);
        int newBitmapSquareWidth = bitmapSquareWidth - borderWidthHalf * 2;
        /*
         * Initializing a new empty bitmap. Set the bitmap size from source
         * bitmap Also add the border space to new bitmap
         */
        //if (mRoundedBitmap != null) {
        //    mRoundedBitmap.recycle();
        //    mRoundedBitmap = null;
        //}
        mRoundedBitmap = Bitmap.createBitmap(newBitmapSquareWidth,
                newBitmapSquareWidth, Bitmap.Config.ARGB_8888);
        // Initialize a new Canvas to draw empty bitmap
        Canvas canvas = new Canvas(mRoundedBitmap);
        // Draw a solid color to canvas
        canvas.drawColor(color);
        // Calculation to draw bitmap at the circular bitmap center position
        int centerX = newBitmapSquareWidth - bitmapWidth;
        int centerY = newBitmapSquareWidth - bitmapHeight;
        /*
         * Now draw the bitmap to canvas. Bitmap will draw its center to
         * circular bitmap center by keeping border spaces
         */
        canvas.drawBitmap(scaledBitmap, centerX, centerY, null);
        // Initializing a new Paint instance to draw circular border
        Paint borderPaint = new Paint();
        borderPaint.setStyle(Paint.Style.STROKE);
        borderPaint.setStrokeWidth(borderWidthHalf * 2);
        borderPaint.setColor(BORDER_COLOR);
        /*
         * Draw the circular border to bitmap. Draw the circle at the center of
         * canvas.
         */
        canvas.drawCircle(canvas.getWidth() / 2, canvas.getWidth() / 2,
                newBitmapSquareWidth / 2, borderPaint);
        // Create a new RoundedBitmapDrawable
        RoundedBitmapDrawable roundedBitmapDrawable = RoundedBitmapDrawableFactory
                .create(mApp.getActivity().getResources(), mRoundedBitmap);
        // Set the corner radius of the bitmap drawable
        roundedBitmapDrawable.setCornerRadius(bitmapRadius);
        roundedBitmapDrawable.setAntiAlias(true);
        bitmap.recycle();
        squareBitmap.recycle();
        scaledBitmap.recycle();
        bitmap = null;
        squareBitmap = null;
        scaledBitmap = null;
        return roundedBitmapDrawable;
    }

    private void cancelLoadThumbnail() {
        synchronized (mLock) {
            if (mLoadBitmapTask != null) {
                LogHelper.d(TAG, "[cancelLoadThumbnail]...");
                mLoadBitmapTask.cancel(true);
                mLoadBitmapTask = null;
            }
        }
    }

    private void updateThumbnailView(Bitmap bitmap) {
        LogHelper.d(TAG, "[updateThumbnailView]...");
        if (mThumbnailView != null) {
            if (bitmap != null) {
                LogHelper.d(TAG, "[updateThumbnailView] set created thumbnail");
                mRoundDrawable = createRoundDrawable(bitmap, THUMB_COLOR);
            } else {
                LogHelper.d(TAG, "[updateThumbnailView] set default thumbnail");
                mRoundDrawable = createRoundDrawable(null, THUMB_COLOR);
            }
        }
    }

    private void getLastThumbnail() {
        cancelLoadThumbnail();
        synchronized (mLock) {
            mLoadBitmapTask = new LoadBitmapTask().execute();
        }
    }

    /**
     * AsyncTask that used for getting the latest bitmap from data base.
     */
    private class LoadBitmapTask extends AsyncTask<Void, Void, Bitmap> {

        public LoadBitmapTask() {
        }

        @Override
        protected Bitmap doInBackground(Void... params) {
            LogHelper.d(TAG, "[doInBackground]begin.");
            try {
                Bitmap bitmap = null;
                if (isCancelled()) {
                    LogHelper.w(TAG, "[doInBackground]task is cancel,return.");
                    return null;
                }
                bitmap = BitmapCreator.getLastBitmapFromDatabase(
                                            mApp.getActivity().getContentResolver());
                mApp.notifyNewMedia(BitmapCreator.getUriAfterQueryDb(), false);
                LogHelper.d(TAG, "getLastBitmapFromDatabase bitmap = " + bitmap);
                return bitmap;
            } catch (Exception ex) {
                ex.printStackTrace();
                return null;
            }
        }

        @Override
        protected void onPostExecute(Bitmap bitmap) {
            LogHelper.d(TAG, "[onPostExecute]isCancelled()="
                    + isCancelled());
            if (isCancelled()) {
                return;
            }
            updateThumbnailView(bitmap);
            mThumbnailView.setImageDrawable(mRoundDrawable);
        }
    }

    private void doAnimation(RotateImageView view) {
        view.clearAnimation();
        AlphaAnimation alphaAnimation = new AlphaAnimation(
                ANIMATION_ALPHA_START, ANIMATION_ALPHA_END);
        alphaAnimation.setDuration(ANIMATION_DURATION);
        alphaAnimation.setInterpolator(new AccelerateInterpolator());
        alphaAnimation.setRepeatMode(Animation.REVERSE);
        alphaAnimation.setRepeatCount(1);
        view.startAnimation(alphaAnimation);
        alphaAnimation.setAnimationListener(new Animation.AnimationListener() {
            @Override
            public void onAnimationStart(Animation animation) {

            }

            @Override
            public void onAnimationEnd(Animation animation) {

            }

            @Override
            public void onAnimationRepeat(Animation animation) {
                mThumbnailView.setImageDrawable(mRoundDrawable);
            }
        });
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            LogHelper.i(TAG, "mReceiver.onReceive(" + intent + ")");
            String action = intent.getAction();
            if (action == null) {
                LogHelper.d(TAG, "[mReceiver.onReceive] action is null");
                return;
            }
            switch (intent.getAction()) {
                case Intent.ACTION_MEDIA_EJECT:
                case Intent.ACTION_MEDIA_SCANNER_FINISHED:
                    Intent activityIntent = mApp.getActivity().getIntent();
                    String activityAction = activityIntent.getAction();
                    if (INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE.equals(activityAction)) {
                        LogHelper.d(TAG, "[mReceiver.onReceive] security camera");
                        if (isSameStorage(intent)) {
                            LogHelper.d(TAG, "[mReceiver.onReceive]" +
                                    " the eject media is same storage.");
                            updateThumbnail(null);
                        }
                        return;
                    }
                    getLastThumbnail();
                    break;
                case Intent.ACTION_MEDIA_UNMOUNTED:
                    sMountPoint = StorageManagerExt.getDefaultPath();
                    break;
                default:
                    break;
            }
        }
    };
    /**
     * Register an intent filter to receive SD card related events.
     * storage monitor will receive these action when storage change.
     */
    private void registerIntentFilter() {
        IntentFilter intentFilter = new IntentFilter(
                Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        intentFilter.addAction(Intent.ACTION_MEDIA_CHECKING);
        intentFilter.addDataScheme("file");
        mContext.registerReceiver(mReceiver, intentFilter);
    }

    /**
     * Unregister intent for filter.
     */
    private void unregisterIntentFilter() {
        mContext.unregisterReceiver(mReceiver);
    }


    private static final String CLASS_NAME = "com.mediatek.storage.StorageManagerEx";
    private static final String METHOD_NAME = "getDefaultPath";
    private static Method sGetDefaultPath;
    private static String sMountPoint;

    static {
        try {
            Class cls = Class.forName(CLASS_NAME);
            if (cls != null) {
                sGetDefaultPath = cls.getDeclaredMethod(METHOD_NAME);
            }
            if (sGetDefaultPath != null) {
                sGetDefaultPath.setAccessible(true);
            }
        } catch (NoSuchMethodException e) {
            LogHelper.e(TAG, "NoSuchMethodException: " + METHOD_NAME);
        } catch (ClassNotFoundException e) {
            LogHelper.e(TAG, "ClassNotFoundException: " + CLASS_NAME);
        }
    }

    boolean isSameStorage(Intent intent) {
        if (isExtendStorageCanUsed()) {
            return StorageManagerExt.isSameStorage(intent, sMountPoint);
        } else {
            return false;
        }
    }

    private boolean isExtendStorageCanUsed() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && isDefaultPathCanUsed();
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
    private boolean isDefaultPathCanUsed() {
        boolean isDefaultPathCanUsed = false;
        if (sGetDefaultPath != null) {
            isDefaultPathCanUsed = true;
        }
        return isDefaultPathCanUsed;
    }
}
