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

package com.mediatek.voicecommand.mgr;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.media.AudioManager;
import android.os.FileUtils;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.business.VoiceTriggerBusiness;
import com.mediatek.voicecommand.business.VoiceWakeupBusiness;
import com.mediatek.voicecommand.cfg.ConfigurationXml;
import com.mediatek.voicecommand.cfg.VoiceCustomization;
import com.mediatek.voicecommand.cfg.VoiceKeyWordInfo;
import com.mediatek.voicecommand.cfg.VoiceLanguageInfo;
import com.mediatek.voicecommand.cfg.VoiceProcessInfo;
import com.mediatek.voicecommand.cfg.VoiceWakeupInfo;
import com.mediatek.voicecommand.util.Log;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;

public class ConfigurationManager {
    private static final String TAG = "ConfigurationManager";

    private volatile static ConfigurationManager sCfgMgr = null;

    private final Context mContext;
    private ConfigurationXml mConfigurationXml;

    private final HashMap<String, String> mPathsMap = new HashMap<String, String>();
    private final HashMap<String, VoiceProcessInfo> mVoiceProcessInfosMap = new HashMap<String, VoiceProcessInfo>();
    private final HashMap<String, VoiceKeyWordInfo> mVoiceKeyWordInfosMap = new HashMap<String, VoiceKeyWordInfo>();

    private ArrayList<VoiceLanguageInfo> mVoiceLanguageInfoArrayList = new ArrayList<VoiceLanguageInfo>();
    private ArrayList<String> mVoiceUiFeatureNameArrayList = new ArrayList<String>();
    private ArrayList<VoiceWakeupInfo> mWakeupAnyoneArrayList = new ArrayList<VoiceWakeupInfo>();
    private ArrayList<VoiceWakeupInfo> mWakeupCommandArrayList = new ArrayList<VoiceWakeupInfo>();
    private ArrayList<VoiceWakeupInfo> mWakeupTriggerArrayList = new ArrayList<VoiceWakeupInfo>();
    private ArrayList<Integer> mEnrolledUsers = new ArrayList<Integer>();

    private static final String UI_PATTERN = "UIPattern";
    private static final String UBM_FILE = "UBMFile";
    private static final String UBM_FILE_UPGRADE = "UBMFileUpgrade";
    private static final String MODE_FILE = "ModeFile";
    private static final String CONTACTS_DB_FILE = "ContactsdbFile";
    private static final String CONTACTS_MODE_FILE = "ContactsModeFile";

    private static final String TRAINING_UNLOCK_PATH = "/training/unlock/";
    private static final String TRAINING_ANYONE_PATH = "/training/anyone/";
    private static final String TRAINING_COMMAND_PATH = "/training/command/";
    private static final String TRAINING_TRIGGER_PATH = "/training/trigger/";

    private static final String RECOG_PATTERN = "recogpattern/";
    private static final String FEATURE_FILE = "featurefile/";
    private static final String PASSWORD_FILE = "passwordfile/";

    private static final String UNLOCK_RECOG_PATTERN = "UnlockRecogPattern";
    private static final String UNLOCK_FEATURE_FILE = "UnlockFeatureFile";
    private static final String UNLOCK_PSWD_FILE = "UnlockPswdFile";

    private static final String ANYONE_RECOG_PATTERN = "AnyoneRecogPattern";
    private static final String ANYONE_FEATURE_FILE = "AnyoneFeatureFile";
    private static final String ANYONE_PSWD_FILE = "AnyonePswdFile";

    private static final String COMMAND_RECOG_PATTERN = "CommandRecogPattern";
    private static final String COMMAND_FEATURE_FILE = "CommandFeatureFile";
    private static final String COMMAND_PSWD_FILE = "CommandPswdFile";

    private static final String TRIGGER_RECOG_PATTERN = "TriggerRecogPattern";
    private static final String TRIGGER_FEATURE_FILE = "TriggerFeatureFile";
    private static final String TRIGGER_PSWD_FILE = "TriggerPswdFile";

    private static final String WAKE_UP_INFO_PATH = "/wakeupinfo/";
    private static final String WAKE_UP_INFO_FILE = "WakeupinfoFile";
    private static final String WAKE_UP_ANYONE_PATH = "wakeup/anyone.xml";
    private static final String WAKE_UP_COMMAND_PATH = "wakeup/command.xml";
    private static final String WAKE_UP_TRIGGER_PATH = "wakeup/trigger.xml";

    private static final String VOICE_UI_CACHE_FILE = "com.mediatek.voicecommand_preferences";
    private static final String VOICE_LANGUAGE_CACHE_FILE = "Voice_Language";
    private static final String CUR_LANGUAGE_INDEX = "CurLanguageIndex";
    private static final String IS_FIRST_BOOT = "IsFirstBoot";
    private static final String CUR_SYSTEM_LANGUAGE_INDEX = "CurSystemLanguageIndex";
    private static final String CONTACTS_DB_PATH = "/contacts/";

    /*Key values used for Voice_Trigger cache*/
    private static final String VOICE_TRIGGER_PARAM_CACHE_FILE = "Voice_Trigger";
    private static final String PARAM_COARSE_CONFIDENCE = "CoarseConfidence";
    private static final String SECOND_STAGE_THRESHOLD = "SecondStageThreshold";
    private static final String TRAINING_CONFIDENCE_THRESHOLD = "TrainingThreshold";
    private static final String ANTI_SPOOF_THRESHOLD = "AntiSpoofThreshold";
    private static final String CURRENT_TRAINING_LOCALE = "CurrentTrainingLocale";
    private static final String ENROLLED_USER_ID = "EnrolledUserId";

    /*Keyphrase mapping macro*/
    private static final String[] VOW_KEYPHRASES = {
                        "MTK_VOW_COMMAND",
                        "MTK_VOW_COMMAND_2ND",
                        "MTK_VOW_COMMAND_3RD"
                        };
    private static final String AUDIO_MANAGER_VALUE_SEPARATOR = "=";

    /*Voice first confidence threshold default value.*/
    public static final int FIRST_CONFIDENCE_THRESHOLD_DEFAULT = 40;
    /*Voice second confidence threshold default value. > 100 means use default value*/
    public static final int SECOND_CONFIDENCE_THRESHOLD_DEFAULT = 150;
    /*Voice training confidence threshold default value.*/
    public static final int TRAINING_CONFIDENCE_THRESHOLD_DEFAULT = 50;
    /*Enrollment default locale.*/
    public static final String VOW_ENROLLMENT_DEFAULT_LOCALE = "en-IN";

