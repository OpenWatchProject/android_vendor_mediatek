/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.keyguard;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.media.AudioManager ;
import android.os.Environment;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;

import com.android.internal.telephony.IccCardConstants;

import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.io.File;

/**
 * Utilities for Keyguard.
 */
public class KeyguardUtils {
    private static final String TAG = "KeyguardUtils";
    private static final boolean DEBUG = KeyguardConstants.DEBUG;
    private KeyguardUpdateMonitor mUpdateMonitor;
    private SubscriptionManager mSubscriptionManager;

    public static final int INVALID_PHONE_ID = -1;
    private static int sSimMECount = 0;

    private static final boolean mIsPrivacyProtectionLockSupport =
        SystemProperties.get("ro.vendor.mtk_privacy_protection_lock").equals("1");

    private static final String MTK_VOW_SUPPORT_State = "MTK_VOW_SUPPORT";
    private static final String MTK_VOW_SUPPORT_On = "MTK_VOW_SUPPORT=true";
    private static final String MTK_TRIGGER_SUPPORT_State = "MTK_VOW_2E2K_SUPPORT";
    private static final String MTK_TRIGGER_SUPPORT_On = "MTK_VOW_2E2K_SUPPORT=true";

    private static KeyguardUtils sInstance = new KeyguardUtils();

    /**
     * Constructor.
     * @param context the context
     */
    public KeyguardUtils(Context context) {
        mUpdateMonitor = KeyguardUpdateMonitor.getInstance(context);
        mSubscriptionManager = SubscriptionManager.from(context);
    }

    private KeyguardUtils() {
    }

    public static KeyguardUtils getDefault() {
        return sInstance;
    }

        /**
     * Return Operator name of related subId.
     * @param phoneId id of phone
     * @param context the context
     * @return operator name.
     */
    public String getOptrNameUsingPhoneId(int phoneId, Context context) {
        int subId = getSubIdUsingPhoneId(phoneId) ;
        SubscriptionInfo info = mSubscriptionManager.getActiveSubscriptionInfo(subId);
        if (null == info) {
           if (DEBUG) {
            Log.d(TAG, "getOptrNameUsingPhoneId, return null");
           }
        } else {
           if (DEBUG) {
               Log.d(TAG, "getOptrNameUsingPhoneId mDisplayName=" + info.getDisplayName());
           }
           if (info.getDisplayName() != null) {
                return info.getDisplayName().toString();
           }
        }
        return null;
    }

        /**
     * Return Operator drawable of related subId.
     * @param phoneId id of phone
     * @param context the context
     * @return operator related drawable.
     */
    public Bitmap getOptrBitmapUsingPhoneId(int phoneId, Context context) {
        int subId = getSubIdUsingPhoneId(phoneId) ;
        Bitmap bgBitmap = null;
        SubscriptionInfo info = mSubscriptionManager.getActiveSubscriptionInfo(subId);
        if (null == info) {
            if (DEBUG) {
                Log.d(TAG, "getOptrBitmapUsingPhoneId, return null");
            }
        } else {
            bgBitmap = info.createIconBitmap(context) ;
        }
        return bgBitmap;
    }

    /********************************************************
     ** Mediatek add begin.
     ********************************************************/
    private static boolean mIsMediatekSimMeLockSupport =
            SystemProperties.get("ro.vendor.sim_me_lock_mode", "0").equals("0");
    private static final int MAX_PHONE_COUNT = 4;
    private static int sPhoneCount = 0 ;

    //Add for support Dismiss button
    private static final String LOCK_REQUEST_NAME = "simme_lock_with_request_";
    private static final String DISMISS_REQUEST_NAME = "dismiss_button_with_request_";
    private static final int MAX_PHONE_SUPPORTED = 15; //Support at most 4 sim cards
    private static boolean mIsDismissEnabled = false;
    private static boolean mIsSimmePolicyEnabled = false;
    private static int mValidPhoneIds = INVALID_PHONE_ID;
    private static boolean mIsDismissSimMeLockSupport = false;
    private static boolean mIsUnlockSimMeWithDeviceSupport = false;
    private static final int SIMME_REQUEST_8 = 17;

    public static final boolean isMediatekSimMeLockSupport() {
        return mIsMediatekSimMeLockSupport;
    }

    public static final boolean isDismissSimMeLockSupport() {
        return mIsDismissSimMeLockSupport;
    }

    /**
     * Return SimMe Lock require 8 support or not.
     */
    public static final boolean isUnlockSimMeWithDeviceSupport() {
        return mIsUnlockSimMeWithDeviceSupport;
    }

