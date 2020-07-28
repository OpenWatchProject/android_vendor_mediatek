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
package com.mediatek.voicecommand.adapter;

public interface IVoiceAdapter {

    /**
     * Judge whether the native is prepared for voice training, recognition and
     * UI.
     *
     * @return true if prepared
     */
    boolean isNativePrepared();

    /**
     * Start voice recognition when power on.
     *
     * @param patternpath
     *            Path used for native
     *
     * @param ubmpath
     *            Path used for native
     *
     * @param ubmUpgradePath
     *            Model upgrade path
     *
     * @param processname
     *            Record the process who start the voice recognition
     *
     * @param pid
     *            Record the process pid because maybe the process was died
     *            during voice recognition starting
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int startVoicePwRecognition(String patternpath, String ubmpath, String ubmUpgradePath,
            String processname, int pid);

    /**
     * Stop voice recognition when power off or time out.
     *
     * @param processname
     *            Used to check if the process has start this business
     *
     * @param pid
     *            Used to check if the pid is the same as before
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int stopVoicePwRecognition(String processname, int pid);

    /**
     * Start voice training.
     *
     * @param pwdpath
     *            Used for native thread
     *
     * @param patternpath
     *            Used for native thread
     *
     * @param featurepath
     *            Used for native thread
     *
     * @param umbpath
     *            Used for native thread
     *
     * @param ubmUpgradePath
     *            Used for native thread
     *
     * @param commandid
     *            Used for native thread and notify App when recognition
     *
     * @param commandMask
     *            Used for native thread and notify App when recognition
     *
     * @param trainingMode
     *            Used for native thread and notify App when recognition
     *
     * @param wakeupinfoPath
     *            Used for native thread and notify App when recognition
     *
     * @param processname
     *            Record the process who start the voice recognition
     *
     * @param pid
     *            Record the process pid because maybe the process was died
     *            during voice recognition starting
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int startVoiceTraining(String pwdpath, String patternpath,
            String featurepath, String umbpath, String ubmUpgradePath, int commandid,
            int[] commandMask, int trainingMode, int realTrainingMode, String wakeupinfoPath,
            String processname, int pid);

    /**
     * Reset voice training.
     *
     * @param pwdpath
     *            Used for native thread
     *
     * @param patternpath
     *            Used for native thread
     *
     * @param featurepath
     *            Used for native thread
     *
     * @param commandid
     *            Used for native thread and notify App when recognition
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int resetVoiceTraining(String pwdpath, String patternpath,
            String featurepath, int commandid);

    /**
     * Modify voice training.
     *
     * @param pwdpath
     *            Used for native thread
     *
     * @param patternpath
     *            Used for native thread
     *
     * @param featurepath
     *            Used for native thread
     *
     * @param commandid
     *            Used for native thread and notify App when recognition
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int modifyVoiceTraining(String pwdpath, String patternpath,
            String featurepath, int commandid);

    /**
     * Stop voice training when app unregister from service or send the stopping
     * command.
     *
     * @param processname
     *            Used to check if the process has start this business
     *
     * @param pid
     *            Used to check if the pid is the same as before
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int stopVoiceTraining(String processname, int pid);

    /**
     * Finish voice training when app training success.
     *
     * @param processname
     *            Used to check if the process has start this business
     *
     * @param pid
     *            Used to check if the pid is the same as before
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int finishVoiceTraining(String processname, int pid);

    /**
     * indicate to continue voice training when app training one time success.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int continueVoiceTraining(String processname, int pid);

    /**
     * indicate to init voice training when app training one time success.
     * @ param isForDemo true indicates that the training used for demo
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int initVoiceTraining(boolean isForDemo);

    /**
     * indicate to pause voice training when app training one time success.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int pauseVoiceTraining(String processname, int pid);

    /**
     * indicate to modify voice training using preset training model.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int modifyPresetVoiceTraining(String patternPath, int commandId);

    /**
     * indicate to modify voice training timeout default value.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int setVoiceTrainingTimeout(int timeOutInMilliseconds);

    /**
     * indicate to modify voice training confidence threshold value.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int setVoiceTrainingThreshold(int threshold);

    /**
     * Get value of voice training repetition count.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int getVoiceTrainingRepeatCount();

    /**
     * Get value of voice training threshold.
     *
     * @return VoiceCommandListener.VOICE_NO_ERROR if success
     */
    int getVoiceTrainingThreshold();

