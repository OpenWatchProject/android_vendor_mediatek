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

package com.mediatek.camera.common.mode.video.intentvideo;

import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.graphics.Bitmap;
import android.media.CamcorderProfile;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.provider.MediaStore;
import android.view.View;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.IReviewUI;
import com.mediatek.camera.common.mode.video.VideoMode;
import com.mediatek.camera.common.mode.video.device.IDeviceController;
import com.mediatek.camera.common.mode.video.recorder.IRecorder;
import com.mediatek.camera.common.mode.video.videoui.IVideoUI;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.storage.MediaSaver;
import com.mediatek.camera.common.utils.BitmapCreator;
import com.mediatek.camera.portability.MediaRecorderEx;

import java.io.FileDescriptor;
import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * This class is used for third ap use camera such as mms and so on.
 */

public class IntentVideoMode extends VideoMode {
    private static final Tag TAG = new Tag(IntentVideoMode.class.getSimpleName());
    private static final int ONE_SECOND_TO_MS = 1000;
    private static final String CAN_SHARE = "CanShare";
    private boolean mIsReviewUIShowing = false;
    private int mLimitDuration;
    private long mLimitSize;

    private ParcelFileDescriptor mVideoFileDescriptor;
    private ContentResolver mContentResolver;
    private Uri mCurrentVideoUri;
    private IReviewUI mReviewUI;
    private String  mFilePath;
    private Bitmap mBitmap;
    private Intent mIntent;

    @Override
    public void init(@Nonnull IApp app, @Nonnull ICameraContext cameraContext,
            boolean isFromLaunch) {
        super.init(app, cameraContext, isFromLaunch);
        mIntent = mApp.getActivity().getIntent();
    }

    @Override
    public void onOrientationChanged(int orientation) {
        super.onOrientationChanged(orientation);
        if (mReviewUI != null) {
            mReviewUI.updateOrientation(orientation);
        }
    }

    @Override
    public void unInit() {
        super.unInit();
        if (mReviewUI != null) {
            mReviewUI.hideReviewUI();
            if (mBitmap != null) {
                mBitmap.recycle();
                mBitmap = null;
            }
        }
    }

    @Override
    protected IRecorder.RecorderSpec modifyRecorderSpec(
            IRecorder.RecorderSpec recorderSpec, boolean isRecording) {
        if (!isRecording) {
            return recorderSpec;
        }
        analysisIntent();
        recorderSpec.maxDurationMs = mLimitDuration * ONE_SECOND_TO_MS;
        if (mLimitSize > 0 && mLimitSize < mVideoHelper.getRecorderMaxSize()) {
            recorderSpec.maxFileSizeBytes = mLimitSize;
        }
        if (mVideoFileDescriptor != null) {
            recorderSpec.outFileDes = mVideoFileDescriptor.getFileDescriptor();
            LogHelper.d(TAG, " recorderSpec.outFileDes = " +  recorderSpec.outFileDes);
        }
        return recorderSpec;
    }

    @Override
    protected IVideoUI.UISpec modifyUISpec(IVideoUI.UISpec spec) {
        if (mLimitSize > 0 && mIsParameterExtraCanUse) {
            spec.recordingTotalSize = mLimitSize;
        }
        spec.isSupportedVss = false;
        return spec;
    }

    @Override
    protected void addFileToMediaStore() {
        initReviewUI();
        configReviewUI();

        if (mVideoFileDescriptor != null) {
            FileDescriptor mFileDescriptor = null;
            mFileDescriptor = mVideoFileDescriptor.getFileDescriptor();
            if (mBitmap != null) {
                mBitmap.recycle();
                mBitmap = null;
            }
            mBitmap = BitmapCreator.createBitmapFromVideo(
                    mFileDescriptor, getProfile().videoFrameWidth);
            mReviewUI.showReviewUI(mBitmap);
            mIsReviewUIShowing = true;
            mAppUi.hideSavingDialog();
            mAppUi.applyAllUIVisibility(View.VISIBLE);
            mAppUi.setUIVisibility(IAppUi.PREVIEW_FRAME, View.INVISIBLE);
            mAppUi.setUIVisibility(IAppUi.SCREEN_HINT, View.INVISIBLE);
            updateVideoState(VideoState.STATE_REVIEW_UI);
        } else {
            int orientation;
            if (getCameraApi() == CameraDeviceManagerFactory.CameraApi.API1) {
                orientation = mVideoHelper.getRecordingRotation(
                        mApp.getGSensorOrientation(), mCameraDevice.getCameraInfo(0));
            } else {
                orientation = mVideoHelper.getRecordingRotation(
                        mApp.getGSensorOrientation(),
                        mVideoHelper.getCameraCharacteristics(mApp.getActivity(), mCameraId));
            }
            ContentValues contentValues =
                    mVideoHelper.prepareContentValues(true, orientation, null);
            mCameraContext.getMediaSaver().addSaveRequest(modifyContentValues(contentValues),
                    mVideoHelper.getVideoTempPath(), mFileSavedListener);
        }
    }

