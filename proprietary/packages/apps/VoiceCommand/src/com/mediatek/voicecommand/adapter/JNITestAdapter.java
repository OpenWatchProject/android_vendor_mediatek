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
import com.mediatek.voicecommand.cfg.VoiceProcessInfo;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

import java.util.Arrays;
import java.util.Random;

public class JNITestAdapter implements IVoiceAdapter {
    private static final String TAG = "JNITestAdapter";

    private int mCurMode;
    private final ConfigurationManager mCfgMgr;
    private final int MODE_VOICE_UNKNOW = -1;

    private final int mDelaytime = 5000;

    private final IMessageDispatcher mUpDispatcher;

    // private VoiceProcessInfo curProcessInfo ;

    private final Handler curHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
                sendVoiceTrainingCommand(msg.arg1, (VoiceProcessInfo) msg.obj);
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
                sendVoiceUICommand(msg.arg1, (VoiceProcessInfo) msg.obj);
                break;
            case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
                sendVoiceContactsCommand(msg.arg1, (VoiceProcessInfo) msg.obj);
                break;
            default:
                break;
            }
        }

    };

    /**
     * Test adapter constructor.
     * 
     * @param dispatcher
     *            NativaDataManager instance
     * @param cfgmgr
     *            ConfigurationManager instance
     */
    public JNITestAdapter(IMessageDispatcher dispatcher, ConfigurationManager cfgmgr) {
        mCurMode = MODE_VOICE_UNKNOW;
        mCfgMgr = cfgmgr;
        mUpDispatcher = dispatcher;
    }

    private void sendVoiceUICommand(int curnumber, VoiceProcessInfo processinfo) {
        if (curnumber >= processinfo.mCommandIDList.size()) {
            return;
        }
        int commandid = processinfo.mCommandIDList.get(curnumber);

        VoiceMessage message = new VoiceMessage();
        // message.mPkgName = processinfo.mProcessName;
        message.mPkgName = processinfo.mFeatureName;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
        // msg.arg1 is the command id
        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, commandid, null);
        mUpDispatcher.dispatchMessageUp(message);
        Message m = curHandler.obtainMessage(VoiceCommandListener.ACTION_MAIN_VOICE_UI,
                ++curnumber, 0, processinfo);
        curHandler.sendMessageDelayed(m, mDelaytime);

    }

    private void sendVoiceContactsCommand(int curnumber, VoiceProcessInfo processinfo) {
        String[] command = { "resultWang", "测试王",
                "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz" };

        VoiceMessage message = new VoiceMessage();
        // message.mPkgName = processinfo.mProcessName;
        message.mPkgName = processinfo.mFeatureName;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;

        // if msg.arg1 is 0 notify speechDetected, if is 1 notify Contacts name.
        if (curnumber == 0) {
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_SPEECH_DETECTED;
            message.mExtraData = DataPackage.packageSuccessResult();
        } else if (curnumber == 1) {
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_CONTACTS_NOTIFY;
            message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, command, null);
        } else {
            return;
        }
        mUpDispatcher.dispatchMessageUp(message);

        Message m = curHandler.obtainMessage(VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS,
                ++curnumber, 0, processinfo);
        curHandler.sendMessage(m);
    }

    private void sendVoiceTrainingCommand(int curnumber, VoiceProcessInfo processinfo) {
        VoiceMessage message = new VoiceMessage();
        // message.mPkgName = processinfo.mProcessName;
        message.mPkgName = processinfo.mFeatureName;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY;

        int commandid = 0;
        if (curnumber < 5) {
            commandid = 1;
            curnumber++;
        }

        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, commandid, curnumber * 20);

        mUpDispatcher.dispatchMessageUp(message);
        if (commandid == 1) {
            Message m = curHandler.obtainMessage(VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING,
                    curnumber, 0, processinfo);
            curHandler.sendMessageDelayed(m, mDelaytime);
        }
    }

    @Override
    public int startVoiceUi(String modelpath, String patternpath, String processname, int pid,
            int languageid) {
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
        VoiceProcessInfo processinfo = mCfgMgr.getProcessInfo(processname);

        if (processinfo == null) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }

        int index = 0;

        Message m = curHandler.obtainMessage(VoiceCommandListener.ACTION_MAIN_VOICE_UI, index, 0,
                processinfo);
        curHandler.sendMessageDelayed(m, mDelaytime);
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoiceUi(String processname, int pid) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int startVoiceContacts(String processname, int pid, int screenOrientation,
            String modelpath, String contactsdbpath) {
        Log.i(TAG, "startVoiceContacts screenOrientation " + screenOrientation);
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS;
        VoiceProcessInfo processinfo = mCfgMgr.getProcessInfo(processname);

        if (processinfo == null) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }
        Message m = curHandler.obtainMessage(VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS, 0, 0,
                processinfo);
        // curHandler.sendMessageDelayed(m, mDelaytime);
        curHandler.sendMessage(m);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoiceContacts(String processname, int pid) {
        Log.i(TAG, "startVoiceContacts");
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int sendContactsName(String modelpath, String contactspath, String[] allContactsName) {
        Log.i(TAG, "sendVoiceContacts : " + Arrays.toString(allContactsName));
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public int sendContactsSelected(String selectedName) {
        Log.i(TAG, "sendContactsSelected : " + selectedName);
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public int sendContactsOrientation(int screenOrientation) {
        Log.i(TAG, "sendContactsOrientation : " + screenOrientation);
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public int sendContactsRecogEnable(int recognitionEnable) {
        Log.i(TAG, "sendContactsRecogEnable : " + recognitionEnable);
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public int sendContactsSearchCnt(int searchCnt) {
        Log.i(TAG, "sendContactsSearchCnt : " + searchCnt);
        int erroid = VoiceCommandListener.VOICE_NO_ERROR;
        return erroid;
    }

    @Override
    public void stopCurMode(String processname, int pid) {
    }

    @Override
    public int getNativeIntensity() {
        return new Random(100).nextInt(100);
    }

    @Override
    public boolean isNativePrepared() {
        return false;
    }

    private int mRecognitionCommandid = 0;

    @Override
    public int startVoicePwRecognition(String patternpath, String ubmpath,
            String ubmUpgradePath, String processname, int pid) {
        VoiceMessage message = new VoiceMessage();
        message.mPkgName = processname;
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_UI;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_UI_NOTIFY;
        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, mRecognitionCommandid++, null);
        if (mRecognitionCommandid > 3) {
            mRecognitionCommandid = 0;
        }
        mUpDispatcher.dispatchMessageUp(message);
        return 0;
    }

    @Override
    public int startVoiceTraining(String pwdpath, String patternpath,
            String featurepath, String umbpath, String ubmUpgradePath,
            int commandid, int[] commandMask, int trainingMode, int realTrainingMode,
            String wakeupInfo, String processname, int pid) {
        mCurMode = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
        VoiceProcessInfo processinfo = mCfgMgr.getProcessInfo(processname);

        if (processinfo == null) {
            return VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
        }
        Message m = curHandler.obtainMessage(VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING, 0, 0,
                processinfo);
        curHandler.sendMessageDelayed(m, mDelaytime);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int resetVoiceTraining(String pwdpath, String patternpath, String featurepath,
            int commandid) {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int modifyVoiceTraining(String pwdpath, String patternpath, String featurepath,
            int commandid) {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int finishVoiceTraining(String processname, int pid) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int continueVoiceTraining(String processname, int pid) {
        // TODO:
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int initVoiceTraining(boolean isForDemo) {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int pauseVoiceTraining(String processname, int pid) {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int setVoiceModelRetrain(String patternPath, String umbPath, String ubmUpgradePath,
                              int commandId, int trainingMode, String wakeupinfoPath){
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int setVoiceTrainingTimeout(int timeOutInMilliseconds) {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int setVoiceTrainingThreshold(int threshold) {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int getVoiceTrainingThreshold() {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int getVoiceTrainingRepeatCount() {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoicePwRecognition(String processname, int pid) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public int stopVoiceTraining(String processname, int pid) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;

    }

    @Override
    public int modifyPresetVoiceTraining(String patternPath, int commandId) {
        mCurMode = MODE_VOICE_UNKNOW;
        return VoiceCommandListener.VOICE_NO_ERROR;
    }


    @Override
    public int modifyVoiceTrainingKeyphrase(String umbPath, String ubmUpgradePath,
                              int commandId, int trainingMode) {
        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    @Override
    public void release() {

    }

    @Override
    public void setCurHeadsetMode(boolean isPlugin) {
    }

    @Override
    public int initVoiceWakeup(int mode, int cmdStatus, int[] cmdIds, String pattern, int mode1,
            String patternPath1, String passwordPath1, int mode2, String patternPath2,
            String passwordPath2, String ubmPath, String wakeupinfoPath) {
        return 0;
    }

    @Override
    public int initVoiceTrigger(int mode, int cmdStatus, int[] cmdIds, String pattern, int mode1,
            String patternPath1, String passwordPath1, int mode2, String patternPath2,
            String passwordPath2, String ubmPath, String ubmUpgradePath, String wakeupinfoPath) {
        return 0;
    }

    @Override
    public int sendVoiceWakeupMode(int mode, String ubmPath, String ubmUpgradePath) {
        return 0;
    }

    @Override
    public int sendVoiceWakeupCmdStatus(int cmdStatus) {
        return 0;
    }

    @Override
    public int startVoiceWakeup(String processname, int pid) {
        return 0;
    }

    @Override
    public int startVoiceTrigger(String processname, int pid) {
        return 0;
    }
}
