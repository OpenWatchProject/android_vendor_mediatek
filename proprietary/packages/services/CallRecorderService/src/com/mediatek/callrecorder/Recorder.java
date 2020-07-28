/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.callrecorder;

import android.content.Context;
import android.content.res.Resources;
import android.media.MediaRecorder;
import android.media.MediaRecorder.OnErrorListener;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Slog;
import android.view.WindowManager;
import android.widget.Toast;

import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

/// M: ALPS04024049, fix issue show save record toast failed when call end. @{
import mediatek.telecom.MtkTelecomManager;
/// @}

public abstract class Recorder implements OnErrorListener {
    private static final String TAG = Recorder.class.getSimpleName();

    static final String SAMPLE_PREFIX = "recording";
    static final String SAMPLE_PATH_KEY = "sample_path";
    static final String SAMPLE_LENGTH_KEY = "sample_length";

    public static final int IDLE_STATE = 0;
    public static final int RECORDING_STATE = 1;

    public static final int NO_ERROR = 0;
    public static final int SDCARD_ACCESS_ERROR = 1;
    public static final int INTERNAL_ERROR = 2;
    public static final int STORAGE_FULL = 3;
    public static final int SUCCESS = 4;
    public static final int STORAGE_UNMOUNTED = 5;

    Context mContext;
    static boolean sIsRecording;

    public interface OnStateChangedListener {
        void onStateChanged(int state);
    }

    /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
    public interface OnEventListener {
        void onEvent(int eventId, String eventContent);
    }
    /// @}

    private long mSampleStart; // time at which latest record or play operation
    private int mState = IDLE_STATE;
    private MediaRecorder mRecorder;
    private OnStateChangedListener mOnStateChangedListener;
    /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
    private OnEventListener mOnEventListener;
    /// @}

    protected long mSampleLength; // length of current sample
    protected File mSampleFile;
    protected String mRecordStoragePath; // the path where saved recording file.

    protected Recorder(Context context) {
        mContext = context;
    }

    public void setOnStateChangedListener(OnStateChangedListener listener) {
        mOnStateChangedListener = listener;
    }

    public void setEventListener(OnEventListener listener) {
        mOnEventListener = listener;
    }

    /**
     * delete the recorded sample file
     */
    protected void deleteSampleFile() {
        if (mSampleFile != null) {
            mSampleFile.delete();
        }
        mSampleFile = null;
        mSampleLength = 0L;
    }

    /**
     * @param outputfileformat
     * @param extension
     * @throws IOException
     */
    public void startRecording(int outputfileformat, String extension) throws IOException {
        log("startRecording");

        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd_HH.mm.ss");
        String prefix = dateFormat.format(new Date());
        //File sampleDir = new File(StorageManagerEx.getDefaultPath());
        File sampleDir = Environment.getExternalStorageDirectory();

        if (!sampleDir.canWrite()) {
            Slog.i(TAG, "----- file can't write!! ---");
            // Workaround for broken sdcard support on the device.
            sampleDir = new File("/sdcard");
        }

        sampleDir = new File(sampleDir.getAbsolutePath() + "/PhoneRecord");
        if (!sampleDir.exists()) {
            sampleDir.mkdirs();
        }
        log("sampleDir path is " + sampleDir.getAbsolutePath());

        /// For ALPS01000670. @{
        // get the current path where saved recording files.
        mRecordStoragePath = sampleDir.getCanonicalPath();
        /// @}

        try {
            mSampleFile = File.createTempFile(prefix, extension, sampleDir);
        } catch (IOException e) {
            showToast(R.string.error_sdcard_access);
            Slog.i(TAG, "----***------- can't access sdcard !! " + e);
            e.printStackTrace();
            throw e;
        }

        log("finish creating temp file, start to record");

        mRecorder = new MediaRecorder();
        mRecorder.setOnErrorListener(this);
        ///M: ALPS02374165
        // change audio source according to system property
        // so that to test different record type @{
        //mRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
        String recordType = SystemProperties.get("persist.vendor.incallrec.audiosource", "-1");
        log("recordType is: " + Integer.parseInt(recordType));
        if (recordType.equals("-1")) {
            mRecorder.setAudioSource(MediaRecorder.AudioSource.VOICE_CALL);
            mRecorder.setAudioChannels(2);
        } else {
            mRecorder.setAudioSource(Integer.parseInt(recordType));
            if (recordType.equals("4")) {
                mRecorder.setAudioChannels(2);
            } else {
                mRecorder.setAudioChannels(1);
            }
        }
        /// @}

        mRecorder.setOutputFormat(outputfileformat);
        /// ALPS01426963 @{
        // change record encoder format for AMR_NB to ACC, so that improve the record quality.
        mRecorder.setAudioEncoder(MediaRecorder.AudioEncoder.AAC);
        mRecorder.setAudioEncodingBitRate(64000);
        mRecorder.setAudioSamplingRate(44100);
        /// @}
        mRecorder.setOutputFile(mSampleFile.getAbsolutePath());

        try {
            mRecorder.prepare();
            mRecorder.start();
            mSampleStart = System.currentTimeMillis();
            setState(RECORDING_STATE);
        } catch (Exception exception) {
            log("startRecording, encounter exception");
            handleException();
            deleteSampleFile();
        }
    }

