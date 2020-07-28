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

import android.os.Handler;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

public class VoiceServiceInternalBusiness extends VoiceCommandBusiness {
    private static final String TAG = "VoiceServiceInternalBusiness";

    public static final int ACTION_MAIN_VOICE_SERVICE_BASE = 10000;
    // Used to deal with the event only inside the service
    public static final int ACTION_MAIN_VOICE_SERVICE = ACTION_MAIN_VOICE_SERVICE_BASE;

    // Used to deal with the event Broadcast
    public static final int ACTION_MAIN_VOICE_BROADCAST = ACTION_MAIN_VOICE_SERVICE + 1;

    // Used to deal with the event Broadcast of boot completed
    public static final int ACTION_MAIN_VOICE_BROADCAST_BOOT_COMPLETED =
            ACTION_MAIN_VOICE_SERVICE + 2;

    // Used to deal with the event of boot completed
    public static final int ACTION_MAIN_VOICE_BOOT_COMPLETED = ACTION_MAIN_VOICE_SERVICE + 3;

    // Used to deal with the event of boot completed
    public static final int ACTION_MAIN_VOICE_BROADCAST_KEYGUARD_VERIFIED =
            ACTION_MAIN_VOICE_SERVICE + 4;

    // Used to deal with the event from death notification
    public static final int ACTION_VOICE_SERVICE_PROCESSEXIT = ACTION_MAIN_VOICE_SERVICE_BASE;

    // Used to deal with the event when service need to be destroyed
    public static final int ACTION_VOICE_SERVICE_SELFEXIT = ACTION_MAIN_VOICE_SERVICE_BASE + 1;

    // Used to deal with the event when HeadSet plug in
    public static final int ACTION_VOICE_BROADCAST_HEADSETPLUGIN = ACTION_MAIN_VOICE_SERVICE_BASE;

    // Used to deal with the event when HeadSet plug in
    public static final int ACTION_VOICE_BROADCAST_HEADSETPLUGOUT =
            ACTION_MAIN_VOICE_SERVICE_BASE + 1;

    private static String[] sActionMainToStr = {
        "ACTION_MAIN_VOICE_SERVICE",
        "ACTION_MAIN_VOICE_BROADCAST",
        "ACTION_MAIN_VOICE_BROADCAST_BOOT_COMPLETED",
        "ACTION_MAIN_VOICE_BOOT_COMPLETED",
        "ACTION_MAIN_VOICE_BROADCAST_KEYGUARD_VERIFIED",
    };

    private static String[][] sActionSubToStr = {
        {
            "ACTION_VOICE_SERVICE_PROCESSEXIT",
            "ACTION_VOICE_SERVICE_SELFEXIT"
        },
        {
            "ACTION_VOICE_BROADCAST_HEADSETPLUGIN",
            "ACTION_VOICE_BROADCAST_HEADSETPLUGOUT"
        }
    };

    public static String getMainActionName(int action) {
        return sActionMainToStr[action - ACTION_MAIN_VOICE_SERVICE_BASE];
    }

    public static String getSubActionName(int mainAction, int subAction) {
        int actionIndex = mainAction - ACTION_MAIN_VOICE_SERVICE_BASE;
        if (actionIndex >= sActionSubToStr.length) {
            return "";
        } else {
            return sActionSubToStr[mainAction - ACTION_MAIN_VOICE_SERVICE_BASE][subAction
                    - ACTION_MAIN_VOICE_SERVICE_BASE];
        }
    }

    public static String sDumpMsg(VoiceMessage message) {
        if (message.mMainAction >= ACTION_MAIN_VOICE_SERVICE_BASE) {
            return "main:" + getMainActionName(message.mMainAction) + " sub:"
                    + getSubActionName(message.mMainAction, message.mSubAction);
        } else {
            return message.toString();
        }
    }

    private IVoiceAdapter mIJniVoiceAdapter;

    public VoiceServiceInternalBusiness(IMessageDispatcher dispatcher, ConfigurationManager cfgMgr,
            Handler handler, IVoiceAdapter adapter) {
        super(dispatcher, cfgMgr, handler);
        Log.i(TAG, "[VoiceServiceInternalBusiness]new...");
        mIJniVoiceAdapter = adapter;
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleAsyncVoiceMessage]message = " + sDumpMsg(message));

        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mMainAction) {
        case ACTION_MAIN_VOICE_SERVICE:
            if (message.mSubAction == ACTION_VOICE_SERVICE_PROCESSEXIT) {
                handleProcessExit(message);
            }
            break;

        case ACTION_MAIN_VOICE_BROADCAST:
            handleHeadsetPlugEvent(message);
            break;

        default:
            break;
        }

        return errorid;
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleSyncVoiceMessage]message = " + sDumpMsg(message));

        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mMainAction) {
        case ACTION_MAIN_VOICE_SERVICE:
            if (message.mSubAction == ACTION_VOICE_SERVICE_PROCESSEXIT) {
                sendMessageToHandler(message);
            } else if (message.mSubAction == ACTION_VOICE_SERVICE_SELFEXIT) {
                handleDataRelease();
            }
            break;

        case ACTION_MAIN_VOICE_BROADCAST:
            if (mHandler.hasMessages(ACTION_MAIN_VOICE_BROADCAST)) {
                mHandler.removeMessages(ACTION_MAIN_VOICE_BROADCAST);
            }
            sendMessageToHandler(message);
            break;

        default:
            break;
        }

        return errorid;
    }

    @Override
    public void handleDataRelease() {
        Log.i(TAG, "[handleDataRelease]...");
        mIJniVoiceAdapter.release();
    }

    /*
     * Notify JNI to stop the voice command business because the app process was
     * died
     * 
     * @param message
     */
    private void handleProcessExit(VoiceMessage message) {
        Log.d(TAG, "[handleProcessExit]pkgName = " + message.mPkgName + ",pid = " + message.pid);
        mIJniVoiceAdapter.stopCurMode(message.mPkgName, message.pid);
    }

    /*
     * Notify JNI to handle HeadsetPlug Event
     * 
     * @param message
     */
    private void handleHeadsetPlugEvent(VoiceMessage message) {
        Log.d(TAG, "[handleHeadsetPlugEvent]mSubAction = " + message.mSubAction);
        if (message.mSubAction == ACTION_VOICE_BROADCAST_HEADSETPLUGIN) {
            mIJniVoiceAdapter.setCurHeadsetMode(true);
        } else {
            mIJniVoiceAdapter.setCurHeadsetMode(false);
        }
    }

}
