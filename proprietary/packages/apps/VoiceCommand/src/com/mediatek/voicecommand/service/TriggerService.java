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
package com.mediatek.voicecommand.service;

import android.app.ActivityManager;
import android.app.IActivityManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.hardware.soundtrigger.IRecognitionStatusCallback;
import android.hardware.soundtrigger.SoundTrigger;
import android.hardware.soundtrigger.SoundTrigger.ConfidenceLevel;
import android.hardware.soundtrigger.SoundTrigger.GenericRecognitionEvent;
import android.hardware.soundtrigger.SoundTrigger.Keyphrase;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseRecognitionEvent;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseRecognitionExtra;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel;
import android.hardware.soundtrigger.SoundTrigger.ModuleProperties;
import android.net.Uri;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.IDeviceIdleController;
import android.os.Bundle;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.Parcelable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.UserHandle;

import com.android.internal.app.ISoundTriggerService;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.business.VoiceTriggerBusiness;
import com.mediatek.voicecommand.cfg.VoiceModelCfg;
import com.mediatek.voicecommand.cfg.VoiceModelInfo;
import com.mediatek.voicecommand.data.ModelDbHelper;
import com.mediatek.voicecommand.util.Log;
import com.mediatek.vow.IVoiceWakeupBridge;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.UUID;

public class TriggerService extends Service {
    private static String TAG = "VoiceTriggerService";
    // Keyphrase related constants, must match those defined in enrollment_application.xml.
    private static final int KEYPHRASE_ID = 201;
    private static final int RECOGNITION_MODES = SoundTrigger.RECOGNITION_MODE_VOICE_TRIGGER;
    // Using "AMZINC" as the combined model label.
    private static final String SOUND_MODEL_LABEL = "AMZINC";
    private static final int PENDING_REQUEST_CODE = 1000;
    private static final String INTENT_KEY = "pending_intent_key";
    private static final String INTENT_EXTRA_STR = "pending_intent_str";
    private static final String[] PKG_NAME_WHITELIST = {
            "com.magiear.handsfree.assistant", "com.amazon.dee.app" };
    private static final int COMBINED_MODEL_HEADER_SIZE = 16;
    private ModelDbHelper mDbHelper = null;
    private final IBinder mBinder = new TriggerBinder();
    private ISoundTriggerService mStService = null;
    private IVoiceWakeupBridge mBridgeService = null;
    private static boolean sIsOwner = true;
    private static boolean sIsModelActive = false;
    private IVoiceWakeupCallback mCallback = null;
    private IPackageUpdateCallback mPackageCallback = null;
    private IVoiceModelUpdateCallback mVoiceModelCallback = null;
    private KeyphraseSoundModel mSoundModel = null;
    private ModuleProperties mModuleProperties = null;
    private VoiceModelCfg mModelCfg = null;
    private PackageListener mListener = null;
    private UserSwitchListener mUserListener = null;
    private KeyphraseRecognitionExtra[] mExtraBak = null;
    private Handler mHandler = new Handler();
    private WorkerHandler mWorkerHandler = null;
    private HandlerThread mWorkerThread = null;
    private SoundTriggerListener mStListener = null;

