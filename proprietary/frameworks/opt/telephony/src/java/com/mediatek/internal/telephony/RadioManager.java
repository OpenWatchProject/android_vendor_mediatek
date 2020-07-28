
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
import android.os.storage.StorageManager;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

import java.math.BigInteger;
import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map.Entry;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import com.mediatek.internal.telephony.IRadioPower;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkProxyController;
// import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;
import com.mediatek.internal.telephony.selfactivation.SaPersistDataHelper;

/// M: CC: Check ECC state according to Telecom state
import android.telecom.TelecomManager;

import vendor.mediatek.hardware.mtkradioex.V1_0.DsbpState;
import vendor.mediatek.hardware.mtkradioex.V1_0.VendorSetting;

public class RadioManager extends Handler  {

    static final String LOG_TAG = "RadioManager";
    private static final String PREF_CATEGORY_RADIO_STATUS = "RADIO_STATUS";
    private static RadioManager sRadioManager;

    protected static final int MODE_PHONE1_ONLY = 1;
    private static final int MODE_PHONE2_ONLY = 2;
    private static final int MODE_PHONE3_ONLY = 4;
    private static final int MODE_PHONE4_ONLY = 8;

    protected static final int INVALID_PHONE_ID = -1;

    private static final int SIM_NOT_INITIALIZED = -1;
    protected static final int NO_SIM_INSERTED = 0;
    protected static final int SIM_INSERTED = 1;

    private static final boolean ICC_READ_NOT_READY = false;
    private static final boolean ICC_READ_READY = true;

    protected static final int INITIAL_RETRY_INTERVAL_MSEC = 200;

    protected static final boolean RADIO_POWER_OFF = false;
    protected static final boolean RADIO_POWER_ON = true;

    protected static final boolean MODEM_POWER_OFF = false;
    protected static final boolean MODEM_POWER_ON = true;

    protected static final boolean AIRPLANE_MODE_OFF = false;
    protected static final boolean AIRPLANE_MODE_ON = true;
    protected boolean mAirplaneMode = AIRPLANE_MODE_OFF;

    private static final int WIFI_ONLY_INIT = -1;
    private static final boolean WIFI_ONLY_MODE_OFF = false;
    private static final boolean WIFI_ONLY_MODE_ON = true;
    private boolean mWifiOnlyMode = WIFI_ONLY_MODE_OFF;
    private static final String ACTION_WIFI_ONLY_MODE_CHANGED =
        "android.intent.action.ACTION_WIFI_ONLY_MODE";

    protected static final String STRING_NO_SIM_INSERTED = "N/A";

    protected static final String PROPERTY_SILENT_REBOOT_MD1 = "vendor.gsm.ril.eboot";

    private static final String IS_NOT_SILENT_REBOOT = "0";
    protected static final String IS_SILENT_REBOOT = "1";
    private static final String REGISTRANTS_WITH_NO_NAME = "NO_NAME";

    // proprietary intent
    public static final String ACTION_FORCE_SET_RADIO_POWER =
        "com.mediatek.internal.telephony.RadioManager.intent.action.FORCE_SET_RADIO_POWER";
    private static final String ACTION_WIFI_OFFLOAD_SERVICE_ON =
        "mediatek.intent.action.WFC_POWER_ON_MODEM";
    private static final String EXTRA_WIFI_OFFLOAD_SERVICE_ON = "mediatek:POWER_ON_MODEM";

    protected static final String ACTION_AIRPLANE_CHANGE_DONE
                                    = "com.mediatek.intent.action.AIRPLANE_CHANGE_DONE";
    protected static final String EXTRA_AIRPLANE_MODE = "airplaneMode";

    // Notify modules that modem power is not changed
    // even RadioManager want to power off modem initially
    public static final String ACTION_MODEM_POWER_NO_CHANGE
                                    = "com.mediatek.intent.action.MODEM_POWER_CHANGE";
    public static final String EXTRA_MODEM_POWER = "modemPower";

    protected static final int TO_SET_RADIO_POWER = 1;
    protected static final int TO_SET_MODEM_POWER = 2;

    protected int[] mSimInsertedStatus;
    private Context mContext;
    private int[] mInitializeWaitCounter;
    private CommandsInterface[] mCi;
    protected static SharedPreferences sIccidPreference;
    protected int mPhoneCount;
    protected int mBitmapForPhoneCount;
    private int mSimModeSetting;
    private Runnable[] mRadioPowerRunnable;
    private Runnable[] mNotifySimModeChangeRunnable;
    private Runnable mNotifyMSimModeChangeRunnable;
    private Runnable[] mForceSetRadioPowerRunnable;

    private boolean mIsWifiOn = false;
    private boolean mAirDnMsgSent;
    private boolean[] mIsDsbpChanging;
    private boolean mIsPendingRadioByDsbpChanging = false;

    private ImsSwitchController mImsSwitchController = null;

    static protected ConcurrentHashMap<IRadioPower, String> mNotifyRadioPowerChange
            = new ConcurrentHashMap<IRadioPower, String>();

