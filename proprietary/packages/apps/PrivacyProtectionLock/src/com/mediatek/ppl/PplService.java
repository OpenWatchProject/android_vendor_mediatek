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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ppl;

import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.common.ppl.IPplManager;
import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.ppl.MessageManager.Type;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
 * Control service of PPL. Configuration activities bind to this service to access the control data, while Keyguard
 * binds to this service to request unlock operation after user inputs unlock password. It monitors SIM status and lock
 * the phone if lock criteria is satisfied.
 * User sent remote SMS instructions are filtered by our plug-in in framework and are sent to this service via
 * startService(). This service will then execute the corresponding operation respectively and will broadcast LOCK,
 * UNLOCK and WIPE event to related components.
 */
public class PplService extends Service {
    private static final String TAG = "PPL/PPLService";

    /**
     * A collection of intents we used.
     */
    public static class Intents {
        // OUT: Send commands to listeners
        public static final String INTENT_NOTIFY_WIPE = "com.mediatek.ppl.NOTIFY_WIPE";
        public static final String INTENT_NOTIFY_MOUNT_SERVICE_WIPE = "com.mediatek.ppl.NOTIFY_MOUNT_SERVICE_WIPE";
        public static final String INTENT_NOTIFY_LOCK = "com.mediatek.ppl.NOTIFY_LOCK";
        public static final String INTENT_NOTIFY_UNLOCK = "com.mediatek.ppl.NOTIFY_UNLOCK";
        public static final String NOTIFICATION_KEY_IS_SIM_LOCK = "SimLock";

        // IN: Supported requests. The requester should provide password as an argument.
        public static final String INTENT_REQUEST_WIPE = "com.mediatek.ppl.REQUEST_WIPE";
        public static final String INTENT_REQUEST_LOCK = "com.mediatek.ppl.REQUEST_LOCK";
        public static final String INTENT_REQUEST_UNLOCK = "com.mediatek.ppl.REQUEST_UNLOCK";
        public static final String REQUEST_KEY_PASSWORD = "Password";
        public static final String INTENT_MOUNT_SERVICE_WIPE_RESPONSE = "com.mediatek.ppl.MOUNT_SERVICE_WIPE_RESPONSE";

        public static final String INTENT_REMOTE_INSTRUCTION_RECEIVED = "com.mediatek.ppl.REMOTE_INSTRUCTION_RECEIVED";
        public static final String INSTRUCTION_KEY_TYPE = "Type";
        public static final String INSTRUCTION_KEY_FROM = "From";
        public static final String INSTRUCTION_KEY_TO = "To";
        public static final String INSTRUCTION_KEY_SIM_ID = "SimId";

        public static final String INTENT_ENABLE = "com.mediatek.ppl.ENABLE";
        public static final String INTENT_DISABLE = "com.mediatek.ppl.DISABLE";

        public static final String PPL_MANAGER_SERVICE = "com.mediatek.ppl.service";

        public static final String UI_QUIT_SETUP_WIZARD = "com.mediatek.ppl.UI_QUIT_SETUP_WIZARD";
        public static final String UI_NO_SIM = "com.mediatek.ppl.UI_NO_SIM";
    }

    public static final int MAX_PASSWORD_LENGTH = 12;
    public static final int MIN_PASSWORD_LENGTH = 6;
    public static final int MAX_CONTACTS_NUM = 3;

    public static final int EDIT_TYPE_SETUP = 0;
    public static final int EDIT_TYPE_UPDATE = 1;

    private PplManager mPPLManager = null;
    private PlatformManager mPlatformManager;
    private EventReceiver mReceiver = null;
    private InternalControllerBinder mPrivateBinder = null;
    private SimTracker mSimTracker = null;
    private Handler mHandler = null;
    private List<Activity> mSensitiveActivityList = null;
    private boolean mPendingMessageProcessed = false;
    private boolean[] mSimReady;// true: receive ABSENT or READY state
    private int[] mReadyNotAbsent;// -1: not init. 1: receive READY but not ABSENT state
    private boolean[] mReadyChange;// "READY" and "ABSENT" are both sim ready state, the variable
                                   //  indicated if sim change from "READY" to "ABSENT" or
                                   // from "ABSENT" to "READY"
    public static final String ACTION_SIM_STATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";

    public static final int MAX_CONTACTS = 3;