    private class WorkerHandler extends Handler {
        public static final int MSG_RESTAET_RECOG = 1;
        public static final int MSG_UPDATE_MODEL = 2;
        public static final int MSG_BACKGROUND_WHITELIST_ADD = 3;
        public static final int MSG_ACTIVE_APP = 4;
        WorkerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_RESTAET_RECOG:
                restartRecognition();
                break;
            case MSG_UPDATE_MODEL:
                String pkg = (String) msg.obj;
                updateModelsIfNeed(pkg);
                break;
            case MSG_BACKGROUND_WHITELIST_ADD:
                String[] bgApps = (String[]) msg.obj;
                addBackgroundWhitelist(bgApps);
                break;
            case MSG_ACTIVE_APP:
                String[] activeApps = (String[]) msg.obj;
                setAppActive(activeApps);
                break;
            default:
                break;
            }
        }
    }

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        super.onCreate();
        mDbHelper = new ModelDbHelper(this.getApplicationContext());
        mStService = ISoundTriggerService.Stub.asInterface(ServiceManager
                .getService(Context.SOUND_TRIGGER_SERVICE));
        mBridgeService = IVoiceWakeupBridge.Stub.asInterface(ServiceManager
                .getService("vow_bridge"));
        if (mBridgeService == null) {
            Log.d(TAG, "IVoiceWakeupBridge is null!");
        }
        ArrayList<ModuleProperties> modules = new ArrayList<>();
        int status = SoundTrigger.listModules(modules);
        if (status != SoundTrigger.STATUS_OK || modules.size() == 0) {
            Log.d(TAG, "listModules status=" + status + ", size=" + modules.size());
        } else {
            mModuleProperties = modules.get(0);
            Log.d(TAG, "module properties:" + mModuleProperties.toString());
        }
        mModelCfg = new VoiceModelCfg(this.getApplicationContext());
        mWorkerThread = new HandlerThread(TAG);
        mWorkerThread.start();
        mWorkerHandler = new WorkerHandler(mWorkerThread.getLooper());
        mListener = new PackageListener();
        mUserListener = new UserSwitchListener();
        mStListener = new SoundTriggerListener();
        registerListener();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "onStartCommand, intent=" + intent + " flags=" + flags + " startId=" + startId);
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG, "onBind");
        return mBinder;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy");
        if (mWorkerThread != null) {
            mWorkerThread.quit();
        }
        unregisterListener();
        super.onDestroy();
    }

    public class TriggerBinder extends Binder {
        public TriggerService getService() {
            Log.i(TAG, "getService");
            return TriggerService.this;
        }
    }

    private void setAppActive(String[] pkgNames) {
        // Set APP active to make sure AudioRecorder could get real data, not 0
        IDeviceIdleController deviceIdle = IDeviceIdleController.Stub.asInterface(
                ServiceManager.getService(Context.DEVICE_IDLE_CONTROLLER));
        for (int i = 0; i < pkgNames.length; i++) {
            try {
                // Max duration is 5 min, we set 1 min
                deviceIdle.addPowerSaveTempWhitelistApp(pkgNames[i],
                        60 * 1000L, UserHandle.myUserId(), "VoW");
            } catch (RemoteException e) {
                Log.d(TAG, "add whitelist remote exception", e);
            }
        }
    }

    private void restartRecognition() {
        if (mExtraBak != null) {
            int level = mExtraBak[0].coarseConfidenceLevel;
            ConfidenceLevel confidenceLevel = mExtraBak[0].confidenceLevels[0];
            startRecognition(confidenceLevel.userId, level, confidenceLevel.confidenceLevel);
        }
    }

    private static boolean verifyKeyphraseSoundModel(KeyphraseSoundModel soundModel) {
        if (soundModel == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] KeyphraseSoundModel must be non-null");
            return false;
        }
        if (soundModel.uuid == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] KeyphraseSoundModel must have a UUID");
            return false;
        }
        if (soundModel.data == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] KeyphraseSoundModel must have data");
            return false;
        }
        if (soundModel.keyphrases == null || soundModel.keyphrases.length != 1) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Keyphrase must be exactly 1");
            return false;
        }
        Keyphrase keyphrase = soundModel.keyphrases[0];
        if (keyphrase.id <= 0) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Keyphrase must have a valid ID");
            return false;
        }
        if (keyphrase.recognitionModes < 0) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Recognition modes must be valid");
            return false;
        }
        if (keyphrase.locale == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Locale must not be null");
            return false;
        }
        if (keyphrase.text == null) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Text must not be null");
            return false;
        }
        if (keyphrase.users == null || keyphrase.users.length == 0) {
            Log.e(TAG, "[verifyKeyphraseSoundModel] Keyphrase must have valid user(s)");
            return false;
        }
        return true;
    }

    /**
     * Gets the sound model byte[] data for the file path, null if none exists.
     * This should be used for enrollment purposes.
     *
     * @param filePath The file path of the sound model.
     * @return The sound model byte[] data.
     */
    private static byte[] getBytes(String filePath) {
        byte[] buffer = null;
        try {
            File file = new File(filePath);
            FileInputStream fis = new FileInputStream(file);
            ByteArrayOutputStream bos = new ByteArrayOutputStream(1000);
            byte[] b = new byte[1000];
            int n;
            while ((n = fis.read(b)) != -1) {
                bos.write(b, 0, n);
            }
            fis.close();
            bos.close();
            buffer = bos.toByteArray();
        } catch (FileNotFoundException e) {
            Log.w(TAG, "getBytes exception", e);
        } catch (IOException e) {
            Log.w(TAG, "getBytes exception", e);
        }
        return buffer;
    }

    /**
     * Gets the Long form for String input.
     * @param leastSigBits String parameter for converting to long.
     * @return long form of the string for leastSigBits.
     */
    private long stringToLong(String leastSigBits) {
        String str = "";
        for (int i = 0; i < leastSigBits.length(); i++) {
            int ch = (int) leastSigBits.charAt(i);
            String s = Integer.toHexString(ch);
            str = str + s;
        }
        // Use the HEX format radix to match sound model requirement
        return Long.parseLong(str, 16);
    }

    /**
     * Gets the UUID for the sound model.
     * Configuration utility UUID, agree with native owner:
     * The 0 bit set training mode: 0 represents speaker independent mode,
     *                              1 represents speaker dependent mode.
     * The 1~2 bits set command id: 00 represents commandId = 0,
     *                              01 represents commandId = 1,
     *                              10 represents commandId = 2.
     * @param trainingMode The training mode.
     * @param commandId The command id.
     * @return The UUID for the sound model.
     */
    private UUID getUUID(int trainingMode, int commandId) {
        long mostSigBits = (trainingMode - 1) | commandId << 1;
        return new UUID(mostSigBits, stringToLong(SOUND_MODEL_LABEL));
    }

    private boolean addOrUpdateSoundModel(KeyphraseSoundModel soundModel, String locale) {
        if (!verifyKeyphraseSoundModel(soundModel)) {
            return false;
        }

        int status = SoundTrigger.STATUS_ERROR;
        if (mDbHelper.updateKeyphraseSoundModel(soundModel)) {
            status = SoundTrigger.STATUS_OK;
        }
        Keyphrase kp = soundModel.keyphrases[0];
        if (loadModel(kp.id, UserHandle.myUserId(), locale)) {
            status = SoundTrigger.STATUS_OK;
        }
        return status == SoundTrigger.STATUS_OK;
    }

    private KeyphraseSoundModel getSoundModel(int keyphraseId,
            int userHandle, String locale) {
        Log.d(TAG, "getSoundModel keyId=" + keyphraseId + " userHandle=" + userHandle
                + " locale=" + locale);
        if (keyphraseId <= 0) {
            Log.d(TAG, "[getSoundModel] Keyphrase must have a valid ID");
            return null;
        }
        // Training model is universal model, do not care locale
        KeyphraseSoundModel model = mDbHelper.getKeyphraseSoundModel(
                keyphraseId, userHandle, VoiceCommandListener.VOW_ENROLLMENT_BCP47_LOCALE);
        if (model != null) {
            model = mergeModel(model, locale);
        }
        if (model == null) {
            Log.w(TAG,
                    "[getSoundModel] No models present for the given keyphrase ID");
            return null;
        } else {
            return model;
        }
    }

    private KeyphraseSoundModel mergeModel(KeyphraseSoundModel model, String locale) {
        byte[] voiceModelData = mDbHelper.getVoiceModelData(locale, 1);
        byte[] soundModelData = model.data;
        if (voiceModelData == null || soundModelData == null) {
            Log.w(TAG, "mergeModel voiceModelData: " + voiceModelData
                    + ", soundModelData:" + soundModelData);
            return null;
        }
        ByteBuffer buffer = ByteBuffer.allocate(voiceModelData.length
                + soundModelData.length + COMBINED_MODEL_HEADER_SIZE);
        // uint uFirstStageModelOffset
        buffer.putInt(COMBINED_MODEL_HEADER_SIZE);
        // uint uFirstStageModelSize
        buffer.putInt(voiceModelData.length);
        // uint uSecondStageModelOffset
        buffer.putInt(COMBINED_MODEL_HEADER_SIZE + voiceModelData.length);
        // uint uSecondStageModelSize
        buffer.putInt(soundModelData.length);
        buffer.put(voiceModelData);
        buffer.put(soundModelData);
        byte[] combineModelData = buffer.array();
        KeyphraseSoundModel soundModel = new KeyphraseSoundModel(model.uuid,
                null, combineModelData, model.keyphrases);
        return soundModel;
    }

    private boolean deleteSoundModel(int keyphraseId, String locale, int userHandle) {
        Log.d(TAG, "deleteSoundModel keyId=" + keyphraseId + " locale="
                + locale + " userHandle=" + userHandle);
        if (keyphraseId <= 0) {
            Log.w(TAG, "[deleteSoundModel] Keyphrase must have a valid ID");
            return false;
        }
        int status = SoundTrigger.STATUS_ERROR;
        unloadSoundModel(keyphraseId, locale, userHandle);
        boolean deleted = mDbHelper.deleteKeyphraseSoundModel(keyphraseId,
                userHandle, VoiceCommandListener.VOW_ENROLLMENT_BCP47_LOCALE);
        status = deleted ? SoundTrigger.STATUS_OK : SoundTrigger.STATUS_ERROR;
        if (status == SoundTrigger.STATUS_OK) {
            mSoundModel = null;
        }
        return status == SoundTrigger.STATUS_OK;
    }

    private boolean unloadSoundModel(int keyphraseId, String locale, int userHandle) {
        Log.d(TAG, "unloadSoundModel keyId=" + keyphraseId + " locale="
                + locale + " userHandle=" + userHandle);
        KeyphraseSoundModel soundModel = getSoundModel(keyphraseId, userHandle,
                locale);
        try {
            if (mStService != null && soundModel != null) {
                if (mStService.unloadSoundModel(new ParcelUuid(
                        soundModel.uuid)) != SoundTrigger.STATUS_OK) {
                    Log.d(TAG, "mStService.unloadSoundModel error");
                }
                return true;
            }
        } catch (RemoteException e) {
            Log.w(TAG, "mStService.unloadSoundModel RemoteException", e);
        }
        return false;
    }

    private void triggerCallback(int subAction, int confidence, Bundle bundle) {
        if (!sIsOwner) {
            Log.w(TAG, "not in Owner profile.");
            return;
        }
        if (mCallback != null) {
            mCallback.onVoiceWakeupDetected(subAction, confidence, bundle);
        }
    }

    private boolean loadModel(int keyphraseId, int userHandle, String locale) {
        int status = SoundTrigger.STATUS_ERROR;
        Log.d(TAG, "loadModel keyId=" + keyphraseId + " userHandle=" + userHandle
                + " locale=" + locale);
        mSoundModel = getSoundModel(keyphraseId, userHandle, locale);
        try {
            if (mStService != null && mSoundModel != null) {
                if ((status = mStService.loadKeyphraseSoundModel(mSoundModel))
                        != SoundTrigger.STATUS_OK) {
                    Log.d(TAG, "mStService.loadKeyphraseSoundModel error: " + status);
                }
            }
        } catch (RemoteException e) {
            Log.w(TAG, "mStService.loadKeyphraseSoundModel RemoteException", e);
        }
        return status == SoundTrigger.STATUS_OK;
    }

    private boolean isModelActive(KeyphraseSoundModel model) {
        boolean ret = false;
        if (mStService != null && model != null) {
            try {
                ret = mStService.isRecognitionActive(new ParcelUuid(model.uuid));
                Log.d(TAG, "isModelActive=" + ret);
                // Need refactory
                return true;
            } catch (RemoteException e) {
                Log.w(TAG, "mStService.isRecognitionActive RemoteException", e);
            }
        }
        return false;
    }

    public boolean enrollSoundModel(int trainingMode, int commandId,
            String patternPath, int userHandle, String locale) {
        boolean status = false;
        Log.d(TAG, "API enrollSoundModel trainingMode = " + trainingMode
                + ", commandId = " + commandId + ", patternPath = "
                + patternPath + ", user = " + userHandle);
        Keyphrase kp = new Keyphrase(KEYPHRASE_ID, RECOGNITION_MODES,
                VoiceCommandListener.VOW_ENROLLMENT_BCP47_LOCALE,
                VoiceCommandListener.VOW_ENROLLMENT_TEXT,
                new int[] { userHandle });
        UUID modelUuid = getUUID(trainingMode, commandId);
        byte[] data = getBytes(patternPath + commandId + ".dat");
        KeyphraseSoundModel soundModel = new KeyphraseSoundModel(modelUuid,
                null, data, new Keyphrase[] { kp });
        status = addOrUpdateSoundModel(soundModel, locale);
        if (!status) {
            Log.d(TAG, "[enrollSoundModel] Failed to enroll!!! model: "
                    + modelUuid + ", status =" + status);
        } else {
            Log.d(TAG, "[enrollSoundModel] Successfully = " + status
                    + ", model UUID = " + modelUuid + ", data.length = "
                    + data.length);
        }
        return status;
    }

    public boolean unEnrollSoundModel(int userHandle, String locale) {
        boolean status = false;
        Log.d(TAG, "API unEnrollSoundModel, user = " + userHandle);
        KeyphraseSoundModel soundModel = getSoundModel(
                KEYPHRASE_ID, userHandle, locale);
        if (soundModel == null) {
            Log.e(TAG, "[unEnrollSoundModel] Sound model not found!!!");
            return status;
        }
        stopRecognition(UserHandle.myUserId());
        status = deleteSoundModel(KEYPHRASE_ID, locale, userHandle);
        Log.d(TAG, "[unEnrollSoundModel] Successfully = " + status
                + ", model UUID=" + soundModel.uuid);
        if (!status) {
            Log.d(TAG, "[unEnrollSoundModel] Failed to un-enroll!!!");
        }
        return status;
    }

    public void setCurrentUserOnSwitch(int userId) {
        Log.d(TAG, "setCurrentUserOnSwitch=" + userId);
        if (userId == UserHandle.USER_SYSTEM) {
            sIsOwner = true;
            Log.d(TAG, "sIsModelActive=" + sIsModelActive);
            if (sIsModelActive) {
                mWorkerHandler.sendEmptyMessageDelayed(
                        WorkerHandler.MSG_RESTAET_RECOG, 1000);
                // restartRecognition();
            }
        } else {
            //sIsModelActive = (mSoundModel != null) && isModelActive(mSoundModel);
            Log.d(TAG, "sIsModelActive=" + sIsModelActive);
            if (sIsModelActive) {
                stopRecognition(UserHandle.myUserId());
                // Will be set as false in stopRecognition
                sIsModelActive = true;
            }
            sIsOwner = false;
        }
    }

    public void registerWakeupCallback(IVoiceWakeupCallback callback) {
        Log.d(TAG, "registerWakeupCallback");
        this.mCallback = callback;
    }

    public void unRegisterWakeupCallback() {
        Log.d(TAG, "unRegisterWakeupCallback");
        this.mCallback = null;
    }

    public void registerPackageUpdateCallback(IPackageUpdateCallback callback) {
        Log.d(TAG, "registerPackageUpdateCallback");
        this.mPackageCallback = callback;
    }

    public void unRegisterPackageUpdateCallback() {
        Log.d(TAG, "unRegisterPackageUpdateCallback");
        this.mPackageCallback = null;
    }

    public void registerVoiceModelUpdateCallback(IVoiceModelUpdateCallback callback) {
        Log.d(TAG, "registerVoiceModelUpdateCallback");
        this.mVoiceModelCallback = callback;
        mWorkerHandler.sendMessage(mWorkerHandler.obtainMessage(
                WorkerHandler.MSG_UPDATE_MODEL, VoiceModelCfg.VOICE_MODEL_PACKAGE_NAME));
    }

    public void unRegisterVoiceModelUpdateCallback() {
        Log.d(TAG, "unRegisterVoiceModelUpdateCallback");
        this.mVoiceModelCallback = null;
    }
    public boolean startRecognition(int myUserId, int coarseConfidence, int secondStageConfidence) {
        Log.d(TAG, "API startRecognition, myUserId=" + myUserId);
        if (mModuleProperties != null) {
            Log.d(TAG, "module properties:" + mModuleProperties.toString());
        }
        int status = SoundTrigger.STATUS_ERROR;
        if (mSoundModel != null) {// && !isModelActive(mSoundModel)) {
            KeyphraseRecognitionExtra[] recognitionExtra = new KeyphraseRecognitionExtra[1];
            recognitionExtra[0] = new KeyphraseRecognitionExtra(
                    mSoundModel.keyphrases[0].id,
                    SoundTrigger.RECOGNITION_MODE_VOICE_TRIGGER,
                    coarseConfidence,
                    new ConfidenceLevel[] { new ConfidenceLevel(myUserId,
                            secondStageConfidence) });
            mExtraBak = recognitionExtra;
            SoundTrigger.RecognitionConfig config = new SoundTrigger.RecognitionConfig(
                    true, false, recognitionExtra, null);
            try {
                if (mStService != null && mSoundModel != null) {
                    //status = mStService.startRecognitionForIntent(new ParcelUuid(mSoundModel.uuid),
                    //        pendIntent, config);
                    status = mBridgeService.startRecognition(KEYPHRASE_ID, mSoundModel,
                            mStListener, config);
                    Log.d(TAG, "startRecognition:" + status);
                    sIsModelActive = true;
                }
            } catch (RemoteException e) {
                Log.w(TAG, "mBridgeService.startRecognition RemoteException", e);
            }
        } else {
            Log.d(TAG, "mSoundModel is null=" + (mSoundModel == null));
        }
        return status == SoundTrigger.STATUS_OK;
    }

    public boolean stopRecognition(int myUserId) {
        Log.d(TAG, "API stopRecognition, myUserId=" + myUserId);
        int status = SoundTrigger.STATUS_ERROR;
        boolean ret = false;
        try {
            if (mStService != null && mSoundModel != null && isModelActive(mSoundModel)) {
                //status = mStService.stopRecognitionForIntent(new ParcelUuid(mSoundModel.uuid));
                status = mBridgeService.stopRecognition(KEYPHRASE_ID, mStListener);
                ret = status == SoundTrigger.STATUS_OK;
                if (ret) {
                    sIsModelActive = false;
                }
            } else {
                Log.d(TAG, "skip stop recognition");
            }
        } catch (RemoteException e) {
            Log.w(TAG, "mBridgeService.stopRecognition RemoteException", e);
        }
        return ret;
    }

    public boolean hasModelByLocale(int userHandle, String locale) {
        boolean ret = mDbHelper.hasVoiceModelByLocale(userHandle, locale);
        Log.d(TAG, "API hasModelByLocale:ret=" + ret + " userHandle="
                + userHandle + " locale=" + locale);
        return ret;
    }

    public boolean loadModel(int userHandle, String locale) {
        Log.d(TAG, "API loadModel: userHandle=" + userHandle + " locale=" + locale);
        return loadModel(KEYPHRASE_ID, userHandle, locale);
    }

    public void unloadModel(int userHandle, String locale) {
        Log.d(TAG, "API unloadModel: userHandle=" + userHandle + " locale=" + locale);
        unloadSoundModel(KEYPHRASE_ID, locale, userHandle);
    }

    public void addBackgroundWhitelist() {
        Log.d(TAG, "API addBackgroundWhitelist");
        addBackgroundWhitelist(PKG_NAME_WHITELIST);
    }

    private void registerListener() {
        PackageListener listener = new PackageListener();
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_PACKAGE_ADDED);
        filter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        filter.addAction(Intent.ACTION_PACKAGE_REPLACED);
        filter.addAction(Intent.ACTION_PACKAGE_CHANGED);
        filter.addAction(Intent.ACTION_PACKAGE_DATA_CLEARED);
        filter.addDataScheme("package");
        registerReceiver(mListener, filter);
        UserSwitchListener userListener = new UserSwitchListener();
        IntentFilter userFilter = new IntentFilter();
        userFilter.addAction(Intent.ACTION_USER_BACKGROUND);
        userFilter.addAction(Intent.ACTION_USER_FOREGROUND);
        userFilter.addAction(Intent.ACTION_USER_SWITCHED);
        registerReceiver(mUserListener, userFilter);
    }

    private void unregisterListener() {
        unregisterReceiver(mListener);
        unregisterReceiver(mUserListener);
    }

    private void addBackgroundWhitelist(String[] pkgNames) {
        if (pkgNames == null) {
            return;
        }
        IActivityManager am = ActivityManager.getService();
        PackageManager pm = this.getPackageManager();
        for (int i = 0; i < pkgNames.length; i++) {
            try {
                int uid = pm.getPackageUid(pkgNames[i], 0);
                if (uid > 0) {
                    Log.d(TAG, "add background whitelist package:" + pkgNames[i]);
                    am.backgroundWhitelistUid(uid);
                }
            } catch (PackageManager.NameNotFoundException e) {
                Log.d(TAG, "invalid package:" + pkgNames[i], e);
            } catch (RemoteException e) {
                Log.d(TAG, "add whitelist remote exception", e);
            }
        }
    }

    private void updateModelsIfNeed(String pkgName) {
        if (VoiceModelCfg.isAvailablePkg(this.getApplicationContext(), pkgName)) {
            Log.d(TAG, "updateModelsIfNeed:" + pkgName);
            ArrayList<VoiceModelInfo> list = new ArrayList<>();
            mModelCfg.parseModelInfosInPackage(pkgName, list);
            if (mModelCfg.updateModels(mDbHelper, list)) {
                if (mVoiceModelCallback != null) {
                    mVoiceModelCallback.onVoiceModelUpdate();
                } else {
                    Log.d(TAG, "mVoiceModelCallback is null");
                }
            }
        }
    }

    private void notifyPackageUpdate(String pkgName, String action) {
        if (mPackageCallback != null) {
            mPackageCallback.onPackageUpdateDetected(pkgName, action);
        }
    }

    private boolean isContains(String[] pkgNameWhitelist, String pkgName) {
        if (pkgNameWhitelist != null) {
            for (int i = 0; i < pkgNameWhitelist.length; i++) {
                if (pkgNameWhitelist[i].equals(pkgName)) {
                    return true;
                }
            }
        }
        return false;
    }

    class PackageListener extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            Uri uri = intent.getData();
            if (uri == null) {
                return;
            }
            String pkgName = uri.getSchemeSpecificPart();
            Log.d(TAG, "package: " + pkgName + " action:" + intent.getAction());
            if (!isContains(PKG_NAME_WHITELIST, pkgName)) {
                return;
            }
            if (Intent.ACTION_PACKAGE_ADDED.equals(intent.getAction())) {
                mWorkerHandler.sendMessage(mWorkerHandler.obtainMessage(
                        WorkerHandler.MSG_BACKGROUND_WHITELIST_ADD,
                        new String[] { pkgName }));
                mWorkerHandler.sendMessage(mWorkerHandler.obtainMessage(
                        WorkerHandler.MSG_UPDATE_MODEL, pkgName));
                notifyPackageUpdate(pkgName, intent.getAction());
            } else if (Intent.ACTION_PACKAGE_REMOVED.equals(intent.getAction())) {
                if (intent.getExtras().getBoolean(Intent.EXTRA_REPLACING)) {
                    // The existing package is updated. Will be handled with the
                    // following ACTION_PACKAGE_ADDED case.
                    Log.d(TAG,
                            "The existing package is updated, ACTION_PACKAGE_ADDED will handle");
                    return;
                }
                notifyPackageUpdate(pkgName, intent.getAction());
            } else if (Intent.ACTION_PACKAGE_REPLACED.equals(intent.getAction())) {
                mWorkerHandler.sendMessage(mWorkerHandler.obtainMessage(
                        WorkerHandler.MSG_UPDATE_MODEL, pkgName));
                notifyPackageUpdate(pkgName, intent.getAction());
            } else if (Intent.ACTION_PACKAGE_CHANGED.equals(intent.getAction())) {
                //Handle app disable then enable case.
                notifyPackageUpdate(pkgName, intent.getAction());
            } else if (Intent.ACTION_PACKAGE_DATA_CLEARED.equals(intent.getAction())) {
                notifyPackageUpdate(pkgName, intent.getAction());
            }
        }

    }

    class UserSwitchListener extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int userId = intent.getIntExtra(Intent.EXTRA_USER_HANDLE, -1);
            Log.d(TAG, "UserSwitch action: " + action + " userId:" + userId);
        }

    }

    class SoundTriggerListener extends IRecognitionStatusCallback.Stub {

        public SoundTriggerListener() {

        }

        @Override
        public void onKeyphraseDetected(KeyphraseRecognitionEvent event) {
            Log.d(TAG, "onKeyphraseDetected: " + event.toString());
            KeyphraseRecognitionExtra[] extras = ((KeyphraseRecognitionEvent) event).keyphraseExtras;
            if (extras.length > 0) {
                byte[] eventData = ((KeyphraseRecognitionEvent) event).data;
                boolean isDetected = extras[0].coarseConfidenceLevel != 0;
                long timeSecond = 0;
                long timeNanoSec = 0;
                long timeStamp = 0;
                long timeStampKel = 0;
                if (eventData != null && eventData.length > 8) {
                    isDetected = eventData[0] == 0;
                    if (!isDetected) {
                        // Restart quickly
                        mWorkerHandler
                                .sendEmptyMessage(WorkerHandler.MSG_RESTAET_RECOG);
                        mWorkerHandler.sendMessage(mWorkerHandler
                                .obtainMessage(WorkerHandler.MSG_ACTIVE_APP,
                                        PKG_NAME_WHITELIST));
                    }
                    timeSecond = eventData[1] & 0xFF
                            | (eventData[2] & 0xFF) << 8
                            | (eventData[3] & 0xFF) << 16
                            | (eventData[4] & 0xFF) << 24;
                    timeNanoSec = eventData[5] & 0xFF
                            | (eventData[6] & 0xFF) << 8
                            | (eventData[7] & 0xFF) << 16
                            | (eventData[8] & 0xFF) << 24;
                    long timeMs = timeSecond * 1000L + timeNanoSec / 1000000L;
                    timeStamp = System.currentTimeMillis();
                    timeStampKel = SystemClock.elapsedRealtimeNanos() / 1000000L;
                    timeStamp = timeStamp - (timeStampKel - timeMs);
                    Bundle bundle = new Bundle();
                    bundle.putLong(IVoiceWakeupCallback.KEY_TIME_STAMP,
                            timeStamp);
                    bundle.putParcelable(
                            IVoiceWakeupCallback.KEY_RECOGNITION_EVENT, event);
                    if (!isDetected) {
                        // Arrived
                        Log.d(TAG, "arrived");
                        triggerCallback(
                                VoiceCommandListener.ACTION_VOICE_TRIGGER_NOTIFY_ARRIVED,
                                extras[0].coarseConfidenceLevel, bundle);
                    } else {
                        // Detected
                        Log.d(TAG, "detected");
                        triggerCallback(
                                VoiceCommandListener.ACTION_VOICE_TRIGGER_NOTIFY_DETECTED,
                                extras[0].confidenceLevels[0].confidenceLevel,
                                bundle);
                    }
                } else {
                    Log.d(TAG, "Event data is not enough");
                }
            }
        }

        @Override
        public void onGenericSoundTriggerDetected(GenericRecognitionEvent event) {
            Log.d(TAG, "onGenericSoundTriggerDetected: " + event);
        }

        @Override
        public void onError(int status) {
            Log.d(TAG, "onError: " + status);
            triggerCallback(
                    VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_NOTIFY_AUDIOERROR,
                    0, null);
        }

        @Override
        public void onRecognitionPaused() {
            Log.d(TAG, "onRecognitionPaused");
        }

        @Override
        public void onRecognitionResumed() {
            Log.d(TAG, "onRecognitionResumed");
        }
    }

    public interface IVoiceWakeupCallback {
        public static final String KEY_TIME_STAMP = "time_stamp";
        public static final String KEY_RECOGNITION_EVENT = "recognition_event";
        void onVoiceWakeupDetected(int subAction, int confidence, Bundle bundle);
    }

    public interface IPackageUpdateCallback {
        void onPackageUpdateDetected(String pkgName, String action);
    }

    public interface IVoiceModelUpdateCallback {
        void onVoiceModelUpdate();
    }
}
