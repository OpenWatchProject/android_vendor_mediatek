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

package com.mediatek.voicecommand.vis;

import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;
import android.service.voice.AlwaysOnHotwordDetector;
import android.service.voice.AlwaysOnHotwordDetector.Callback;
import android.service.voice.AlwaysOnHotwordDetector.EventPayload;
import android.service.voice.VoiceInteractionService;
import android.widget.Toast;

import java.util.Arrays;
import java.util.Locale;

import com.mediatek.common.voicecommand.IVoiceWakeupInteractionCallback;
import com.mediatek.common.voicecommand.IVoiceWakeupInteractionService;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.vis.Log;
import com.mediatek.voicecommand.vis.VoiceTrainingEnrollmentUtil;

/**
 * Service implementation for VoiceWakeup.
 */
public class VoiceWakeupInteractionService extends VoiceInteractionService {
    private static final String TAG = "VoiceWakeupInteractionService";

    private static final boolean DBG_TOAST = false;
    private static final boolean DEBUG = true;
    private static final int HANDLER_DELAY_TIME_MS = 1000;
    private AlwaysOnHotwordDetector mHotwordDetector = null;
    private IVoiceWakeupInteractionCallback mVoiceInteractionCallback;
    private int mHotwordAvailability = 0;
    private static boolean sIsOwner = true;

    private Handler mHandler = new Handler();
    private WorkerHandler mWorkerHandler = null;
    private HandlerThread mWorkerThread = null;

    /**
     * Worker thread for handling delayed messages in VIS.
     */
    private class WorkerHandler extends Handler {
        public static final int MSG_START_RECOGNITION_DELAYED = 1;
        public static final int MSG_SHUTDOWN_RECOGNITION = 2;

        WorkerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_START_RECOGNITION_DELAYED:
            {
                if (DEBUG) Log.i(TAG, "handle start recognition message");
                startRecognitionInternal();
            }
            break;

            case MSG_SHUTDOWN_RECOGNITION:
            {   //May need to use later
                if (DEBUG) Log.i(TAG, "handle Shutdown message");
            }
            break;

