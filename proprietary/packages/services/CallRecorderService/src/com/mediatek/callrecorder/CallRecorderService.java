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

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.storage.StorageVolume;
import android.util.Slog;
import android.widget.Toast;

import com.mediatek.internal.telecom.ICallRecorderCallback;
import com.mediatek.internal.telecom.ICallRecorderService;

public class CallRecorderService extends Service {
    private static final String TAG = CallRecorderService.class.getSimpleName();
    private static final int REQUEST_START_RECORDING = 1;
    private static final int REQUEST_STOP_RECORDING = 2;
    private static final int REQUEST_BIND_SERVICE = 3;
    private static final int REQUEST_UNBIND_SERVICE = 4;
    private static final int REQUEST_SET_CALLBACK = 5;
    private static final int REQUEST_QUIT = 6;

    private CallRecorder mCallRecorder;
    private HandlerThread mWorkerThread;
    private Handler mRecordHandler;
    private ICallRecorderCallback mCallback;

    private IBinder mBinder = new ICallRecorderService.Stub() {
        @Override
        public void startVoiceRecord() throws RemoteException {
            logd("[startVoiceRecord]");
            mRecordHandler.obtainMessage(REQUEST_START_RECORDING).sendToTarget();
        }

        @Override
        public void stopVoiceRecord() throws RemoteException {
            logd("[stopVoiceRecord]");
            mRecordHandler.obtainMessage(REQUEST_STOP_RECORDING).sendToTarget();
        }

        @Override
        public void setCallback(ICallRecorderCallback callback) throws RemoteException {
            logd("[setCallback]callback = " + callback);
            mRecordHandler.obtainMessage(REQUEST_SET_CALLBACK, callback).sendToTarget();
        }
    };

    private Recorder.OnStateChangedListener
            mCallRecorderStateListener = new Recorder.OnStateChangedListener() {
                @Override
                public void onStateChanged(int state) {
                    try {
                        mCallback.onRecordStateChanged(state);
                        // Remove disk check runnable after back to idle state
                        if (mRecordHandler != null && state == Recorder.IDLE_STATE) {
                            logd("remove mRecordDiskCheck");
                            mRecordHandler.removeCallbacks(mRecordDiskCheck);
                        }
                    } catch(RemoteException e) {
                        Slog.e(TAG, "CallRecordService: call listener onStateChanged() failed", e);
                    }
                }
            };

    /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
    private Recorder.OnEventListener
            mCallRecorderEventListener = new Recorder.OnEventListener() {
                @Override
                public void onEvent(int eventId, String eventContent) {
                    try {
                        mCallback.onRecordEvent(eventId, eventContent);
                    } catch (RemoteException e) {
                        Slog.e(TAG, "CallRecordService: call listener onRecordEvent() failed", e);
                    }
                }
            };
    /// @}

    @Override
    public IBinder onBind(Intent intent) {
        logd("[onBind]");
        mRecordHandler.obtainMessage(REQUEST_BIND_SERVICE).sendToTarget();
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        logd("[onUnbind]");
        mRecordHandler.obtainMessage(REQUEST_UNBIND_SERVICE).sendToTarget();
        return super.onUnbind(intent);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        logd("onCreate");
        mCallRecorder = CallRecorder.getInstance(this);
        mCallRecorder.setOnStateChangedListener(mCallRecorderStateListener);
        /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
        mCallRecorder.setEventListener(mCallRecorderEventListener);
        /// @}

        mWorkerThread = new HandlerThread("RecordWorker");
        mWorkerThread.start();
        mRecordHandler = new RecordHandler(mWorkerThread.getLooper());
        registerMediaStateReceiver();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        logd("onDestroy");
        unregisterMediaStateReceiver();
        mRecordHandler.sendEmptyMessage(REQUEST_QUIT);
    }

