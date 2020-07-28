/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.mode.photo.intent;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.view.View;

import com.mediatek.camera.common.IAppUiListener;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.DeviceUsage;
import com.mediatek.camera.common.mode.photo.PhotoMode;
import com.mediatek.camera.common.mode.photo.PhotoModeHelper;
import com.mediatek.camera.common.mode.photo.device.IDeviceController.DataCallbackInfo;
import com.mediatek.camera.common.mode.photo.intent.IIntentPhotoUi.OkButtonClickListener;
import com.mediatek.camera.common.mode.photo.intent.IIntentPhotoUi.RetakeButtonClickListener;
import com.mediatek.camera.common.storage.MediaSaver;
import com.mediatek.camera.common.storage.MediaSaver.MediaSaverListener;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.Size;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * A mode used to process 3rd-party intent photo logical.
 */
public class IntentPhotoMode extends PhotoMode implements OkButtonClickListener,
        IAppUiListener.OnShutterButtonListener,
        RetakeButtonClickListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(IntentPhotoMode.class.getSimpleName());

    private static final String INLINED_DATA_INTENT = "inline-data";
    private static final String INTENT_EXTRA_DATA = "data";
    private static final String TEMP_CROP_FILE_NAME = "crop-temp";
    private static final String INTENT_EXTRA_CIRCLE = "circle";
    private static final String INTENT_EXTRA_CIRCLE_CROP = "circleCrop";
    private static final String INTENT_EXTRA_RETURN_DATA = "return-data";
    private static final String EXTRA_PHOTO_CROP_VALUE = "crop";
    private static final String EXTRA_QUICK_CAPTURE = "android.intent.extra.quickCapture";
    private static final String CROP_INTENT = "com.android.camera.action.CROP";

    private static final int REQUEST_CROP = 1000;
    private static final int MAX_NUMBER_PIXELS = 50 * 1024;

    private boolean mIsQuickCapture;
    private byte[] mJpegData;

    private String mCropValue;
    private Activity mActivity;
    private IIntentPhotoUi mIIntentPhotoUi;
    private Uri mSaveUri;

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
            boolean isFromLaunch) {
        super.init(app, cameraContext, isFromLaunch);
        LogHelper.d(TAG, "[init]");
        mIIntentPhotoUi = mIApp.getAppUi().getPhotoUi();
        mActivity = app.getActivity();
        //set the click event listener to listen the UI event.
        registerUIListener();
        //parser intent.
        parserIntent();
    }

    @Override
    public void resume(@Nonnull DeviceUsage deviceUsage) {
        super.resume(deviceUsage);
        //in such case the ui is not correct.
        //take a picture -> press home key ->relaunch camera.
        //why the ui is wrong?
        // the reason is in review state, all the UI is invisible,
        // because current preview is not started, but when camera is reopen again,
        // all the ui is need back to the normal state.
        // in this case, also need hide the review ui.
        // because the review ui maybe show after the pause called.
        mIIntentPhotoUi.hide();
        mIApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
    }

    @Override
    public void pause(@Nullable DeviceUsage nextModeDeviceUsage) {
        super.pause(nextModeDeviceUsage);
        mIIntentPhotoUi.hide();
        mJpegData = null;
    }

    @Override
    public void onDataReceived(DataCallbackInfo dataCallbackInfo) {
        //when data received, need notify UI, capture done.
        byte[] jpegData = dataCallbackInfo.data;
        LogHelper.d(TAG, "[onDataReceived] data: " + jpegData + ",mIsResumed = " + mIsResumed);
        //because api2 take picture don't stop preview, so need AP do this when picture callback.
        if (mIsResumed && mCameraApi == CameraDeviceManagerFactory.CameraApi.API2) {
            if (mModeHandler != null) {
                mModeHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        mIDeviceController.stopPreview();
                    }
                });
            }
        }
        if (jpegData != null && mIsResumed) {
            mJpegData = jpegData;

            if (mIsQuickCapture) {
                doAttach();
            } else if (mIIntentPhotoUi != null) {
                //when intent photo ui receive this message,can update the ui.
                mIIntentPhotoUi.onPictureReceived(jpegData);
            }
        }
    }

    @Override
    public void onPreviewCallback(byte[] data, int format) {
        if (mJpegData == null) {
            super.onPreviewCallback(data, format);
        }
    }

    @Override
    public void unInit() {
        super.unInit();
        unRegisterUIListener();
    }

    @Override
    public boolean onShutterButtonFocus(boolean pressed) {
        if (mIIntentPhotoUi != null && mIIntentPhotoUi.isShown()) {
            return true;
        }
        return super.onShutterButtonFocus(pressed);
    }

    @Override
    public boolean onShutterButtonClick() {
        if (mIIntentPhotoUi != null && mIIntentPhotoUi.isShown()) {
            return true;
        }
        return super.onShutterButtonClick();
    }

    @Override
    public boolean onShutterButtonLongPressed() {
        if (mIIntentPhotoUi != null && mIIntentPhotoUi.isShown()) {
            return true;
        }
        return super.onShutterButtonLongPressed();
    }

    @Override
    public boolean onBackPressed() {
        LogHelper.i(TAG, "[onBackPressed]");
        // if mode init is not completed before onBackPressed, do nothing.
        if (mActivity == null) {
            return true;
        }
        // Maybe current activity has a fragment in foreground, firstly
        // destroy fragment.
        if (mActivity.getFragmentManager().popBackStackImmediate()) {
            return true;
        }
        //need back to the launch camera activity.
        mActivity.setResult(Activity.RESULT_CANCELED, new Intent());
        mJpegData = null;
        //finish itself.
        mActivity.finish();
        return true;
    }

    @Override
    public void onOrientationChanged(int orientation) {
        super.onOrientationChanged(orientation);
        if (mIIntentPhotoUi != null) {
            mIIntentPhotoUi.onOrientationChanged(orientation);
        }
    }

    @Override
    public void onOkClickClicked() {
        if (!mActivity.isFinishing()) {
            doAttach();
        }
    }

    @Override
    public void onRetakeClicked() {
        if (isCameraAvailable()) {
            //restart preview
            mIDeviceController.stopPreview();
            mPhotoStatusResponder.statusChanged(KEY_PHOTO_CAPTURE, PHOTO_CAPTURE_STOP);
            mIDeviceController.startPreview();
            //reset view state
            mIApp.getAppUi().applyAllUIVisibility(View.VISIBLE);
            mIApp.getAppUi().applyAllUIEnabled(true);
            mJpegData = null;
        }
    }

    private void registerUIListener() {
        if (mIIntentPhotoUi != null) {
            mIIntentPhotoUi.setOkButtonClickListener(this);
            mIIntentPhotoUi.setRetakeButtonClickListener(this);
        }
    }

    private void unRegisterUIListener() {
        if (mIIntentPhotoUi != null) {
            mIIntentPhotoUi.setOkButtonClickListener(null);
            mIIntentPhotoUi.setRetakeButtonClickListener(null);
        }
    }

    private void parserIntent() {
        Intent intent = mActivity.getIntent();
        mCropValue = intent.getStringExtra(EXTRA_PHOTO_CROP_VALUE);
        mIsQuickCapture = intent.getBooleanExtra(EXTRA_QUICK_CAPTURE, false);
        mSaveUri = intent.getParcelableExtra(MediaStore.EXTRA_OUTPUT);
    }

    private void doAttach() {
        LogHelper.d(TAG, "[doAttach]mCropValue = " + mCropValue);

        if (mCropValue == null) {
            // First handle the no crop case -- just return the value. If the
            // caller specifies a "save uri" then write the data to its
            // stream. Otherwise, pass back a scaled down version of the bitmap
            // directly in the extras.
            if (mSaveUri != null) {
                attachToSaveUri();
            } else {
                attachToInLinedIntent();
            }
        } else {
            attachToCropIntent();
        }
    }

    private boolean isCameraAvailable() {
        return !MODE_DEVICE_STATE_CLOSED.equals(getModeDeviceStatus());
    }

    private void attachToSaveUri() {
        LogHelper.d(TAG, "[attachToSaveUri]");
        OutputStream outputStream = null;
        try {
            outputStream = mActivity.getContentResolver().openOutputStream(mSaveUri);
            if (outputStream != null) {
                outputStream.write(mJpegData);
                outputStream.close();
            }
            Intent resultIntent = new Intent();
            mActivity.setResult(Activity.RESULT_OK,resultIntent);
            mActivity.finish();
        } catch (IOException e) {
            LogHelper.w(TAG, "[doAttach] IOException");
        } finally {
            PhotoModeHelper.closeSilently(outputStream);
        }
    }

    private void attachToInLinedIntent() {
        LogHelper.d(TAG, "[attachToInLinedIntent]");
        int orientation = CameraUtil.getOrientationFromExif(mJpegData);
        Bitmap bitmap = PhotoModeHelper.makeBitmap(mJpegData, MAX_NUMBER_PIXELS);
        bitmap = PhotoModeHelper.rotateAndMirror(bitmap, orientation, false);
        Intent intent = new Intent(INLINED_DATA_INTENT);
        intent.putExtra(INTENT_EXTRA_DATA, bitmap);
        mActivity.setResult(Activity.RESULT_OK, intent);
        mActivity.finish();
    }

    private void attachToCropIntent() {
        LogHelper.d(TAG, "[attachToCropIntent]");
        // Save the image to a temp file and invoke the cropper
        Uri tempUri = getTempUri();
        if (tempUri == null) {
            LogHelper.w(TAG, "[attachToCropIntent] return because temp uri is null");
            return;
        }
        Bundle newExtras = new Bundle();
        if (INTENT_EXTRA_CIRCLE.equalsIgnoreCase(mCropValue)) {
            newExtras.putString(INTENT_EXTRA_CIRCLE_CROP, "true");
        }
        if (mSaveUri != null) {
            newExtras.putParcelable(MediaStore.EXTRA_OUTPUT, mSaveUri);
        } else {
            newExtras.putBoolean(INTENT_EXTRA_RETURN_DATA, true);
        }
        Intent cropIntent = new Intent(CROP_INTENT);
        cropIntent.setData(tempUri);
        cropIntent.putExtras(newExtras);
        mActivity.startActivityForResult(cropIntent, REQUEST_CROP);
    }

    private Uri getTempUri() {
        Uri tempUri = null;
        FileOutputStream tempStream = null;
        File path = mActivity.getFileStreamPath(TEMP_CROP_FILE_NAME);
        path.delete();
        try {
            tempStream = mActivity.openFileOutput(TEMP_CROP_FILE_NAME, Context.MODE_PRIVATE);
            try {
                tempStream.write(mJpegData);
                tempStream.close();
                tempUri = Uri.fromFile(path);
            } catch (IOException e) {
                mActivity.setResult(Activity.RESULT_CANCELED);
                return tempUri;
            }
        } catch (FileNotFoundException e) {
            mActivity.setResult(Activity.RESULT_CANCELED);
            return tempUri;
        } finally {
            PhotoModeHelper.closeSilently(tempStream);
        }
        return tempUri;
    }
}
