/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.rcs.provisioning.https;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Parcel;
import android.os.Process;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ServiceState;
import android.text.format.DateUtils;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import com.mediatek.rcs.provisioning.AcsConfigInfo;
import com.mediatek.rcs.provisioning.IAcsCallback;
import com.mediatek.rcs.provisioning.IAcsService;
import com.mediatek.rcs.provisioning.ProvisioningFailureReasons;
import com.mediatek.rcs.provisioning.ProvisioningInfo.Version;
import com.mediatek.rcs.provisioning.R;
import com.mediatek.rcs.provisioning.TermsAndConditionsRequest;
import com.mediatek.rcs.utils.AndroidFactory;
import com.mediatek.rcs.utils.ContactId;
import com.mediatek.rcs.utils.DeviceUtils;
import com.mediatek.rcs.utils.LauncherUtils;
import com.mediatek.rcs.utils.NetworkUtils;
import com.mediatek.rcs.utils.RcsSettings;
import com.mediatek.rcs.utils.RcsSettings.TermsAndConditionsResponse;
import com.mediatek.rcs.utils.TimerUtils;
import com.mediatek.rcs.utils.logger.Logger;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class AcsService extends Service {
    private static final boolean TEST_MODE_ENABLED = true;
    // SystemProperties.get("ro.build.type").equals("user") ? false : true;

    /**
     * Intent key - Provisioning requested after (re)boot
     */
    private static final String FIRST_KEY = "first";

    /**
     * Intent key - Provisioning requested by user
     */
    private static final String USER_KEY = "user";

    /**
     * Retry Intent
     */
    private PendingIntent mRetryIntent;

    private RcsSettings mRcsSettings;

    private Context mContext;

    /**
     * Provisioning manager
     */
    private HttpsProvisioningManager mHttpsProvisioningMng = null;

    /**
     * Retry action for provisioning failure
     */
    private static final String ACTION_RETRY = "com.mediatek.rcs.provisioning.https.HttpsProvisioningService.ACTION_RETRY";

    private static final Logger sLogger = Logger.getLogger(AcsService.class.getName());

    private static RemoteCallbackList<IAcsCallback> sListenerList = new RemoteCallbackList<IAcsCallback>();

    private AtomicBoolean mIsServiceDestroyed = new AtomicBoolean(false);
    private static final String PACKAGE_PERMISSION =
        "com.mediatek.rcs.permission.ACCESS_ACS_SERVICE";
    private String mLastUserAccount;
    private String mCurrentUserAccount;
    private boolean mFirstLaunch = false;
    private static long sRetryAlarmTime = 0;
    private static long sLastTriggerTime = 0;
    private static final int EVENT_INTERNAL_REQUEST = 0;
    private static final int EVENT_IMS_REGISTERED = 1;
    private static final int EVENT_ROAMING_CHANGE = 2;
    private static final int EVENT_ROAMING_SETTING_CHANGE = 3;
    private static final int EVENT_SWITCH_RCS_STATE = 4;
    private static final int EVENT_USER_TRIGGER_REQUEST = 5;
    private static final int EVENT_USER_CLEAR_CONFIGURATION = 6;
    private static final int EVENT_USER_SET_SWITCH_STATE = 7;
    private static final int EVENT_USER_SET_PROVISIONING_ADDR = 8;
    private static final int EVENT_SET_MSISDN = 9;
    private static boolean mUserDisableAcs = false;
    private static String mUserProvisioningAddr = null;
    private SettingsObserver mSettingsObserver;
    private boolean mIsRoaming;
    private boolean mWaitMSISDN = false;
    private Binder mBinder = new IAcsService.Stub() {
            @Override
            public AcsConfigInfo getAcsConfiguration() throws RemoteException {
                int version = mRcsSettings.getProvisioningVersion();
                int state = (version > 0)
                    ? AcsConfigInfo.CONFIGURED
                    : AcsConfigInfo.PRE_CONFIGURATION;
                return new AcsConfigInfo(mRcsSettings.getConfigFileContent(), state, version);
            }

            @Override
            public int getAcsConfigInt(String configItem) {
                int config = Integer.MIN_VALUE;
                switch(configItem) {
                case RcsSettings.PUBLISH_SOURCE_THROTTLE:
                case RcsSettings.CAPABILITY_POLLING_PERIOD:
                case RcsSettings.CAPABILITY_NON_RCSCAPINFO_EXPIRAY:
                case RcsSettings.CAPABILITY_INFO_EXPIRY:
                case RcsSettings.PUBLISH_DISABLE_INITIAL_ADDRESS_BOOK_SCAN:
                case RcsSettings.CAPABILITY_POLLIGN_RATE:
                case RcsSettings.CAPABILITY_POLLIGN_RATE_PERIOD:
                case RcsSettings.CAPABILITY_DEFAULT_DISC:
                case RcsSettings.CAPABILITY_DISC_COMMON_STACK:
                    config = (int) mRcsSettings.readLong(configItem);
                    break;

                default:
                    if (TEST_MODE_ENABLED) {
                        try {
                            config = mRcsSettings.readInteger(configItem);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                    break;
                }

                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("getAcsConfigInt configItem:" + configItem + "," + config);
                }
                return config;
            }

            @Override
            public String getAcsConfigString(String configItem) {
                String config = null;
                switch(configItem) {
                case RcsSettings.PUBLISH_SOURCE_THROTTLE:
                case RcsSettings.CAPABILITY_POLLING_PERIOD:
                case RcsSettings.CAPABILITY_NON_RCSCAPINFO_EXPIRAY:
                case RcsSettings.CAPABILITY_INFO_EXPIRY:
                case RcsSettings.PUBLISH_DISABLE_INITIAL_ADDRESS_BOOK_SCAN:
                case RcsSettings.CAPABILITY_POLLIGN_RATE:
                case RcsSettings.CAPABILITY_POLLIGN_RATE_PERIOD:
                case RcsSettings.CAPABILITY_DEFAULT_DISC:
                case RcsSettings.CAPABILITY_DISC_COMMON_STACK:
                    config = String.valueOf(getAcsConfigInt(configItem));
                    break;

                case "ACS_URL":
                    if (TEST_MODE_ENABLED) {
                        if (mHttpsProvisioningMng != null) {
                            config = mHttpsProvisioningMng.buildProvisioningAddress();
                        }
                    }
                    break;

                case "ACS_VALIDITY":
                    if (TEST_MODE_ENABLED) {
                        config = String.valueOf(LauncherUtils.getProvisioningValidity(mContext));
                    }
                    break;

                case "ACS_EXPIRATION":
                    if (TEST_MODE_ENABLED) {
                        config = String
                            .valueOf(LauncherUtils.getProvisioningExpirationDate(mContext));
                    }
                    break;

                case RcsSettings.FT_HTTP_SERVER:
                    if (TEST_MODE_ENABLED) {
                        Uri addr = mRcsSettings.getFtHttpServer();
                        config = (addr == null) ? null : addr.toString();
                    }
                    break;

                case RcsSettings.FT_HTTP_LOGIN:
                case RcsSettings.FT_HTTP_PASSWORD:
                default:
                    if (TEST_MODE_ENABLED) {
                        try {
                            config = mRcsSettings.readString(configItem);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }
                    break;
                }

                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("getAcsConfigString configItem:" + configItem + "," + config);
                }
                return config;
            }

            @Override
            public void setRcsSwitchState(boolean state) {
                if (sLogger.isActivated()) {
                    sLogger.info("setRcsSwitchState state:" + state
                                 + ",cur:" + mRcsSettings.isServiceActivated());
                }
                if (mRcsSettings.isServiceActivated() != state) {
                    mRcsSettings.setServiceActivationState(state);
                    /*
                     * force update for ATT SPEC, EX: switch toggle
                     * off, ACS client shall send the appropriate
                     * signals to the server, specifically rcs_state
                     * shall be -4.
                     *
                     */
                    onConfigChange(AcsConfigInfo.PRE_CONFIGURATION,
                                   Version.RESETED.toInt());

                    if (RcsSettings.ATT_SUPPORT) {
                        mHandler.obtainMessage(EVENT_SWITCH_RCS_STATE).sendToTarget();
                    }
                }
            }

            @Override
            public int getAcsConnectionStatus() throws RemoteException {
                if (mHttpsProvisioningMng != null
                    && mHttpsProvisioningMng.isInConfiguringState()) {
                    return 0;
                }
                return 1;
            }

            @Override
            public boolean triggerAcsRequest(int reason) throws RemoteException {
                // 0 : REASON_DEFAULT_SMS_UNKNOWN
                // 1 : REASON_RCS_DEFAULT_SMS,
                // 2 : REASON_RCS_NOT_DEFAULT_SMS
                // 3 : REASON_FT_AUTHENTICATE_FAILURE
                if (sLogger.isActivated()) {
                    sLogger.info("triggerAcsRequest reason:" + reason);
                }
                switch (reason) {
                    case 0:
                        mRcsSettings.setDefaultSmsApp("unknow_default_sms_app");
                        break;

                    case 1:
                        mRcsSettings.setDefaultSmsApp(getString(R.string.default_sms_app));
                        break;

                    case 2:
                        mRcsSettings.setDefaultSmsApp("not_default_sms_app");
                        break;

                    case 3:
                        break;

                    default:
                        return false;
                }

                if (mHttpsProvisioningMng != null
                    && !mHttpsProvisioningMng.isRetryingOrInNoMoreRetryState()) {
                    mHandler.obtainMessage(EVENT_USER_TRIGGER_REQUEST).sendToTarget();
                    return true;
                } else {
                    if (sLogger.isActivated()) {
                        sLogger.info("triggerAcsRequest, isRetryingOrInNoMoreRetryState,ignore");
                    }
                    return false;
                }
            }

            @Override
            public void registerAcsCallback(IAcsCallback callback) throws RemoteException {
                if (callback == null) {
                    sLogger.debug("registerCallback, callback is null!");
                    return;
                }

                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("registerCallback, before size:" + getCallbackCount());
                }

                sListenerList.register(callback);

                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("registerCallback, after size:" + getCallbackCount());
                }

                int version = mRcsSettings.getProvisioningVersion();
                int state = (version > 0)
                    ? AcsConfigInfo.CONFIGURED
                    : AcsConfigInfo.PRE_CONFIGURATION;
                try {
                    callback.onAcsConfigChange(state, version);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }

            @Override
            public void unregisterAcsCallback(IAcsCallback callback) throws RemoteException {
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("unregisterCallback, before size:" + getCallbackCount());
                }

                sListenerList.unregister(callback);

                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("unregisterCallback, after size:" + getCallbackCount());
                }
            }

            @Override
            public boolean setAcsSwitchState(boolean state) throws RemoteException {
                if (TEST_MODE_ENABLED) {
                    mHandler.obtainMessage(EVENT_USER_SET_SWITCH_STATE, state).sendToTarget();
                    return true;
                }
                return false;
            }

            @Override
            public boolean getAcsSwitchState() throws RemoteException {
                if (TEST_MODE_ENABLED) {
                    return !mUserDisableAcs;
                }
                return true;
            }

            @Override
            public boolean setAcsProvisioningAddress(String address) throws RemoteException {
                if (TEST_MODE_ENABLED) {
                    mHandler.obtainMessage(EVENT_USER_SET_PROVISIONING_ADDR, address)
                        .sendToTarget();
                    return true;
                }

                return false;
            }

            @Override
            public boolean clearAcsConfiguration() throws RemoteException {
                if (TEST_MODE_ENABLED) {
                    mHandler.obtainMessage(EVENT_USER_CLEAR_CONFIGURATION).sendToTarget();
                    return true;
                }
                return false;
            }

            @Override
            public boolean setAcsMsisdn(String msisdn) throws RemoteException {
                if (!isSimSupported(mContext)) {
                    if (sLogger.isActivated()) {
                        sLogger.warn("current sim is not support, ignore...");
                    }
                    return false;
                }

                if (msisdn != null) {
                    String REGEXP_CONTACT = "^00\\d{1,15}$|^[+]?\\d{1,15}$|^\\d{1,15}$";
                    Pattern pattern = Pattern.compile(REGEXP_CONTACT);
                    Matcher matcher = pattern.matcher(msisdn);
                    if (!matcher.find()) {
                        if (sLogger.isActivated()) {
                            sLogger.warn("unknown msisdn format, ignore");
                        }
                        return false;
                    }
                    mHandler.obtainMessage(EVENT_SET_MSISDN, msisdn).sendToTarget();
                    if (sLogger.isActivatedDebug()) {
                        sLogger.info("setAcsMsisdn:"
                                     + msisdn.replaceAll(Logger.sLogNumPattern, "*"));
                    }
                    return true;
                } else {
                    sLogger.info("setAcsMsisdn: null, ignore");
                    return false;
                }
            }

            @Override
            public boolean onTransact(int code, Parcel data, Parcel reply, int flags)
                throws RemoteException {
                if (sLogger.isActivatedDebug()) {
                    sLogger.info("onTransact, code:" + code);
                }

                // String packageName = null;
                // String[] packages = getPackageManager().getPackagesForUid(getCallingUid());
                // if (packages != null && packages.length > 0) {
                //     packageName = packages[0];
                // }
                // if (!packageName.startsWith("com.mediatek")) {
                //     if (sLogger.isActivated()) {
                //         sLogger.info("onTransact packageName:" + packageName + ",not support");
                //     }
                //     return false;
                // }
                return super.onTransact(code, data, reply, flags);
            }
        };

    private int getCallbackCount() {
        int count = 0;
        final int N = sListenerList.beginBroadcast();
        for (int i = 0; i < N; i++) {
            IAcsCallback callback = sListenerList.getBroadcastItem(i);
            if (callback != null) {
                count++;
            }
        }
        sListenerList.finishBroadcast();
        return count;
    }

    private int getConfigStatus() {
        int state = AcsConfigInfo.PRE_CONFIGURATION;
        long expiration = LauncherUtils.getProvisioningExpirationDate(mContext);
        int version = mRcsSettings.getProvisioningVersion();
        if (version > 0) {
            state = AcsConfigInfo.CONFIGURED;
        }
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("getAcsConfig expiration:" + expiration
                          + ", version:" + version
                          + ", state:" + state);
        }

        return state;
    }

    static void onConfigChange(int state, int version) {
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("onConfigChange " + ",state:" + state + ",version:" + version);
        }

        final int N = sListenerList.beginBroadcast();
        for (int i = 0; i < N; i++) {
            IAcsCallback callback = sListenerList.getBroadcastItem(i);
            if (callback != null) {
                try {
                    callback.onAcsConfigChange(state, version);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }
        sListenerList.finishBroadcast();
    }


    static void onConnectionStateChanged(int state) {
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("onConnectionStateChanged " + ",state:" + state);
        }

        final int N = sListenerList.beginBroadcast();
        for (int i = 0; i < N; i++) {
            IAcsCallback callback = sListenerList.getBroadcastItem(i);
            if (callback != null) {
                try {
                    callback.onAcsConnectionStatusChange(state);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }
        sListenerList.finishBroadcast();
    }


    @Override
    public void onCreate() {
        if (sLogger.isActivated()) {
            sLogger.info("onCreate");
        }
        super.onCreate();

        mContext = getApplicationContext();
        AndroidFactory.setApplicationContext(mContext);

        mRcsSettings = RcsSettings.getInstance();
        mRetryIntent = PendingIntent.getBroadcast(mContext, 0, new Intent(ACTION_RETRY), 0);

        initialize();
    }

    private void initialize() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        filter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        mContext.registerReceiver(mBroadcastReceiver, filter);

        if (mRcsSettings.getClientVersion() == null) {
            mRcsSettings.setClientVersion(getString(R.string.client_version));
        }

        if (mRcsSettings.getClientVendor() == null) {
            mRcsSettings.setClientVendor(getString(R.string.client_vendor));
        }

        if (mRcsSettings.getDefaultSmsApp() == null) {
            mRcsSettings.setDefaultSmsApp(getString(R.string.default_sms_app));
        }
        mUserDisableAcs = mRcsSettings.isUserDisableAcs();
        mUserProvisioningAddr = mRcsSettings.getUserProvisioningAddr();

        mCurrentUserAccount = LauncherUtils.getCurrentUserAccount(mContext);
        mLastUserAccount = LauncherUtils.getLastUserAccount(mContext);
        if (sLogger.isActivatedDebug()) {
            String logLastUserAccount = (mLastUserAccount == null)
                ? "null" : mLastUserAccount.replaceAll(Logger.sLogNumPattern, "*");
            String logCurrentUserAccount = (mCurrentUserAccount == null)
                ? "null" : mCurrentUserAccount.replaceAll(Logger.sLogNumPattern, "*");
            sLogger.info("Last user account:" + logLastUserAccount
                         + ",Current user account:" + logCurrentUserAccount
                         + ",versionCode:" + LauncherUtils.getVersionCode(mContext));
        }

        if (mCurrentUserAccount == null) {
            if (!isFirstLaunch()) {
                /* If it is not the first launch then set the user account ID from the last used IMSI */
                mCurrentUserAccount = mLastUserAccount;
            }
        } else {
            if (!mCurrentUserAccount.equals(mLastUserAccount)) {
                LauncherUtils.resetRcsConfig(mContext, mRcsSettings);
                mRcsSettings.setProvisioningToken(null);
                LauncherUtils.saveProvisioningValidity(mContext, 1);
                mRcsSettings.setUserProfileImsUserName(null);
                onConfigChange(AcsConfigInfo.PRE_CONFIGURATION,
                               Version.RESETED.toInt());
            }

            mLastUserAccount = mCurrentUserAccount;
            LauncherUtils.setLastUserAccount(mContext, mLastUserAccount);
        }

        String release = android.os.Build.VERSION.RELEASE;
        String lastRelease = mRcsSettings.getLastBuildVersionRelease();
        if ((lastRelease == null) || !release.equals(lastRelease)) {
            mRcsSettings.setLastBuildVersionRelease(release);
            LauncherUtils.resetRcsConfig(mContext, mRcsSettings);
            mRcsSettings.setUserProfileImsUserName(null);
            onConfigChange(AcsConfigInfo.PRE_CONFIGURATION,
                           Version.RESETED.toInt());
        }

        mSettingsObserver = new SettingsObserver(mContext, mHandler);
        registerSettingsObserver();
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        TelephonyManager tm = TelephonyManager.from(mContext).createForSubscriptionId(subId);
        mIsRoaming = tm.isNetworkRoaming();
        tm.listen(mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE);
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("release:" + release + ",lastRelease:" + lastRelease
                          + ",roaming:" + mIsRoaming);
        }
    }

    /**
     * Describe <code>onStartCommand</code> method here.
     *
     * @param intent an <code>Intent</code> value
     * @param n an <code>int</code> value
     * @param n1 an <code>int</code> value
     * @return an <code>int</code> value
     */
    public final int onStartCommand(final Intent intent, final int n, final int n1) {
        boolean logActivated = sLogger.isActivated();
        if (logActivated) {
            sLogger.info("onStartCommand");
        }
        if (mHttpsProvisioningMng != null) {
            return START_STICKY;
        }

        boolean first = false;
        boolean user = false;
        if (intent != null) {
            first = intent.getBooleanExtra(FIRST_KEY, false);
            user = intent.getBooleanExtra(USER_KEY, false);
        }

        int version = mRcsSettings.getProvisioningVersion();
        if (Version.RESETED.toInt() == version) {
            first = true;
            mFirstLaunch = true;
        }

        registerReceiver(mRetryReceiver, new IntentFilter(ACTION_RETRY),
                         "com.mediatek.rcs.permission.ACCESS_ACS_SERVICE", null);
        TelephonyManager tm = (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
        String imsi = tm.getSubscriberId();
        if (imsi == null) {
            /*
             * IMSI may be null if SIM card is not present or Telephony manager is not fully
             * initialized and it is not the first launch. We should then consider the last user
             * account.
             */
            imsi = LauncherUtils.getLastUserAccount(mContext);
        }
        String imei = tm.getDeviceId();
        if (DeviceUtils.isTestSim()) {
            if (imsi == null || "".equals(imsi)) {
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("test sim, imsi is " + imsi);
                }
            }

            if (imei == null || "".equals(imei)) {
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("test sim, imei is " + imei);
                }
            }
        }

        mHttpsProvisioningMng = new HttpsProvisioningManager(imei,
                                                             imsi,
                                                             mContext,
                                                             mRetryIntent,
                                                             first,
                                                             user,
                                                             mRcsSettings);
        mHttpsProvisioningMng.initialize();
        int simState = tm.getSimState();
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("Provisioning (first=" + first + ") (user=" + user + ") (version="
                          + version + ")" + ",simState:" + simState);
        }

        if (simState != TelephonyManager.SIM_STATE_READY) {
            return START_STICKY;
        }

        if (isAcsSupported()) {
            mHandler.obtainMessage(EVENT_INTERNAL_REQUEST, true).sendToTarget();
        }

        /*
         * We want this service to continue running until it is explicitly stopped, so return
         * sticky.
         */
        return START_STICKY;
    }

    /**
     * Describe <code>onBind</code> method here.
     *
     * @param intent an <code>Intent</code> value
     * @return an <code>IBinder</code> value
     */
    @Override
    public final IBinder onBind(final Intent intent) {
        if (sLogger.isActivated()) {
            sLogger.info("onBind");
        }
        return mBinder;
    }

    /**
     * Describe <code>onRebind</code> method here.
     *
     * @param intent an <code>Intent</code> value
     */
    @Override
    public final void onRebind(final Intent intent) {
        if (sLogger.isActivated()) {
            sLogger.info("onRebind");
        }
    }

    /**
     * Describe <code>onUnbind</code> method here.
     *
     * @param intent an <code>Intent</code> value
     * @return a <code>boolean</code> value
     */
    @Override
    public final boolean onUnbind(final Intent intent) {
        if (sLogger.isActivated()) {
            sLogger.info("onUnbind");
        }
        return true;
    }

    @Override
    public void onDestroy() {
        if (sLogger.isActivated()) {
            sLogger.info("onDestroy");
        }
        if (mHttpsProvisioningMng != null) {
            mHttpsProvisioningMng.unregisterSmsProvisioningReceiver();
            mHttpsProvisioningMng.quitProvisioningOperation();
        }

        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        int phoneId = SubscriptionManager.getPhoneId(subId);
        ImsManager imsMgr = ImsManager.getInstance(mContext, phoneId);
        imsMgr.removeRegistrationListener(mRegistrationCallback);

        cancelRetryAlarm(this, mRetryIntent);
        try {
            unregisterReceiver(mRetryReceiver);
            unregisterReceiver(mBroadcastReceiver);
        } catch (IllegalArgumentException e) {
            /* Nothing to be handled here */
        }
        TelephonyManager tm = TelephonyManager.from(mContext).createForSubscriptionId(subId);
        tm.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        mSettingsObserver.unobserve();
        mIsServiceDestroyed.set(true);

        super.onDestroy();
        Process.killProcess(Process.myPid());
    }


    /**
     * Start retry alarm
     *
     * @param context the application context
     * @param intent the pending intent to execute when alarm is raised
     * @param delay delay in milliseconds
     */
    public static void startRetryAlarm(Context context, PendingIntent intent, long delay) {
        long now = System.currentTimeMillis();
        if (sRetryAlarmTime > 0 && (Math.abs(now + delay - sRetryAlarmTime) < 30000)) {
            if (sLogger.isActivatedDebug()) {
                sLogger.info("warning, startRetryAlarm last interval < 30s, sRetryAlarmTime:"
                             + sRetryAlarmTime + ",now:" + now + ", delay:" + delay);
            }
            //return;
        }

        cancelRetryAlarm(context, intent);
        sRetryAlarmTime = now + delay;

        if (sLogger.isActivatedDebug()) {
            sLogger.debug("Retry HTTP configuration update in "
                          + DateUtils.formatElapsedTime(delay / 1000));
        }
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        TimerUtils.setExactTimer(am, System.currentTimeMillis() + delay, intent);
    }

    /**
     * Cancel retry alarm
     *
     * @param context the application context
     * @param intent the pending intent to cancel
     */
    public static void cancelRetryAlarm(Context context, PendingIntent intent) {
        sRetryAlarmTime = 0;
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("Stop retry configuration update");
        }
        AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        am.cancel(intent);
    }

    /**
     * Retry receiver
     */
    private BroadcastReceiver mRetryReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, final Intent intent) {
            boolean resetCounter =
                intent.getBooleanExtra(HttpsProvisioningUtils.RESET_CONFIG_SUFFIX, false);
            if (sLogger.isActivated()) {
                sLogger.debug("mRetryReceiver receives, reProvisioning:" + resetCounter);
            }
            if (resetCounter && (mHttpsProvisioningMng != null)) {
                mHttpsProvisioningMng.resetCounters();
            }

            sRetryAlarmTime = 0;
            mHandler.obtainMessage(EVENT_INTERNAL_REQUEST, false).sendToTarget();
        }
    };

    /**
     * Re-provision
     *
     * @param ctx the application context
     */
    public static void reProvisioning(Context ctx) {
        if (sLogger.isActivated()) {
            sLogger.debug("Request re-provisioning");
        }
        Intent intent = new Intent(ACTION_RETRY);
        intent.putExtra(HttpsProvisioningUtils.RESET_CONFIG_SUFFIX, true);
        ctx.sendBroadcast(intent, "com.mediatek.rcs.permission.ACCESS_ACS_SERVICE");
        onConfigChange(AcsConfigInfo.PRE_CONFIGURATION,
                       Version.RESETED.toInt());
    }

    /**
     * Is the first ACS is launched ?
     *
     * @return true if it's the first time ACS is launched
     */
    private boolean isFirstLaunch() {
        return mFirstLaunch;
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                boolean logActivated = sLogger.isActivated();
                if (logActivated) {
                    sLogger.info("onReceive:action=" + action);
                }
                if ((TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED).equals(action)) {
                    handleSimStateChanged(intent);
                } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                    boolean airplaneModeOn = intent.getBooleanExtra("state", false) ? true : false;
                    if (logActivated) {
                        sLogger.info("ACTION_AIRPLANE_MODE_CHANGED, enabled = " + airplaneModeOn);
                    }

                    if (!airplaneModeOn) {
                        if (mHttpsProvisioningMng != null) {
                            mHttpsProvisioningMng.tryReleaseNetwork();
                        }

                        TelephonyManager tm =
                            (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
                        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
                        if (tm.isImsRegistered(subId)) {
                            if (logActivated) {
                                sLogger.info("ims already registered, ignore");
                            }
                            return;
                        }

                        if ((mHttpsProvisioningMng != null)
                             && mHttpsProvisioningMng.isRetryingOrInNoMoreRetryState()) {
                            if (logActivated) {
                                sLogger.info("isRetryingOrInNoMoreRetryState, ignore");
                            }
                            return;
                        }

                        int version = mRcsSettings.getProvisioningVersion();
                        if (version < 0) {
                            return;
                        }

                        mHandler.obtainMessage(EVENT_INTERNAL_REQUEST, true).sendToTarget();
                    }
                }
            }
        };

    private final ImsMmTelManager.RegistrationCallback mRegistrationCallback
            = new ImsMmTelManager.RegistrationCallback() {
            @Override
            public final void onRegistered(int imsRadioTech) {
                if (sLogger.isActivated()) {
                    sLogger.info("onRegistered imsRadioTech:" + imsRadioTech
                                 + ",roaming:" + mIsRoaming);
                }

                int subId = SubscriptionManager.getDefaultDataSubscriptionId();
                int phoneId = SubscriptionManager.getPhoneId(subId);
                ImsManager imsMgr = ImsManager.getInstance(mContext, phoneId);
                imsMgr.removeRegistrationListener(mRegistrationCallback);

                mHandler.obtainMessage(EVENT_IMS_REGISTERED, false).sendToTarget();
            }
        };


    private void handleSimStateChanged(Intent intent) {
        int simStatus = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                                           TelephonyManager.SIM_STATE_UNKNOWN);
        int phoneId = intent.getIntExtra("phone", SubscriptionManager.INVALID_PHONE_INDEX);
        int defaultSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        int defaultPhoneId = SubscriptionManager.getPhoneId(defaultSubId);
        if (sLogger.isActivated()) {
            sLogger.info("intent:" + intent + ",simStatus:" + simStatus
                         + ",phoneId:" + phoneId + ",defaultPhoneId:" + defaultPhoneId);
        }

        if ((phoneId != defaultPhoneId)
            && (SubscriptionManager.isValidPhoneId(defaultPhoneId))) {
            return;
        }

        if ((simStatus == TelephonyManager.SIM_STATE_ABSENT)
            || (simStatus == TelephonyManager.SIM_STATE_NOT_READY)) {
            if (sLogger.isActivated()) {
                sLogger.info("SIM removed");
            }
            mRcsSettings.setUserProfileImsUserName(null);
            mWaitMSISDN = false;
            cancelRetryAlarm(this, mRetryIntent);
        } else if (simStatus == TelephonyManager.SIM_STATE_LOADED) {
            TelephonyManager tm =
                (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
            String imsi = tm.getSubscriberId();
            if (imsi == null) {
                imsi = LauncherUtils.getLastUserAccount(mContext);
            }
            String imei = tm.getDeviceId();
            if (mHttpsProvisioningMng != null) {
                mHttpsProvisioningMng.updateSimInfo(imsi, imei);
            }

            mCurrentUserAccount = LauncherUtils.getCurrentUserAccount(mContext);
            if (sLogger.isActivatedDebug()) {
                String logCurrentUserAccount = (mCurrentUserAccount == null)
                    ? "null" : mCurrentUserAccount.replaceAll(Logger.sLogNumPattern, "*");
                String logLastUserAccount = (mLastUserAccount == null)
                    ? "null" : mLastUserAccount.replaceAll(Logger.sLogNumPattern, "*");
                sLogger.debug("SIM loaded, mCurrentUserAccount:" + logCurrentUserAccount
                              + ",mLastUserAccount:" + logLastUserAccount);
            }
            if ((mLastUserAccount != null) && !mLastUserAccount.equals(mCurrentUserAccount)) {
                // new sim inserted
                mLastUserAccount = mCurrentUserAccount;
                LauncherUtils.setLastUserAccount(mContext, mLastUserAccount);
                LauncherUtils.resetRcsConfig(mContext, mRcsSettings);
                mRcsSettings.setProvisioningToken(null);
                mRcsSettings.setServiceActivationState(true);
                mRcsSettings.setUserProfileImsUserName(null);
                onConfigChange(AcsConfigInfo.PRE_CONFIGURATION,
                               Version.RESETED.toInt());
                if (mHttpsProvisioningMng != null) {
                    mHttpsProvisioningMng.resetCounters();
                }
                cancelRetryAlarm(this, mRetryIntent);
                mHandler.obtainMessage(EVENT_INTERNAL_REQUEST, false).sendToTarget();
            } else {
                // same sim
                mHandler.obtainMessage(EVENT_INTERNAL_REQUEST, true).sendToTarget();
            }
        }
    }

    private boolean triggerRequest(int action, boolean first, boolean user) {
        final boolean logActivated = sLogger.isActivated();
        if (logActivated) {
            sLogger.debug("triggerRequest action:" + action);
        }

        if (!isSimSupported(mContext)) {
            if (logActivated) {
                sLogger.warn("current sim is not support, ignore...");
            }
            return false;
        }

        if (!preCheckTrigger(first, user)) {
            return false;
        }

        if (mHttpsProvisioningMng == null) {
            if (logActivated) {
                sLogger.debug("mHttpsProvisioningMng is still null, ignore request...");
            }
            return false;
        }

        if (RcsSettings.ATT_SUPPORT) {
            TelephonyManager tm = (TelephonyManager)mContext.getSystemService(Context.TELEPHONY_SERVICE);
            int subId = SubscriptionManager.getDefaultDataSubscriptionId();
            if (logActivated) {
                sLogger.debug("current network type:" + NetworkUtils.getNetworkAccessType(mContext)
                              + ",isImsRegistered:" + tm.isImsRegistered(subId)
                              + ",isVolteAvailable:" + tm.isVolteAvailable()
                              + ",isWifiCallingAvailable:" + tm.isWifiCallingAvailable());
            }

            if (!tm.isImsRegistered(subId)) {
                int phoneId = SubscriptionManager.getPhoneId(subId);
                ImsManager imsMgr = ImsManager.getInstance(mContext, phoneId);
                try {
                    imsMgr.removeRegistrationListener(mRegistrationCallback);
                    imsMgr.addRegistrationCallback(mRegistrationCallback);
                } catch (ImsException e) {
                    e.printStackTrace();
                }

                if (logActivated) {
                    sLogger.debug("current ims not registered, phoneId:" + phoneId + ",ignore");
                }
                return true;
            }

            if (tm.isNetworkRoaming()) {
                if (!getDataRoamingEnabled()) {
                    if (sLogger.isActivatedDebug()) {
                        sLogger.info("ims registered, roaming, setting disabled, ignore");
                    }
                    return false;
                }
            }
        }

        boolean result = true;
        switch (action) {
        case 0:
            if (logActivated) {
                sLogger.debug("Request HTTP configuration update");
            }
            final long now = System.currentTimeMillis();
            long passedTime = now - sLastTriggerTime;
            if (sRetryAlarmTime > 0 && (Math.abs(now - sRetryAlarmTime) < 30000)
                || (passedTime < 30000)) {
                if (logActivated) {
                    sLogger.info("triggerRequest last interval < 30s, ignore...");
                }

                if (user) {
                    long delay = (passedTime < 30000) ? (30000 - passedTime) : 0;
                    if (logActivated) {
                        sLogger.info("by user, restart retry alarm " + delay);
                    }
                    startRetryAlarm(mContext, mRetryIntent, delay);
                    result = true;
                } else {
                    result = false;
                }
                break;
            }

            if (!isSimSupported(mContext)) {
                if (logActivated) {
                    sLogger.warn("current sim is not support, ignore...");
                }

                result = false;
                break;
            }

            sLastTriggerTime = now;
            mHttpsProvisioningMng.scheduleProvisioningOperation(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            // Send default connection event
                            if (mHttpsProvisioningMng
                                .connectionEvent(ConnectivityManager.CONNECTIVITY_ACTION)) {
                                if (sLogger.isActivatedDebug()) {
                                    sLogger.debug("connectionEvent complete");
                                }
                            }
                            // } catch (RcsAccountException e) {
                            //     /**
                            //      * This is a non revocable use-case as the RCS account itself was not
                            //      * created, So we log this as error and stop the service itself.
                            //      */
                            //     sLogger.error("Failed to handle connection event!", e);
                            //     stopSelf();
                        } catch (RuntimeException e) {
                            /*
                             * Normally we are not allowed to catch runtime exceptions as these are
                             * genuine bugs which should be handled/fixed within the code. However the
                             * cases when we are executing operations on a thread unhandling such
                             * exceptions will eventually lead to exit the system and thus can bring the
                             * whole system down, which is not intended.
                             */
                            sLogger.error("Unable to handle connection event!", e);
                        } catch (IOException e) {
                            if (logActivated) {
                                sLogger.debug("Unable to handle connection event, Message="
                                              + e.getMessage());
                            }
                            /* Start the RCS service */
                            if (mHttpsProvisioningMng.isFirstProvisioningAfterBoot()) {
                                /* Reason: No configuration present */
                                if (logActivated) {
                                    sLogger.debug("Initial provisioning failed!");
                                }
                                mHttpsProvisioningMng
                                    .provisioningFails(ProvisioningFailureReasons.CONNECTIVITY_ISSUE);
                                mHttpsProvisioningMng.retry();
                            }
                            // else {
                            //     mHttpsProvisioningMng.tryLaunchRcsCoreService(mContext, -1);
                            // }
                        }
                    }
                });
            break;

        case 1:
            cancelRetryAlarm(this, mRetryIntent);
            break;

        default:
            break;
        }

        return result;
    }

    private boolean preCheckTrigger(boolean first, boolean user) {
        boolean logActivated = sLogger.isActivatedDebug();
        boolean requestConfig = false;
        int version = mRcsSettings.getProvisioningVersion();
        if (logActivated) {
            sLogger.debug("first:" + first + ",user:" + user + ",version:" + version);
        }

        if (!isAcsSupported()) {
            return false;
        }

        ContactId contact = mRcsSettings.getUserProfileImsUserName();
        if (contact == null) {
            mWaitMSISDN = true;
            sLogger.debug("waiting for msisdn...");
            return false;
        }

        if (TermsAndConditionsResponse.DECLINED == mRcsSettings.getTermsAndConditionsResponse()) {
            if (logActivated) {
                sLogger.debug("Do not request configuration since TC were declined!");
            }
        } else if (first) {
            requestConfig = true;
        } else if (Version.RESETED.toInt() == version) {
            requestConfig = true;
        } else if (Version.RESETED_NOQUERY.toInt() == version) {
            // Nothing to do
        } else if (Version.DISABLED_NOQUERY.toInt() == version) {
            if (user) {
                requestConfig = true;
            }
        } else if (Version.DISABLED_DORMANT.toInt() == version && user) {
            requestConfig = true;
        } else { // version > 0
            long expiration = LauncherUtils.getProvisioningExpirationDate(this);
            if (expiration <= 0) {
                requestConfig = true;
            } else {
                long now = System.currentTimeMillis();
                long validity = LauncherUtils.getProvisioningValidity(this);
                if ((expiration - validity / 5) <= now) {
                    if (logActivated) {
                        sLogger.debug("Configuration validity expired at "
                            .concat(DateUtils.formatDateTime(mContext,
                                                             expiration,
                                                             DateUtils.FORMAT_SHOW_DATE
                                                                 | DateUtils.FORMAT_SHOW_TIME
                                                                 | DateUtils.FORMAT_NUMERIC_DATE)));
                    }
                    requestConfig = true;
                } else {
                    if (!RcsSettings.ATT_SUPPORT) {
                        if (TermsAndConditionsResponse.NO_ANSWER == mRcsSettings
                            .getTermsAndConditionsResponse()) {
                            TermsAndConditionsRequest.showTermsAndConditions(mContext);
                        }
                    }

                    if (user) {
                        requestConfig = true;
                    }

                    if (logActivated) {
                        sLogger.debug("Configuration will expire at "
                            + DateUtils.formatDateTime(mContext,
                                                       expiration,
                                                       DateUtils.FORMAT_SHOW_DATE
                                                           | DateUtils.FORMAT_SHOW_TIME
                                                           | DateUtils.FORMAT_NUMERIC_DATE));
                    }
                }
            }
        }

        if (logActivated) {
            sLogger.debug("preCheckTrigger, requestConfig:" + requestConfig);
        }
        return requestConfig;
    }

    private static boolean isAirplaneModeOn(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                                      Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    private static int getSupportedCarrier(Context ctx, String plmn) {
        if (plmn == null || plmn.isEmpty()) {
            return 0;
        }

        int mccmnc = Integer.parseInt(plmn);
        String[] opList = ctx.getResources().getStringArray(R.array.operator_list);

        for (String op : opList) {
            String[] plmns = op.split(",");
            int min = Integer.parseInt(plmns[0].trim());
            int max = Integer.parseInt(plmns[1].trim());
            if (mccmnc >= min && mccmnc <= max) {
                if (sLogger.isActivated()) {
                    sLogger.debug("getSupportedCarrier,return:"
                                  + Integer.parseInt(plmns[2].trim()));
                }
                return Integer.parseInt(plmns[2].trim());
            }
        }

        return 0;
    }

    private static boolean isSimSupported(Context ctx) {
        TelephonyManager telMgr =
            (TelephonyManager)ctx.getSystemService(Context.TELEPHONY_SERVICE);
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        int phoneId = SubscriptionManager.getPhoneId(subId);
        int simState = telMgr.getSimState(phoneId);
        if (simState == TelephonyManager.SIM_STATE_READY) {
            String plmn = telMgr.getSimOperator();
            if (getSupportedCarrier(ctx, plmn) > 0) {
                return true;
            }
        }

        return false;
    }

    private static boolean isAcsSupported() {
        boolean supported = true;
        if (TEST_MODE_ENABLED && mUserDisableAcs) {
            supported = false;
            if (sLogger.isActivated()) {
                sLogger.debug("isAcsSupported, user disable");
            }
        }

        if (SystemProperties.getInt("persist.vendor.mtk_acs_support", 1) != 1) {
            supported = false;
        }

        if (sLogger.isActivated()) {
            sLogger.debug("isAcsSupported:" + supported);
        }
        return  supported;
    }

    private void onRoamingOrSettingsChanged(int message) {
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("dataRoamingSettingsChanged:" + message
                          + ",roaming:" + mIsRoaming
                          + ",roaming setting:" + getDataRoamingEnabled());
        }

        switch (message) {
            case EVENT_ROAMING_CHANGE:
                if (!mIsRoaming) {
                    triggerRequest(0, false, false);
                } else {
                    if (!getDataRoamingEnabled()) {
                        if (mHttpsProvisioningMng != null) {
                            mHttpsProvisioningMng.tryReleaseNetwork();
                        }
                    }
                }
                break;

            case EVENT_ROAMING_SETTING_CHANGE:
                if (getDataRoamingEnabled()) {
                    if (mIsRoaming) {
                        triggerRequest(0, false, false);
                    }
                } else {
                    if (mIsRoaming) {
                        if (mHttpsProvisioningMng != null) {
                            mHttpsProvisioningMng.tryReleaseNetwork();
                        }
                    }
                }
                break;

            default:
                break;
        }
    }

    private boolean getDataRoamingEnabled() {
        boolean isDataRoamingEnabled;
        ContentResolver resolver = mContext.getContentResolver();
        String subId = "";
        // For single SIM phones, this is a per phone property.
        if (TelephonyManager.getDefault().getSimCount() > 1) {
            subId = String.valueOf(SubscriptionManager.getDefaultDataSubscriptionId());
        }
        isDataRoamingEnabled = Settings.Global.getInt(resolver,
            Settings.Global.DATA_ROAMING + subId, 0) != 0;

        if (sLogger.isActivatedDebug()) {
            sLogger.debug("getDataRoamingEnabled: subId=" + subId
                          + ",isDataRoamingEnabled=" + isDataRoamingEnabled);
        }

        return isDataRoamingEnabled;
    }

    private Handler mHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("receive msg:" + msg.what);
                }
                switch (msg.what) {
                    case EVENT_ROAMING_CHANGE:
                    case EVENT_ROAMING_SETTING_CHANGE:
                        onRoamingOrSettingsChanged(msg.what);
                        break;

                    case EVENT_SWITCH_RCS_STATE:
                    case EVENT_USER_TRIGGER_REQUEST:
                        triggerRequest(0, false, true);
                        break;

                    case EVENT_INTERNAL_REQUEST:
                    case EVENT_IMS_REGISTERED:
                        boolean needRetry = false;
                        if ((msg.obj != null) && (msg.obj instanceof Boolean)) {
                            needRetry = ((Boolean)msg.obj).booleanValue();
                        }
                        sLogger.debug("handle msg needRetry:" + needRetry);
                        if (!triggerRequest(0, false, false) && needRetry) {
                            long expiration = LauncherUtils.getProvisioningExpirationDate(mContext);
                            long validity = LauncherUtils.getProvisioningValidity(mContext);
                            long now = System.currentTimeMillis();
                            long delay = (expiration - validity / 5) - now;
                            if (delay < 0) {
                                // if delay < 0, retry immediately
                                delay = 0;
                            }
                            if (mRcsSettings.getProvisioningVersion() > 0) {
                                if (sLogger.isActivated()) {
                                    sLogger.info("restart retry alarm");
                                }
                                startRetryAlarm(mContext, mRetryIntent, delay);
                            }
                        }
                        break;

                    case EVENT_USER_CLEAR_CONFIGURATION:
                        LauncherUtils.resetRcsConfig(mContext, mRcsSettings);
                        mRcsSettings.setProvisioningToken(null);
                        LauncherUtils.saveProvisioningValidity(mContext, 1);
                        cancelRetryAlarm(mContext, mRetryIntent);
                        onConfigChange(AcsConfigInfo.PRE_CONFIGURATION, Version.RESETED.toInt());
                        break;

                    case EVENT_USER_SET_SWITCH_STATE:
                        if ((msg.obj != null) && (msg.obj instanceof Boolean)) {
                            mUserDisableAcs = !((Boolean)msg.obj).booleanValue();
                            mRcsSettings.setUserDisableAcs(mUserDisableAcs);
                            if (sLogger.isActivatedDebug()) {
                                sLogger.debug("user set swith state:" + (Boolean)msg.obj);
                            }
                        }
                        break;

                    case EVENT_USER_SET_PROVISIONING_ADDR:
                        mUserProvisioningAddr = (String)msg.obj;
                        mRcsSettings.setUserProvisioningAddr(mUserProvisioningAddr);
                        if (sLogger.isActivatedDebug()) {
                            sLogger.debug("user set url:" + mUserProvisioningAddr);
                        }
                        break;

                    case EVENT_SET_MSISDN:
                        if ((msg.obj != null) && (msg.obj instanceof String)) {
                            onSetMsisdn((String)msg.obj);
                        }
                        break;

                    default:
                        break;
                }
            }
        };

    private void registerSettingsObserver() {
        mSettingsObserver.unobserve();
        String subId = "";
        if (TelephonyManager.getDefault().getSimCount() > 1) {
            subId = String.valueOf(SubscriptionManager.getDefaultDataSubscriptionId());
        }
        mSettingsObserver.observe(
            Settings.Global.getUriFor(Settings.Global.DATA_ROAMING + subId),
            EVENT_ROAMING_SETTING_CHANGE);

        if (sLogger.isActivatedDebug()) {
            sLogger.debug("register Settings Observer");
        }
    }

    private class SettingsObserver extends ContentObserver {
        private final Map<Uri, Integer> mUriEventMap;
        private final Context mContext;
        private final Handler mHandler;
        public SettingsObserver(Context context, Handler handler) {
            super(null);
            mUriEventMap = new HashMap<>();
            mContext = context;
            mHandler = handler;
        }

        public void observe(Uri uri, int what) {
            mUriEventMap.put(uri, what);
            final ContentResolver resolver = mContext.getContentResolver();
            resolver.registerContentObserver(uri, false, this);
        }

        public void unobserve() {
            final ContentResolver resolver = mContext.getContentResolver();
            resolver.unregisterContentObserver(this);
        }

        @Override
        public void onChange(boolean selfChange) {
            if (sLogger.isActivatedDebug()) {
                sLogger.debug("Should never be reached.");
            }
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            final Integer what = mUriEventMap.get(uri);
            if (sLogger.isActivatedDebug()) {
                sLogger.debug("onChange: what" + what + ",selfChange:" + selfChange);
            }

            if (what != null) {
                mHandler.obtainMessage(what.intValue()).sendToTarget();
            } else {
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("No matching event to send for URI=" + uri);
                }
            }
        }
    }

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
            @Override
            public void onServiceStateChanged(ServiceState serviceState) {
                super.onServiceStateChanged(serviceState);
                if (sLogger.isActivatedDebug()) {
                    sLogger.debug("onServiceStateChanged, roaming:" + serviceState.getRoaming()
                                  + ",mRoaming:" + mIsRoaming);
                }

                if (serviceState.getRoaming()) {
                    if (!mIsRoaming) {
                        mIsRoaming = true;
                        mHandler.obtainMessage(EVENT_ROAMING_CHANGE).sendToTarget();
                    }
                } else {
                    if (mIsRoaming) {
                        mIsRoaming = false;
                        mHandler.obtainMessage(EVENT_ROAMING_CHANGE).sendToTarget();
                    }
                }
            }
        };

    private void onSetMsisdn(String msisdn) {
        if (msisdn != null) {
            ContactId newContact = new ContactId(msisdn);
            ContactId contact = mRcsSettings.getUserProfileImsUserName();
            if (!newContact.equals(contact)) {
                mRcsSettings.setUserProfileImsUserName(newContact);
                if (sLogger.isActivatedDebug()) {
                    sLogger.info("msisdn change:wait msisdn:" + mWaitMSISDN);
                }
                if (mWaitMSISDN) {
                    mWaitMSISDN = false;
                    triggerRequest(0, false, false);
                }
            }
        }
    }
}
