/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.soundrecorder;

import android.content.Context;
import android.media.MediaRecorder;
import android.os.Environment;
import android.os.SystemClock;
import android.os.storage.StorageManager;

import com.android.soundrecorder.RecordParamsSetting.RecordParams;

import com.mediatek.soundrecorder.ext.ExtensionHelper;
import com.mediatek.soundrecorder.ext.IRecordingTimeCalculationExt;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Recorder class to wrapper the MediaRecorder.
 */
public class Recorder implements MediaRecorder.OnErrorListener {

    public static final String RECORD_FOLDER = "Recording";
    public static final String SAMPLE_SUFFIX = ".tmp";

    private static final String TAG = "SR/Recorder";
    private static final String SAMPLE_PREFIX = "record";

    // M: the three below are all in millseconds
    private long mSampleLength = 0;
    private long mSampleStart = 0;
    private long mPreviousTime = 0;

    private File mSampleFile = null;
    private final StorageManager mStorageManager;
    private MediaRecorder mRecorder = null;
    private RecorderListener mListener = null;
    private int mCurrentState = SoundRecorderService.STATE_IDLE;

    // M: used for audio pre-process
    private boolean[] mSelectEffect = null;

    /**
     * Recorder Callback.
     */
    public interface RecorderListener {
        /**
         * State Callback.
         *
         * @param recorder the Recorder instance
         * @param stateCode the status code
         */
        void onStateChanged(Recorder recorder, int stateCode);

        /**
         * Error Callback.
         *
         * @param recorder the Recorder instance
         * @param errorCode the error code
         */
        void onError(Recorder recorder, int errorCode);
    }

    @Override
    public void onError(MediaRecorder recorder, int errorType, int extraCode) {
        LogUtils.i(TAG, "<onError> errorType = " + errorType + "; extraCode = " + extraCode);
        stopRecording();
        mListener.onError(this, ErrorHandle.ERROR_RECORDING_FAILED);
    }

    /**
     * M: Constructor of Recorder.
     * @param storageManager instance of StorageManager
     * @param listener recording callback
     */
    public Recorder(StorageManager storageManager, RecorderListener listener) {
        mStorageManager = storageManager;
        mListener = listener;
    }

    /**
     * M: get the current amplitude of MediaRecorder, used by VUMeter.
     * @return the amplitude value
     */
    public int getMaxAmplitude() {
        synchronized (this) {
            if (null == mRecorder) {
                return 0;
            }
            return (SoundRecorderService.STATE_RECORDING != mCurrentState) ? 0 : mRecorder
                    .getMaxAmplitude();
        }
    }

    /**
     * M: get the file path of current sample file.
     * @return sample file path
     */
    public String getSampleFilePath() {
        return (null == mSampleFile) ? null : mSampleFile.getAbsolutePath();
    }

    public long getSampleLength() {
        return mSampleLength;
    }

    public File getSampFile() {
        return mSampleFile;
    }

    /**
     * M: get how long time we has recorded.
     * @return the record length, in millseconds
     */
    public long getCurrentProgress() {
        if (SoundRecorderService.STATE_RECORDING == mCurrentState) {
            long current = SystemClock.elapsedRealtime();
            return (long) (current - mSampleStart + mPreviousTime);
        } else if (SoundRecorderService.STATE_PAUSE_RECORDING == mCurrentState) {
            return (long) (mPreviousTime);
        }
        return 0;
    }

    /**
     * M: set Recorder to initial state.
     *
     * @return reset result
     */
    public boolean reset() {
        /** M:modified for stop recording failed. @{ */
        boolean result = true;
        synchronized (this) {
            if (null != mRecorder) {
                try {
                    /**M: To avoid NE while mCurrentState is not prepared.@{**/
                    if (mCurrentState == SoundRecorderService.STATE_PAUSE_RECORDING
                            || mCurrentState == SoundRecorderService.STATE_RECORDING) {
                        mRecorder.stop();
                    }
                    /**@}**/
                } catch (RuntimeException exception) {
                    exception.printStackTrace();
                    result = false;
                    LogUtils.e(TAG,
                            "<stopRecording> recorder illegalstate exception in recorder.stop()");
                } finally {
                    mRecorder.reset();
                    mRecorder.release();
                    mRecorder = null;
                }
            }
        }
        mSampleFile = null;
        mPreviousTime = 0;
        mSampleLength = 0;
        mSampleStart = 0;
        /**
         * M: add for some error case for example pause or goon recording
         * failed. @{
         */
        mCurrentState = SoundRecorderService.STATE_IDLE;
        /** @} */
        return result;
    }