    private int mWakeupMode = 1;
    private int mWakeupStatus = 0;
    private int mCurrentLanguageIndex = -1;
    private int mActiveCommandId = -1;

    private int mCoarseConfidence = -1;
    private int mSecondStageConfidence = -1;
    private int mTrainingConfidence = -1;
    private String mTrainingLocale = "";

    private boolean mIsCfgPrepared = true;
    private boolean mIsSystemLanguage = false;
    private boolean mIsDemoSupported = false;

    private VoiceCustomization mVoiceCustomization = new VoiceCustomization();

    /*
     * ConfigurationManager for control configuration
     * 
     * @param context
     */
    private ConfigurationManager(Context context) {
        Log.i(TAG, "[ConfigurationManager]new ...");
        mContext = context.getApplicationContext();
        mConfigurationXml = new ConfigurationXml(context);

        mConfigurationXml.readVoiceProcessInfoFromXml(mVoiceProcessInfosMap,
                mVoiceUiFeatureNameArrayList);
        mConfigurationXml.readVoiceCommandPathFromXml(mPathsMap);
        mCurrentLanguageIndex = mConfigurationXml
                .readVoiceLanguangeFromXml(mVoiceLanguageInfoArrayList);
        mConfigurationXml.readVoiceWakeupFromXml(mWakeupAnyoneArrayList, WAKE_UP_ANYONE_PATH);
        mConfigurationXml.readVoiceWakeupFromXml(mWakeupCommandArrayList, WAKE_UP_COMMAND_PATH);
        mConfigurationXml.readVoiceWakeupFromXml(mWakeupTriggerArrayList, WAKE_UP_TRIGGER_PATH);

        String dataDir = context.getApplicationInfo().dataDir;
        mPathsMap.put(CONTACTS_DB_FILE, dataDir + CONTACTS_DB_PATH);

        mPathsMap.put(UNLOCK_RECOG_PATTERN, dataDir + TRAINING_UNLOCK_PATH + RECOG_PATTERN);
        mPathsMap.put(UNLOCK_FEATURE_FILE, dataDir + TRAINING_UNLOCK_PATH + FEATURE_FILE);
        mPathsMap.put(UNLOCK_PSWD_FILE, dataDir + TRAINING_UNLOCK_PATH + PASSWORD_FILE);

        mPathsMap.put(ANYONE_RECOG_PATTERN, dataDir + TRAINING_ANYONE_PATH + RECOG_PATTERN);
        mPathsMap.put(ANYONE_FEATURE_FILE, dataDir + TRAINING_ANYONE_PATH + FEATURE_FILE);
        mPathsMap.put(ANYONE_PSWD_FILE, dataDir + TRAINING_ANYONE_PATH + PASSWORD_FILE);

        mPathsMap.put(COMMAND_RECOG_PATTERN, dataDir + TRAINING_COMMAND_PATH + RECOG_PATTERN);
        mPathsMap.put(COMMAND_FEATURE_FILE, dataDir + TRAINING_COMMAND_PATH + FEATURE_FILE);
        mPathsMap.put(COMMAND_PSWD_FILE, dataDir + TRAINING_COMMAND_PATH + PASSWORD_FILE);

        mPathsMap.put(TRIGGER_RECOG_PATTERN, dataDir + TRAINING_TRIGGER_PATH + RECOG_PATTERN);
        mPathsMap.put(TRIGGER_FEATURE_FILE, dataDir + TRAINING_TRIGGER_PATH + FEATURE_FILE);
        mPathsMap.put(TRIGGER_PSWD_FILE, dataDir + TRAINING_TRIGGER_PATH + PASSWORD_FILE);

        mPathsMap.put(WAKE_UP_INFO_FILE, dataDir + WAKE_UP_INFO_PATH);

        mConfigurationXml.readVoiceCustomizationFromXml(mVoiceCustomization);

        mIsSystemLanguage = mVoiceCustomization.mIsSystemLanguage;
        if (mIsSystemLanguage) {
            Log.i(TAG, "[ConfigurationManager]new,isSystemLanguage.");
            // Register Local change receiver.
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_LOCALE_CHANGED);
            mContext.registerReceiver(mSystemLanguageReceiver, filter);
        }

