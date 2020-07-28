/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.common.voicecommand;

public abstract class VoiceCommandListener {

    // =================Voice command service field====================//
    /**
     * Voice command service name.
     */
    public static final String VOICE_COMMAND_SERVICE = "voicecommand";
    /**
     * Service action used for bind voice command service.
     * @hide
     * @internal
     */
    public static final String VOICE_SERVICE_ACTION = "com.mediatek.voicecommand";
    /**
     * Service category used for bind voice command service.
     * @hide
     * @internal
     */
    public static final String VOICE_SERVICE_CATEGORY = "com.mediatek.nativeservice";
    /**
     * Service package name used for bind voice command service.
     * @hide
     * @internal
     */
    public static final String VOICE_SERVICE_PACKAGE_NAME = "com.mediatek.voicecommand";

    // =================VOW_VIMS fields====================//
    /**
     * Service action used for bind voice interaction service.
     */
    public static final String VOICE_WAKEUP_SERVICE_ACTION =
        "com.mediatek.voicecommand.VoiceWakeupInteractionService";
    /**
     * Service action used for bind voice training enrollment service.
     */
    public static final String VOICE_TRAINING_SERVICE_ACTION =
        "com.mediatek.intent.action.bindEnrollmentService";
    /**
     * Activity action used for voice training interaction activity.
     */
    public static final String VOICE_WAKEUP_ACTIVTY_ACTION =
    "com.mediatek.voicecommand.VOW_INTERACT";
     /**
     * Service package name used for bind voice training enrollment service.
     */
    public static final String VOICE_TRAINING_SERVICE_PACKAGE_NAME =
        "com.mediatek.voicecommand.vis";
   /**
     * Voice wakeup enrollment text.
     */
    public static final String VOW_ENROLLMENT_TEXT = "Hello There";
    /**
     * Voice wakeup enrollment locale.
     */
    public static final String VOW_ENROLLMENT_BCP47_LOCALE = "en-US";
    /**
     * Voice confidence threshold default value.
     */
    public static final int VOICE_CONFIDENCE_THRESHOLD_DEFAULT = 50;
    /**
     * Voice confidence threshold minimum value.
     */
    public static final int VOICE_CONFIDENCE_THRESHOLD_MIN = 0;
    /**
     * Voice confidence threshold maximum value.
     */
    public static final int VOICE_CONFIDENCE_THRESHOLD_MAX = 100;

    // ===================Voice command main features====================//
    private static final int ACTION_COMMON_INDEX = 1;

    /**
     * Voice command common feature.
     * @hide
     * @internal
     */
    public static final int ACTION_MAIN_VOICE_COMMON = ACTION_COMMON_INDEX + 0;
    /**
     * Voice UI feature.
     * @hide
     * @internal
     */
    public static final int ACTION_MAIN_VOICE_UI = ACTION_COMMON_INDEX + 1;
    /**
     * Voice training feature.
     * @hide
     * @internal
     */
    public static final int ACTION_MAIN_VOICE_TRAINING = ACTION_COMMON_INDEX + 2;
    /**
     * Voice recognition feature.
     * @hide
     * @internal
     */
    public static final int ACTION_MAIN_VOICE_RECOGNITION = ACTION_COMMON_INDEX + 3;
    /**
     * Voice contacts search feature.
     * @hide
     * @internal
     */
    public static final int ACTION_MAIN_VOICE_CONTACTS = ACTION_COMMON_INDEX + 4;
    /**
     * Voice wake up feature.
     * @hide
     * @internal
     */
    public static final int ACTION_MAIN_VOICE_WAKEUP = ACTION_COMMON_INDEX + 5;
    /**
     * Voice trigger feature.
     * @hide
     * @internal
     */
    public static final int ACTION_MAIN_VOICE_TRIGGER = ACTION_COMMON_INDEX + 6;