    /**
     * Start recording.
     *
     * @param context the context
     * @param params the recording parameters.
     * @param fileSizeLimit the file size limitation
     * @return the record result
     */
    public boolean startRecording(Context context, RecordParams params, int fileSizeLimit) {
        LogUtils.i(TAG, "<startRecording> begin");
        if (SoundRecorderService.STATE_IDLE != mCurrentState) {
            return false;
        }
        reset();

        if (!createRecordingFile(params.mExtension)) {
            LogUtils.i(TAG, "<startRecording> createRecordingFile return false");
            return false;
        }
        if (!initAndStartMediaRecorder(context, params, fileSizeLimit)) {
            LogUtils.i(TAG, "<startRecording> initAndStartMediaRecorder return false");
            return false;
        }
        mSampleStart = SystemClock.elapsedRealtime();
        setState(SoundRecorderService.STATE_RECORDING);
        LogUtils.i(TAG, "<startRecording> end");
        return true;
    }

    /**
     * Pause the recording.
     *
     * @return the pause result
     */
    public boolean pauseRecording() {
        if ((SoundRecorderService.STATE_RECORDING != mCurrentState) || (null == mRecorder)) {
            mListener.onError(this, SoundRecorderService.STATE_ERROR_CODE);
            return false;
        }
        try {
            mRecorder.pause();
        } catch (RuntimeException e) {
            LogUtils.e(TAG, "<pauseRecording> IllegalArgumentException");
            handleException(false, e);
            mListener.onError(this, ErrorHandle.ERROR_RECORDING_FAILED);
            return false;
        }
        mPreviousTime += SystemClock.elapsedRealtime() - mSampleStart;
        setState(SoundRecorderService.STATE_PAUSE_RECORDING);
        return true;
    }

    /**
     * Resume the recording.
     *
     * @return the resume result
     */
    public boolean goonRecording() {
        if ((SoundRecorderService.STATE_PAUSE_RECORDING != mCurrentState) || (null == mRecorder)) {
            return false;
        }
        // Handle RuntimeException if the recording couldn't start
        try {
            mRecorder.resume();
        } catch (RuntimeException exception) {
            LogUtils.e(TAG, "<goOnRecording> IllegalArgumentException");
            exception.printStackTrace();
            mRecorder.reset();
            mRecorder.release();
            mRecorder = null;
            mListener.onError(this, ErrorHandle.ERROR_RECORDING_FAILED);
            return false;
        }

        mSampleStart = SystemClock.elapsedRealtime();
        setState(SoundRecorderService.STATE_RECORDING);
        return true;
    }

    /**
     * Stop recording.
     *
     * @return the stop result
     */
    public boolean stopRecording() {
        LogUtils.i(TAG, "<stopRecording> start");
        if (((SoundRecorderService.STATE_PAUSE_RECORDING != mCurrentState) &&
             (SoundRecorderService.STATE_RECORDING != mCurrentState)) || (null == mRecorder)) {
            LogUtils.i(TAG, "<stopRecording> end 1");
            mListener.onError(this, SoundRecorderService.STATE_ERROR_CODE);
            return false;
        }
        boolean isAdd = (SoundRecorderService.STATE_RECORDING == mCurrentState) ? true : false;
        synchronized (this) {
            try {
                if (mCurrentState != SoundRecorderService.STATE_IDLE) {
                    mRecorder.stop();
                }
            } catch (RuntimeException exception) {
                /** M:modified for stop recording failed. @{ */
                handleException(false, exception);
                mListener.onError(this, ErrorHandle.ERROR_RECORDING_FAILED);
                LogUtils.e(TAG,
                        "<stopRecording> recorder illegalstate exception in recorder.stop()");
            } finally {
                if (null != mRecorder) {
                    mRecorder.reset();
                    mRecorder.release();
                    mRecorder = null;
                }
                if (isAdd) {
                    mPreviousTime += SystemClock.elapsedRealtime() - mSampleStart;
                }
                mSampleLength = mPreviousTime;
                LogUtils.i(TAG, "<stopRecording> mSampleLength in ms is " + mPreviousTime);
                LogUtils.i(TAG, "<stopRecording> mSampleLength in s is = " + mSampleLength);
                setState(SoundRecorderService.STATE_IDLE);
            }
            /** @} */
        }
        LogUtils.i(TAG, "<stopRecording> end 2");
        return true;
    }

    public int getCurrentState() {
        return mCurrentState;
    }

    private void setState(int state) {
        mCurrentState = state;
        mListener.onStateChanged(this, state);
    }