    /**
     * Start voice ui business.
     *
     * @param modelpath
     *            Used for native thread
     *
     * @param patternpath
     *            Used for native thread
     *
     * @param processname
     *            Used for native thread
     *
     * @param pid
     *            Used for native thread
     *
     * @param languageid
     *            Notify native thread the current supprot language
     *
     * @return result
     */
    int startVoiceUi(String modelpath, String patternpath, String processname,
            int pid, int languageid);

    /**
     * Stop voice ui business.
     *
     * @param processname
     *            Used to check if the process has start this business
     *
     * @param pid
     *            Used to check if the pid is the same as before
     *
     * @return result
     */
    int stopVoiceUi(String processname, int pid);

    /**
     * Start voice contacts business.
     *
     * @param processname
     *            Used for native thread
     *
     * @param pid
     *            Used for native thread
     *
     * @param screenOrientation
     *            Screen Orientation from contacts App
     *
     * @param modelpath
     *            Used for native thread
     *
     * @param contactsdbpath
     *            Used for native thread
     *
     * @return result
     */
    int startVoiceContacts(String processname, int pid, int screenOrientation,
            String modelpath, String contactsdbpath);

    /**
     * Stop voice contacts business.
     *
     * @param processname
     *            Used to check if the process has start this business
     *
     * @param pid
     *            Used to check if the pid is the same as before
     *
     * @return result
     */
    int stopVoiceContacts(String processname, int pid);

    /**
     * Send all voice contacts name.
     *
     * @param modelpath
     *            Used for native thread
     *
     * @param contactsdbpath
     *            Used for native thread
     *
     * @param allContactsName
     *            Used to send all contacts name
     * @return result
     */
    int sendContactsName(String modelpath, String contactsdbpath, String[] allContactsName);

    /**
     * Send learning voice contacts name.
     *
     * @param selectName
     *            Used to send learning contacts name
     * @return result
     */
    int sendContactsSelected(String selectName);

    /**
     * Send required contacts result count from contacts app.
     *
     * @param searchCnt
     *            Used to send search voice contacts count
     * @return result
     */
    int sendContactsSearchCnt(int searchCnt);

    /**
     * Send screen orientation from contacts app.
     *
     * @param screenOrientation
     *            Used to send screen orientation
     * @return result
     */
    int sendContactsOrientation(int screenOrientation);

    /**
     * Send recognition Enable flag from contacts app.
     *
     * @param recognitionEnable
     *            Used to send recognition Enable flag
     * @return result
     */
    int sendContactsRecogEnable(int recognitionEnable);

    /**
     * Init voice wakeup name info when boot.
     *
     * @param mode
     *            wakeup mode used for native thread
     *
     * @param cmdStatus
     *            wakeup command status used for native thread
     *
     * @param cmdIds
     *            the all command id have been training used for native thread
     *
     * @param patternPath
     *            the pattern path of this wakeup mode used for native thread
     *
     * @param mode1
     *            wakeup by anyone used for native thread
     *
     * @param patternPath1
     *            wakeup by anyone used for native thread
     *
     * @param passwordPath1
     *            wakeup by anyone used for native thread
     *
     * @param mode2
     *            wakeup by command used for native thread
     *
     * @param patternPath2
     *            wakeup by command used for native thread
     *
     * @param passwordPath2
     *            wakeup by command used for native thread
     *
     * @param ubmPath
     *            init model path used for native thread
     *
     * @param wakeupinfoPath
     *            used for native thread
     * @return result
     */
    int initVoiceWakeup(int mode, int cmdStatus, int[] cmdIds,
            String patternPath, int mode1, String patternPath1,
            String passwordPath1, int mode2, String patternPath2,
            String passwordPath2, String ubmPath, String wakeupinfoPath);