    // ==================voice common feature action====================//
    /**
     * Uses to get keyword for voice common feature.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_COMMON_KEYWORD = ACTION_COMMON_INDEX + 0;
    /**
     * Uses to get command path for voice common feature.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_COMMON_COMMAND_PATH = ACTION_COMMON_INDEX + 1;
    /**
     * Uses to get process state for voice common feature.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_COMMON_PROCESS_STATE = ACTION_COMMON_INDEX + 2;

    // ====================voice ui feature action=======================//
    /**
     * Uses to start voice ui.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_UI_START = ACTION_COMMON_INDEX + 0;
    /**
     * Uses to stop voice ui.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_UI_STOP = ACTION_COMMON_INDEX + 1;
    /**
     * Uses to enable voice ui.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_UI_ENABLE = ACTION_COMMON_INDEX + 2;
    /**
     * Uses to disable voice ui.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_UI_DISALBE = ACTION_COMMON_INDEX + 3;
    /**
     * Uses to receive voice ui result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_UI_NOTIFY = ACTION_COMMON_INDEX + 4;

    // ===============voice training feature action========================//
    /**
     * Uses to start voice training.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_START = ACTION_COMMON_INDEX + 0;
    /**
     * Uses to stop voice training.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_STOP = ACTION_COMMON_INDEX + 1;
    /**
     * Uses to get voice intensity of voice training.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_INTENSITY = ACTION_COMMON_INDEX + 2;
    /**
     * Uses to get training password file path.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_PASSWORD_FILE = ACTION_COMMON_INDEX + 3;
    /**
     * Uses to receive voice training result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_NOTIFY = ACTION_COMMON_INDEX + 4;
    /**
     * Uses to reset voice training.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_RESET = ACTION_COMMON_INDEX + 5;
    /**
     * Uses to modify voice training.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_MODIFY = ACTION_COMMON_INDEX + 6;
    /**
     * Uses to finish voice training.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_FINISH = ACTION_COMMON_INDEX + 7;

    /**
     * Uses to continue to do voice training.
     * @hide
     */
    public static final int ACTION_VOICE_TRAINING_CONTINUE = ACTION_COMMON_INDEX + 8;

    /**
     * Uses to pause current ongoing training. This will cancel active training.
     * @hide
     */
    public static final int ACTION_VOICE_TRAINING_PAUSE = ACTION_COMMON_INDEX + 9;

     /**
     * Uses to bind enrollment service and init other voice training items.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_INIT = ACTION_COMMON_INDEX + 10;

      /**
      * Uses to update training parameter.
      * @hide
      * @internal
      */
     public static final int ACTION_VOICE_TRAINING_MODIFY_PARAM = ACTION_COMMON_INDEX + 11;

      /**
      * Uses to get training parameter.
      * @hide
      * @internal
      */
     public static final int ACTION_VOICE_TRAINING_QUERY_PARAM = ACTION_COMMON_INDEX + 12;

     /**
     * Uses to continue to do update model for internal testing.
     * @hide
     */
    public static final int ACTION_VOICE_TRAINING_UPDATE_MODEL = ACTION_COMMON_INDEX + 13;

