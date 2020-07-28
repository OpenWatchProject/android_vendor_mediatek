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

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;

import android.os.IBinder;
import android.os.RemoteException;

import android.content.Context;
import android.hardware.soundtrigger.SoundTrigger.Keyphrase;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel;
import android.os.Bundle;
import android.os.Handler;
import android.os.UserManager;
import android.widget.Toast;

import com.android.internal.util.ArrayUtils;

import com.mediatek.common.voicecommand.IVoiceTrainingEnrollmentService;
import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.cfg.VoiceWakeupInfo;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.service.TriggerService;
import com.mediatek.voicecommand.service.TriggerService.TriggerBinder;
import com.mediatek.voicecommand.util.Log;

public class VoiceTrainingBusiness extends VoiceCommandBusiness {
    private static final String TAG = "VoiceTrainingBusiness";
    private IVoiceAdapter mIJniVoiceAdapter;
    private Context mContext;
    private int mTrainingMode = -1;
    private int mCommandId;
    private String mPatternPath;
    private IVoiceTrainingEnrollmentService mVISEnrollmentService;
    private TriggerService mTriggerService;
    private int mUserId = 0;

    public VoiceTrainingBusiness(IMessageDispatcher dispatcher, ConfigurationManager cfgMgr,
            Handler handler, IVoiceAdapter adapter, Context context) {
        super(dispatcher, cfgMgr, handler);
        Log.i(TAG, "[VoiceTrainingBusiness]new...");
        mIJniVoiceAdapter = adapter;
        mContext = context;
        if (mCfgMgr.isDemoSupported()) {
            bindVoiceTrainingEnrollmentService();
        } else {
            bindTriggerService();
        }
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleSyncVoiceMessage]message = " + message);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_TRAINING_START:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_STOP:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_RESET:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_MODIFY:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_FINISH:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_CONTINUE:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_PAUSE:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_INIT:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_MODIFY_PARAM:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_QUERY_PARAM:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_UPDATE_MODEL:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_ENROLL_START:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_GET_USER_LIST:
        case VoiceCommandListener.ACTION_VOICE_TRAINING_CHANGE_KEYPHRASE:
            sendMessageToHandler(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_PASSWORD_FILE:
            getPasswordFilePath(message);
            break;

        default:
            // do nothing because illegal action has been filtered in AppDataManager
            break;
        }