    private boolean initAndStartMediaRecorder(
            Context context, RecordParams recordParams, int fileSizeLimit) {
        LogUtils.i(TAG, "<initAndStartMediaRecorder> start");
        try {
            /**
             * M:Changed to catch the IllegalStateException and NullPointerException.
             * And the IllegalStateException will be caught and handled in RuntimeException
             * .@{
             */
            mSelectEffect = recordParams.mAudioEffect;
            IRecordingTimeCalculationExt ext =
                    ExtensionHelper.getRecordingTimeCalculationExt(context);
            mRecorder = ext.getMediaRecorder();
            mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mRecorder.setOutputFormat(recordParams.mOutputFormat);
            mRecorder.setOutputFile(mSampleFile.getAbsolutePath());
            mRecorder.setAudioEncoder(recordParams.mAudioEncoder);
            mRecorder.setAudioChannels(recordParams.mAudioChannels);
            mRecorder.setAudioEncodingBitRate(recordParams.mAudioEncodingBitRate);
            mRecorder.setAudioSamplingRate(recordParams.mAudioSamplingRate);
            if (fileSizeLimit > 0) {
                mRecorder.setMaxFileSize(fileSizeLimit);
            }
            mRecorder.setOnErrorListener(this);
            /**@}**/
            mRecorder.prepare();
            mRecorder.start();
        } catch (IOException exception) {
            LogUtils.e(TAG, "<initAndStartMediaRecorder> IO exception");
            // M:Add for when error ,the tmp file should been delete.
            handleException(true, exception);
            mListener.onError(this, ErrorHandle.ERROR_RECORDING_FAILED);
            return false;
        } catch (NullPointerException exception) {
            /**
             * M: used to catch the null pointer exception in ALPS01226113,
             * and never show any toast or dialog to end user. Because this
             * error just happened when fast tapping the file list button
             * after tapping record button(which triggered by tapping the
             * recording button in audio play back view).@{
             */
            handleException(true, exception);
            return false;
        } catch (IllegalStateException exception) {
            LogUtils.e(TAG, "<initAndStartMediaRecorder> RuntimeException");
            // M:Add for when error ,the tmp file should been delete.
            handleException(true, exception);
            mListener.onError(this, ErrorHandle.ERROR_RECORDER_OCCUPIED);
            return false;
        } catch (RuntimeException exception) {
            LogUtils.e(TAG, "<initAndStartMediaRecorder> RuntimeException");
            handleException(true, exception);
            mListener.onError(this, ErrorHandle.ERROR_RECORDING_FAILED);
            return false;
        }
        LogUtils.i(TAG, "<initAndStartMediaRecorder> end");
        return true;
    }

    private boolean createRecordingFile(String extension) {
        LogUtils.i(TAG, "<createRecordingFile> begin");
        String myExtension = extension + SAMPLE_SUFFIX;
        File sampleDir = null;
        if (null == mStorageManager) {
            return false;
        }
        sampleDir = new File(Environment.getExternalStorageDirectory().getAbsolutePath());
        LogUtils.i(TAG, "<createRecordingFile> sd card directory is:" + sampleDir);
        String sampleDirPath = sampleDir.getAbsolutePath() + File.separator + RECORD_FOLDER;
        sampleDir = new File(sampleDirPath);

        // find a available name of recording folder,
        // Recording/Recording(1)/Recording(2)
        int dirID = 1;
        while ((null != sampleDir) && sampleDir.exists() && !sampleDir.isDirectory()) {
            sampleDir = new File(sampleDirPath + '(' + dirID + ')');
            dirID++;
        }

        if ((null != sampleDir) && !sampleDir.exists() && !sampleDir.mkdirs()) {
            LogUtils.i(TAG, "<createRecordingFile> make directory [" + sampleDir.getAbsolutePath()
                    + "] fail");
        }

        boolean isCreateSuccess = true;
        try {
            if (null != sampleDir) {
                LogUtils.i(TAG, "<createRecordingFile> sample directory  is:"
                        + sampleDir.toString());
            }
            SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyyMMddHHmmss");
            String time = simpleDateFormat.format(new Date(System.currentTimeMillis()));
            StringBuilder stringBuilder = new StringBuilder();
            stringBuilder.append(SAMPLE_PREFIX).append(time).append(myExtension);
            String name = stringBuilder.toString();
            mSampleFile = new File(sampleDir, name);
            isCreateSuccess = mSampleFile.createNewFile();
            LogUtils.i(TAG, "<createRecordingFile> creat file success is " + isCreateSuccess);
            LogUtils.i(TAG, "<createRecordingFile> mSampleFile.getAbsolutePath() is: "
                    + mSampleFile.getAbsolutePath());
        } catch (IOException e) {
            mListener.onError(this, ErrorHandle.ERROR_CREATE_FILE_FAILED);
            LogUtils.e(TAG, "<createRecordingFile> io exception happens");
            e.printStackTrace();
            isCreateSuccess = false;
        } finally {
            LogUtils.i(TAG, "<createRecordingFile> end");
            return isCreateSuccess;
        }
    }

    /**
     * M: Handle Exception when call the function of MediaRecorder.
     *
     * @param isDeleteSample need to delete sample file or not
     * @param exception the exception info
     */
    public void handleException(boolean isDeleteSample, Exception exception) {
        LogUtils.i(TAG, "<handleException> the exception is: " + exception);
        exception.printStackTrace();
        if (isDeleteSample && mSampleFile != null) {
            mSampleFile.delete();
        }
        try {
            if (mRecorder != null) {
                mRecorder.reset();
            }
        } catch (IllegalStateException ex) {
            LogUtils.i(TAG, "Excpetion " + ex);
        } finally {
            mRecorder.release();
            mRecorder = null;
        }
    }
}