    /**
     * Return AirPlane mode is on or not.
     * @param context the context
     * @return airplane mode is on or not
     */
    public static boolean isAirplaneModeOn(Context context) {
        boolean airplaneModeOn = Settings.Global.getInt(context.getContentResolver(),
                                                        Settings.Global.AIRPLANE_MODE_ON,
                                                        0) != 0;
        Log.d(TAG, "isAirplaneModeOn() = " + airplaneModeOn) ;
        return airplaneModeOn ;
    }

    /**
     * if return true, it means that Modem will turn off after entering AirPlane mode.
     * @return support or not
     */
    public static boolean isFlightModePowerOffMd() {
        boolean powerOffMd = SystemProperties.get("ro.vendor.mtk_flight_mode_power_off_md").equals("1") ;
        Log.d(TAG, "powerOffMd = " + powerOffMd) ;
        return powerOffMd ;
    }

    /**
     * Get phone count.
     * @return phone count.
     **/
    public static int getNumOfPhone() {
        if (sPhoneCount == 0) {
            sPhoneCount = TelephonyManager.getDefault().getPhoneCount(); //hw can support in theory
            // MAX_PHONE_COUNT : in fact our ui layout max support 4. maybe update in future
            sPhoneCount = ((sPhoneCount > MAX_PHONE_COUNT) ? MAX_PHONE_COUNT : sPhoneCount);
        }
        return sPhoneCount;
    }

    /**
     * Is phone id valid.
     * @param phoneId phoneId.
     * @return valid or not.
     **/
    public static boolean isValidPhoneId(int phoneId) {
        return (phoneId != SubscriptionManager.DEFAULT_PHONE_INDEX) &&
               (0 <= phoneId) && (phoneId < getNumOfPhone());
    }

    /** get PhoneId from SubManager.
     * @param subId subId
     * @return phoneId
     */
    public static int getPhoneIdUsingSubId(int subId) {
        Log.e(TAG, "getPhoneIdUsingSubId: subId = " + subId);
        int phoneId = SubscriptionManager.getPhoneId(subId);
        if (phoneId < 0 || phoneId >= getNumOfPhone()) {
            Log.e(TAG, "getPhoneIdUsingSubId: invalid phonId = " + phoneId);
        } else {
            Log.e(TAG, "getPhoneIdUsingSubId: get phone ID = " + phoneId);
        }

        return phoneId ;
    }

    /**
     * Send phoneId to Sub-Mgr and get subId.
     * @param phoneId phoneId.
     * @return subid.
     */
    public static int getSubIdUsingPhoneId(int phoneId) {
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(phoneId);
        return subId;
    }

    /**
     * M : fix ALPS01564588. Refresh IME Stauts to hide ALT-BACK key correctly.
     * @param context the context
     */
    public static void requestImeStatusRefresh(Context context) {
        InputMethodManager imm =
            ((InputMethodManager) context.getSystemService(Context.INPUT_METHOD_SERVICE));
        if (imm != null) {
            if (DEBUG) {
                Log.d(TAG, "call imm.requestImeStatusRefresh()");
            }
            // M: ignor for BSP+
            // imm.refreshImeWindowVisibility();
        }
    }

    ///M: [ALPS02009053] avoid to show SIM pin view during decryption phase
    public static boolean isSystemEncrypted() {
        boolean bRet = true;
        String cryptoType = SystemProperties.get("ro.crypto.type");
        String state = SystemProperties.get("ro.crypto.state");
        String decrypt = SystemProperties.get("vold.decrypt");
        if ("unsupported".equals(state)) {
            return false;
        }

        if ("unencrypted".equals(state)) {
            if ("".equals(decrypt)) {
                bRet = false;
            }
        } else if ("".equals(state)) {
            // Always do nothing
        } else if ("encrypted".equals(state)) {
            if ("file".equals(cryptoType)) {
                bRet = false;
            } else if ("block".equals(cryptoType)) {
                 if ("trigger_restart_framework".equals(decrypt)) {
                    bRet = false;
                 }
            }
        } else {
            // Unexpected state
        }
        if (DEBUG) {
            Log.d(TAG, "cryptoType=" + cryptoType + " ro.crypto.state=" + state +
                " vold.decrypt=" + decrypt + " sysEncrypted=" + bRet);
        }
        return bRet;
    }

    public static final boolean isPrivacyProtectionLockSupport() {
        return mIsPrivacyProtectionLockSupport ;
    }

    /**
     * Check if voice wakeup service is supported in project.
     *
     * @param context
     *            application context for checking
     *
     * @return result true if voice wakeup service is supported
     */
    public static final boolean isVoiceWakeupSupport(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE) ;
        if (am == null) {
            Log.d(TAG, "isVoiceWakeupSupport() - get AUDIO_SERVICE fails, return false.") ;
            return false ;
        }