    private void registerMediaStateReceiver() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addDataScheme("file");
        registerReceiver(mBroadcastReceiver, intentFilter);
    }

    private void unregisterMediaStateReceiver() {
        if (mBroadcastReceiver != null) {
            unregisterReceiver(mBroadcastReceiver);
        }
    }

    private String mRecordStoragePath;
    /**
     * Handler base on worker thread Looper.
     * it will deal with the time consuming operations, such as start/stop recording
     */
    private class RecordHandler extends Handler {
        public RecordHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            logd("[handleMessage] message = " + messageToStrings(msg.what));
            switch (msg.what) {
                case REQUEST_START_RECORDING:
                    if (null != mCallRecorder) {
                        logd("[handleMessage]do start recording");
                        mRecordStoragePath = RecorderUtils.getExternalStorageDefaultPath(CallRecorderService.this);
                        mCallRecorder.startRecord();
                    }
                    break;
                case REQUEST_STOP_RECORDING:
                    if (null != mCallRecorder) {
                        logd("[handleMessage]do stop recording");
                        mCallRecorder.stopRecord();
                    }
                    mRecordStoragePath = null;
                    break;
                case REQUEST_BIND_SERVICE:
                    removeCallbacks(mRecordDiskCheck);
                    postDelayed(mRecordDiskCheck, 500);
                    break;
                case REQUEST_UNBIND_SERVICE:
                    mCallback = null;
                    mRecordHandler.removeCallbacks(mRecordDiskCheck);
                    break;
                case REQUEST_SET_CALLBACK:
                    ICallRecorderCallback callback = (ICallRecorderCallback) msg.obj;
                    mCallback = callback;
                    break;
                case REQUEST_QUIT:
                    logd("[handleMessage]quit worker thread and clear handler");
                    // quit to avoid looper leakage, and make sure the pending
                    // operations can finish before really quit
                    mWorkerThread.quit();
                    break;
                default:
                    logd("[handleMessage]unexpected message: " + msg.what);
                    break;
            }
        }
    }

    private String messageToStrings(int msgId) {
        switch (msgId) {
            case REQUEST_START_RECORDING:
                return "REQUEST_START_RECORDING";
            case REQUEST_STOP_RECORDING:
                return "REQUEST_STOP_RECORDING";
            case REQUEST_BIND_SERVICE:
                return "REQUEST_BIND_SERVICE";
            case REQUEST_UNBIND_SERVICE:
                return "REQUEST_UNBIND_SERVICE";
            case REQUEST_SET_CALLBACK:
                return "REQUEST_SET_CALLBACK";
            case REQUEST_QUIT:
                return "REQUEST_QUIT";
            default:
                return "Unknown message";
        }
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            if (mCallRecorder != null && (Intent.ACTION_MEDIA_EJECT.equals(intent.getAction())
                    || Intent.ACTION_MEDIA_UNMOUNTED.equals(intent.getAction()))) {
                StorageVolume storageVolume =
                        (StorageVolume) intent
                                .getParcelableExtra(StorageVolume.EXTRA_STORAGE_VOLUME);
                if (null == storageVolume) {
                    logd("storageVolume is null");
                    return;
                }
                String currentPath = storageVolume.getPath();
                if (null == mRecordStoragePath || !currentPath.equals(mRecordStoragePath)) {
                    logd("not current used storage unmount or eject");
                    return;
                }

                if (mCallRecorder.isRecording()) {
                    mRecordHandler.removeCallbacks(mRecordDiskCheck);
                    logd("Current used sd card is ejected, stop voice record");
                    mRecordHandler.obtainMessage(REQUEST_STOP_RECORDING).sendToTarget();
                }
            }
        }
    };

    private void checkRecordDisk() {
        logd("checkRecordDisk " + mRecordStoragePath);
        if (mCallRecorder != null && mCallRecorder.isRecording()
                && !RecorderUtils.diskSpaceAvailable(mRecordStoragePath)) {
            mRecordHandler.removeCallbacks(mRecordDiskCheck);
            Slog.e("AN: ", "Checking result, disk is full, stop recording...");
            mRecordHandler.obtainMessage(REQUEST_STOP_RECORDING).sendToTarget();
            mCallRecorder.showToast(R.string.confirm_device_info_full);
        } else {
            mRecordHandler.postDelayed(mRecordDiskCheck, 50);
        }
    }

    private Runnable mRecordDiskCheck = new Runnable() {
        public void run() {
            checkRecordDisk();
        }
    };

    private void logd(String msg) {
        android.util.Slog.d(TAG, msg);
    }
}
