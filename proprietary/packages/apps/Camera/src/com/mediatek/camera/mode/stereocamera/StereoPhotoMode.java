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

package com.mediatek.camera.mode.stereocamera;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.net.Uri;
import android.os.SystemProperties;
import android.util.Log;

import com.android.camera.R;
import com.android.camera.Util;
//import com.mediatek.accessor.StereoInfoAccessor;
//import com.mediatek.accessor.data.StereoCaptureInfo;
import com.mediatek.camera.ICameraAddition.AdditionActionType;
import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraMode.ActionType;
import com.mediatek.camera.ICameraMode.ModeState;
import com.mediatek.camera.mode.PhotoMode;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoPreviewRatioRule;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoSettingRule;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoZsdRule;
import com.mediatek.camera.platform.ICameraAppUi.ShutterButtonType;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager.ICameraDevice.StereoDataCallback;
import com.mediatek.camera.platform.IFileSaver.FILE_TYPE;
import com.mediatek.camera.platform.IFileSaver.OnFileSavedListener;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;

import java.io.FileOutputStream;
import java.io.IOException;

/**
 * This class used for Stereo Camera capture.
 */
public class StereoPhotoMode extends PhotoMode {
    private static final String TAG = "StereoPhotoMode";
    private static final int PASS_NUM = 0;
    private static final int FAIL_NUM = 2;
    private static final String GEO_QUALITY = "Geometric Quality: ";
    private static final String PHO_QUALITY = "Photo Quality: ";
    private static final String PASS = "Pass";
    private static final String WARNING = "Pass(limited)";
    private static final String FAIL = "Fail";
    private static final int JPEG_ROTATION_0 = 0;
    private static final int JPEG_ROTATION_180 = 180;
    private static final String KEY_ROTATION = "rotation";
    //private StereoInfoAccessor mAccessor;
    private final StereoPhotoDataCallback mStereoPhotoDataCallback = new StereoPhotoDataCallback();
    private int mJpegRotation = 0;
    /**
     * Create a stereo camera mode.
     * @param cameraContext camera context instance.
     */
    public StereoPhotoMode(ICameraContext cameraContext) {
        super(cameraContext);
        Log.i(TAG, "[StereoPhotoMode]constructor...");
        mCameraCategory = new StereoPhotoCategory();
        //mAccessor = new StereoInfoAccessor();
        StereoSettingRule zsdRule = new StereoZsdRule(cameraContext,
                StereoSettingRule.DENOISE);
        zsdRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_PHOTO_STEREO,
                SettingConstants.KEY_CAMERA_ZSD, zsdRule);
        setModeState(ModeState.STATE_CLOSED);
    }

    @Override
    public boolean open() {
        Log.i(TAG, "[openMode] ...");
        super.open();
        mICameraAppUi.setGestureListener(mStereoGestureListener);
        mICameraAppUi.switchShutterType(ShutterButtonType.SHUTTER_TYPE_PHOTO);
        return true;
    }

    @Override
    public boolean close() {
        Log.i(TAG, "[closeMode]...");
        mICameraAppUi.setGestureListener(null);
        super.close();
        return true;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        if (type != ActionType.ACTION_ORITATION_CHANGED) {
            Log.i(TAG, "[execute]type = " + type);
        }
        switch (type) {

        case ACTION_ON_CAMERA_OPEN:
            updateDevice();
            mCameraClosed = false;
            ParametersHelper.setVsDofMode(mICameraDevice.getParameters(), false);
            ParametersHelper.setStereoCaptureMode(mICameraDevice.getParameters(), false);
            ParametersHelper.setDenoiseMode(mICameraDevice.getParameters(), true);
            break;

        case ACTION_FACE_DETECTED:
            // Do-Noting,Because not need show super's entry FB icon
            break;

        default:
            return super.execute(type, arg);
        }
        return true;
    }
    @Override
    public void resume() {

    }

    @Override
    public void pause() {
        Log.i(TAG, "pause");
    }

    @Override
    protected PictureCallback getUncompressedImageCallback() {
        return mUncompressedImageCallback;
    }

    @Override
    public boolean capture() {
        Log.i(TAG, "capture()");
        mIFileSaver.init(FILE_TYPE.JPEG, 0, null, -1);
        mJpegRotation = Integer.parseInt(mICameraDevice.getParameters().get(KEY_ROTATION));
        mICameraAppUi.showRemaining();
        mCaptureStartTime = System.currentTimeMillis();
        mCameraCategory.takePicture();
        setModeState(ModeState.STATE_CAPTURING);
        return true;
    }

    private final PictureCallback mUncompressedImageCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            Log.i(TAG, "[UncompressedImageCallback]onCanCapture");
            restartPreview(false);
        }
    };

    private OnFileSavedListener mFileSaverListener = new OnFileSavedListener() {
        @Override
        public void onFileSaved(Uri uri) {
            Log.d(TAG, "[onFileSaved]uri= " + uri);
        }
    };

    private final PictureCallback mJpegPictureCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] jpegData, Camera camera) {
            Log.d(TAG, "[mJpegPictureCallback]");
            if (mCameraClosed) {
                Log.i(TAG, "[mJpegPictureCallback] mCameraClosed:" + mCameraClosed);
                mICameraAppUi.setSwipeEnabled(true);
                mICameraAppUi.restoreViewState();
                return;
            }
            if (jpegData == null) {
                Log.i(TAG, "[mJpegPictureCallback] jpegData is null");
                mICameraAppUi.setSwipeEnabled(true);
                mICameraAppUi.restoreViewState();
                restartPreview(false);
                return;
            }
            mIFocusManager.updateFocusUI(); // Ensure focus indicator
            mIFileSaver.savePhotoFile(jpegData, null, mCaptureStartTime,
                    mIModuleCtrl.getLocation(), 0, null);
            Log.d(TAG, "[mJpegPictureCallback] end");
        }
    };

    private final ShutterCallback mShutterCallback = new ShutterCallback() {
        @Override
        public void onShutter() {
            long shutterCallbackTime = System.currentTimeMillis();
            long shutterLag = shutterCallbackTime - mCaptureStartTime;
            Log.d(TAG, "[mShutterCallback] mShutterLag = " + shutterLag + "ms");
        }
    };

    private final PictureCallback mPostViewCallback = new PictureCallback() {
        @Override
        public void onPictureTaken(byte[] postViewData, Camera camera) {
            Log.d(TAG, "[mPostViewCallback]");
            if (mCameraClosed) {
                Log.i(TAG, "[mPostViewCallback] mCameraClosed:" + mCameraClosed);
                return;
            }
            if (postViewData == null) {
                Log.i(TAG, "[mPostViewCallback] postViewData is null");
                return;
            }
            int previewHeight = 0;
            int previewWidth = 0;
            // Need to update post view size when camera sensor is not 0 degree.
            if (mJpegRotation == JPEG_ROTATION_0 || mJpegRotation == JPEG_ROTATION_180) {
                previewHeight = mICameraDevice.getParameters().getPreviewSize().height;
                previewWidth = mICameraDevice.getParameters().getPreviewSize().width;
            } else {
                previewWidth = mICameraDevice.getParameters().getPreviewSize().height;
                previewHeight = mICameraDevice.getParameters().getPreviewSize().width;
            }
            if (isDebugOpened()) {
                savePostViewBuffer(postViewData, "/sdcard/postView.yuv");
                Log.d(TAG, "previewWidth = " + previewWidth + ", previewHeight = " + previewHeight);
            }
            mICameraAppUi.updateThumbnailViewWithYuv(postViewData, previewWidth,
                    previewHeight, 0, ImageFormat.NV21);
            Log.d(TAG, "[mPostViewCallback] end");
        }
    };

    private class StereoPhotoDataCallback implements StereoDataCallback {
        public void onJpsCapture(byte[] jpsData) {
        }
        public void onMaskCapture(byte[] maskData) {
            if (maskData == null) {
                Log.i(TAG, "Mask data is null");
                return;
            }
            Log.i(TAG, "onMaskCapture maskData:" + maskData.length);
//            mAccessor.setJsonBuffer(maskData);
//            showQualityStatus(mAccessor.getGeoVerifyLevel(maskData),
//                    mAccessor.getPhoVerifyLevel(maskData));
        }
        public void onDepthMapCapture(byte[] depthData) {
        }
        public void onClearImageCapture(byte[] clearImageData) {
        }
        public void onLdcCapture(byte[] ldcData) {
        }

        public void onN3dCapture(byte[] n3dData) {
        }

        public void onDepthWrapperCapture(byte[] depthWrapper) {
        }
    }

    private void showQualityStatus(int geoFlag, int photoFlag) {
        if (!isDebugOpened()) {
            return;
        }
        String msg = null;
        msg = formateShow(geoFlag, photoFlag);
        DialogInterface.OnClickListener buttonListener =
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        };

        new AlertDialog.Builder(mActivity).setCancelable(false)
                .setIconAttribute(android.R.attr.alertDialogIcon).setTitle("")
                .setMessage(msg)
                .setNeutralButton(R.string.dialog_ok, buttonListener)
                .show();
    }

    private String formateShow(int geoFlag, int photoFlag) {
        Log.i(TAG, "geoFlag = " + geoFlag +  "photoFlag = " + photoFlag);
        String geo = null;
        String photo = null;
        if (geoFlag == PASS_NUM) {
            geo = PASS;
        } else if (geoFlag == FAIL_NUM) {
            geo = FAIL;
        } else {
            geo = WARNING;
        }
        if (photoFlag == PASS_NUM) {
            photo = PASS;
        } else if (photoFlag == FAIL_NUM) {
            photo = FAIL;
        } else {
            photo = WARNING;
        }
        return GEO_QUALITY + geo + "\n" + PHO_QUALITY + photo;
    }

    private boolean isDebugOpened() {
        boolean enable = SystemProperties
                .getInt("debug.STEREO.enable_verify", 0) == 1 ? true : false;
        Log.i(TAG, "[isDebugOpened]return :" + enable);
        return enable;
    }

    private boolean isShot2ShotDebugOpened() {
        boolean enable = SystemProperties
                .getInt("debug.bmdenoise.fasts2s", 0) == 0 ? true : false;
        Log.i(TAG, "[isShot2ShotDebugOpened]return :" + enable);
        return enable;
    }

    /**
     * This class used for Stereo photo take picture.
     */
    class StereoPhotoCategory extends CameraCategory {
        public StereoPhotoCategory() {
        }

        public void takePicture() {
            if (!mAdditionManager.execute(AdditionActionType.ACTION_TAKEN_PICTURE)) {
                mICameraDevice.setStereoDataCallback(mStereoPhotoDataCallback);
                if (isShot2ShotDebugOpened()) {
                    mICameraDevice.takePicture(mShutterCallback, null, null, mJpegPictureCallback);
                } else {
                    mICameraDevice.takePicture(mShutterCallback, null, mPostViewCallback,
                                               mJpegPictureCallback);
                }
                mICameraAppUi.setViewState(ViewState.VIEW_STATE_CAPTURE);
            }
        }
    }

    private StereoGestureListener mStereoGestureListener = new StereoGestureListener() {

        @Override
        public boolean onSingleTapUp(float x, float y) {
            return false;
        }

        @Override
        public boolean onScale(float focusX, float focusY, float scale) {
            return false;
        }

        @Override
        public boolean onScaleBegin(float focusX, float focusY) {
            return false;
        }
    };


    private void savePostViewBuffer(byte[] postViewBuffer, String path) {
        Log.i(TAG, "[savePostViewBuffer]path = " + path);
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(path);
            out.write(postViewBuffer);
            out.close();
        } catch (IOException e) {
            Log.e(TAG, "[savePostViewBuffer]Failed to write image,exception:", e);
        } finally {
            if (out != null) {
                try {
                    out.close();
                } catch (IOException e) {
                    Log.e(TAG, "[savePostViewBuffer]ioexception:", e);
                }
            }
        }
    }
}