    /**
     * Init voice trigger name info when boot.
     *
     * @param mode
     *            wakeup mode used for native thread
     *
     * @param cmdStatus
     *            wakeup command status used for native thread
     *
     * @param cmdIds
     *            the all command id have been training used for native thread
     *
     * @param patternPath
     *            the pattern path of this wakeup mode used for native thread
     *
     * @param mode1
     *            wakeup by anyone used for native thread
     *
     * @param patternPath1
     *            wakeup by anyone used for native thread
     *
     * @param passwordPath1
     *            wakeup by anyone used for native thread
     *
     * @param mode2
     *            wakeup by command used for native thread
     *
     * @param patternPath2
     *            wakeup by command used for native thread
     *
     * @param passwordPath2
     *            wakeup by command used for native thread
     *
     * @param ubmPath
     *            init model path used for native thread
     *
     * @param ubmUpgradePath
     *            model upgrade path
     *
     * @param wakeupinfoPath
     *            used for native thread
     * @return result
     */
    int initVoiceTrigger(int mode, int cmdStatus, int[] cmdIds,
            String patternPath, int mode1, String patternPath1,
            String passwordPath1, int mode2, String patternPath2,
            String passwordPath2, String ubmPath, String ubmUpgradePath, String wakeupinfoPath);


    /**
     * Send voice wakeup mode.
     *
     * @param mode
     *            wakeup mode used for native thread
     * @param ubmPath
     *            init model path used for native thread
     * @param ubmUpgradePath
     *            model upgrade path
     * @return result
     */
    int sendVoiceWakeupMode(int mode, String ubmPath, String ubmUpgradePath);

    /**
     * Send voice wakeup command status.
     *
     * @param cmdStatus
     *            wakeup command status used for native thread
     *
     * @return result
     */
    int sendVoiceWakeupCmdStatus(int cmdStatus);

    /**
     * Start voice wakeup business, but native will start itself.
     *
     * @param processname Used for native thread
     *
     * @param pid Used for native thread
     *
     * @return result
     */

    int startVoiceWakeup(String processname, int pid);

    /**
     * Start voice trigger business, but native will start itself.
     *
     * @param processname Used for native thread
     *
     * @param pid Used for native thread
     *
     * @return result
     */

    int startVoiceTrigger(String processname, int pid);

    /**
     * Get the intensity from native.
     *
     * @return voice intensity
     */
    int getNativeIntensity();

    /**
     * Set VoiceModel retrain action to native.
     *
     */
    int setVoiceModelRetrain(String patternPath, String umbPath, String ubmUpgradePath,
                              int commandId, int trainingMode, String wakeupinfoPath);

    /**
     * modify VoiceModel using new training flow.
     *
     * @param ubmPath
     *            init model path used for native thread
     *
     * @param ubmUpgradePath
     *            model upgrade path
     *
     * @param commandId
     *            the command id used for training native thread
     *
     * @param trainingMode
     *            the training mode used
     *
     * @return result
     */
    int modifyVoiceTrainingKeyphrase(String ubmPath, String ubmUpgradePath,
                              int commandId, int trainingMode);

    /**
     * Stop native thread business if needed when process died.
     *
     * @param processname
     *            process name
     *
     * @param pid
     *            process id
     */
    void stopCurMode(String processname, int pid);

    /**
     * Set current Headset status.
     *
     * @param isPlugin
     *            true if the head set is plugin
     */
    void setCurHeadsetMode(boolean isPlugin);

    /**
     * Release all the native memory when service is called onDestroy.
     */
    void release();

}