    @Override
    protected void setMediaRecorderParameters() {
        try {
            super.setMediaRecorderParameters();
            if (mLimitSize > 0) {
                MediaRecorderEx.setParametersExtra(
                        mRecorder.getMediaRecorder(),
                        mVideoHelper.RECORDER_INFO_SUFFIX
                                + mVideoHelper.MEDIA_INFO_RECORDING_SIZE);
            }
        } catch (IllegalStateException ex) {
            ex.printStackTrace();
        } catch (RuntimeException ex) {
            ex.printStackTrace();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    @Override
    protected ContentValues modifyContentValues(ContentValues contentValues) {
        if (mVideoFileDescriptor == null) {
            mFilePath = contentValues.getAsString(MediaStore.Video.Media.DATA);
        }
        return contentValues;
    }

    @Override
    protected Relation getPreviewedRestriction() {
        return IntentRestriction.getPreviewRelation().getRelation("preview", true);
    }

    @Override
    protected IDeviceController.DeviceCallback getPreviewStartCallback() {
        return mPreviewStartCallback;
    }

    @Override
    protected List<Relation> getRecordedRestriction(boolean isRecording) {
        List<Relation> relationList = new ArrayList<>();
        return relationList;
    }

    private IDeviceController.DeviceCallback mPreviewStartCallback = new
            IDeviceController.DeviceCallback() {
                @Override
                public void onCameraOpened(String cameraId) {
                    updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
                }

                @Override
                public void afterStopPreview() {
                    updateModeDeviceState(MODE_DEVICE_STATE_OPENED);
                }

                @Override
                public void beforeCloseCamera() {
                    updateModeDeviceState(MODE_DEVICE_STATE_CLOSED);
                }
                @Override
                public void onPreviewStart() {
                    if (mIsReviewUIShowing) {
                        updateVideoState(VideoState.STATE_REVIEW_UI);
                    } else {
                        updateVideoState(VideoState.STATE_PREVIEW);
                    }
                    mAppUi.applyAllUIEnabled(true);
                    updateModeDeviceState(MODE_DEVICE_STATE_PREVIEWING);
                    LogHelper.d(TAG, "[onPreviewStart]");
                }
                @Override
                public void onError() {
                    if (getVideoState() == VideoState.STATE_PAUSED
                            || getVideoState() == VideoState.STATE_RECORDING) {
                        onShutterButtonClick();
                    }
                }
            };



    private MediaSaver.MediaSaverListener mFileSavedListener =
            new MediaSaver.MediaSaverListener() {
                @Override
                public void onFileSaved(Uri uri) {
                    LogHelper.d(TAG, "[onFileSaved] uri = " + uri);
                    if (getVideoState() == VideoState.STATE_RECORDING ||
                            getVideoState() == VideoState.STATE_PAUSED) {
                        return;
                    }
                    if (mBitmap != null) {
                        mBitmap.recycle();
                        mBitmap = null;
                    }
                    if (mCurrentVideoUri == null) {
                        mCurrentVideoUri = uri;
                    }
                    mBitmap = BitmapCreator.createBitmapFromVideo(
                                      mFilePath, getProfile().videoFrameWidth);
                    mReviewUI.showReviewUI(mBitmap);
                    mIsReviewUIShowing = true;
                    updateVideoState(VideoState.STATE_PREVIEW);
                    mAppUi.hideSavingDialog();
                    mAppUi.applyAllUIVisibility(View.VISIBLE);
                    mAppUi.setUIVisibility(IAppUi.PREVIEW_FRAME, View.INVISIBLE);
                    mAppUi.setUIVisibility(IAppUi.SCREEN_HINT, View.INVISIBLE);
                }
            };

    private View.OnClickListener mRetakeListener = new View.OnClickListener() {
        public void onClick(View v) {
            LogHelper.d(TAG, "[mRetakeListener] onClick");
            mIsReviewUIShowing = false;
            updateVideoState(VideoState.STATE_PREVIEW);
            mReviewUI.hideReviewUI();
            mAppUi.setUIVisibility(IAppUi.PREVIEW_FRAME, View.VISIBLE);
            mAppUi.setUIVisibility(IAppUi.SCREEN_HINT, View.VISIBLE);
            mAppUi.applyAllUIEnabled(true);
            if (mBitmap != null) {
                mBitmap.recycle();
                mBitmap = null;
            }
            deleteCurrentVideo();
        }
    };

    private View.OnClickListener mPlayListener = new View.OnClickListener() {
        public void onClick(View v) {
            LogHelper.d(TAG, "[mPlayListener] onClick");
            startPlayVideoActivity(mCurrentVideoUri, getProfile());
        }
    };

    private View.OnClickListener mSaveListener = new View.OnClickListener() {
        public void onClick(View v) {
            LogHelper.d(TAG, "[mSaveListener] onClick");
            doReturnToCaller(mCurrentVideoUri);
            mApp.getActivity().finish();
        }
    };

    private void initReviewUI() {
        mReviewUI = mAppUi.getReviewUI();
    }

    private void configReviewUI() {
        LogHelper.d(TAG, "[configReviewUI]");
        IReviewUI.ReviewSpec spec = new IReviewUI.ReviewSpec();
        spec.retakeListener = mRetakeListener;
        spec.playListener = mPlayListener;
        spec.saveListener = mSaveListener;
        mReviewUI.initReviewUI(spec);
    }

    private void analysisIntent() {
        mContentResolver = mApp.getActivity().getContentResolver();
            mLimitSize = mIntent.getLongExtra(MediaStore.EXTRA_SIZE_LIMIT, 0L);
            mLimitDuration = mIntent.getIntExtra(MediaStore.EXTRA_DURATION_LIMIT, 0);
            mCurrentVideoUri = null;
            Uri saveUri = mIntent.getParcelableExtra(MediaStore.EXTRA_OUTPUT);
            if (saveUri != null) {
                try {
                    mVideoFileDescriptor = mContentResolver.openFileDescriptor(saveUri, "rw");
                    mCurrentVideoUri = saveUri;
                } catch (java.io.FileNotFoundException ex) {
                    LogHelper.e(TAG, ex.toString());
                }
            }
    }

    private void startPlayVideoActivity(Uri uri, CamcorderProfile profile) {
        LogHelper.d(TAG, "[startPlayVideoActivity], mCurrentVideoUri = "
                      + uri + ",profile = " + profile);
        if (profile == null) {
            LogHelper.e(TAG, "[startPlayVideoActivity] current proflie is error,please check!");
        }
        boolean canShow = true;
        Bundle extra = mIntent.getExtras();
        if (extra != null) {
            canShow = extra.getBoolean(CAN_SHARE, true);
        }
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.putExtra(CAN_SHARE, canShow);
        intent.setDataAndType(uri, mVideoHelper.convertOutputFormatToMimeType(profile.fileFormat));
        try {
            mApp.getActivity().startActivity(intent);
        } catch (ActivityNotFoundException ex) {
            LogHelper.e(TAG, "[startPlayVideoActivity] Couldn't view video " + uri, ex);
        }
    }

    private void doReturnToCaller(Uri uri) {
        LogHelper.d(TAG, "[doReturnToCaller] uri = " + uri);
        Intent resultIntent = new Intent();
        int resultCode = mApp.getActivity().RESULT_CANCELED;
        resultCode = mApp.getActivity().RESULT_OK;
        resultIntent.setData(uri);
        resultIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        LogHelper.d(TAG, "[doReturnToCaller] uri = " + uri);
        mApp.getActivity().setResult(resultCode, resultIntent);
    }

    /**
     * if video is started by MMS,when after recording,user want recording again.
     * ,so we need delete current video;
     */
    private void deleteCurrentVideo() {
        LogHelper.i(TAG, "[deleteCurrentVideo()] mCurrentVideoUri = " + mCurrentVideoUri);
            if (mCurrentVideoUri != null) {
                mContentResolver.delete(mCurrentVideoUri, null, null);
                mCurrentVideoUri = null;
            }
    }
}
