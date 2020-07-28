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

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.UserHandle;
import android.provider.Settings;

import com.mediatek.common.voicecommand.IVoiceWakeupInteractionCallback;
import com.mediatek.common.voicecommand.IVoiceWakeupInteractionService;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;
/**
 * Manage Voice Wake up business.
 *
 */
public class VoiceWakeupBusiness extends VoiceCommandBusiness {
    private static final String TAG = "VoiceWakeupBusiness";

    public static final int ACTION_MAIN_INTERNAL_WAKEUP = 20000;
    public static final int MSG_GET_WAKEUP_INIT = 10000;
    public static final int MSG_GET_WAKEUP_MODE = MSG_GET_WAKEUP_INIT + 1;
    public static final int MSG_GET_WAKEUP_COMMAND_STATUS = MSG_GET_WAKEUP_INIT + 2;
    public static final int MSG_GET_WAKEUP_VIS = MSG_GET_WAKEUP_INIT + 3;
    public static final int ACTION_VOICE_WAKEUP_IPO_BOOT = ACTION_MAIN_INTERNAL_WAKEUP;
    public static final int ACTION_VOICE_WAKEUP_IPO_SHUTDOWN = ACTION_MAIN_INTERNAL_WAKEUP + 1;
    public static final int ACTION_VOICE_WAKEUP_USER_SWITCHED_OWNER =
            ACTION_MAIN_INTERNAL_WAKEUP + 2;
    public static final int ACTION_VOICE_WAKEUP_USER_SWITCHED_NON_OWNER =
            ACTION_MAIN_INTERNAL_WAKEUP + 3;

    public static final String VOICE_WAKEUP_MODE = MtkSettingsExt.System.VOICE_WAKEUP_MODE;
    public static final String VOICE_WAKEUP_COMMAND_STATUS =
                                         MtkSettingsExt.System.VOICE_WAKEUP_COMMAND_STATUS;
    public static final String VOICE_WAKEUP_ACTIVE_COMMAND_ID =
                                         MtkSettingsExt.System.VOICE_WAKEUP_ACTIVE_COMMAND_ID;
    public static final String VOICE_WAKEUP_VIS = Settings.Secure.VOICE_INTERACTION_SERVICE;
    public static final String VOICE_WAKEUP_INTERACTION_SERVICE =
        "com.mediatek.voicecommand.vis/.VoiceWakeupInteractionService";

    private static final String MTK_VOW_SUPPORT_State = "MTK_VOW_SUPPORT";
    private static final String MTK_VOW_SUPPORT_on = "MTK_VOW_SUPPORT=true";

    /// Status variable to indicate if already started service.
    private static int sRecognitionStatus =
        VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED;

    private static String[] sActionMainToStr = {
        "ACTION_MAIN_INTERNAL_WAKEUP"
    };

    private static String[][] sActionSubToStr = {
        {
            "ACTION_VOICE_WAKEUP_IPO_BOOT",
            "ACTION_VOICE_WAKEUP_IPO_SHUTDOWN",
            "ACTION_VOICE_WAKEUP_USER_SWITCHED_OWNER",
            "ACTION_VOICE_WAKEUP_USER_SWITCHED_NON_OWNER"
        }
    };

    public static String getMainActionName(int action) {
        return sActionMainToStr[action - ACTION_MAIN_INTERNAL_WAKEUP];
    }

    public static String getSubActionName(int mainAction, int subAction) {
        int actionIndex = mainAction - ACTION_MAIN_INTERNAL_WAKEUP;
        if (actionIndex >= sActionSubToStr.length) {
            return "";
        } else {
            return sActionSubToStr[mainAction - ACTION_MAIN_INTERNAL_WAKEUP][subAction
                    - ACTION_MAIN_INTERNAL_WAKEUP];
        }
    }

    public static String sDumpMsg(VoiceMessage message) {
        if (message.mMainAction >= ACTION_MAIN_INTERNAL_WAKEUP) {
            return "main:" + getMainActionName(message.mMainAction) + " sub:"
                    + getSubActionName(message.mMainAction, message.mSubAction);
        } else {
            return message.toString();
        }
    }