        if (!makeDirForPath(mPathsMap.get(UNLOCK_RECOG_PATTERN))
                || !makeDirForPath(mPathsMap.get(UNLOCK_FEATURE_FILE))
                || !makeDirForPath(mPathsMap.get(UNLOCK_PSWD_FILE))
                || !makeDirForPath(mPathsMap.get(ANYONE_RECOG_PATTERN))
                || !makeDirForPath(mPathsMap.get(ANYONE_FEATURE_FILE))
                || !makeDirForPath(mPathsMap.get(ANYONE_PSWD_FILE))
                || !makeDirForPath(mPathsMap.get(COMMAND_RECOG_PATTERN))
                || !makeDirForPath(mPathsMap.get(COMMAND_FEATURE_FILE))
                || !makeDirForPath(mPathsMap.get(COMMAND_PSWD_FILE))
                || !makeDirForPath(mPathsMap.get(TRIGGER_RECOG_PATTERN))
                || !makeDirForPath(mPathsMap.get(TRIGGER_FEATURE_FILE))
                || !makeDirForPath(mPathsMap.get(TRIGGER_PSWD_FILE))
                || !makeDirForPath(mPathsMap.get(WAKE_UP_INFO_FILE))
                || !makeDirForPath(mPathsMap.get(CONTACTS_DB_FILE))) {
            mIsCfgPrepared = false;
        }
        if (mIsCfgPrepared) {
            Log.i(TAG, "[ConfigurationManager]new,mIsCfgPrepared is true.");
            checkVoiceCachePref();
            if (mCurrentLanguageIndex >= 0) {
                mConfigurationXml.readKeyWordFromXml(mVoiceKeyWordInfosMap,
                        mVoiceLanguageInfoArrayList.get(mCurrentLanguageIndex).mFilePath);
            }
        }
        mIsDemoSupported = VoiceWakeupBusiness.isWakeupSupport(mContext)
                && !VoiceTriggerBusiness.isTriggerSupport(mContext);
        if (isDemoSupported()) {
            mWakeupMode = VoiceWakeupBusiness.getWakeupMode(mContext);
            mWakeupStatus = VoiceWakeupBusiness.getWakeupCmdStatus(mContext);
        } else {
            mWakeupMode = VoiceTriggerBusiness.getTriggerMode(mContext);
            mWakeupStatus = VoiceTriggerBusiness.getTriggerCmdStatus(mContext);
        }
    }

    public static ConfigurationManager getInstance(Context context) {
        if (null == sCfgMgr) {
            synchronized (ConfigurationManager.class) {
                if (null == sCfgMgr) {
                    sCfgMgr = new ConfigurationManager(context);
                }
            }

        }
        return sCfgMgr;
    }

    /**
     * Handle the case of using the system language.
     * 
     */
    public void useSystemLanguage() {
        mCurrentLanguageIndex = -1;
        String systemLanguage = Locale.getDefault().getLanguage() + "-"
                + Locale.getDefault().getCountry();

        SharedPreferences languagePref = mContext.getSharedPreferences(VOICE_LANGUAGE_CACHE_FILE,
                Context.MODE_PRIVATE);
        boolean isFirstBoot = languagePref.getBoolean(IS_FIRST_BOOT, true);
        Log.i(TAG, "[useSystemLanguage]isFirstBoot = " + isFirstBoot + ",systemLanguage = "
                + systemLanguage);
        updateCurLanguageIndex(systemLanguage);
        if (mCurrentLanguageIndex < 0) {
            if (isFirstBoot) {
                updateCurLanguageIndex(mVoiceCustomization.mDefaultLanguage);
            } else {
                mCurrentLanguageIndex = languagePref.getInt(CUR_SYSTEM_LANGUAGE_INDEX,
                        mCurrentLanguageIndex);
            }
        }
        if (isFirstBoot) {
            languagePref.edit().putBoolean(IS_FIRST_BOOT, false).apply();
        }
    }

    /**
     * Update mCurrentLanguageIndex.
     * 
     * @param language
     *            local language string
     */
    public void updateCurLanguageIndex(String language) {
        Log.w(TAG, "[updateCurLanguageIndex]language = " + language);
        if (language == null) {
            Log.w(TAG, "[updateCurLanguageIndex]language is null,return.");
            return;
        }
        for (int i = 0; i < mVoiceLanguageInfoArrayList.size(); i++) {
            if (language.equals(mVoiceLanguageInfoArrayList.get(i).mLanguageCode)) {
                mCurrentLanguageIndex = i;
                SharedPreferences languagePref = mContext.getSharedPreferences(
                        VOICE_LANGUAGE_CACHE_FILE, Context.MODE_PRIVATE);
                languagePref.edit().putInt(CUR_SYSTEM_LANGUAGE_INDEX, mCurrentLanguageIndex)
                        .apply();
                break;
            }
        }
    }

    /**
     * Check whether configuration file is prepared.
     */
    public boolean isCfgPrepared() {
        return mIsCfgPrepared;
    }

    public boolean getIsSystemLanguage() {
        return mIsSystemLanguage;
    }

    /**
     * Check whether process is Allowed to register or not.
     * 
     * @param processname
     *            process name
     * @return result
     */
    public int isAllowProcessRegister(String processname) {
        return mVoiceProcessInfosMap.containsKey(processname) ? VoiceCommandListener.VOICE_NO_ERROR
                : VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
    }

    /**
     * Get the numbers of UiFeature.
     * 
     * @return the numbers of UiFeature
     */
    public int getUiFeatureNumber() {
        return mVoiceUiFeatureNameArrayList.size();
    }

    /**
     * Check whether process has permission to do action or not.
     * 
     * @param featurename
     *            feature or package name of application
     * @param mainaction
     *            main action
     * @return true if has operation permission
     */
    public boolean hasOperationPermission(String featurename, int mainaction) {
        Log.i(TAG, "[hasOperationPermission]featurename = " + featurename);
        VoiceProcessInfo info = mVoiceProcessInfosMap.get(featurename);
        if (info == null) {
            Log.i(TAG, "[hasOperationPermission]no this permission,return false.");
            return false;
        }
        return info.mPermissionIDList.contains(mainaction);
    }

    /**
     * Check whether process has permission to do actions in array or not.
     * 
     * @param featurename
     *            feature or package name of application
     * @param mainaction
     *            main action
     * @return true if has operation permission
     */
    public boolean containOperationPermission(String featurename, int[] mainaction) {
        Log.i(TAG, "[containOperationPermission]featurename = " + featurename);
        if (mainaction == null || mainaction.length == 0) {
            Log.w(TAG, "[containOperationPermission]mainaction = " + mainaction);
            return false;
        }
        VoiceProcessInfo info = mVoiceProcessInfosMap.get(featurename);
        if (info == null) {
            Log.w(TAG, "[containOperationPermission]info is null.");
            return false;
        }
        for (int i = 0; i < mainaction.length; i++) {
            if (info.mPermissionIDList.contains(mainaction[i])) {
                return true;
            }
        }
        return false;
    }

    /**
     * Get the feature name list and the process enable list.
     * 
     * @return the feature name list
     */
    public String[] getFeatureNameList() {
        int size = mVoiceUiFeatureNameArrayList.size();
        if (size == 0) {
            Log.w(TAG, "[getFeatureNameList]size is 0.");
            return null;
        }
        return mVoiceUiFeatureNameArrayList.toArray(new String[size]);
    }

    /**
     * Get the feature enable list.
     * 
     * @return the feature enable list
     */
    public int[] getFeatureEnableArray() {
        int size = mVoiceUiFeatureNameArrayList.size();
        if (size == 0) {
            Log.w(TAG, "[getFeatureEnableArray]size is 0.");
            return null;
        }
        int[] isEnableArray = new int[size];

        for (int i = 0; i < size; i++) {
            VoiceProcessInfo info = mVoiceProcessInfosMap.get(mVoiceUiFeatureNameArrayList.get(i));
            if (info != null) {
                isEnableArray[i] = info.mIsVoiceEnable ? 1 : 0;
            }
        }
        return isEnableArray;
    }

    /**
     * Update Feature enable in mVoiceProcessInfos.
     * 
     * @param featurename
     *            application feature name
     * @param enable
     *            true if want to enable
     * @return true if update success
     */
    public boolean updateFeatureEnable(String featurename, boolean enable) {
        Log.i(TAG, "[updateFeatureEnable]featurename = " + featurename + ",enable = " + enable);
        VoiceProcessInfo info = mVoiceProcessInfosMap.get(featurename);
        if (info == null) {
            Log.w(TAG, "[updateFeatureEnable]info is null.");
            return false;
        }

        info.mIsVoiceEnable = enable;
        if (info.mRelationProcessName != null
                && !info.mRelationProcessName.equals(ConfigurationXml.sPublicFeatureName)) {
            updateFeatureEnable(info.mRelationProcessName, enable);
        }
        updateFeatureListEnableToPref();

        return true;
    }

    /**
     * Get language list that this feature support.
     * 
     * @return language list
     */
    public String[] getLanguageList() {
        int size = mVoiceLanguageInfoArrayList.size();
        if (size == 0) {
            Log.w(TAG, "[getLanguageList]size is 0.");
            return null;
        }
        String[] language = new String[size];
        for (int i = 0; i < size; i++) {
            language[i] = mVoiceLanguageInfoArrayList.get(i).mName;
        }

        return language;
    }

    /**
     * Get Current Language.
     * 
     * @return the current Language
     */
    public int getCurrentLanguage() {
        return mCurrentLanguageIndex;
    }

    /**
     * Get Current Language ID defined in voicelanguage.xml.
     * 
     * @return the current Language ID
     */
    public int getCurrentLanguageID() {
        return mCurrentLanguageIndex < 0 ? mCurrentLanguageIndex : mVoiceLanguageInfoArrayList
                .get(mCurrentLanguageIndex).mLanguageID;
    }

    /**
     * Get Override Trigger Mode value within range of Wakeup mode.
     * Since native only able to handle command/anyone, so need this override.
     *
     * @param mode
     *            wakeup mode
     * @return the mapped value as wakeup anyone/command
     */
    public int getOverrideTriggerToWakeupMode(int mode) {
        return (VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER == mode) ?
            VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT : mode;
    }

    /**
     * Get Current Wakeup Info defined in voiceweakup.xml.
     * 
     * @param mode
     *            wakeup mode
     * @return wakeup information
     */
    public VoiceWakeupInfo[] getCurrentWakeupInfo(int mode) {
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT == mode) {
            if ((mWakeupAnyoneArrayList == null) || (mWakeupAnyoneArrayList.size() == 0)) {
                return null;
            }
            return mWakeupAnyoneArrayList
                    .toArray(new VoiceWakeupInfo[mWakeupAnyoneArrayList.size()]);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT == mode) {
            if ((mWakeupCommandArrayList == null) || (mWakeupCommandArrayList.size() == 0)) {
                return null;
            }
            return mWakeupCommandArrayList.toArray(new VoiceWakeupInfo[mWakeupCommandArrayList
                    .size()]);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER == mode) {
            if ((mWakeupTriggerArrayList == null) || (mWakeupTriggerArrayList.size() == 0)) {
                return null;
            }
            return mWakeupTriggerArrayList
                    .toArray(new VoiceWakeupInfo[mWakeupTriggerArrayList.size()]);
        }
        return null;
    }

    /**
     * Get APP Label from package name and class name.
     * 
     * @param packageName
     *            package name
     * @param className
     *            class name
     * @return application label
     */
    public String getAppLabel(String packageName, String className) {

        ComponentName component = new ComponentName(packageName, className);
        ActivityInfo info;
        try {
            info = mContext.getPackageManager().getActivityInfo(component, 0);
        } catch (NameNotFoundException e) {
            Log.e(TAG, "[getAppLabel] not found packageName: " + packageName);
            return null;
        }
        CharSequence name = info.loadLabel(mContext.getPackageManager());
        String appLabel = name.toString();
        Log.i(TAG, "[getAppLabel] appLabel:" + appLabel);

        return appLabel;
    }

    public void updateCurLanguageKeyword() {
        synchronized (mVoiceKeyWordInfosMap) {
            mVoiceKeyWordInfosMap.clear();
            if (mCurrentLanguageIndex >= 0) {
                mConfigurationXml.readKeyWordFromXml(mVoiceKeyWordInfosMap,
                        mVoiceLanguageInfoArrayList.get(mCurrentLanguageIndex).mFilePath);
            }
        }
    }

    /**
     * Set Current Language.
     * 
     * @param languageIndex
     *            the current language index
     */
    public void setCurrentLanguage(int languageIndex) {
        mCurrentLanguageIndex = languageIndex;
        updateCurLanguageToPref();
    }

    /**
     * Get process ID defined in voiceprocessinfo.xml.
     * 
     * @param featurename
     *            application feature name
     * 
     * @return process ID
     */
    public int getProcessID(String featurename) {
        VoiceProcessInfo processinfo = mVoiceProcessInfosMap.get(featurename);
        if (processinfo == null) {
            return -1;
        }

        return processinfo.mID;
    }

    /**
     * Check process is enable or not.
     * 
     * @param featurename
     *            application feature name
     * @return true if the feature name is enable
     */
    public boolean isProcessEnable(String featurename) {
        VoiceProcessInfo processinfo = mVoiceProcessInfosMap.get(featurename);
        if (processinfo == null) {
            return false;
        }
        return processinfo.mIsVoiceEnable;
    }

    /**
     * Get VoiceProcessInfo from processname.
     * 
     * @param processname
     *            process name
     * @return VoiceProcessInfo
     */
    public VoiceProcessInfo getProcessInfo(String processname) {
        return mVoiceProcessInfosMap.get(processname);
    }

    /**
     * Get Voice Recognition Pattern File Path from data/data.
     * 
     * @param mode
     *            Voice Wakeup mode
     * @return data/data/com.mediatek.voicecommand/training/***
     */
    public String getVoiceRecognitionPatternFilePath(int mode) {
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT == mode) {
            return mPathsMap.get(ANYONE_RECOG_PATTERN);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT == mode) {
            return mPathsMap.get(COMMAND_RECOG_PATTERN);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER == mode) {
            return mPathsMap.get(TRIGGER_RECOG_PATTERN);
        }
        return mPathsMap.get(UNLOCK_RECOG_PATTERN);
    }

    /**
     * Get Feature File Path from data/data.
     * 
     * @param mode
     *            Voice Wakeup mode
     * @return data/data/com.mediatek.voicecommand/training/***
     */
    public String getFeatureFilePath(int mode) {
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT == mode) {
            return mPathsMap.get(ANYONE_FEATURE_FILE);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT == mode) {
            return mPathsMap.get(COMMAND_FEATURE_FILE);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER == mode) {
            return mPathsMap.get(TRIGGER_FEATURE_FILE);
        }
        return mPathsMap.get(UNLOCK_FEATURE_FILE);
    }

    /**
     * Get Password File Path from data/data.
     * 
     * @param mode
     *            Voice Wakeup mode
     * @return data/data/com.mediatek.voicecommand/training/***
     */
    public String getPasswordFilePath(int mode) {
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_INDEPENDENT == mode) {
            return mPathsMap.get(ANYONE_PSWD_FILE);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT == mode) {
            return mPathsMap.get(COMMAND_PSWD_FILE);
        }
        if (VoiceCommandListener.VOICE_WAKEUP_MODE_TRIGGER == mode) {
            return mPathsMap.get(TRIGGER_PSWD_FILE);
        }
        return mPathsMap.get(UNLOCK_PSWD_FILE);
    }

    /**
     * Get Voice Wakeup info Path from data/data/com.mediatek.voicecommand.
     * 
     * @return Voice Wakeup info Path
     */
    public String getWakeupInfoPath() {
        return mPathsMap.get(WAKE_UP_INFO_FILE);
    }

    /**
     * Get Voice Wakeup mode from setting provider.
     * 
     * @return Voice Wakeup mode
     */
    public int getWakeupMode() {
        return mWakeupMode;
    }

    /**
     * Set Voice Wakeup mode to setting provider.
     * 
     * @param mode
     *            Voice Wakeup mode
     */
    public void setWakeupMode(int mode) {
        mWakeupMode = mode;
    }

    /**
     * Get Voice Wakeup Status from setting provider.
     * 
     * @return Voice Wakeup Status
     */
    public int getWakeupCmdStatus() {
        return mWakeupStatus;
    }

    /**
     * Set Voice Wakeup Status to setting provider.
     * 
     * @param wakeupStatus
     *            Voice Wakeup Status
     */
    public void setWakeupStatus(int wakeupStatus) {
        mWakeupStatus = wakeupStatus;
    }

    /**
     * Get Voice Trigger Status from setting provider.
     *
     * @return Voice Trigger Status
     */
    public int getTriggerCmdStatus() {
        return mWakeupStatus;
    }

    /**
     * Set Voice Trigger Status to setting provider.
     *
     * @param wakeupStatus
     *            Voice Trigger Status
     */
    public void setTriggerStatus(int triggerStatus) {
        mWakeupStatus = triggerStatus;
    }

    /**
     * Get VoiceUI Pattern Path from voicecommandpath.xml.
     * 
     * @return VoiceUI Pattern Path
     */
    public String getVoiceUIPatternPath() {
        return mPathsMap.get(UI_PATTERN);
    }

    /**
     * Get Contacts db File Path from voicecommandpath.xml.
     * 
     * @return Contacts db File Path
     */
    public String getContactsdbFilePath() {
        return mPathsMap.get(CONTACTS_DB_FILE);
    }

    /**
     * Get UBM File Path from voicecommandpath.xml.
     *
     * @return UBM File Path
     */
    public String getUbmFilePath() {
        return mPathsMap.get(UBM_FILE)
            + getKeywordString(mContext, 0);
    }

    /**
     * Get UBM File Path from voicecommandpath.xml.
     *
     * @param keyphraseId
     *            keyphrase id to use for getting UBM path
     * @return UBM File Path
     */
    public String getUbmFilePath(int keyphraseId) {
        return mPathsMap.get(UBM_FILE)
            + getKeywordString(mContext, keyphraseId);
    }

    /**
     * Update UBM File Path
     *
     * @param path
     *            The new path
     */
    public void setUbmFilePath(String path) {
        mPathsMap.put(UBM_FILE, path);
    }

    /**
     * Get UBM Upgrade File Path if have.
     *
     * @return UBM File Upgrade Path
     */
    public String getUbmFileUpgradePath() {
        return mPathsMap.get(UBM_FILE_UPGRADE);
    }

    /**
     * Update UBM File Upgrade Path
     *
     * @param path
     *            The new path
     */
    public void setUbmFileUpgradePath(String path) {
        mPathsMap.put(UBM_FILE_UPGRADE, path);
    }

    /**
     * Get Model File Path from voicecommandpath.xml.
     * 
     * @return Model File Path
     */
    public String getModelFile() {
        return mPathsMap.get(MODE_FILE);
    }

    /**
     * Get Contacts Model File Path from voicecommandpath.xml.
     * 
     * @return Contacts Model File Path
     */
    public String getContactsModelFile() {
        return mPathsMap.get(CONTACTS_MODE_FILE);
    }

    /**
     * Get KeyWord refer to processName defined in keyword.xml.
     * 
     * @param processName
     *            feature name
     * @return KeyWord list
     */
    public String[] getKeyWord(String processName) {
        synchronized (mVoiceKeyWordInfosMap) {
            // return mVoiceKeyWordInfos.get(processName).mKeyWordArray;
            VoiceKeyWordInfo info = mVoiceKeyWordInfosMap.get(processName);
            if (info == null) {
                Log.w(TAG, "[getKeyWord]info is null.1");
                return null;
            }
            return info.mKeyWordArray;
        }
    }

    /**
     * Get command path refer to processName and id defined in keyword.xml.
     * 
     * @param processName
     *            feature name
     * 
     * @return command path
     */
    public String getCommandPath(String processName) {
        synchronized (mVoiceKeyWordInfosMap) {
            // return mVoiceKeyWordInfos.get(processName).mKeyWordPath;
            VoiceKeyWordInfo info = mVoiceKeyWordInfosMap.get(processName);
            if (info == null) {
                Log.w(TAG, "[getCommandPath]info is null.1");
                return null;
            }
            return info.mKeyWordPath;
        }
    }

    /**
     * Get KeyWord refer to processName defined in keyword.xml.
     * 
     * @param processName
     *            feature name
     * @return KeyWord list
     */
    public String[] getKeyWordForSettings(String processName) {
        synchronized (mVoiceKeyWordInfosMap) {
            VoiceProcessInfo info = mVoiceProcessInfosMap.get(processName);
            if (info != null && info.mRelationProcessName != null) {
                ArrayList<String> keywordList = new ArrayList<String>();
                // String[] keyword =
                // mVoiceKeyWordInfos.get(processName).mKeyWordArray;
                VoiceKeyWordInfo keyWordInfo = mVoiceKeyWordInfosMap.get(processName);
                if (keyWordInfo == null) {
                    Log.w(TAG, "[getKeyWordForSettings]keyWordInfo is null.1");
                    return null;
                }
                String[] keyword = keyWordInfo.mKeyWordArray;
                if (keyword != null) {
                    for (int i = 0; i < keyword.length; i++) {
                        keywordList.add(keyword[i]);
                    }
                }
                // String[] keywordRelation =
                // mVoiceKeyWordInfos.get(info.mRelationProcessName).mKeyWordArray;
                keyWordInfo = mVoiceKeyWordInfosMap.get(info.mRelationProcessName);
                if (keyWordInfo == null) {
                    Log.w(TAG, "[getKeyWordForSettings]keyWordInfo is null.2");
                    return null;
                }
                String[] keywordRelation = keyWordInfo.mKeyWordArray;
                if (keywordRelation != null) {
                    for (int i = 0; i < keywordRelation.length; i++) {
                        if (!keywordList.contains(keywordRelation[i])) {
                            keywordList.add(keywordRelation[i]);
                        }
                    }
                }

                return keywordList.toArray(new String[keywordList.size()]);
            } else {
                // return mVoiceKeyWordInfos.get(processName).mKeyWordArray;
                VoiceKeyWordInfo keyWordInfo = mVoiceKeyWordInfosMap.get(processName);
                if (keyWordInfo == null) {
                    Log.w(TAG, "[getKeyWordForSettings]keyWordInfo is null.3");
                    return null;
                }
                return keyWordInfo.mKeyWordArray;
            }
        }
    }

    /**
     * Get Process Name list according to featureName.
     * 
     * @param featureName
     *            feature name
     * @return process name list
     */
    public ArrayList<String> getProcessName(String featureName) {
        VoiceProcessInfo processinfo = mVoiceProcessInfosMap.get(featureName);
        if (processinfo == null) {
            return null;
        }
        return processinfo.mProcessNameList;
    }

    /**
     * ConfigurationManager release when VoiceCommandManagerService destroy.
     */
    public void release() {
        Log.i(TAG, "[release]mIsSystemLanguage : " + mIsSystemLanguage);
        if (mIsSystemLanguage) {
            mContext.unregisterReceiver(mSystemLanguageReceiver);
            sCfgMgr = null;
        }
    }

    /**
     * Create dir and file.
     * 
     * @param path
     *            file path
     * @return true if make dir success
     */
    public static boolean makeDirForPath(String path) {
        if (path == null) {
            Log.w(TAG, "[makeDirForPath]path is null");
            return false;
        }
        try {
            File dir = new File(path);
            if (!dir.exists()) {
                dir.mkdirs();
                FileUtils.setPermissions(dir.getPath(), 0775, -1, -1); // dwxrwxr-x
            }
        } catch (NullPointerException ex) {
            Log.e(TAG, "[makeDirForPath]NullPointerException.");
            return false;
        }
        return true;
    }

    /**
     * Create dir and file.
     * 
     * @param file
     *            file which want to be made dir
     * @return true if make dir success
     */
    public static boolean makeDirForFile(String file) {
        if (file == null) {
            Log.w(TAG, "[makeDirForFile]file is null");
            return false;
        }
        try {
            File f = new File(file);
            File dir = f.getParentFile();
            if (dir != null && !dir.exists()) {
                dir.mkdirs();
                FileUtils.setPermissions(dir.getPath(), 0775, -1, -1); // dwxrwxr-x
            }
            FileUtils.setPermissions(f.getPath(), 0666, -1, -1); // -rw-rw-rw-
        } catch (NullPointerException ex) {
            Log.e(TAG, "[makeDirForFile]NullPointerException.");
            return false;
        }

        return true;
    }

    /**
     * Read Pref in mVoiceProcessInfos and mCurrentLanguageIndex.
     */
    private void checkVoiceCachePref() {
        SharedPreferences processPref = mContext.getSharedPreferences(VOICE_UI_CACHE_FILE,
                Context.MODE_PRIVATE);
        Map<String, Boolean> enableMap;
        try {
            enableMap = (Map<String, Boolean>) processPref.getAll();
        } catch (NullPointerException ex) {
            Log.e(TAG, "[checkVoiceCachePref] NullPointerException.");
            enableMap = null;
        }

        if (enableMap != null) {
            Iterator iter = enableMap.entrySet().iterator();
            while (iter.hasNext()) {
                Entry entry = (Entry) iter.next();
                String name = (String) entry.getKey();
                VoiceProcessInfo info = mVoiceProcessInfosMap.get(name);
                if (info != null) {
                    info.mIsVoiceEnable = (Boolean) entry.getValue();
                    if (info.mRelationProcessName != null) {
                        VoiceProcessInfo relationInfo = mVoiceProcessInfosMap
                                .get(info.mRelationProcessName);
                        if (relationInfo != null) {
                            relationInfo.mIsVoiceEnable = info.mIsVoiceEnable;
                        }
                    }
                }
            }
        }

        if (mIsSystemLanguage) {
            useSystemLanguage();
        } else {
            processPref = mContext.getSharedPreferences(VOICE_LANGUAGE_CACHE_FILE,
                    Context.MODE_PRIVATE);
            mCurrentLanguageIndex = processPref.getInt(CUR_LANGUAGE_INDEX, mCurrentLanguageIndex);
        }
        Log.d(TAG, "[checkVoiceCachePref]mCurrentLanguageIndex = " + mCurrentLanguageIndex);
    }

    /**
     * Receive Local change broadcast.
     */
    private BroadcastReceiver mSystemLanguageReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "[onReceive]...");
            if ((Intent.ACTION_LOCALE_CHANGED).equals(intent.getAction())) {
                String systemLanguage = Locale.getDefault().getLanguage() + "-"
                        + Locale.getDefault().getCountry();
                Log.d(TAG, "[onReceive]mSystemLanguageReceiver systemLanguage : " + systemLanguage);
                updateCurLanguageIndex(systemLanguage);
                updateCurLanguageKeyword();
            }
        }
    };

    /**
     * Save Current Language to Shared Preferences.
     */
    private void updateCurLanguageToPref() {
        Log.d(TAG, "[updateCurLanguageToPref]...");
        synchronized (mVoiceLanguageInfoArrayList) {
            SharedPreferences processPref = mContext.getSharedPreferences(
                    VOICE_LANGUAGE_CACHE_FILE, Context.MODE_PRIVATE);
            processPref.edit().putInt(CUR_LANGUAGE_INDEX, mCurrentLanguageIndex).apply();
        }
        updateCurLanguageKeyword();
    }

    /**
     * Save Feature enable to Shared Preferences.
     */
    private void updateFeatureListEnableToPref() {
        Log.d(TAG, "[updateFeatureListEnableToPref]...");
        synchronized (mVoiceProcessInfosMap) {
            SharedPreferences processPref = mContext.getSharedPreferences(VOICE_UI_CACHE_FILE,
                    Context.MODE_PRIVATE);
            for (String featurename : mVoiceUiFeatureNameArrayList) {
                VoiceProcessInfo info = mVoiceProcessInfosMap.get(featurename);
                if (info != null) {
                    processPref.edit().putBoolean(featurename, info.mIsVoiceEnable).apply();
                }
            }
        }
    }

    /**
     * Read values from Trigger Cache Preferences.
     */
    private void readTriggerParamCacheFile() {
        Log.d(TAG, "[readTriggerParamCacheFile]...");
        SharedPreferences processPref = mContext.getSharedPreferences(
                VOICE_TRIGGER_PARAM_CACHE_FILE, Context.MODE_PRIVATE);
        mCoarseConfidence = processPref.getInt(PARAM_COARSE_CONFIDENCE,
                FIRST_CONFIDENCE_THRESHOLD_DEFAULT);
        mSecondStageConfidence = processPref.getInt(SECOND_STAGE_THRESHOLD,
                SECOND_CONFIDENCE_THRESHOLD_DEFAULT);
        mTrainingConfidence = processPref.getInt(TRAINING_CONFIDENCE_THRESHOLD,
                TRAINING_CONFIDENCE_THRESHOLD_DEFAULT);
        mTrainingLocale = processPref.getString(CURRENT_TRAINING_LOCALE,
                VOW_ENROLLMENT_DEFAULT_LOCALE);
        Integer enrolledUserId = new Integer(processPref.getInt(ENROLLED_USER_ID, -1));
        if (enrolledUserId.intValue() != -1) {
            mEnrolledUsers.add(enrolledUserId);
        }
    }


    /**
     * Get CoarseConfidence Preferences.
     *
     * @return result
     */
    public int getCoarseConfidence() {
        Log.d(TAG, "[getCoarseConfidence]...");
        if (mCoarseConfidence == -1) {
            readTriggerParamCacheFile();
        }
        return mCoarseConfidence;
    }

    /**
     * Get Second stage confidence threshold
     *
     * @return result
     */
    public int getSecondStageThreshold() {
        Log.d(TAG, "[getSecondStageThreshold]...");
        if (mSecondStageConfidence == -1) {
            readTriggerParamCacheFile();
        }
        return mSecondStageConfidence;
    }

    /**
     * Get Training confidence threshold from Shared Preferences.
     *
     * @return result
     */
    public int getTrainingThreshold() {
        Log.d(TAG, "[getTrainingThreshold]...");
        if (mTrainingConfidence == -1) {
            readTriggerParamCacheFile();
        }
        return mTrainingConfidence;
    }

    /**
     * Get Training language from Shared Preferences.
     *
     * @return result
     */
    public String getTrainingLocale() {
        Log.d(TAG, "[getTrainingLocale]...");
        if (mTrainingLocale.equals("")) {
            readTriggerParamCacheFile();
        }
        return mTrainingLocale;
    }

    /**
     * Get Enrolled users.
     *
     * @return result
     */
    public ArrayList<Integer> getEnrolledUsers(int triggerStatus) {
        Log.d(TAG, "[getEnrolledUsers]...");
        if (triggerStatus == VoiceCommandListener.VOICE_WAKEUP_STATUS_NOCOMMAND_UNCHECKED) {
            return new ArrayList<Integer>(); //Empty list
        } else if (mEnrolledUsers.isEmpty()) {
            readTriggerParamCacheFile();
        }
        return mEnrolledUsers;
    }

    /**
     * Save CoarseConfidence Preferences.
     *
     * @param confidence
     *            parameter value to update to preference
     */
    public void updateCoarseConfidenceToPref(int confidence) {
        Log.d(TAG, "[updateCoarseConfidenceToPref]...");
        mCoarseConfidence = confidence;
        SharedPreferences processPref = mContext.getSharedPreferences(
                VOICE_TRIGGER_PARAM_CACHE_FILE, Context.MODE_PRIVATE);
        processPref.edit().putInt(PARAM_COARSE_CONFIDENCE, mCoarseConfidence).apply();
    }

    /**
     * Save Second stage confidence threshold to Shared Preferences.
     *
     * @param confidence
     *            parameter value to update to preference
     */
    public void updateSecondStageThresholdToPref(int confidence) {
        Log.d(TAG, "[updateSecondStageThresholdToPref]...");
        mSecondStageConfidence = confidence;
        SharedPreferences processPref = mContext.getSharedPreferences(
                VOICE_TRIGGER_PARAM_CACHE_FILE, Context.MODE_PRIVATE);
        processPref.edit().putInt(SECOND_STAGE_THRESHOLD, mSecondStageConfidence).apply();
    }


    /**
     * Save training confidence Preferences.
     *
     * @param file
     *            parameter value to update to preference
     */
    public void updateTrainingConfidenceToPref(int confidence) {
        Log.d(TAG, "[updateTrainingConfidenceToPref]...");
        mTrainingConfidence = confidence;
        SharedPreferences processPref = mContext.getSharedPreferences(
                VOICE_TRIGGER_PARAM_CACHE_FILE, Context.MODE_PRIVATE);
        processPref.edit().putInt(TRAINING_CONFIDENCE_THRESHOLD, mTrainingConfidence).apply();
    }

    /**
     * Save training confidence Preferences.
     *
     * @param file
     *            parameter value to update to preference
     */
    public void setTrainingLocale(String locale) {
        Log.d(TAG, "[setTrainingLocale]...");
        mTrainingLocale = locale;
        SharedPreferences processPref = mContext.getSharedPreferences(
                VOICE_TRIGGER_PARAM_CACHE_FILE, Context.MODE_PRIVATE);
        processPref.edit().putString(CURRENT_TRAINING_LOCALE, mTrainingLocale).apply();
    }

    /**
     * Set Enrolled users.
     *
     * @param file
     *            parameter value to update to preference
     */
    public void setEnrolledUser(int userId) {
        //Since right now we support only 1 user, it is easy to maintain as single value
        //In future, we may need to use Array, so count and key will be used.
        Log.d(TAG, "[setEnrolledUser]...");
        if (mEnrolledUsers.isEmpty()) {
            mEnrolledUsers.add(new Integer(userId));
            SharedPreferences processPref = mContext.getSharedPreferences(
                    VOICE_TRIGGER_PARAM_CACHE_FILE, Context.MODE_PRIVATE);
            processPref.edit().putInt(ENROLLED_USER_ID, userId).apply();
        }
    }

    /**
     * Reset Enrolled user.
     *
     * @param userId
     *            parameter value for which update to preference is needed
     */
    public void resetEnrolledUser(int userId) {
        Log.d(TAG, "[resetEnrolledUser]...");
        if (!mEnrolledUsers.isEmpty()) {
            mEnrolledUsers.remove(new Integer(userId));
            SharedPreferences processPref = mContext.getSharedPreferences(
                    VOICE_TRIGGER_PARAM_CACHE_FILE, Context.MODE_PRIVATE);
            processPref.edit().remove(ENROLLED_USER_ID).apply();
        }
    }

    /**
     * Check if the parameter value is within range of confidence.
     *
     * @param thresholdValue
     *            parameter value to compare for range check
     * @return true if the confidence value is within expected range
     */
    public boolean isValidTriggerConfidenceRange(int thresholdValue) {
        Log.d(TAG, "[isValidTriggerConfidenceRange]...");
        if (VoiceCommandListener.VOICE_CONFIDENCE_THRESHOLD_MIN > thresholdValue ||
            VoiceCommandListener.VOICE_CONFIDENCE_THRESHOLD_MAX < thresholdValue) {
            Log.d(TAG, "[isValidTriggerConfidenceRange] invalid value: " + thresholdValue);
            return false;
        }
        return true;
    }

    /**
     * Condition to check if we need to support the demo app or not.
     * @return true if need to support demo app
     */
    public boolean isDemoSupported() {
        return mIsDemoSupported;
    }

    /**
     * Get keyword string mapped for a particular id.
     * @param context
     *            context
     * @param id
     *            commandId
     * @return string keyword associated with MTK_VOW_COMMAND macro
     */
    public static String getKeywordString(Context context, int id) {
        String keyword = null;
        if ((id >= 0 && id < VOW_KEYPHRASES.length)) {
            keyword = VOW_KEYPHRASES[id];
        }
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        if (keyword == null || keyword.isEmpty()) {
            return null;
        } else {
            String[] result = am.getParameters(keyword).split(AUDIO_MANAGER_VALUE_SEPARATOR);
            if ((result.length == 2) && !result[1].equals("none")) {
                //MTK_VOW_COMMAND=keyword01
                return result[1] + "/";
            } else {
                return null;
            }
        }
    }

    /**
     * Check if there is a keyphrase mapped to command id.
     * @param context
     *            context
     * @param id
     *            commandId
     * @return true is keyword value is not none
     */
    public static boolean isValidCommandId(Context context, int id) {
        return (getKeywordString(context, id) != null);
    }

    /**
     * Get active command id value.
     *
     * @return commandId
     */
    public int getActiveCommandId() {
        int activeCmdValue = 1; /*Default set to 1 means use commandId 0 for trigger*/
        if (mActiveCommandId < 0) {
            /*Need to initialize*/
            if (isDemoSupported()) {
                activeCmdValue = VoiceWakeupBusiness.getWakeupActiveCmdId(mContext);
            }

            int maxCount = getCurrentWakeupInfo(getWakeupMode()).length;
            for (int i = 0; i < maxCount; i++) {
                if (checkCommandActive(activeCmdValue, i)) {
                    //If this is Dynamic switch wakeup, then only single command will be active
                    mActiveCommandId = i;
                    break;
                }
            }
        }
        return mActiveCommandId;
    }

    /**
     * Set active command id value.
     *
     * @param commandId
     *            command Id
     */
    public void setActiveCommandId(int commandId) {
        mActiveCommandId = commandId;
        setCommandActive(mContext, mActiveCommandId);
    }

    /**
     * Check if the commandId is active.
     * So, we need to convert and get valid command id.
     *
     * @param activeCmdValue
     *            activeCmdValue
     * @param commandId
     *            command Id
     * @return wakeup command status
     */
    public boolean checkCommandActive(int activeCmdValue, int commandId) {
        //If this is Dynamic switch wakeup, then only single command will be active
        //If this is Concurrent wakeup, then multiple command will be active
        boolean isCommandActive = (activeCmdValue & (0x01 << commandId)) != 0;
        Log.i(TAG,
                "[checkCommandActive] commandId : " + commandId
                + ", isActive" + isCommandActive);

        return isCommandActive;
    }

    /**
     * Check if the commandId is active.
     * So, we need to convert and get valid command id.
     *
     * @param context
     *            context
     * @param commandId
     *            command Id
     * @return wakeup command status
     */
    public boolean checkCommandActive(Context context, int commandId) {
        int activeCmdValue = 1;
        if (isDemoSupported()) {
            activeCmdValue = VoiceWakeupBusiness.getWakeupActiveCmdId(context);
        }
        //If this is Dynamic switch wakeup, then only single command will be active
        //If this is Concurrent wakeup, then multiple command will be active
        boolean isCommandActive = ((activeCmdValue & (0x01 << commandId)) != 0);
        Log.i(TAG,
                "[checkCommandActive] commandId : " + commandId
                + ", isActive" + isCommandActive);

        return isCommandActive;
    }

    /**
     * Set commandId value.
     * Right now we only write logic for dynamic keyphrase switch. So, single active command.
     *
     * @param context
     *            context
     * @param commandId
     *            command Id
     * @return wakeup command status
     */
    public int setCommandActive(Context context, int commandId) {
        int activeCmdValue = 0;
        //If this is Dynamic switch wakeup, then only single command will be active
        activeCmdValue = activeCmdValue | (0x01 << commandId);
        Log.i(TAG,
                "[checkCommandActive] commandId : " + commandId
                + ", activeCommandValue" + activeCmdValue);

        if (isDemoSupported()) {
            VoiceWakeupBusiness.setWakeupActiveCmdId(context, activeCmdValue);
        }

        //If this is Concurrent wakeup, then multiple command will be active
        return activeCmdValue;
    }
}