    @Override
    public void onCreate() {
        mPlatformManager = PplApplication.getPlatformManager();
        mHandler = new Handler();
        // mSimTracker should be initialized before EventReceiver is registered
        mSimTracker = new SimTracker(PlatformManager.SIM_NUMBER, this);
        mSimReady = new boolean[mSimTracker.slotNumber];
        mReadyNotAbsent = new int[mSimTracker.slotNumber];
        mReadyChange = new boolean[mSimTracker.slotNumber];
        for (int i = 0 ; i < mSimTracker.slotNumber ; i++) {
            mSimReady[i] = false;
            mReadyNotAbsent[i] = -1;
            mReadyChange[i] = false;
        }
        mReceiver = new EventReceiver();
        mReceiver.initialize();
        mPPLManager = new PplManager(this, mSimTracker);
        mPrivateBinder = new InternalControllerBinder();
        mSensitiveActivityList = new ArrayList<Activity>();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "onStartCommand(" + intent + ")");
        if (intent == null) {
            return START_NOT_STICKY;
        }

        String action = intent.getAction();
        if (action == null) {
            return START_NOT_STICKY;
        }

        Log.d(TAG, "action is " + action);

        if (action.equals(Intents.INTENT_ENABLE)) {
            /// For case: onCreate -> enalbe -> disable -> service no destory -> enable
            /// flags must be re-init due to the value was outdated,
            /// and we preset SimTrakcer takeSnapShot must be executed while enable.
            for (int i = 0 ; i < mSimTracker.slotNumber ; i++) {
                if (IccCardConstants.State.UNKNOWN.ordinal() == mSimTracker.states[i] ||
                        IccCardConstants.State.NOT_READY.ordinal() == mSimTracker.states[i] ||
                        IccCardConstants.State.PIN_REQUIRED.ordinal() == mSimTracker.states[i] ||
                        IccCardConstants.State.PUK_REQUIRED.ordinal() == mSimTracker.states[i]) {
                    mSimReady[i] = false;
                    mReadyNotAbsent[i] = -1;
                } else if (IccCardConstants.State.ABSENT.ordinal() == mSimTracker.states[i]) {
                    mSimReady[i] = true;
                    mReadyNotAbsent[i] = 0;
                } else {
                    mSimReady[i] = true;
                    mReadyNotAbsent[i] = 1;
                }
                mReadyChange[i] = false;
            }
            Log.i(TAG, "Privacy Protection Lock enabled. mSimReady[]=" +
                    Arrays.toString(mSimReady) +
                    ", mReadyNotAbsent[]=" + Arrays.toString(mReadyNotAbsent) +
                    ", mReadyChange[]=" + Arrays.toString(mReadyChange));
            mPlatformManager.stayForeground(this);
            return START_STICKY;
        } else if (!mPPLManager.isEnabled()) {
            Log.i(TAG, "Privacy Protection Lock is disabled. stopSelf().");
            mPlatformManager.leaveForeground(this);
            stopSelf();
            return START_NOT_STICKY;
        } else {
            if (action.equals(Intents.INTENT_REMOTE_INSTRUCTION_RECEIVED)) {
                byte type = intent.getByteExtra(Intents.INSTRUCTION_KEY_TYPE, MessageManager.Type.INVALID);
                String from = intent.getStringExtra(Intents.INSTRUCTION_KEY_FROM);
                String to = intent.getStringExtra(Intents.INSTRUCTION_KEY_TO);
                int simId = intent.getIntExtra(Intents.INSTRUCTION_KEY_SIM_ID, -1);
                Log.d(TAG, "Remote instruction: " + type + ", " + Utils.stringForSecureLog(from) +
                        ", " + Utils.stringForSecureLog(to) + ", " + simId);
                if (type == MessageManager.Type.INVALID || simId == -1
                        || !mPPLManager.isTrustNumber(from)) {
                    Log.e(TAG, "Illegal reqeust, ignore it !!!");
                    return START_STICKY;
                }
                switch (type) {
                case Type.LOCK_REQUEST:
                    mPPLManager.lock(false, simId, from);
                    break;
                case Type.RESET_PW_REQUEST:
                    mPPLManager.resetPassword(simId, from);
                    break;
                case Type.UNLOCK_REQUEST:
                    //only for debug in eng or userdebug load.
                    if (!Utils.USER_LOAD) {
                        mPPLManager.unlock(null, simId, from);
                    }
                    break;
                case Type.WIPE_REQUEST:
                    mPPLManager.wipe(simId, from);
                    break;
                default:
                    Log.w(TAG, "Unsupported remote instruction type");
                    break;
                }
            } else if (action.equals(Intents.INTENT_DISABLE)) {
                if (!mPPLManager.hasPendingMessageInNvram()) {
                    Log.i(TAG, "Privacy Protection Lock is disabled. stopSelf().");
                    mPlatformManager.leaveForeground(this);
                    stopSelf();
                    return START_NOT_STICKY;
                }
            } else if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
                Log.w(TAG, "Boot complete start");
                performBootCheck();
                Log.w(TAG, "Boot complete end");
            } else if (action.equals(PplService.ACTION_SIM_STATE_CHANGED)) {
                Log.w(TAG, PplService.ACTION_SIM_STATE_CHANGED + " received");
                performBootCheck();
                if (!mPPLManager.hasPendingWipeRequest()) {
                    String state = intent.getStringExtra("ss");
                    //From Android Q, Phone ID instead of sim ID in this broadcast.
                    int simId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                    Log.d(TAG, "stateExtra = " + state + ", simId = " + simId);
                    // sim state change: NOT_READY->READY->IMSI->LOADED
                    if (simId != -1 && state != null &&
                        ("ABSENT".equals(state) || "READY".equals(state))) {
                        mSimReady[simId] = true;
                        //init mReadyNotAbsent
                        if (mReadyNotAbsent[simId] == -1) {
                            mReadyNotAbsent[simId] = "READY".equals(state) ? 1 : 0;
                            Log.d(TAG, "mReadyNotAbsent[" + simId + "] init as: "
                                    + mReadyNotAbsent[simId]);
                        }
                        if ("READY".equals(state)) {
                            if (mReadyNotAbsent[simId] == 1) {
                                mReadyChange[simId] = false;
                            } else {
                                mReadyNotAbsent[simId] = 1;
                                mReadyChange[simId] = true;
                            }
                        } else {
                            if (mReadyNotAbsent[simId] == 0) {
                                mReadyChange[simId] = false;
                            } else {
                                mReadyNotAbsent[simId] = 0;
                                mReadyChange[simId] = true;
                            }
                        }
                        Log.d(TAG, "mReadyNotAbsent[" + simId + "]=" + mReadyNotAbsent[simId] +
                                ", mReadyChange[" + simId + "]=" + mReadyChange[simId]);
                    } else if (simId != -1 && state != null && "NOT_READY".equals(state)) {
                        mSimReady[simId] = false;
                    }
                    if (allReady()) {
                        mPPLManager.onSimStateChanged(mReadyNotAbsent, mReadyChange);
                        if (!mPendingMessageProcessed) {
                            mPPLManager.processPendingMessagesInNvram();
                            mPendingMessageProcessed = true;
                        }
                    }
                }
            } else {
                Log.w(TAG, "Unsupported action: " + action);
            }
        }

        mPlatformManager.stayForeground(this);
        return START_STICKY;
    }

    private void performBootCheck() {
        // Check lock first.
        int lockFlag = mPPLManager.needLock();
        Log.w(TAG, "lockFlag is " + lockFlag);
        if (lockFlag != PplManager.NO_LOCK) {
            mPPLManager.doLock((lockFlag & PplManager.SIM_LOCK) == PplManager.SIM_LOCK);
        }
        if (mPPLManager.hasPendingWipeRequest()) {
            Log.w(TAG, "Wiping...do nothing");
            return;
        }
        Log.w(TAG, "hasPendingWipeRequest? " + mPPLManager.hasPendingWipeFlag());
        // Check for pending wipe operation.
        if (mPPLManager.hasPendingWipeFlag()) {
            Log.w(TAG, "wipeSucceeded? " + mPPLManager.wipeSucceeded() +
                    ". hasSentShutDown? " + mPPLManager.hasSentShutdown());
            if (mPPLManager.wipeSucceeded()) {
                mPPLManager.clearPendingWipeFlag();
            } else if (!mPPLManager.hasSentShutdown()){
                mPPLManager.wipe();
            }
        }
    }

    /*
     * Check whether all the SIM slots are ready. By "ready", we mean we can check whether there is SIM card is
     * inserted.
     */
    public boolean allReady() {
        String s = "mSimReady:";
        for (int i = 0; i < mSimReady.length; ++i) {
            s += mSimReady[i];
            s += ", ";
        }
        Log.d(TAG, s);
        for (int i = 0; i < mSimReady.length; ++i) {
            if (!mSimReady[i]) {
                return false;
            }
        }
        return true;
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (intent == null) {
            return null;
        }
        String action = intent.getAction();
        Log.d(TAG, "Bind request of action: " + action);
        if (action == null || !action.equals(Intents.PPL_MANAGER_SERVICE)) {
            return null;
        }

        String from = intent.getPackage();
        Log.d(TAG, "Bind request from " + from);

        if (from != null && from.equals("com.mediatek.ppl")) {
            return mPrivateBinder;
        } else {
            return new ExternalControllerBinder();
        }
    }

    @Override
    public void onDestroy() {
        mReceiver.destroy();
        mSimReady = null;
        mReadyNotAbsent = null;
        mPlatformManager.leaveForeground(this);
    }

    private class EventReceiver extends BroadcastReceiver {
        public void initialize() {
            registerReceiver(this, new IntentFilter(Intent.ACTION_SCREEN_OFF));
            registerReceiver(this,
                    new IntentFilter(Intents.INTENT_MOUNT_SERVICE_WIPE_RESPONSE),
                    "com.mediatek.permission.MOUNT_SERVICE_WIPE", null);

            IntentFilter filter = new IntentFilter();
            filter.addAction(MessageManager.SMS_SENT_ACTION);
            filter.addDataAuthority(MessageManager.SMS_PENDING_INTENT_DATA_AUTH, null);
            filter.addDataScheme(MessageManager.SMS_PENDING_INTENT_DATA_SCHEME);
            registerReceiver(this, filter);
        }

        public void destroy() {
            unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "EventReceiver: onReceive(" + intent + ")");
            String action = intent.getAction();
            if (action == null) {
                return;
            }
            if (action.equals(MessageManager.SMS_SENT_ACTION)) {
                mPPLManager.processMessageSentAction(intent, getResultCode());
            } else if (action.equals(Intents.INTENT_MOUNT_SERVICE_WIPE_RESPONSE)) {
                mPPLManager.checkPendingWipeRequest();
            } else if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                for (Activity activity : mSensitiveActivityList) {
                    activity.finish();
                }
            }
        }

    }

    /**
     * Binder used by internal clients such as activities.
     */
    public class InternalControllerBinder extends Binder {

        public void setProvision(boolean provision) {
            Log.d(TAG, "setProvision: provision=" + provision);
            ControlData editBuffer = mPPLManager.startEdit();
            editBuffer.setProvision(provision);
            mPPLManager.finishEdit(PplManager.ACTION_COMMIT);
        }

        /**
         * Enable PPL.
         *
         * @param updateSimInfo Whether update the SIM information with the values in current system.
         */
        public void enable(boolean updateSimInfo) {
            Log.d(TAG, "enable. updateSimInfo=" + updateSimInfo);
            mPPLManager.enable(null, updateSimInfo);
        }

        /**
         * Disable PPL.
         */
        public void disable() {
            Log.d(TAG, "disable.");
            mPPLManager.disable();
        }

        public void savePassword(String password, int type) {
            Log.d(TAG, "save pass word" + ", type=" + type);
            if (type == EDIT_TYPE_UPDATE) {
                mPPLManager.loadCurrentData();
            }
            ControlData editBuffer = mPPLManager.startEdit();
            if (type == EDIT_TYPE_SETUP) {
                editBuffer.clear();
            }
            byte[] salt = new byte[ControlData.SALT_SIZE];
            byte[] secret = PlatformManager.generateSecrets(password.getBytes(), salt);
            System.arraycopy(salt, 0, editBuffer.salt, 0, salt.length);
            System.arraycopy(secret, 0, editBuffer.secret, 0, editBuffer.secret.length);
            if (type == EDIT_TYPE_UPDATE) {
                mPPLManager.finishEdit(PplManager.ACTION_COMMIT);
            } else {
                mPPLManager.finishEdit(PplManager.ACTION_KEEP);
            }
        }

        /**
         * Check whether the password is correct.
         *
         * @param password
         * @return
         */
        public boolean verifyPassword(String password) {
            Log.d(TAG, "verify: data =" + Utils.stringForSecureLog(password));
            return mPPLManager.verifyPassword(password.getBytes());
        }

        /**
         * Send instruction description message.
         *
         * @param simId The SIM user selected via UI.
         */
        public void sendInstructionDescriptionMessage(int simId) {
            Log.d(TAG, "sendInstructionDescriptionMessage(" + simId + ")");
            mPPLManager.sendMessage(MessageManager.Type.INSTRUCTION_DESCRIPTION, null, simId,
                    mPPLManager.buildMessage(MessageManager.Type.INSTRUCTION_DESCRIPTION));
        }

        /**
         * Get a list of the IDs of current inserted SIM cards.
         *
         * @return
         */
        public int[] getInsertedSim() {
            mSimTracker.takeSnapshot();
            return mSimTracker.getInsertedSim();
        }

        public boolean isEnabled() {
            boolean ret = mPPLManager.isEnabled();
            Log.d(TAG, "isEnabled. ret=" + ret);
            return ret;
        }

        public boolean isProvisioned() {
            boolean ret = mPPLManager.isProvisioned();
            Log.d(TAG, "isProvisioned. ret=" + ret);
            return ret;
        }

        public void registerSensitiveActivity(Activity activity) {
            Log.d(TAG, "registerSensitiveActivity. actvity=" + activity);
            if (!mSensitiveActivityList.contains(activity)) {
                mSensitiveActivityList.add(activity);
            }
        }

        public void unregisterSensitiveActivity(Activity activity) {
            Log.d(TAG, "unregisterSensitiveActivity. actvity=" + activity);
            mSensitiveActivityList.remove(activity);
        }

        public void saveTustedNumberList(List<String> numberList, int type) {
            if (numberList == null || numberList.size() <= 0) {
                Log.d(TAG, "save data: list is null");
                return ;
            }
            for (int i = 0; numberList != null && i < numberList.size(); i++ ) {
                Log.d(TAG, "savedata: " + Utils.stringForSecureLog(numberList.get(i)) +
                           ", type=" + type);
            }
            if (type == EDIT_TYPE_UPDATE) {
                mPPLManager.loadCurrentData();
            }
            ControlData editBuffer = mPPLManager.startEdit();
            if (null == editBuffer.TrustedNumberList) {
                editBuffer.TrustedNumberList = new LinkedList<String>();
            } else {
                editBuffer.TrustedNumberList.clear();
            }
            for (String number : numberList) {
                editBuffer.TrustedNumberList.add(number);
            }

            if (type == EDIT_TYPE_UPDATE) {
                mPPLManager.finishEdit(PplManager.ACTION_COMMIT);
            } else {
                mPPLManager.finishEdit(PplManager.ACTION_KEEP);
            }
        }

        public List<String> loadTrustedNumberList(int type) {
            Log.d(TAG, "loadTrustedNumberList. type=" + type);

            if (type == EDIT_TYPE_UPDATE) {
                mPPLManager.loadCurrentData();
            }
            ControlData editBuffer = mPPLManager.startEdit();
            List<String> numberList = editBuffer.TrustedNumberList;
            mPPLManager.finishEdit(PplManager.ACTION_KEEP);
            return numberList;

        }
    }

    /**
     * Binder used by external clients such as Keyguard.
     */
    private class ExternalControllerBinder extends IPplManager.Stub {

        /**
         * Reset the password. It is provided here for possible future requirement to reset password via lock screen.
         */
        @Override
        public void resetPassword() throws RemoteException {
            Log.d(TAG, "resetPassword()");
            mPPLManager.resetPassword(-1, null);
        }

        /**
         * Check whether we should lock the phone.
         */
        @Override
        public int needLock() throws RemoteException {
            Log.d(TAG, "needLock()");
            return mPPLManager.needLock();
        }

        /**
         * Lock the phone.
         */
        @Override
        public void lock() throws RemoteException {
            Log.d(TAG, "lock()");
            mPPLManager.lock(false, -1, null);
        }

        /**
         * Interface for Keyguard to unlock the phone.
         *
         * @param password  The password used to unlock the phone. If the password is null, then return false;
         * @return
         */
        @Override
        public boolean unlock(String password) throws RemoteException {
            if (password == null) {
                Log.d(TAG, "unlock(<null>)");
                return false;
            } else {
                Log.d(TAG, "unlock(<password>)");
                return mPPLManager.unlock(password.getBytes(), PendingMessage.INVALID_SIM_ID, null);
            }
        }
    }

    /**
     * Show the toast message.
     *
     * @param message   The message to show.
     */
    void showToast(final String message) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast toast = Toast.makeText(
                        PplService.this,
                        message,
                        Toast.LENGTH_SHORT);
                toast.show();
            }
        });
    }

    /**
     * Run the Runnable r after delayMillis milliseconds.
     *
     * @param r
     * @param delayMillis
     */
    void postRun(Runnable r, long delayMillis) {
        mHandler.postDelayed(r, delayMillis);
    }

}
