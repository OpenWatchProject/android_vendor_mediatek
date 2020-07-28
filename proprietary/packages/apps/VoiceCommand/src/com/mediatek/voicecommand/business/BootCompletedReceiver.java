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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;

import com.mediatek.voicecommand.util.Log;

/**
 * Receive intents sent by sendBroadcast.
 * 
 */
public class BootCompletedReceiver extends BroadcastReceiver {
    private static final String TAG = "BootCompletedReceiver";

    private Context mContext;
    private Handler mHandler;

    private VoiceContactsObserver mVoiceContactsObserver;
    private VoiceWakeupModeObserver mVoiceWakeupModeObserver;
    private VoiceWakeupCmdStatusObserver mVoiceWakeupCmdStatusObserver;
    private VoiceWakeupVISObserver mVoiceWakeupVISObserver = null;

    /**
     * BootCompletedReceiver constructor.
     * 
     * @param context
     *            context
     * @param handler
     *            the handler to run onReceive(Context, Intent) on
     */
    public BootCompletedReceiver(Context context, Handler handler) {
        Log.i(TAG, "[BootCompletedReceiver]new...");
        mContext = context;
        mHandler = handler;
        mVoiceContactsObserver = new VoiceContactsObserver(mContext, mHandler);
        mVoiceWakeupModeObserver = new VoiceWakeupModeObserver(mContext, mHandler);
        mVoiceWakeupCmdStatusObserver = new VoiceWakeupCmdStatusObserver(mContext, mHandler);
        if (!VoiceTriggerBusiness.isTriggerSupport(mContext)) {
            mVoiceWakeupVISObserver = new VoiceWakeupVISObserver(mContext, mHandler);
        }
        // Must register wakeup mode/commandstatus/VIS observer when boot up.
        registerWakeupObserver();
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
        Message msg = mHandler.obtainMessage();
        msg.what = VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BROADCAST_BOOT_COMPLETED;
        mHandler.sendMessage(msg);
        // Must register contacts change observer when boot complete receive, because it will
        // create sound models when send contacts which take up of many CPU, then maybe cause
        // ANR when boot up.
        registerContactsObserver();
    }

    /**
     * Register content observer for Contacts.
     */
    public void registerContactsObserver() {
        Log.d(TAG, "[registerContactsObserver]...");
        if (VoiceContactsBusiness.MTK_VOICE_CONTACT_SEARCH_SUPPORT) {
            Log.i(TAG, "[registerObserver]register contacts.");
            mVoiceContactsObserver.bootCompletedReceive();
            mContext.getContentResolver().registerContentObserver(
                    VoiceContactsObserver.CONTACTS_URI, true, mVoiceContactsObserver);
        }
    }

    /**
     * Register voice wakeup command status and content observer.
     */
    public void registerWakeupObserver() {
        Log.d(TAG, "[registerWakeupObserver]...");
        if (VoiceWakeupBusiness.isWakeupSupport(mContext)) {
            Log.i(TAG, "[registerObserver]register wake up.");
            if (!VoiceTriggerBusiness.isTriggerSupport(mContext)) {
                mVoiceWakeupModeObserver.bootUp();
                mContext.getContentResolver().registerContentObserver(
                        Settings.Secure.getUriFor(VoiceWakeupBusiness.VOICE_WAKEUP_VIS), true,
                        mVoiceWakeupVISObserver);
                mContext.getContentResolver().registerContentObserver(
                        Settings.System.getUriFor(VoiceWakeupBusiness.VOICE_WAKEUP_COMMAND_STATUS),
                        true, mVoiceWakeupCmdStatusObserver);
                mContext.getContentResolver().registerContentObserver(
                        Settings.System.getUriFor(VoiceWakeupBusiness.VOICE_WAKEUP_MODE), true,
                        mVoiceWakeupModeObserver);
            } else {
                mContext.getContentResolver().registerContentObserver(
                        Settings.System.getUriFor(
                                VoiceTriggerBusiness.VOICE_TRIGGER_COMMAND_STATUS),
                        true, mVoiceWakeupCmdStatusObserver);
                mContext.getContentResolver().registerContentObserver(
                        Settings.System.getUriFor(VoiceTriggerBusiness.VOICE_TRIGGER_MODE), true,
                        mVoiceWakeupModeObserver);
            }
        }
    }

    /**
     * Release all variables when service is destroy.
     */
    public void handleDataRelease() {
        Log.i(TAG, "[handleDataRelease]...");
        if (VoiceContactsBusiness.MTK_VOICE_CONTACT_SEARCH_SUPPORT) {
            Log.i(TAG, "[handleDataRelease]unregister contacts.");
            mContext.getContentResolver().unregisterContentObserver(mVoiceContactsObserver);
        }

        if (VoiceWakeupBusiness.isWakeupSupport(mContext)) {
            Log.i(TAG, "[handleDataRelease]unregister wake up.");
            mContext.getContentResolver().unregisterContentObserver(mVoiceWakeupModeObserver);
            mContext.getContentResolver().unregisterContentObserver(mVoiceWakeupCmdStatusObserver);
            if (mVoiceWakeupVISObserver != null) {
                mContext.getContentResolver().unregisterContentObserver(mVoiceWakeupVISObserver);
            }
        }
    }

    /**
     * Send voice wakeup init message to MD32.
     */
    public void sendWakeupInitMessage() {
        Log.i(TAG, "[sendWakeupInitMessage]...");
        Handler handler = mVoiceWakeupModeObserver.getVoiceWakeupModeHandler();
        if (handler != null) {
            handler.sendEmptyMessage(VoiceWakeupBusiness.MSG_GET_WAKEUP_INIT);
        }
    }

    /**
     * Send voice trigger init message to MD32.
     */
    public void sendTriggerInitMessage() {
        Log.i(TAG, "[sendTriggerInitMessage]...");
        Handler handler = mVoiceWakeupModeObserver.getVoiceWakeupModeHandler();
        if (handler != null) {
            handler.sendEmptyMessage(VoiceTriggerBusiness.MSG_GET_TRIGGER_INIT);
        }
    }
}
