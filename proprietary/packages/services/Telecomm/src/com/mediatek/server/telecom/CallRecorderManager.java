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

package com.mediatek.server.telecom;

import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;
import android.preference.PreferenceManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.TextView;
import android.widget.Toast;
import com.android.server.telecom.Call;
import com.android.server.telecom.CallState;
import com.android.server.telecom.CallsManager;
import com.android.server.telecom.CallsManagerListenerBase;
import com.android.server.telecom.TelephonyUtil;
import com.mediatek.internal.telecom.ICallRecorderCallback;
import com.mediatek.internal.telecom.ICallRecorderService;
import mediatek.telecom.MtkTelecomManager;
import com.android.server.telecom.R;

import java.util.Objects;

public class CallRecorderManager {

    private static final String TAG = CallRecorderManager.class.getSimpleName();
    private static final String CALL_RECORDER_SERVICE_PACKAGE_NAME = "com.mediatek.callrecorder";
    private static final String CALL_RECORDER_SERVICE_CLASS_NAME =
            "com.mediatek.callrecorder.CallRecorderService";

    private static final int RECORD_STATE_IDLE = 0;
    private static final int RECORD_STATE_STARTING = 1;
    private static final int RECORD_STATE_STARTED = 2;
    private static final int RECORD_STATE_STOPING = 3;

    private static final int MSG_SERVICE_CONNECTED = 1;
    private static final int MSG_SERVICE_DISCONNECTED = 2;
    private static final int MSG_START_RECORD = 3;
    private static final int MSG_STOP_RECORD = 4;
    private static final int MSG_RECORD_STATE_CHANGED = 5;
    /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
    private static final int MSG_RECORD_EVENT_NOTIFY = 6;
    /// @}
    /// M: Message id for showing call recorder confirm dialog in main thread. @{
    private static final int MSG_START_CALL_RECORDER_CONFIRM_DIALOG = 7;
    // SharedPreference key to mark whether show call recorder confirm dialog only once.
    private static final String KEY_NO_REMINDER_FOR_CALL_RECORDER
            = "no_reminder_for_call_recorder";
    /// @}
    private final Context mContext;
    private final CallsManager mCallsManager;
    private ICallRecorderService mCallRecorderService;
    private int mRecordingState = RECORD_STATE_IDLE;