        return errorid;
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleAsyncVoiceMessage]message = " + message);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_TRAINING_START:
            handleTrainingStart(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_INTENSITY:
            handleTrainingIntensity(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_STOP:
            handleTrainingStop(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_RESET:
            handleTrainingReset(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_MODIFY:
            handleTrainingModify(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_FINISH:
            handleTrainingFinish(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_CONTINUE:
            handleTrainingContinue(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_INIT:
            handleTrainingInit(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_PAUSE:
            handleTrainingPause(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_MODIFY_PARAM:
            handleTrainingModifyParam(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_QUERY_PARAM:
            handleTrainingQueryParam(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_UPDATE_MODEL:
            handleTrainingUpdateModel(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_ENROLL_START:
            handleTrainingEnrollStart(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_GET_USER_LIST:
            handleTrainingGetUserList(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_TRAINING_CHANGE_KEYPHRASE:
            handleTrainingChangeKeyphrase(message);
            break;

        default:
            break;
        }

     return errorid;
    }

    /**
     *  Voice training start. This initialises and calls first screen of training flow.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingStart(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingStart]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int commandid = 0;
        int trainingMode = 0;
        String patternpath = null;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
        } else {
            commandid = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            trainingMode = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);
            int[] commandMask = message.mExtraData
                    .getIntArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO2);
            if (commandid < 0) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            } else {
                String pwdpath = mCfgMgr.getPasswordFilePath(trainingMode);
                patternpath = mCfgMgr.getVoiceRecognitionPatternFilePath(trainingMode);
                String featurepath = mCfgMgr.getFeatureFilePath(trainingMode);
                String umbpath = mCfgMgr.getUbmFilePath(commandid);
                String ubmUpgradePath = mCfgMgr.getUbmFileUpgradePath();
                String wakeupinfoPath = mCfgMgr.getWakeupInfoPath();
                if (pwdpath == null || patternpath == null || featurepath == null
                        || umbpath == null || wakeupinfoPath == null) {
                    Log.d(TAG, "[handleTrainingStart] error pwdpath=" + pwdpath + " patternpath="
                            + patternpath + " featurepath=" + featurepath + " umbpath=" + umbpath
                            + " wakeupinfoPath =" + wakeupinfoPath);
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                } else {
                    errorid = mIJniVoiceAdapter.startVoiceTraining(pwdpath, patternpath,
                            featurepath, umbpath, ubmUpgradePath, commandid, commandMask,
                            mCfgMgr.getOverrideTriggerToWakeupMode(trainingMode), trainingMode,
                            wakeupinfoPath, message.mPkgName, message.pid);
                }
            }
        }
        Log.d(TAG, "[handleTrainingStart]errorid = " + errorid);
        // Set these info for enroll sound model.
        setModeAndCommand(trainingMode, commandid, patternpath, false);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     *  Handle the Training stop action i.e. reset the training state.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingStop(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingStop]...");
        int errorId = VoiceCommandListener.VOICE_NO_ERROR;
        errorId = mIJniVoiceAdapter.stopVoiceTraining(message.mPkgName, message.pid);
        message.mExtraData = DataPackage.packageSuccessResult();
        mDispatcher.dispatchMessageUp(message);
        return errorId;
    }

    /**
     *  Handle getter for training intensity. This works on polling mechanism.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingIntensity(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingIntensity]...");
        int intensity = mIJniVoiceAdapter.getNativeIntensity();
        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, intensity, 0);

        mDispatcher.dispatchMessageUp(message);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    /**
     *  Read the password file corresponding to input command and training mode.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int getPasswordFilePath(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = null;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            bundle = DataPackage.packageErrorResult(errorid);
        } else {
            int commandid = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            int trainingMode = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);
            String passwordpath = mCfgMgr.getPasswordFilePath(trainingMode);
            if (commandid < 0 || passwordpath == null) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                bundle = DataPackage.packageErrorResult(errorid);
            } else {
                String path = passwordpath + commandid + ".dat";
                File file = new File(path);
                if (file.exists()) {
                    bundle = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, path, null);
                } else {
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                    bundle = DataPackage.packageErrorResult(errorid);
                }
            }
        }
        message.mExtraData = bundle;
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
    *  Read the commandId list that is available.
    *
    * @param fileNameList
    *            File name list
    * @param end
    *            Ending character list
    *
    * @return commandIdList
    */
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

    /**
    *  Get directory list based on input path.
    *
    * @param path
    *            path
    *
    * @return directory list
    */
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

    /**
     *  Handle request for resetting the training data. This calls unenroll of Sound model.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingReset(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingReset]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int commandid = -1;
        int trainingMode = -1;
        int isNeedUpdateCommandId = 0;
        String patternPath = null;
        Bundle bundle = null;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
        } else {
            commandid = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            trainingMode = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);
            if (commandid < 0) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            } else {
                String pwdpath = mCfgMgr.getPasswordFilePath(trainingMode);
                String featurepath = mCfgMgr.getFeatureFilePath(trainingMode);
                patternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(trainingMode);

                if (pwdpath == null || patternPath == null || featurepath == null) {
                    Log.d(TAG, "[handleTrainingReset] error pwdpath=" + pwdpath + " patternpath="
                            + patternPath + " featurepath=" + featurepath);
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                } else {
                    errorid = mIJniVoiceAdapter.resetVoiceTraining(pwdpath, patternPath,
                            featurepath, commandid);
                }
            }
        }
        Log.d(TAG, "[handleTrainingReset]errorid = " + errorid);
        int[] commandIds = getCommandIdList(getDirectory(patternPath), ".dat");
        // Only Voice Wakeup training should unEnroll sound model.
        if ((errorid == VoiceCommandListener.VOICE_NO_ERROR)
                && (trainingMode != VoiceCommandListener.VOICE_WAKEUP_MODE_UNLOCK)) {
            if ((trainingMode == VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT)
                && (commandIds.length > 0)) {
                setModeAndCommand(trainingMode, commandIds[0], patternPath);
                if (!enrollSoundModel()) {
                    errorid =  VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                    Log.e(TAG, "[handleTrainingReset] Critical issue error = " + errorid);
                }

                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    isNeedUpdateCommandId = 1;
                    bundle = DataPackage.packageResultInfo(
                            VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS,
                            isNeedUpdateCommandId, commandIds[0]);
                }
            } else {
                setModeAndCommand(trainingMode, commandid, patternPath);
                unEnrollSoundModel();
                mCfgMgr.resetEnrolledUser(mUserId);
            }
        }
        if (isNeedUpdateCommandId == 0) {
            sendMessageToApps(message, errorid);
        } else {
            message.mExtraData = bundle;
            mDispatcher.dispatchMessageUp(message);
        }

        return errorid;
    }

    /**
     *  Handle training modify request.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingModify(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingModify]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
        } else {
            int commandid = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            int trainingMode = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);
            if (commandid < 0) {
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            } else {
                String pwdpath = mCfgMgr.getPasswordFilePath(trainingMode);
                String patternpath = mCfgMgr.getVoiceRecognitionPatternFilePath(trainingMode);
                String featurepath = mCfgMgr.getFeatureFilePath(trainingMode);

                if (pwdpath == null || patternpath == null || featurepath == null) {
                    Log.d(TAG, "[handleTrainingModify] error pwdpath=" + pwdpath + " patternpath="
                            + patternpath + " featurepath=" + featurepath);
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                } else {
                    errorid = mIJniVoiceAdapter.modifyVoiceTraining(pwdpath, patternpath,
                            featurepath, commandid);
                }
            }
        }
        Log.d(TAG, "[handleTrainingModify]errorid = " + errorid);
        sendMessageToApps(message, errorid);
        return errorid;
    }

    /**
     *  Handle Training finish request.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingFinish(VoiceMessage message) {
        int errorid = mIJniVoiceAdapter.finishVoiceTraining(message.mPkgName, message.pid);
        Log.d(TAG, "[handleTrainingFinish]errorid = " + errorid);

        return errorid;
    }

    /**
     *  Handle request to start enroll Sound model.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingEnrollStart(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.d(TAG, "[handleTrainingEnrollStart]...");
        // Only Voice Wakeup training should enroll sound model.
        if ((errorid == VoiceCommandListener.VOICE_NO_ERROR)
                && (mTrainingMode != VoiceCommandListener.VOICE_WAKEUP_MODE_UNLOCK)) {
            setModeAndCommand(mTrainingMode, mCommandId, mPatternPath);
            if (!enrollSoundModel()) {
                errorid =  VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            }

            // Since loaded sound model, so update the setting value.
            if (mCfgMgr.isDemoSupported()) {
                if ((VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT == mTrainingMode
                    || VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT == mTrainingMode)
                    && VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED ==
                    mCfgMgr.getWakeupCmdStatus()) {
                    VoiceWakeupBusiness.setWakeupCmdStatus(
                        mContext, VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED);
                }
            } else {
                if (VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER == mTrainingMode
                    && VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED ==
                        mCfgMgr.getTriggerCmdStatus()) {
                    VoiceTriggerBusiness.setTriggerCmdStatus(
                        mContext, VoiceCommandListener.VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED);
                }
            }
            mCfgMgr.setEnrolledUser(mUserId);
        }
        message.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_NOTIFY_FINISH;
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
     *  Handle request for training screens following the first iteration.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingContinue(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        errorid = mIJniVoiceAdapter.continueVoiceTraining(message.mPkgName, message.pid);
        Log.d(TAG, "[handleTrainingContinue]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     *  Handle request to cancel a single iteration of training.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingPause(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        errorid = mIJniVoiceAdapter.pauseVoiceTraining(message.mPkgName, message.pid);
        Log.d(TAG, "[handleTrainingPause]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     *  Handle training init request. This will receive the User ID to be used in training flow.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingInit(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.d(TAG, "[handleTrainingInit] extra data is null");
        } else {
            mUserId = message.mExtraData.getInt(
                    VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 1);
            mCfgMgr.updateTrainingConfidenceToPref(mIJniVoiceAdapter
                    .getVoiceTrainingThreshold());
            mIJniVoiceAdapter.initVoiceTraining(mCfgMgr.isDemoSupported());
        }
        /*Store this information*/
        /*Setup the bind part with the service here */
        /*Can think to shift some of the parameters for StartRecogition here and call JNI Adapter*/
        Log.d(TAG, "[handleTrainingInit]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     *  Handle request for modification of Training setting parameters.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingModifyParam(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingModifyParam]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int param = 0;
        int parameterValue = 0;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            Log.d(TAG, "[handleTrainingModifyParam] extra data is null");
        } else {
            param =
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            parameterValue =
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1);

            switch (param) {
                case VoiceCommandListener.VOICE_TRAINING_PARAM_CONFIDENCE_THRESHOLD:
                    //Set error if out of range value 0-100.
                    if (mCfgMgr.isValidTriggerConfidenceRange(parameterValue)) {
                        errorid = mIJniVoiceAdapter.setVoiceTrainingThreshold(parameterValue);
                        //Should check for success of JNI command
                        mCfgMgr.updateTrainingConfidenceToPref(parameterValue);
                    } else {
                        errorid = VoiceCommandListener.VOICE_ERROR_SETTING_VALUE_OUT_OF_RANGE;
                    }
                    break;
                case VoiceCommandListener.VOICE_TRAINING_PARAM_REPEAT_TIMES:
                    //Set error if out of range value.
                    //Can use a setter if available from Native. Till not, just return error.
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                    break;
                case VoiceCommandListener.VOICE_TRAINING_PARAM_TIMEOUT:
                    //mCfgMgr.setVoiceTrainingTimeout(parameterValue);
                    errorid = mIJniVoiceAdapter.setVoiceTrainingTimeout(parameterValue);
                    break;
                default:
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
                break;
            }
        }
        Log.d(TAG, "[handleTrainingModifyParam]errorid = " + errorid);
        sendMessageToApps(message, param, errorid);
        return errorid;
    }

    /**
     * Query the training setting parameters from stored value/JNI part.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingQueryParam(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingQueryParam]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = null;
        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
            bundle = DataPackage.packageErrorResult(errorid);
            Log.e(TAG, "[handleTrainingQueryParam]extra data is null");
        } else {
            int param =
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            int commandId =
                message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO1, 0);
            int parameterValue = 0;
            // We need init mTrainingMode, if do not training after boot device, this value is 0,
            // but handleTrainingQueryParam maybe called with this value
            if (mTrainingMode == -1) {
                mTrainingMode = mCfgMgr.getWakeupMode();
            }
            Log.d(TAG, "mTrainingMode=" + mTrainingMode);
            VoiceWakeupInfo[] triggerInfo = mCfgMgr.getCurrentWakeupInfo(mTrainingMode);
            String[] utteranceString = triggerInfo[commandId].mKeyWord;
            switch (param) {
                case VoiceCommandListener.VOICE_TRAINING_PARAM_CONFIDENCE_THRESHOLD:
                    parameterValue = mCfgMgr.getTrainingThreshold();
                    break;
                case VoiceCommandListener.VOICE_TRAINING_PARAM_REPEAT_TIMES:
                    parameterValue = mIJniVoiceAdapter.getVoiceTrainingRepeatCount();
                    break;
                case VoiceCommandListener.VOICE_TRAINING_PARAM_TIMEOUT:
                    //Can use a getter if available from Native. Till not, just return error.
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
                if (VoiceCommandListener.VOICE_TRAINING_PARAM_REPEAT_TIMES == param) {
                    bundle.putStringArray(
                        VoiceCommandListener.ACTION_EXTRA_RESULT_INFO2, utteranceString);
                }
                Log.d(TAG, "[handleTrainingQueryParam]parameter = " + param
                    + ", Value" + parameterValue);
            } else {
                bundle = DataPackage.packageErrorResult(param, errorid);
                Log.e(TAG, "[handleTrainingQueryParam]errorid = " + errorid);
            }
        }
        message.mExtraData = bundle;
        mDispatcher.dispatchMessageUp(message);
        return errorid;
    }

    /**
     *  Handle request to fetch the enrolled User list.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingGetUserList(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingGetUserList]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Bundle bundle = null;

        ArrayList<Integer> usersList = mCfgMgr.getEnrolledUsers(mCfgMgr.getTriggerCmdStatus());
        int[] users = ArrayUtils.convertToIntArray(usersList);

        if (usersList.isEmpty()) {
            bundle = DataPackage.packageErrorResult(-1, errorid);
        } else {
            bundle = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, null, users);
        }
        message.mExtraData = bundle;

        Log.d(TAG, "[handleTrainingGetUserList]errorid = " + errorid);
        mDispatcher.dispatchMessageUp(message);

        return errorid;
    }

    /**
     *  Handle special request to update the sound model for testing.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingUpdateModel(VoiceMessage message) {
        Log.d(TAG, "[handleTrainingUpdateModel]...");
        int trainingMode = mCfgMgr.getWakeupMode();
        String patternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(trainingMode);
        VoiceWakeupInfo[] wakeupInfos = mCfgMgr.getCurrentWakeupInfo(trainingMode);
        int commandId = wakeupInfos[mCfgMgr.getActiveCommandId()].mID;
        int errorid =
            mIJniVoiceAdapter.modifyPresetVoiceTraining(patternPath, commandId);

        //Set default training mode and commandId
        setModeAndCommand(trainingMode, commandId, patternPath);

        if ((errorid == VoiceCommandListener.VOICE_NO_ERROR) &&
            (mTrainingMode != VoiceCommandListener.VOICE_WAKEUP_MODE_UNLOCK)) {
            enrollSoundModel();
        }
        sendMessageToApps(message, errorid);

        return errorid;
    }

    /**
     *  Handle request to change the keyphrase for testing.
     *
     * @param message
     *            VoiceMessage instance
     *
     * @return result
     */
    private int handleTrainingChangeKeyphrase(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int trainingMode = mTrainingMode;
        int commandId = 0;
        Bundle bundle = null;
        int isAlreadyEnrolled = 0;

        if (message.mExtraData != null) {
            commandId = message.mExtraData.getInt(
                VoiceCommandListener.ACTION_EXTRA_SEND_INFO, 0);
        }
        Log.d(TAG, "[handleTrainingChangeKeyphrase]... commandId = " + commandId);

        String pwdpath = mCfgMgr.getPasswordFilePath(trainingMode);
        String patternPath = mCfgMgr.getVoiceRecognitionPatternFilePath(trainingMode);
        String umbpath = mCfgMgr.getUbmFilePath(commandId);
        String ubmUpgradePath = mCfgMgr.getUbmFileUpgradePath();

        if (pwdpath == null || patternPath == null) {
            Log.d(TAG, "[handleTrainingChangeKeyphrase] error pwdpath=" + pwdpath
                + " patternPath=" + patternPath + " umbpath" + umbpath);
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
        } else if (commandId < 0 || pwdpath == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_DATA;
        } else {
            String path = pwdpath + commandId + ".dat";
            File file = new File(path);
            if (file.exists()) {
                //enroll new data here
                if (mTrainingMode != VoiceCommandListener.VOICE_WAKEUP_MODE_UNLOCK) {
                    Log.d(TAG, "[handleTrainingChangeKeyphrase]... enrolling model" + path);
                    //Set default training mode and commandId
                    setModeAndCommand(trainingMode, commandId, patternPath);
                    enrollSoundModel();
                    isAlreadyEnrolled = 1; //Can maintain for all command id
                }
            } else {
                //user need to train again. so send input to JNI adapter
                errorid = mIJniVoiceAdapter.modifyVoiceTrainingKeyphrase(umbpath,
                    ubmUpgradePath, commandId, trainingMode);
                Log.d(TAG, "[handleTrainingChangeKeyphrase]... need to train new model");
            }
        }
        Log.d(TAG, "[handleTrainingChangeKeyphrase]errorid = " + errorid);
        if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
            bundle = DataPackage.packageErrorResult(-1, errorid);
        } else {
            bundle = DataPackage.packageResultInfo(
                    VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, isAlreadyEnrolled,
                    commandId);
        }
        message.mExtraData = bundle;

        mDispatcher.dispatchMessageUp(message);
        // In return value we may send some content. So, will check this later

        return errorid;
    }

    /**
     * Save training mode, command id and pattern path, they will be used in
     * enroll sound model.
     *
     * @param traningMode training mode
     * @param commandId command id
     * @param patternPath pattern path
     */
    private void setModeAndCommand(int traningMode, int commandId, String patternPath) {
        setModeAndCommand(traningMode, commandId, patternPath, true);
    }

    /**
     * Save training mode, command id and pattern path, they will be used in
     * enroll sound model.
     *
     * @param traningMode training mode
     * @param commandId command id
     * @param patternPath pattern path
     */
    private void setModeAndCommand(int traningMode, int commandId,
            String patternPath, boolean updateCommandId) {
        Log.d(TAG, "[setModeAndCommand] trainingMode: " + traningMode +
            "commandId: " + commandId + "patternPath: " + patternPath);
        mTrainingMode = traningMode;
        mPatternPath = patternPath;
        if (updateCommandId == true) {
            mCfgMgr.setActiveCommandId(commandId);
        } else {
            mCommandId = commandId; //local copy of commandId
        }
    }

    /**
    * Bind Voice Training Enrollment service.
    */
    public void bindVoiceTrainingEnrollmentService() {
        Log.d(TAG, "[bindVoiceTrainingEnrollmentService]... ");
        Intent intent = new Intent();
        intent.setAction(VoiceCommandListener.VOICE_TRAINING_SERVICE_ACTION);
        intent.setPackage(VoiceCommandListener.VOICE_TRAINING_SERVICE_PACKAGE_NAME);
        if (!mContext.bindService(intent, mVISConnection, Context.BIND_AUTO_CREATE)) {
            Log.d(TAG, "Fail to bind service");
        }
    }

    /**
     * VoiceWakeupInteractionService connection.
     */
    private ServiceConnection mVISConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "[mVISConnection.onServiceConnected]... ");
            mVISEnrollmentService = IVoiceTrainingEnrollmentService.Stub.asInterface(service);
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "[mVISConnection.onServiceDisconnected]... ");
            mVISEnrollmentService = null;
        }
    };

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
     *  Service connection for Trigger service.
     */
    private ServiceConnection mTriggerConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.d(TAG, "[mTriggerConnection.onServiceConnected]... ");
            mTriggerService = ((TriggerBinder)service).getService();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "[mTriggerConnection.onServiceDisconnected]... ");
            mTriggerService = null;
        }
    };

    /**
     * Called when the user training success. Performs a fresh enrollment.
     */
    private boolean enrollSoundModel() {
        boolean success = false;
        if (mCfgMgr.isDemoSupported()) {
            try {
                if (mVISEnrollmentService != null) {
                    success = mVISEnrollmentService.enrollSoundModel(
                        mCfgMgr.getOverrideTriggerToWakeupMode(mTrainingMode),
                        mCfgMgr.getActiveCommandId(), mPatternPath,
                        UserManager.get(mContext).getUserHandle()/* current user*/);
                } else {
                    Log.e(TAG,
                        "[enrollSoundModel]mVISEnrollmentService is null, can not enroll");
                }
            } catch (RemoteException e) {
                Log.e(TAG, "[enrollSoundModel]exception:" + e.getMessage());
            }
        } else {
            if (mTriggerService != null) {
                String locale = mCfgMgr.getTrainingLocale();
                success = mTriggerService.enrollSoundModel(mCfgMgr
                        .getOverrideTriggerToWakeupMode(mTrainingMode),
                        mCfgMgr.getActiveCommandId(), mPatternPath,
                        UserManager.get(mContext).getUserHandle(), locale);
            } else {
                Log.e(TAG,
                    "[enrollSoundModel]mTriggerService is null, can not enroll");
            }
        }
        return success;
    }

    /**
     * Called when the training reset.
     * Clears the enrollment information for the user.
     */
    private boolean unEnrollSoundModel() {
        boolean success = false;
        if (mCfgMgr.isDemoSupported()) {
            try {
                if (mVISEnrollmentService != null) {
                    success = mVISEnrollmentService.unEnrollSoundModel();
                } else {
                    Log.e(TAG,
                        "[unEnrollSoundModel]mVISEnrollmentService is null, can not unEnroll");
                }
            } catch (RemoteException e) {
                Log.e(TAG, "[unEnrollSoundModel]exception:" + e.getMessage());
            }
        } else {
            if (mTriggerService != null) {
                String locale = mCfgMgr.getTrainingLocale();
                success = mTriggerService.unEnrollSoundModel(
                    UserManager.get(mContext).getUserHandle(), locale);
            } else {
                Log.e(TAG,
                    "[unEnrollSoundModel]mTriggerService is null, can not unEnroll");
            }
        }
        return success;

    }

    /**
     * Handle data release. Here is used to unbind services.
     */
    @Override
    public void handleDataRelease() {
        Log.d(TAG, "[handleDataRelease]...");
        if (mVISEnrollmentService != null) {
            mContext.unbindService(mVISConnection);
            mVISEnrollmentService = null;
        }
        if (mTriggerService != null) {
            mContext.unbindService(mTriggerConnection);
            mTriggerService = null;
        }
    }
}
