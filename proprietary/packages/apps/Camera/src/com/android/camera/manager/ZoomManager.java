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

import java.util.List;
import java.util.Locale;

import com.android.camera.CameraActivity;
import com.android.camera.FeatureSwitcher;
import com.android.camera.GestureDispatcher;
import com.android.camera.Log;

import android.view.MotionEvent;
import android.view.View;

public class ZoomManager extends ViewManager implements
        CameraActivity.Resumable, GestureDispatcher.GestureDispatcherListener {
    private static final String TAG = "ZoomManager";

    private static final String PATTERN = "%.1f";

    private static final int UNKNOWN = -1;
    private static final int RATIO_FACTOR_RATE = 100;
    private static float mIgoreDistance;
    private boolean mDeviceSupport;
    private List<Integer> mZoomRatios;
    private int mLastZoomRatio = UNKNOWN;
    // for scale gesture
    private static final float ZERO = 1;
    private float mZoomIndexFactor = ZERO; // zoom rate from 1, mapping zoom
                                           // gesture rate

    private boolean isHDRecord = false;
    private CameraActivity mCameraActivity;

    private static final boolean[] MATRIX_ZOOM_ENABLE = new boolean[] {
        true, // photo
        true, // hdr
        true, // facebeauty
        true, // panorama
        true, // asd
        true, // photo pip
        true, // video
        true, // video pip
        true, // normal3d
        true, // panorama3d
        true,
    };

    public ZoomManager(CameraActivity context) {
        super(context);
        context.addResumable(this);
        mCameraActivity = (CameraActivity) context;
    }

    @Override
    protected View getView() {
        return null;
    }

    @Override
    public void begin() {
    }

    @Override
    public void resume() {
        mCameraActivity.setGestureDispatcherListener(this);
    }

    @Override
    public void pause() {
        mCameraActivity.setGestureDispatcherListener(null);
    }

    @Override
    public void finish() {
    }

    @Override
    public boolean onDown(float x, float y, int width, int height) {
        return false;
    }
    @Override
    public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
            float velocityY) {
        return false;
    }

        @Override
    public boolean onScroll(float dx, float dy, float totalX, float totalY) {
        return false;
    }

    @Override
    public boolean onSingleTapUp(float x, float y) {
        return false;
    }
    @Override
    public boolean onSingleTapConfirmed(float x, float y) {
        return false;
    }
    @Override
    public boolean onUp() {
        return false;
    }

        @Override
        public boolean onDoubleTap(float x, float y) {
            if (!FeatureSwitcher.isSupportDoubleTapUp())
                return false;
            Log.d(TAG, "onDoubleTap(" + x + ", " + y + ") mZoomIndexFactor=" + mZoomIndexFactor
                    + ", isAppSupported()=" + isAppSupported() + ", isEnabled()=" + isEnabled());
            if (!isAppSupported() || !isEnabled()) {
                return false;
            }
            int oldIndex = findZoomIndex(mLastZoomRatio);
            int zoomIndex = 0;
            if (oldIndex == 0) {
                zoomIndex = getMaxZoomIndex();
                mZoomIndexFactor = getMaxZoomIndexFactor();
            } else {
                mZoomIndexFactor = ZERO;
            }
            performZoom(zoomIndex, true);
            return true;
        }

        @Override
        public boolean onScale(float focusX, float focusY, float scale) {
            if (!isAppSupported() || !isEnabled()) {
                return false;
            }
            if (Float.isNaN(scale) || Float.isInfinite(scale)) {
                return false;
            }
            mZoomIndexFactor *= scale;
            if (mZoomIndexFactor <= ZERO) {
                mZoomIndexFactor = ZERO;
            } else if (mZoomIndexFactor >= getMaxZoomIndexFactor()) {
                mZoomIndexFactor = getMaxZoomIndexFactor();
            }
            int zoomIndex = findZoomIndex(Math.round(mZoomIndexFactor * RATIO_FACTOR_RATE));
            performZoom(zoomIndex, true);
            Log.d(TAG, "onScale() mZoomIndexFactor=" + mZoomIndexFactor);
            return true;
        }

        @Override
        public boolean onScaleBegin(float focusX, float focusY) {
        Log.d(TAG, "onScaleBegin(" + focusX + ", " + focusY + ")");
            return true;
        }

    @Override
    public boolean onLongPress(float x, float y) {
        return false;
    }

    public void resetZoom() {
        Log.d(TAG, "resetZoom() mZoomRatios=" + mZoomRatios
                + ", mLastZoomRatio=" + mLastZoomRatio);
        mZoomIndexFactor = ZERO;
        if (isValidZoomIndex(0)) {
            mLastZoomRatio = mZoomRatios.get(0);
        }
    }
    private void performZoom(int zoomIndex, boolean userAction) {
        Log.d(TAG, "performZoom(" + zoomIndex + ", " + userAction
                + ", mDeviceSupport="
                + mDeviceSupport);
        if (getContext().getCameraDevice() != null && mDeviceSupport
                && isValidZoomIndex(zoomIndex)) {
            getContext().startAsyncZoom(zoomIndex);
            int newRatio = mZoomRatios.get(zoomIndex);
            if (mLastZoomRatio != newRatio) {
                mLastZoomRatio = newRatio; // change last zoom value to new
            }
        }
        if (userAction) {
            float zoomRation = ((float) mLastZoomRatio) / RATIO_FACTOR_RATE;
            // here use English to format, since just display numbers.
            getContext().getCameraAppUI().showInfo(
                    "x" + String.format(Locale.ENGLISH, PATTERN, zoomRation));
        }
    }

    public void setZoomParameter() {
        if (isAppSupported()) {
            if (getContext().getParameters() == null) {
                Log.d(TAG, "setZoomParameter() getContext().getParameters() is null!");
                return;
            }
            mDeviceSupport = getContext().getParameters().isZoomSupported();
            mZoomRatios = getContext().getParameters().getZoomRatios();
            int index = getContext().getParameters().getZoom();
            if (mZoomRatios != null) {
                int len = mZoomRatios.size();
                int curRatio = mZoomRatios.get(index);
                int maxRatio = mZoomRatios.get(len - 1);
                int minRatio = mZoomRatios.get(0);
                int finalIndex = index;
                if (mLastZoomRatio == UNKNOWN || mLastZoomRatio == curRatio) {
                    mLastZoomRatio = curRatio;
                    finalIndex = index;
                } else {
                    finalIndex = findZoomIndex(mLastZoomRatio);
                }
                int newRatio = mZoomRatios.get(finalIndex);
                performZoom(finalIndex, newRatio != mLastZoomRatio);
                Log.d(TAG, "onCameraParameterReady() index = " + index + ", len = " + len
                        + ", maxRatio = " + maxRatio + ", minRatio = " + minRatio + ", curRatio = "
                        + curRatio + ", finalIndex = " + finalIndex + ", newRatio = " + newRatio
                        + ", mSupportZoom = " + mDeviceSupport + ", mLastZoomRatio = "
                        + mLastZoomRatio);
            }
        } else { // reset zoom if App limit zoom function.
            resetZoom();
            performZoom(0, false);
        }
    }

    public void checkQualityForZoom() {
        isHDRecord = true;
    }

    public void changeZoomForQuality() {
        isHDRecord = false;
    }

    private boolean isAppSupported() {
        boolean enable = isZoomEnable(getContext().getCurrentMode());
        if (isHDRecord) {
            enable = false;
        }
        Log.d(TAG, "isAppSupported() return " + enable);
        return enable;
    }

    private boolean isZoomEnable(int mode) {
        if (mode > -1 && mode < MATRIX_ZOOM_ENABLE.length) {
            return MATRIX_ZOOM_ENABLE[mode];
        } else {
            throw new RuntimeException("Get zoom enable out of index");
        }
    }

    private int findZoomIndex(int zoomRatio) {
        int find = 0; // if not find, return 0
        if (mZoomRatios != null) {
            int len = mZoomRatios.size();
            if (len == 1) {
                find = 0;
            } else {
                int max = mZoomRatios.get(len - 1);
                int min = mZoomRatios.get(0);
                if (zoomRatio <= min) {
                    find = 0;
                } else if (zoomRatio >= max) {
                    find = len - 1;
                } else {
                    for (int i = 0; i < len - 1; i++) {
                        int cur = mZoomRatios.get(i);
                        int next = mZoomRatios.get(i + 1);
                        if (zoomRatio >= cur && zoomRatio < next) {
                            find = i;
                            break;
                        }
                    }
                }
            }
        }
        return find;
    }

    private boolean isValidZoomIndex(int zoomIndex) {
        boolean valid = false;
        if (mZoomRatios != null && zoomIndex >= 0
                && zoomIndex < mZoomRatios.size()) {
            valid = true;
        }
        Log.d(TAG, "isValidZoomIndex(" + zoomIndex + ") return " + valid);
        return valid;
    }

    private int getMaxZoomIndex() {
        int index = UNKNOWN;
        if (mZoomRatios != null) {
            index = mZoomRatios.size() - 1;
        }
        return index;
    }

    private float getMaxZoomIndexFactor() {
        return (float) getMaxZoomRatio() / RATIO_FACTOR_RATE;
    }

    private int getMaxZoomRatio() {
        int ratio = UNKNOWN;
        if (mZoomRatios != null) {
            ratio = mZoomRatios.get(mZoomRatios.size() - 1);
        }
        return ratio;
    }

}
