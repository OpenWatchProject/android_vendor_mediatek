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

package com.mediatek.camera.v2.stream;

import android.media.CamcorderProfile;
import android.media.MediaRecorder;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.util.ReflectUtil;
import com.mediatek.media.MediaRecorderEx;

import junit.framework.Assert;

import java.io.File;
import java.io.FileDescriptor;
import java.io.IOException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

/**
 *  A stream used for recording.
 */
public class RecordStream implements IRecordStream {
    private static final Tag TAG = new Tag(RecordStream.class.getSimpleName());
    private static final String[] PREF_CAMERA_VIDEO_HD_RECORDING_ENTRYVALUES =
        { "normal", "indoor" };
    private static final String CLASS_NAME = "android.media.MediaRecorder";
    private static final String METHOD_NAME_EXTRA = "setParameter";
    private static final Class[] METHOD_TYPES = new Class[] {String.class};
    protected MediaRecorder mMediaRecorder;

    // status observer
    protected final ArrayList<RecordStreamStatus>
                      mRecordingStreamObservers = new ArrayList<RecordStreamStatus>();
    // common part
    private CamcorderProfile mProfile;
    private int                                 mMaxDurationMs; // allow 0
    private long                                mMaxFileSizeBytes; // allow 0l
    private String                              mFileName; // allow to be null
    private FileDescriptor                      mFileDescriptor; //allow to be null
    private double                              mCaptureRateFps; //allow to be 0.0d
    private float                               mLocationLatitude; //allow to be 0.0f
    private float                               mLocationLongitude; //allow to be 0.0f
    private int                                 mRecordingOrientation; //only can be 0,90,180,270
    private boolean isExtraSuccess = false;
    // audio part
    private boolean                             mNeedRecordingAudio;
    private int                                 mAudioeSource = MediaRecorder.AudioSource.CAMCORDER;
//    private int                                 mHDRecordMode = HDRecordMode.UNSUPPORT;

    // video part
    private int                                 mVideoSource;

    private List<String> mRecorderParameters = new ArrayList<String>();

    private MediaRecorder.OnInfoListener mInfoListener = new MediaRecorder.OnInfoListener() {
        @Override
        public void onInfo(MediaRecorder mr, int what, int extra) {
            LogHelper.i(TAG, "onInfo what = " + what + " extra = " + extra);
            for (RecordStreamStatus observer : mRecordingStreamObservers) {
                observer.onInfo(what, extra);
            }
        }
    };

    private MediaRecorder.OnErrorListener mErrorListener = new MediaRecorder.OnErrorListener() {
        @Override
        public void onError(MediaRecorder mr, int what, int extra) {
            LogHelper.i(TAG, "onError what = " + what + " extra = " + extra);
            for (RecordStreamStatus observer : mRecordingStreamObservers) {
                observer.onError(what, extra);
            }
        }
    };

    public RecordStream() {

    }

    @Override
    public void releaseRecordStream() {

    }

    @Override
    public void registerRecordingObserver(RecordStreamStatus status) {
        if (status != null && !mRecordingStreamObservers.contains(status)) {
            mRecordingStreamObservers.add(status);
        }
    }

    @Override
    public void unregisterCaptureObserver(RecordStreamStatus status) {
        if (status != null && mRecordingStreamObservers.contains(status)) {
            mRecordingStreamObservers.remove(status);
        }
    }

    @Override
    public void setRecordingProfile(CamcorderProfile profile) {
        mProfile = profile;
    }

    @Override
    public void setMaxDuration(int max_duration_ms) {
        mMaxDurationMs = max_duration_ms;
    }

    @Override
    public void setMaxFileSize(long max_filesize_bytes) {
        mMaxFileSizeBytes = max_filesize_bytes;
    }

    @Override
    public void setOutputFile(String path) {
        mFileName = path;
    }

    @Override
    public void setOutputFile(FileDescriptor fd) {
        mFileDescriptor = fd;
    }

    @Override
    public void setCaptureRate(double fps) {
        mCaptureRateFps = fps;
    }

    @Override
    public void setLocation(float latitude, float longitude) {
        mLocationLatitude = latitude;
        mLocationLongitude = longitude;
    }

    @Override
    public void setOrientationHint(int degrees) {
        mRecordingOrientation = degrees;
    }


    @Override
    public void setMediaRecorderParameters(List<String> paramters) {
        mRecorderParameters = paramters;
    }

    @Override
    public void enalbeAudioRecording(boolean enable_audio) {
        mNeedRecordingAudio = enable_audio;
    }

    @Override
    public void setAudioSource(int audio_source) {
        mAudioeSource = audio_source;
    }

    @Override
    public void setHDRecordMode(String mode) {
    }

    @Override
    public void setVideoSource(int video_source) {
        mVideoSource = video_source;
    }

    @Override
    public void prepareRecord() {
        checkPrepareCondition();
        mMediaRecorder = new MediaRecorder();
        initializeRecorder();
        doPrepareRecord();
        mMediaRecorder.setOnErrorListener(getErrorListener());
        mMediaRecorder.setOnInfoListener(getInfoListener());
    }

