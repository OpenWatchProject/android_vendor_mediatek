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

import android.app.KeyguardManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;

import com.mediatek.voicecommand.util.Log;

/**
 * Receive intents sent by Screen.
 *
 */
public class ScreenBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "ScreenBroadcastReceiver";

    private Context mContext;
    private Handler mHandler;
    private KeyguardManager mKeyguard;

    /**
     * ScreenBroadcastReceiver constructor.
     *
     * @param context
     *            context
     * @param handler
     *            the handler to run onReceive(Context, Intent) on
     */
    public ScreenBroadcastReceiver(Context context, Handler handler) {
        Log.i(TAG, "[ScreenBroadcastReceiver]new...");
        mContext = context;
        mHandler = handler;
        mKeyguard = (KeyguardManager) mContext.getSystemService(Context.KEYGUARD_SERVICE);
        if (mKeyguard != null) {
            boolean isSecure = mKeyguard.isKeyguardSecure();
            boolean isLocked = mKeyguard.isKeyguardLocked();
            Log.d(TAG, "isKeyguardSecure=" + isSecure
                    + " isKeyguardLocked=" + isLocked);
            if (isSecure && isLocked) {
                Log.d(TAG, "keyguard is secure and locked");
            } else {
                sendUnlockMsg();
            }
        } else {
            Log.d(TAG, "keyguard manager is null");
        }
    }

    /**
     * Called when the BroadcastReceiver is receiving an Intent broadcast.
     *
     * @param context
     *            the Context in which the receiver is running
     * @param intent
     *            the Intent being received
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        Log.i(TAG, "[onReceive]...");
        String action = intent.getAction();
        switch (action) {
        case Intent.ACTION_SCREEN_ON:
            Log.d(TAG, "[ACTION_SCREEN_ON]...");
            break;
        case Intent.ACTION_SCREEN_OFF:
            Log.d(TAG, "[ACTION_SCREEN_OFF]...");
            break;
        case Intent.ACTION_USER_PRESENT:
            Log.d(TAG, "[ACTION_USER_PRESENT]Keyguard secure confirmed");
            sendUnlockMsg();
            break;
        default:
            break;
        }
    }

    private void sendUnlockMsg() {
        Log.d(TAG, "send keyguard unlock msg");
        Message msg = mHandler.obtainMessage();
        msg.what = VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BROADCAST_KEYGUARD_VERIFIED;
        mHandler.sendMessage(msg);
    }
}
