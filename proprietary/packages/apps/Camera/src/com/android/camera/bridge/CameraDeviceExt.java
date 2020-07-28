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
package com.android.camera.bridge;

import java.util.List;

import com.android.camera.CameraManager;
import com.android.camera.ComboPreferences;
import com.android.camera.Log;
import com.android.camera.Util;

import android.app.Activity;
import android.content.SharedPreferences;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.Camera.Area;
import android.hardware.Camera.ErrorCallback;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.location.Location;
import android.media.CameraProfile;
import android.view.SurfaceHolder;

import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingUtils;

import junit.framework.Assert;

public class CameraDeviceExt implements ICameraDeviceExt {
    private final static String TAG = "CameraDeviceExt";

    private final static int UNKNOWN = -1;
    private final static String KEY_IMAGE_REFOCUS_SUPPORTED = "stereo-image-refocus-values";
    private final CameraManager.CameraProxy mCameraDevice;
    private final ComboPreferences mPreferences;
    private final Parameters mInitialParams;
    private final Activity mActivity;
    private final ParametersExt mParametersExt;
    private final int mCameraId;

    private Parameters mParameters;
    private Size mLastPreviewSize;
    private Size mLastPictureSize;
    private String mLastHdrMode;
    private String mLastSceneMode;
    private String mLastDngState = "off";

    private int mDisplayRotation;
    private int mDisplayOrientation;
    private int mCameraDisplayOrientation;
    private int mJpegRotation = UNKNOWN;

    CameraDeviceExt(Activity activity, CameraManager.CameraProxy cameraDevice,
            Parameters parameters, int CameraId, ComboPreferences preferences) {
        Assert.assertNotNull(cameraDevice);
        Assert.assertNotNull(parameters);
        mActivity = activity;
        mCameraDevice = cameraDevice;
        mInitialParams = parameters;
        mParameters = mInitialParams.copy();
        mCameraId = CameraId;
        mPreferences = preferences;
        mParametersExt = new ParametersExt(mCameraDevice, mParameters, CameraId);
    }

    @Override
    public Parameters getInitialParams() {
        return mInitialParams;
    }

    @Override
    public Parameters getParameters() {
        return mParameters;
    }

    @Override
    public ParametersExt getParametersExt() {
        return mParametersExt;
    }

    @Override
    public int getCameraId() {
        return mCameraId;
    }

    @Override
    public CameraManager.CameraProxy getCameraDevice() {
        return mCameraDevice;
    }

    @Override
    public void setPreviewDisplayAsync(SurfaceHolder holder) {
        mCameraDevice.setPreviewDisplayAsync(holder);
    }

    @Override
    public void setPreviewTextureAsync(SurfaceTexture surfaceTexture) {
        mCameraDevice.setPreviewTextureAsync(surfaceTexture);
    }

    public void stopFaceDetection() {
        mCameraDevice.setFaceDetectionListener(null);
        mCameraDevice.stopFaceDetection();
    }

    public void setOneShotPreviewCallback(PreviewCallback cb) {
        mCameraDevice.setOneShotPreviewCallback(cb);
    }

    @Override
    public void setErrorCallback(ErrorCallback cb) {
        mCameraDevice.setErrorCallback(cb);
    }

    @Override
    public void setFaceDetectionListener(FaceDetectionListener listener) {
        mCameraDevice.setFaceDetectionListener(listener);
    }

