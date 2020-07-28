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
import android.os.SystemProperties;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

public class VoiceUiBusiness extends VoiceCommandBusiness {
    private static final String TAG = "VoiceUiBusiness";

    private IVoiceAdapter mIJniVoiceAdapter;
    public static final boolean MTK_VOICE_UI_SUPPORT = SystemProperties.get(
            "ro.mtk_voice_ui_support").equals("1");

    public VoiceUiBusiness(IMessageDispatcher dispatcher, ConfigurationManager cfgMgr,
            Handler handler, IVoiceAdapter adapter) {
        super(dispatcher, cfgMgr, handler);
        Log.i(TAG, "[VoiceUiBusiness]new...");
        mIJniVoiceAdapter = adapter;
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[handleSyncVoiceMessage]mSubAction = " + message.mSubAction);

        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_UI_START:
        case VoiceCommandListener.ACTION_VOICE_UI_STOP:
        case VoiceCommandListener.ACTION_VOICE_UI_ENABLE:
        case VoiceCommandListener.ACTION_VOICE_UI_DISALBE:
            errorid = sendMessageToHandler(message);
            break;

        default:
            break;
        }
        Log.i(TAG, "[handleSyncVoiceMessage]errorid = " + errorid);

        return errorid;
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[handleAsyncVoiceMessage]mSubAction = " + message.mSubAction);
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_UI_START:
            errorid = handleUiStart(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_UI_STOP:
            errorid = handleUiStop(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_UI_ENABLE:
            handleUiEnable(message, true);
            break;

        case VoiceCommandListener.ACTION_VOICE_UI_DISALBE:
            handleUiEnable(message, false);
            break;

        default:
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            break;
        }
        Log.i(TAG, "[handleAsyncVoiceMessage]errorid = " + errorid);

        return errorid;
    }

    private int handleUiStart(VoiceMessage message) {
        Log.d(TAG, "[handleUiStart]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCfgMgr.isProcessEnable(message.mPkgName)) {
            String modelpath = mCfgMgr.getModelFile();
            String patternpath = mCfgMgr.getVoiceUIPatternPath();
            int languageid = mCfgMgr.getCurrentLanguageID();
            if (modelpath == null || patternpath == null || languageid < 0) {
                Log.d(TAG, "[handleUiStart] error modelpath=" + modelpath + ",languageid="
                        + languageid);
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            } else {
                int pid = mCfgMgr.getProcessID(message.mPkgName);
                errorid = mIJniVoiceAdapter.startVoiceUi(modelpath, patternpath, message.mPkgName,
                        pid, languageid);
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_PROCESS_OFF;
        }
        Log.d(TAG, "[handleUiStart]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    private int handleUiStop(VoiceMessage message) {
        Log.d(TAG, "[handleUiStop]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCfgMgr.isProcessEnable(message.mPkgName)) {
            int pid = mCfgMgr.getProcessID(message.mPkgName);
            errorid = mIJniVoiceAdapter.stopVoiceUi(message.mPkgName, pid);
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_PROCESS_OFF;
        }
        Log.d(TAG, "[handleUiStop]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    private int handleUiEnable(VoiceMessage message, boolean isEnable) {
        Log.d(TAG, "[handleUiEnable]isEnable = " + isEnable);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Boolean isCurEnable = mCfgMgr.isProcessEnable(message.mPkgName);
        if (!(isCurEnable & isEnable)) {
            errorid = mCfgMgr.updateFeatureEnable(message.mPkgName, isEnable) ? VoiceCommandListener.VOICE_NO_ERROR
                    : VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;

            if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
                sendMessageToApps(message, errorid);
            }
        }

        if (isEnable) {
            handleUiStart(message);
        } else {
            handleUiStop(message);
        }
        Log.d(TAG, "[handleUiEnable]errorid = " + errorid);

        return errorid;
    }

}