    private IVoiceAdapter mIJniVoiceAdapter;
    private Context mContext;
    private IMessageDispatcher mUpDispatcher;
    private String mPakageName;
    private IVoiceWakeupInteractionService mInteractionService;

    /**
     * VoiceWakeup constructor.
     * 
     * @param dispatcher
     *            NativeDataManager instance
     * @param cfgMgr
     *            ConfigurationManager instance
     * @param handler
     *            the handler to run voice wake up message
     * @param adapter
     *            SwipAdapter instance
     * @param context
     *            context
     */
    public VoiceWakeupBusiness(IMessageDispatcher dispatcher, ConfigurationManager cfgMgr,
            Handler handler, IVoiceAdapter adapter, Context context) {
        super(dispatcher, cfgMgr, handler);
        Log.i(TAG, "[VoiceWakeupBusiness]new...");
        mIJniVoiceAdapter = adapter;
        mContext = context;
        mUpDispatcher = dispatcher;
        bindVoiceInteractionService();
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleSyncVoiceMessage]message = " + sDumpMsg(message));
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (!isWakeupSupport(mContext) || !mCfgMgr.isDemoSupported()) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.i(TAG, "[handleSyncVoiceMessage]Voice Wakeup feature is off, return!");
            sendMessageToApps(message, errorid);
            return errorid;
        }

        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_WAKEUP_START:
            errorid = sendMessageToHandler(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_ENABLE:
            errorid = sendMessageToHandler(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_DISABLE:
            errorid = sendMessageToHandler(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_READ_STATUS:
            errorid = handleWakeupReadStatus(message);
            break;

        default:
            break;
        }
        Log.i(TAG, "[handleSyncVoiceMessage]errorid = " + errorid);

        return errorid;
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleAsyncVoiceMessage]message = " + sDumpMsg(message));
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (!isWakeupSupport(mContext) || !mCfgMgr.isDemoSupported()) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.i(TAG, "[handleAsyncVoiceMessage]Voice Wakeup feature is off, return!");
            sendMessageToApps(message, errorid);
            return errorid;
        }
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_WAKEUP_START:
            errorid = handleWakeupStart(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_ENABLE:
            errorid = handleWakeupEnable(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_DISABLE:
            errorid = handleWakeupDisable(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_INIT:
            errorid = handleWakeupInit(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_MODE:
            errorid = handleWakeupMode(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_COMMAND_STATUS:
            errorid = handleWakeupCmdStatus(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_WAKEUP_NOTIFY:
            errorid = handleWakeupNotify(message);
            break;

        case ACTION_VOICE_WAKEUP_USER_SWITCHED_OWNER:
            errorid = handleWakeupUserSwitchedOwner(message);
            break;

        case ACTION_VOICE_WAKEUP_USER_SWITCHED_NON_OWNER:
            errorid = handleWakeupUserSwitchedNonOwner(message);
            break;

        default:
            break;
        }
        Log.i(TAG, "[handleAsyncVoiceMessage]errorid = " + errorid);

        return errorid;
    }

    /**
     * Query wakeup mode from setting provider.
     * 
     * @param context
     *            context
     * @return wakeup mode
     */
    public static int getWakeupMode(Context context) {
        int mode = Settings.System.getInt(context.getContentResolver(), VOICE_WAKEUP_MODE, 1);
        Log.i(TAG, "[getWakeupMode]mode : " + VoiceCommandListener.getWakeupModeStr(mode));

        return mode;
    }

    /**
     * Query wakeup command status from setting provider.
     * 
     * @param context
     *            context
     * @return wakeup command status
     */
    public static int getWakeupCmdStatus(Context context) {
        int cmdStatus = Settings.System.getInt(context.getContentResolver(),
                VOICE_WAKEUP_COMMAND_STATUS, 0);
        Log.i(TAG,
                "[getWakeupCmdStatus]cmdStatus : "
                        + VoiceCommandListener.getWakeupStatusStr(cmdStatus));

        return cmdStatus;
    }

    /**
     * Query wakeup command id that is currently active.
     *
     * @param context
     *            context
     * @return wakeup command status
     */
    public static int getWakeupActiveCmdId(Context context) {
        int activeCmdValue = Settings.System.getInt(context.getContentResolver(),
                VOICE_WAKEUP_ACTIVE_COMMAND_ID, 1);
        Log.i(TAG,
                "[getWakeupActiveCmdId]activeCmdValue : "
                        + activeCmdValue);

        return activeCmdValue;
    }

    /**
     * Save wakeup command id that is currently active to setting provider.
     *
     * @param context
     *            context
     * @param cmdValue
     *            active Command value
     */
    public static void setWakeupActiveCmdId(Context context, int cmdValue) {
        Log.i(TAG, "[setWakeupActiveCmdId] to setting provider activeCommandId : " + cmdValue);
        Settings.System
                .putInt(context.getContentResolver(), VOICE_WAKEUP_ACTIVE_COMMAND_ID, cmdValue);
    }


    /**
     * get wakeup enable status through wakeup command status.
     *
     * @param cmdStatus
     *            wakeup command status
     * @return wakeup enable status
     */
    public static int getWakeupEnableStatus(int cmdStatus) {
        int wakeupEnableStatus = 0;
        if ((cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED)
                || (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED)) {
            wakeupEnableStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED;
        } else if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
            wakeupEnableStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED;
        }
        Log.i(TAG, "[getWakeupEnableStatus]wakeupEnableStatus: " + wakeupEnableStatus);

        return wakeupEnableStatus;
    }

    /**
     * Save wakeup status to setting provider.
     * 
     * @param context
     *            context
     * @param cmdStatus
     *            wakeup status
     */
    public static void setWakeupCmdStatus(Context context, int cmdStatus) {
        Log.i(TAG, "[setWakeupCmdStatus] to setting provider cmdStatus : " + cmdStatus);
        Settings.System
                .putInt(context.getContentResolver(), VOICE_WAKEUP_COMMAND_STATUS, cmdStatus);
    }

    /**
     * Check if support voice wakeup feature.
     * 
     * @param context
     *            context
     * @return true if support, otherwise false
     */
    public static boolean isWakeupSupport(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am == null) {
            Log.e(TAG, "[isWakeupSupport] get audio service is null");
            return false;
        }
        String state = am.getParameters(MTK_VOW_SUPPORT_State);
        if (state != null) {
            return state.equalsIgnoreCase(MTK_VOW_SUPPORT_on);
        }
        return false;
    }

    /**
     * Start the wake up business in native adapter.
     * 
     * @param message
     *            VoiceMessage instance
     * 
     * @return result
     */
    private int handleWakeupStart(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupStart]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        errorid = mIJniVoiceAdapter.startVoiceWakeup(message.mPkgName, message.pid);
        sendMessageToApps(message, errorid);
        mPakageName = message.mPkgName;

        return errorid;
    }

    /**
     * Handle request for Wakeup mode enable.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleWakeupEnable(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupEnable]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int wakeupCmdStatus = mCfgMgr.getWakeupCmdStatus();
        if (wakeupCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
        } else if (wakeupCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED) {
            setWakeupCmdStatus(mContext, VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED);
        }
        Log.d(TAG, "[handleWakeupEnable]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     * Handle request for Wakeup mode disable/reset.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleWakeupDisable(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupDisable]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int wakeupCmdStatus = mCfgMgr.getWakeupCmdStatus();
        int isReset = 0;
        if (message.mExtraData != null) {
            isReset = message.mExtraData.getInt(
                    VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        }
        if (wakeupCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
            if (isReset == 1) {
                setWakeupCmdStatus(
                        mContext,
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED);
            } else {
                setWakeupCmdStatus(
                        mContext,
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED);
            }
        }
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     * Handle wakeup init request sent from mode observer.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleWakeupInit(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupInit]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int mode = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        int cmdStatus = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);
        int[] cmdIds = message.mExtraData.getIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO2);
        String patternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(mode);
        String anyOnePatternPath = mCfgMgr
                .getVoiceRecognitionPatternFilePath(VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT);
        String anyOnePasswordPath = mCfgMgr
                .getPasswordFilePath(VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT);
        String commandPatternPath = mCfgMgr
                .getVoiceRecognitionPatternFilePath(VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT);
        String commandPasswordPath = mCfgMgr
                .getPasswordFilePath(VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT);
        String ubmPath;
        if (cmdIds.length > 0
            && mode == VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT) {
            //Need to override the cmdIds and update ubm file path.
            ubmPath = mCfgMgr.getUbmFilePath(mCfgMgr.getActiveCommandId());
        } else {
            ubmPath = mCfgMgr.getUbmFilePath();
        }
        String wakeupinfoPath = mCfgMgr.getWakeupInfoPath();
        if (patternPath == null || anyOnePatternPath == null || anyOnePasswordPath == null
                || commandPatternPath == null || commandPasswordPath == null || ubmPath == null
                || wakeupinfoPath == null) {
            Log.d(TAG, "[handleWeakupInit] error patternPath=" + patternPath
                    + " anyOnePatternPath=" + anyOnePatternPath + " anyOnePasswordPath="
                    + anyOnePasswordPath + " commandPatternPath=" + commandPatternPath
                    + " commandPasswordPath=" + commandPasswordPath + " ubmPath=" + ubmPath
                    + "wakeupinfoPath=" + wakeupinfoPath);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        } else {
            int wakeupEnableStatus = getWakeupEnableStatus(cmdStatus);
            mCfgMgr.setWakeupMode(mode);
            mCfgMgr.setWakeupStatus(cmdStatus);
            errorid = mIJniVoiceAdapter.initVoiceWakeup(mode, wakeupEnableStatus, cmdIds,
                    patternPath, VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT,
                    anyOnePatternPath, anyOnePasswordPath,
                    VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT, commandPatternPath,
                    commandPasswordPath, ubmPath, wakeupinfoPath);
        }
        Log.d(TAG, "[handleWakeupInit]errorid = " + errorid);

        // After reboot, we need to check and startRecognition if not already activated.
        // This is added to fix ALPS03080838 on GMS builds.
        if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
            // TODO: May need to additionally check if already startedRecognition.
            // No special check available yet.
            changeVoiceWakeupVIS(cmdStatus);
        }

        return errorid;
    }

    /**
     * Handle wakeup mode update request.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleWakeupMode(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupMode]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int mode = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        mCfgMgr.setWakeupMode(mode);
        String ubmPath = mCfgMgr.getUbmFilePath();
        String ubmUpgradePath = mCfgMgr.getUbmFileUpgradePath();
        if (ubmPath == null) {
            Log.i(TAG, "[handleWakeupMode] error ubmPath=" + ubmPath);
        } else {
            errorid = mIJniVoiceAdapter.sendVoiceWakeupMode(mode, ubmPath, ubmUpgradePath);
        }
        Log.d(TAG, "[handleWakeupMode]errorid = " + errorid);

        return errorid;
    }

    /**
     * Handle wakeup command status update request.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleWakeupCmdStatus(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupCmdStatus]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int wakeupCmdStatus = message.mExtraData
                .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        int wakeupEnableStatus = getWakeupEnableStatus(wakeupCmdStatus);
        mCfgMgr.setWakeupStatus(wakeupCmdStatus);
        errorid = mIJniVoiceAdapter.sendVoiceWakeupCmdStatus(wakeupEnableStatus);
        Log.d(TAG, "[handleWakeupCmdStatus]errorid = " + errorid);

        // Change Voice Wakeup Interaction Service action.
        changeVoiceWakeupVIS(wakeupCmdStatus);

        return errorid;
    }

    /**
     * When User switched to non owner, need stop recognition if Wakeup is enable.
     * @param message
     * @return
     */
    private int handleWakeupUserSwitchedNonOwner(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int cmdStatus = getWakeupCmdStatus(mContext);
        String interactionService = Settings.Secure.getString(mContext.getContentResolver(),
                Settings.Secure.VOICE_INTERACTION_SERVICE);
        int newUserId = UserHandle.USER_SYSTEM + 1;
        if (message.mExtraData != null) {
            newUserId = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        }

        Log.d(TAG, "[handleWakeupUserSwitchedNonOwner] cmdStatus: " + cmdStatus
                + ", interactionService: " + interactionService);
        Log.d(TAG, "[handleWakeupUserSwitchedNonOwner] UserId:" + UserHandle.myUserId());

        try {
            if (mInteractionService != null) {
                //Set non Owner. Currently use USER_SYSTEM+1.
                mInteractionService.setCurrentUserOnSwitch(newUserId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[handleWakeupUserSwitchedNonOwner] exception:" + e.getMessage());
        }
        return errorid;
    }

    /**
     * When user switch to owner, need start recognition if VoiceWakeup is enable.
     * @param message
     * @return
     */
    private int handleWakeupUserSwitchedOwner(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int cmdStatus = getWakeupCmdStatus(mContext);
        String interactionService = Settings.Secure.getString(mContext.getContentResolver(),
                Settings.Secure.VOICE_INTERACTION_SERVICE);
        int newUserId = UserHandle.USER_SYSTEM;

        if (message.mExtraData != null) {
            newUserId = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        }
        Log.d(TAG, "[handleWakeupUserSwitchedOwner] cmdStatus: " + cmdStatus
                + ", interactionService: " + interactionService);
        Log.d(TAG, "[handleWakeupUserSwitchedOwner] UserId:" + UserHandle.myUserId());

        try {
            if (mInteractionService != null) {
                mInteractionService.setCurrentUserOnSwitch(newUserId);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[handleWakeupUserSwitchedOwner] exception:" + e.getMessage());
        }
        return errorid;
    }

    /**
     *  Read and return directly the Wakeup status value from Setting preference.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleWakeupReadStatus(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupReadStatus]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = null;

        int status = getWakeupCmdStatus(mContext);

        if (VoiceCommandListener.VOICE_NO_ERROR == errorid) {
            bundle = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                    status, null);
        } else {
            bundle = DataPackage.packageErrorResult(
                    errorid);
        }
        message.mExtraData = bundle;

        Log.d(TAG, "[handleWakeupReadStatus]errorid = " + errorid);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
     * Bind Voice Wakeup Interaction service.
     */
    public void bindVoiceInteractionService() {
        Log.d(TAG, "[bindVoiceInteractionService]... ");
        Intent intent = new Intent();
        intent.setAction(VoiceCommandListener.VOICE_WAKEUP_SERVICE_ACTION);
        intent.setPackage(VoiceCommandListener.VOICE_TRAINING_SERVICE_PACKAGE_NAME);
        mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    /**
     * VoiceWakeupInteractionService connection.
     */
    private ServiceConnection mConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "[mConnection.onServiceConnected]... ");

            mInteractionService = IVoiceWakeupInteractionService.Stub.asInterface(service);
            try {
                mInteractionService.registerCallback(mCallback);
            } catch (RemoteException e) {
                Log.e(TAG, "[registerCallback]exception: " + e.getMessage());
            }
        }
        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "[mConnection.onServiceDisconnected]... ");
            mInteractionService = null;
        }
    };

    /**
     * It is used to receive VoiceWakeupInteractionService callback when there
     * has recognition result.
     */
  private IVoiceWakeupInteractionCallback mCallback = new IVoiceWakeupInteractionCallback.Stub() {
        @Override
        public void onVoiceWakeupDetected(int commandId) {
            if (commandId == VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE) {
                // Need to restart recognition.
                String interactionService = Settings.Secure.getString(
                        mContext.getContentResolver(),
                        Settings.Secure.VOICE_INTERACTION_SERVICE);
                int cmdStatus = getWakeupCmdStatus(mContext);

                Log.d(TAG, "[onVoiceWakeupDetected] restartRecognition as AudioServer died");
                if ((cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) &&
                    (VOICE_WAKEUP_INTERACTION_SERVICE).equals(interactionService)) {
                    startRecognition(true);
                }
                return;
            }
            // Need to handle message in Main thread.
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP;
            message.mSubAction = VoiceCommandListener.ACTION_VOICE_WAKEUP_NOTIFY;
            message.mPkgName = mPakageName;
            int wakeupMode = VoiceWakeupBusiness.getWakeupMode(mContext);
            message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                    mCfgMgr.getActiveCommandId(), wakeupMode);
            Log.d(TAG, "[onVoiceWakeupDetected] mPakageName : " + mPakageName + ", commandId : "
                    + commandId + ", wakeupMode : " + wakeupMode);
            sendMessageToHandler(message);
        }
    };

    /**
     * Change VoiceWakeupInteractionService when command status changed.
     * @param cmdStatus command status.
     */
    private void changeVoiceWakeupVIS(int cmdStatus) {
        String interactionService = Settings.Secure.getString(mContext.getContentResolver(),
                Settings.Secure.VOICE_INTERACTION_SERVICE);
        Log.d(TAG, "[changeVoiceWakeupVIS] cmdStatus: " + cmdStatus + ", interactionService: "
                + interactionService);
        if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
            if ((VOICE_WAKEUP_INTERACTION_SERVICE).equals(interactionService)) {
                startRecognition();
            } else {
                Settings.Secure
                        .putString(mContext.getContentResolver(),
                                Settings.Secure.VOICE_INTERACTION_SERVICE,
                                VOICE_WAKEUP_INTERACTION_SERVICE);
            }
        } else if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED) {
            if ((VOICE_WAKEUP_INTERACTION_SERVICE).equals(interactionService)) {
                stopRecognition();
            }
        }
    }

    /**
     * Start recognition, invoke VoiceWakeupInteractionService.startRecognition().
     *
     * @return true if success.
     */
    private boolean startRecognition() {
        return startRecognition(false);
    }

    /**
     * Start recognition, invoke VoiceWakeupInteractionService.startRecognition().
     * @param isForced Force to startRecognition
     * @return true if success.
     */
    private boolean startRecognition(boolean isForced) {
        boolean success = false;
        Log.i(TAG, "[startRecognition] isForced =" + isForced
                   + ", sRecognitionStatus =" + sRecognitionStatus);
        try {
            if (mInteractionService == null) {
                Log.e(TAG,
                  "[startRecognition]mInteractionService is null, can not startRecognition");
            } else if (isForced ||
                (sRecognitionStatus != VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED)) {
                success = mInteractionService.startRecognition();
            } else {
                Log.e(TAG,
                  "[startRecognition]recognition status is VOICE_WAKEUP_STATUS_COMMAND_CHECKED.");
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[startRecognition]exception:" + e.getMessage());
        }
        if (success) {
            sRecognitionStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED;
        }
        Log.i(TAG, "[startRecognition] success =" + success + ", sRecognitionStatus="
                + sRecognitionStatus);
        return success;
    }

    /**
     * Stop recognition, invoke VoiceWakeupInteractionService.stopRecognition().
     *
     * @return true if success.
     */
    private boolean stopRecognition() {
        boolean success = false;
        try {
            if (mInteractionService != null &&
              sRecognitionStatus != VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED) {
                success = mInteractionService.stopRecognition();
            } else if (mInteractionService == null) {
                Log.e(TAG,
               "[stopRecognition]mInteractionService is null, can not stopRecognition");
            } else {
                Log.e(TAG,
                "[stopRecognition]recognition status is not VOICE_WAKEUP_STATUS_COMMAND_CHECKED.");
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[stopRecognition] exception:" + e.getMessage());
        }

        if (success) {
            sRecognitionStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED;
        }
        Log.d(TAG, "[stopRecognition] success =" + success + ", sRecognitionStatus="
                + sRecognitionStatus);
        return success;
    }

    /**
     * Dispatch wakeup recognition result to App.
     * @param message voice message
     * @return
     */
    private int handleWakeupNotify(VoiceMessage message) {
        Log.d(TAG, "[handleWakeupNotify]...");
        return mUpDispatcher.dispatchMessageUp(message);
    }

    /**
     * Handle data release. Here is used to unbind VoiceWakeupInteractionService.
     */
    @Override
    public void handleDataRelease() {
        Log.d(TAG, "[handleDataRelease]...");
        if (mInteractionService != null) {
            mContext.unbindService(mConnection);
            mInteractionService = null;
        }
    }
}