    @Override
    public void setPhotoModeParameters(final boolean isNonePickerIntent) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                if (isNonePickerIntent) {
                    mParametersExt.setCameraMode(
                            com.mediatek.camera.platform.Parameters.CAMERA_MODE_MTK_PRV);
                } else {
                    mParametersExt.setCameraMode(
                            com.mediatek.camera.platform.Parameters.CAMERA_MODE_NORMAL);
                }
                mParametersExt.setCaptureMode(
                        com.mediatek.camera.platform.Parameters.CAPTURE_MODE_NORMAL);
                mParametersExt.setBurstShotNum(1);
                mParametersExt.setRecordingHint(false);
                int jpegQuality = CameraProfile.getJpegEncodingQualityParameter(mCameraId,
                        CameraProfile.QUALITY_HIGH);
                mParametersExt.setJpegQuality(jpegQuality);
                mParametersExt.enableRecordingSound(String.valueOf(0));
                String supported = mParametersExt.get("mtk-heartbeat-monitor-supported");
                if (supported != null && Boolean.valueOf(supported)) {
                    mParametersExt.set("mtk-heartbeat-monitor", "true");
                }
            }
        });
    }

    // TODO
    @Override
    public void setPreviewSize() {
        String pictureRatio = mPreferences.getString(SettingConstants.KEY_PICTURE_RATIO, null);
        if (pictureRatio == null) {
            List<String> supportedRatios = SettingUtils.buildPreviewRatios(mActivity,
                    mParametersExt);
            if (supportedRatios != null && supportedRatios.size() > 0) {
                SharedPreferences.Editor editor = mPreferences.edit();
                String ratioString = supportedRatios.get(supportedRatios.size() - 1);
      editor.putString(SettingConstants.KEY_PICTURE_RATIO, ratioString);
                editor.apply();
                pictureRatio = ratioString;
            }
        }
        SettingUtils.setPreviewSize(mActivity, mParametersExt, pictureRatio);

        String pictureSize = mPreferences.getString(SettingConstants.KEY_PICTURE_SIZE, null);
        int limitedResolution = SettingUtils.getLimitResolution();
        if (limitedResolution > 0) {
            int index = pictureSize.indexOf('x');
            int width = Integer.parseInt(pictureSize.substring(0, index));
            int height = Integer.parseInt(pictureSize.substring(index + 1));
            if (width * height > limitedResolution) {
                pictureSize = null;
            }
        }
        if (pictureSize == null) {
            List<String> supportedSizes = SettingUtils
                    .buildSupportedPictureSizeByRatio(mParametersExt, pictureRatio);
            SettingUtils.sortSizesInAscending(supportedSizes);
            if (limitedResolution > 0) {
                SettingUtils.filterLimitResolution(supportedSizes);
            }

            if (supportedSizes != null && supportedSizes.size() > 0) {
                pictureSize = supportedSizes.get(supportedSizes.size() - 1);
                SharedPreferences.Editor editor = mPreferences.edit();
                editor.putString(SettingConstants.KEY_PICTURE_SIZE, pictureSize);
                editor.apply();

            }
        }
        Point ps = SettingUtils.getSize(pictureSize);
        mParametersExt.setPictureSize(ps.x, ps.y);
    }

    @Override
    public boolean isSceneModeChanged() {
        boolean isChanged = false;
        String curScene = mParametersExt.get("scene-mode");
        isChanged = curScene == null ? mLastSceneMode != null : !curScene.equals(mLastSceneMode);
        Log.d(TAG, "[isSceneModeChanged] curScene:" + curScene + ",mLastSceneMode:"
                + mLastSceneMode);
        return isChanged;
    }

    @Override
    public boolean isHdrChanged() {
        boolean isChanged = false;
        String curHdr = mParametersExt.get("video-hdr");
        isChanged = curHdr == null ? mLastHdrMode != null : !curHdr.equals(mLastHdrMode);
        Log.d(TAG, "[ishdrChanged] hdr:" + curHdr + ",oldHdr:" + mLastHdrMode);
        return isChanged;
    }

    @Override
    public boolean isPictureSizeChanged() {
        boolean isChanged = false;
        Size curPictureSize = mParametersExt.getPictureSize();
        if (!curPictureSize.equals(mLastPictureSize)) {
            isChanged = true;
        }
        Log.d(TAG, "[isPictureSizeChanged] size : " + curPictureSize + ", oldsize : "
                + mLastPictureSize);
        return isChanged;
    }

    @Override
    public void updateParameters() {
        mLastHdrMode = mParametersExt.get("video-hdr");
        mLastPictureSize = mParametersExt.getPictureSize();
        Log.d(TAG, "[mLastZsdMode]mLastPictureSize" + mLastPictureSize);
    }

    @Override
    public void applyParametersToServer() {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mCameraDevice.setParameters(mParameters);
            }
        });
        Log.d(TAG, "[applyParametersToServer]");
    }

    @Override
    public Size getPreviewSize() {
        return mParametersExt.getPreviewSize();
    }

    public String getZsdMode() {
        return mParametersExt.getZSDMode();
    }

    @Override
    public List<Size> getSupportedPreviewSizes() {
        return mParametersExt.getSupportedPreviewSizes();
    }

    @Override
    public void setFocusMode(String value) {
        mParametersExt.setFocusMode(value);
    }

    @Override
    public boolean isSupportFocusMode(String value) {
        return isSupported(value, mParametersExt.getSupportedFocusModes());
    }

    @Override
    public void setRefocusMode(boolean isOpen) {
        mParametersExt.setRefocusMode(isOpen);
    }

    @Override
    public List<String> getSupportedFocusModes() {
        return mParametersExt.getSupportedFocusModes();
    }

    @Override
    public void fetchParametersFromServer() {
        lockRun(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "fetchParameterFromServer() mParameters=" + mParameters.flatten()
                        + ", mCameraDevice=" + mCameraDevice);
                mParameters = mCameraDevice.getParameters();
                mParametersExt.setparameters(mParameters);
            }
        });
    }

    @Override
    public void setPreviewFormat(int format) {
        mParametersExt.setPreviewFormat(format);
    }

    @Override
    public void setDisplayOrientation(boolean isUseDisplayOrientation) {
        mDisplayRotation = Util.getDisplayRotation(mActivity);
        mDisplayOrientation = Util.getDisplayOrientation(mDisplayRotation, mCameraId);
        mCameraDisplayOrientation = Util.getDisplayOrientation(0, mCameraId);
        // TODO when do dynamic switch SurfaceTexture <--> SurfaceView, this
        // will be change
        mCameraDevice.setDisplayOrientation(isUseDisplayOrientation ? mDisplayOrientation
                : mCameraDisplayOrientation);
    }

    @Override
    public int getDisplayOrientation() {
        return mDisplayOrientation;
    }

    @Override
    public int getCameraDisplayOrientation() {
        return mCameraDisplayOrientation;
    }

    @Override
    public void setJpegRotation(int orientation) {
        mJpegRotation = UNKNOWN;
        mJpegRotation = Util.getJpegRotation(mCameraId, orientation);
        mParametersExt.setRotation(mJpegRotation);
        Log.d(TAG, "setRotationToParameters() mCameraId=" + mCameraId + ", mOrientation="
                + orientation + ", jpegRotation = " + mJpegRotation);
    }

    @Override
    public int getJpegRotation() {
        return mJpegRotation;
    }

    @Override
    public void setGpsParameters(final Location loc) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                Util.setGpsParameters(mParameters, loc);
            }
        });
    }

    /**
     * update dng state to on or off.
     * @param state "on" or "off."
     */
    public void updateDngState(String state) {
        mLastDngState = state;
    }

    public String getDngState() {
        return mLastDngState;
    }

    @Override
    public void setAutoExposureLock(boolean toggle) {
        mParametersExt.setAutoExposureLock(toggle);
    }

    @Override
    public void setAutoWhiteBalanceLock(boolean toggle) {
        mParametersExt.setAutoWhiteBalanceLock(toggle);
    }

    @Override
    public void setFocusAreas(List<Area> focusAreas) {
        mParametersExt.setFocusAreas(focusAreas);
    }

    @Override
    public void setMeteringAreas(List<Area> meteringAreas) {
        mParametersExt.setMeteringAreas(meteringAreas);
    }

    @Override
    public void setCapturePath(String value) {
        mParametersExt.setCapturePath(value);
    }

    @Override
    public boolean isZoomSupported() {
        return mParametersExt.isZoomSupported();
    }

    @Override
    public int getZoom() {
        return mParametersExt.getZoom();
    }

    @Override
    public void setZoom(final int value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setZoom(value);
                mCameraDevice.setParametersAsync(mParameters, value);
            }
        });
    }

    @Override
    public void setPhotoModeBasicParameters() {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParametersExt.setCameraMode(
                        com.mediatek.camera.platform.Parameters.CAMERA_MODE_MTK_PRV);
                mParametersExt.setRecordingHint(false);
                mParametersExt.enableRecordingSound(String.valueOf(0));
            }
        });
    }

    private boolean isImageRefocusSupported() {
        String str = mParametersExt.get(KEY_IMAGE_REFOCUS_SUPPORTED);
        if ("off".equals(str) || null == str) {
            return false;
        } else {
            return true;
        }
    }

    private boolean isSupported(Object value, List<?> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    private void lockRun(Runnable runnable) {
        Log.d(TAG, "lockRun(" + runnable + ") mCameraDevice=" + mCameraDevice);
        if (mCameraDevice != null) {
            mCameraDevice.lockParametersRun(runnable);
        }
    }
}