    protected static String[] PROPERTY_ICCID_SIM = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };

    protected static String[] PROPERTY_RADIO_OFF = {
        "vendor.ril.ipo.radiooff",
        "vendor.ril.ipo.radiooff.2",
    };

    /** events id definition */
    private static final int EVENT_RADIO_AVAILABLE_SLOT_1 = 1;
    private static final int EVENT_RADIO_AVAILABLE_SLOT_2 = 2;
    private static final int EVENT_RADIO_AVAILABLE_SLOT_3 = 3;
    private static final int EVENT_RADIO_AVAILABLE_SLOT_4 = 4;
    private static final int EVENT_VIRTUAL_SIM_ON = 5;
    private static final int EVENT_SET_MODEM_POWER_OFF_DONE = 6;
    private static final int EVENT_SET_SILENT_REBOOT_DONE = 7;
    private static final int EVENT_REPORT_AIRPLANE_DONE = 8;
    private static final int EVENT_REPORT_SIM_MODE_DONE = 9;
    private static final int EVENT_DSBP_STATE_CHANGED_SLOT_1 = 10;
    private static final int EVENT_DSBP_STATE_CHANGED_SLOT_2 = 11;
    private static final int EVENT_DSBP_STATE_CHANGED_SLOT_3 = 12;
    private static final int EVENT_DSBP_STATE_CHANGED_SLOT_4 = 13;
    private static final int[] EVENT_RADIO_AVAILABLE = {EVENT_RADIO_AVAILABLE_SLOT_1,
            EVENT_RADIO_AVAILABLE_SLOT_2, EVENT_RADIO_AVAILABLE_SLOT_3,
            EVENT_RADIO_AVAILABLE_SLOT_4};
    private static final int[] EVENT_DSBP_STATE_CHANGED = {EVENT_DSBP_STATE_CHANGED_SLOT_1,
            EVENT_DSBP_STATE_CHANGED_SLOT_2, EVENT_DSBP_STATE_CHANGED_SLOT_3,
            EVENT_DSBP_STATE_CHANGED_SLOT_4};

    /* error cause of setRadioPower */
    public static final int SUCCESS = 0;
    public static final int ERROR_NO_PHONE_INSTANCE = 1;
    public static final int ERROR_AIRPLANE_MODE = 2;
    public static final int ERROR_WIFI_ONLY = 3;
    public static final int ERROR_MODEM_OFF = 4;
    public static final int ERROR_ICCID_NOT_READY = 5;
    public static final int ERROR_PCO = 6;
    public static final int ERROR_PCO_ALREADY_OFF = 7;
    public static final int ERROR_SIM_SWITCH_EXECUTING = 8;
    public static final int ERROR_TURN_OFF_RADIO_DURING_ECC = 9;
    /* reason for setRadioPower */
    public static final int REASON_NONE = -1;
    public static final int REASON_PCO_ON = 0;
    public static final int REASON_PCO_OFF = 1;
    /* reason flag */
    public int[] mReason;
    private static final boolean mFlightModePowerOffModem =
            SystemProperties.get("ro.vendor.mtk_flight_mode_power_off_md").equals("1");

    private static final boolean isOP01
                = "OP01".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));
    private static final boolean isOP09
                = "OP09".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));

    public static RadioManager init(Context context, int phoneCount, CommandsInterface[] ci) {
        synchronized (RadioManager.class) {
            if (sRadioManager == null) {
                sRadioManager =  new RadioManager(context, phoneCount, ci);
            }
            return sRadioManager;
        }
    }

    private PowerSM mPowerSM;
    private boolean mModemPower = true;
    private boolean mNeedIgnoreMessageForChangeDone;
    private boolean mNeedIgnoreMessageForWait;
    private boolean mIsRadioUnavailable = false;
    private boolean mIsWifiOnlyDevice;
    private ModemPowerMessage[] mModemPowerMessages;

    // enhancement of RF
    private static final String PROPERTY_AIRPLANE_MODE =
            "persist.vendor.radio.airplane.mode.on";
    private static final String PROPERTY_SIM_MODE = "persist.vendor.radio.sim.mode";

    /**
     * @internal
     */
    public static RadioManager getInstance() {
        synchronized (RadioManager.class) {
            return sRadioManager;
        }
    }

    protected RadioManager(Context context , int phoneCount, CommandsInterface[] ci) {

        int airplaneMode = Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0);
        int wifionlyMode = ImsManager.getWfcMode(context);

        mAirDnMsgSent = false;

        // WFC modem power control: reference platform WFC capability to sync mIsWifiOn. @{
        ImsManager imsMgr = ImsManager.getInstance(context, 0);
        if (imsMgr.isServiceReady() && ImsManager.isWfcEnabledByPlatform(context) &&
                !StorageManager.inCryptKeeperBounce()) {
            log("initial actual wifi state when wifi calling is on");
            WifiManager wiFiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
            if (wiFiManager != null) {
              mIsWifiOn = (wiFiManager.isWifiEnabled() == true) ? true : false;
            }
        }
        // @}

        log("Initialize RadioManager under airplane mode:" + airplaneMode +
            " wifi only mode:" + wifionlyMode + " wifi mode: " + mIsWifiOn);

        mSimInsertedStatus = new int[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            mSimInsertedStatus[i] = SIM_NOT_INITIALIZED;
        }
        mInitializeWaitCounter = new int[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            mInitializeWaitCounter[i] = 0;
        }
        mRadioPowerRunnable = new RadioPowerRunnable[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            mRadioPowerRunnable[i] = new RadioPowerRunnable(RADIO_POWER_ON, i);
        }
        mNotifySimModeChangeRunnable = new SimModeChangeRunnable[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            mNotifySimModeChangeRunnable[i] = new SimModeChangeRunnable(RADIO_POWER_ON, i);
        }
        mNotifyMSimModeChangeRunnable = new MSimModeChangeRunnable(3);
        mForceSetRadioPowerRunnable = new ForceSetRadioPowerRunnable[phoneCount];

        mContext = context;
        mAirplaneMode = ((airplaneMode == 0) ? AIRPLANE_MODE_OFF : AIRPLANE_MODE_ON);
        mWifiOnlyMode = (ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY == wifionlyMode);

        mCi = ci;
        mPhoneCount = phoneCount;
        mBitmapForPhoneCount = convertPhoneCountIntoBitmap(phoneCount);
        sIccidPreference = mContext.getSharedPreferences(PREF_CATEGORY_RADIO_STATUS, 0);
        mSimModeSetting = Settings.Global.getInt(context.getContentResolver(),
                MtkRILConstants.MSIM_MODE_SETTING, mBitmapForPhoneCount);
        mImsSwitchController = new ImsSwitchController(mContext, mPhoneCount, mCi);

        int mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        ((MtkRIL)mCi[mainPhoneId]).setVendorSetting(
                VendorSetting.VENDOR_SETTING_RADIO_AIRPLANE_MODE,
                Integer.toString(airplaneMode),
                obtainMessage(EVENT_REPORT_AIRPLANE_DONE));

        log("Not BSP Package, register intent!!!");
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);
        filter.addAction(ACTION_FORCE_SET_RADIO_POWER);
        filter.addAction(ACTION_WIFI_ONLY_MODE_CHANGED);
        filter.addAction(ACTION_WIFI_OFFLOAD_SERVICE_ON);
        filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
        mContext.registerReceiver(mIntentReceiver, filter);

        registerListener();

        final ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        mIsWifiOnlyDevice = !cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE);

        mPowerSM = new PowerSM("PowerSM");//PowerSM.init(phoneCount);
        mPowerSM.start();

        mReason = new int[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(i);
            int selfActivationState = SaPersistDataHelper.getIntData(mContext, i,
                    SaPersistDataHelper.DATA_KEY_SA_STATE, ISelfActivation.STATE_NONE);
            String pcoEnable = SystemProperties.get("persist.vendor.pco5.radio.ctrl", "0");
            if (ISelfActivation.STATE_NOT_ACTIVATED == selfActivationState &&
                    !pcoEnable.equals("0")) {
                mReason[i] = REASON_PCO_OFF;
            } else {
                mReason[i] = REASON_NONE;
            }
        }

        mIsDsbpChanging = new boolean[phoneCount];
        for (int i = 0; i < phoneCount; i++) {
            mIsDsbpChanging[i] = false;
        }
    }

    private int convertPhoneCountIntoBitmap(int phoneCount) {
        int ret = 0;
        for (int i = 0; i < phoneCount; i++) {
            ret += MODE_PHONE1_ONLY << i;
        }
        log("Convert phoneCount " + phoneCount + " into bitmap " + ret);
        return ret;
    }

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

           log("BroadcastReceiver: " + intent.getAction());

            if (intent.getAction().equals(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED)) {
                onReceiveSimStateChangedIntent(intent);
            } else if (intent.getAction().equals(ACTION_FORCE_SET_RADIO_POWER)) {
                onReceiveForceSetRadioPowerIntent(intent);
            } else if (intent.getAction().equals(ACTION_WIFI_ONLY_MODE_CHANGED)) {
                onReceiveWifiOnlyModeStateChangedIntent(intent);
            } else if (intent.getAction().equals(ACTION_WIFI_OFFLOAD_SERVICE_ON)) {
                onReceiveWifiStateChangedIntent(intent);
            } else if (intent.getAction().equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)
                    || intent.getAction().equals(
                    TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED)) {
                if (isFlightModePowerOffModemConfigEnabled()) {
                    mPowerSM.updateModemPowerState(mAirplaneMode, mBitmapForPhoneCount,
                            ModemPowerCasue.CAUSE_SIM_SWITCH);
                }
                if (mIsDsbpChanging[findMainCapabilityPhoneId()]) {
                    mIsPendingRadioByDsbpChanging = true;
                } else {
                    setRadioPowerAfterCapabilitySwitch();
                }
            }
        }
    };

    private void setRadioPowerAfterCapabilitySwitch() {
        log("Update radio power after capability switch or dsbp changing");
        // turn on data SIM first. If data SIM is invalid, turn on ps1 first
        int targetPhoneId = SubscriptionManager.from(mContext).getDefaultDataPhoneId();
        if (!SubscriptionManager.isValidPhoneId(targetPhoneId)) {
            targetPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        }
        setRadioPower(!mAirplaneMode, targetPhoneId);
        for (int i = 0; i < mPhoneCount; i++) {
            if (targetPhoneId != i) {
                // scenario: Enter/leave airplane mode under SIM switch,
                // setRadioPower would be blocked by SIM switch and
                // not update mDesiredPowerState. When SIM switch done,
                // RM should set radio power according to airplane mode state
                setRadioPower(!mAirplaneMode, i);
            }
        }
    }

    protected void onReceiveWifiStateChangedIntent(Intent intent) {
        int extraWifiState = WifiManager.WIFI_STATE_DISABLED;
        if (intent.getAction().equals(ACTION_WIFI_OFFLOAD_SERVICE_ON)) {
            extraWifiState = intent.getBooleanExtra(EXTRA_WIFI_OFFLOAD_SERVICE_ON, false)
                    ? WifiManager.WIFI_STATE_ENABLED : WifiManager.WIFI_STATE_DISABLED;
        } else {
            log("Wrong intent");
            return;
        }

        log("Receiving ACTION_WIFI_OFFLOAD_SERVICE_ON, "
                + "airplaneMode: " + mAirplaneMode + " isFlightModePowerOffModemConfigEnabled:"
                + isFlightModePowerOffModemConfigEnabled()
                + ", mIsWifiOn: " + mIsWifiOn);

        boolean modemPower;

        switch(extraWifiState) {
            case WifiManager.WIFI_STATE_ENABLED:
                log("WIFI_STATE_CHANGED enabled");
                modemPower = MODEM_POWER_ON;
                mIsWifiOn = true;
                if (mAirplaneMode == AIRPLANE_MODE_ON &&
                        isFlightModePowerOffModemConfigEnabled()) {

                    // current Modem ON, target Modem ON => conflict, skip
                    boolean currModemPower = true;
                    if (isModemPowerOff(0)) {  // phoneId is not used for non GSM+GSM DSDA project
                        currModemPower = false;
                    }

                    log("WIFI_STATE_CHANGED enabled, set modem on");
                    setSilentRebootPropertyForAllModem(IS_SILENT_REBOOT);
                    mPowerSM.updateModemPowerState(modemPower, mBitmapForPhoneCount,
                            ModemPowerCasue.CAUSE_WIFI_CALLING);
                }
                break;
            case WifiManager.WIFI_STATE_DISABLED:
                log("WIFI_STATE_CHANGED disabled");
                modemPower = MODEM_POWER_OFF;
                mIsWifiOn = false;
                if (mAirplaneMode == AIRPLANE_MODE_ON &&
                        isFlightModePowerOffModemConfigEnabled()) {
                    log("WIFI_STATE_CHANGED disabled, set modem off");
                    setSilentRebootPropertyForAllModem(IS_SILENT_REBOOT);
                    mPowerSM.updateModemPowerState(modemPower, mBitmapForPhoneCount,
                            ModemPowerCasue.CAUSE_WIFI_CALLING);
                }
                break;
            default:
                log("default: WIFI_STATE_CHANGED extra" + extraWifiState);
                break;
         }
    }

    protected void onReceiveSimStateChangedIntent(Intent intent) {
        int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                TelephonyManager.SIM_STATE_UNKNOWN);

        // TODO: phone_key now is equals to slot_key, change in the future
        int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, INVALID_PHONE_ID);

        if (!isValidPhoneId(phoneId)) {
            log("INTENT:Invalid phone id:" + phoneId + ", do nothing!");
            return;
        }

        log("INTENT:SIM_STATE_CHANGED: " + intent.getAction() + ", sim status: " + simStatus +
                ", phoneId: " + phoneId);

        boolean desiredRadioPower = RADIO_POWER_ON;

        if (TelephonyManager.SIM_STATE_PRESENT == simStatus) {
            mSimInsertedStatus[phoneId] = SIM_INSERTED;
            log("Phone[" + phoneId + "]: " + simStatusToString(SIM_INSERTED));

            // if we receive ready, but can't get iccid, we do nothing
            String iccid = readIccIdUsingPhoneId(phoneId);
            if (STRING_NO_SIM_INSERTED.equals(iccid)) {
                log("Phone " + phoneId + ":SIM ready but ICCID not ready, do nothing");
                return;
            }

            desiredRadioPower = RADIO_POWER_ON;
            if (mAirplaneMode == AIRPLANE_MODE_OFF) {
                log("Set Radio Power due to SIM_STATE_CHANGED, power: " + desiredRadioPower +
                        ", phoneId: " + phoneId);
                setRadioPower(desiredRadioPower, phoneId);
            }
        }

        else if (TelephonyManager.SIM_STATE_ABSENT == simStatus) {
            mSimInsertedStatus[phoneId] = NO_SIM_INSERTED;
            log("Phone[" + phoneId + "]: " + simStatusToString(NO_SIM_INSERTED));
            // TODO: read property to decide turn off power or not
            desiredRadioPower = RADIO_POWER_OFF;
            if (mAirplaneMode == AIRPLANE_MODE_OFF) {
                log("Set Radio Power due to SIM_STATE_CHANGED, power: " + desiredRadioPower +
                        ", phoneId: " + phoneId);
                setRadioPower(desiredRadioPower, phoneId);
            }
        }
    }

   /**
     * enter or leave wifi only mode
     *
     */
    public void onReceiveWifiOnlyModeStateChangedIntent(Intent intent) {

        boolean enabled = intent.getBooleanExtra("state", false);
        log("Received ACTION_WIFI_ONLY_MODE_CHANGED, enabled = " + enabled);

        // we expect wifi only mode is on-> off or off->on
        if (enabled == mWifiOnlyMode) {
            log("enabled = " + enabled + ", mWifiOnlyMode = "+ mWifiOnlyMode +
                    " is not expected (the same)");
            return;
        }

        mWifiOnlyMode = enabled;
        if (mAirplaneMode == AIRPLANE_MODE_OFF) {
            boolean radioPower = enabled ? RADIO_POWER_OFF : RADIO_POWER_ON;
            for (int i = 0; i < mPhoneCount; i++) {
                setRadioPower(radioPower, i);
            }
        }
    }

    private void onReceiveForceSetRadioPowerIntent(Intent intent) {
        int phoneId = 0;
        int mode = intent.getIntExtra(TelephonyIntents.EXTRA_MSIM_MODE, -1);
        log("force set radio power, mode: " + mode);
        if (mode == -1) {
            log("Invalid mode, MSIM_MODE intent has no extra value");
            return;
        }
        for (phoneId = 0; phoneId < mPhoneCount; phoneId++) {
            boolean singlePhonePower =
                ((mode & (MODE_PHONE1_ONLY << phoneId)) == 0) ? RADIO_POWER_OFF : RADIO_POWER_ON;
            if (RADIO_POWER_ON == singlePhonePower) {
                forceSetRadioPower(true, phoneId);
            }
        }
    }

    protected boolean isValidPhoneId(int phoneId) {
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            return false;
        } else {
            return true;
        }
    }

    protected String simStatusToString(int simStatus) {
        String result = null;
        switch(simStatus) {
            case SIM_NOT_INITIALIZED:
                result = "SIM HAVE NOT INITIALIZED";
                break;
            case SIM_INSERTED:
                result = "SIM DETECTED";
                break;
            case NO_SIM_INSERTED:
                result = "NO SIM DETECTED";
                break;
            }
        return result;
    }

    /**
     * Modify mAirplaneMode and set modem power
     * @param enabled 0: normal mode
     *                1: airplane mode
     * @internal
     */
    public void notifyAirplaneModeChange(boolean enabled) {
        // we expect airplane mode is on-> off or off->on
        if (enabled == mAirplaneMode) {
            log("enabled = " + enabled + ", mAirplaneMode = " + mAirplaneMode +
                    " is not expected (the same)");
            return;
        }
        int phoneId = findMainCapabilityPhoneId();
        mAirplaneMode = enabled;
        log("Airplane mode changed: " + enabled +
                " mDesiredPower: " + mPowerSM.mDesiredModemPower +
                " mCurrentModemPower: " + mPowerSM.mCurrentModemPower);
        ((MtkRIL)mCi[phoneId]).setVendorSetting(
                VendorSetting.VENDOR_SETTING_RADIO_AIRPLANE_MODE,
                Integer.toString(enabled ? 1 : 0),
                obtainMessage(EVENT_REPORT_AIRPLANE_DONE));

        // Avoid timing issue: reset mIsWifiOn to false if airplane mode on
        if (enabled) {
            mIsWifiOn = false;
        }

        int radioAction = -1;  // 1:setRadioPower, 2:setModemPower

        if (isFlightModePowerOffModemConfigEnabled() && !isUnderCryptKeeper()) {
            if (mPowerSM.mDesiredModemPower && AIRPLANE_MODE_OFF == mAirplaneMode) {
                // current Modem ON, target Airplane mode OFF => conflict, set Radio Power
                log("Airplane mode changed: turn on all radio due to mode conflict");
                radioAction = TO_SET_RADIO_POWER;
            } else if (AIRPLANE_MODE_OFF == mAirplaneMode && mIsWifiOn) {
                // airplane mode on and wifi-calling on. Then, leave airplane mode
                log("airplane mode changed: airplane mode on and wifi-calling on. Then," +
                        "leave airplane mode: turn on/off all radio");
                radioAction = TO_SET_RADIO_POWER;
            } else {
                log("Airplane mode changed: turn on/off all modem");
                radioAction = TO_SET_MODEM_POWER;
            }
        } else if (isMSimModeSupport()) {
            log("Airplane mode changed: turn on/off all radio");
            radioAction = TO_SET_RADIO_POWER;
        }

        if (radioAction == TO_SET_RADIO_POWER) {
            boolean radioPower = enabled ? RADIO_POWER_OFF : RADIO_POWER_ON;
            // turn on data SIM first
            int targetPhoneId = SubscriptionManager.from(mContext).getDefaultDataPhoneId();
            if (!SubscriptionManager.isValidPhoneId(targetPhoneId)) {
                targetPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
            }
            setRadioPower(radioPower, targetPhoneId);
            for (int i = 0; i < mPhoneCount; i++) {
                if (targetPhoneId != i) {
                    setRadioPower(radioPower, i);
                }
            }

            Intent intent = new Intent(RadioManager.ACTION_AIRPLANE_CHANGE_DONE);
            intent.putExtra(RadioManager.EXTRA_AIRPLANE_MODE, !enabled);
            mContext.sendBroadcastAsUser(intent, UserHandle.ALL);

        } else if (radioAction == TO_SET_MODEM_POWER) {
            boolean modemPower = enabled ? MODEM_POWER_OFF : MODEM_POWER_ON;
            setSilentRebootPropertyForAllModem(IS_SILENT_REBOOT);
            mPowerSM.updateModemPowerState(modemPower, mBitmapForPhoneCount,
                    ModemPowerCasue.CAUSE_AIRPLANE_MODE);
        }
    }

    /*
     *A special paragraph, not to trun off modem power under cryptkeeper
     */
    public static boolean isUnderCryptKeeper() {
        /*
        /* android N provides new encryption  mechanism "file base encryption"
        /* FBS would not enctypt NV-Ram of modem. So, RadioManager can turn on/off
        /* under FBE
        /* ro.crypto.type == block => FDE
        /* ro.crypto.type == file => FBE
        */
        if (SystemProperties.get("ro.crypto.type").equals("block")
            && SystemProperties.get("ro.crypto.state").equals("encrypted")
            && SystemProperties.get("vold.decrypt").equals("trigger_restart_min_framework")) {
            log("[Special Case] Under CryptKeeper, Not to turn on/off modem");
            return true;
        }
        log("[Special Case] Not Under CryptKeeper");
        return false;
    }

    public void setSilentRebootPropertyForAllModem(String isSilentReboot) {
        TelephonyManager.MultiSimVariants config =
                TelephonyManager.getDefault().getMultiSimConfiguration();
        int phoneId = findMainCapabilityPhoneId();
        int on = 0;
        if (isSilentReboot.equals("1")) {
            on = 1;
        }

        log("enable silent reboot");
        ((MtkRIL)mCi[phoneId]).setVendorSetting(VendorSetting.VENDOR_SETTING_RADIO_SILENT_REBOOT,
                Integer.toString(on),
                obtainMessage(EVENT_SET_SILENT_REBOOT_DONE));
    }

    /*
     * Called From GSMSST, if boot up under airplane mode, power-off modem
     */
    public void notifyRadioAvailable(int phoneId) {
        log("Phone " + phoneId + " notifies radio available" +
                " airplane mode: " + mAirplaneMode + " cryptkeeper: "
                + isUnderCryptKeeper() + " mIsWifiOn:" + mIsWifiOn);
        if (isRadioAvaliable()) {
            mPowerSM.sendEvent(PowerEvent.EVENT_RADIO_AVAILABLE);
        }
        if (RadioCapabilitySwitchUtil.getMainCapabilityPhoneId() == phoneId) {
            // Avoid message unsync at the following case:
            // Enter/leave flight mode continually and trigger TRM
            cleanModemPowerMessage();
            if (mAirplaneMode == AIRPLANE_MODE_ON && isFlightModePowerOffModemConfigEnabled()
                    && !isUnderCryptKeeper() && !mIsWifiOn) {

                log("Power off modem because boot up under airplane mode");
                mPowerSM.updateModemPowerState(MODEM_POWER_OFF, MODE_PHONE1_ONLY << phoneId,
                        ModemPowerCasue.CAUSE_RADIO_AVAILABLE);
            }
        }

        /* To resolve the alarm bootup(enter/quit airplane mode) blocking issue.
           Send out ACTION_AIRPLANE_CHANGE_DONE if bootup under airplane mode
           (for project which flightmode do not turn off md)
        */
        if (!mAirDnMsgSent && mAirplaneMode == AIRPLANE_MODE_ON &&
                !(isFlightModePowerOffModemConfigEnabled() && !isUnderCryptKeeper())) {
            Intent intent = new Intent(RadioManager.ACTION_AIRPLANE_CHANGE_DONE);
            intent.putExtra(RadioManager.EXTRA_AIRPLANE_MODE, true);
            mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
            mAirDnMsgSent = true;
        }
    }

    /**
     * Set modem power on/off according to DSDS or DSDA.
     *
     * @param power desired power of modem
     * @param phoneId a bit map of phones you want to set
     *              1: phone 1 only
     *              2: phone 2 only
     *              3: phone 1 and phone 2
     */
    private void setModemPower(boolean power, int phoneBitMap) {
        log("Set Modem Power according to bitmap, Power:" + power + ", PhoneBitMap:" + phoneBitMap);
        if (PhoneFactory.getDefaultPhone().getServiceStateTracker().isDeviceShuttingDown()) {
            Rlog.d(LOG_TAG, "[RadioManager] skip the request because device is shutdown");
            return;
        }
        TelephonyManager.MultiSimVariants config =
                TelephonyManager.getDefault().getMultiSimConfiguration();

        final Message[] responses;
        ///M: Add for the airplane mode frequently switch issue.@{
        responses = monitorModemPowerChangeDone(power, phoneBitMap, findMainCapabilityPhoneId());
        /// @}

        int phoneId = 0;
        switch(config) {
            case DSDS:
            case DSDA:
            case TSTS:
                phoneId = findMainCapabilityPhoneId();
                log("Set Modem Power, Power:" + power + ", phoneId:" + phoneId);
                ((MtkRIL)mCi[phoneId]).setModemPower(power, responses[phoneId]);
                if (power == MODEM_POWER_OFF) {
                    for (int i = 0; i < mPhoneCount; i++) {
                        resetSimInsertedStatus(i);
                    }
                }
                break;

            default:
                phoneId = PhoneFactory.getDefaultPhone().getPhoneId();
                log("Set Modem Power under SS mode:" + power + ", phoneId:" + phoneId);
                ((MtkRIL)mCi[phoneId]).setModemPower(power, responses[phoneId]);
                break;
        }

        if (power == MODEM_POWER_ON && (isOP01 || isOP09)
                && SystemProperties.get("vendor.ril.atci.flightmode").equals("1")) {
            log("Power on Modem, Set vendor.ril.atci.flightmode to 0");
            SystemProperties.set("vendor.ril.atci.flightmode", "0");
        }
    }

    protected int findMainCapabilityPhoneId() {
        int result = 0;
        int switchStatus = Integer.valueOf(
                SystemProperties.get(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, "1"));
        result = switchStatus - 1;
        if (result < 0 || result >= mPhoneCount) {
            return 0;
        } else {
            return result;
        }
    }

    /**
     * Radio power Runnable for power on/off retry.
     */
    protected class RadioPowerRunnable implements Runnable {
        boolean retryPower;
        int retryPhoneId;
        public  RadioPowerRunnable(boolean power, int phoneId) {
            retryPower = power;
            retryPhoneId = phoneId;
        }
        @Override
        public void run() {
            setRadioPower(retryPower, retryPhoneId);
        }
    }

    /*
     * MTK flow to control radio power
     */
    public int setRadioPower(boolean power, int phoneId) {
        log("setRadioPower, power=" + power + "  phoneId=" + phoneId);
        Phone phone;
        phone = PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            return ERROR_NO_PHONE_INSTANCE;
        }

        if ((isFlightModePowerOffModemEnabled() || power) && mAirplaneMode == AIRPLANE_MODE_ON) {
            log("Set Radio Power on under airplane mode, ignore");
            return ERROR_AIRPLANE_MODE;
        }

        ///M:There is no modem on wifi-only device @{
        ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(
                Context.CONNECTIVITY_SERVICE);
        if (cm.isNetworkSupported(ConnectivityManager.TYPE_MOBILE) == false) {
            log("wifi-only device, so return");
            return ERROR_WIFI_ONLY;
        }
        ///  @}

        if (((MtkProxyController)MtkProxyController.getInstance()).isCapabilitySwitching()) {
            log("SIM switch executing, return and wait SIM switch done");
            return ERROR_SIM_SWITCH_EXECUTING;
        }

        if (isModemPowerOff(phoneId)) {
            log("modem for phone " + phoneId + " off, do not set radio again");
            return ERROR_MODEM_OFF;
        }

        /* CMDA-Less case */
        String pcoEnable = SystemProperties.get("persist.vendor.pco5.radio.ctrl", "0");
        if (REASON_PCO_OFF == mReason[phoneId] && power && !pcoEnable.equals("0")) {
            log("Not allow to turn on radio under PCO=5");
            return ERROR_PCO;
        }

        /* For  */
        if (REASON_PCO_OFF == mReason[phoneId] &&
                TelephonyManager.RADIO_POWER_OFF == phone.mCi.getRadioState()) {
            log("PCO5 and already off");
            return ERROR_PCO_ALREADY_OFF;
        }

        /// M: CC: Check ECC state according to Telecom state
        boolean isInEcc = false;
        TelecomManager tm = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
        if (tm != null && tm.isInEmergencyCall()) {
            isInEcc = true;
        }

        // Don't allow opetate radio off during ECC to avoid ATD after EFUN
        if (!power && isInEcc) {
            if (mAirplaneMode) {
                // Switch airplane mode back to off.
                ConnectivityManager.from(mContext).setAirplaneMode(false);
                Intent intent = new Intent(ACTION_AIRPLANE_CHANGE_DONE);
                intent.putExtra(EXTRA_AIRPLANE_MODE, false);
                mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
            }
            log("Not allow to operate radio power during emergency call");
            return ERROR_AIRPLANE_MODE;
        }

        /**
        * We want iccid ready berfore we check if SIM is once manually turned-offedd
        * So we check ICCID repeatedly every 300 ms
        */
        removeCallbacks(mRadioPowerRunnable[phoneId]);
        if (!isIccIdReady(phoneId)) {
            if (hasCallbacks(mForceSetRadioPowerRunnable[phoneId])) {
                log("ForceSetRadioPowerRunnable exists queue, do not execute RadioPowerRunnable" +
                        "for phone " + phoneId);
            } else {
                log("RILD initialize not completed, wait for " + INITIAL_RETRY_INTERVAL_MSEC + "ms");
                mRadioPowerRunnable[phoneId] = new RadioPowerRunnable(power, phoneId);
                postDelayed(mRadioPowerRunnable[phoneId], INITIAL_RETRY_INTERVAL_MSEC);
            }
            return ERROR_ICCID_NOT_READY;
        }

        setSimInsertedStatus(phoneId);

        boolean radioPower = power;
        String iccId = readIccIdUsingPhoneId(phoneId);
        //adjust radio power according to ICCID
        if (isRequiredRadioOff(iccId)) {
            if (isInEcc) {
                log("Adjust radio to off because once manually turned off during ECC, return");
                return ERROR_TURN_OFF_RADIO_DURING_ECC;
            }
            String printableIccid;
            if ("N/A".equals(iccId)) {
                printableIccid = "N/A";
            } else {
                printableIccid = binaryToHex(getHashCode(
                        SubscriptionInfo.givePrintableIccid(iccId)));
            }
            log("Adjust radio to off because once manually turned off, hash(iccid): " +
                    printableIccid + " , phone: " + phoneId);
            radioPower = RADIO_POWER_OFF;
        } else if (!mAirplaneMode && !phone.isShuttingDown() &&
                !(REASON_PCO_OFF == mReason[phoneId])) {
            radioPower = RADIO_POWER_ON;
        }

        if (mWifiOnlyMode == WIFI_ONLY_MODE_ON && isInEcc == false) {
            log("setradiopower but wifi only, turn off");
            radioPower = RADIO_POWER_OFF;
        }

        boolean isCTACase = checkForCTACase();

        if (getSimInsertedStatus(phoneId) == NO_SIM_INSERTED) {
            if (isCTACase == true) {
                int capabilityPhoneId = findMainCapabilityPhoneId();
                log("No SIM inserted, force to turn on 3G/4G phone " +
                    capabilityPhoneId + " radio if no any sim radio is enabled!");
                PhoneFactory.getPhone(capabilityPhoneId).setRadioPower(RADIO_POWER_ON);
                for (int i = 0; i < mPhoneCount; i++) {
                    phone = PhoneFactory.getPhone(i);
                    if (phone == null) {
                        continue;
                    }
                    if (i != capabilityPhoneId && isInEcc != true) {
                        phone.setRadioPower(RADIO_POWER_OFF);
                    }
                }

            } else if (true == isInEcc && mAirplaneMode == AIRPLANE_MODE_OFF) {
                log("No SIM inserted, turn/keep Radio On for ECC! target power: "
                        + radioPower + ", phoneId: " + phoneId);
                if (radioPower == RADIO_POWER_ON) {
                    PhoneFactory.getPhone(phoneId).setRadioPower(radioPower);
                }
            } else {
                log("No SIM inserted, turn Radio off!");
                radioPower = RADIO_POWER_OFF;
                PhoneFactory.getPhone(phoneId).setRadioPower(radioPower);
            }
        } else {
            log("Trigger set Radio Power, power: " + radioPower + ", phoneId: " + phoneId);
            PhoneFactory.getPhone(phoneId).setRadioPower(radioPower);
        }
        refreshSimSetting(radioPower, phoneId);
        return SUCCESS;
    }

    /* @para power: indicates desired power state (on/off)
        * @para phoneId: target phone id
        * @para reason: to record the reason why to terigger setRadioPower
        * @return success or failure reason
        */
    public int setRadioPower(boolean power, int phoneId, int reason) {
        mReason[phoneId] = reason;
        return setRadioPower(power, phoneId);
    }

    protected int getSimInsertedStatus(int phoneId) {
        return mSimInsertedStatus[phoneId];
    }

    protected void setSimInsertedStatus(int phoneId) {
        String iccId = readIccIdUsingPhoneId(phoneId);
        if (STRING_NO_SIM_INSERTED.equals(iccId)) {
            mSimInsertedStatus[phoneId] = NO_SIM_INSERTED;
        } else {
            mSimInsertedStatus[phoneId] = SIM_INSERTED;
        }
    }

    protected boolean isIccIdReady(int phoneId) {
        String iccId = readIccIdUsingPhoneId(phoneId);
        boolean ret = ICC_READ_NOT_READY;
        if (iccId == null || "".equals(iccId)) {
            ret = ICC_READ_NOT_READY;
        } else {
            //log("ICC read ready, iccid[" + phoneId + "]: " + iccId);
            ret = ICC_READ_READY;
        }
        return ret;
    }

    protected String readIccIdUsingPhoneId(int phoneId) {
        String ret = SystemProperties.get(PROPERTY_ICCID_SIM[phoneId]);
        String printableIccid;
        if ("N/A".equals(ret)) {
            printableIccid = "N/A";
        } else {
            printableIccid = binaryToHex(getHashCode(SubscriptionInfo.givePrintableIccid(ret)));
        }
        log("Hash(ICCID) for phone " + phoneId + " is " +  printableIccid);
        return ret;
    }

    protected boolean checkForCTACase() {
        boolean isCTACase = true;
        //log("Check For CTA case!");
        if (mAirplaneMode == AIRPLANE_MODE_OFF && mWifiOnlyMode != WIFI_ONLY_MODE_ON){
            for (int i = 0; i < mPhoneCount; i++) {
                log("Check For CTA case: mSimInsertedStatus[" + i + "]:"  + mSimInsertedStatus[i]);
                if (mSimInsertedStatus[i] == SIM_INSERTED ||
                        mSimInsertedStatus[i] == SIM_NOT_INITIALIZED) {
                    isCTACase = false;
                }
            }
        } else {
            isCTACase = false;
        }

        /// M: CC: Check ECC state according to Telecom state
        boolean isInEcc = false;
        TelecomManager tm = (TelecomManager) mContext.getSystemService(Context.TELECOM_SERVICE);
        if (tm != null && tm.isInEmergencyCall()) {
            isInEcc = true;
        }

        if ((false == isCTACase) && (false == isInEcc)) {
            turnOffCTARadioIfNecessary();
        }
        log("CTA case: " + isCTACase);
        return isCTACase;
    }

    /*
     * We need to turn off Phone's radio if no SIM inserted (radio on because CTA) after we leave
     * CTA case
     */
    private void turnOffCTARadioIfNecessary() {
        Phone phone;
        for (int i = 0; i < mPhoneCount; i++) {
            phone = PhoneFactory.getPhone(i);
            if (phone == null) {
                continue;
            }
            if (mSimInsertedStatus[i] == NO_SIM_INSERTED) {
                if (isModemPowerOff(i)) {
                    log("modem off, not to handle CTA");
                    return;
                } else {
                    log("turn off phone " + i + " radio because we are no longer in CTA mode");
                    phone.setRadioPower(RADIO_POWER_OFF);
                }
            }
        }
    }

    /*
     * Refresh MSIM Settings only when:
     * We auto turn off a SIM card once manually turned off
     */
    protected void refreshSimSetting(boolean radioPower, int phoneId) {
        if (PhoneFactory.getDefaultPhone().getServiceStateTracker().isDeviceShuttingDown()) {
            Rlog.i(LOG_TAG,
                    "[RadioManager] skip the refreshSimSetting because device is shutdown");
            return;
        }

        int oldMode = mSimModeSetting;

        if (radioPower == RADIO_POWER_OFF) {
            mSimModeSetting &= ~(MODE_PHONE1_ONLY << phoneId);
        } else {
            mSimModeSetting |= (MODE_PHONE1_ONLY << phoneId);
        }

        log("Refresh MSIM mode setting to " + mSimModeSetting + " from " + oldMode);
        ((MtkRIL)mCi[findMainCapabilityPhoneId()]).setVendorSetting(
                VendorSetting.VENDOR_SETTING_RADIO_SIM_MODE,
                Integer.toString(mSimModeSetting),
                obtainMessage(EVENT_REPORT_SIM_MODE_DONE));
        Settings.Global.putInt(mContext.getContentResolver(),
                MtkRILConstants.MSIM_MODE_SETTING, mSimModeSetting);
    }

    /**
     * Wait ICCID ready when force set radio power.
     */
    protected class ForceSetRadioPowerRunnable implements Runnable {
        boolean mRetryPower;
        int mRetryPhoneId;
        public  ForceSetRadioPowerRunnable(boolean power, int phoneId) {
            mRetryPower = power;
            mRetryPhoneId = phoneId;
        }
        @Override
        public void run() {
            forceSetRadioPower(mRetryPower, mRetryPhoneId);
        }
    }

    /**
     * force turn on radio and remove iccid for preference to prevent being turned off again
     * 1. For ECC call
     */
    public void forceSetRadioPower(boolean power, int phoneId) {
        log("force set radio power for phone" + phoneId + " ,power: " + power);
        Phone phone;
        phone = PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            return;
        }

        if (isFlightModePowerOffModemConfigEnabled() && mAirplaneMode == AIRPLANE_MODE_ON) {
            log("Force Set Radio Power under airplane mode, ignore");
            return;
        }

        if (isModemPowerOff(phoneId) && mAirplaneMode == AIRPLANE_MODE_ON) {
            // refreshIccIdPreference(power, readIccIdUsingPhoneId(phoneId));
            log("Modem Power Off for phone " + phoneId + ", Power on modem first");
            mPowerSM.updateModemPowerState(MODEM_POWER_ON, MODE_PHONE1_ONLY << phoneId,
                    ModemPowerCasue.CAUSE_ECC);
            // return;
        }

        /**
        * We want iccid ready berfore we check if SIM is once manually turned-offedd
        * So we check ICCID repeatedly every 300 ms
        */
        removeCallbacks(mForceSetRadioPowerRunnable[phoneId]);
        if (!isIccIdReady(phoneId) || (isFlightModePowerOffModemConfigEnabled() &&
                mAirplaneMode == AIRPLANE_MODE_OFF && power == true && isModemOff(phoneId))) {
            log("force set radio power, read iccid not ready, wait for" +
                INITIAL_RETRY_INTERVAL_MSEC + "ms");
            mForceSetRadioPowerRunnable[phoneId] = new ForceSetRadioPowerRunnable(power, phoneId);
            postDelayed(mForceSetRadioPowerRunnable[phoneId], INITIAL_RETRY_INTERVAL_MSEC);
            return;
        }

        boolean radioPower = power;
        refreshIccIdPreference(radioPower, readIccIdUsingPhoneId(phoneId));
        phone.setRadioPower(power);
        refreshSimSetting(radioPower, phoneId);
    }

    /*
     * wait ICCID ready when SIM mode change
     */
    private class SimModeChangeRunnable implements Runnable {
        boolean mPower;
        int mPhoneId;
        public SimModeChangeRunnable(boolean power, int phoneId) {
            mPower = power;
            mPhoneId = phoneId;
        }
        @Override
        public void run() {
            notifySimModeChange(mPower, mPhoneId);
        }
    }

    /**
     * Refresh ICCID preference due to toggling on SIM management except for below cases:
     * 1. SIM Mode Feature not defined
     * 2. Under Airplane Mode (PhoneGlobals will call GSMPhone.setRadioPower after receving
     * airplane mode change)
     * @param power power on -> remove preference
     *               power off -> add to preference
     */
    public void notifySimModeChange(boolean power, int phoneId) {
        log("SIM mode changed, power: " + power + ", phoneId" + phoneId);
        if (!isMSimModeSupport() || mAirplaneMode == AIRPLANE_MODE_ON) {
            log("Airplane mode on or MSIM Mode option is closed, do nothing!");
            return;
        } else {
            removeCallbacks(mNotifySimModeChangeRunnable[phoneId]);
            if (!isIccIdReady(phoneId)) {
                log("sim mode read iccid not ready, wait for "
                    + INITIAL_RETRY_INTERVAL_MSEC + "ms");
                mNotifySimModeChangeRunnable[phoneId]
                    = new SimModeChangeRunnable(power, phoneId);
                postDelayed(mNotifySimModeChangeRunnable[phoneId], INITIAL_RETRY_INTERVAL_MSEC);
                return;
            }
            //once ICCIDs are ready, then set the radio power
            if (STRING_NO_SIM_INSERTED.equals(readIccIdUsingPhoneId(phoneId))) {
                power = RADIO_POWER_OFF;
                log("phoneId " + phoneId + " sim not insert, set  power  to " + power);
            }
            refreshIccIdPreference(power, readIccIdUsingPhoneId(phoneId));
            log("Set Radio Power due to SIM mode change, power: " + power + ", phoneId: "
                    + phoneId);
            setRadioPower(power, phoneId);
        }
    }

    /**
     * Wait ICCID ready when MSIM modem change.
     * @Deprecated
     */
    protected class MSimModeChangeRunnable implements Runnable {
        int mRetryMode;
        public  MSimModeChangeRunnable(int mode) {
            mRetryMode = mode;
        }
        @Override
        public void run() {
            notifyMSimModeChange(mRetryMode);
        }
    }

    /**
     * Refresh ICCID preference due to toggling on SIM management except for below cases:
     * 1. SIM Mode Feature not defined
     * 2. Under Airplane Mode (PhoneGlobals will call GSMPhone.setRadioPower after receving
     * airplane mode change)
     * @param power power on -> remove preference
     *               power off -> add to preference
     * @internal
     * @Deprecated
     */
    public void notifyMSimModeChange(int mode) {
        log("MSIM mode changed, mode: " + mode);
        if (mode == -1) {
            log("Invalid mode, MSIM_MODE intent has no extra value");
            return;
        }
        if (!isMSimModeSupport() || mAirplaneMode == AIRPLANE_MODE_ON) {
            log("Airplane mode on or MSIM Mode option is closed, do nothing!");
            return;
        } else {
            //all ICCCIDs need be ready berfore set radio power
            int phoneId = 0;
            boolean iccIdReady = true;
            for (phoneId = 0; phoneId < mPhoneCount; phoneId++) {
                if (!isIccIdReady(phoneId)) {
                    iccIdReady = false;
                    break;
                }
            }

            removeCallbacks(mNotifyMSimModeChangeRunnable);
            if (!iccIdReady) {
                /*log("msim mode read iccid not ready, wait for "
                    + INITIAL_RETRY_INTERVAL_MSEC + "ms");*/
                mNotifyMSimModeChangeRunnable
                    = new MSimModeChangeRunnable(mode);
                postDelayed(mNotifyMSimModeChangeRunnable, INITIAL_RETRY_INTERVAL_MSEC);
                return;
            }
            //once ICCIDs are ready, then set the radio power
            for (phoneId = 0; phoneId < mPhoneCount; phoneId++) {
                boolean singlePhonePower = ((mode & (MODE_PHONE1_ONLY << phoneId)) == 0) ?
                        RADIO_POWER_OFF : RADIO_POWER_ON;
                if (STRING_NO_SIM_INSERTED.equals(readIccIdUsingPhoneId(phoneId))) {
                    singlePhonePower = RADIO_POWER_OFF;
                    log("phoneId " + phoneId + " sim not insert, set  power  to "
                            + singlePhonePower);
                }
                refreshIccIdPreference(singlePhonePower, readIccIdUsingPhoneId(phoneId));
                log("Set Radio Power due to MSIM mode change, power: " + singlePhonePower
                        + ", phoneId: " + phoneId);
                setRadioPower(singlePhonePower, phoneId);
            }
        }
    }

    protected void refreshIccIdPreference(boolean power, String iccid) {
        log("refresh iccid preference");
        SharedPreferences.Editor editor = sIccidPreference.edit();
        if (power == RADIO_POWER_OFF && !STRING_NO_SIM_INSERTED.equals(iccid)) {
            putIccIdToPreference(editor, iccid);
        } else {
            removeIccIdFromPreference(editor, iccid);
        }
        editor.commit();
    }

    private void putIccIdToPreference(SharedPreferences.Editor editor, String iccid) {
        if (iccid != null) {
            String printableIccid;
            if ("N/A".equals(iccid)) {
                printableIccid = "N/A";
            } else {
                printableIccid = binaryToHex(
                        getHashCode(SubscriptionInfo.givePrintableIccid(iccid)));
            }
            log("Add radio off SIM: " + printableIccid);
            editor.putInt(getHashCode(iccid), 0);
         }
    }

    private void removeIccIdFromPreference(SharedPreferences.Editor editor, String iccid) {
        if (iccid != null) {
            String printableIccid;
            if ("N/A".equals(iccid)) {
                printableIccid = "N/A";
            } else {
                printableIccid = binaryToHex(
                        getHashCode(SubscriptionInfo.givePrintableIccid(iccid)));
            }
            log("Remove radio off SIM: " + printableIccid);
            editor.remove(getHashCode(iccid));
        }
    }

    /*
     * Some Request or AT command must made before EFUN
     * 1. Prevent waiting for response
     * 2. Send commands as the same channel as EFUN or CFUN
     */
    public static void sendRequestBeforeSetRadioPower(boolean power, int phoneId) {
        log("Send request before EFUN, power:" + power + " phoneId:" + phoneId);

        notifyRadioPowerChange(power, phoneId);
    }

    /**
     * MTK Power on feature
     * 1. Radio off a card from SIM Management
     * 2. Flight power off modem
     * @internal
     */
    public static boolean isPowerOnFeatureAllClosed() {
        boolean ret = true;
        if (isFlightModePowerOffModemConfigEnabled()) {
            ret = false;
        } else if (isMSimModeSupport()) {
            ret = false;
        }
        return ret;
    }

    public static boolean isFlightModePowerOffModemConfigEnabled() {
        //first check testmode
        if (SystemProperties.get("vendor.ril.testmode").equals("1")) {
            return SystemProperties.get("vendor.ril.test.poweroffmd").equals("1");
        }

        //second check special test request
        if (isOP01 || isOP09) {
            //for modem OP01 PCT2.0 case 11.2.4/19.4.1/9.2.1.29
            String fromAtci = SystemProperties.get("vendor.ril.atci.flightmode");
            if (fromAtci.equals("1")) {
                return true;
            }

            //CMCC VOLTE TEST NEED START
            boolean isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                       SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                       SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                       SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
            if (isTestSim) {
                return true;
            }
        }

        //at last check the project feature option.
        return mFlightModePowerOffModem;
    }

    public static boolean isFlightModePowerOffModemEnabled() {

        if (RadioManager.getInstance() == null) {
            log("Instance not exists, return config only");
            return isFlightModePowerOffModemConfigEnabled();
        }

        if (isFlightModePowerOffModemConfigEnabled() == true) {
            return !RadioManager.getInstance().mIsWifiOn;
        } else {
            return false;
        }
    }

    /**
     *  Check if modem is already power off.
     **/
    public static boolean isModemPowerOff(int phoneId) {
        return RadioManager.getInstance().isModemOff(phoneId);
    }

    public static boolean isMSimModeSupport() {
        // TODO: adds logic
        /*if (SystemProperties.get("ro.mtk_bsp_package").equals("1")) {
            return false;
        } else {
            return true;
        }*/
        return true;
    }

    protected void resetSimInsertedStatus(int phoneId) {
        log("reset Sim InsertedStatus for Phone:" + phoneId);
        mSimInsertedStatus[phoneId] = SIM_NOT_INITIALIZED;
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        int[] ints;
        String[] strings;
        Message message;
        int phoneIdForMsg = getCiIndex(msg);

        log("handleMessage msg.what: " + eventIdtoString(msg.what));
        switch (msg.what) {
            case EVENT_RADIO_AVAILABLE_SLOT_1:
            case EVENT_RADIO_AVAILABLE_SLOT_2:
            case EVENT_RADIO_AVAILABLE_SLOT_3:
            case EVENT_RADIO_AVAILABLE_SLOT_4:
                // send event to check modem request
                notifyRadioAvailable(msg.what-1);
                break;
            case EVENT_VIRTUAL_SIM_ON:
                forceSetRadioPower(RADIO_POWER_ON, phoneIdForMsg);
                break;
            case EVENT_SET_MODEM_POWER_OFF_DONE:
                log("handle EVENT_SET_MODEM_POWER_OFF_DONE -> " + (mModemPower ? "ON" : "OFF"));
                if (!mModemPower) {
                    ar = (AsyncResult) msg.obj;
                    final ModemPowerMessage powerMessage = (ModemPowerMessage) ar.userObj;
                    log("handleModemPowerMessage, message:" + powerMessage.toString());
                    if (ar.exception == null) {
                        log("handleModemPowerMessage, result: " + ar.result);
                    } else {
                        // error handle
                        log("handleModemPowerMessage, Unhandle ar.exception:" + ar.exception);
                    }

                    powerMessage.isFinish = true;

                    if (!isSetModemPowerFinish()) {
                        return;
                    }
                    cleanModemPowerMessage();

                    unMonitorModemPowerChangeDone();
                    if (ar.exception == null) {
                        log("send EVENT_MODEM_POWER_OFF_DONE");
                        mPowerSM.sendEvent(PowerEvent.EVENT_MODEM_POWER_OFF_DONE);
                    } else {
                        // MTK HIDL will set later than AOSP HIDL,
                        // radio available will send after AOSP HIDL ready, but MTK HIDL is not
                        // ready, it will cause timing issue that set modem power request fail due
                        // to send set_modem_power reqest when receive radio available but
                        // MTK HIDL not ready. Retry it when modem power request got fail
                        log("retry turn off MD()");
                        setModemPower(false, mPhoneCount);
                    }
                } else {
                    log("EVENT_SET_MODEM_POWER_OFF_DONE: wrong state");
                }
                break;
            case EVENT_DSBP_STATE_CHANGED_SLOT_1:
            case EVENT_DSBP_STATE_CHANGED_SLOT_2:
            case EVENT_DSBP_STATE_CHANGED_SLOT_3:
            case EVENT_DSBP_STATE_CHANGED_SLOT_4:
                ar = (AsyncResult) msg.obj;
                notifyDsbpStateChanged(msg.what, ar);
                break;
            default:
                super.handleMessage(msg);
                break;
        }
    }

    private void notifyDsbpStateChanged(int what, AsyncResult ar) {
        if ((ar.exception == null) && (ar.result != null)) {
            int state = (int) ar.result;
            int phoneId = 0;
            switch (what) {
                case EVENT_DSBP_STATE_CHANGED_SLOT_1:
                    phoneId = 0;
                    break;
                case EVENT_DSBP_STATE_CHANGED_SLOT_2:
                    phoneId = 1;
                    break;
                case EVENT_DSBP_STATE_CHANGED_SLOT_3:
                    phoneId = 2;
                    break;
                case EVENT_DSBP_STATE_CHANGED_SLOT_4:
                    phoneId = 3;
                    break;
                default:
                    break;
            }
            log("notifyDsbpStateChanged state:" + state + "phoneId:" + phoneId);
            if (state == DsbpState.DSBP_ENHANCEMENT_START) {
                mIsDsbpChanging[phoneId] = true;
            } else {
                mIsDsbpChanging[phoneId] = false;
                if (findMainCapabilityPhoneId() == phoneId) {
                    if (mIsPendingRadioByDsbpChanging) {
                        mIsPendingRadioByDsbpChanging = false;
                        setRadioPowerAfterCapabilitySwitch();
                    }
                }
            }
        }
    }

    private String eventIdtoString(int what) {
        String str = null;
        switch (what) {
            case EVENT_RADIO_AVAILABLE_SLOT_1:
            case EVENT_RADIO_AVAILABLE_SLOT_2:
            case EVENT_RADIO_AVAILABLE_SLOT_3:
            case EVENT_RADIO_AVAILABLE_SLOT_4:
                str = "EVENT_RADIO_AVAILABLE";
                break;
            case EVENT_VIRTUAL_SIM_ON:
                str = "EVENT_VIRTUAL_SIM_ON";
                break;
            case EVENT_SET_SILENT_REBOOT_DONE:
                str = "EVENT_SET_SILENT_REBOOT_DONE";
                break;
            case EVENT_REPORT_AIRPLANE_DONE:
                str = "EVENT_REPORT_AIRPLANE_DONE";
                break;
            case EVENT_REPORT_SIM_MODE_DONE:
                str = "EVENT_REPORT_SIM_MODE_DONE";
                break;
            case EVENT_DSBP_STATE_CHANGED_SLOT_1:
            case EVENT_DSBP_STATE_CHANGED_SLOT_2:
            case EVENT_DSBP_STATE_CHANGED_SLOT_3:
            case EVENT_DSBP_STATE_CHANGED_SLOT_4:
                str = "EVENT_DSBP_STATE_CHANGED";
            default:
                break;
        }
        return str;
    }

    private int getCiIndex(Message msg) {
        AsyncResult ar;
        Integer index = new Integer(PhoneConstants.DEFAULT_CARD_INDEX);

        /*
         * The events can be come in two ways. By explicitly sending it using
         * sendMessage, in this case the user object passed is msg.obj and from
         * the CommandsInterface, in this case the user object is msg.obj.userObj
         */
        if (msg != null) {
            if (msg.obj != null && msg.obj instanceof Integer) {
                index = (Integer) msg.obj;
            } else if (msg.obj != null && msg.obj instanceof AsyncResult) {
                ar = (AsyncResult) msg.obj;
                if (ar.userObj != null && ar.userObj instanceof Integer) {
                    index = (Integer) ar.userObj;
                }
            }
        }
        return index.intValue();
    }

    protected boolean isModemOff(int phoneId) {
        boolean powerOff = false;
        TelephonyManager.MultiSimVariants config
            = TelephonyManager.getDefault().getMultiSimConfiguration();
        switch(config) {
            case DSDS:
                powerOff = SystemProperties.get("vendor.ril.ipo.radiooff").equals("1");
                break;
            case DSDA:
                switch (phoneId) {
                    case 0: //phone 1
                        powerOff = SystemProperties.get("vendor.ril.ipo.radiooff").equals("1");
                        break;
                    case 1: //phone 2
                        powerOff = SystemProperties.get("vendor.ril.ipo.radiooff.2").equals("1");
                        break;
                    default:
                        powerOff = true;
                        break;
                }
                break;
            case TSTS:
                //TODO: check 3 SIM case
                powerOff = SystemProperties.get("vendor.ril.ipo.radiooff").equals("1");
                break;
            default:
                 powerOff = SystemProperties.get("vendor.ril.ipo.radiooff").equals("1");
                break;
        }
        return powerOff;
    }

     public static synchronized void registerForRadioPowerChange(String name,
            IRadioPower iRadioPower) {
        if (name == null) {
            name = REGISTRANTS_WITH_NO_NAME;
        }
        log(name + " registerForRadioPowerChange");
        mNotifyRadioPowerChange.put(iRadioPower, name);
    }

    public static synchronized void unregisterForRadioPowerChange(IRadioPower iRadioPower) {
        log(mNotifyRadioPowerChange.get(iRadioPower) + " unregisterForRadioPowerChange");
        mNotifyRadioPowerChange.remove(iRadioPower);
    }

    private static synchronized void notifyRadioPowerChange(boolean power, int phoneId) {
        for (Entry<IRadioPower, String> e : mNotifyRadioPowerChange.entrySet()) {
            log("notifyRadioPowerChange: user:" + e.getValue());
            IRadioPower iRadioPower = e.getKey();
            iRadioPower.notifyRadioPowerChange(power, phoneId);
        }
    }

    private static void log(String s) {
        Rlog.d(LOG_TAG, "[RadioManager] " + s);
    }

    public boolean isAllowAirplaneModeChange() {
        log("always allow airplane mode");
        return true;
    }

    /**
     * Set Whether force allow airplane mode change.
     * @return true or false
     */
    public void forceAllowAirplaneModeChange(boolean forceSwitch) {
        // TODO: remove this API
    }

    protected final Message[] monitorModemPowerChangeDone(boolean power, int phoneBitMap,
            int mainCapabilityPhoneId) {
        mModemPower = power;

        log("monitorModemPowerChangeDone, Power:" + power + ", PhoneBitMap:" + phoneBitMap
                + ", mainCapabilityPhoneId:" + mainCapabilityPhoneId
                + ", mPhoneCount:" + mPhoneCount);
        mNeedIgnoreMessageForChangeDone = false;
        mIsRadioUnavailable = false;
        final Message[] msgs = new Message[mPhoneCount];

        if (!mModemPower) {
            // [Modem Power Off] wait response message
            // Create Message
            final ModemPowerMessage[] messages = createMessage(power, phoneBitMap,
                    mainCapabilityPhoneId, mPhoneCount);
            mModemPowerMessages = messages;

            // Obtain Message
            for (int i = 0; i < messages.length; i++) {
                if (messages[i] != null) {
                    msgs[i] = obtainMessage(EVENT_SET_MODEM_POWER_OFF_DONE, messages[i]);
                }
            }
        }

        return msgs;
    }

    protected void unMonitorModemPowerChangeDone() {
        mNeedIgnoreMessageForChangeDone = true;
        // TODO: remove this
        Intent intent = new Intent(RadioManager.ACTION_AIRPLANE_CHANGE_DONE);
        intent.putExtra(RadioManager.EXTRA_AIRPLANE_MODE, !mModemPower);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);

        Phone phone;
        for (int i = 0; i < mPhoneCount; i++) {
            phone = PhoneFactory.getPhone(i);
            if (phone == null) {
                continue;
            }
            phone.mCi.unregisterForRadioStateChanged(this);
            log("unMonitorModemPowerChangeDone, phoneId = " + i);
        }
    }

    protected boolean waitForReady(boolean state) {
        if (waitRadioAvaliable(state)) {
            log("waitForReady, wait radio avaliable");
            mPowerSM.updateModemPowerState(state, mBitmapForPhoneCount,
                    ModemPowerCasue.CAUSE_AIRPLANE_MODE);
            //mPowerSM.sendEvent(state? PowerEvent.EVENT_MODEM_POWER_ON:
            //        PowerEvent.EVENT_MODEM_POWER_OFF);
            return true;
        } else {
            return false;
        }
    }

    private boolean waitRadioAvaliable(boolean state) {
        final boolean wait = !mIsWifiOnlyDevice && !isRadioAvaliable();
        log("waitRadioAvaliable, state=" + state + ", wait=" + wait);
        return wait;
    }

    private boolean isRadioAvaliable() {
        boolean isRadioAvaliable = true;
        for (int i = 0; i < mPhoneCount; i++) {
            if (!isRadioAvaliable(i)) {
                log("isRadioAvaliable=false, phoneId = " + i);
                isRadioAvaliable = false;
                break;
            }
        }
        return isRadioAvaliable;
    }

    private boolean isRadioAvaliable(int phoneId) {
        Phone phone;
        phone = PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            return false;
        }

        log("phoneId = " + phoneId + ", RadioState="
                + phone.mCi.getRadioState());
        return phone.mCi.getRadioState() != TelephonyManager.RADIO_POWER_UNAVAILABLE;
    }

    private boolean isRadioOn() {
        boolean isRadioOn = true;
        for (int i = 0; i < mPhoneCount; i++) {
            if (!isRadioOn(i)) {
                isRadioOn = false;
                break;
            }
        }
        return isRadioOn;
    }

    private boolean isRadioOn(int phoneId) {
        Phone phone;
        phone = PhoneFactory.getPhone(phoneId);
        if (phone == null) {
            return false;
        }

        return phone.mCi.getRadioState() == TelephonyManager.RADIO_POWER_ON;
    }

    private boolean isRadioUnavailable() {
        boolean isRadioUnavailable = true;
        for (int i = 0; i < mPhoneCount; i++) {
            if (isRadioAvaliable(i)) {
                log("isRadioUnavailable=false, phoneId = " + i);
                isRadioUnavailable = false;
                break;
            }
        }
        return isRadioUnavailable;
    }

    private final boolean isSetModemPowerFinish() {
        if (mModemPowerMessages != null) {
            for (int i = 0; i < mModemPowerMessages.length; i++) {
                if (mModemPowerMessages[i] != null) {
                    log("isSetModemPowerFinish [" + i + "]: " + mModemPowerMessages[i]);
                    if (!mModemPowerMessages[i].isFinish) {
                        return false;
                    }
                } else {
                    log("isSetModemPowerFinish [" + i + "]: MPMsg is null");
                }
            }
        }
        return true;
    }

    private final void cleanModemPowerMessage() {
        log("cleanModemPowerMessage");
        if (mModemPowerMessages != null) {
            for (int i = 0; i < mModemPowerMessages.length; i++) {
                mModemPowerMessages[i] = null;
            }
            mModemPowerMessages = null;
        }
    }

    private static final class ModemPowerMessage {
        private final int mPhoneId;
        public boolean isFinish = false;

        public ModemPowerMessage(int phoneId) {
            this.mPhoneId = phoneId;
        }

        @Override
        public String toString() {
            return "MPMsg [mPhoneId=" + mPhoneId + ", isFinish=" + isFinish + "]";
        }
    }

    private static final ModemPowerMessage[] createMessage(
            boolean power, int phoneBitMap, int mainCapabilityPhoneId, int phoneCount) {
        final TelephonyManager.MultiSimVariants config =
                TelephonyManager.getDefault().getMultiSimConfiguration();
        log("createMessage, config:" + config);

        final ModemPowerMessage[] msgs = new ModemPowerMessage[phoneCount];
        int phoneId = 0;
        switch (config) {
            case DSDS:
            case DSDA:
            case TSTS:
                phoneId = mainCapabilityPhoneId;
                msgs[phoneId] = new ModemPowerMessage(phoneId);
                break;
            default:
                phoneId = PhoneFactory.getDefaultPhone().getPhoneId();
                msgs[phoneId] = new ModemPowerMessage(phoneId);
                break;
        }

        for (int i = 0; i < phoneCount; i++) {
            if (msgs[i] != null) {
                log("createMessage, [" + i + "]: " + msgs[i].toString());
            }
        }
        return msgs;
    }

    private void registerListener() {
        for (int i = 0; i < mPhoneCount; i++) {
            ((MtkRIL)mCi[i]).registerForVirtualSimOn(this, EVENT_VIRTUAL_SIM_ON, null);
            mCi[i].registerForAvailable(this, EVENT_RADIO_AVAILABLE[i], null);
            ((MtkRIL)mCi[i]).registerForDsbpStateChanged(this,
                    EVENT_DSBP_STATE_CHANGED[i], null);
        }
    }

    private boolean isRequiredRadioOff(String iccid) {
        if (sIccidPreference.contains(getHashCode(iccid))) {
            return true;
        }
        return false;
    }

    public String getHashCode(String iccid) {
        try {
            MessageDigest alga = MessageDigest.getInstance("SHA-256");
            alga.update(iccid.getBytes());
            byte[] hashCode = alga.digest();
            String strIccid = new String(hashCode);
            return strIccid;
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("isRequiredRadioOff SHA-256 must exist");
        }
    }

    /* State machine for modem/radio power */
    private class PowerSM extends StateMachine {
        private PowerSM self = null;
        /* States */
        protected PowerIdleState mIdleState = new PowerIdleState();
        protected PowerTurnOnState mTurnOnState = new PowerTurnOnState();
        protected PowerTurnOffState mTurnOffState = new PowerTurnOffState();

        public boolean mCurrentModemPower;
        public boolean mDesiredModemPower;
        private int mCurrentModemCause;
        private int mDesiredModemCause;
        protected int mPhoneBitMap;

        /*public PowerSM init (int phoneCount) {
            if (self == null) {
                self = new PowerSM();
            } else {
                Rlog.e("RadioManager", "PowerSM already init");
            }
        }*/

        /*public*/PowerSM(String name) {
            super(name);
            mCurrentModemPower = true;
            mDesiredModemPower = true;
            mCurrentModemCause = ModemPowerCasue.CAUSE_START;
            mDesiredModemCause = ModemPowerCasue.CAUSE_START;

            // Initialize state and set all state
            addState(mIdleState);
            addState(mTurnOnState);
            addState(mTurnOffState);
            setInitialState(mIdleState);
        }

        private void updateModemPowerState(boolean power, int phoneBitMap,
                int cause) {
            if (!power & isUnderCryptKeeper()) {
                log("Skip MODEM_POWER_OFF due to CryptKeeper mode");
                return;
            }
            mPhoneBitMap = phoneBitMap;
            if(ModemPowerCasue.CAUSE_WIFI_CALLING == cause) {
                if (mAirplaneMode == AIRPLANE_MODE_ON &&
                        isFlightModePowerOffModemConfigEnabled()) {
                    mDesiredModemCause |= ModemPowerCasue.CAUSE_WIFI_CALLING;
                    if (mIsWifiOn) {
                        // update desired state
                        mDesiredModemPower = true;
                    } else {
                        // update desired state
                        mDesiredModemPower = false;
                    }
                }
                sendEvent(power? PowerEvent.EVENT_MODEM_POWER_ON: PowerEvent.EVENT_MODEM_POWER_OFF);
            } else if (ModemPowerCasue.CAUSE_AIRPLANE_MODE == cause) {
                // update desired state
                mDesiredModemCause |= ModemPowerCasue.CAUSE_AIRPLANE_MODE;
                mDesiredModemPower = power;
                sendEvent(power? PowerEvent.EVENT_MODEM_POWER_ON: PowerEvent.EVENT_MODEM_POWER_OFF);
            } else if (ModemPowerCasue.CAUSE_ECC == cause) {
                // update desired state
                mDesiredModemCause |= ModemPowerCasue.CAUSE_ECC;
                mDesiredModemPower = power;
                sendEvent(PowerEvent.EVENT_MODEM_POWER_ON);
            } else if (ModemPowerCasue.CAUSE_IPO == cause) {
                // update desired state
                mDesiredModemCause |= ModemPowerCasue.CAUSE_IPO;
                mDesiredModemPower = power;
                sendEvent(power? PowerEvent.EVENT_MODEM_POWER_ON: PowerEvent.EVENT_MODEM_POWER_OFF);
            } else if (ModemPowerCasue.CAUSE_RADIO_AVAILABLE == cause) {
                // meants RILD restart => modem is on
                mCurrentModemPower = true;
                mDesiredModemPower = false;
                sendEvent(PowerEvent.EVENT_MODEM_POWER_OFF);
            } else if (ModemPowerCasue.CAUSE_SIM_SWITCH == cause) {
                sendEvent(PowerEvent.EVENT_SIM_SWITCH_DONE);
            }
        }

        private void sendEvent(int event, int arg1) {
            Rlog.i(LOG_TAG, "sendEvent: " + PowerEvent.print(event));
            Message msg = Message.obtain(getHandler(), event);
            msg.arg1 = arg1;
            getHandler().sendMessage(msg);
        }

        private void sendEvent(int event) {
            Rlog.i(LOG_TAG, "sendEvent: " + PowerEvent.print(event));
            Message msg = Message.obtain(getHandler(), event);
            getHandler().sendMessage(msg);
        }

        private class PowerIdleState extends State {
            @Override
            public void enter() {
                Rlog.i(LOG_TAG, "PowerIdleState: enter");
                log("mDesiredModemPower: " + mDesiredModemPower +
                        " mCurrentModemPower: " + mCurrentModemPower);
                if (mPowerSM.mDesiredModemPower != mPowerSM.mCurrentModemPower) {
                    if (mPowerSM.mDesiredModemPower) {
                        mPowerSM.transitionTo(mPowerSM.mTurnOnState);
                    } else {
                        mPowerSM.transitionTo(mPowerSM.mTurnOffState);
                    }
                }
            }
            @Override
            public void exit() {
                Rlog.i(LOG_TAG, "PowerIdleState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                Rlog.i(LOG_TAG, "processMessage: " + PowerEvent.print(msg.what));
                switch(msg.what) {
                    case PowerEvent.EVENT_MODEM_POWER_ON:
                        if (mPowerSM.mDesiredModemPower != mPowerSM.mCurrentModemPower) {
                            mPowerSM.transitionTo(mPowerSM.mTurnOnState);
                        } else {
                            Rlog.i(LOG_TAG, "the same power state: " + PowerEvent.print(msg.what));

                            // Fix timing issue
                            // Broadcast intent to trigger SIM re-poll sim state because modem will
                            // keep alive and no radio available event send again
                            Intent intent = new Intent(RadioManager.ACTION_MODEM_POWER_NO_CHANGE);
                            intent.putExtra(RadioManager.EXTRA_MODEM_POWER, true);
                            mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
                        }
                        break;
                    case PowerEvent.EVENT_MODEM_POWER_OFF:
                        if (mPowerSM.mDesiredModemPower != mPowerSM.mCurrentModemPower) {
                            mPowerSM.transitionTo(mPowerSM.mTurnOffState);
                        } else {
                            Rlog.i(LOG_TAG, "the same power state: " + PowerEvent.print(msg.what));
                        }
                        break;
                    case PowerEvent.EVENT_RADIO_AVAILABLE:
                        mPowerSM.mCurrentModemPower = true;
                        if (mPowerSM.mDesiredModemPower != mPowerSM.mCurrentModemPower) {
                            if (mPowerSM.mDesiredModemPower) {
                                mPowerSM.transitionTo(mPowerSM.mTurnOnState);
                            } else {
                                mPowerSM.transitionTo(mPowerSM.mTurnOffState);
                            }
                        }
                        break;
                    case PowerEvent.EVENT_MODEM_POWER_OFF_DONE:
                        mPowerSM.mCurrentModemPower = false;
                        if (mPowerSM.mDesiredModemPower != mPowerSM.mCurrentModemPower) {
                            mPowerSM.transitionTo(mPowerSM.mDesiredModemPower ?
                                    mPowerSM.mTurnOnState : mPowerSM.mTurnOffState);
                        } else {
                            Rlog.i(LOG_TAG, "the same power state: " + PowerEvent.print(msg.what));
                        }
                        break;
                    default:
                        Rlog.i(LOG_TAG, "un-expected event, stay at idle");
                }
                return true;
            }
        }

        private class PowerTurnOnState extends State {
            @Override
            public void enter() {
                Rlog.i(LOG_TAG, "PowerTurnOnState: enter");
                // check condition
                if (!waitForReady(true)
                        && !((MtkProxyController)MtkProxyController.getInstance()).
                        isCapabilitySwitching()) {
                    mPowerSM.mCurrentModemPower = true;
                    mPowerSM.mCurrentModemCause = mPowerSM.mDesiredModemCause;
                    setModemPower(true, mPowerSM.mPhoneBitMap);
                }
            }
            @Override
            public void exit() {
                Rlog.i(LOG_TAG, "PowerTurnOnState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                Rlog.i(LOG_TAG, "processMessage: " + PowerEvent.print(msg.what));
                switch(msg.what) {
                    case PowerEvent.EVENT_RADIO_AVAILABLE:
                        mCurrentModemPower = true;
                        mPowerSM.transitionTo(mPowerSM.mIdleState);
                        break;
                    case PowerEvent.EVENT_MODEM_POWER_ON_DONE:
                    case PowerEvent.EVENT_SIM_SWITCH_DONE:
                        mPowerSM.transitionTo(mPowerSM.mIdleState);
                        break;
                    default:
                        Rlog.i(LOG_TAG, "un-expected event, stay at PowerTurnOnState");
                }
                return true;
            }
        }

        private class PowerTurnOffState extends State {
            @Override
            public void enter() {
                Rlog.i(LOG_TAG, "PowerTurnOffState: enter");
                if (!waitForReady(false)
                        && !((MtkProxyController)MtkProxyController.getInstance()).
                        isCapabilitySwitching()) {
                    mPowerSM.mCurrentModemPower = false;
                    mPowerSM.mCurrentModemCause = mPowerSM.mDesiredModemCause;
                    setModemPower(false, mPowerSM.mPhoneBitMap);
                }
            }
            @Override
            public void exit() {
                Rlog.i(LOG_TAG, "PowerTurnOffState: exit");
            }
            @Override
            public boolean processMessage(Message msg) {
                Rlog.i(LOG_TAG, "processMessage: " + PowerEvent.print(msg.what));
                switch(msg.what) {
                    case PowerEvent.EVENT_RADIO_AVAILABLE:
                        mCurrentModemPower = true;
                        mPowerSM.transitionTo(mPowerSM.mIdleState);
                        break;
                    case PowerEvent.EVENT_MODEM_POWER_OFF_DONE:
                    case PowerEvent.EVENT_SIM_SWITCH_DONE:
                        mPowerSM.transitionTo(mPowerSM.mIdleState);
                        break;
                    default:
                        Rlog.i(LOG_TAG, "un-expected event, stay at PowerTurnOffState");
                }
                return true;
            }
        }
    }

    static class PowerEvent {
        static final int EVENT_START = 0;
        static final int EVENT_MODEM_POWER_ON = EVENT_START + 1;
        static final int EVENT_MODEM_POWER_OFF = EVENT_START + 2;
        static final int EVENT_RADIO_AVAILABLE = EVENT_START + 3;
        static final int EVENT_MODEM_POWER_ON_DONE = EVENT_START + 4;
        static final int EVENT_MODEM_POWER_OFF_DONE = EVENT_START + 5;
        static final int EVENT_SIM_SWITCH_DONE = EVENT_START + 6;

        public static String print(int eventCode) {
            String outString = "";
            switch (eventCode) {
                case EVENT_MODEM_POWER_ON: outString = "EVENT_MODEM_POWER_ON"; break;
                case EVENT_MODEM_POWER_OFF: outString = "EVENT_MODEM_POWER_OFF"; break;
                case EVENT_RADIO_AVAILABLE: outString = "EVENT_RADIO_AVAILABLE"; break;
                case EVENT_MODEM_POWER_ON_DONE: outString = "EVENT_MODEM_POWER_ON_DONE"; break;
                case EVENT_MODEM_POWER_OFF_DONE: outString = "EVENT_MODEM_POWER_OFF_DONE"; break;
                case EVENT_SIM_SWITCH_DONE: outString = "EVENT_SIM_SWITCH_DONE"; break;
                default:
                    throw new IllegalArgumentException("Invalid eventCode: " + eventCode);
            }
            return outString;
        }
    }

    static class ModemPowerCasue {
        static final int CAUSE_START = 0;
        static final int CAUSE_AIRPLANE_MODE = 1 << 1;
        static final int CAUSE_WIFI_CALLING = 1 << 2;
        static final int CAUSE_IPO = 1 << 3;
        static final int CAUSE_ECC = 1 << 4;
        static final int CAUSE_FORCE = 1 << 5; // useless
        static final int CAUSE_RADIO_AVAILABLE = 1 << 6;
        static final int CAUSE_SIM_SWITCH = 1 << 7;

        public static String print(int eventCode) {
            String outString = "";
            switch (eventCode) {
                case CAUSE_AIRPLANE_MODE: outString = "CAUSE_AIRPLANE_MODE"; break;
                case CAUSE_WIFI_CALLING: outString = "CAUSE_WIFI_CALLING"; break;
                case CAUSE_IPO: outString = "CAUSE_IPO"; break;
                case CAUSE_ECC: outString = "CAUSE_ECC"; break;
                case CAUSE_FORCE: outString = "CAUSE_FORCE"; break;
                case CAUSE_RADIO_AVAILABLE: outString = "CAUSE_RADIO_AVAILABLE"; break;
                default:
                    throw new IllegalArgumentException("Invalid eventCode: " + eventCode);
            }
            return outString;
        }
    }

    private String binaryToHex(String binaryStr) {
        return String.format("%040x", new BigInteger(1, binaryStr.getBytes()));
    }
}
