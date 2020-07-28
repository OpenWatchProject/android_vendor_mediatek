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
package com.mediatek.voicecommand.adapter;

import android.os.Handler;
import android.os.Message;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.business.VoiceTriggerBusiness;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.Integer;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class JNICommandAdapter implements IVoiceAdapter {
    private static final String TAG = "JNICommandAdapter";

    private long mNativeContext = 0;

    /*
     * Do not change these values without updating their counterparts!
     */
    // The mode currently supported
    private static final int MODE_VOICE_UNKNOW = -1;
    private static final int NATIVE_MODE_VOICE_UI = 1;
    private static final int NATIVE_MODE_VOICE_TRAINING = 2;
    private static final int NATIVE_MODE_VOICE_RECOGNITION = 4;
    private static final int NATIVE_MODE_VOICE_CONTACTS = 8;
    private static final int NATIVE_MODE_VOICE_WAKEUP = 16;

    // The return message notified from native counterpart @{
    private static final int NOTIFY_VOICE_ERROR = -1;
    private static final int NOTIFY_VOICE_UI = 0;
    private static final int NOTIFY_VOICE_TRAINING = 1;
    private static final int NOTIFY_VOICE_RECOGNITION = 2;
    private static final int NOTIFY_VOICE_CONTACTS = 3;
    private static final int NOTIFY_VOICE_WAKEUP = 4;
    private static final int NOTIFY_VOICE_TRAINING_PROGRESS = 5;
    private static final int NOTIFY_VOICE_TRAINING_PROGRESS_FINISHED = 6;
    private static final int NOTIFY_VOICE_TRIGGER_RETRAIN = 7;
    // voice_mode to native counterpart @{
    private static final int VOICE_NORMAL_MODE = 1;
    private static final int VOICE_HEADSET_MODE = 2;
    // @}

    // The detail error when native counterpart notify returns for error @{
    // private final int NOTIFY_ERROR_UI = 0;
    // private final int NOTIFY_ERROR_VOICE_TRAINING = 1;
    // private final int NOTIFY_ERROR_VOICE_RECOGNITION = 2;
    // @}

    // The detail information when native counterpart notify returns for voice
    // training
    private static final int NOTIFY_VOICE_TRAINING_FINISHED = 0;
    // private final int NOTIFY_VOICE_TRAINING_NEED_MORE_VOICE = 1;
    // private final int NOTIFY_VOICE_TRAINING_TOO_NOISY = 2;
    // private final int NOTIFY_VOICE_TRAINING_DIFF_PSWD = 3;
    private static final int NOTIFY_VOICE_TRAINING_EXIST_PSWD = 5;
    private static final int NOTIFY_VOICE_TRAINING_TIMEOUT = 6;
    private static final int NOTIFY_VOICE_TRAINING_OK_CONFIDENCE = 100;
    private static final int NOTIFY_VOICE_HEADSET_PLUG = 100;

    private static final int NOTIFY_VOICE_CONTACTS_COMMANDARRAY = 0;
    private static final int NOTIFY_VOICE_CONTACTS_SPEECHDETECTED = 1;

    private IMessageDispatcher mUpDispatcher;

    private FileOutputStream mPatternFileStream;
    private FileOutputStream mPasswordFileStream;
    private FileOutputStream mFeatureFileStream;

    private String mPatternFilePath;
    private String mPasswordFilePath;
    private String mFeatureFilePath;
    private String mNewPatternFilePath;
    private String mNewPasswordFilePath;
    private String mNewFeatureFilePath;

    private String mVoiceUiPatternPath;
    private String mVoiceUiModelPath;

    private String mModelpath;
    private String mContactsdbpath;

    private String mPatternPath;

    private ActiveProcess mCurTopActiveProcess;
    private ArrayList<ActiveProcess> mActiveProcessList = new ArrayList<ActiveProcess>();

    private ActiveProcess mCurRecogProcess;
    private ActiveProcess mCurTrainingProcess;
    private ActiveProcess mCurContactsProcess;
    private ActiveProcess mCurWakeupProcess;
    private ActiveProcess mCurVoiceTriggerProcess;

    private boolean mIsTrainingModify = false;
    private boolean mIsTrainingForDemo = true; //Reset value if Init may not be called.
    private boolean mIsTrainingModifyKeyphrase = false;

    private int mWakeupMode;
    private int mCommandId;
    private int mVoiceUiLanguageId;
    private int mCurMode;
    private int mHeadsetMode;
    private int mScreenOrientation;

    public JNICommandAdapter(IMessageDispatcher dispatcher) {
        Log.i(TAG, "[JNICommandAdapter]new...");

        mUpDispatcher = dispatcher;
        mCurMode = MODE_VOICE_UNKNOW;
        mHeadsetMode = VOICE_NORMAL_MODE;
        native_setup(new WeakReference<JNICommandAdapter>(this));
    }

    static {
        System.loadLibrary("voicerecognition_jni");
        try {
            native_init();

        } catch (NoSuchMethodException ex) {
            ex.printStackTrace();
        }
    }

    @Override
    public boolean isNativePrepared() {
        return mNativeContext == 0 ? false : true;
    }

    @Override
    public int startVoicePwRecognition(String patternpath, String ubmpath,
            String ubmUpgradePath, String processname, int pid) {
        Log.i(TAG, "[startVoicePwRecognition] patternpath:" + patternpath
                + ",ubmpath:" + ubmpath + ", ubmUpgradePath:" + ubmUpgradePath
                + ", processname:" + processname + ",pid" + pid);
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.w(TAG, "[startVoicePwRecognition] errorid:" + errorid + ",return!");
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW) {
            // in other mode , we need to notify apps
            Log.i(TAG, "[startVoicePWRecognition] stop current mode =" + mCurMode);
            stopTopProcess();
            // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        }

        try {
            Log.i(TAG, "[startVoicePWRecognition]setVoicePatternFile");
            setVoicePatternFile(patternpath);
            setVoiceUBMFile(ubmpath);
            setInputMode(mHeadsetMode);
            Log.i(TAG, "startVoicePWRecognition  startCaptureVoice.");
            startCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
            mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION;
            mCurRecogProcess = new ActiveProcess(processname, pid);
        } catch (IllegalStateException e) {
            mCurMode = MODE_VOICE_UNKNOW;
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[startVoicePWRecognition] Error " + e.getMessage());
        }

        // wait the notification from natvie and decide to do
        return errorid;
    }

    @Override
    public int stopVoicePwRecognition(String processname, int pid) {
        Log.i(TAG, "[stopVoicePwRecognition]processname:" + processname + ",pid" + pid);
        if (mCurRecogProcess == null || !mCurRecogProcess.mProcessName.equals(processname)
                || mCurRecogProcess.mPid != pid) {
            Log.w(TAG, "[stopVoicePwRecognition]error,return.");
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }

        return stopVoicePwRecognition();
    }

    @Override
    public int startVoiceTraining(String pwdpath, String patternpath, String featurepath,
            String umbpath, String ubmUpgradePath, int commandid, int[] commandMask,
            int trainingMode, int realTrainingMode, String wakeupinfoPath,
            String processname, int pid) {
        Log.i(TAG, "[startVoiceTraining]processname:" + processname + ",pid"
                + pid + ",pwdpath = " + pwdpath + ",patternpath = "
                + patternpath + ",featurepath = " + featurepath + ",umbpath = "
                + umbpath + ",ubmUpgradePath = " + ubmUpgradePath
                + ",commandid = " + commandid + ",trainingMode = "
                + trainingMode + ", realTrainingMode = " + realTrainingMode
                + ",wakeupinfoPath = " + wakeupinfoPath);
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.w(TAG, "[startVoiceTraining]errorid = " + errorid + ",return!");
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW) {
            Log.i(TAG, "[startVoiceTraining] stop current mode = " + mCurMode);
            stopTopProcess();
            // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        }
        deleteCommandFiles(pwdpath, patternpath, featurepath, commandMask);
        String filename = commandid + ".dat";
        if (resetFdStream(pwdpath + filename, patternpath + filename, featurepath + filename)) {
            try {
                Log.i(TAG, "[startVoiceTraining]setVoicePasswordFile.");
                if (mPasswordFileStream != null) {
                    setVoicePasswordFile(mPasswordFileStream.getFD(), 0, 0);
                }
                if (mPatternFileStream != null) {
                    setVoicePatternFile(mPatternFileStream.getFD(), 0, 0);
                }
                if (mFeatureFileStream != null) {
                    setVoiceFeatureFile(mFeatureFileStream.getFD(), 0, 0);
                }
                //Native will use this file to save retrain need data.
                setVoicePatternFile(patternpath);
                if (realTrainingMode != VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER) {
                    setVoiceUBMFile(umbpath);
                }
                if (realTrainingMode == VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER
                        && ubmUpgradePath != null) {
                    setUpgradeVoiceUBMFile(ubmUpgradePath);
                }
                setCommandId(commandid);
                setVoiceTrainingMode(trainingMode);
                setVoiceWakeupInfoPath(wakeupinfoPath);
                setInputMode(mHeadsetMode);
                Log.i(TAG, "[startVoiceTraining]startCaptureVoice.");
                startCaptureVoice(NATIVE_MODE_VOICE_TRAINING);
            } catch (Exception e) {
                Log.e(TAG, "[startVoiceTraining] Error " + e.getMessage());
                stopFdStream(true, false);
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            }
            mCurTrainingProcess = new ActiveProcess(processname, pid);
            mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
            mCommandId = commandid;
            mPatternPath = patternpath;
        } else {
            Log.w(TAG,
                    "[startVoiceTraining] Error because can't create the output stream,mCurMode = "
                            + mCurMode);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        }

        return errorid;
    }

    @Override
    public int resetVoiceTraining(String pwdpath, String patternpath, String featurepath,
            int commandid) {
        Log.i(TAG, "[resetVoiceTraining] pwdpath:" + pwdpath + ", patternpath:" + patternpath
                + ", featurepath:" + featurepath + ",commandid:" + commandid);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING
                && mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION) {
            String filename = commandid + ".dat";
            deleteFile(pwdpath + filename);
            deleteFile(patternpath + filename);
            deleteFile(featurepath + filename);
            //delete retrain file.
            deleteRetrainFiles(patternpath);
            try {
                resetVoiceWakeupCmd(commandid);
            } catch (IllegalStateException e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[resetVoiceTraining] Error:" + e.getMessage());
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            Log.w(TAG, "[resetVoiceTraining] Error because the Native in other mode,mCurMode = "
                    + mCurMode);
        }

        return errorid;
    }

    @Override
    public int modifyVoiceTraining(String pwdpath, String patternpath, String featurepath,
            int commandid) {
        Log.i(TAG, "[modifyVoiceTraining]pwdpath:" + pwdpath + ", patternpath:" + patternpath
                + ",featurepath:" + featurepath + ",commandid" + commandid);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING
                && mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION) {
            String filename = commandid + ".dat";
            String newfilename = "new" + commandid + ".dat";
            if (renameFile(pwdpath + filename, pwdpath + newfilename)
                    && renameFile(patternpath + filename, patternpath + newfilename)
                    && renameFile(featurepath + filename, featurepath + newfilename)) {
                mNewPasswordFilePath = pwdpath + newfilename;
                mNewPatternFilePath = patternpath + newfilename;
                mNewFeatureFilePath = featurepath + newfilename;
                mIsTrainingModify = true;
            } else {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
                Log.i(TAG, "[modifyVoiceTraining] Error because rename file failed " + "filename:"
                        + filename + ",newfilename:" + newfilename);
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            Log.w(TAG, "[modifyVoiceTraining]Error because the Native in other mode,mCurMode = "
                    + mCurMode);
        }

        return errorid;
    }

    private void resetTraining() {
        Log.d(TAG, "[resetTraining]mIsTrainingModify=" + mIsTrainingModify);
        if (mIsTrainingModify) {
            renameFile(mNewPasswordFilePath, mPasswordFilePath);
            renameFile(mNewFeatureFilePath, mFeatureFilePath);
            renameFile(mNewPatternFilePath, mPatternFilePath);
        }
        mPasswordFilePath = null;
        mFeatureFilePath = null;
        mPatternFilePath = null;
        mNewPasswordFilePath = null;
        mNewFeatureFilePath = null;
        mNewPatternFilePath = null;
        mIsTrainingModify = false;
        mIsTrainingModifyKeyphrase = false;
        mIsTrainingForDemo = true;
        mCommandId = -1;
        mPatternPath = null;
        mCurTrainingProcess = null;
        mCurMode = MODE_VOICE_UNKNOW;
    }

    @Override
    public int stopVoiceTraining(String processname, int pid) {
        Log.i(TAG, "[stopVoiceTraining]processname=" + processname + ",pid = " + pid);
        if (mCurTrainingProcess == null || !mCurTrainingProcess.mProcessName.equals(processname)
                || mCurTrainingProcess.mPid != pid) {
            if (mCurTrainingProcess != null) {
                Log.w(TAG, "[stopVoiceTraining]error,mCurTrainingProcess = " + mCurTrainingProcess
                        + ",curProcessName:" + mCurTrainingProcess.mProcessName + ",curPid = "
                        + mCurTrainingProcess.mPid);
            } else {
                Log.w(TAG, "[stopVoiceTraining]error,mCurTrainingProcess is null.");
            }
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }
        stopVoiceTraining(false);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int finishVoiceTraining(String processname, int pid) {
        Log.i(TAG, "[finishVoiceTraining]processname=" + processname + ",pid = " + pid);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (mCurTrainingProcess == null || !mCurTrainingProcess.mProcessName.equals(processname)
                || mCurTrainingProcess.mPid != pid) {
            Log.e(TAG, "[finishVoiceTraining]return illegal!");
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }
        try {
            startVoiceTraining();
        } catch (IllegalStateException e) {
            // Maybe the native training failed , but as discussed with
            // native guy ,Service and Apps don't need to do anything
            Log.e(TAG,
                    "[finishVoiceTraining]startVoiceTraining Error " + e.getMessage());
        }
        Log.i(TAG, "[finishVoiceTraining] native startVoiceTraining success");

        return errorid;
    }

    @Override
    public int continueVoiceTraining(String processname, int pid) {
        Log.i(TAG, "[continueVoiceTraining]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (mCurTrainingProcess == null) {
            Log.e(TAG, "[continueVoiceTraining]return illegal!");
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }

        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING) {

            try {
                continueVoiceTraining();
                Log.i(TAG, "[continueVoiceTraining] success");
            } catch (IllegalStateException e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[continueVoiceTraining] Error " + e.getMessage());
            }

        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            Log.i(TAG, "[continueVoiceTraining] Error because the Native in other mode " + mCurMode);
        }

        return errorid;
    }

    @Override
    public int initVoiceTraining(boolean isForDemo) {
        /*Only used in 2E2K*/
        Log.i(TAG, "[initVoiceTraining] isForDemo=" + isForDemo);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        try {
            mIsTrainingForDemo = isForDemo;
            //initVoiceTraining();
            //Log.i(TAG, "[initVoiceTraining] success");
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[initVoiceTraining] Error " + e.getMessage());
        }

        return errorid;
    }

    @Override
    public int pauseVoiceTraining(String processname, int pid) {
        Log.i(TAG, "[pauseVoiceTraining]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (mCurTrainingProcess == null) {
            Log.e(TAG, "[pauseVoiceTraining]return illegal!");
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }

        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING) {

            try {
                pauseVoiceTraining();
                Log.i(TAG, "[pauseVoiceTraining] success");
            } catch (IllegalStateException e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[pauseVoiceTraining] Error " + e.getMessage());
            }

        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            Log.i(TAG, "[pauseVoiceTraining] Error because the Native in other mode " + mCurMode);
        }

        return errorid;
    }

    @Override
    public int modifyPresetVoiceTraining(String patternPath, int commandId) {
        Log.i(TAG, "[modifyPresetVoiceTraining]commandId=" + commandId + ",patternPath = " + patternPath);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        mCommandId = commandId;
        mPatternPath = patternPath;

        try {
            resetVoiceWakeupCmd(mCommandId);
            setVoiceWakeupModel(mCommandId, mPatternPath);
            Log.i(TAG, "[modifyPresetVoiceTraining] mCommandId=" + mCommandId + ", mPatternPath="
                    + mPatternPath);
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[modifyPresetVoiceTraining] setVoiceWakeupModel Error " + e.getMessage());
        }
        mCommandId = -1;
        mPatternPath = null;

        return errorid;
    }

    @Override
    public int modifyVoiceTrainingKeyphrase(String umbPath, String ubmUpgradePath,
                              int commandId, int trainingMode) {
        Log.i(TAG, "[modifyVoiceTrainingKeyphrase], umbpath = "
                + umbPath + ",ubmUpgradePath = " + ubmUpgradePath
                + ",commandId = " + commandId + ",trainingMode = "
                + trainingMode);
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.w(TAG, "[modifyVoiceTrainingKeyphrase]errorid = " + errorid + ",return!");
            return errorid;
        }
        mIsTrainingModifyKeyphrase = true;

        try {
            setVoiceUBMFile(umbPath);
        } catch (IllegalStateException e) {
            Log.e(TAG, "[modifyVoiceTrainingKeyphrase] Error " + e.getMessage());
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        }
        return errorid;
    }

    @Override
    public int setVoiceModelRetrain(String patternPath, String umbPath, String ubmUpgradePath,
                              int commandId, int trainingMode, String wakeupinfoPath) {
        Log.i(TAG, "[setVoiceModelRetrain], patternpath = " + patternPath + ",umbpath = "
                + umbPath + ",ubmUpgradePath = " + ubmUpgradePath
                + ",commandId = " + commandId + ",trainingMode = "
                + trainingMode + ",wakeupinfoPath = " + wakeupinfoPath);
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.w(TAG, "[setVoiceModelRetrain]errorid = " + errorid + ",return!");
            return errorid;
        }
        String fileName = commandId + ".dat";
        String newFileNme = "new" + commandId + ".dat";
        if (renameFile(patternPath + fileName, patternPath + newFileNme)) {
            mNewPatternFilePath = patternPath + newFileNme;
        }
        if (createFdStream(patternPath + fileName)) {
            try {
                if (mPatternFileStream != null) {
                    setVoicePatternFile(mPatternFileStream.getFD(), 0, 0);
                }
                //Native will use this file to save retrain need data.
                setVoicePatternFile(patternPath);
                setVoiceUBMFile(umbPath);
                if (ubmUpgradePath != null) {
                    setUpgradeVoiceUBMFile(ubmUpgradePath);
                }
                setCommandId(commandId);
                setVoiceTrainingMode(trainingMode);
                setVoiceWakeupInfoPath(wakeupinfoPath);
                setInputMode(mHeadsetMode);
                setVoiceModelRetrain();
            } catch (Exception e) {
                Log.e(TAG, "[setVoiceModelRetrain] Error " + e.getMessage());
                stopFdStream(true, true);
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            }
            mPatternPath = patternPath;
        } else {
            Log.w(TAG, "[setVoiceModelRetrain] Error because can't create the output stream");
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        }
        return errorid;
    }

    @Override
    public int startVoiceUi(String modelpath, String patternpath, String processname, int pid,
            int languageid) {
        Log.i(TAG, "[startVoiceUi] processname = " + processname + ",pid =" + pid + ",modelpath = "
                + modelpath + ",patternpath = " + patternpath + ",languageid = " + languageid);
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.e(TAG, "[startVoiceUi] errorid = " + errorid + ",return!");
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW && mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_UI) {
            // Because the native is in training or recognition mode
            Log.i(TAG, "[startVoiceUi]: stop current mode =" + mCurMode);
            stopTopProcess();
            // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALIDACTION;
        } else if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_UI
                && mCurTopActiveProcess != null) {
            try {
                setActiveAP(mCurTopActiveProcess.mPid, false);
                stopCaptureVoice(NATIVE_MODE_VOICE_UI);
                mCurTopActiveProcess = null;
            } catch (IllegalStateException e) {
                Log.e(TAG, "[startVoiceUi] Error when stop capture voice first" + e.getMessage());
                // Do nothing because we need to continue to start the voice ui
            }
        }

        if (addActiveProcess(processname, pid)) {
            mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
            try {
                mVoiceUiPatternPath = patternpath;
                mVoiceUiModelPath = modelpath;
                mVoiceUiLanguageId = languageid;
                setInputMode(mHeadsetMode);
                setModeIFile(modelpath);
                setVoicePatternPath(patternpath);
                setActiveAP(pid, true);
                setActiveLanguage((0x01 << (languageid - 1)) & 0xFF);
                Log.i(TAG, "[startVoiceUi]  modelpath=" + modelpath + " patternpath=" + patternpath
                        + " languageid = " + ((0x01 << (languageid - 1)) & 0xFF));
                startCaptureVoice(NATIVE_MODE_VOICE_UI);
                Log.i(TAG, "[startVoiceUi] startCaptureVoice success");
            } catch (IllegalStateException e) {
                mCurMode = MODE_VOICE_UNKNOW;
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[startVoiceUi] Error " + e.getMessage());
            }
        }

        return errorid;
    }

    @Override
    public int stopVoiceUi(String processname, int pid) {
        Log.i(TAG, "[stopVoiceUi] processname = " + processname + ",pid =" + pid);
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.e(TAG, "[stopVoiceUi] errorid = " + errorid + ",return!");
            return errorid;
        }
        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_UI) {
            Log.i(TAG, "[stopVoiceUi] Error because the Native in other mode " + mCurMode);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
        } else {
            deleteActiveProcess(processname, pid);
            int size = mActiveProcessList.size();
            if (size > 0) {
                // Other process are waitting the command id
                // We don't support multi processes use the voice ui function
                // now, this case won't happen at this time
                mCurTopActiveProcess = mActiveProcessList.get(size - 1);
            } else {
                // No process started the voice ui
                mCurMode = MODE_VOICE_UNKNOW;
                mCurTopActiveProcess = null;
                try {
                    mVoiceUiPatternPath = null;
                    mVoiceUiModelPath = null;
                    mVoiceUiLanguageId = -1;
                    setActiveAP(pid, false);
                    stopCaptureVoice(NATIVE_MODE_VOICE_UI);
                    Log.i(TAG, "[stopVoiceUi] stopCaptureVoice success");
                } catch (IllegalStateException e) {
                    // Although exception happened ,we also think stopping voice
                    // ui success errorid =
                    // VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                    Log.e(TAG, "[stopVoiceUi] Error:" + e.getMessage());
                }
            }
        }
        return errorid;
    }

    @Override
    public int startVoiceContacts(String processname, int pid, int screenOrientation,
            String modelpath, String contactsdbpath) {
        Log.i(TAG, "[startVoiceContacts] screenOrientation " + screenOrientation + " modelpath="
                + modelpath + ", contactsdbpath=" + contactsdbpath + ",processname=" + processname
                + ",pid = " + pid);
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            Log.e(TAG, "[startVoiceContacts] errorid = " + errorid + ",return!");
            return errorid;
        }

        if (mCurMode != MODE_VOICE_UNKNOW) {
            // in other mode , we need to notify apps
            Log.i(TAG, "[startVoiceContacts] stop current mode =" + mCurMode);
            stopTopProcess();
        }

        try {
            setContactProModePath(modelpath, contactsdbpath);
            mScreenOrientation = screenOrientation;
            mModelpath = modelpath;
            mContactsdbpath = contactsdbpath;
            setInputMode(mHeadsetMode);
            setScreenOrientation(screenOrientation);
            startCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
            Log.i(TAG, "[startVoiceContacts] success");
        } catch (IllegalStateException e) {
            mCurMode = MODE_VOICE_UNKNOW;
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[startVoiceContacts Error] " + e.getMessage());
        }
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
        mCurContactsProcess = new ActiveProcess(processname, pid);
        // wait the notification from natvie and decide to do
        return errorid;
    }

    @Override
    public int stopVoiceContacts(String processname, int pid) {
        Log.i(TAG, "[stopVoiceContacts] processname : " + processname + ",pid = " + pid);
        if (mCurContactsProcess == null || !mCurContactsProcess.mProcessName.equals(processname)
                || mCurContactsProcess.mPid != pid) {
            Log.w(TAG, "[stopVoiceContacts] error,return!");
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }

        return stopVoiceContacts();
    }

    @Override
    public int sendContactsName(String modelpath, String contactsdbpath, String[] allContactsName) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        try {
            Log.i(TAG, "[sendContactsName] modelpath=" + modelpath + ", contactsdbpath="
                    + contactsdbpath + ", contactscount=" + allContactsName.length);
            setContactProModePath(modelpath, contactsdbpath);
            setContactName(allContactsName);
            Log.i(TAG, "[sendContactsName] success");
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[sendVoiceContacts] Error " + e.getMessage());
        }
        return errorid;
    }

    @Override
    public int sendContactsSelected(String selectedName) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[sendContactsSelected] selectedName : " + selectedName);
        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            try {
                setAPSelectedRlt(selectedName);
                Log.i(TAG, "[sendContactsSelected] success");
            } catch (IllegalStateException e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[sendContactsSelected] Error " + e.getMessage());
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            Log.i(TAG, "[sendContactsSelected] Error because the Native in other mode " + mCurMode);
        }
        return errorid;
    }

    @Override
    public int sendContactsSearchCnt(int searchCnt) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[sendContactsSearchCnt] searchCnt : " + searchCnt);
        try {
            setSearchRltCnts(searchCnt);
            Log.i(TAG, "[sendContactsSearchCnt]success");
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[sendContactsSearchCnt] Error " + e.getMessage());
        }
        return errorid;
    }

    @Override
    public int sendContactsOrientation(int screenOrientation) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[sendContactsOrientation]screenOrientation : " + screenOrientation);
        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            try {
                setScreenOrientation(screenOrientation);
                Log.i(TAG, "[sendContactsOrientation] success");
            } catch (IllegalStateException e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[sendContactsOrientation] Error " + e.getMessage());
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            Log.i(TAG, "[sendContactsOrientation] Error because the Native in other mode "
                    + mCurMode);
        }
        return errorid;
    }

    @Override
    public int sendContactsRecogEnable(int recognitionEnable) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[sendContactsRecogEnable] recognitionEnable : " + recognitionEnable);
        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            try {
                voiceRecognitionEnable(NATIVE_MODE_VOICE_CONTACTS, recognitionEnable);
                Log.i(TAG, "[sendContactsRecogEnable] success");
            } catch (IllegalStateException e) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[sendContactsRecogEnable] Error " + e.getMessage());
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            Log.i(TAG, "[sendContactsRecogEnable] Error because the Native in other mode "
                    + mCurMode);
        }
        return errorid;
    }

    @Override
    public int initVoiceWakeup(int mode, int cmdStatus, int[] cmdIds, String patternPath,
            int mode1, String patternPath1, String passwordPath1, int mode2, String patternPath2,
            String passwordPath2, String ubmPath, String wakeupinfoPath) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        try {
            Log.i(TAG, "[initVoiceWakeup] mode=" + mode + ", cmdStatus=" + cmdStatus + ", cmdIds="
                    + Arrays.toString(cmdIds) + ", patternPath=" + patternPath + ", mode1=" + mode1
                    + ", patternPath1=" + patternPath1 + ", passwordPath1=" + passwordPath1
                    + ", mode2=" + mode2 + ", patternPath2=" + patternPath2 + ", passwordPath2="
                    + passwordPath2 + ", ubmPath=" + ubmPath + ", wakeupinfoPath=" + wakeupinfoPath);
            mWakeupMode = mode;
            setVoiceWakeupMode(mode);
            setVoiceWakeupPatternPath(mode1, patternPath1);
            setVoiceWakeupPWPath(mode1, passwordPath1);
            setVoiceWakeupPatternPath(mode2, patternPath2);
            setVoiceWakeupPWPath(mode2, passwordPath2);
            setVoiceUBMFile(ubmPath);
            for (int i = 0; i < cmdIds.length; i++) {
                setVoiceWakeupModel(cmdIds[i], patternPath);
            }
            setVoiceWakeupInfoPath(wakeupinfoPath);
            // No need to send VOW command status to MTK VoiceCommand Native.
            // voiceRecognitionEnable(NATIVE_MODE_VOICE_WAKEUP, cmdStatus);
            Log.i(TAG, "[initVoiceWakeup] success");
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[initVoiceWakeup] Error:" + e.getMessage());
        }
        return errorid;
    }

    @Override
    public int sendVoiceWakeupMode(int mode, String ubmPath, String ubmUpgradePath) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        try {
            Log.i(TAG, "[sendVoiceWakeupMode] mode=" + mode + ", ubmPath="
                    + ubmPath + ", ubmUpgradePath=" + ubmUpgradePath);
            mWakeupMode = mode;
            setVoiceWakeupMode(mode);
            setVoiceUBMFile(ubmPath);
            Log.i(TAG, "[sendVoiceWakeupMode] success");
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[sendVoiceWakeupMode] Error:" + e.getMessage());
        }
        return errorid;
    }

    @Override
    public int sendVoiceWakeupCmdStatus(int cmdStatus) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        try {
            Log.i(TAG, "[sendVoiceWakeupCmdStatus] cmdStatus=" + cmdStatus);
            // No need to send VOW command status to MTK VoiceCommand Native.
            // voiceRecognitionEnable(NATIVE_MODE_VOICE_WAKEUP, cmdStatus);
            Log.i(TAG, "[sendVoiceWakeupCmdStatus] success");
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[sendVoiceWakeupCmdStatus] Error: " + e.getMessage());
        }
        return errorid;
    }

    @Override
    public int initVoiceTrigger(int mode, int cmdStatus, int[] cmdIds, String patternPath,
            int mode1, String patternPath1, String passwordPath1, int mode2, String patternPath2,
            String passwordPath2, String ubmPath, String ubmUpgradePath, String wakeupinfoPath) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        try {
            Log.i(TAG, "[initVoiceTrigger] mode=" + mode + ", cmdStatus=" + cmdStatus + ", cmdIds="
                  + Arrays.toString(cmdIds) + ", patternPath=" + patternPath + ", mode1=" + mode1
                  + ", patternPath1=" + patternPath1 + ", passwordPath1=" + passwordPath1
                  + ", mode2=" + mode2 + ", patternPath2=" + patternPath2 + ", passwordPath2="
                  + passwordPath2 + ", ubmPath=" + ubmPath + ", ubmUpgradePath=" + ubmUpgradePath
                  + ", wakeupinfoPath=" + wakeupinfoPath);
            mWakeupMode = mode;
            setVoiceWakeupMode(mode);
            setVoiceUBMFile(ubmPath);
            if (ubmUpgradePath != null) {
                setUpgradeVoiceUBMFile(ubmUpgradePath);
            }
/* below items may not be needed.
            setVoiceWakeupPatternPath(mode1, patternPath1);
            setVoiceWakeupPWPath(mode1, passwordPath1);
            setVoiceWakeupPatternPath(mode2, patternPath2);
            setVoiceWakeupPWPath(mode2, passwordPath2);
            for (int i = 0; i < cmdIds.length; i++) {
                setVoiceWakeupModel(cmdIds[i], patternPath);
            }
*/
            setVoiceWakeupInfoPath(wakeupinfoPath);
            // No need to send VOW command status to MTK VoiceCommand Native.
            // voiceRecognitionEnable(NATIVE_MODE_VOICE_WAKEUP, cmdStatus);
            Log.i(TAG, "[initVoiceTrigger] success");
        } catch (IllegalStateException e) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            Log.e(TAG, "[initVoiceTrigger] Error:" + e.getMessage());
        }
        return errorid;
    }


    @Override
    public int startVoiceWakeup(String processname, int pid) {
        Log.i(TAG, "[startVoiceWakeup]processname = " + processname + ",pid = " + pid);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        mCurWakeupProcess = new ActiveProcess(processname, pid);
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;

        return errorid;
    }

    @Override
    public int startVoiceTrigger(String processname, int pid) {
        Log.i(TAG, "[startVoiceTrigger]processname = " + processname + ",pid = " + pid);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        mCurVoiceTriggerProcess = new ActiveProcess(processname, pid);
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER;

        return errorid;
    }

    @Override
    public int getNativeIntensity() {
        int intersity = 0;
        if (mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING
                || mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION
                || mCurMode == VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            try {
                intersity = getVoiceIntensity();
                // bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
                // intersity);

            } catch (IllegalStateException e) {
                mCurMode = MODE_VOICE_UNKNOW;
                Log.e(TAG, "[getVoiceIntensity]Error:" + e.getMessage());
            }
        }
        Log.i(TAG, "[getNativeIntensity]intersity: " + intersity + ",mCurMode = " + mCurMode);

        return intersity;
    }

    @Override
    public void stopCurMode(String processname, int pid) {
        Log.i(TAG, "[stopCurMode]mCurMode = " + mCurMode + ",processname = " + processname
                + ",pid = " + pid);
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            stopVoiceUi(processname, pid);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
            stopVoicePwRecognition(processname, pid);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            stopVoiceTraining(processname, pid);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            stopVoiceContacts(processname, pid);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP:
            stopVoiceWakeup(processname, pid);
            break;

        default:
            break;
        }
    }

    @Override
    public void setCurHeadsetMode(boolean isPlugin) {
        Log.i(TAG, "[setCurHeadsetMode]mCurMode = " + mCurMode + ",isPlugin = " + isPlugin);
        if (isPlugin) {
            Log.i(TAG, "[setCurHeadsetMode]handleHeadSetPlugEvent in");
            mHeadsetMode = VOICE_HEADSET_MODE;
        } else {
            Log.i(TAG, "[setCurHeadsetMode]handleHeadSetPlugEvent out");
            mHeadsetMode = VOICE_NORMAL_MODE;
        }

        if (mCurMode == MODE_VOICE_UNKNOW) {
            Log.i(TAG, "[setCurHeadsetMode]mCurMode = MODE_VOICE_UNKNOW,return.");
            return;
        }

        Message message = new Message();
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            message.what = NOTIFY_VOICE_UI;
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
            message.what = NOTIFY_VOICE_RECOGNITION;
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            message.what = NOTIFY_VOICE_TRAINING;
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            message.what = NOTIFY_VOICE_CONTACTS;
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP:
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER:
            message.what = NOTIFY_VOICE_WAKEUP;
            break;

        default:
            break;
        }
        message.arg1 = NOTIFY_VOICE_HEADSET_PLUG;
        curHandler.sendMessage(message);
    }

    @Override
    public void release() {
        Log.i(TAG, "[release]mCurMode = " + mCurMode);
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            stopCaptureVoice(NATIVE_MODE_VOICE_UI);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
            stopCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            stopCaptureVoice(NATIVE_MODE_VOICE_TRAINING);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            stopCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
            break;

        default:
            break;
        }

        _release();
    }

    private final Handler curHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "[handleMessage]msg.what = " + msg.what);

            switch (msg.what) {
            case NOTIFY_VOICE_ERROR:
                handleNotifyVoiceError(msg);
                break;

            case NOTIFY_VOICE_UI:
                handleNotifyVoiceUi(msg);
                break;

            case NOTIFY_VOICE_TRAINING:
                handleNotifyVoiceTraining(msg);
                break;

            case NOTIFY_VOICE_TRAINING_PROGRESS:
                handleNotifyVoiceTrainingProgress(msg);
                break;

            case NOTIFY_VOICE_TRAINING_PROGRESS_FINISHED:
                handleNotifyVoiceTrainingFinish(msg);
                break;

            case NOTIFY_VOICE_TRIGGER_RETRAIN:
                handleNotifyVoiceRetrain(msg);
                break;

            case NOTIFY_VOICE_RECOGNITION:
                handleNotifyVoiceRecognition(msg);
                break;

            case NOTIFY_VOICE_CONTACTS:
                handleNotifyVoiceContacts(msg);
                break;

            case NOTIFY_VOICE_WAKEUP:
                handleNotifyVoiceWakeup(msg);
                break;

            default:
                break;
            }
        }
    };

    private void handleNotifyVoiceError(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceError]mCurMode = " + mCurMode + ",msg.arg1 = " + msg.arg1);
        // Currently we just notify something wrong in service

        if (mCurMode == MODE_VOICE_UNKNOW) {
            // Do nothing because app didn't request any business
            Log.i(TAG, "[handleNotifyVoiceError] mCurMode =  MODE_VOICE_UNKNOW");
        } else {
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = mCurMode;
            switch (mCurMode) {
            case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
                if (mCurTopActiveProcess != null) {
                    message.mPkgName = mCurTopActiveProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
                    stopVoiceUi(mCurTopActiveProcess.mProcessName, mCurTopActiveProcess.mPid);
                }
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
                if (mCurTrainingProcess != null) {
                    message.mPkgName = mCurTrainingProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY;
                    stopVoiceTraining(false);
                }
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
                if (mCurRecogProcess != null) {
                    message.mPkgName = mCurRecogProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_RECOGNITION_NOTIFY;
                    stopVoicePwRecognition();
                }
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
                if (mCurContactsProcess != null) {
                    message.mPkgName = mCurContactsProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NOTIFY;
                    stopVoiceContacts();
                }
                break;

            default:
                break;
            }

            // handle wakeup error from native
            if (msg.arg1 == NOTIFY_VOICE_WAKEUP) {
                message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
                if (mCurWakeupProcess != null) {
                    message.mPkgName = mCurWakeupProcess.mProcessName;
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_NOTIFY;
                    stopVoiceWakeup(mCurWakeupProcess.mProcessName, mCurWakeupProcess.mPid);
                }
            }

            message.mExtraData = DataPackage
                    .packageErrorResult(VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE);
            mUpDispatcher.dispatchMessageUp(message);
        }
    }

    private void handleNotifyVoiceUi(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceError]mCurTopActiveProcess = " + mCurTopActiveProcess
                + ",msg.arg1 = " + msg.arg1);
        if (mCurTopActiveProcess != null) {
            if (msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                String processname = mCurTopActiveProcess.mProcessName;
                int pid = mCurTopActiveProcess.mPid;
                deleteActiveProcess(processname, pid);
                try {
                    setActiveAP(pid, false);
                    stopCaptureVoice(NATIVE_MODE_VOICE_UI);
                    mCurTopActiveProcess = null;
                    Log.i(TAG, "[handleNotifyVoiceUi] Headset stopCaptureVoice success");
                } catch (IllegalStateException e) {
                    Log.e(TAG, "[handleNotifyVoiceUi]stop capture voice Error" + e.getMessage());
                }
                startVoiceUi(mVoiceUiModelPath, mVoiceUiPatternPath, processname, pid,
                        mVoiceUiLanguageId);
            } else {
                VoiceMessage message = new VoiceMessage();
                message.mPkgName = mCurTopActiveProcess.mProcessName;
                message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
                message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
                // msg.arg1 is the command id
                message.mExtraData = DataPackage.packageResultInfo(
                        VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg2, null);
                if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                    stopVoiceUi(mCurTopActiveProcess.mProcessName, mCurTopActiveProcess.mPid);
                }
            }
        }
    }

    private void handleNotifyVoiceTraining(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceTraining]mCurTrainingProcess = " + mCurTrainingProcess
                + ",msg.arg1 = " + msg.arg1 + ",msg.arg2 = " + msg.arg2
                + ",msg.arg3 = " + msg.obj);
        if (mCurTrainingProcess != null) {
            VoiceMessage message = new VoiceMessage();
            int snrValue = 0;
            //When handle headset plugin, the msg.obj is null, so need add the filter.
            if (msg.obj != null && (Integer) msg.obj != null) {
                snrValue = ((Integer) msg.obj).intValue();
            }
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY;
            message.mPkgName = mCurTrainingProcess.mProcessName;
            if (msg.arg1 == NOTIFY_VOICE_TRAINING_FINISHED) {
                msg.arg2 = NOTIFY_VOICE_TRAINING_OK_CONFIDENCE;
                // Because the capture is finished ,we had stop capture and need
                // start
                // training
                Log.i(TAG, "[handleNotifyVoiceTraining]stop and start voice training.");
                stopVoiceTraining(true);
            } else if (msg.arg1 == NOTIFY_VOICE_TRAINING_EXIST_PSWD
                    || msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                Log.i(TAG, "[handleNotifyVoiceTraining]stop voice training.");
                stopVoiceTraining(false);
                // mCurTrainingProcess = null;
                // mCurMode = MODE_VOICE_UNKNOW;
            } else if (msg.arg1 == NOTIFY_VOICE_TRAINING_TIMEOUT) {
                Log.i(TAG, "[handleNotifyVoiceTraining] timeout case.");
                //stopVoiceTraining(false);
            }
            message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg1, msg.arg2,
                    snrValue);
            if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                Log.e(TAG,
                        "[handleNotifyVoiceTraining]dispatchMessageUp error,stop voice training.");
                stopVoiceTraining(false);
                // mCurTrainingProcess = null;
                // mCurMode = MODE_VOICE_UNKNOW;
            }
        }
    }

    private void handleNotifyVoiceTrainingProgress(Message msg) {
        int errorId = VoiceCommandListener.VOICE_NO_ERROR;
        Log.d(TAG, "[handleNotifyVoiceTrainingProgress]mCurTrainingProcess = "
            + mCurTrainingProcess + ",msg.arg1 = " + msg.arg1 + ",msg.arg2 = " + msg.arg2);
        if (mCurTrainingProcess != null) {
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY_PROGRESS;
            message.mPkgName = mCurTrainingProcess.mProcessName;

            if (VoiceCommandListener.VOICE_NO_ERROR == errorId) {
                message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg1, msg.arg2);
            } else {
                message.mExtraData = DataPackage.packageErrorResult(errorId);
            }

            if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                Log.e(TAG,
                        "[handleNotifyVoiceTraining]dispatchMessageUp error,stop voice training.");
                ///TODO: check if need to stop
                //stopVoiceTraining(false);
                // mCurTrainingProcess = null;
                // mCurMode = MODE_VOICE_UNKNOW;
            }
        }
    }

    private void handleNotifyVoiceTrainingFinish(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceTrainingFinish]mCurTrainingProcess = "
            + mCurTrainingProcess + ",msg.arg1 = " + msg.arg1 + ",msg.arg2 = " + msg.arg2);
        int errorId = VoiceCommandListener.VOICE_NO_ERROR;
        if (mCurTrainingProcess != null) {
            try {
                if (mIsTrainingModify) {
                    deleteFile(mNewPasswordFilePath);
                    deleteFile(mNewPatternFilePath);
                    deleteFile(mNewFeatureFilePath);
                    resetVoiceWakeupCmd(mCommandId);
                }
                setVoiceWakeupModel(mCommandId, mPatternPath);
                Log.i(TAG, "[handleNotifyVoiceTrainingFinish] mCommandId=" + mCommandId
                        + ", mPatternPath=" + mPatternPath);
            } catch (IllegalStateException e) {
                errorId = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[handleNotifyVoiceTrainingFinish] setVoiceWakeupModel Error "
                        + e.getMessage());
            }
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_ENROLL_START;
            message.mPkgName = mCurTrainingProcess.mProcessName;

            if (VoiceCommandListener.VOICE_NO_ERROR == errorId) {
                message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg1, msg.arg2);
            } else {
                message.mExtraData = DataPackage.packageErrorResult(errorId);
            }

            if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                Log.e(TAG,
                    "[handleNotifyVoiceTrainingFinish]dispatchMessageUp error,stop training.");
                ///TODO: check if need to stop
                //stopVoiceTraining(false);
                // mCurTrainingProcess = null;
                // mCurMode = MODE_VOICE_UNKNOW;
            }

            mPasswordFilePath = null;
            mFeatureFilePath = null;
            mPatternFilePath = null;
            mNewPasswordFilePath = null;
            mNewFeatureFilePath = null;
            mNewPatternFilePath = null;
            mIsTrainingModify = false;
            mIsTrainingModifyKeyphrase = false;
            mIsTrainingForDemo = true;
            mCommandId = -1;
            mPatternPath = null;
            mCurTrainingProcess = null;
            mCurMode = MODE_VOICE_UNKNOW;
        }
    }

    private void handleNotifyVoiceRetrain(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceRetrain]mCurTrainingProcess = "
                + mCurTrainingProcess + ",msg.arg1 = " + msg.arg1 + ",msg.arg2 = " + msg.arg2);
        //Sync with native, 0: success, -1: fail.
        stopFdStream(mPatternFileStream);
        if (msg.arg1 == 0) {
            deleteFile(mNewPatternFilePath);
        } else {
            deleteFile(mPatternFilePath);
            renameFile(mNewPatternFilePath, mPatternFilePath);
        }
        mPatternFilePath = null;
        mNewPatternFilePath = null;

        //Send to VoiceTriggerBusiness to handle.
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER;
        message.mSubAction = VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_NOTIFY_RETRAIN;
        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg1, msg.arg2);
        mUpDispatcher.dispatchMessageUp(message);
    }

    private void handleNotifyVoiceRecognition(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceRecognition]mCurRecogProcess = " + mCurRecogProcess
                + ",msg.arg1 = " + msg.arg1);
        if (mCurRecogProcess != null) {
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_RECOGNITION_NOTIFY;
            message.mPkgName = mCurRecogProcess.mProcessName;
            try {
                Log.i(TAG, "[handleNotifyVoiceRecognition]stopCaptureVoice.");
                stopCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
                if (msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                    message.mExtraData = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg1, 0);
                } else {
                    Log.i(TAG, "[handleNotifyVoiceRecognition]startVoicePWRecognition.");
                    RecognitionResult result = startVoicePWRecognition();
                    Log.i(TAG, "handleNotifyVoiceRecognition msgid=" + result.msgid
                            + " voicecmdid=" + result.voicecmdid);
                    message.mExtraData = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, result.msgid,
                            result.voicecmdid);
                }

            } catch (IllegalStateException ex) {
                // Althoungh error happened ,we also think the training has been
                // finished , so do nothing just notify app training ok
                message.mExtraData = DataPackage
                        .packageErrorResult(VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE);
                Log.e(TAG,
                        "[handleNotifyVoiceRecognition] Error " + mCurMode + "exception = "
                                + ex.getMessage());
            }
            mUpDispatcher.dispatchMessageUp(message);
            mCurMode = MODE_VOICE_UNKNOW;
        }
    }

    private void handleNotifyVoiceContacts(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceContacts]mCurContactsProcess = " + mCurContactsProcess
                + ",msg.arg1 = " + msg.arg1);
        if (mCurContactsProcess != null) {
            VoiceMessage message = new VoiceMessage();
            String processname = mCurContactsProcess.mProcessName;
            int pid = mCurContactsProcess.mPid;

            if (msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                try {
                    stopCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
                    mCurContactsProcess = null;
                    Log.i(TAG, "[handleNotifyVoiceContacts] Headset stopCaptureVoice success");
                } catch (IllegalStateException e) {
                    Log.e(TAG,
                            "[handleNotifyVoiceContacts] stopcapturevoice Error" + e.getMessage());
                }
                startVoiceContacts(processname, pid, mScreenOrientation, mModelpath,
                        mContactsdbpath);
            } else {
                message.mPkgName = processname;
                message.pid = pid;
                message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
                // msg.obj is the command contacts name array
                String[] commandArray = (String[]) msg.obj;
                if (msg.arg2 == NOTIFY_VOICE_CONTACTS_SPEECHDETECTED) {
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_SPEECH_DETECTED;
                    message.mExtraData = DataPackage.packageSuccessResult();
                    Log.d(TAG, "handleNotifyVoiceContacts speech detected");
                } else if (msg.arg2 == NOTIFY_VOICE_CONTACTS_COMMANDARRAY) {
                    message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NOTIFY;
                    Log.d(TAG,
                            "[handleNotifyVoiceContacts] commandArray"
                                    + Arrays.toString(commandArray));
                    message.mExtraData = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, commandArray, null);
                }

                if (mUpDispatcher.dispatchMessageUp(message) != VoiceCommandListener.VOICE_NO_ERROR) {
                    Log.e(TAG, "[handleNotifyVoiceContacts]dispatchMessageUp Error");
                    stopVoiceContacts(message.mPkgName, message.pid);
                }
            }
        }
    }

    private void handleNotifyVoiceWakeup(Message msg) {
        Log.d(TAG, "[handleNotifyVoiceWakeup]mCurWakeupProcess = " + mCurWakeupProcess
                + ",msg.arg1 = " + msg.arg1);
        if (mCurWakeupProcess != null) {
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_NOTIFY;
            message.mPkgName = mCurWakeupProcess.mProcessName;

            if (msg.arg1 == NOTIFY_VOICE_HEADSET_PLUG) {
                try {
                    setInputMode(mHeadsetMode);
                    Log.i(TAG, "[handleNotifyVoiceWakeup]mHeadsetMode=" + mHeadsetMode);
                } catch (IllegalStateException e) {
                    Log.e(TAG, "[handleNotifyVoiceWakeup] Headset Error" + e.getMessage());
                }
            } else {
                // msg.arg1 is the command id, mWakeupMode is wakeup mode
                message.mExtraData = DataPackage.packageResultInfo(
                        VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, msg.arg1, mWakeupMode);
                mUpDispatcher.dispatchMessageUp(message);
                Log.i(TAG, "[handleNotifyVoiceWakeup] commandid:" + msg.arg1 + ", mWakeupMode:"
                        + mWakeupMode);
            }
        }
    }

    /*
     * Called from native code when an interesting event happens. This method
     * just uses the EventHandler system to post the event back to the main app
     * thread. We use a weak reference to the original VoiceRecognition object
     * so that the native code is safe from the object disappearing from
     * underneath it. (This is the cookie passed to native_setup().)
     */
    private static void postEventFromNative(Object jniCommandAdapter_ref, int what, int arg1,
            int arg2, Object obj) {

        JNICommandAdapter adapter = (JNICommandAdapter) ((WeakReference<?>) jniCommandAdapter_ref)
                .get();
        if (adapter == null) {
            Log.w(TAG, "[postEventFromNative]adapter is null,return.");
            return;
        }
        Log.i(TAG, "[postEventFromNative]Message from native what=" + what + ",arg1=" + arg1
                + ",arg2=" + arg2);
        if (adapter.curHandler != null) {
            Object newObject = null;
            // If handle Notify VoiceContacts copy String[] from native
            if (what == NOTIFY_VOICE_CONTACTS) {
                String[] commandArray = null;
                String[] tempArray = (String[]) obj;
                if (tempArray != null) {
                    int length = tempArray.length;
                    commandArray = new String[length];
                    System.arraycopy(tempArray, 0, commandArray, 0, length);
                } else {
                    commandArray = new String[0];
                }
                newObject = commandArray;
            }
            if (what == NOTIFY_VOICE_TRAINING) {
                Integer snrVal = null;
                char[] numChar = null;
                String[] tempArray = (String[]) obj;
                if (tempArray != null) {
                    int length = tempArray.length;
                    numChar = tempArray[0].toCharArray();
                    //Log.i(TAG, "[postEventFromNative]tempArray[0]= " + tempArray[0]);
                    if (length > 0) {
                        snrVal = new Integer((int) numChar[0]);
                    } else {
                        snrVal = new Integer(0);
                    }
                } else {
                    snrVal = new Integer(0);
                    Log.i(TAG, "[postEventFromNative] Obj array is empty");
                }
                newObject = snrVal;
            }

            // Send the message to the handler to deal with it later
            Message m = adapter.curHandler.obtainMessage(what, arg1, arg2, newObject);
            adapter.curHandler.sendMessage(m);
        }
    }

    private boolean createFdStream(String patternfile) {
        Log.d(TAG, "[createFdStream] ,patternfile = " + patternfile);
        if ((mPatternFileStream = createFdStream(mPatternFileStream, patternfile)) == null) {
            return false;
        }
        mPatternFilePath = patternfile;

        return true;
    }

    private FileOutputStream createFdStream(FileOutputStream out, String path) {
        Log.d(TAG, "[createFdStream] path =  " + path);
        FileOutputStream outs = null;
        ConfigurationManager.makeDirForFile(path);
        try {
            outs = new FileOutputStream(path);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "[createFdStream] Error e =  " + e);
        }
        return outs;
    }

    /*
     * Close the stream if used before and create a new file output stream
     * 
     * @param out
     * 
     * @param path
     * 
     * @return
     */
    private FileOutputStream resetFdStream(FileOutputStream out, String path) {
        Log.d(TAG, "[resetFdStream] path =  " + path);
        stopFdStream(out);
        FileOutputStream outs = null;
        ConfigurationManager.makeDirForFile(path);

        try {
            outs = new FileOutputStream(path);
        } catch (FileNotFoundException e) {
            Log.e(TAG, "[resetFdStream] Error e =  " + e);
        }
        return outs;
    }

    private boolean resetFdStream(String pwdfile, String patternfile, String featurefile) {
        Log.d(TAG, "[resetFdStream] pwdfile =  " + pwdfile + ",patternfile = " + patternfile
                + ",featurefile = " + featurefile);
        if ((mPasswordFileStream = resetFdStream(mPasswordFileStream, pwdfile)) == null
                || (mPatternFileStream = resetFdStream(mPatternFileStream, patternfile)) == null
                || (mFeatureFileStream = resetFdStream(mFeatureFileStream, featurefile)) == null) {
            return false;
        }

        mPasswordFilePath = pwdfile;
        mPatternFilePath = patternfile;
        mFeatureFilePath = featurefile;

        return true;
    }

    /*
     * Service need to mantain the output stream for the native
     * 
     * @param outs
     * 
     * @return
     */
    private void stopFdStream(FileOutputStream outs) {
        Log.d(TAG, "[stopFdStream]...");
        if (outs != null) {
            try {
                outs.close();

            } catch (IOException e) {
                Log.e(TAG, "[stopFdStream] Error e =  " + e);
                outs = null;
                e.printStackTrace();
            }
        }
    }

    private void stopFdStream(boolean isdeletefile, boolean isOnlyPatternFile) {
        Log.d(TAG, "[stopFdStream]isdeletefile = " + isdeletefile
                + ", isOnlyPatternFile = " + isOnlyPatternFile);
        if (!isOnlyPatternFile) {
            stopFdStream(mPasswordFileStream);
            stopFdStream(mPatternFileStream);
            stopFdStream(mFeatureFileStream);

            if (isdeletefile) {
                deleteFile(mPasswordFilePath);
                deleteFile(mPatternFilePath);
                deleteFile(mFeatureFilePath);
            }
        } else {
            stopFdStream(mPatternFileStream);
            if (isdeletefile) {
                deleteFile(mPatternFilePath);
            }
        }
    }

    private boolean deleteFile(String path) {
        Log.d(TAG, "[deleteFile]path = " + path);
        boolean isDeleted = false;
        try {
            File file = new File(path);
            if (file.isFile() && file.exists()) {
                isDeleted = file.delete();
            }
        } catch (NullPointerException ex) {
            Log.e(TAG, "[deleteFile] ex: " + ex);
        }

        return isDeleted;
    }

    private boolean renameFile(String path1, String path2) {
        Log.d(TAG, "[renameFile]path1 = " + path1 + ",path2 = " + path2);
        ConfigurationManager.makeDirForFile(path1);
        ConfigurationManager.makeDirForFile(path2);
        try {
            File file1 = new File(path1);
            File file2 = new File(path2);
            if (file1.isFile() && file1.exists()) {
                return file1.renameTo(file2);
            }
        } catch (NullPointerException ex) {
            Log.e(TAG, "[renameFile] ex: " + ex);
        }
        return false;
    }

    private void deleteCommandFiles(String pwdpath, String patternpath, String featurepath,
            int[] commandMask) {
        Log.d(TAG, "[deleteCommandFiles]pwdpath = " + pwdpath + ",patternpath = " + patternpath
                + ",featurepath = " + featurepath);
        if (pwdpath == null || patternpath == null || featurepath == null || commandMask == null
                || commandMask.length != 2) {
            Log.e(TAG, "[deleteCommandFiles] error ");
            return;
        }

        for (int i = 0; i < commandMask[1]; i++) {
            if ((commandMask[0] >> i & 1) == 0) {
                String filename = i + ".dat";
                deleteFile(pwdpath + filename);
                deleteFile(patternpath + filename);
                deleteFile(featurepath + filename);
            }
        }
        //delete retrain file.
        deleteRetrainFiles(patternpath);
    }

    /**
     * delete retrain file when use want to train themselves.
     * Because there are save some other files for retrain, need reset.
     * @param patternpath
     */
    private void deleteRetrainFiles(String patternpath) {
        List<String> fileNameList = getDirectory(patternpath);
        for (int i = 0; i < fileNameList.size(); i++) {
            String path = fileNameList.get(i);
            if (path != null && !path.contains(".dat")) {
                deleteFile(patternpath + path);
            }
        }
    }

    private List<String> getDirectory(String path) {
        List<String> fileNameList = new ArrayList<String>();
        if (path == null) {
            return fileNameList;
        }
        File file = new File(path);
        File flist[] = file.listFiles();
        if (flist == null || flist.length == 0) {
            return fileNameList;
        }
        for (File f : flist) {
            if (f.isDirectory()) {
                getDirectory(f.getPath());
            } else {
                String name = f.getName();
                fileNameList.add(name);
            }
        }
        return fileNameList;
    }
    /*
     * used for initialize variable state of after training
     * 
     * @param isfinished: whether is finish success
     */
    private void stopVoiceTraining(boolean isfinished) {
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        Log.d(TAG, "[stopVoiceTraining]errorid = " + errorid + ",isfinished = " + isfinished);
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return;
        }
        try {
            stopCaptureVoice(NATIVE_MODE_VOICE_TRAINING);
            Log.i(TAG, "[stopVoiceTraining] stopCaptureVoice success");
        } catch (IllegalStateException ex) {
            // Althoungh error happened ,we also think the training has been
            // finished , so do nothing just notify app training ok
            Log.e(TAG,
                    "[stopVoiceTraining] mCurMode = " + mCurMode + ",exception = "
                            + ex.getMessage());
        }

        stopFdStream(!isfinished, false);
        if (!isfinished) {
            resetTraining();
        }
    }



    /**
     * indicate to modify voice training confidence threshold.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    public int setVoiceTrainingThreshold(int threshold) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        Log.d(TAG, "[setVoiceTrainingThreshold]...");
        try {
            setTrainingThreshold(threshold);
        } catch (Exception e) {
            Log.e(TAG, "[setVoiceTrainingThreshold] Error " + e.getMessage());
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        }
        return errorid;
    }

    /**
     * indicate to modify voice training timeout default value.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    public int setVoiceTrainingTimeout(int timeOutInMilliseconds) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        Log.d(TAG, "[setVoiceTrainingTimeout]...");
        try {
            setUtteranceTrainingTimeout(timeOutInMilliseconds);
        } catch (Exception e) {
            Log.e(TAG, "[setVoiceTrainingTimeout] Error " + e.getMessage());
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        }
        return errorid;
    }

    /**
     * Get value of voice training repetition count.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    public int getVoiceTrainingRepeatCount() {
        int trainingRepeatCount  = 0;
        try {
            trainingRepeatCount = getTrainigTimes();
            // bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
            // trainingRepeatCount);
        } catch (IllegalStateException e) {
            mCurMode = MODE_VOICE_UNKNOW;
            Log.e(TAG, "[getVoiceTrainingRepeatCount]Error:" + e.getMessage() +
                 ",mCurMode = " + mCurMode);
        }
        Log.i(TAG, "[getVoiceTrainingRepeatCount]trainingRepeatCount: " + trainingRepeatCount);

        return trainingRepeatCount;
    }

    /**
     * Get value of voice training confidence threshold.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    public int getVoiceTrainingThreshold() {
        int trainingThreshold = 0;
        try {
            trainingThreshold = getTrainingThreshold();
            // bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO,
            // trainingThreshold);

        } catch (IllegalStateException e) {
            Log.e(TAG, "[trainingThreshold]Error:" + e.getMessage());
        }
        Log.i(TAG, "[getVoiceTrainingThreshold]trainingThreshold: "
                + trainingThreshold);

        return trainingThreshold;
    }

    private int stopVoicePwRecognition() {
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        Log.d(TAG, "[stopVoicePwRecognition]errorid = " + errorid + ",mCurMode = " + mCurMode);
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }
        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
        } else {
            try {
                stopCaptureVoice(NATIVE_MODE_VOICE_RECOGNITION);
                Log.i(TAG, "[stopVoicePwRecognition] stopCaptureVoice success");
            } catch (IllegalStateException e) {
                // Although exception happened ,we also think
                // stopVoicePWRecognition success
                // errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                Log.e(TAG, "[stopVoicePwRecognition] Error " + e.getMessage());
            }
            mCurRecogProcess = null;
            mCurMode = MODE_VOICE_UNKNOW;
        }

        return errorid;
    }

    private int stopVoiceWakeup(String processname, int pid) {
        Log.d(TAG, "[stopVoiceWakeup]processname = " + processname + ",pid = " + pid);
        if (mCurWakeupProcess == null || !mCurWakeupProcess.mProcessName.equals(processname)
                || mCurWakeupProcess.mPid != pid) {
            Log.e(TAG, "[stopVoiceWakeup]reyurn illgal.");
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        mCurWakeupProcess = null;
        mCurMode = MODE_VOICE_UNKNOW;
        return errorid;
    }

    private int stopVoiceContacts() {
        int errorid = isNativePrepared() ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        Log.d(TAG, "[stopVoiceContacts]errorid = " + errorid + ",mCurMode = " + mCurMode);
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            return errorid;
        }
        if (mCurMode != VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
        } else {
            try {
                stopCaptureVoice(NATIVE_MODE_VOICE_CONTACTS);
                Log.i(TAG, "[stopVoiceContacts] stopCaptureVoice success");
            } catch (IllegalStateException e) {
                Log.e(TAG, "[stopVoiceContacts] Error " + e.getMessage());
            }
            mCurContactsProcess = null;
            mCurMode = MODE_VOICE_UNKNOW;
        }

        return errorid;
    }

    private boolean addActiveProcess(String processname, int pid) {
        Log.d(TAG, "[addActiveProcess]processname = " + processname + ",pid = " + pid);
        if (mCurTopActiveProcess != null && mCurTopActiveProcess.mProcessName.equals(processname)
                && mCurTopActiveProcess.mPid == pid) {
            Log.d(TAG, "[addActiveProcess]return false.");
            return false;
        }

        int size = mActiveProcessList.size() - 1;

        ActiveProcess activeProcess = null;

        for (int i = 0; i < size; i++) {

            activeProcess = mActiveProcessList.get(i);

            if (activeProcess.mProcessName.equals(processname)) {
                mActiveProcessList.remove(activeProcess);
                if (activeProcess.mPid != pid) {
                    // Will this case happen?
                    // Means Apps died but service didn't know the event
                    activeProcess = null;
                }
                break;
            }
        }

        if (activeProcess == null) {
            activeProcess = new ActiveProcess(processname, pid);
        }

        mCurTopActiveProcess = activeProcess;
        mActiveProcessList.add(activeProcess);

        return true;
    }

    private void stopTopProcess() {
        Log.d(TAG, "[stopTopProcess]mCurMode = " + mCurMode);
        switch (mCurMode) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            if (mCurTopActiveProcess != null) {
                stopVoiceUi(mCurTopActiveProcess.mProcessName, mCurTopActiveProcess.mPid);
            }
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
            stopVoicePwRecognition();
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            stopVoiceTraining(false);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            if (mCurContactsProcess != null) {
                stopVoiceContacts();
            }
            break;

        default:
            break;
        }
    }

    private void deleteActiveProcess(String processname, int pid) {
        Log.d(TAG, "[deleteActiveProcess]processname = " + processname + ",pid = " + pid);
        int size = mActiveProcessList.size();
        for (int i = (size - 1); i >= 0; i--) {
            ActiveProcess activeProcess = mActiveProcessList.get(i);
            if (activeProcess.mProcessName.equals(processname) && activeProcess.mPid == pid) {
                mActiveProcessList.remove(activeProcess);
            }
        }
    }

    private class ActiveProcess {
        ActiveProcess(String processname, int pid) {
            mProcessName = processname;
            mPid = pid;
        }

        String mProcessName;
        int mPid;
    }

    private native void setVoicePasswordFile(FileDescriptor fd, long offset, long length)
            throws IllegalStateException, IllegalArgumentException;

    private native void setVoicePatternFile(FileDescriptor fd, long offset, long length)
            throws IllegalStateException, IllegalArgumentException;

    private native void setVoicePatternFile(String path) throws IllegalStateException;

    private native void setVoicePatternPath(String path) throws IllegalStateException;

    private native void setVoiceFeatureFile(FileDescriptor fd, long offset, long length)
            throws IllegalStateException, IllegalArgumentException;

    private native void setVoiceUBMFile(String path) throws IllegalStateException;

    private native void setUpgradeVoiceUBMFile(String path) throws IllegalStateException;

    private native void setModeIFile(String path) throws IllegalStateException;

    private native void setActiveAP(int apid, boolean isEnalbe) throws IllegalStateException;

    private native void setCommandId(int id) throws IllegalStateException;

    private native void setVoiceTrainingMode(int mode) throws IllegalStateException;

    private native int getVoiceIntensity() throws IllegalStateException;

    private native void startCaptureVoice(int mode) throws IllegalStateException;

    private native void stopCaptureVoice(int mode) throws IllegalStateException;

    private native void startVoiceTraining() throws IllegalStateException;

    private native void continueVoiceTraining() throws IllegalStateException;

    private native RecognitionResult startVoicePWRecognition() throws IllegalStateException;

    private native void setActiveLanguage(int id) throws IllegalStateException;

    private static final native void native_init() throws NoSuchMethodException;

    private final native void native_setup(Object voicerecognition_this) throws RuntimeException;

    private final native void setInputMode(int input_mode) throws IllegalStateException;

    private native void setContactProModePath(String modelpath, String contactsdbpath)
            throws IllegalStateException;

    private native void setContactName(String[] allContactsName) throws IllegalStateException;

    private native void setAPSelectedRlt(String selectedName) throws IllegalStateException;

    private native void setSearchRltCnts(int searchCnts) throws IllegalStateException;

    private native void setScreenOrientation(int screenOrientation) throws IllegalStateException;

    private native void setVoiceWakeupInfoPath(String path) throws IllegalStateException;

    private native void setVoiceWakeupPatternPath(int mode, String path)
            throws IllegalStateException;

    private native void setVoiceWakeupPWPath(int mode, String path) throws IllegalStateException;

    private native void setVoiceWakeupMode(int mode) throws IllegalStateException;

    private native void setVoiceWakeupModel(int cmdID, String path) throws IllegalStateException;

    private native void resetVoiceWakeupCmd(int cmdID) throws IllegalStateException;

    private native void voiceRecognitionEnable(int mode, int enable) throws IllegalStateException;

    private native void pauseVoiceTraining() throws IllegalStateException;

    private native int getTrainigTimes() throws IllegalStateException;

    private native void setUtteranceTrainingTimeout(int msec) throws IllegalStateException;

    private native void setTrainingThreshold(int threshold) throws IllegalStateException;

    private native int getTrainingThreshold() throws IllegalStateException;

    private native void setVoiceModelRetrain() throws IllegalStateException;

    private final native void native_finalize();

    private final native void _release();
}