    /**
     * Uses to receive voice training progress callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_NOTIFY_PROGRESS = ACTION_COMMON_INDEX + 14;

    /**
     * Uses to receive voice training finish callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_NOTIFY_FINISH = ACTION_COMMON_INDEX + 15;

    /**
     * Uses to receive voice training enroll start in training business.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_ENROLL_START = ACTION_COMMON_INDEX + 16;

    /**
     * Uses to get registered voice training user id array.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_GET_USER_LIST = ACTION_COMMON_INDEX + 17;

    /**
     * Modify command id. This is used when we need to change keyphrase.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRAINING_CHANGE_KEYPHRASE = ACTION_COMMON_INDEX + 18;

    // ==============voice recognize feature action=====================//
    /**
     * Uses to start voice recognize.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_RECOGNITION_START = ACTION_COMMON_INDEX + 0;
    /**
     * Uses to get voice intensity of voice recognize.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_RECOGNITION_INTENSITY = ACTION_COMMON_INDEX + 1;
    /**
     * Uses to receive voice recognize result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_RECOGNITION_NOTIFY = ACTION_COMMON_INDEX + 2;

    // =============voice contacts search feature action================//
    /**
     * Uses to start voice contacts search.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_START = ACTION_COMMON_INDEX + 0;
    /**
     * Uses to stop voice contacts search.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_STOP = ACTION_COMMON_INDEX + 1;
    /**
     * Uses to enable voice contacts search.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_ENABLE = ACTION_COMMON_INDEX + 2;
    /**
     * Uses to disable voice contacts search.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_DISABLE = ACTION_COMMON_INDEX + 3;
    /**
     * Uses to get voice intensity of voice contacts search.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_INTENSITY = ACTION_COMMON_INDEX + 4;
    /**
     * Uses to indicate this contacts name is selected.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_SELECTED = ACTION_COMMON_INDEX + 5;
    /**
     * Uses to receive voice contacts search result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_NOTIFY = ACTION_COMMON_INDEX + 6;
    /**
     * Uses to send all voice contacts name to swip.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_NAME = ACTION_COMMON_INDEX + 7;
    /**
     * Uses to receive speech detected result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_SPEECH_DETECTED = ACTION_COMMON_INDEX + 8;
    /**
     * Uses to send needed contacts count to swip.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_SEARCH_COUNT = ACTION_COMMON_INDEX + 9;
    /**
     * Uses to send physical orientation to swip.
     ** @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_ORIENTATION = ACTION_COMMON_INDEX + 10;
    /**
     * Uses to enable voice contacts search.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_RECOGNITION_ENABLE = ACTION_COMMON_INDEX + 11;
    /**
     * Uses to disable voice contacts search.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_CONTACTS_RECOGNITION_DISABLE = ACTION_COMMON_INDEX + 12;

    // =================voice wake up feature action========================//
    /**
     * Uses to start voice wake up.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_START = ACTION_COMMON_INDEX + 0;
    /**
     * Uses to enable voice wake up.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_ENABLE = ACTION_COMMON_INDEX + 1;
    /**
     * Uses to disable voice wake up.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_DISABLE = ACTION_COMMON_INDEX + 2;
    /**
     * Uses to receive voice wake up result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_NOTIFY = ACTION_COMMON_INDEX + 3;
    /**
     * Uses to send init model/speak model to MD32.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_INIT = ACTION_COMMON_INDEX + 4;
    /**
     * Uses to send wake up mode to MD32.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_MODE = ACTION_COMMON_INDEX + 5;
    /**
     * Uses to send wake up commands status to MD32.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_COMMAND_STATUS = ACTION_COMMON_INDEX + 6;
    /**
     * Uses to get voice wakeup status.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_WAKEUP_READ_STATUS = ACTION_COMMON_INDEX + 7;

    // =========voice wake up mode and command status field==============//
    /**
     * Voice wakeup mode: unlock mode.
     * @hide
     * @internal
     */
    public static final int VOICE_WAKEUP_MODE_UNLOCK = 0;
    /**
     * Voice wakeup mode: speaker independent mode.
     * @hide
     * @internal
     */
    public static final int VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT = 1;
    /**
     * Voice wakeup mode: speaker dependent mode.
     * @hide
     * @internal
     */
    public static final int VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT = 2;
    /**
     * Voice wakeup mode: sound trigger mode.
     * @hide
     * @internal
     */
    public static final int VOICE_WAKEUP_MODE_TRIGGER = 3;

    /**
     * Voice wakeup mode: no command and unchecked.
     * @hide
     * @internal
     */
    public static final int VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED = 0;
    /**
     * Voice wakeup command status: no command and unchecked.
     * @hide
     * @internal
     */
    public static final int VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED = 1;
    /**
     * Voice wakeup command status: has command and unchecked.
     * @hide
     * @internal
     */
    public static final int VOICE_WAKEUP_STATUS_COMMAND_CHECKED = 2;

    // =============voice training tuning parameters==================//
      /**
       * Voice training parameter : Timeout in MS
       * @hide
       * @internal
       */
      public static final int VOICE_TRAINING_PARAM_TIMEOUT = 0;