    private void handleException() {
        if (sIsRecording) {
            sIsRecording = false;
        }
        showToast(R.string.alert_device_error);
        mRecorder.reset();
        mRecorder.release();
        mRecorder = null;
        setState(IDLE_STATE, true);
    }

    public void stopRecording() {
        log("stopRecording");
        if (mRecorder == null) {
            return;
        }
        mSampleLength = System.currentTimeMillis() - mSampleStart;
        try {
            mRecorder.stop();
        } catch (RuntimeException e) {
            // no output, use to delete the file
            e.printStackTrace();
            deleteSampleFile();
        }

        mRecorder.release();
        mRecorder = null;

        // setState(IDLE_STATE);
    }

    abstract protected void onMediaServiceError();

    void setState(int state) {
        setState(state, false);
    }

    void setState(int state, boolean force) {
        if (state != mState || force) {
            mState = state;
            fireStateChanged(mState);
        }
    }

    private void fireStateChanged(int state) {
        log("fireStateChanged " + state);
        if (mOnStateChangedListener != null) {
            mOnStateChangedListener.onStateChanged(state);
        }
    }

    @Override
    public void onError(MediaRecorder mp, int what, int extra) {
        log("onError");
        if (what == MediaRecorder.MEDIA_RECORDER_ERROR_UNKNOWN) {
            onMediaServiceError();
        }
        return;
    }

    /**
     * Get the recording path.
     * @return
     */
    public String getRecordingPath() {
        return mRecordStoragePath;
    }

    private void log(String msg) {
        Slog.d(TAG, msg);
    }

    final int SHOW_TOAST = 1;
    Resources mResources;
    Handler mMainThreadHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == SHOW_TOAST) {
                /** M: ALPS03786540
                 * When screen is locked, common toast can not be shown on UI.
                 * Solution: Change toast parameters: add flag(FLAG_SHOW_WHEN_LOCKED).
                 * @{
                 */
                Toast toast = Toast.makeText(mContext, (String) msg.obj, Toast.LENGTH_LONG);
                toast.getWindowParams().flags |= WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
                toast.show();
                /** @} */
            }
        }
    };

    void showToast(int resId) {
        showToast(mContext.getResources().getString(resId));
    }

    void showToast(String text) {
        mMainThreadHandler.obtainMessage(SHOW_TOAST, text).sendToTarget();
        log("showToast");
    }

    /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
    void showToastInClient(String text) {
        if (mOnEventListener != null) {
            log("showToastInClient");
            mOnEventListener.onEvent(MtkTelecomManager.CALL_RECORDING_EVENT_SHOW_TOAST, text);
        }
    }

    void showToastInClient(int resId) {
        showToastInClient(mContext.getResources().getString(resId));
    }
    /// @}
}
