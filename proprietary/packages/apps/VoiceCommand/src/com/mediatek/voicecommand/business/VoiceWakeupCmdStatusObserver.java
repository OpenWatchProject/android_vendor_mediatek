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
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

/**
 * Receives call backs for voice wakeup status changes to content.
 * 
 */
public class VoiceWakeupCmdStatusObserver extends ContentObserver {
    private static final String TAG = "VoiceWakeupCmdStatusObserver";

    private Context mContext;
    private Handler mMainHandler;
    private HandlerThread mHandlerThread;
    private Handler mVoiceWakeupCmdStatusHandler;

    /**
     * VoiceWakeupCmdStatus constructor.
     * 
     * @param context
     *            context
     * @param handler
     *            the handler to run onChange(boolean) on
     */
    public VoiceWakeupCmdStatusObserver(Context context, Handler handler) {
        super(handler);
        Log.i(TAG, "[VoiceWakeupCmdStatusObserver]new...");
        mContext = context;
        mMainHandler = handler;
        mHandlerThread = new HandlerThread("VoiceWakeupHandlerThread");
        mHandlerThread.start();
        mVoiceWakeupCmdStatusHandler = new VoiceWakeupCmdStatusHandler(mHandlerThread.getLooper());
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
        super.onChange(selfChange, uri);
        Log.i(TAG, "[onChange] uri : " + uri);
        if (uri == null) {
            return;
        }
        if (uri.toString().contains(VoiceWakeupBusiness.VOICE_WAKEUP_COMMAND_STATUS)) {
            if (mVoiceWakeupCmdStatusHandler
                    .hasMessages(VoiceWakeupBusiness.MSG_GET_WAKEUP_COMMAND_STATUS)) {
                mVoiceWakeupCmdStatusHandler
                        .removeMessages(VoiceWakeupBusiness.MSG_GET_WAKEUP_COMMAND_STATUS);
            }
            mVoiceWakeupCmdStatusHandler
                    .sendEmptyMessage(VoiceWakeupBusiness.MSG_GET_WAKEUP_COMMAND_STATUS);
        } else if (uri.toString().contains(VoiceTriggerBusiness.VOICE_TRIGGER_COMMAND_STATUS)) {
            if (mVoiceWakeupCmdStatusHandler
                    .hasMessages(VoiceTriggerBusiness.MSG_GET_TRIGGER_COMMAND_STATUS)) {
                mVoiceWakeupCmdStatusHandler
                        .removeMessages(VoiceTriggerBusiness.MSG_GET_TRIGGER_COMMAND_STATUS);
            }
            mVoiceWakeupCmdStatusHandler
                    .sendEmptyMessage(VoiceTriggerBusiness.MSG_GET_TRIGGER_COMMAND_STATUS);
        }
    }

    /**
     * A Handler allows you to send and process voice wake up Message and
     * Runnable objects associated with VoiceWakeupHandlerThread's MessageQueue.
     * 
     */
    private class VoiceWakeupCmdStatusHandler extends Handler {
        public VoiceWakeupCmdStatusHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage]msg.what=" + msg.what);
            super.handleMessage(msg);
            switch (msg.what) {
            case VoiceWakeupBusiness.MSG_GET_WAKEUP_COMMAND_STATUS:
                sendCmdStatusToMainHandler();
                break;

            case VoiceTriggerBusiness.MSG_GET_TRIGGER_COMMAND_STATUS:
                sendTriggerCmdStatusToMainHandler();
                break;
            default:
                break;
            }
        }
    }

    private void sendCmdStatusToMainHandler() {
        Log.d(TAG, "[sendCmdStatusToMainHandler]...");
        if (mMainHandler.hasMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_COMMAND_STATUS)) {
            mMainHandler.removeMessages(VoiceCommandListener.ACTION_VOICE_WAKEUP_COMMAND_STATUS);
        }
        // Query database after remove the wakeup msg of main handler
        int cmdStatus = VoiceWakeupBusiness.getWakeupCmdStatus(mContext);
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_COMMAND_STATUS;
        Bundle bundle = DataPackage.packageSendInfo(cmdStatus);
        message.mExtraData = bundle;

        Message msg = mMainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
        msg.obj = message;
        mMainHandler.sendMessage(msg);
    }

    private void sendTriggerCmdStatusToMainHandler() {
        Log.d(TAG, "[sendTriggerCmdStatusToMainHandler]...");
        if (mMainHandler.hasMessages(VoiceCommandListener.ACTION_VOICE_TRIGGER_COMMAND_STATUS)) {
            mMainHandler.removeMessages(VoiceCommandListener.ACTION_VOICE_TRIGGER_COMMAND_STATUS);
        }
        // Query database after remove the trigger msg of main handler
        int cmdStatus = VoiceTriggerBusiness.getTriggerCmdStatus(mContext);
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER;
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRIGGER_COMMAND_STATUS;
        Bundle bundle = DataPackage.packageSendInfo(cmdStatus);
        message.mExtraData = bundle;

        Message msg = mMainHandler.obtainMessage();
        msg.what = VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER;
        msg.obj = message;
        mMainHandler.sendMessage(msg);
    }

}