    /**
     * Voice training parameter : Confidence threshold
     * @hide
     * @internal
     */
    public static final int VOICE_TRAINING_PARAM_CONFIDENCE_THRESHOLD =
        VOICE_TRAINING_PARAM_TIMEOUT + 1;

    /**
     * Voice training parameter : Iterations of keyphrase for training
     * @hide
     * @internal
     */
    public static final int VOICE_TRAINING_PARAM_REPEAT_TIMES =
        VOICE_TRAINING_PARAM_TIMEOUT + 2;

    /**
     * Voice trigger parameter : First stage threshold
     * @hide
     * @internal
     */
    public static final int VOICE_TRIGGER_PARAM_COARSE_CONFIDENCE = 0;

    /**
     * Voice trigger parameter : Second stage threshold
     * @hide
     * @internal
     */
    public static final int VOICE_TRIGGER_SECOND_STAGE_THRESHOLD =
        VOICE_TRIGGER_PARAM_COARSE_CONFIDENCE + 1;

    /**
     * Voice trigger parameter : Anti-spoof threshold
     * @hide
     * @internal
     */
    public static final int VOICE_TRIGGER_ANTI_SPOOF_THRESHOLD =
            VOICE_TRIGGER_PARAM_COARSE_CONFIDENCE + 2;
    // =================voice trigger feature action========================//
    /**
     * Uses to start voice trigger.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_START = ACTION_COMMON_INDEX + 0;
    /**
     * Uses to enable voice trigger
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_ENABLE = ACTION_COMMON_INDEX + 1;
    /**
     * Uses to disable voice trigger.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_DISABLE = ACTION_COMMON_INDEX + 2;
    /**
     * Uses to receive voice trigger result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_NOTIFY_ARRIVED = ACTION_COMMON_INDEX + 3;
    /**
     * Uses to receive voice trigger result callback from service.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_NOTIFY_DETECTED = ACTION_COMMON_INDEX + 4;
    /**
     * Uses to send init model/speak model to MD32.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_INIT = ACTION_COMMON_INDEX + 5;
    /**
     * Uses to send trigger mode to MD32.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_MODE = ACTION_COMMON_INDEX + 6;
    /**
     * Uses to send trigger command status to MD32.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_COMMAND_STATUS = ACTION_COMMON_INDEX + 7;
    /**
     * Uses to send trigger command to update recognition parameter value.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_MODIFY_PARAM = ACTION_COMMON_INDEX + 8;
    /**
     * Uses to send trigger command to query recognition parameter value.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_QUERY_PARAM = ACTION_COMMON_INDEX + 9;

    /**
     * Uses to send voice training set locale.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_SET_LOCALE = ACTION_COMMON_INDEX + 10;

    /**
     * Uses to send voice trigger get locale.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_GET_LOCALE = ACTION_COMMON_INDEX + 11;

    /**
     * Uses to get voice trigger status.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_READ_STATUS = ACTION_COMMON_INDEX + 12;

    /**
     * Uses to get voice trigger initial params.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_GET_INITIAL_PARAMS = ACTION_COMMON_INDEX + 13;

    /**
     * Uses to get voice trigger package changed.
     * @hide
     * @internal
     */
    public static final int ACTION_VOICE_TRIGGER_NOTIFY_PACKAGE_CHANGED =
        ACTION_COMMON_INDEX + 14;

    /**
     * @hide
     * @internal
     */
    public static String getMainActionName(int action) {
        return sActionMainToStr[action];
    }

    /**
     * @hide
     * @internal
     */
    public static String getSubActionName(int mainAction, int subAction) {
        return sActionVoiceToStr[mainAction][subAction];
    }

    /**
     * @hide
     * @internal
     */
    public static String getWakeupModeStr(int wakeupMode) {
        return sWakeupModeToStr[wakeupMode];
    }

    /**
     * @hide
     * @internal
     */
    public static String getWakeupStatusStr(int wakeupStatus) {
        return sWakeupStatusToStr[wakeupStatus];
    }

    /**
     * @hide
     * @internal
     */
    public static String getTrainingParamStr(int param) {
        return sTrainingParamToStr[param];
    }

    /**
     * @hide
     * @internal
     */
    public static String getTriggerParamStr(int param) {
        return sTriggerParamToStr[param];
    }