    private Call mRecordingCall = null;
    private RecordStateListener mListener;
    private Call mPendingStopRecordCall = null;
    // Call recorder confirm dialog object.
    private AlertDialog mCallRecorderConfirmDialog;
    // The call related with call recorder confirm dialog, it will be set when dialog showing,
    // change to null after dialog dismiss.
    private Call mWaitRecordingConfirmCall;

    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            logd("[onServiceConnected]");
            mHandler.obtainMessage(MSG_SERVICE_CONNECTED, service).sendToTarget();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            logd("[onServiceDisconnected]");
            mHandler.obtainMessage(MSG_SERVICE_DISCONNECTED).sendToTarget();
        }
    };
    private Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SERVICE_CONNECTED:
                    handleServiceConnected((IBinder) msg.obj);
                    break;
                case MSG_SERVICE_DISCONNECTED:
                    handleServiceDisconnected();
                    break;
                case MSG_START_RECORD:
                    handleStartRecord((Call) msg.obj);
                    break;
                case MSG_STOP_RECORD:
                    handleStopRecord((Call) msg.obj);
                    break;
                case MSG_RECORD_STATE_CHANGED:
                    handleRecordStateChanged((Integer) msg.obj);
                    break;
                /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
                case MSG_RECORD_EVENT_NOTIFY:
                    handleRecordeEvent(msg.arg1, (String) msg.obj);
                    break;
                case MSG_START_CALL_RECORDER_CONFIRM_DIALOG:
                    handleStartCallRecorderConfirmDialog((Call) msg.obj);
                    break;
                default:
                    break;
                /// @}
            }
        }
    };

    private synchronized void setRecordingState(int recordState) {
        logd("setRecordingState to " + recordStateToString(recordState));
        mRecordingState = recordState;
    }

    private synchronized int getRecordingState() {
        return mRecordingState;
    }

    private synchronized Call getPendingStopRecordCall() {
        return mPendingStopRecordCall;
    }

    private synchronized void setPendingStopRecordCall(Call call) {
        mPendingStopRecordCall = call;
    }
    private boolean canStartRecord() {
        return getRecordingState() == RECORD_STATE_IDLE;
    }

    private boolean canStopRecord() {
        return getRecordingState() == RECORD_STATE_STARTED;
    }

    private boolean needPendingStopRecord() {
        return getRecordingState() == RECORD_STATE_STARTING;
    }

    private String recordStateToString(int recordState) {
        switch (recordState) {
            case RECORD_STATE_IDLE:
                return "RECORD_STATE_IDLE";
            case RECORD_STATE_STARTING:
                return "RECORD_STATE_STARTING";
            case RECORD_STATE_STARTED:
                return "RECORD_STATE_STARTED";
            case RECORD_STATE_STOPING:
                return "RECORD_STATE_STOPING";
            default:
                return "Unknown message";
        }
    }

    private void handleStartRecord(Call call) {
        logd("[handleStartRecord] on call " + call.getId());
        if (getRecordingState() != RECORD_STATE_STARTING
                    && getRecordingState() != RECORD_STATE_IDLE) {
            logw("[handleStartRecord] return without start, mPendingRequest=" + getRecordingState()
                    + ", mRecordingCall=" + mRecordingCall);
            return;
        }

        if (call.getState() != CallState.ACTIVE) {
            logw("[handleStartRecord]call not active: " + call.getState());
            setRecordingState(RECORD_STATE_IDLE);
            return;
        }

        /// M: ALPS03759580, Cannot start record if call is not foreground. @{
        if (mCallsManager.getForegroundCall() != call) {
            logw("[handleStartRecord]call not foreground");
            setRecordingState(RECORD_STATE_IDLE);
            return;
        }
        /// @}

        mRecordingCall = call;
        setRecordingState(RECORD_STATE_STARTING);
        if (mCallRecorderService != null) {
            startVoiceRecordInternal();
        } else {
            logd("[handleStartRecord]start bind");
            Intent intent = new Intent(MtkTelecomManager.ACTION_CALL_RECORD);
            intent.setComponent(new ComponentName(
                    CALL_RECORDER_SERVICE_PACKAGE_NAME,
                    CALL_RECORDER_SERVICE_CLASS_NAME));
            /**
             * M: ALPS03583071
             * [Detail] CallRecorderService will be killed when memory is low.
             * [Solution] Set flag(BIND_FOREGROUND_SERVICE) to CallRecorderService;
             */
            boolean isBound = mContext.bindServiceAsUser(intent, mConnection,
                    Context.BIND_AUTO_CREATE | Context.BIND_FOREGROUND_SERVICE, UserHandle.SYSTEM);
            if (!isBound) {
                MtkTelecomGlobals.getInstance().showToast(R.string.start_record_failed);
                mRecordingCall = null;
                mContext.unbindService(mConnection);
                setRecordingState(RECORD_STATE_IDLE);
            }
        }
    }

    private void handleStopRecord(Call call) {
        logd("[handleStopRecord] on call " + call.getId());
        if (getRecordingState() != RECORD_STATE_STOPING
                    && getRecordingState() != RECORD_STATE_STARTED) {
            logw("[handleStopRecord] unexpected state, just return");
            return;
        }
        if (mRecordingCall == null) {
            logw("[handleStopRecord] no call recording");
            setRecordingState(RECORD_STATE_IDLE);
            return;
        }
        if (mCallRecorderService == null) {
            logw("[handleStopRecord] call recorder service not connected");
            setRecordingState(RECORD_STATE_IDLE);
            return;
        }
        if (call != null && mRecordingCall != call) {
            logw("[handleStopRecord] state machine wrong, trying to stop a call which is not" +
                    "in recording state: " + mRecordingCall.getId() + " vs " + call.getId());
        }
        try {
            //M:fix CR:ALPS03438135,null pointer exception.
            if (mCallRecorderService == null) {
                logw("[handleStopRecord] call recorder service not connected");
                setRecordingState(RECORD_STATE_IDLE);
                return;
            }
            setRecordingState(RECORD_STATE_STOPING);
            mCallRecorderService.stopVoiceRecord();
        } catch (RemoteException e) {
            e.printStackTrace();
            setRecordingState(RECORD_STATE_IDLE);
        }
    }

    private void handleServiceConnected(IBinder binder) {
        mCallRecorderService = ICallRecorderService.Stub.asInterface(binder);
        startVoiceRecordInternal();
    }

    private void handleServiceDisconnected() {
        logd("[handleServiceDisconnected]");
        if (mRecordingCall != null && mListener != null) {
            logd("handleServiceDisconnected mRecordingCall not null, do error handling");
            mListener.onRecordStateChanged(MtkTelecomManager.CALL_RECORDING_STATE_IDLE);
        }
        mRecordingCall = null;
        mCallRecorderService = null;
        setRecordingState(RECORD_STATE_IDLE);
        setListener(null);
    }

    private void handleRecordStateChanged(int state) {
        logd("[handleRecordStateChanged]");
        if (mRecordingCall != null && mListener != null) {
            mListener.onRecordStateChanged(state);
        }
        if (state == MtkTelecomManager.CALL_RECORDING_STATE_IDLE) {
            mContext.unbindService(mConnection);
            mRecordingCall = null;
            mCallRecorderService = null;
            setListener(null);
            setRecordingState(RECORD_STATE_IDLE);
        } else if (state == MtkTelecomManager.CALL_RECORDING_STATE_ACTIVE) {
            setRecordingState(RECORD_STATE_STARTED);
            ///M: ALPS03638218 Call recording cannot be started, @{
            // If there has pending stop record which caused by pervious record stop failed.
            // Trigger stop record msg  again when record state change to STARTED.
            Call pendingStopRecordCall = getPendingStopRecordCall();
            if (pendingStopRecordCall != null && pendingStopRecordCall == mRecordingCall) {
                logd("handlePendingStopRecord");
                mHandler.obtainMessage(MSG_STOP_RECORD,
                        pendingStopRecordCall).sendToTarget();
            }
            setPendingStopRecordCall(null);
            /// @}
        }
    }

    /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
    private void handleRecordeEvent(int eventId, String eventContent) {
         logd("[handleRecordeEvent]event: " + eventId);
         switch (eventId) {
             case MtkTelecomManager.CALL_RECORDING_EVENT_SHOW_TOAST: {
                 Toast toast = Toast.makeText(mContext, eventContent, Toast.LENGTH_LONG);
                 toast.getWindowParams().flags |= WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
                 toast.show();
                 } break;
             default:
                 break;
         }
    }
    /// @}

    public CallRecorderManager(Context context, CallsManager callsManager) {
        mContext = context;
        mCallsManager = callsManager;
    }

    /**
     * Start voice recording during call.
     *
     * @param call The call will be recording.
     * @param needAllow True mean need to decide whether to show CallRecorderConfirmDialog
     * according to the user`s setting. false mean user already allow start voice recording, can
     * start voice recording directly.
     */
    public void startVoiceRecord(Call call, boolean needAllow) {
        if (!canStartRecord()) {
            logd("[startVoiceRecord] fail, record state is "
                + recordStateToString(getRecordingState()));
            return;
        }

        if (needAllow && !isAllowedToCallRecordByUser()) {
            logd("[startVoiceRecordIfAllowed] need user allow record");
            startCallRecorderConfirmDialog(call);
            return;
        }

        mRecordingCall = call;
        setRecordingState(RECORD_STATE_STARTING);
        setPendingStopRecordCall(null);
        logd("[startVoiceRecord] on call " + call.getId());
        mHandler.obtainMessage(MSG_START_RECORD, call).sendToTarget();
        /// M:  ALPS03666427 @{
        //   Issue scenario step by step:
        //   1. DSDA, 1A(TC@2)+1H(TC@1), TC@2 is recording.
        //   2. Swap them.
        //   3. TC@1 change to active firsty.
        //   4. User triggers starting record with TC@1 call, since TC@2 is already in recording
        //      state, TC@1's startVoiceRecord will fail. But TC@1 still called setLisenter
        //      to replace the RecordStateListener, this will cause TC@2 cannot get call state
        //      change notify anymore.
        //   5. After TC@2 enter hold state, we will try to stop voice recording.
        //   6. After CallRecorderManager stop call recording done, it will notify listener to
        //      update state. Because the latest listener is from TC@1 call, so the TC@2 call cannot
        //      get notified to update UI.
        //   Solution:
        //      Should not call setListener if start record failed. Then the call redord stop state
        //      change will be notified to TC@2 call, and UI will update accordingly.
        setListener(call.getRecordStateListener());
        /// @}
    }

    public void stopVoiceRecord(Call call) {
        if (!canStopRecord()) {
            ///M: ALPS03638218 Call recording cannot be started @{
            //  1. Current record is STARTING state and waiting response from record service.
            //  2. Stop record operation is triggered by call state change to hold.
            //      CallRecorderManager cannot handle stop operation here and just pending.
            //  3. When record service start record done, record state will change to STARTED.
            //     Then we will execute stop record msg.
            if (needPendingStopRecord()) {
                logd("[stopVoiceRecord] pending, record state is"
                        + recordStateToString(getRecordingState()));
                setPendingStopRecordCall(call);
            /// @}
            } else {
                logd("[stopVoiceRecord] fail, record state is"
                        + recordStateToString(getRecordingState()));
            }
            return;
        }

        setRecordingState(RECORD_STATE_STOPING);
        setPendingStopRecordCall(null);
        mHandler.obtainMessage(MSG_STOP_RECORD, call).sendToTarget();
    }

    private void startVoiceRecordInternal() {
        if (mCallRecorderService == null) {
            return;
        }
        try {
            mCallRecorderService.setCallback(new Callback());
            if (getRecordingState() == RECORD_STATE_STARTING) {
                mCallRecorderService.startVoiceRecord();
            } else if ((getRecordingState() == RECORD_STATE_STARTED)
                    || (getRecordingState() == RECORD_STATE_STOPING)) {
                logw("handleServiceConnected, unexpeted state %d" + getRecordingState());
                setRecordingState(RECORD_STATE_IDLE);
            }
        } catch (RemoteException e) {
            mRecordingCall = null;
            mCallRecorderService = null;
            setRecordingState(RECORD_STATE_IDLE);
            e.printStackTrace();
        }
    }

    private class Callback extends ICallRecorderCallback.Stub {

        @Override
        public void onRecordStateChanged(int state) throws RemoteException {
            logd("[onRecordStateChanged] state: " + state);
            mHandler.obtainMessage(MSG_RECORD_STATE_CHANGED, state).sendToTarget();
        }

        /// M: ALPS04024049, fix issue show save record toast failed when call end. @{
        /// Background:
        /// WMS add new rule for showing toast on P:
        /// Same uid do not allow exist two toast at the same time.
        /// Issue reproduce:
        /// 1. MO call and start call recording.
        /// 2. End call, CallLogManager will show call end toast in system process,
        /// CallRecorderService will show save record toast in its process.
        /// Cause:
        /// Since both toasts are showing with system uid but in different
        /// process within a short time interval. WMS will treate they are at the
        /// same time and remove the first toast in this case.
        /// Solution:
        /// Pass saving record toast string to CallRecordManager and let system
        /// process show it. Since both toasts are showing with system uid
        /// in system process, WMS can handle show toast normallly.
        ///
        /// Add callback method for call recorder service notify CallRecorderManager which
        /// in system process to show save record toast.
        @Override
        public void onRecordEvent(int eventId, String eventContent) throws RemoteException {
            logd("[onRecordEvent] event: " + eventId);
            mHandler.obtainMessage(MSG_RECORD_EVENT_NOTIFY, eventId, 0, eventContent)
                    .sendToTarget();
        }
        /// @}
    }

    public void setListener(RecordStateListener listener) {
        mListener = listener;
    }

    public interface RecordStateListener {
        void onRecordStateChanged(int state);
    }

    private void logd(String msg) {
        Log.d(TAG, msg);
    }
    private void logw(String msg) {
        Log.w(TAG, msg);
    }

    /**
     * M: Check whether call recorder is installed.
     *
     * @return true if call recorder is installed.
     */

    public boolean isCallRecorderInstalled() {
        boolean installed = false;
        try {
            ApplicationInfo info = mContext.getPackageManager().getApplicationInfo(
                    CALL_RECORDER_SERVICE_PACKAGE_NAME, 0);
            installed = true;
        } catch (NameNotFoundException e) {
            installed = false;
        }
        logd("[isCallRecorderInstalled] " + installed);
        return installed;
    }

    public void setAllowedToCallRecordByUser(boolean isAllowed) {
        logd("[setAllowedToCallRecordByUser] " + isAllowed);
        PreferenceManager.getDefaultSharedPreferences(mContext).edit()
                .putBoolean(KEY_NO_REMINDER_FOR_CALL_RECORDER, isAllowed).commit();
    }

    public boolean isAllowedToCallRecordByUser() {
        boolean isAllowed = PreferenceManager.getDefaultSharedPreferences(mContext)
                .getBoolean(KEY_NO_REMINDER_FOR_CALL_RECORDER, false);
        logd("[isAllowedToCallRecordByUser] " + isAllowed);
        return isAllowed;
    }

    public void startCallRecorderConfirmDialog(Call call) {
        mHandler.obtainMessage(MSG_START_CALL_RECORDER_CONFIRM_DIALOG, call).sendToTarget();
    }

    public Call getWaitRecordingConfirmCall() {
        return mWaitRecordingConfirmCall;
    }

    public void dismissCallRecorderConfirmDialog() {
        logd("[dismissCallRecorderConfirmDialog]");
        if (mCallRecorderConfirmDialog != null) {
            mCallRecorderConfirmDialog.dismiss();
            mCallRecorderConfirmDialog = null;
        }
        mWaitRecordingConfirmCall = null;
    }

    private void handleStartCallRecorderConfirmDialog(Call call) {
        logd("[handleStartCallRecorderConfirmDialog] on call " + call.getId());
        if (mCallRecorderConfirmDialog == null) {
            mCallRecorderConfirmDialog = createCallRecorderConfirmDialog();
        }
        mCallRecorderConfirmDialog.getWindow()
                .setType(WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);

        mWaitRecordingConfirmCall = call;
        mCallRecorderConfirmDialog.show();
    }

    // Since the voice recording files will be stored on external storage, which may later be
    // accessed by any other application then cause user privacy leak, need to show a alert dialog
    // for the user to decide whether to continue recording.
    private AlertDialog createCallRecorderConfirmDialog() {
        LayoutInflater factory = LayoutInflater.from(mContext);
        final View view = factory.inflate(R.layout.call_recorder_confirm_dialog, null);
        CheckBox checkbox = (CheckBox) view.findViewById(R.id.closeReminder);
        checkbox.setOnCheckedChangeListener((buttonView, isChecked) ->
                setAllowedToCallRecordByUser(isChecked));
        boolean isChecked = isAllowedToCallRecordByUser();
        checkbox.setChecked(isChecked);

        return new AlertDialog.Builder(mContext)
                        .setView(view)
                        .setPositiveButton(R.string.call_recorder_confirm_dialog_positive,
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int whichButton) {
                                        logd("CallRecorderConfirmDialog positive button onClick");
                                        Call call = mWaitRecordingConfirmCall;
                                        mWaitRecordingConfirmCall = null;
                                        if (call != null && call.canVoiceRecord()) {
                                           startVoiceRecord(call, false);
                                        }
                                        mCallRecorderConfirmDialog = null;
                                }
                        })
                        .setNegativeButton(R.string.call_recorder_confirm_dialog_negative, null)
                        .setOnDismissListener(new DialogInterface.OnDismissListener() {
                            @Override
                            public void onDismiss(DialogInterface dialog) {
                                logd("CallRecorderConfirmDialog onDismiss");
                                mWaitRecordingConfirmCall = null;
                                mCallRecorderConfirmDialog = null;
                            }
                        })
                        .create();

    }
}
