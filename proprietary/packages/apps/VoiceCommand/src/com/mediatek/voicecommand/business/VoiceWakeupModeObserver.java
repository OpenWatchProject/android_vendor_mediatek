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
package com.mediatek.voicecommand.business;

import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Receives call backs for voice wakeup mode changes to content.
 * 
 */
public class VoiceWakeupModeObserver extends ContentObserver {
    private static final String TAG = "VoiceWakeupModeObserver";

    private Context mContext;
    private Handler mMainHandler;
    private HandlerThread mHandlerThread;
    private Handler mVoiceWakeupModeHandler;
    private Handler mVoiceTriggerModeHandler;
    private ConfigurationManager mVoiceConfigMgr;

    /**
     * VoiceWakeupMode constructor.
     * 
     * @param context
     *            context
     * @param handler
     *            the handler to run onChange(boolean) on
     */
    public VoiceWakeupModeObserver(Context context, Handler handler) {
        super(handler);
        Log.i(TAG, "[VoiceWakeupModeObserver]new...");

        mContext = context;
        mMainHandler = handler;
        mVoiceConfigMgr = ConfigurationManager.getInstance(mContext);
        mHandlerThread = new HandlerThread("VoiceWakeupModeThread");
        mHandlerThread.start();
        mVoiceWakeupModeHandler = new VoiceWakeupModeHandler(mHandlerThread.getLooper());
        mVoiceTriggerModeHandler = mVoiceWakeupModeHandler;
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
        super.onChange(selfChange, uri);
        Log.i(TAG, "[onChange ]uri : " + uri);
        if (uri == null) {
            return;
        }
        if (uri.toString().contains(VoiceWakeupBusiness.VOICE_WAKEUP_MODE)) {
            if (mVoiceWakeupModeHandler.hasMessages(VoiceWakeupBusiness.MSG_GET_WAKEUP_MODE)) {
                mVoiceWakeupModeHandler.removeMessages(VoiceWakeupBusiness.MSG_GET_WAKEUP_MODE);
            }
            mVoiceWakeupModeHandler.sendEmptyMessage(VoiceWakeupBusiness.MSG_GET_WAKEUP_MODE);
        } else if (uri.toString().contains(VoiceTriggerBusiness.VOICE_TRIGGER_MODE)) {
            // Now only support dependency SID
        }
    }

    /**
     * Need to send some init message to Native when boot up.
     */
    public void bootUp() {
         mVoiceWakeupModeHandler.sendEmptyMessage(VoiceWakeupBusiness.MSG_GET_WAKEUP_INIT);
    }

    /**
     * Get the new voice wakeup mode handler.
     * 
     * @return the voice wakeup mode handler
     */
    public Handler getVoiceWakeupModeHandler() {
        return mVoiceWakeupModeHandler;
    }

    /**
     * A Handler allows you to send and process voice wake up Message and
     * Runnable objects associated with VoiceWakeupModeThread's MessageQueue.
     * 
     */
    private class VoiceWakeupModeHandler extends Handler {
        public VoiceWakeupModeHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            Log.d(TAG, "[handleMessage ]msg.what : " + msg.what);
            switch (msg.what) {
            case VoiceWakeupBusiness.MSG_GET_WAKEUP_INIT:
                sendInitToMainHandler();
                break;

            case VoiceWakeupBusiness.MSG_GET_WAKEUP_MODE:
                sendModeToMainHandler();
                break;

            case VoiceTriggerBusiness.MSG_GET_TRIGGER_INIT:
                sendTriggerInitToMainHandler();
                break;

            default:
                break;
            }
        }
    }

    private void sendTriggerInitToMainHandler() {
        Log.d(TAG,"[sendTriggerInitToMainHandler]...");
        if (!VoiceTriggerBusiness.isTriggerSupport(mContext)) {
            Log.i(TAG, "[sendTriggerInitToMainHandler]Voice Wakeup feature is off, return!");
            return;
        }

        if (mMainHandler.hasMessages(VoiceCommandListener.ACTION_VOICE_TRIGGER_INIT)) {
            mMainHandler.removeMessages(VoiceCommandListener.ACTION_VOICE_TRIGGER_INIT);
        }
        // Query database after remove the wake up msg of main handler
        int mode = VoiceTriggerBusiness.getTriggerMode(mContext);
        int cmdStatus = VoiceTriggerBusiness.getTriggerCmdStatus(mContext);
        String patternPath = mVoiceConfigMgr.getVoiceRecognitionPatternFilePath(mode);
        int[] commandIds = getCommandIdList(getDirectory(patternPath), ".dat");
        Log.d(TAG,
                "[sendTriggerInitToMainHandler]First send init commandIds: "
                + Arrays.toString(commandIds));

        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRIGGER_INIT;
        Bundle bundle = DataPackage.packageSendInfo(mode, cmdStatus, commandIds);
        message.mExtraData = bundle;

        Message msg = mMainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER;
        msg.obj = message;
        mMainHandler.sendMessage(msg);
    }

    private void sendInitToMainHandler() {
        Log.d(TAG,"[sendInitToMainHandler]...");
        if (!VoiceWakeupBusiness.isWakeupSupport(mContext)) {
            Log.i(TAG, "[sendInitToMainHandler]Voice Wakeup feature is off, return!");
            return;
        }

        if (mMainHandler.hasMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_INIT)) {
            mMainHandler.removeMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_INIT);
        }
        // Query database after remove the wake up msg of main handler
        int mode = VoiceWakeupBusiness.getWakeupMode(mContext);
        int cmdStatus = VoiceWakeupBusiness.getWakeupCmdStatus(mContext);
        String patternPath = mVoiceConfigMgr.getVoiceRecognitionPatternFilePath(mode);
        int[] commandIds = getCommandIdList(getDirectory(patternPath), ".dat");
        Log.d(TAG,
                "[sendInitToMainHandler]First send init commandIds: " + Arrays.toString(commandIds));

        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_INIT;
        Bundle bundle = DataPackage.packageSendInfo(mode, cmdStatus, commandIds);
        message.mExtraData = bundle;

        Message msg = mMainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        msg.obj = message;
        mMainHandler.sendMessage(msg);
    }

    private void sendModeToMainHandler() {
        Log.d(TAG, "[sendModeToMainHandler]...");
        if (mMainHandler.hasMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_MODE)) {
            mMainHandler.removeMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_MODE);
        }
        // Query database after remove the wake up msg of main handler
        int mode = VoiceWakeupBusiness.getWakeupMode(mContext);
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_MODE;
        Bundle bundle = DataPackage.packageSendInfo(mode);
        message.mExtraData = bundle;

        Message msg = mMainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        msg.obj = message;
        mMainHandler.sendMessage(msg);
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

    private int[] getCommandIdList(List<String> fileNameList, String end) {
        List<Integer> commandIdList = new ArrayList<Integer>();
        for (int i = 0; i < fileNameList.size(); i++) {
            String path = fileNameList.get(i);
            if (path == null || !path.contains(end)) {
                Log.d(TAG, "getCommandIdList continue, path: " + path);
                continue;
            } else {
                String subPath = path.substring(0, path.indexOf(end));
                try {
                    Integer commandId = Integer.valueOf(subPath);
                    commandIdList.add(commandId);
                } catch (NumberFormatException e) {
                    Log.e(TAG, "[getCommandIdList] error " + e.toString());
                }
            }
        }
        int[] commandIds = new int[commandIdList.size()];
        for (int i = 0; i < commandIdList.size(); i++) {
            commandIds[i] = commandIdList.get(i).intValue();
        }
        return commandIds;
    }
}