    private static String[] sActionMainToStr = {
        "EMPTY",
        "ACTION_MAIN_VOICE_COMMON",
        "ACTION_MAIN_VOICE_UI",
        "ACTION_MAIN_VOICE_TRAINING",
        "ACTION_MAIN_VOICE_RECOGNITION",
        "ACTION_MAIN_VOICE_CONTACTS",
        "ACTION_MAIN_VOICE_WAKEUP",
        "ACTION_MAIN_VOICE_TRIGGER"
    };

    private static String[][] sActionVoiceToStr = {
        {"EMPTY", "EMPTY"},
        {
            "EMPTY",
            "ACTION_VOICE_COMMON_KEYWORD",
            "ACTION_VOICE_COMMON_COMMAND_PATH",
            "ACTION_VOICE_COMMON_PROCESS_STATE"
        },
        {
            "EMPTY",
            "ACTION_VOICE_UI_START",
            "ACTION_VOICE_UI_STOP",
            "ACTION_VOICE_UI_ENABLE",
            "ACTION_VOICE_UI_DISALBE",
            "ACTION_VOICE_UI_NOTIFY"
        },
        {
            "EMPTY",
            "ACTION_VOICE_TRAINING_START",
            "ACTION_VOICE_TRAINING_STOP",
            "ACTION_VOICE_TRAINING_INTENSITY",
            "ACTION_VOICE_TRAINING_PASSWORD_FILE",
            "ACTION_VOICE_TRAINING_NOTIFY",
            "ACTION_VOICE_TRAINING_RESET",
            "ACTION_VOICE_TRAINING_MODIFY",
            "ACTION_VOICE_TRAINING_FINISH",
            "ACTION_VOICE_TRAINING_CONTINUE",
            "ACTION_VOICE_TRAINING_PAUSE",
            "ACTION_VOICE_TRAINING_INIT",
            "ACTION_VOICE_TRAINING_MODIFY_PARAM",
            "ACTION_VOICE_TRAINING_QUERY_PARAM",
            "ACTION_VOICE_TRAINING_UPDATE_MODEL",
            "ACTION_VOICE_TRAINING_NOTIFY_PROGRESS",
            "ACTION_VOICE_TRAINING_NOTIFY_FINISH",
            "ACTION_VOICE_TRAINING_ENROLL_START",
            "ACTION_VOICE_TRAINING_GET_USER_LIST",
            "ACTION_VOICE_TRAINING_CHANGE_KEYPHRASE"
        },
        {
            "EMPTY",
            "ACTION_VOICE_RECOGNITION_START",
            "ACTION_VOICE_RECOGNITION_INTENSITY",
            "ACTION_VOICE_RECOGNITION_NOTIFY"
        },
        {
            "EMPTY",
            "ACTION_VOICE_CONTACTS_START",
            "ACTION_VOICE_CONTACTS_STOP",
            "ACTION_VOICE_CONTACTS_ENABLE",
            "ACTION_VOICE_CONTACTS_DISABLE",
            "ACTION_VOICE_CONTACTS_INTENSITY",
            "ACTION_VOICE_CONTACTS_SELECTED",
            "ACTION_VOICE_CONTACTS_NOTIFY",
            "ACTION_VOICE_CONTACTS_NAME",
            "ACTION_VOICE_CONTACTS_SPEECH_DETECTED",
            "ACTION_VOICE_CONTACTS_SEARCH_COUNT",
            "ACTION_VOICE_CONTACTS_ORIENTATION",
            "ACTION_VOICE_CONTACTS_RECOGNITION_ENABLE",
            "ACTION_VOICE_CONTACTS_RECOGNITION_DISABLE"
        },
        {
            "EMPTY",
            "ACTION_VOICE_WAKEUP_START",
            "ACTION_VOICE_WAKEUP_ENABLE",
            "ACTION_VOICE_WAKEUP_DISABLE",
            "ACTION_VOICE_WAKEUP_NOTIFY",
            "ACTION_VOICE_WAKEUP_INIT",
            "ACTION_VOICE_WAKEUP_MODE",
            "ACTION_VOICE_WAKEUP_COMMAND_STATUS",
            "ACTION_VOICE_WAKEUP_READ_STATUS"
        },
        {
            "EMPTY",
            "ACTION_VOICE_TRIGGER_START",
            "ACTION_VOICE_TRIGGER_ENABLE",
            "ACTION_VOICE_TRIGGER_DISABLE",
            "ACTION_VOICE_TRIGGER_NOTIFY_ARRIVED",
            "ACTION_VOICE_TRIGGER_NOTIFY_DETECTED",
            "ACTION_VOICE_TRIGGER_INIT",
            "ACTION_VOICE_TRIGGER_MODE",
            "ACTION_VOICE_TRIGGER_COMMAND_STATUS",
            "ACTION_VOICE_TRIGGER_MODIFY_PARAM",
            "ACTION_VOICE_TRIGGER_QUERY_PARAM",
            "ACTION_VOICE_TRIGGER_SET_LOCALE",
            "ACTION_VOICE_TRIGGER_GET_LOCALE",
            "ACTION_VOICE_TRIGGER_READ_STATUS",
            "ACTION_VOICE_TRIGGER_GET_INITIAL_PARAMS",
            "ACTION_VOICE_TRIGGER_NOTIFY_PACKAGE_CHANGED"
        }
    };