            default:
                break;
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "[onBind] intent action = " + intent.getAction());
        if ((VoiceCommandListener.VOICE_WAKEUP_SERVICE_ACTION).equals(intent.getAction())) {
            return mBinder.asBinder();
        }
        return super.onBind(intent);
    }

    private final Callback mHotwordCallback = new Callback() {
        @Override
        public void onAvailabilityChanged(int status) {
            Log.d(TAG, "[onAvailabilityChanged](" + status + ")");
            hotwordAvailabilityChangeHelper(status);
        }

        @Override
        public void onDetected(EventPayload eventPayload) {
            Log.d(TAG, "[onDetected]");
            hotwordDetectedHelper();
        }

        @Override
        public void onError() {
            Log.d(TAG, "[onError]");
            errorDetectedHelper();
        }

        @Override
        public void onRecognitionPaused() {
            Log.d(TAG, "[onRecognitionPaused]");
        }

        @Override
        public void onRecognitionResumed() {
            Log.d(TAG, "[onRecognitionResumed]");
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();

        mWorkerThread = new HandlerThread(TAG);
        mWorkerThread.start();
        mWorkerHandler = new WorkerHandler(mWorkerThread.getLooper());
    }

    public boolean isVOWActiveService() {
        return isActiveService(this, new ComponentName(this, getClass()));
    }

    @Override
    public void onReady() {
        super.onReady();
        Log.d(TAG, "Creating " + this);
        Log.d(TAG, "Keyphrase enrollment error? " + getKeyphraseEnrollmentInfo().getParseError());
        Log.d(TAG,
                "Keyphrase enrollment meta-data: "
                        + Arrays.toString(getKeyphraseEnrollmentInfo().listKeyphraseMetadata()));
        Log.d(TAG, "[onReady] UserId:" + UserHandle.myUserId() + ". Is mHotwordDetector NULL?" +
              (mHotwordDetector == null));

        // Old case ALPS02383951 required placing null check to ensure that
        // HotWordDetector restart is avoided for IPO boot case.
        if (mHotwordDetector == null) {
            Log.d(TAG, "Creating new");
            mHotwordDetector = createAlwaysOnHotwordDetector(
                VoiceCommandListener.VOW_ENROLLMENT_TEXT,
                Locale.forLanguageTag(VoiceCommandListener.VOW_ENROLLMENT_BCP47_LOCALE),
                mHotwordCallback);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Bundle args = new Bundle();
//        args.putParcelable("intent", new Intent(this, TestInteractionActivity.class));
//        startSession(args);
//        stopSelf(startId);
        Log.d(TAG, "onStartCommand ");
        return START_NOT_STICKY;
    }

    /**
     * Handle hotword availability change message.
     *
     * @param availability
     *            state of keyphrase availability
     */
    private void hotwordAvailabilityChangeHelper(int availability) {
        mHotwordAvailability = availability;
        Log.d(TAG, "hotwordAvailabilityChangeHelper availability = " + availability);
        Log.d(TAG, "[hotwordAvailabilityChangeHelper] UserId:" + UserHandle.myUserId()
              + ". Is mHotwordDetector NULL?" + (mHotwordDetector == null));

        switch (availability) {
        case AlwaysOnHotwordDetector.STATE_HARDWARE_UNAVAILABLE:
            break;

        case AlwaysOnHotwordDetector.STATE_KEYPHRASE_UNSUPPORTED:
            break;

        case AlwaysOnHotwordDetector.STATE_KEYPHRASE_UNENROLLED:
            Intent enroll = mHotwordDetector.createEnrollIntent();
            Log.d(TAG, "[hotwordAvailabilityChangeHelper]Need to enroll with " + enroll);
            break;

        case AlwaysOnHotwordDetector.STATE_KEYPHRASE_ENROLLED:
            if (VoiceTrainingEnrollmentUtil.getWakeupCmdStatus(getApplicationContext())
                   == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
                Log.d(TAG, "[hotwordAvailabilityChangeHelper] starting recognition");
                //Start recognition delayed
                mWorkerHandler.sendEmptyMessageDelayed(
                    WorkerHandler.MSG_START_RECOGNITION_DELAYED, HANDLER_DELAY_TIME_MS);
            }
            break;

         default:
            break;
        }
    }

    /**
     * Start recognition.
     *
     * @return boolean success
     */
    public boolean startRecognitionInternal() {
        Log.d(TAG, "[startRecognitionInternal] UserId:" + UserHandle.myUserId() +
              ". Is mHotwordDetector NULL?" + (mHotwordDetector == null));

        if (mHotwordDetector == null) {
            Log.e(TAG, "[startRecognition] mHotwordDetector is null ");
            return false;
        }
        if (!isActiveService(this, new ComponentName(this, getClass()))) {
            Log.e(TAG, "[startRecognition] VoiceWakeupInteractionService is not active service.");
            //return false;
        }
        if (!sIsOwner) {
            Log.e(TAG, "[startRecognition] Not in Owner profile.");
            //return false;
        }
        boolean success = false;
        try {
            success = mHotwordDetector
                    .startRecognition(AlwaysOnHotwordDetector.RECOGNITION_FLAG_NONE);
        } catch (UnsupportedOperationException e) {
            Log.e(TAG, "[startRecognition] exception: " + e.getMessage());
        } catch (IllegalStateException e) {
            Log.e(TAG, "[startRecognition] exception: " + e.getMessage());
        }
        Log.d(TAG, "[startRecognition]: " + success);
        if (!success) {
            if (DBG_TOAST) {
                Toast.makeText(getApplicationContext(), "Fail to startRecognition",
                    Toast.LENGTH_SHORT).show();
            }
            Log.d(TAG, "Fail to startRecognition");
        }
        return success;
    }

    /**
     * Stop recognition.
     *
     * @return boolean success
     */
    public boolean stopRecognitionInternal() {
        Log.d(TAG, "[stopRecognitionInternal] UserId:" + UserHandle.myUserId() +
              ". Is mHotwordDetector NULL?" + (mHotwordDetector == null));
        if (mHotwordDetector == null) {
            Log.e(TAG, "[stopRecognition] mHotwordDetector is null ");
            return false;
        }
        if (!isActiveService(this, new ComponentName(this, getClass()))) {
            Log.e(TAG, "[stopRecognition] VoiceWakeupInteractionService is not active service.");
            //return false;
        }
        if (!sIsOwner) {
            Log.e(TAG, "[stopRecognition] Not in Owner profile.");
            //return false;
        }
        boolean success = false;
        try {
            success = mHotwordDetector.stopRecognition();
        } catch (UnsupportedOperationException e) {
            Log.e(TAG, "[stopRecognition] exception: " + e.getMessage());
        } catch (IllegalStateException e) {
            Log.e(TAG, "[stopRecognition] exception: " + e.getMessage());
        }
        Log.d(TAG, "[stopRecognition]: " + success);
        if (!success) {
            if (DBG_TOAST) {
                Toast.makeText(getApplicationContext(), "Fail to stopRecognition",
                    Toast.LENGTH_SHORT).show();
            }
            Log.d(TAG, "Fail to stopRecognition");
        }
        return success;
    }

    /**
     * It is used to implement the VoiceWakeupInteractionService public methods.
     */
    private IVoiceWakeupInteractionService.Stub mBinder =
            new IVoiceWakeupInteractionService.Stub() {
        @Override
        public void registerCallback(IVoiceWakeupInteractionCallback callback) {
            Log.d(TAG, "IVoiceWakeupInteractionService registerCallback");
            mVoiceInteractionCallback = callback;
        }

        @Override
        public boolean startRecognition() {
            Log.d(TAG, "IVoiceWakeupInteractionService startRecognition mHotwordAvailability = "
                + mHotwordAvailability);
            if (mHotwordAvailability == AlwaysOnHotwordDetector.STATE_KEYPHRASE_ENROLLED) {
                return startRecognitionInternal();
            }
            return false;
        }

        @Override
        public boolean stopRecognition() {
            Log.d(TAG, "IVoiceWakeupInteractionService stopRecognition mHotwordAvailability = "
                + mHotwordAvailability);
            if (mHotwordAvailability == AlwaysOnHotwordDetector.STATE_KEYPHRASE_ENROLLED) {
                return stopRecognitionInternal();
            }
            return false;
        }

        @Override
        public void setCurrentUserOnSwitch(int userId) {
            if (userId == UserHandle.USER_SYSTEM) {
                sIsOwner = true;
                if (VoiceTrainingEnrollmentUtil.getWakeupCmdStatus(getApplicationContext())
                       == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
                    Log.d(TAG, "[setCurrentUserOnSwitch] Post msg for start recognition");
                    mWorkerHandler.sendEmptyMessageDelayed(
                        WorkerHandler.MSG_START_RECOGNITION_DELAYED, HANDLER_DELAY_TIME_MS);
                }
            } else {
                if (VoiceTrainingEnrollmentUtil.getWakeupCmdStatus(getApplicationContext())
                       == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED
                       && isVOWActiveService()) {
                    try {
                        stopRecognitionInternal();
                    } catch (SecurityException e) {
                        Log.e(TAG, "[setCurrentUserOnSwitch] Exception" + e);
                    }
                }
                sIsOwner = false;
            }
            if (DEBUG) Log.d(TAG, "[setCurrentUserOnSwitch] sIsOwner = " + sIsOwner);
        }
    };

    /**
     * Handle hotword detected message.
     */
    private void hotwordDetectedHelper() {
        // Only commandId = 0 support.
        int commandId = 0;
        if (!sIsOwner) {
            Log.e(TAG, "[hotwordDetectedHelper] Not in Owner profile.");
            return;
        }

        try {
            if (mVoiceInteractionCallback != null) {
                mVoiceInteractionCallback.onVoiceWakeupDetected(commandId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[hotwordDetectedHelper]exception: " + e.getMessage());
        }
        // In Google design once voice detected, Soundtrigger will be set up
        // mStart=false and Native will be set up IDLE.
        // But in mtk design, we need detect continuity, so workaround as:
        // when detected, startRecognition once again.
        startRecognitionInternal();
    }

    /**
     * Handle error detected message.
     */
    private void errorDetectedHelper() {
        if (!sIsOwner) {
            Log.e(TAG, "[hotwordDetectedHelper] Not in Owner profile.");
            return;
        }

        try {
            if (mVoiceInteractionCallback != null) {
                mVoiceInteractionCallback.onVoiceWakeupDetected(
                    VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[hotwordDetectedHelper]exception: " + e.getMessage());
        }
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        mVoiceInteractionCallback = null;
        mHotwordAvailability = 0;
        //mHotwordDetector = null;
        if (mWorkerThread != null) {
            mWorkerThread.quit();
        }
    }

    @Override
    public void onShutdown() {
        Log.d(TAG, "[onShutdown] UserId:" + UserHandle.myUserId()
              + ". Is mHotwordDetector NULL?" + (mHotwordDetector == null));
        super.onShutdown();
        mWorkerHandler.sendEmptyMessageDelayed(
            WorkerHandler.MSG_SHUTDOWN_RECOGNITION, HANDLER_DELAY_TIME_MS);
        //mHotwordAvailability = 0;
        //mHotwordDetector = null;
    }
}
