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

import java.util.ArrayList;

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
import android.os.UserManager;
import android.provider.Settings;

import com.android.internal.util.ArrayUtils;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.voicecommand.cfg.VoiceWakeupInfo;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.service.TriggerService;
import com.mediatek.voicecommand.service.TriggerService.IPackageUpdateCallback;
import com.mediatek.voicecommand.service.TriggerService.IVoiceWakeupCallback;
import com.mediatek.voicecommand.service.TriggerService.IVoiceModelUpdateCallback;
import com.mediatek.voicecommand.service.TriggerService.TriggerBinder;
import com.mediatek.voicecommand.util.Log;
/**
 * Manage Voice Trigger wakeup business.
 *
 */
public class VoiceTriggerBusiness extends VoiceCommandBusiness {
    private static final String TAG = "VoiceTriggerBusiness";

    public static final int MSG_GET_TRIGGER_INIT = 20000;
    public static final int MSG_GET_TRIGGER_MODE = MSG_GET_TRIGGER_INIT + 1;
    public static final int MSG_GET_TRIGGER_COMMAND_STATUS = MSG_GET_TRIGGER_INIT + 2;
    public static final int ACTION_MAIN_INTERNAL_TRIGGER = 30000;
    public static final int ACTION_VOICE_TRIGGER_START_ADAPTER =
            ACTION_MAIN_INTERNAL_TRIGGER;
    public static final int ACTION_VOICE_TRIGGER_USER_SWITCHED_OWNER =
            ACTION_MAIN_INTERNAL_TRIGGER + 1;
    public static final int ACTION_VOICE_TRIGGER_USER_SWITCHED_NON_OWNER =
            ACTION_MAIN_INTERNAL_TRIGGER + 2;
    public static final int ACTION_VOICE_TRIGGER_NOTIFY_RETRAIN =
            ACTION_MAIN_INTERNAL_TRIGGER + 3;
    public static final int ACTION_VOICE_TRIGGER_NOTIFY_AUDIOERROR =
            ACTION_MAIN_INTERNAL_TRIGGER + 4;
    //Should these be stored in Settings or in VoiceCommand share
    public static final String VOICE_TRIGGER_MODE = MtkSettingsExt.System.VOICE_TRIGGER_MODE;
    public static final String VOICE_TRIGGER_COMMAND_STATUS =
                                         MtkSettingsExt.System.VOICE_TRIGGER_COMMAND_STATUS;

    private static final String MTK_TRIGGER_SUPPORT_State = "MTK_VOW_2E2K_SUPPORT";
    private static final String MTK_TRIGGER_SUPPORT_on = "MTK_VOW_2E2K_SUPPORT=true";

    /// Status variable to indicate if already started service.
    private static int sRecognitionStatus =
        VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED;


    private static String[] sActionMainToStr = {
        "ACTION_MAIN_INTERNAL_TRIGGER"
    };

    private static String[][] sActionSubToStr = {
        {
            "ACTION_VOICE_TRIGGER_START_ADAPTER",
            "ACTION_VOICE_TRIGGER_USER_SWITCHED_OWNER",
            "ACTION_VOICE_TRIGGER_USER_SWITCHED_NON_OWNER",
            "ACTION_VOICE_TRIGGER_NOTIFY_RETRAIN"
        }
    };

    public static String getMainActionName(int action) {
        return sActionMainToStr[action - ACTION_MAIN_INTERNAL_TRIGGER];
    }

    public static String getSubActionName(int mainAction, int subAction) {
        int actionIndex = mainAction - ACTION_MAIN_INTERNAL_TRIGGER;
        if (actionIndex >= sActionSubToStr.length) {
            return "";
        } else {
            return sActionSubToStr[mainAction - ACTION_MAIN_INTERNAL_TRIGGER][subAction
                    - ACTION_MAIN_INTERNAL_TRIGGER];
        }
    }