    private static String[] sWakeupModeToStr = {
        "VOICE_WAKEUP_MODE_UNLOCK",
        "VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT",
        "VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT",
        "VOICE_WAKEUP_MODE_TRIGGER"
    };

    private static String[] sWakeupStatusToStr = {
        "VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED",
        "VOICE_WAKEUP_STATUS_COMMAND_UNCHECKED",
        "VOICE_WAKEUP_STATUS_COMMAND_CHECKED"
    };

    private static String[] sTrainingParamToStr = {
        "VOICE_TRAINING_PARAM_TIMEOUT",
        "VOICE_TRAINING_PARAM_CONFIDENCE_THRESHOLD",
        "VOICE_TRAINING_PARAM_REPEAT_TIMES"
    };

    private static String[] sTriggerParamToStr = {
        "VOICE_TRIGGER_PARAM_COARSE_CONFIDENCE",
        "VOICE_TRIGGER_SECOND_STAGE_THRESHOLD",
        "VOICE_TRIGGER_ANTI_SPOOF_THRESHOLD"
    };

    // =================call back result=========================//
    /**
     * Callback result extra success value.
     * @hide
     * @internal
     */
    public static final int ACTION_EXTRA_RESULT_SUCCESS = 1;
    /**
     * Callback result extra error value.
     * @internal
     */
    public static final int ACTION_EXTRA_RESULT_ERROR = 10;
    /**
     * Uses to get callback result value mapping from String values "Result".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_RESULT = "Result";
    /**
     * Uses to get callback result of mapping from String values "Result_Info".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_RESULT_INFO = "Result_Info";
    /**
     * Uses to get callback result of mapping from String values "Result_Info1".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_RESULT_INFO1 = "Result_Info1";
    /**
     * Uses to get callback result of mapping from String values "Result_Info2".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_RESULT_INFO2 = "Result_Info2";
    /**
     * Uses to get callback result of mapping from String values "Result_Info3".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_RESULT_INFO3 = "Result_Info3";
    /**
     * Uses to get callback result of mapping from String values "Result_Info4".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_RESULT_INFO4 = "Result_Info4";
    /**
     * Uses to get callback result of mapping from String values "Result_Info5".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_RESULT_INFO5 = "Result_Info5";
    /**
     * A mapping from String values with "Send".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_SEND = "Send";
    /**
     * A mapping from String values with "Send_Info".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_SEND_INFO = "Send_Info";
    /**
     * A mapping from String values with "Send_Info1".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_SEND_INFO1 = "Send_Info1";
    /**
     * A mapping from String values with "Send_Info2".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_SEND_INFO2 = "Send_Info2";
    /**
     * A mapping from String values with "Send_Info3".
     * @hide
     * @internal
     */
    public static final String ACTION_EXTRA_SEND_INFO3 = "Send_Info3";

