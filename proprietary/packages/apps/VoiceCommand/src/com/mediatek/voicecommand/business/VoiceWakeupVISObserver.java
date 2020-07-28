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
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.Settings;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.util.Log;

/**
 * Receives call backs for voice wakeup interaction service changes to content.
 *
 */
public class VoiceWakeupVISObserver extends ContentObserver {
    private static final String TAG = "VoiceWakeupVISObserver";
    private Context mContext;

    private HandlerThread mHandlerThread;
    private Handler mVoiceWakeupVISHandler;

    /**
     * VoiceWakeupVISObserver constructor.
     *
     * @param context
     *            context
     * @param handler
     *            the handler to run onChange(boolean) on
     */
    public VoiceWakeupVISObserver(Context context, Handler handler) {
        super(handler);
        Log.i(TAG, "[VoiceWakeupVISObserver]new...");

        mContext = context;
        mHandlerThread = new HandlerThread("VoiceWakeupVISThread");
        mHandlerThread.start();
        mVoiceWakeupVISHandler = new VoiceWakeupVISHandler(mHandlerThread.getLooper());
    }

    @Override
    public void onChange(boolean selfChange, Uri uri) {
        super.onChange(selfChange, uri);
        Log.i(TAG, "[onChange ]uri : " + uri);
        if (mVoiceWakeupVISHandler.hasMessages(VoiceWakeupBusiness.MSG_GET_WAKEUP_VIS)) {
            mVoiceWakeupVISHandler.removeMessages(VoiceWakeupBusiness.MSG_GET_WAKEUP_VIS);
        }
        mVoiceWakeupVISHandler.sendEmptyMessage(VoiceWakeupBusiness.MSG_GET_WAKEUP_VIS);
    }

    /**
     * A Handler allows you to send and process voice wake up Message and
     * Runnable objects associated with VoiceWakeupVISHandler's MessageQueue.
     */
    private class VoiceWakeupVISHandler extends Handler {
        public VoiceWakeupVISHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            Log.d(TAG, "[handleMessage ]msg.what : " + msg.what);
            switch (msg.what) {
            case VoiceWakeupBusiness.MSG_GET_WAKEUP_VIS:
                handleVISChanged();
                break;

            default:
                break;
            }
        }
    }

    /**
     * Handle voice wakeup interaction service change message.
     */
    private void handleVISChanged() {
        String interactionSerivce = Settings.Secure.getString(mContext.getContentResolver(),
                Settings.Secure.VOICE_INTERACTION_SERVICE);
        int cmdStatus = VoiceWakeupBusiness.getWakeupCmdStatus(mContext);
        Log.d(TAG, "[handleVISChanged]interactionSerivce : " + interactionSerivce
                + ", cmdStatus : " + cmdStatus);
        if ((VoiceWakeupBusiness.VOICE_WAKEUP_INTERACTION_SERVICE).equals(interactionSerivce)) {
            if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED) {
                VoiceWakeupBusiness.setWakeupCmdStatus(mContext,
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED);
            }
        } else {
            if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
                VoiceWakeupBusiness.setWakeupCmdStatus(mContext,
                    VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED);
            }
        }
    }
}