        String val = am.getParameters(MTK_VOW_SUPPORT_State) ;
        return val != null && val.equalsIgnoreCase(MTK_VOW_SUPPORT_On) ;
    }

    /**
     * Check if Trigger service is supported in project.
     *
     * @param context
     *            application context for checking
     *
     * @return result true if trigger service is supported
     */
    public static final boolean isVoiceTriggerSupport(Context context) {
        AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE) ;
        if (am == null) {
            Log.d(TAG, "isVoiceTriggerSupport() - get AUDIO_SERVICE fails, return false.") ;
            return false ;
        }

        String val = am.getParameters(MTK_TRIGGER_SUPPORT_State) ;
        return val != null && val.equalsIgnoreCase(MTK_TRIGGER_SUPPORT_On) ;
    }

    public static final boolean isSimMeLockValid(int phoneId) {
        if (mValidPhoneIds == INVALID_PHONE_ID) {
            return true;
        }
        int validMask = mValidPhoneIds & (1 << phoneId);
        Log.d(TAG, "isSimMeLockValid phoneId=" + phoneId + ", result=" + validMask);
        if ((1 << phoneId) == validMask) return true;
        return false;
    }

    /**
     * Avoid to show simme lock twice when framework send two sim cards np state
     * at the same time.
     * @param sim state.
     * @return show or not.
     */
    public static final boolean isShowSimMeLock(boolean reset) {
        sSimMECount++;
        if (reset) {
            sSimMECount = 0;
        }
        if (DEBUG) {
            Log.d(TAG, "isShowSimMeLock sSimMECount=" + sSimMECount);
        }
        if (sSimMECount > 1) {
            return false;
        }
        return true;
    }

    public final void initSimmePolicy(Context context) {
        if (mIsMediatekSimMeLockSupport) {
            return;
        }
        //Check option for simme dependency
        if ("1".equals(
                context.getResources().getString(R.string.simme_dependency_enabled))) {
            Log.d(TAG, "config enabled for simme dependency");
            mIsSimmePolicyEnabled = true;
        } else {
            File simmeOption = new File(Environment.getExternalStorageDirectory(),
                    "SimmeLock");
            mIsSimmePolicyEnabled = simmeOption.exists();
        }

        //Check option for simme dismiss button
        if ("1".equals(
                context.getResources().getString(R.string.dismiss_button_enabled))) {
            Log.d(TAG, "config enabled for simme dismiss");
            mIsDismissEnabled = true;
        } else {
            File dismissOption = new File(Environment.getExternalStorageDirectory(),
                    "SimmeDismiss");
            mIsDismissEnabled = dismissOption.exists();
        }

        //Check for special policy
        int lockPolicy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        Log.d(TAG, "initSimmePolicy lockPolicy=" + lockPolicy);
        mIsMediatekSimMeLockSupport = lockPolicy > 0 ? true : false;
        if (lockPolicy <= 0) return;
        if (mIsSimmePolicyEnabled) {
            mValidPhoneIds = parseValueFromConfig(context, LOCK_REQUEST_NAME, lockPolicy);
            if (mValidPhoneIds > MAX_PHONE_SUPPORTED) {
                mValidPhoneIds = INVALID_PHONE_ID;
            }
            if (parseValueFromConfig(context, LOCK_REQUEST_NAME, lockPolicy) == SIMME_REQUEST_8) {
                mIsUnlockSimMeWithDeviceSupport = true;
            }
        }

        if (mIsDismissEnabled) {
            mIsDismissSimMeLockSupport = true;
            if (parseValueFromConfig(context, DISMISS_REQUEST_NAME, lockPolicy) == 0) {
                mIsDismissSimMeLockSupport = false;
            }
        }
    }
    private final int parseValueFromConfig(Context context, String configName, int lockPolicy) {
        int resourceId = context.getResources().getIdentifier(
                configName + lockPolicy, "string", context.getPackageName());
        if (resourceId == 0) {
            Log.w(TAG, "Cannot get valid source id for " + configName + lockPolicy);
            return -1;
        }
        String configValueString = context.getResources().getString(resourceId);
        Log.d(TAG, "Get config for " +  configName + lockPolicy + ", value=" + configValueString);
        if (configValueString != null && !configValueString.isEmpty()) {
            int configInt;
            try {
                configInt = Integer.parseInt(configValueString);
            } catch (Exception e) {
                Log.e(TAG, "Exception happened: " + e);
                configInt = -1;
            }
            return configInt;
        }
        return -1;
    }
}