    public static String sDumpMsg(VoiceMessage message) {
        if (message.mMainAction >= ACTION_MAIN_INTERNAL_TRIGGER) {
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
    private TriggerService mTriggerService;

    /**
     * VoiceTrigger constructor.
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
    public VoiceTriggerBusiness(IMessageDispatcher dispatcher, ConfigurationManager cfgMgr,
            Handler handler, IVoiceAdapter adapter, Context context) {
        super(dispatcher, cfgMgr, handler);
        Log.i(TAG, "[VoiceTriggerBusiness]new...");
        mIJniVoiceAdapter = adapter;
        mContext = context;
        mUpDispatcher = dispatcher;
        bindTriggerService();
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleSyncVoiceMessage]message = " + sDumpMsg(message));
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (!isTriggerSupport(mContext) || mCfgMgr.isDemoSupported()) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.i(TAG, "[handleSyncVoiceMessage]Voice Trigger feature is off, return!");
            sendMessageToApps(message, errorid);
            return errorid;
        }
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_START:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_ENABLE:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_DISABLE:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_QUERY_PARAM:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_GET_INITIAL_PARAMS:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_MODIFY_PARAM:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_SET_LOCALE:
        case ACTION_VOICE_TRIGGER_START_ADAPTER:
            errorid = sendMessageToHandler(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_GET_LOCALE:
            errorid = handleTriggerGetLocale(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_READ_STATUS:
            errorid = handleTriggerReadStatus(message);
            break;

        case ACTION_VOICE_TRIGGER_NOTIFY_RETRAIN:
            errorid = handleTriggerNotifyRetrain(message);

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
        if (!isTriggerSupport(mContext) || mCfgMgr.isDemoSupported()) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.i(TAG, "[handleAsyncVoiceMessage]Voice Trigger feature is off, return!");
            sendMessageToApps(message, errorid);
            return errorid;
        }
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_START:
            errorid = handleTriggerStart(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_ENABLE:
            errorid = handleTriggerEnable(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_DISABLE:
            errorid = handleTriggerDisable(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_INIT:
            errorid = handleTriggerInit(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_COMMAND_STATUS:
            errorid = handleTriggerCmdStatus(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_NOTIFY_ARRIVED:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_NOTIFY_DETECTED:
        case VoiceCommandListener.ACTION_VOICE_TRIGGER_NOTIFY_PACKAGE_CHANGED:
            errorid = handleTriggerNotify(message);
            break;

        case ACTION_VOICE_TRIGGER_USER_SWITCHED_OWNER:
            errorid = handleTriggerUserSwitchedOwner(message);
            break;

        case ACTION_VOICE_TRIGGER_USER_SWITCHED_NON_OWNER:
            errorid = handleTriggerUserSwitchedNonOwner(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_QUERY_PARAM:
            errorid = handleTriggerQueryParam(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_MODIFY_PARAM:
            errorid = handleTriggerModifyParam(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_SET_LOCALE:
            errorid = handleTriggerSetLocale(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRIGGER_GET_INITIAL_PARAMS:
            errorid = handleTriggerGetInitialParams(message);
            break;

        case ACTION_VOICE_TRIGGER_START_ADAPTER:
            errorid = handleTriggerStartAdapter(message);
            break;

        default:
            break;
        }
        Log.i(TAG, "[handleAsyncVoiceMessage]errorid = " + errorid);

        return errorid;
    }

    /**
     * Query trigger mode from setting provider.
     *
     * @param context
     *            context
     * @return trigger mode
     */
    public static int getTriggerMode(Context context) {
        int mode = Settings.System.getInt(context.getContentResolver(),
                VOICE_TRIGGER_MODE,
                VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER);
        Log.i(TAG,
                "[getTriggerMode]mode : "
                        + VoiceCommandListener.getWakeupModeStr(mode));

        return mode;
    }

    /**
     * Query trigger command status from setting provider.
     *
     * @param context
     *            context
     * @return trigger command status
     */
    public static int getTriggerCmdStatus(Context context) {
        int cmdStatus = Settings.System.getInt(context.getContentResolver(),
                VOICE_TRIGGER_COMMAND_STATUS,
                VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED);
        Log.i(TAG,
                "[getTriggerCmdStatus]cmdStatus : "
                        + VoiceCommandListener.getWakeupStatusStr(cmdStatus));

        return cmdStatus;
    }

    /**
     * get trigger enable status through trigger command status.
     *
     * @param cmdStatus
     *            trigger command status
     * @return trigger enable status
     */
    public static int getTriggerEnableStatus(int cmdStatus) {
        int triggerEnableStatus = 0;
        if ((cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED)
                || (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED)) {
            triggerEnableStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED;
        } else if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
            triggerEnableStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED;
        }
        Log.i(TAG, "[getTriggerEnableStatus]triggerEnableStatus: "
                + VoiceCommandListener.getWakeupStatusStr(triggerEnableStatus));

        return triggerEnableStatus;
    }

    /**
     * Save trigger status to setting provider.
     *
     * @param context
     *            context
     * @param cmdStatus
     *            trigger status
     */
    public static void setTriggerCmdStatus(Context context, int cmdStatus) {
        Log.i(TAG, "[setTriggerCmdStatus] to setting provider cmdStatus : "
                + VoiceCommandListener.getWakeupStatusStr(cmdStatus));
        Settings.System
                .putInt(context.getContentResolver(), VOICE_TRIGGER_COMMAND_STATUS, cmdStatus);
    }

    /**
     * Check if support voice trigger feature.
     *
     * @param context
     *            context
     * @return true if support, otherwise false
     */
    public static boolean isTriggerSupport(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (am == null) {
            Log.e(TAG, "[isTriggerSupport] get audio service is null");
            return false;
        }
        String state = am.getParameters(MTK_TRIGGER_SUPPORT_State);
        if (state != null) {
            return state.equalsIgnoreCase(MTK_TRIGGER_SUPPORT_on);
        }
        return false;
    }

    /**
     * Start the trigger business in native adapter.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerStart(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerStart]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        errorid = mIJniVoiceAdapter.startVoiceTrigger(message.mPkgName, message.pid);
        sendMessageToApps(message, errorid);
        mPakageName = message.mPkgName;
        return errorid;
    }

    /**
     * Modify the Setting preference to trigger startRecognition.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerEnable(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerEnable]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int force = 0;
        if (message.mExtraData != null) {
            force = message.mExtraData.getInt(
                    VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        }
        int wakeupCmdStatus = mCfgMgr.getTriggerCmdStatus();
        if (wakeupCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
        } else if (wakeupCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED) {
            //Loaded sound model. So proceed with trigger command status update.
            setTriggerCmdStatus(mContext, VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED);
        } else if (wakeupCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
            if (force == 1) {
                startRecognition(true);
            }
        }
        Log.d(TAG, "[handleTriggerEnable]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     * Modify the Setting preference to trigger stopRecognition.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerDisable(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerDisable]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int wakeupCmdStatus = mCfgMgr.getTriggerCmdStatus();
        int isReset = 0;
        if (message.mExtraData != null) {
            isReset = message.mExtraData.getInt(
                    VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        }
        if (wakeupCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
            if (isReset == 1) {
                setTriggerCmdStatus(
                        mContext,
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED);
            } else {
                setTriggerCmdStatus(
                        mContext,
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED);
            }
        }
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     * Handle Trigger business to init native part for recognition after boot.
     * Also, call start recognition if needed.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerInit(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerInit]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.d(TAG, "[handleTriggerInit] extra data is null");
            return errorid;
        }
        int mode = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        int cmdStatus = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);
        int[] cmdIds = message.mExtraData.getIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO2);
        String patternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(mode);
        String anyOnePatternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(
            VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT);
        String anyOnePasswordPath = mCfgMgr.getPasswordFilePath(
            VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT);
        String commandPatternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(
            VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT);
        String commandPasswordPath = mCfgMgr
                .getPasswordFilePath(VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT);
        String ubmPath = mCfgMgr.getUbmFilePath();
        String ubmUpgradePath = mCfgMgr.getUbmFileUpgradePath();
        String wakeupinfoPath = mCfgMgr.getWakeupInfoPath();
        if (patternPath == null || anyOnePatternPath == null || anyOnePasswordPath == null
                || commandPatternPath == null || commandPasswordPath == null || ubmPath == null
                || wakeupinfoPath == null) {
            Log.d(TAG, "[handleTriggerInit] error patternPath=" + patternPath
                    + " anyOnePatternPath=" + anyOnePatternPath + " anyOnePasswordPath="
                    + anyOnePasswordPath + " commandPatternPath=" + commandPatternPath
                    + " commandPasswordPath=" + commandPasswordPath + " ubmPath=" + ubmPath
                    + " ubmUpgradePath=" + ubmUpgradePath + "wakeupinfoPath=" + wakeupinfoPath);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        } else {
            int triggerEnableStatus = getTriggerEnableStatus(cmdStatus);
            mCfgMgr.setWakeupMode(mode);
            mCfgMgr.setTriggerStatus(cmdStatus);
            errorid = mIJniVoiceAdapter.initVoiceTrigger(mode, triggerEnableStatus, cmdIds,
                    patternPath, VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT,
                    anyOnePatternPath, anyOnePasswordPath,
                    VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT, commandPatternPath,
                    commandPasswordPath, ubmPath, ubmUpgradePath, wakeupinfoPath);
        }
        Log.d(TAG, "[handleTriggerInit]errorid = " + errorid);

        // After reboot, we need to check and startRecognition if not already activated.
        if (mCfgMgr.getTriggerCmdStatus() !=
                VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED) {
            loadModel(mCfgMgr.getTrainingLocale());
        }
        if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
            updateSoundTriggerRecognitionState(cmdStatus, false);
        }

        return errorid;
    }

    /**
     * Condition checker which checks the existing recognition status for restart request.
     *
     * @param recognitionStatus
     *            int value that is to be used to compare status.
     * @param recognitionFlagValue
     *            int value that is received from user app for restart recognition request.
     *
     * @return result
     */
    private boolean checkRecognitionStatusValueOn(
                    int recognitionStatus, int recognitionFlagValue) {
        return (recognitionFlagValue != 0) &&
            (recognitionStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED);
    }

    /**
     * Condition checker which checks the existing recognition status for restart request.
     *
     * @param recognitionStatus
     *            int value that is to be used to compare status.
     * @return result
     */
    private boolean checkRecognitionStatusValueOn(int recognitionStatus) {
        return checkRecognitionStatusValueOn(recognitionStatus, 1);
    }

    /**
     * Modify parameter values that impact Voice trigger recognition.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerModifyParam(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerModifyParam]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int param = 0;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.d(TAG, "[handleTriggerModifyParam] extra data is null");
        } else {
            param =
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            int parameterValue =
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);
            int isStartRecognitionReqd = /*1: require start recog, 0: not require start recog*/
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO2, 0);