    // =================Error message=========================//
    private static final int VOICE_ERROR_RECOGNIZE = 0;
    private static final int VOICE_ERROR_TRAINING = 100;
    private static final int VOICE_ERROR_SETTING = 200;
    private static final int VOICE_ERROR_UI = 300;
    private static final int VOICE_ERROR_CONTACTS = 400;
    private static final int VOICE_ERROR_COMMON = 1000;

    /**
     * Successful result, including all voice feature.
     * @hide
     * @internal
     */
    public static final int VOICE_NO_ERROR = 0;
    /**
     * Password Error.
     */
    public static final int VOICE_ERROR_RECOGNIZE_DENIED = VOICE_ERROR_RECOGNIZE + 1;
    /**
     * Around is too noisy, closer to the phone say again.
     */
    public static final int VOICE_ERROR_RECOGNIZE_NOISY = VOICE_ERROR_RECOGNIZE + 2;
    /**
     * Voice is too low, can not hear clearly.
     */
    public static final int VOICE_ERROR_RECOGNIZE_LOWLY = VOICE_ERROR_RECOGNIZE + 3;

    /**
     * Training is not enough, please say a few words to let me learn more.
     */
    public static final int VOICE_ERROR_TRAINING_NOT_ENOUGH = VOICE_ERROR_TRAINING + 1;
    /**
     * Around is too noisy, it is recommended that the training to the quiet
     * places.
     */
    public static final int VOICE_ERROR_TRAINING_NOISY = VOICE_ERROR_TRAINING + 2;
    /**
     * Speakers training that do not match the password.
     */
    public static final int VOICE_ERROR_TRAINING_PASSWORD_DIFF = VOICE_ERROR_TRAINING + 3;
    /**
     * The Password has been exist, please change the Password.
     */
    public static final int VOICE_ERROR_TRAINING_PASSWORD_EXIST = VOICE_ERROR_TRAINING + 4;
    /**
     * Voice can’t be recognized.
     */
    public static final int VOICE_ERROR_UI_INVALID = VOICE_ERROR_UI + 1;

    /**
     * Setting error. Language id not existing for usage.
     */
    public static final int VOICE_ERROR_SETTING_LANGUAGE_UPDATE = VOICE_ERROR_SETTING + 1;
    /**
     * Setting error. Invalid parameter threshold value used.
     * places.
     */
    public static final int VOICE_ERROR_SETTING_VALUE_OUT_OF_RANGE = VOICE_ERROR_SETTING + 2;

    /**
     * Voice can’t be recognized.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_CONTACTS_VOICE_INVALID = VOICE_ERROR_CONTACTS + 1;
    /**
     * Send data is invalid, please check it's format.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_CONTACTS_SEND_INVALID = VOICE_ERROR_CONTACTS + 2;

    /**
     * Voice command has been shut down in Setting.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_PROCESS_OFF = VOICE_ERROR_COMMON + 1;
    /**
     * No permission to use the function.
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_NO_PERMISSION = VOICE_ERROR_COMMON + 2;
    /**
     * App already registered, unregistered first.
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_REGISTERED = VOICE_ERROR_COMMON + 3;
    /**
     * No registered App in the service.
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_UNREGISTER = VOICE_ERROR_COMMON + 4;
    /**
     * Illegal voice process, may be caused by no permission to use the function
     * or no register service.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_ILLEGAL_PROCESS = VOICE_ERROR_COMMON + 5;
    /**
     * Voice command service error.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_SERVICE = VOICE_ERROR_COMMON + 6;
    /**
     * The action is invalid, need to check the send action.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_INVALID_ACTION = VOICE_ERROR_COMMON + 7;
    /**
     * The data is invalid, need to check the send data.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_INVALID_DATA = VOICE_ERROR_COMMON + 8;
    /**
     * Notify message fail.
     * @hide
     * @internal
     */
    public static final int VOICE_ERROR_COMMON_NOTIFY_FAIL = VOICE_ERROR_COMMON + 9;
}