    @Override
    public void startRecord() {
        mMediaRecorder.start();
        for (RecordStreamStatus observer : mRecordingStreamObservers) {
            observer.onRecordingStarted(isExtraSuccess);
        }
    }

    @Override
    public void pauseRecord() {
        try {
            mMediaRecorder.pause();
        } catch (IllegalStateException e) {
            LogHelper.e(TAG, "Could not pause media recorder. ");
        }
    }

    @Override
    public void resumeRecord() {
        mMediaRecorder.resume();
    }

    @Override
    public void stopRecord(boolean need_save_video) {
        try {
            mMediaRecorder.stop();
            mMediaRecorder.release();
        } catch (RuntimeException e) {
            deleteVideoFile();
            throw e;
        }

        boolean deleted = false;
        if (!need_save_video) {
            deleted = deleteVideoFile();
        }
        for (RecordStreamStatus observer : mRecordingStreamObservers) {
            observer.onRecordingStoped();
        }

        // after stop clear previous resource
        mProfile = null;
    }

    @Override
    public boolean deleteVideoFile() {
        if (mFileName == null) {
            return true;
        }
        File f = new File(mFileName);
        boolean deleted = f.delete();
        mFileName = null;
        LogHelper.i(TAG, "deleteVideoFile result:" + deleted);
        return deleted;
    }

    @Override
    public Surface getRecordInputSurface() {
        return mMediaRecorder.getSurface();
    }

    protected void checkPrepareCondition() {
        Assert.assertNotNull(mProfile);
        Assert.assertTrue(mFileDescriptor != null || mFileName != null);
    }

    protected void initializeRecorder() {
        Assert.assertNotNull(mMediaRecorder);
        if (mNeedRecordingAudio) {
            mMediaRecorder.setAudioSource(mAudioeSource);
            mMediaRecorder.setAudioChannels(mProfile.audioChannels);
        }
        mMediaRecorder.setVideoSource(mVideoSource);
        mMediaRecorder.setOutputFormat(mProfile.fileFormat);
        mMediaRecorder.setVideoFrameRate(mProfile.videoFrameRate);
        mMediaRecorder.setVideoSize(mProfile.videoFrameWidth, mProfile.videoFrameHeight);
        mMediaRecorder.setVideoEncodingBitRate(mProfile.videoBitRate);
        mMediaRecorder.setVideoEncoder(mProfile.videoCodec);
        MediaRecorderEx.setVideoBitOffSet(mMediaRecorder, 1, true);
        if (mNeedRecordingAudio) {
            mMediaRecorder.setAudioEncoder(mProfile.audioCodec);
            mMediaRecorder.setAudioEncodingBitRate(mProfile.audioBitRate);
            mMediaRecorder.setAudioSamplingRate(mProfile.audioSampleRate);
        }
        if (mCaptureRateFps > 0) {
            mMediaRecorder.setCaptureRate(mCaptureRateFps);
        }
        if (mLocationLatitude > 0 && mLocationLongitude > 0) {
            mMediaRecorder.setLocation(mLocationLatitude, mLocationLongitude);
        }
        if (mMaxFileSizeBytes > 0) {
            mMediaRecorder.setMaxFileSize(mMaxFileSizeBytes);
        }
        if (mFileDescriptor != null) {
            mMediaRecorder.setOutputFile(mFileDescriptor);
        } else {
            mMediaRecorder.setOutputFile(mFileName);
        }
        mMediaRecorder.setOrientationHint(mRecordingOrientation);
        mMediaRecorder.setMaxDuration(mMaxDurationMs);
        setRecorderParameters(mRecorderParameters);
    }

    protected MediaRecorder.OnErrorListener getErrorListener() {
        return mErrorListener;
    }

    protected MediaRecorder.OnInfoListener getInfoListener() {
        return mInfoListener;
    }

    protected String getFileName() {
        return mFileName;
    }

    protected CamcorderProfile getCamcorderProfile() {
        return mProfile;
    }

    protected int getRecordingOrientation() {
        return mRecordingOrientation;
    }
    protected void doPrepareRecord() {
        try {
            mMediaRecorder.prepare();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void setRecorderParameters(List<String> recordParamters) {
        if (mRecorderParameters != null) {
            try {
                for (int i = 0; i < mRecorderParameters.size(); i++) {
                    setParametersExtra(mMediaRecorder, mRecorderParameters.get(i));
                }
                isExtraSuccess = true;
            } catch (Exception ex) {
                isExtraSuccess = false;
                ex.printStackTrace();
            }
        }
    }
    public void setParametersExtra(MediaRecorder recorder, String info) {
        try {
            Class cls = Class.forName(CLASS_NAME);
            Method setParameterExtra = ReflectUtil.getMethod(cls, METHOD_NAME_EXTRA, METHOD_TYPES);
            ReflectUtil.callMethodOnObject(recorder, setParameterExtra, info);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }

    }
}