            //Check threshold parameter is correct
            if (mCfgMgr.isValidTriggerConfidenceRange(parameterValue)) {
                //Update the value
                switch (param) {
                    case VoiceCommandListener.VOICE_TRIGGER_PARAM_COARSE_CONFIDENCE:
                        mCfgMgr.updateCoarseConfidenceToPref(parameterValue);
                        break;
                    case VoiceCommandListener.VOICE_TRIGGER_SECOND_STAGE_THRESHOLD:
                        mCfgMgr.updateSecondStageThresholdToPref(parameterValue);
                        break;
                    case VoiceCommandListener.VOICE_TRIGGER_ANTI_SPOOF_THRESHOLD:
                        //This is not supported as of now.
                        errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                        break;
                    default:
                        errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                        break;
                }
            } else {
                errorid = VoiceCommandListener.VOICE_ERROR_SETTING_VALUE_OUT_OF_RANGE;
            }

            if (VoiceCommandListener.VOICE_NO_ERROR == errorid
                    && getTriggerCmdStatus(mContext)
                    == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED
                    && (isStartRecognitionReqd == 1)) {
                stopRecognition();
                startRecognition();
            }
        }
        Log.d(TAG, "[handleTriggerModifyParam]errorid = " + errorid);
        sendMessageToApps(message, param, errorid);
        return errorid;
    }

    /**
     * Query the Setting parameter values that are getting used for trigger recognition.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerQueryParam(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerQueryParam]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = null;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            bundle = DataPackage.packageErrorResult(errorid);
            Log.d(TAG, "[handleTriggerQueryParam] extra data is null");
        } else {
            int param =
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            int parameterValue = 0;
            switch (param) {
                case VoiceCommandListener.VOICE_TRIGGER_PARAM_COARSE_CONFIDENCE:
                    //Query the stored value here
                    parameterValue = mCfgMgr.getCoarseConfidence();
                    break;
                case VoiceCommandListener.VOICE_TRIGGER_SECOND_STAGE_THRESHOLD:
                    //Query the stored value here
                    parameterValue = mCfgMgr.getSecondStageThreshold();
                    break;
                case VoiceCommandListener.VOICE_TRIGGER_ANTI_SPOOF_THRESHOLD:
                    //Not support
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                    break;
                default:
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                    break;
            }
            if (VoiceCommandListener.VOICE_NO_ERROR == errorid) {
                bundle = DataPackage.packageResultInfo(
                        VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                        param, parameterValue);
            } else {
                bundle = DataPackage.packageErrorResult(
                        param, errorid);
            }
        }
        message.mExtraData = bundle;
        Log.d(TAG, "[handleTriggerQueryParam]errorid = " + errorid);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
     *  Command status query command handler from status observer.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerCmdStatus(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerCmdStatus]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.d(TAG, "[handleTriggerCmdStatus] extra data is null");
            return errorid;
        }
        int wakeupCmdStatus = message.mExtraData
                .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        int triggerEnableStatus = getTriggerEnableStatus(wakeupCmdStatus);
        mCfgMgr.setTriggerStatus(wakeupCmdStatus);
        errorid = mIJniVoiceAdapter.sendVoiceWakeupCmdStatus(triggerEnableStatus);
        Log.d(TAG, "[handleTriggerCmdStatus]errorid = " + errorid);

        // Change Voice Trigger Service action.
        updateSoundTriggerRecognitionState(wakeupCmdStatus, true);

        return errorid;
    }

    /**
     * When User switched to non owner, need stop recognition if Trigger is enable.
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerUserSwitchedNonOwner(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int cmdStatus = getTriggerCmdStatus(mContext);
        Log.d(TAG, "[handleTriggerUserSwitchedNonOwner] cmdStatus: " + cmdStatus
                + ", UserId:" + UserHandle.myUserId());
        int newUserId = UserHandle.USER_SYSTEM + 1;
        if (message.mExtraData != null) {
            newUserId = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        }
        if (mTriggerService != null) {
            mTriggerService.setCurrentUserOnSwitch(newUserId);
        }
        return errorid;
    }

    /**
     * When user switch to owner, need start recognition if VoiceTrigger is enable.
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerUserSwitchedOwner(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int cmdStatus = getTriggerCmdStatus(mContext);
        Log.d(TAG, "[handleTriggerUserSwitchedOwner] cmdStatus: " + cmdStatus
                + ", UserId: " + UserHandle.myUserId());
        int newUserId = UserHandle.USER_SYSTEM;
        if (message.mExtraData != null) {
            newUserId = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
        }
        if (mTriggerService != null) {
            mTriggerService.setCurrentUserOnSwitch(newUserId);
        }
        return errorid;
    }

    /**
     *  Update the recognition sound models based on locale modification.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerSetLocale(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerSetLocale]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.d(TAG, "[handleTriggerSetLocale] extra data is null");
        } else {
            String locale =
                message.mExtraData.getString(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            // For locale setting, must reload model
            int isStartRecognitionReqd = 1;/*1 (default): req start recog, 0: not req start recog*/
                //message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, 1);
            String oldLocal = mCfgMgr.getTrainingLocale();
            Log.d(TAG, "locale:" + locale + " oldLocal=" + oldLocal);
            if (!oldLocal.equals(locale)) {
                if (!mTriggerService.hasModelByLocale(UserHandle.myUserId(),
                        locale)) {
                    // Fallback to default locale
                    errorid = VoiceCommandListener.VOICE_ERROR_SETTING_LANGUAGE_UPDATE;
                    locale = ConfigurationManager.VOW_ENROLLMENT_DEFAULT_LOCALE;
                }
                mCfgMgr.setTrainingLocale(locale);
                boolean needRestart = getTriggerCmdStatus(mContext) == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED
                        && (isStartRecognitionReqd == 1);
                if (needRestart) {
                    stopRecognition();
                }
                unloadModel(oldLocal);
                errorid = loadModel(locale) ? errorid
                        : VoiceCommandListener.VOICE_ERROR_SETTING_LANGUAGE_UPDATE;
                if (needRestart) {
                    startRecognition();
                }
            }
        }

        Log.d(TAG, "[handleTriggerSetLocale]errorid = " + errorid);
        sendMessageToApps(message, errorid);
        return errorid;

    }

    /**
     *  Get the saved locale string used for sound trigger recognition.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerGetLocale(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerGetLocale]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = null;

        String locale = mCfgMgr.getTrainingLocale();

        if (VoiceCommandListener.VOICE_NO_ERROR == errorid) {
            bundle = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                    locale, null);
        } else {
            bundle = DataPackage.packageErrorResult(
                    errorid);
        }
        message.mExtraData = bundle;

        Log.d(TAG, "[handleTriggerGetLocale]errorid = " + errorid);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
     *  Read and return directly the Trigger status value from Setting preference.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerReadStatus(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerReadStatus]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = null;

        int status = getTriggerCmdStatus(mContext);

        if (VoiceCommandListener.VOICE_NO_ERROR == errorid) {
            bundle = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                    status, null);
        } else {
            bundle = DataPackage.packageErrorResult(
                    errorid);
        }
        message.mExtraData = bundle;

        Log.d(TAG, "[handleTriggerReadStatus]errorid = " + errorid);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
     *  Return the parameters required for Adapter service after init.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerGetInitialParams(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerGetInitialParams]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = new Bundle();
        int status = getTriggerCmdStatus(mContext);
        String locale = mCfgMgr.getTrainingLocale();
        int recognitionThreshold = mCfgMgr.getCoarseConfidence();
        int utteranceCount = mIJniVoiceAdapter.getVoiceTrainingRepeatCount();
        VoiceWakeupInfo[] triggerInfo = mCfgMgr.getCurrentWakeupInfo(
                VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER);
        String[] utteranceString = triggerInfo[0].mKeyWord;
        ArrayList<Integer> usersList = mCfgMgr.getEnrolledUsers(status);

        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT,
            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS);
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO, status);
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1, recognitionThreshold);
        bundle.putString(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO2, locale);
        bundle.putInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO3, utteranceCount);
        bundle.putStringArray(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO4, utteranceString);
        bundle.putIntegerArrayList(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO5, usersList);

        message.mExtraData = bundle;

        Log.d(TAG, "[handleTriggerGetInitialParams]errorid = " + errorid);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
     *  Start the adapter service based on information received from trigger.xml.
     */
    private void startAdapterService() {
        VoiceWakeupInfo[] triggerInfo = mCfgMgr.getCurrentWakeupInfo(
                VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER);
        try {
            if (mTriggerService != null) {
                mTriggerService.addBackgroundWhitelist();
                Intent intent = new Intent();
                intent.setAction(triggerInfo[0].mActionName);
                intent.setPackage(triggerInfo[0].mPackageName);
                Log.d(TAG, "[startAdapterService] Starting action="
                        + triggerInfo[0].mActionName + ", package="
                        + triggerInfo[0].mPackageName);
                mContext.startService(intent);
            }
        } catch (Exception e) {
            //catch this error because when install HandsFreeAssistant,
            //it will forcestop first, so it maybe start many times.
            //And there has a time exception when start service the package is frozen.
            Log.e(TAG, "startAdapterService: ", e);
        }
    }

    /**
     *  Handle request to start adapter service.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTriggerStartAdapter(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerStartAdapter]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (mTriggerService == null) {
            VoiceMessage msg = new VoiceMessage();
            msg.mMainAction = VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER;
            msg.mSubAction = VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_START_ADAPTER;
            sendMessageToHandler(msg);
            Log.d(TAG, "[handleTriggerStartAdapter] reschedule");
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        } else {
            startAdapterService();
        }
        return errorid;
    }

    /**
     * Handle native retrain message.
     * @param message
     * @return
     */
    private int handleTriggerNotifyRetrain(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        //1. unEnroll old model from database
        if (mTriggerService == null) {
            Log.w(TAG,
                "[handleTriggerNotifyRetrain]mTriggerService is null, can not handle");
        }
        String locale = mCfgMgr.getTrainingLocale();
        boolean success = mTriggerService.unEnrollSoundModel(UserManager.get(
                mContext).getUserHandle(), locale);
        if (success) {
            // 2. Enroll new model
            int trainingMode = mCfgMgr.getWakeupMode();
            String patternPath = mCfgMgr
                    .getVoiceRecognitionPatternFilePath(trainingMode);
            VoiceWakeupInfo[] wakeupInfos = mCfgMgr
                    .getCurrentWakeupInfo(trainingMode);
            int commandId = wakeupInfos[0].mID;
            mTriggerService.enrollSoundModel(mCfgMgr
                    .getOverrideTriggerToWakeupMode(trainingMode), commandId,
                    patternPath, UserManager.get(mContext).getUserHandle(), locale);
        } else {
            Log.w(TAG, "[handleTriggerNotifyRetrain]unEnrollSoundModel fail");
        }
        //3. Load new model
        errorid = loadModel(locale) ? errorid
                : VoiceCommandListener.VOICE_ERROR_SETTING_LANGUAGE_UPDATE;

        boolean needRestart = getTriggerCmdStatus(mContext) ==
                VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED;
        if (needRestart) {
            startRecognition();
        }
        Log.d(TAG, "handleTriggerNotifyRetrain locale: " +  locale + ", needRestart: " + needRestart
                + ", errorid: " + errorid);
        return errorid;
    }

    /**
     *  Bind the sound trigger user service.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private void bindTriggerService() {
        Log.d(TAG, "[bindTriggerService]... ");
        Intent intent = new Intent(mContext, TriggerService.class);
        if (!mContext.bindService(intent, mTriggerConnection, Context.BIND_AUTO_CREATE)) {
            Log.d(TAG, "Fail to bind service");
        }
    }

    /**
     * Trigger service connection.
     */
    private ServiceConnection mTriggerConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mTriggerService = ((TriggerBinder)service).getService();
            Log.d(TAG, "[ServiceConnection.onServiceConnected]..., srv:" + mTriggerService);
            mTriggerService.registerWakeupCallback(mTriggerCallback);
            mTriggerService.registerPackageUpdateCallback(mPackageUpdateCallback);
            mTriggerService.registerVoiceModelUpdateCallback(mVoiceModelUpdateCallback);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "[ServiceConnection.onServiceDisconnected]... ");
            mTriggerService.unRegisterWakeupCallback();
            mTriggerService.unRegisterPackageUpdateCallback();
            mTriggerService.unRegisterVoiceModelUpdateCallback();
            mTriggerService = null;
        }
    };

    /**
     *  Package update callback.
     */
    private IPackageUpdateCallback mPackageUpdateCallback = new IPackageUpdateCallback() {
        @Override
        public void onPackageUpdateDetected(String pkgName, String action) {
            VoiceMessage message = new VoiceMessage();
            VoiceWakeupInfo[] triggerInfo = mCfgMgr.getCurrentWakeupInfo(
                VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER);
            Log.d(TAG, "[onPackageUpdateDetected] package : " + pkgName + "action : " + action);
            if (pkgName.equals(triggerInfo[0].mPackageName)) {
                //Action is ACTION_PACKAGE_REPLACED, ACTION_PACKAGE_ADDED,
                //ACTION_PACKAGE_CHANGED.
                startAdapterService();
                return;
            }

            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER;
            message.mSubAction =
                VoiceCommandListener.ACTION_VOICE_TRIGGER_NOTIFY_PACKAGE_CHANGED;
            message.mPkgName = mPakageName;
            message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                    pkgName, action);
            sendMessageToHandler(message);
        }
    };

    /**
     * It is used to receive VoiceTrigger callback
     */
    private IVoiceWakeupCallback mTriggerCallback = new IVoiceWakeupCallback() {
        @Override
        public void onVoiceWakeupDetected(int subAction, int confidence, Bundle bundle) {
            if (subAction == ACTION_VOICE_TRIGGER_NOTIFY_AUDIOERROR) {
                String locale = mCfgMgr.getTrainingLocale();
                loadModel(locale);
                boolean needRestart = getTriggerCmdStatus(mContext) ==
                        VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED;
                if (needRestart) {
                    startRecognition();
                }
                Log.d(TAG, "[onVoiceWakeupDetected] locale: " + locale
                        + ", needRestart: " + needRestart);
                return;
            }
            // Need to handle message in Main thread.
            VoiceMessage message = new VoiceMessage();
            message.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER;
            message.mSubAction = subAction;
            message.mPkgName = mPakageName;
            int wakeupMode = VoiceTriggerBusiness.getTriggerMode(mContext);
            message.mExtraData = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, confidence, wakeupMode);
            message.mExtraData.putBundle("bundle", bundle);
            Log.d(TAG, "[onVoiceWakeupDetected] msg:" + message
                    + ", confidence: " + confidence + ", wakeupMode : "
                    + wakeupMode);
            sendMessageToHandler(message);
        }
    };

    /**
     * It is used to receive VoiceTrigger callback when voice model update
     */
    private IVoiceModelUpdateCallback mVoiceModelUpdateCallback = new IVoiceModelUpdateCallback() {
        @Override
        public void onVoiceModelUpdate() {
            Log.d(TAG, "[onVoiceModelUpdate]");
            int triggerCmdStatus = getTriggerCmdStatus(mContext);
            if (triggerCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
                stopRecognition();
            }
            if (triggerCmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED
                    || triggerCmdStatus ==
                    VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED) {
                //First unload model
                unloadModel(mCfgMgr.getTrainingLocale());
                // Then notify native to update
                int commandId = 0; //set always 0, beacuse in voiceadapter is always set 0
                int trainingMode = getTriggerMode(mContext);
                String pwdpath = mCfgMgr.getPasswordFilePath(trainingMode);
                String patternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(trainingMode);
                String umbPath = mCfgMgr.getUbmFilePath();
                String ubmUpgradePath = mCfgMgr.getUbmFileUpgradePath();
                String wakeupinfoPath = mCfgMgr.getWakeupInfoPath();
                mIJniVoiceAdapter.setVoiceModelRetrain(patternPath, umbPath, ubmUpgradePath,
                        commandId, mCfgMgr.getOverrideTriggerToWakeupMode(trainingMode),
                        wakeupinfoPath);
            }
        }
    };

    /**
     * Update SoundTrigger service recognition state based on command status modification.
     *
     * @param cmdStatus command status.
     * @param isForceUpdate flag for indicating special condition for stopRecognition.
     */
    private void updateSoundTriggerRecognitionState(int cmdStatus, boolean isForceUpdate) {
        Log.d(TAG,
                "[updateSoundTriggerRecognitionState] requested cmdStatus=" + cmdStatus
                + ", existing status=" + sRecognitionStatus);

        if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED) {
            startRecognition();
        } else if (isForceUpdate) { //when settings update, then force stop recognition for reset
            stopRecognition();
        } else if (cmdStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED) {
            stopRecognition();
        }
    }

    private boolean startRecognition() {
        return startRecognition(false);
    }

    /**
     * Start recognition, invoke VoiceTriggerInteractionService.startRecognition().
     *
     * @return true if success.
     */
    private boolean startRecognition(boolean force) {
        boolean success = false;
        if (mTriggerService != null
                && (force ||
                (VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED ==
                        getTriggerCmdStatus(mContext)))) {
            success = mTriggerService.startRecognition(UserHandle.myUserId(),
                    mCfgMgr.getCoarseConfidence(),
                    mCfgMgr.getSecondStageThreshold());
        } else if (mTriggerService == null) {
            Log.d(TAG,
                    "[startRecognition]mTriggerService is null, can not startRecognition");
        } else {
            Log.d(TAG,
                    "[startRecognition]recognition status is VOICE_WAKEUP_STATUS_COMMAND_CHECKED.");
        }
        if (success) {
            sRecognitionStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_CHECKED;
        }
        return success;
    }

    /**
     * Stop recognition, invoke VoiceTriggerService.stopRecognition().
     *
     * @return true if success.
     */
    private boolean stopRecognition() {
        boolean success = false;
        if (mTriggerService != null
                && (VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED !=
                getTriggerCmdStatus(mContext))) {
            success = mTriggerService.stopRecognition(UserHandle.myUserId());
        } else if (mTriggerService == null) {
            Log.d(TAG,
                    "[stopRecognition]mTriggerService is null, can not stopRecognition");
        } else {
            Log.d(TAG,
                    "[stopRecognition]recognition status is" +
                            " VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED.");
        }
        if (success) {
            sRecognitionStatus = VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED;
        }
        return success;
    }

    /**
     *  Load sound model for recognition.
     *
     * @param locale language_country combination in String form
     *
     * @return true if success
     */
    private boolean loadModel(String locale) {
        boolean success = false;
        if (mTriggerService != null) {
            success = mTriggerService.loadModel(UserHandle.myUserId(), locale);
        } else {
            Log.d(TAG, "loadModel: mTriggerService is null");
        }
        return success;
    }

    /**
     *  Unload sound model for recognition.
     *
     * @param locale language_country combination in String form
     *
     * @return true if success
     */
    private void unloadModel(String locale) {
        if (mTriggerService != null) {
            mTriggerService.unloadModel(UserHandle.myUserId(), locale);
        } else {
            Log.d(TAG, "loadModel: mTriggerService is null");
        }
    }

    /**
     * Dispatch wakeup recognition result to App.
     * @param message voice message
     * @return status
     */
    private int handleTriggerNotify(VoiceMessage message) {
        Log.d(TAG, "[handleTriggerNotify]...");
        return mUpDispatcher.dispatchMessageUp(message);
    }

    /**
     * Handle data release. Here is used to unbind VoiceTriggerInteractionService.
     */
    @Override
    public void handleDataRelease() {
        Log.d(TAG, "[handleDataRelease]...");
        if (mTriggerService != null) {
            mContext.unbindService(mTriggerConnection);
            mTriggerService = null;
        }
    }
}
