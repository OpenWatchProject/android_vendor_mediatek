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

import android.hardware.Camera.Size;
import android.media.CamcorderProfile;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.widget.TextView;

import com.android.camera.CameraActivity;
import com.android.camera.Log;
import com.android.camera.R;
import com.android.camera.Storage;
import com.android.camera.Util;

import java.util.Locale;

public class RemainingManager extends ViewManager implements
               CameraActivity.Resumable, CameraActivity.OnParametersReadyListener {
    private static final String TAG = "RemainingManager";

    private static final int TYPE_COUNT = 0;
    private static final int TYPE_TIME = 1;
    private static final Long REMAIND_THRESHOLD = 100L;

    private static final int[] MATRIX_REMAINING_TYPE = new int[ModePicker.MODE_NUM_ALL];
    static {
        MATRIX_REMAINING_TYPE[ModePicker.MODE_PHOTO] = TYPE_COUNT;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_HDR] = TYPE_COUNT;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_FACE_BEAUTY] = TYPE_COUNT;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_PANORAMA] = TYPE_COUNT;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_ASD] = TYPE_COUNT;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_PHOTO_PIP] = TYPE_COUNT;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_VIDEO] = TYPE_TIME;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_VIDEO_PIP] = TYPE_TIME;
        MATRIX_REMAINING_TYPE[ModePicker.MODE_VIDEO_STEREO] = TYPE_TIME;
    }

    private TextView mRemainingView;
    private OnScreenHint mStorageHint;
    private WorkerHandler mWorkerHandler;
    private int mType = TYPE_COUNT;
    private String mRemainingText;
    private boolean mResumed;
    private boolean mParametersReady;
    private CamcorderProfile mProfile;
    private String mDngState = "off";
    private static final int DNG_IMAGE_SIZE = 30000000;
    private static final int MSG_UPDATE_STORAGE = 0;
    private Long mAvaliableSpace;

    private static final int DELAY_MSG_MS = 1500;
    private static final int DEFAULT_SOLUTION = 2048 * 1360;

    private class WorkerHandler extends Handler {
        public WorkerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_UPDATE_STORAGE:
                mAvaliableSpace = Storage.getAvailableSpace();
                removeMessages(MSG_UPDATE_STORAGE);
                sendEmptyMessageDelayed(MSG_UPDATE_STORAGE, DELAY_MSG_MS);
                break;
            default:
                break;
            }
        }
    }

    private Handler mMainHandler = new Handler();
    private CameraActivity mContext;

    public RemainingManager(CameraActivity context) {
        super(context);
        mContext = context;
        context.addResumable(this);
        context.addOnParametersReadyListener(this);
        // disable animation for cross with remaining.
        // disable fade in, in order to not affect setVisibility(View.Gone)
        setAnimationEnabled(false, false);
    }

    @Override
    public void begin() {
        if (mWorkerHandler == null) {
            HandlerThread t = new HandlerThread("thumbnail-creation-thread");
            t.start();
            mWorkerHandler = new WorkerHandler(t.getLooper());
            mWorkerHandler.sendEmptyMessage(MSG_UPDATE_STORAGE);
        }
    }

    @Override
    public void resume() {
        mResumed = true;
        mAvaliableSpace = Storage.getAvailableSpace();
        showHint();
    }

    @Override
    public void pause() {
        mResumed = false;
        if (mStorageHint != null) {
            mStorageHint.cancel();
            mStorageHint = null;
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
        View view = inflate(R.layout.remaining);
        mRemainingView = (TextView) view.findViewById(R.id.remaining_view);
        return view;
    }

    @Override
    protected void onRefresh() {
        super.onRefresh();
        if (mRemainingView != null) {
            mRemainingView.setText(mRemainingText);
        }
    }

    public boolean showIfNeed() {
        if (mParametersReady) {
            if (mAvaliableSpace == null || mAvaliableSpace.longValue() <= 0) {
                mAvaliableSpace = Storage.getAvailableSpace();
            }
            long leftSpace = mAvaliableSpace.longValue();
            mType = MATRIX_REMAINING_TYPE[getContext().getCurrentMode() % ModePicker.OFFSET];
            leftSpace = computeStorage(leftSpace);
            updateStorageHint(leftSpace);
            updateRemainingView(REMAIND_THRESHOLD, leftSpace);
        }
        return isShowing();
    }

    public void show() {
        if (!isShowing()) {
            showAways();
        }
    }

    public boolean showAways() {
        if (mParametersReady) {
            mAvaliableSpace = Storage.getAvailableSpace();
            long leftSpace = mAvaliableSpace.longValue();
            mType = MATRIX_REMAINING_TYPE[getContext().getCurrentMode() % ModePicker.OFFSET];
            leftSpace = computeStorage(leftSpace);
            updateStorageHint(leftSpace);
            updateRemainingView(Long.MAX_VALUE, leftSpace);
        }
        return isShowing();
    }

    public void clearAvaliableSpace() {
        mAvaliableSpace = null;
    }

    public void showHint() {
        boolean isCameraOpened = mContext.isCameraOpened();
        if (mParametersReady && isCameraOpened) {
            mMainHandler.post(new Runnable() { // delay for after resume
                        @Override
                        public void run() {
                            if (mAvaliableSpace == null || mAvaliableSpace.longValue() <= 0) {
                                mAvaliableSpace = Storage.getAvailableSpace();
                            }
                            long leftSpace = mAvaliableSpace.longValue();
                            leftSpace = computeStorage(leftSpace);
                            updateStorageHint(leftSpace);
                        }
                    });
        }
    }

    public long updateStorage() {
        if (mAvaliableSpace == null || mAvaliableSpace.longValue() <= 0) {
            mAvaliableSpace = Storage.getAvailableSpace();
        }
        long leftSpace = mAvaliableSpace.longValue();
        return computeStorage(leftSpace);
    }

    public long getLeftStorage() {
        return Storage.getAvailableSpace() - Storage.LOW_STORAGE_THRESHOLD;
    }

    private long computeStorage(long avaliableSpace) {
        long pictureSize;
        if ("on".equals(mDngState)) {
            //normal mode and dng on, need add the dng size
            pictureSize = pictureSize() + DNG_IMAGE_SIZE;
        } else {
            pictureSize = pictureSize();
        }
        if (avaliableSpace > Storage.LOW_STORAGE_THRESHOLD) {
            avaliableSpace = (avaliableSpace - Storage.LOW_STORAGE_THRESHOLD)
                    / (mType == TYPE_COUNT ? pictureSize : videoFrameRate());
        } else if (avaliableSpace > 0) {
            avaliableSpace = 0;
        }
        Storage.setLeftSpace(avaliableSpace);
        return avaliableSpace;
    }

    private void updateRemainingView(long threshold, long leftSpace) {
        boolean needShow = (mType == TYPE_COUNT) ? (leftSpace <= threshold) : true;
        if (needShow) { // only show remaining view in some cases
            if (leftSpace < 0) {
                mRemainingText = mType == TYPE_COUNT ? stringForCount(0) : stringForTime(0);
            } else {
                mRemainingText = mType == TYPE_COUNT ? stringForCount(leftSpace)
                        : stringForTime(leftSpace);
            }
            super.show(); // show remaining view
        }
    }

    private void updateStorageHint(long leftSpace) {
        Log.d(TAG, "updateStorageHint(" + leftSpace + ") isFullScreen="
                + getContext().isFullScreen());
        String message = null;
        if (leftSpace == Storage.UNAVAILABLE
                || leftSpace == Storage.PREPARING
                || leftSpace == Storage.UNKNOWN_SIZE) {
            message = getContext().getString(R.string.can_not_use_storage);
        } else if (leftSpace <= 0) {
            message = getContext().getString(R.string.storage_full);
        }
        if (message != null && getContext().isFullScreen()) {
            if (mStorageHint == null) {
                mStorageHint = OnScreenHint.makeText(getContext(), message);
            } else {
                mStorageHint.setText(message);
            }
            mStorageHint.show();
        } else if (mStorageHint != null) {
            mStorageHint.cancel();
            mStorageHint = null;
        }
    }

    public long pictureSize() {
        int pictureFormat = Storage.FORMAT_JPEG;
        int resolution = 0;
        switch (getContext().getCameraActor().getMode()) {
        case ModePicker.MODE_PANORAMA:
            pictureFormat = Storage.FORMAT_AUTORAMA;
            break;
        default:
            if (getContext().getParameters() != null) {
                Size size = getContext().getParameters().getPictureSize();
                if (size != null) {
                    resolution = size.width * size.height;
                } else {
                    resolution = DEFAULT_SOLUTION;
                }
            } else {
                resolution = DEFAULT_SOLUTION;
            }
            break;
        }
        long psize = Storage.getBytePerImage(pictureFormat, resolution);
        return psize;
    }

    public void setCamcorderProfile(CamcorderProfile profile) {
        mProfile = profile;
    }

    public void setDngState(String state) {
        mDngState = state;
    }

    public long videoFrameRate() {
            long bytePerMs = ((mProfile.videoBitRate + mProfile.audioBitRate) >> 3) / 1000;
            return bytePerMs;
    }

    private static String stringForTime(final long millis) {
        final long totalSeconds = millis / 1000;
        final long seconds = totalSeconds % 60;
        final long minutes = (totalSeconds / 60) % 60;
        final long hours = totalSeconds / 3600;
        if (hours > 0) {
            return String.format(Locale.ENGLISH, "%d:%02d:%02d", hours, minutes, seconds);
        } else {
            return String.format(Locale.ENGLISH, "%02d:%02d", minutes, seconds);
        }
    }

    private static String stringForCount(final long count) {
        return String.format("%d", count);
    }
    @Override
    public void onCameraParameterReady() {
        mParametersReady = true;
    }

    private class Holder {
        long mThreshold;
        long mLeftSpace;

        public Holder() {
        }

        public Holder(long threshold, long leftSpace) {
            mThreshold = threshold;
            mLeftSpace = leftSpace;
        }

        @Override
        public String toString() {
            return new StringBuilder().append("Holder(mThreshold=").append(mThreshold)
                    .append(", mLeftSpace=").append(mLeftSpace).append(")").toString();
        }
    }
}
