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

import android.hardware.Camera.Area;
import android.hardware.Camera.Size;
import android.text.TextUtils;

import com.android.camera.CameraManager;
import com.android.camera.Log;

import com.mediatek.camera.platform.Parameters;

import java.util.ArrayList;
import java.util.List;

public class ParametersExt implements Parameters {

    private static final  String TAG = ParametersExt.class.getSimpleName();
    private static final String CONTINUOUS_SHOT_MODE = "continuousshot";
    private final CameraManager.CameraProxy mCameraDevice;
    private android.hardware.Camera.Parameters mParameters;
    private int mCameraId;

    public ParametersExt(CameraManager.CameraProxy cameraDevice,
            android.hardware.Camera.Parameters parameters,
            int cameraId) {
        mCameraDevice = cameraDevice;
        mParameters = parameters;
        mCameraId = cameraId;
    }

    public void setparameters(android.hardware.Camera.Parameters parameters) {
        mParameters = parameters;
    }

    @Override
    public void set(final String key, final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(key, value);
            }
        });
    }

    @Override
    public void set(final String key, final int value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(key, value);
            }
        });
    }

    @Override
    public String get(String key) {
        String value = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.get(key);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public int getInt(String key) {
        int value = 0;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.getInt(key);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public void setPreviewSize(final int width, final int height) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setPreviewSize(width, height);
            }
        });
    }

    @Override
    public Size getPreviewSize() {
        Size size = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                size = mParameters.getPreviewSize();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        Log.d(TAG, "getPreviewSize, size:" + size);
        return size;
    }

    @Override
    public List<Size> getSupportedPreviewSizes() {
        return mParameters.getSupportedPreviewSizes();
    }

    @Override
    public List<Size> getSupportedVideoSizes() {
        return mParameters.getSupportedVideoSizes();
    }

    @Override
    public Size getPreferredPreviewSizeForVideo() {
        return mParameters.getPreferredPreviewSizeForVideo();
    }

    @Override
    public void setJpegQuality(final int quality) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setJpegQuality(quality);
            }
        });
    }

    @Override
    public int getJpegQuality() {
        int value = 0;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.getJpegQuality();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public void setPreviewFrameRate(final int fps) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setPreviewFrameRate(fps);
            }
        });
    }

    @Override
    public List<Integer> getSupportedPreviewFrameRates() {
        return mParameters.getSupportedPreviewFrameRates();
    }

    @Override
    public void setPreviewFormat(final int pixelFormat) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setPreviewFormat(pixelFormat);
            }
        });
    }

    @Override
    public int getPreviewFormat() {
        int format = 0;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                format = mParameters.getPreviewFormat();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return format;
    }

    @Override
    public void setPictureSize(final int width, final int height) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setPictureSize(width, height);
            }
        });
    }

    @Override
    public Size getPictureSize() {
        Size size = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                size = mParameters.getPictureSize();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        Log.d(TAG, "getPictureSize, size:" + size);
        return size;
    }

    @Override
    public List<Size> getSupportedPictureSizes() {
        return mParameters.getSupportedPictureSizes();
    }

    @Override
    public void setRotation(final int rotation) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setRotation(rotation);
            }
        });
    }

    @Override
    public String getWhiteBalance() {
        String value = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.getWhiteBalance();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public void setWhiteBalance(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setWhiteBalance(value);
            }
        });
    }

    @Override
    public List<String> getSupportedWhiteBalance() {
        return mParameters.getSupportedWhiteBalance();
    }

    @Override
    public String getColorEffect() {
        String value = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.getColorEffect();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public void setColorEffect(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setColorEffect(value);
            }
        });
    }

    @Override
    public List<String> getSupportedColorEffects() {
        return mParameters.getSupportedColorEffects();
    }

    @Override
    public String getAntibanding() {
        String value = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.getAntibanding();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public void setAntibanding(final String antibanding) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setAntibanding(antibanding);
            }
        });
    }

    @Override
    public List<String> getSupportedAntibanding() {
        return mParameters.getSupportedAntibanding();
    }

    @Override
    public String getSceneMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.getSceneMode();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setSceneMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setSceneMode(value);
            }
        });
    }

    @Override
    public List<String> getSupportedSceneModes() {
        return mParameters.getSupportedSceneModes();
    }

    @Override
    public String getFlashMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.getFlashMode();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setFlashMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setFlashMode(value);
            }
        });
    }

    @Override
    public List<String> getSupportedFlashModes() {
        return mParameters.getSupportedFlashModes();
    }

    @Override
    public String getFocusMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.getFocusMode();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setFocusMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setFocusMode(value);
            }
        });
    }

    @Override
    public List<String> getSupportedFocusModes() {
        return mParameters.getSupportedFocusModes();
    }

    @Override
    public int getExposureCompensation() {
        int value = 0;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.getExposureCompensation();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public void setExposureCompensation(final int value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setExposureCompensation(value);
            }
        });
    }

    @Override
    public int getMaxExposureCompensation() {
        return mParameters.getMaxExposureCompensation();
    }

    @Override
    public int getMinExposureCompensation() {
        return mParameters.getMinExposureCompensation();
    }

    @Override
    public float getExposureCompensationStep() {
        return mParameters.getExposureCompensationStep();
    }

    @Override
    public void setAutoExposureLock(final boolean toggle) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setAutoExposureLock(toggle);
            }
        });
    }

    @Override
    public void setAutoWhiteBalanceLock(final boolean toggle) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setAutoWhiteBalanceLock(toggle);
            }
        });
    }

    @Override
    public int getZoom() {
        int value = 0;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.getZoom();
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return value;
    }

    @Override
    public boolean isZoomSupported() {
        return mParameters.isZoomSupported();
    }

    @Override
    public void setCameraMode(final int value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_CAMERA_MODE, value);;
            }
        });
    }

    @Override
    public String getISOSpeed() {
        String value = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                value = mParameters.get(KEY_ISOSPEED_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }

        return value;
    }

    @Override
    public void setISOSpeed(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_ISOSPEED_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedISOSpeed() {
        String str = get(KEY_ISOSPEED_MODE + SUPPORTED_VALUES_SUFFIX);
        return split(str);
    }

    @Override
    public int getMaxNumDetectedObjects() {
        try {
            return mParameters.getInt(Parameters.KEY_MAX_NUM_DETECTED_OBJECT);
        } catch (NumberFormatException ex) {
            return 0;
        }
    }

    @Override
    public String getEdgeMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_EDGE_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setEdgeMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_EDGE_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedEdgeMode() {
        String str = get(KEY_EDGE_MODE + SUPPORTED_VALUES_SUFFIX);
        return split(str);
    }

    @Override
    public String getHueMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_HUE_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setHueMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_HUE_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedHueMode() {
        String str = get(KEY_HUE_MODE + SUPPORTED_VALUES_SUFFIX);
        return split(str);
    }

    @Override
    public String getSaturationMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_SATURATION_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setSaturationMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_SATURATION_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedSaturationMode() {
        String str = get(KEY_SATURATION_MODE + SUPPORTED_VALUES_SUFFIX);
        return split(str);
    }

    @Override
    public String getBrightnessMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_BRIGHTNESS_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setBrightnessMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_BRIGHTNESS_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedBrightnessMode() {
        String str = get(KEY_BRIGHTNESS_MODE + SUPPORTED_VALUES_SUFFIX);
        return split(str);
    }

    @Override
    public String getContrastMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_CONTRAST_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setContrastMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_CONTRAST_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedContrastMode() {
        String str = get(KEY_CONTRAST_MODE + SUPPORTED_VALUES_SUFFIX);
        return split(str);
    }

    @Override
    public String getCaptureMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParameters() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_CAPTURE_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setCaptureMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_CAPTURE_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedCaptureMode() {
        String str = get(KEY_CAPTURE_MODE + SUPPORTED_VALUES_SUFFIX);
        List<String> captureModes = split(str);
        if (mCameraId > 0
                && captureModes.indexOf(CONTINUOUS_SHOT_MODE) >= 0) {
            captureModes.remove(CONTINUOUS_SHOT_MODE);
        }
        return captureModes;
    }

    @Override
    public void setCapturePath(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                if (value == null) {
                    mParameters.remove(KEY_CAPTURE_PATH);
                } else {
                    mParameters.set(KEY_CAPTURE_PATH, value);
                }
            }
        });
    }

    @Override
    public void setBurstShotNum(final int value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_BURST_SHOT_NUM, value);
            }
        });
    }

    @Override
    public String getZSDMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParametersRun() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_ZSD_MODE);;
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setZSDMode(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_ZSD_MODE, value);
            }
        });
    }

    @Override
    public List<String> getSupportedZSDMode() {
        String str = get(KEY_ZSD_MODE + SUPPORTED_VALUES_SUFFIX);
        return split(str);
    }

    @Override
    public void setFocusAreas(final List<Area> focusAreas) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setFocusAreas(focusAreas);
            }
        });
    }

    @Override
    public int getMaxNumFocusAreas() {
        return mParameters.getMaxNumFocusAreas();
    }

    @Override
    public void setMeteringAreas(final List<Area> meteringAreas) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setMeteringAreas(meteringAreas);
            }
        });
    }

    @Override
    public void setRecordingHint(final boolean hint) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setRecordingHint(hint);
            }
        });
    }

    @Override
    public boolean isVideoSnapshotSupported() {
        return mParameters.isVideoSnapshotSupported();
    }

    @Override
    public void enableRecordingSound(final String value) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                if (value.equals("1") || value.equals("0")) {
                    mParameters.set(KEY_MUTE_RECORDING_SOUND, value);
                }
            }
        });
    }

    @Override
    public void setVideoStabilization(final boolean toggle) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.setVideoStabilization(toggle);
            }
        });
    }

    @Override
    public boolean isVideoStabilizationSupported() {
        return mParameters.isVideoStabilizationSupported();
    }

    @Override
    public List<Integer> getPIPFrameRateZSDOn() {
        String str = get(KEY_MAX_FRAME_RATE_ZSD_ON);
        return splitInt(str);
    }

    @Override
    public List<Integer> getPIPFrameRateZSDOff() {
        String str = get(KEY_MAX_FRAME_RATE_ZSD_OFF);
        return splitInt(str);
    }

    @Override
    public void setDynamicFrameRate(final boolean toggle) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_DYNAMIC_FRAME_RATE, toggle ? TRUE : FALSE);
            }
        });
    }

    @Override
    public boolean isDynamicFrameRateSupported() {
        String str = mParameters.get(KEY_DYNAMIC_FRAME_RATE_SUPPORTED);
        return TRUE.equals(str);
    }

    @Override
    public void setRefocusJpsFileName(final String fineName) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_REFOCUS_JPS_FILE_NAME, fineName);
            }
        });
    }

    @Override
    public String getDepthAFMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParametersRun() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_STEREO_DEPTHAF_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public String getDistanceMode() {
        String mode = null;
        if (mCameraDevice != null) {
            boolean lockedParameters = false;
            try {
                mCameraDevice.lockParameters();
                lockedParameters = true;
            } catch (InterruptedException ex) {
                Log.e(TAG, "lockParametersRun() not successfull.", ex);
            } finally {
                mode = mParameters.get(KEY_STEREO_DISTANCE_MODE);
                if (lockedParameters) {
                    mCameraDevice.unlockParameters();
                }
            }
        }
        return mode;
    }

    @Override
    public void setDepthAFMode(final boolean isDepthAfMode) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_STEREO_DEPTHAF_MODE, isDepthAfMode ? ON : OFF);;
            }
        });
    }

    @Override
    public void setDistanceMode(final boolean isDistanceMode) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_STEREO_DISTANCE_MODE, isDistanceMode ? ON : OFF);
            }
        });
    }

    /**
     * Set refocus mode.
     * @param isOpen True open refocus mode.
     */
    public void setRefocusMode(final boolean isOpen) {
        lockRun(new Runnable() {
            @Override
            public void run() {
                mParameters.set(KEY_STEREO_REFOCUS_MODE, isOpen ? ON : OFF);
            }
        });
    }

    private void lockRun(Runnable runnable) {
        if (mCameraDevice != null) {
            mCameraDevice.lockParametersRun(runnable);
        }
    }

    // Splits a comma delimited string to an ArrayList of String.
    // Return null if the passing string is null or the size is 0.
    private ArrayList<String> split(String str) {
        if (str == null) return null;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<String> substrings = new ArrayList<String>();
        for (String s : splitter) {
            substrings.add(s);
        }
        return substrings;
    }

    // Splits a comma delimited string to an ArrayList of Integer.
    // Return null if the passing string is null or the size is 0.
    private ArrayList<Integer> splitInt(String str) {
        if (str == null) return null;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<Integer> substrings = new ArrayList<Integer>();
        for (String s : splitter) {
            substrings.add(Integer.parseInt(s));
        }
        if (substrings.size() == 0) return null;
        return substrings;
    }
}
