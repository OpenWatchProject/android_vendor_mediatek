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

package com.mediatek.internal.telephony.dataconnection;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.NetworkConfig;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.NetworkStats;
import android.net.NetworkUtils;
import android.net.ProxyInfo;
import android.net.TrafficStats;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.provider.Telephony;
import android.provider.Telephony.Carriers;
import android.telephony.AccessNetworkConstants;
import android.telephony.AccessNetworkConstants.TransportType;
import android.telephony.CarrierConfigManager;
import android.telephony.DataFailCause;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.telephony.data.ApnSetting.ApnType;
import android.telephony.data.DataProfile;
import android.telephony.data.DataService;
import android.text.TextUtils;
import android.util.EventLog;
import android.util.LocalLog;
import android.util.Pair;

import android.telephony.PcoData;

import com.android.internal.R;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.CommandException.Error;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.DctConstants;
import com.android.internal.telephony.EventLogTags;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.ServiceStateTracker;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyDevController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.dataconnection.ApnSettingUtils;
import com.android.internal.telephony.dataconnection.DataConnection.ConnectionParams;
import com.android.internal.telephony.dataconnection.DataConnection;
import com.android.internal.telephony.dataconnection.DataConnectionReasons;
import com.android.internal.telephony.dataconnection.DataConnectionReasons.DataAllowedReasonType;
import com.android.internal.telephony.dataconnection.DataConnectionReasons.DataDisallowedReasonType;
import com.android.internal.telephony.dataconnection.DataEnabledSettings;
import com.android.internal.telephony.dataconnection.DataEnabledSettings.DataEnabledChangedReason;
import com.android.internal.telephony.dataconnection.DcController;
import com.android.internal.telephony.dataconnection.DcTesterFailBringUpAll;
import com.android.internal.telephony.dataconnection.DcTracker;
import com.android.internal.telephony.metrics.TelephonyMetrics;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.RuimRecords;
import com.android.internal.telephony.uicc.SIMRecords;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.util.ArrayUtils;
import com.android.internal.util.AsyncChannel;

import com.mediatek.common.carrierexpress.CarrierExpressManager;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkHardwareConfig;
import com.mediatek.internal.telephony.MtkDctConstants;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.internal.telephony.MtkServiceStateTracker;
import com.mediatek.internal.telephony.MtkSubscriptionController;
import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.RadioManager;
import com.mediatek.internal.telephony.uicc.MtkIccUtilsEx;
import com.mediatek.internal.telephony.uicc.MtkUiccCardApplication;
import com.mediatek.internal.telephony.uicc.MtkUiccController;
import com.mediatek.internal.telephony.worldphone.WorldPhoneUtil;
import com.mediatek.internal.telephony.datasub.SmartDataSwitchAssistant;
import com.mediatek.internal.telephony.ims.MtkDedicateDataCallResponse;
import com.mediatek.provider.MtkSettingsExt;

import com.mediatek.telephony.internal.telephony.vsim.ExternalSimManager;

import mediatek.telephony.data.MtkApnSetting;
import mediatek.telephony.MtkCarrierConfigManager;
import mediatek.telephony.MtkServiceState;
import vendor.mediatek.hardware.mtkradioex.V1_0.VendorSetting;

import java.lang.StringBuilder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Objects;
import java.util.PriorityQueue;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

/**
 * {@hide}
 */
public class MtkDcTracker extends DcTracker {
    private static final boolean DBG = true;
    private static final boolean VDBG = Build.IS_ENG;

    // M: [skip data stall] @{
    private static final String SKIP_DATA_STALL_ALARM = "persist.vendor.skip.data.stall.alarm";
    // M: [skip data stall] @}

    private OpTelephonyCustomizationFactoryBase mTelephonyCustomizationFactory = null;
    private IDataConnectionExt mDataConnectionExt = null;

    // M: Data Framework - common part enhancement
    // Sync data settings
    private static final int MOBILE_DATA_IDX        = 0;
    private static final int ROAMING_DATA_IDX       = 1;
    private static final int DEFAULT_DATA_SIM_IDX   = 2;
    private static final int DOMESTIC_DATA_ROAMING_IDX = 3;
    private static final int INTERNATIONAL_DATA_ROAMING_IDX = 4;
    private static final int SKIP_DATA_SETTINGS     = -2;

    // M: IMS E911 Bearer Management
    private int mDedicatedBearerCount = 0;

    public ArrayList <ApnContext> mPrioritySortedApnContextsEx;

    protected ApnSetting mInitialAttachApnSetting;

    private String[] PROPERTY_ICCID = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };

    // M: [LTE][Low Power][UL traffic shaping] @{
    private String mLteAccessStratumDataState = MtkPhoneConstants.LTE_ACCESS_STRATUM_STATE_UNKNOWN;
    private static final int LTE_AS_CONNECTED = 1;
    private int mNetworkType = -1;
    private boolean mIsLte = false;
    private boolean mIsSharedDefaultApn = false;
    private int mDefaultRefCount = 0;
    // M: [LTE][Low Power][UL traffic shaping] @}

    // member variables
    private AtomicReference<UiccCardApplication> mUiccCardApplication;

    // M: Multi-PS attach @{
    private boolean mAllowConfig = false;
    private boolean mHasFetchModemDeactPdnCapabilityForMultiPS = false;
    private boolean mModemDeactPdnCapabilityForMultiPS = false;
    private boolean mHasFetchMdAutoSetupImsCapability = false;
    private boolean mMdAutoSetupImsCapability = false;
    // M: Multi-PS attach @}

    // M: PS/CS concurrent @{
    // True when supporting PS/CS concurrent
    private boolean mIsSupportConcurrent = false;
    // M: PS/CS concurrent @}

    // M: IMS E911 Bearer Management
    private static final int THROTTLING_MAX_PDP_SIZE = 8;

    // M: IA
    // VZW
    /**
     * M: IA- for IMS test mode and change attach APN for OP12.
     *    Enable : Set Attach PDN to VZWINTERNET
     *    Disable: Set Attach PDN to VZWIMS (Default)
     */
    protected static final boolean MTK_IMS_TESTMODE_SUPPORT =
            SystemProperties.getInt("persist.vendor.radio.imstestmode", 0) == 1;

    /* Set to true if IMS pdn handover to WIFI(EPDG) and used for change attach APN */
    private boolean mIsImsHandover = false;
    /* Vzw IMS hand over
       Value:
           0: reset
           1: handover start
           2: handover end
    */
    private ApnSetting mMdChangedAttachApn = null;
    private static final int APN_CLASS_0 = 0;
    private static final int APN_CLASS_1 = 1;
    private static final int APN_CLASS_2 = 2;
    private static final int APN_CLASS_3 = 3;
    private static final int APN_CLASS_4 = 4;
    private static final int APN_CLASS_5 = 5;
    private static final String VZW_EMERGENCY_NI = "VZWEMERGENCY";
    private static final String VZW_IMS_NI = "VZWIMS";
    private static final String VZW_ADMIN_NI = "VZWADMIN";
    private static final String VZW_INTERNET_NI = "VZWINTERNET";
    private static final String VZW_APP_NI = "VZWAPP";
    private static final String VZW_800_NI = "VZW800";
    private static final String PROP_APN_CLASS_ICCID = "vendor.ril.md_changed_apn_class.iccid.";
    private static final String PROP_APN_CLASS = "vendor.ril.md_changed_apn_class.";
    private static final String SPRINT_IA_NI = "otasn";
    private static final String GID1_DEFAULT =
            "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
    // M: For IMS pdn handover to WIFI
    private static final String NETWORK_TYPE_WIFI = "WIFI";
    private static final String NETWORK_TYPE_MOBILE_IMS = "MOBILEIMS";

    // JPN
    protected int mSuspendId = 0;
    protected static final String[] MCC_TABLE_TEST = {
        "001"
    };
    protected static final String[] MCC_TABLE_DOMESTIC = {
        "440"
    };
    protected static final int REGION_UNKNOWN  = 0;
    protected static final int REGION_DOMESTIC = 1;
    protected static final int REGION_FOREIGN  = 2;
    protected int mRegion = REGION_UNKNOWN;
    protected Object mNeedsResumeModemLock = new Object();
    protected boolean mNeedsResumeModem = false;
    // Attach APN is assigned empty but need to raise P-CSCF discovery flag
    // 26201 DTAG D1(T-Mobile)
    // 44010 DOCOMO
    private String[] PLMN_EMPTY_APN_PCSCF_SET = {
        "26201",
        "44010"
    };
    /**
    * Handles changes to the settings of IMS switch db.
    */
    private ContentObserver mImsSwitchChangeObserver  = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            if (DBG) {
                log("mImsSwitchChangeObserver: onChange=" + selfChange);
            }
            if (isOp17IaSupport()) {
                log("IA : OP17, set IA");
                setInitialAttachApn();
            }
        }
    };

    // RJIL
    private String[] MCCMNC_OP18 = {
        "405840", "405854", "405855", "405856", "405857",
        "405858", "405859", "405860", "405861", "405862",
        "405863", "405864", "405865", "405866", "405867",
        "405868", "405869", "405870", "405871", "405872",
        "405873", "405874"
    };
    // M: Query modem hardware capability
    private TelephonyDevController mTelDevController = TelephonyDevController.getInstance();
    // M: IA end

    // M: Google issue, this thread should quit when DcTracker dispose,
    //    otherwise memory leak will happen.
    private HandlerThread mDcHandlerThread;

    /// M: load romaing config from carrier config to cache @{
    private boolean mCcDomesticRoamingEnabled = false;
    private String[] mCcDomesticRoamingSpecifiedNw = null;
    private boolean mCcIntlRoamingEnabled = false;
    private boolean mCcUniqueSettingsForRoaming = false;
    private boolean mCcOneSettingForRoaming = false;
    /// @}
    private boolean mIsAddMnoApnsIntoAllApnList = false;

    private static final String[] PRIVATE_APN_OPERATOR = {
        "732101", "330110", "334020", "71610", "74001",
        "71403", "73003", "72405", "722310", "37002",
        "71203", "70401", "70601", "708001", "71021",
        "74810", "74402"
    };
    private static final String[] KDDI_OPERATOR = {
        "44007", "44008", "44050", "44051", "44052",
        "44053", "44054", "44055", "44056", "44070",
        "44071", "44072", "44073", "44074", "44075",
        "44076", "44077", "44078", "44079", "44088",
        "44089", "44170"
    };

    /// M: Data icon performance enhancement @{
    private static final String PROP_RIL_DATA_GSM_MCC_MNC = "vendor.ril.data.gsm_mcc_mnc";
    private static final String PROP_RIL_DATA_CDMA_MCC_MNC = "vendor.ril.data.cdma_mcc_mnc";
    private static final String PROP_RIL_DATA_GSM_SPN = "vendor.ril.data.gsm_spn";
    private static final String PROP_RIL_DATA_CDMA_SPN = "vendor.ril.data.cdma_spn";
    private static final String PROP_RIL_DATA_GSM_IMSI = "vendor.ril.data.gsm_imsi";
    private static final String PROP_RIL_DATA_CDMA_IMSI = "vendor.ril.data.cdma_imsi";
    private static final String PROP_RIL_DATA_GID1 = "vendor.ril.data.gid1-";
    private static final String PROP_RIL_DATA_ICCID = "vendor.ril.iccid.sim";
    private static final String PROP_RIL_DATA_PNN = "vendor.ril.data.pnn";
    private int mPhoneType = PhoneConstants.PHONE_TYPE_NONE;
    private boolean mIsOperatorNumericEmpty = false;
    private boolean mIsSimNotReady = false;
    private boolean mIsNotifyDataAttached = false;
    private MtkServiceState mTurboSS = null;
    private String mImsiGsm = null;
    private String mImsiCdma = null;
    private boolean mIsRecordsOverride = false;
    /// @}

    // M: Worker Handler @{
    private Handler mWorkerHandler;
    // M: Worker Handler @}

    /// M: Data Retry requirement for Telcel and Telstra operators @{
    private enum DataRetryOperator {
        UNKNOWN,
        TELCEL,
        TELSTRA,
        EE
    };
    private String[] MCCMNC_TELCEL = {"33402", "334020"};
    private String[] MCCMNC_TELSTRA = {"50501"};
    private String[] MCCMNC_EE = {"23430"};
    private DataRetryOperator mDataRetryOperator = DataRetryOperator.UNKNOWN;
    /// @}

    private int mRilRat = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
    private boolean mIsModemReset = false;
    private int mHandoverApnType = ApnSetting.TYPE_NONE;

    //M: [Data Usage Update][Network Latency Optimization] @{
    private volatile NetworkStats mMobileDataUsage = null;
    //M: [Data Usage Update][Network Latency Optimization] @}

    /// M: Disconnect the default PDN from CS when it has mms apn type @{
    MtkDataConnection mDetachedDc = null;
    /// @}

    private final BroadcastReceiver mIntentReceiverEx = new BroadcastReceiver () {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (DBG) log("mIntentReceiverEx onReceive: action=" + action);
            if (action.equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                /// M: Fix CarrierConfigLoader timing issue. @{
                int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                if (DBG) log("mIntentReceiverEx ACTION_CARRIER_CONFIG_CHANGED: subId=" + subId +
                        ", mPhone.getSubId()=" + mPhone.getSubId());
                if (subId == mPhone.getSubId()) {
                    if (DBG) log("CarrierConfigLoader is loading complete!");
                    sendMessage(obtainMessage(MtkDctConstants.EVENT_CARRIER_CONFIG_LOADED));
                    loadCarrierConfig(subId);
                }
                /// @}
            } else if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)) {
                if (false == hasOperatorIaCapability()) {
                    // M:For OP12, in EPDG handover case to change initial attach APN.
                    final NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra(
                            ConnectivityManager.EXTRA_NETWORK_INFO);
                    int apnType = networkInfo.getType();
                    String typeName = networkInfo.getTypeName();
                    logd("onReceive: ConnectivityService action change apnType = " +
                            apnType + " typename =" + typeName);

                    // The case of IMS handover to WIFI
                    // Note: EPDG is implemented on CS framework
                    if (apnType == ConnectivityManager.TYPE_MOBILE_IMS
                            && typeName.equals(NETWORK_TYPE_WIFI)) {
                        onAttachApnChangedByHandover(true);
                    } else if (apnType == ConnectivityManager.TYPE_MOBILE_IMS &&
                            typeName.equals(NETWORK_TYPE_MOBILE_IMS)) {
                        onAttachApnChangedByHandover(false);
                    }
                }
            } else if (action.equals(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED)) {
                reloadOpCustomizationFactory();
            } else if (action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
                if (DBG) log("Wifi state changed");
                final boolean enabled = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
                        WifiManager.WIFI_STATE_UNKNOWN) == WifiManager.WIFI_STATE_ENABLED;
                onWifiStateChanged(enabled);
            } else {
                if (DBG) log("onReceive: Unknown action=" + action);
            }
        }
    };

    //***** Constructor
    public MtkDcTracker(Phone phone, int transportType) {
        super(phone, transportType);

        // M: Since add resource string will change resource id,
        //    using add-on funciton to assign class name of iwlan data service instead
        //    of add resource string.
        mDataServiceManager.setIwlanDataServiceClassName(
                "com.mediatek.internal.telephony.dataconnection.IwlanDataService");

        reloadOpCustomizationFactory();

        if (false == hasOperatorIaCapability()){
            phone.getContext().getContentResolver().registerContentObserver(
                    Settings.Global.getUriFor(Settings.Global.ENHANCED_4G_MODE_ENABLED), true,
                    mImsSwitchChangeObserver);
        }

        IntentFilter filter = new IntentFilter();
        filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        filter.addAction(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED);
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        mPhone.getContext().registerReceiver(mIntentReceiverEx, filter, null, mPhone);

        // special handle for SS + MPS project
        if (TelephonyManager.getDefault().getPhoneCount() == 1) {
            mAllowConfig = true;
        }

        // M: [Data Usage Update][Network Latency Optimization]
        long currentTime = SystemClock.elapsedRealtime();
        mMobileDataUsage = new NetworkStats(currentTime, 1);

        // M: Create Worker Handler @{
        createWorkerHandler();
        // M: Create Worker Handler @}

        /// SSC Mode 3 @{
        setSscMode();
        /// @}
    }

    @Override
    public void registerServiceStateTrackerEvents() {
        super.registerServiceStateTrackerEvents();

        // M: Data on domestic roaming.
        ((MtkServiceStateTracker) mPhone.getServiceStateTracker())
                .registerForDataRoamingTypeChange(this, MtkDctConstants.EVENT_ROAMING_TYPE_CHANGED,
                null);
    }

    @Override
    public void unregisterServiceStateTrackerEvents() {
        super.unregisterServiceStateTrackerEvents();

        // M: Data on domestic roaming.
        ((MtkServiceStateTracker) mPhone.getServiceStateTracker())
                .unregisterForDataRoamingTypeChange(this);
    }

    @Override
    protected void registerForAllEvents() {
        super.registerForAllEvents();

        // M: PS/CS concurrent @{
        mPhone.getCallTracker().unregisterForVoiceCallEnded(this);
        mPhone.getCallTracker().unregisterForVoiceCallStarted(this);
        // M: PS/CS concurrent @}

        logd("registerForAllEvents: mPhone = " + mPhone);

        // M: Data Framework - CC 33
        ((MtkRIL)mPhone.mCi).registerForRemoveRestrictEutran(this,
                MtkDctConstants.EVENT_REMOVE_RESTRICT_EUTRAN, null);

        // M: Data Framework - Data Retry enhancement
        ((MtkRIL)mPhone.mCi).registerForMdDataRetryCountReset(this,
                MtkDctConstants.EVENT_MD_DATA_RETRY_COUNT_RESET, null);

        if (false == hasOperatorIaCapability()) {
            // M: JPN IA
            if (!WorldPhoneUtil.isWorldPhoneSupport() &&
                    !("OP01".equals(SystemProperties.get("persist.vendor.operator.optr")))) {
                ((MtkRIL) mPhone.mCi).setOnPlmnChangeNotification(this,
                        MtkDctConstants.EVENT_REG_PLMN_CHANGED, null);
                ((MtkRIL) mPhone.mCi).setOnRegistrationSuspended(this,
                        MtkDctConstants.EVENT_REG_SUSPENDED, null);
            }
            // M: JPN IA End

            //M: Reset Attach Apn
            ((MtkRIL) mPhone.mCi).registerForResetAttachApn(this,
                    MtkDctConstants.EVENT_RESET_ATTACH_APN, null);

            // M: IA-change attach APN
            ((MtkRIL) mPhone.mCi).registerForAttachApnChanged(this,
                    MtkDctConstants.EVENT_ATTACH_APN_CHANGED, null);
        }
        // M: Reset MD changed APN
        mPhone.mCi.registerForRilConnected(this,
                MtkDctConstants.EVENT_RIL_CONNECTED, null);

        // M: [LTE][Low Power][UL traffic shaping] @{
        ((MtkRIL) mPhone.mCi).registerForLteAccessStratumState(this,
                MtkDctConstants.EVENT_LTE_ACCESS_STRATUM_STATE, null);
        // M: [LTE][Low Power][UL traffic shaping] @}

        // M: Multi-PS Attach Start
        ((MtkRIL) mPhone.mCi).registerForDataAllowed(this,
                MtkDctConstants.EVENT_DATA_ALLOWED, null);
        // M: Multi-PS Attach End

        // M: [VzW] Data Framework @{
        ((MtkRIL) mPhone.mCi).registerForPcoDataAfterAttached(this,
                MtkDctConstants.EVENT_DATA_ATTACHED_PCO_STATUS, null);
        // M: [VzW] Data Framework @}

        /// Ims Data Framework @{
        ((MtkRIL) mPhone.mCi).registerForDedicatedBearerActivated(this,
                MtkDctConstants.EVENT_DEDICATED_BEARER_ACTIVATED, null);
        ((MtkRIL) mPhone.mCi).registerForDedicatedBearerModified(this,
                MtkDctConstants.EVENT_DEDICATED_BEARER_MODIFIED, null);
        ((MtkRIL) mPhone.mCi).registerForDedicatedBearerDeactivationed(this,
                MtkDctConstants.EVENT_DEDICATED_BEARER_DEACTIVATED, null);
        /// @}

        ((MtkRIL) mPhone.mCi).registerForNetworkReject(this,
            MtkDctConstants.EVENT_NETWORK_REJECT, null);
        ((MtkRIL) mPhone.mCi).registerForModemReset(this, MtkDctConstants.EVENT_MODEM_RESET, null);

        ((MtkRIL) mPhone.mCi).registerForMobileDataUsage(this,
            MtkDctConstants.EVENT_MOBILE_DATA_USAGE, null);

        ((MtkRIL) mPhone.mCi).registerForNwLimitState(this,
                MtkDctConstants.EVENT_NW_LIMIT_STATE, null);
    }

    @Override
    protected void registerSettingsObserver() {
        // M: comment super to not listen roaming data switch @{
        // our modem requires that it must first received roaming switch status
        // then activate/deactivate request for some features, the roaming event
        // will be sent by MtkTelephonyAsistent after synced settings.
        // super.registerSettingsObserver();
        // copy super block other code to here, currently only this one:
        mSettingsObserver.unobserve();
        mSettingsObserver.observe(
                Settings.Global.getUriFor(Settings.Global.DEVICE_PROVISIONED),
                DctConstants.EVENT_DEVICE_PROVISIONED_CHANGE);
        // M: comment super to not listen roaming data switch @}

        // M: Register FDN Content Observer @{
        registerFdnContentObserver();
        // M: Register FDN Content Observer @}
    }

    @Override
    public void dispose() {
        super.dispose();

        if (mDataConnectionExt != null) {
            mDataConnectionExt.stopDataRoamingStrategy();
        }

        mPrioritySortedApnContextsEx.clear();

        mPhone.getContext().getContentResolver().unregisterContentObserver(
                mImsSwitchChangeObserver);
        mPhone.getContext().unregisterReceiver(mIntentReceiverEx);
        if (mDcHandlerThread != null) {
            mDcHandlerThread.quitSafely();
            mDcHandlerThread = null;
        }

        // M: Reset FDN related flags @{
        mIsFdnChecked = false;
        mIsMatchFdnForAllowData = false;
        mIsPhbStateChangedIntentRegistered = false;
        mPhone.getContext().unregisterReceiver(mPhbStateChangedIntentReceiver);
        // M: Reset FDN related flags @}

        // M: Quit Worker Handler @{
        if (mWorkerHandler != null) {
            Looper looper = mWorkerHandler.getLooper();
            looper.quit();
        }
        // M: Quit Worker Handler @}
    }

    @Override
    protected void unregisterForAllEvents() {
        super.unregisterForAllEvents();
        logd("unregisterForAllEvents: mPhone = " + mPhone);

        // M: Data Framework - CC 33
        ((MtkRIL)mPhone.mCi).unregisterForRemoveRestrictEutran(this);

        // M: Data Framework - Data Retry enhancement
        ((MtkRIL)mPhone.mCi).unregisterForMdDataRetryCountReset(this);

        if (false == hasOperatorIaCapability()) {
            // M: JPN IA Start
            if (!WorldPhoneUtil.isWorldPhoneSupport() &&
                    !("OP01".equals(SystemProperties.get("persist.vendor.operator.optr")))) {
                ((MtkRIL) mPhone.mCi).unSetOnPlmnChangeNotification(this);
                ((MtkRIL) mPhone.mCi).unSetOnRegistrationSuspended(this);
            }
            // M: JPN IA End
            // M: Reset Attach Apn
            ((MtkRIL) mPhone.mCi).unregisterForResetAttachApn(this);

            // M: IA-change attach APN from modem.
            ((MtkRIL) mPhone.mCi).unregisterForAttachApnChanged(this);
        }
        // M: Reset MD changed APN
        mPhone.mCi.unregisterForRilConnected(this);

        // M: [LTE][Low Power][UL traffic shaping] @{
        ((MtkRIL) mPhone.mCi).unregisterForLteAccessStratumState(this);
        // M: [LTE][Low Power][UL traffic shaping] @}

        // M: Multi-PS Attach Start
        ((MtkRIL) mPhone.mCi).unregisterForDataAllowed(this);
        // M: Multi-PS Attach End

        // M: [VzW] Data Framework
        ((MtkRIL) mPhone.mCi).unregisterForPcoDataAfterAttached(this);

        /// Ims Data Framework @{
        ((MtkRIL) mPhone.mCi).unregisterForDedicatedBearerActivated(this);
        ((MtkRIL) mPhone.mCi).unregisterForDedicatedBearerModified(this);
        ((MtkRIL) mPhone.mCi).unregisterForDedicatedBearerDeactivationed(this);
        /// @}

        ((MtkRIL) mPhone.mCi).unregisterForNetworkReject(this);
        ((MtkRIL) mPhone.mCi).unregisterForModemReset(this);
        ((MtkRIL) mPhone.mCi).unregisterForMobileDataUsage(this);
        ((MtkRIL) mPhone.mCi).unregisterForNwLimitState(this);
    }

    @Override
    protected ApnContext addApnContext(String type, NetworkConfig networkConfig) {
        ApnContext apnContext = new MtkApnContext(mPhone, type, mLogTag, networkConfig, this);
        mApnContexts.put(type, apnContext);
        mApnContextsByType.put(ApnSetting.getApnTypesBitmaskFromString(type), apnContext);
        mPrioritySortedApnContextsEx.add(apnContext);
        return apnContext;
    }

    @Override
    protected void initApnContexts() {
        log("initApnContexts: E");
        // M @{
        if (mPrioritySortedApnContextsEx == null) {
            mPrioritySortedApnContextsEx = new ArrayList<ApnContext>();
        }
        // M @}

        super.initApnContexts();

        // Load device network attributes from resources
        String[] networkConfigStrings = mPhone.getContext().getResources().getStringArray(
                com.android.internal.R.array.networkAttributes);
        for (String networkConfigString : networkConfigStrings) {
            NetworkConfig networkConfig = new NetworkConfig(networkConfigString);
            ApnContext apnContext = null;

            switch (networkConfig.type) {
                // M @{
                case ConnectivityManager.TYPE_MOBILE_WAP:
                    apnContext = addApnContext(MtkPhoneConstants.APN_TYPE_WAP, networkConfig);
                    break;
                case ConnectivityManager.TYPE_MOBILE_XCAP:
                    apnContext = addApnContext(MtkPhoneConstants.APN_TYPE_XCAP, networkConfig);
                    break;
                case ConnectivityManager.TYPE_MOBILE_RCS:
                    apnContext = addApnContext(MtkPhoneConstants.APN_TYPE_RCS, networkConfig);
                    break;
                case ConnectivityManager.TYPE_MOBILE_BIP:
                    apnContext = addApnContext(MtkPhoneConstants.APN_TYPE_BIP, networkConfig);
                    break;
                case ConnectivityManager.TYPE_MOBILE_VSIM:
                    apnContext = addApnContext(MtkPhoneConstants.APN_TYPE_VSIM, networkConfig);
                    break;
                // M @}
                default:
                    log("initApnContexts: skipping unknown type=" + networkConfig.type);
                    continue;
            }
            log("initApnContexts: apnContext=" + apnContext);
        }

        // M @{
        ////The implement of priorityQueue class is incorrect, we sort the list by ourself
        Collections.sort(mPrioritySortedApnContextsEx, new Comparator<ApnContext>() {
            public int compare(ApnContext c1, ApnContext c2) {
                return c2.priority - c1.priority;
            }
        });
        logd("initApnContexts: mPrioritySortedApnContextsEx=" + mPrioritySortedApnContextsEx);
        if (VDBG) log("initApnContexts: X mApnContexts=" + mApnContexts);
        // M @}
    }

    @Override
    protected void onDataConnectionDetached() {
        super.onDataConnectionDetached();
        // Notify the UI with 'DISCONNECTED' state instead of 'UNKNOWN' state.
        mPhone.notifyDataConnection(PhoneConstants.APN_TYPE_DEFAULT);

        /// M: Remove mms capability temporarily when ps is detached @{
        final boolean isMobileDataAlwaysOn = (Settings.Global.getInt(
                mResolver, Settings.Global.MOBILE_DATA_ALWAYS_ON, 0) != 0);
        log("isMobileDataAlwaysOn = " + isMobileDataAlwaysOn);
        if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WWAN && isMobileDataAlwaysOn) {
            ApnContext apnContextDefault = mApnContextsByType.get(ApnSetting.TYPE_DEFAULT);
            if (apnContextDefault != null
                    && apnContextDefault.getState() == DctConstants.State.CONNECTED) {
                ApnSetting apnSetting = apnContextDefault.getApnSetting();
                if (apnSetting != null && apnSetting.canHandleType(ApnSetting.TYPE_MMS)) {
                    ApnContext apnContextMms = mApnContextsByType.get(ApnSetting.TYPE_MMS);
                    if (!apnContextMms.isEnabled()) {
                        MtkDataConnection dataConnection =
                                (MtkDataConnection)apnContextDefault.getDataConnection();
                        if (dataConnection != null) {
                            log("Remove mms capability temporarily when ps is detached");
                            mDetachedDc = dataConnection;
                            mDetachedDc.removeMmsCapability();
                        }
                    }
                }
            }
        }
        /// @}
    }

    @Override
    protected void onDataConnectionAttached() {
        super.onDataConnectionAttached();

        /// M: Add mms capability back when ps is attached @{
        if (mDetachedDc != null) {
            log("Add mms capability back when ps is attached");
            mDetachedDc.addMmsCapability();
            mDetachedDc = null;
        }
        /// @}
    }

    @Override
    public boolean isDataAllowed(ApnContext apnContext, @RequestNetworkType int requestType,
                                 DataConnectionReasons dataConnectionReasons) {
        // Step 1: Get all environment conditions.
        // Step 2: Special handling for emergency APN.
        // Step 3. Build disallowed reasons.
        // Step 4: Determine if data should be allowed in some special conditions.

        DataConnectionReasons reasons = new DataConnectionReasons();

        // Step 1: Get all environment conditions.
        final boolean internalDataEnabled = mDataEnabledSettings.isInternalDataEnabled();
        boolean attachedState = mAttached.get();
        boolean desiredPowerState = mPhone.getServiceStateTracker().getDesiredPowerState();
        boolean radioStateFromCarrier = mPhone.getServiceStateTracker().getPowerStateFromCarrier();
        // TODO: Remove this hack added by ag/641832.
        int dataRat = getDataRat();
        if (dataRat == ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN) {
            desiredPowerState = true;
            radioStateFromCarrier = true;
        }

        /// M: Data icon performance enhancement @{
        boolean bIgnoreRecordLoaded = apnContext != null &&
                (TextUtils.equals(apnContext.getApnType(), PhoneConstants.APN_TYPE_DEFAULT) ||
                TextUtils.equals(apnContext.getApnType(), PhoneConstants.APN_TYPE_IMS));
        boolean recordsLoaded = false;
        if (bIgnoreRecordLoaded) {
            recordsLoaded = true;
        } else {
            recordsLoaded = mIccRecords.get() != null && mIccRecords.get().getRecordsLoaded();
        }
        /// @}

        /// M: CDMA 3G card timing @{
        // CDMA 3G dual mode + CDMA 4G, default data on SIM1.
        // Switch default data to SIM2(1st switch), then switch back to SIM1(2nd switch).
        // In case of poor performance, such as low-end chips.
        // After 2nd switch, SIM1 may get records loaded event triggered by 1st switch,
        // and setup data connection with SIMRecords(instead of RUIMRecords) PLMN.
        // (i.e. 20404 for CT 3G dual mode card.)
        // Thus, when records loaded event triggered by 2nd switch arrieved,
        // (i.e. use RUIMRecords PLMN, 46003 for CT 3G dual mode card.)
        // try setup data will be failed, since data conneciton has been setup with
        // 20404's APN. In other words, 46003's APN can not be used to set up data
        // connection.
        // Wait for next records loaded event triggered by 2st switch.
        if (recordsLoaded && MtkDcHelper.isCdma3GDualModeCard(mPhone.getPhoneId())) {
            if (mIccRecords == null) {
                logd("isDataAllowed: icc records is null.");
                recordsLoaded = false;
            }
            IccRecords curIccRecords = mIccRecords.get();
            MtkServiceStateTracker sst = (MtkServiceStateTracker) mPhone.getServiceStateTracker();
            boolean isRoaming = sst.mSS.getRoaming();
            int defaultSubId = SubscriptionController.getInstance().getDefaultDataSubId();
            log("isDataAllowed: " + ", current sub=" + mPhone.getSubId()
                    + ", default sub=" + defaultSubId + ", isRoaming=" + isRoaming
                    + ", icc records=" + mIccRecords);
            if (mPhone.getSubId() == defaultSubId && !isRoaming && curIccRecords != null
                    && (curIccRecords instanceof SIMRecords
                    || (curIccRecords instanceof RuimRecords && !curIccRecords.isLoaded()))) {
                recordsLoaded = false;
            }
        }
        /// @}

        boolean defaultDataSelected = SubscriptionManager.isValidSubscriptionId(
                SubscriptionManager.getDefaultDataSubscriptionId());

        boolean isMeteredApnType = apnContext == null
                || ApnSettingUtils.isMeteredApnType(ApnSetting.getApnTypesBitmaskFromString(
                        apnContext.getApnType()) , mPhone);

        PhoneConstants.State phoneState = PhoneConstants.State.IDLE;
        // Note this is explicitly not using mPhone.getState.  See b/19090488.
        // mPhone.getState reports the merge of CS and PS (volte) voice call state
        // but we only care about CS calls here for data/voice concurrency issues.
        // Calling getCallTracker currently gives you just the CS side where the
        // ImsCallTracker is held internally where applicable.
        // This should be redesigned to ask explicitly what we want:
        // voiceCallStateAllowDataCall, or dataCallAllowed or something similar.
        if (mPhone.getCallTracker() != null) {
            phoneState = mPhone.getCallTracker().getState();
        }

        // Step 2: Special handling for emergency APN.
        if (apnContext != null
                && apnContext.getApnType().equals(PhoneConstants.APN_TYPE_EMERGENCY)
                && apnContext.isConnectable()) {
            // If this is an emergency APN, as long as the APN is connectable, we
            // should allow it.
            if (dataConnectionReasons != null) {
                dataConnectionReasons.add(DataAllowedReasonType.EMERGENCY_APN);
            }
            // Bail out without further checks.
            return true;
        }

        // Step 3. Build disallowed reasons.
        if (apnContext != null && !apnContext.isConnectable()) {
            reasons.add(DataDisallowedReasonType.APN_NOT_CONNECTABLE);
        }

        // In legacy mode, if RAT is IWLAN then don't allow default/IA PDP at all.
        // Rest of APN types can be evaluated for remaining conditions.
        if ((apnContext != null && (apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DEFAULT)
                || apnContext.getApnType().equals(PhoneConstants.APN_TYPE_IA)))
                && mPhone.getTransportManager().isInLegacyMode()
                && dataRat == ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN) {
            reasons.add(DataDisallowedReasonType.ON_IWLAN);
        }

        if (isEmergency()) {
            reasons.add(DataDisallowedReasonType.IN_ECBM);
        }

        if (!attachedState && !shouldAutoAttach() && requestType != REQUEST_TYPE_HANDOVER) {
            reasons.add(DataDisallowedReasonType.NOT_ATTACHED);
        }
        if (!recordsLoaded) {
            reasons.add(DataDisallowedReasonType.RECORD_NOT_LOADED);
        }
        /// M: PS/CS concurrent @{
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();
        if (dcHelper != null && !dcHelper.isDataAllowedForConcurrent(mPhone.getPhoneId())) {
            reasons.add(DataDisallowedReasonType.INVALID_PHONE_STATE);
            reasons.add(DataDisallowedReasonType.CONCURRENT_VOICE_DATA_NOT_ALLOWED);
        }
        /// @}
        if (!internalDataEnabled) {
            reasons.add(DataDisallowedReasonType.INTERNAL_DATA_DISABLED);
        }
        if (!defaultDataSelected) {
            reasons.add(DataDisallowedReasonType.DEFAULT_DATA_UNSELECTED);
        }
        if (mPhone.getServiceState().getDataRoaming() && !getDataRoamingEnabled()) {
            reasons.add(DataDisallowedReasonType.ROAMING_DISABLED);
        }
        if (mIsPsRestricted) {
            reasons.add(DataDisallowedReasonType.PS_RESTRICTED);
        }
        if (!desiredPowerState) {
            reasons.add(DataDisallowedReasonType.UNDESIRED_POWER_STATE);
        }
        if (!radioStateFromCarrier) {
            reasons.add(DataDisallowedReasonType.RADIO_DISABLED_BY_CARRIER);
        }

        boolean isDataEnabled = apnContext == null ? mDataEnabledSettings.isDataEnabled()
                : mDataEnabledSettings.isDataEnabled(apnContext.getApnTypeBitmask());

        if (!isDataEnabled) {
            if (apnContext == null || !(isDataAllowedAsOff(apnContext.getApnType())
                    && mDataEnabledSettings.isPolicyDataEnabled())) {
                reasons.add(DataDisallowedReasonType.DATA_DISABLED);
            }
        }

        /// M: [General Operator] Data Framework - WWOP requirements @{
        isDataAllowedForRoamingFeature(reasons);
        /// @}
        /// M: SIM ME LOCK @{
        if (dcHelper != null && !dcHelper.isSimMeLockAllowed(mPhone.getPhoneId())) {
            reasons.add(DataDisallowedReasonType.MTK_SIM_ME_LOCK_NOT_ALLOWED);
        }
        /// @}
        /// M: FDN enabled @{
        if (isFdnEnabled()) {
            reasons.add(DataDisallowedReasonType.MTK_FDN_ENABLED);
        }
        /// @}
        /// M: Allow config @{
        if (!getAllowConfig()) {
            reasons.add(DataDisallowedReasonType.MTK_NOT_ALLOWED);
        }
        /// @}
        /// M: VSIM @{
        MtkIccCardConstants.VsimType type = MtkUiccController.getVsimCardType(mPhone.getPhoneId());
        if (type.isAllowOnlyVsimNetwork()) {
            reasons.add(DataDisallowedReasonType.MTK_NON_VSIM_PDN_NOT_ALLOWED);
        }
        /// @}
        /// M: Data Retry requirement for Telcel and Telstra operators @{
        if (isDataRetryRestrictEnabled()) {
            if (apnContext != null
                    && (apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DEFAULT)
                    || apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DUN))) {
                ApnSetting apnSetting = apnContext.getApnSetting();
                if (apnSetting != null && apnSetting.getPermanentFailed()) {
                    reasons.add(DataDisallowedReasonType.MTK_DATA_RETRY_NOT_ALLOWED);
                }
            }
        }
        /// @}
        /// M: Not allow default apn when the call is ended for temp data switch @{
        if (apnContext != null
                && apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DEFAULT)
                && TextUtils.equals(Phone.REASON_VOICE_CALL_ENDED, apnContext.getReason())
                && mPhone.getSubId() != SubscriptionManager.getDefaultDataSubscriptionId()) {
            reasons.add(DataDisallowedReasonType.MTK_TEMP_DATA_SWITCH_NOT_ALLOWED);
        }
        /// @}
        /// M: Pco @{
        if (apnContext != null
                && apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DEFAULT)
                && !getIsPcoAllowedDefault()) {
            reasons.add(DataDisallowedReasonType.MTK_PCO_NOT_ALLOWED);
        }
        /// @}
        /// M: [pending data call during located plmn changing] @{
        // check this in the last
        if (apnContext != null
                && reasons.allowed() && isLocatedPlmnChanged()) {
            reasons.add(DataDisallowedReasonType.MTK_LOCATED_PLMN_CHANGED);
        }
        /// @}

        // If there are hard disallowed reasons, we should not allow data connection no matter what.
        if (reasons.containsHardDisallowedReasons()) {
            if (dataConnectionReasons != null) {
                dataConnectionReasons.copyFrom(reasons);
            }
            return false;
        }

        // Step 4: Determine if data should be allowed in some special conditions.

        // At this point, if data is not allowed, it must be because of the soft reasons. We
        // should start to check some special conditions that data will be allowed.
        if (!reasons.allowed()) {
            // If the device is on IWLAN, then all data should be unmetered. Check if the transport
            // is WLAN (for AP-assisted mode devices), or RAT equals IWLAN (for legacy mode devices)
            if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WLAN
                    || (mPhone.getTransportManager().isInLegacyMode()
                    && dataRat == ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN)) {
                reasons.add(DataAllowedReasonType.UNMETERED_APN);
            // Or if the data is on cellular, and the APN type is determined unmetered by the
            // configuration.
            } else if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WWAN
                    && !isMeteredApnType) {
                reasons.add(DataAllowedReasonType.UNMETERED_APN);
            }

            // If the request is restricted and there are only soft disallowed reasons (e.g. data
            // disabled, data roaming disabled) existing, we should allow the data.
            if (apnContext != null
                    && apnContext.hasRestrictedRequests(true)
                    && !reasons.allowed()) {
                reasons.add(DataAllowedReasonType.RESTRICTED_REQUEST);
            }
        } else {
            // If there is no disallowed reasons, then we should allow the data request with
            // normal reason.
            reasons.add(DataAllowedReasonType.NORMAL);
        }

        if (dataConnectionReasons != null) {
            dataConnectionReasons.copyFrom(reasons);
        }

        return reasons.allowed();
    }

    @Override
    protected void setupDataOnAllConnectableApns(String reason, RetryFailures retryFailures) {
        if (VDBG) log("setupDataOnAllConnectableApns: " + reason);

        if (DBG && !VDBG) {
            StringBuilder sb = new StringBuilder(120);
            for (ApnContext apnContext : mPrioritySortedApnContextsEx) {
                sb.append(apnContext.getApnType());
                sb.append(":[state=");
                sb.append(apnContext.getState());
                sb.append(",enabled=");
                sb.append(apnContext.isEnabled());
                sb.append("] ");
            }
            log("setupDataOnAllConnectableApns: " + reason + " " + sb);
        }

        for (ApnContext apnContext : mPrioritySortedApnContextsEx) {
            /// M: Data Retry requirement for Telcel and Telstra operators @{
            if (isDataRetryRestrictEnabled()) {
                if (apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DEFAULT)
                        || apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DUN)) {
                    if (TextUtils.equals(reason, MtkGsmCdmaPhone.REASON_CARRIER_CONFIG_LOADED)
                            || TextUtils.equals(reason, Phone.REASON_VOICE_CALL_ENDED)
                            || TextUtils.equals(reason, Phone.REASON_NW_TYPE_CHANGED)) {
                        log("ignore setup data call of default/dun apn for cc33 operators");
                        continue;
                    }
                    cancelReconnectAlarm(apnContext);
                    ApnSetting apnSetting = apnContext.getApnSetting();
                    if (apnSetting != null) {
                        apnSetting.setPermanentFailed(false);
                        log("set permanentFailed as false for default apn type");
                    }
                }
            }
            /// @}
            setupDataOnConnectableApn(apnContext, reason, retryFailures);
        }
    }

    @Override
    protected void setupDataOnConnectableApn(ApnContext apnContext, String reason,
            RetryFailures retryFailures) {
        if (VDBG) log("setupDataOnConnectableApn: apnContext " + apnContext);

        if (apnContext.getState() == DctConstants.State.FAILED
                || apnContext.getState() == DctConstants.State.RETRYING) {
            if (retryFailures == RetryFailures.ALWAYS) {
                apnContext.releaseDataConnection(reason);
            } else if (!apnContext.isConcurrentVoiceAndDataAllowed()
                    && mPhone.getServiceStateTracker().isConcurrentVoiceAndDataAllowed()) {
                // RetryFailures.ONLY_ON_CHANGE - check if voice concurrency has changed
                apnContext.releaseDataConnection(reason);
            }
        }
        if (apnContext.isConnectable()) {
            /// M: No need to trysetupdata as higher priority apncontext exists @{
            if (!(isOnlySingleDcAllowed(mPhone.getServiceState().getRilDataRadioTechnology())
                    && isHigherPriorityApnContextActive(apnContext))) {
                log("isConnectable() call trySetupData");
                apnContext.setReason(reason);
                /// M: Consider the handover case when the data rat is unknown @{
                if (mHandoverApnType == apnContext.getApnTypeBitmask()) {
                    mHandoverApnType = ApnSetting.TYPE_NONE;
                    trySetupData(apnContext, REQUEST_TYPE_HANDOVER);
                } else {
                /// @}
                    trySetupData(apnContext, REQUEST_TYPE_NORMAL);
                }
            } else {
                log("No need to trysetupdata as higher priority apncontext exists");
            }
            /// @}
        }
        /// SSC Mode 3 @{
        if (apnContext.getState() == DctConstants.State.CONNECTED &&
                TextUtils.equals(reason, MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3) &&
                TextUtils.equals(reason, apnContext.getReason())) {
                log("SSC mode3 call trySetupData for APN type" + apnContext.getApnType());
                trySetupData(apnContext, REQUEST_TYPE_NORMAL);
        }
        /// @}
        /// M: For 'apn changed' case, reset preferred apn as valid one and notify state @{
        if (TextUtils.equals(apnContext.getApnType(), PhoneConstants.APN_TYPE_DEFAULT)
                && apnContext.getState() == DctConstants.State.CONNECTED
                && TextUtils.equals(reason, Phone.REASON_APN_CHANGED)) {
            if (mCanSetPreferApn && mPreferredApn == null) {
                if (DBG) log("setupDataOnConnectableApns: PREFERRED APN is null");
                mPreferredApn = apnContext.getApnSetting();
                if (mPreferredApn != null) {
                    setPreferredApn(mPreferredApn.getId());
                    mPhone.notifyDataConnection(apnContext.getApnType());
                }
            }
        }
        /// @}
    }

    @Override
    protected boolean trySetupData(ApnContext apnContext, @RequestNetworkType int requestType) {
        if (mPhone.getSimulatedRadioControl() != null) {
            // Assume data is connected on the simulator
            // FIXME  this can be improved
            apnContext.setState(DctConstants.State.CONNECTED);
            mPhone.notifyDataConnection(apnContext.getApnType());

            log("trySetupData: X We're on the simulator; assuming connected retValue=true");
            return true;
        }

        DataConnectionReasons dataConnectionReasons = new DataConnectionReasons();
        boolean isDataAllowed = isDataAllowed(apnContext, requestType, dataConnectionReasons) ||
                // M: extend the logics of isDataAllowed()
                isDataAllowedExt(dataConnectionReasons, apnContext);

        // M: IMS E911 Bearer Management @{
        boolean isEmergencyApn = apnContext.getApnType().equals(PhoneConstants.APN_TYPE_EMERGENCY);
        if (!hasMdAutoSetupImsCapability()) {
            if (isEmergencyApn) {
                int defaultBearerCount = ((MtkDcController) mDcc).getActiveDcCount();
                log("defaultBearerCount: " + defaultBearerCount +
                        ", mDedicatedBearerCount: " + mDedicatedBearerCount);
                if ((defaultBearerCount + mDedicatedBearerCount) >= (THROTTLING_MAX_PDP_SIZE - 1)) {
                    teardownDataByEmergencyPolicy();
                    return false;
                }
            }
        }
        /// @}

        String logStr = "trySetupData for APN type " + apnContext.getApnType() + ", reason: "
                + apnContext.getReason() + ", requestType=" + requestTypeToString(requestType)
                + ". " + dataConnectionReasons.toString();
        if (DBG) log(logStr);
        apnContext.requestLog(logStr);
        if (isDataAllowed) {
            if (apnContext.getState() == DctConstants.State.FAILED) {
                String str = "trySetupData: make a FAILED ApnContext IDLE so its reusable";
                if (DBG) log(str);
                apnContext.requestLog(str);
                apnContext.setState(DctConstants.State.IDLE);
            }
            int radioTech = getDataRat();
            if (radioTech == ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
                radioTech = getVoiceRat();
            }
            log("service state=" + mPhone.getServiceState());
            apnContext.setConcurrentVoiceAndDataAllowed(mPhone.getServiceStateTracker()
                    .isConcurrentVoiceAndDataAllowed());
            if (apnContext.getState() == DctConstants.State.IDLE) {
                // M: ECC w/o SIM {
                if (TextUtils.equals(apnContext.getApnType(), PhoneConstants.APN_TYPE_EMERGENCY)) {
                    if (mAllApnSettings.isEmpty()) {
                        log("add mEmergencyApn: " + mEmergencyApn + " to mAllApnSettings");
                        addEmergencyApnSetting();
                    }
                }
                // M: ECC w/o SIM }
                ArrayList<ApnSetting> waitingApns =
                        buildWaitingApns(apnContext.getApnType(), radioTech);
                if (waitingApns.isEmpty()) {
                    notifyNoData(DataFailCause.MISSING_UNKNOWN_APN, apnContext);
                    String str = "trySetupData: X No APN found retValue=false";
                    if (DBG) log(str);
                    apnContext.requestLog(str);
                    /// M: Fake a data connection to notify caller to avoid meaningless waiting @{
                    mtkFakeDataConnection(apnContext);
                    /// @}
                    return false;
                } else {
                    apnContext.setWaitingApns(waitingApns);
                    // M: [OD over ePDG] @{
                    ((MtkApnContext) apnContext).setWifiApns(
                            buildWifiApns(apnContext.getApnType()));
                    // M: [OD over ePDG] @}
                    if (DBG) {
                        log ("trySetupData: Create from mAllApnSettings : "
                                    + apnListToString(mAllApnSettings));
                    }
                }
            }

            // M: [OD over ePDG] @{
            logd("trySetupData: call setupData, waitingApns : "
                    + apnListToString(apnContext.getWaitingApns())
                    + ", wifiApns : "
                    + apnListToString(((MtkApnContext) apnContext).getWifiApns()));
            // M: [OD over ePDG] end

            boolean retValue = setupData(apnContext, radioTech, requestType);

            if (DBG) log("trySetupData: X retValue=" + retValue);
            return retValue;
        } else {
            if (!apnContext.getApnType().equals(PhoneConstants.APN_TYPE_DEFAULT)
                    && apnContext.isConnectable()) {
                // M: [OD over ePDG] @{
                if (apnContext.getApnType().equals(PhoneConstants.APN_TYPE_MMS)
                        && TelephonyManager.getDefault().isMultiSimEnabled() && !mAttached.get()) {
                    log("Wait for attach");
                    return true;
                } else {
                // M: [OD over ePDG] @}
                    mPhone.notifyDataConnectionFailed(apnContext.getApnType());
                }
            }

            StringBuilder str = new StringBuilder();

            str.append("trySetupData failed. apnContext = [type=" + apnContext.getApnType()
                    + ", mState=" + apnContext.getState() + ", apnEnabled="
                    + apnContext.isEnabled() + ", mDependencyMet="
                    + apnContext.isDependencyMet() + "] ");

            if (!mDataEnabledSettings.isDataEnabled()) {
                str.append("isDataEnabled() = false. " + mDataEnabledSettings);
            }

            // If this is a data retry, we should set the APN state to FAILED so it won't stay
            // in RETRYING forever.
            if (apnContext.getState() == DctConstants.State.RETRYING) {
                apnContext.setState(DctConstants.State.FAILED);
                str.append(" Stop retrying.");
            }

            if (DBG) log(str.toString());
            apnContext.requestLog(str.toString());
            return false;
        }
    }

    @Override
    protected boolean cleanUpAllConnectionsInternal(boolean detach, String reason) {
        if (DBG) log("cleanUpAllConnectionsInternal: detach=" + detach + " reason=" + reason);
        boolean didDisconnect = false;
        boolean disableMeteredOnly = false;

        // reasons that only metered apn will be torn down
        if (!TextUtils.isEmpty(reason)) {
            disableMeteredOnly = reason.equals(Phone.REASON_DATA_SPECIFIC_DISABLED) ||
                    reason.equals(Phone.REASON_ROAMING_ON) ||
                    reason.equals(Phone.REASON_CARRIER_ACTION_DISABLE_METERED_APN);
            /// M: Ignore cleanup all connections if modem reset @{
            if (reason.equals(Phone.REASON_SIM_NOT_READY)) {
                log("cleanUpAllConnectionsInternal: not cleanup connections for sim not ready");
                return didDisconnect;
            }
            /// @}
        }

        for (ApnContext apnContext : mApnContexts.values()) {
            /// M: Ignore data roaming @{
            if (reason.equals(Phone.REASON_ROAMING_ON)
                    && ignoreDataRoaming(apnContext.getApnType())) {
                log("cleanUpAllConnectionsInternal: Ignore Data Roaming for apnType = "
                        + apnContext.getApnType());
                continue;
            }
            /// @}
            // Exclude the IMS APN from single data connection case.
            if (reason.equals(Phone.REASON_SINGLE_PDN_ARBITRATION)
                    && apnContext.getApnType().equals(PhoneConstants.APN_TYPE_IMS)) {
                continue;
            }
            if (reason.equals(Phone.REASON_RADIO_TURNED_OFF)
                    && (!hasMdAutoSetupImsCapability())
                    && apnContext.getApnType().equals(PhoneConstants.APN_TYPE_IMS)) {
                log("cleanUpAllConnectionsInternal: Ignore legacy ims for apnType = "
                        + apnContext.getApnType() + " reason = "+ reason);
                continue;
            }
            if (shouldCleanUpConnection(apnContext, disableMeteredOnly)) {
                // TODO - only do cleanup if not disconnected
                if (apnContext.isDisconnected() == false) didDisconnect = true;
                apnContext.setReason(reason);
                cleanUpConnectionInternal(detach, RELEASE_TYPE_DETACH, apnContext);
            } else if (DBG) {
                log("cleanUpAllConnectionsInternal: APN type " + apnContext.getApnType()
                        + " shouldn't be cleaned up.");
            }
        }

        stopNetStatPoll();
        stopDataStallAlarm();

        // TODO: Do we need mRequestedApnType?
        mRequestedApnType = ApnSetting.TYPE_DEFAULT;

        log("cleanUpAllConnectionsInternal: mDisconnectPendingCount = "
                + mDisconnectPendingCount);
        if (detach && mDisconnectPendingCount == 0) {
            notifyAllDataDisconnected();
        }

        return didDisconnect;
    }

    @Override
    protected void setInitialAttachApn() {
        if (hasOperatorIaCapability() == true) {
            super.setInitialAttachApn();
            return;
        }
        // M: JPN IA Start
        boolean needsResumeModem = false;
        String currentMcc;
        // M: JPN IA End

        boolean isIaApn = false;
        ApnSetting previousAttachApn = mInitialAttachApnSetting;
        IccRecords r = mIccRecords.get();
        String operatorNumeric = mtkGetOperatorNumeric(r);
        if (operatorNumeric == null || operatorNumeric.length() == 0) {
            log("setInitialApn: but no operator numeric");
            return;
        } else {
            // M: JPN IA Start
            synchronized (mNeedsResumeModemLock) {
                if (mNeedsResumeModem) {
                    mNeedsResumeModem = false;
                    needsResumeModem = true;
                }
            }
            currentMcc = operatorNumeric.substring(0, 3);
            log("setInitialApn: currentMcc = " + currentMcc + ", needsResumeModem = "
                    + needsResumeModem);
            // M: JPN IA End
        }

        String[] dualApnPlmnList = null;

        log("setInitialAttachApn: current attach Apn [" + mInitialAttachApnSetting + "]");

        ApnSetting iaApnSetting = null;
        ApnSetting defaultApnSetting = null;
        ApnSetting firstApnSetting = null;
        ApnSetting manualChangedAttachApn = null;

        log("setInitialApn: E mPreferredApn=" + mPreferredApn);

        // M: change attach APN for MD changed APN and handover to WIFI
        if (mIsImsHandover || MTK_IMS_TESTMODE_SUPPORT) {
            // In those case should change attach APN to  class3 APN (VZWINTERNET)
            // The use of getClassTypeApn will return the ApnSetting of specify class APN.
            // Need to make sure the class number is valid (e.g. class1~4) for OP12 APN.
            manualChangedAttachApn = getClassTypeApn(APN_CLASS_3);

            if (manualChangedAttachApn != null) {
                log("setInitialAttachApn: manualChangedAttachApn = " + manualChangedAttachApn);
            }
        }

        if (mMdChangedAttachApn == null) {
            // Restore MD requested APN class from property, for cases that DCT object disposed.
            // Don't restore if card changed.
            int phoneId = mPhone.getPhoneId();
            if (SubscriptionManager.isValidPhoneId(phoneId)) {
                int apnClass = SystemProperties.getInt(PROP_APN_CLASS + phoneId, -1);
                if (apnClass >= 0) {
                    String iccId = SystemProperties.get(PROPERTY_ICCID[phoneId], "");
                    String apnClassIccId = SystemProperties.get(PROP_APN_CLASS_ICCID + phoneId, "");
                    log("setInitialAttachApn: " + iccId + " , " + apnClassIccId + ", " + apnClass);
                    if (TextUtils.equals(iccId, apnClassIccId)) {
                        updateMdChangedAttachApn(apnClass);
                    } else {
                        SystemProperties.set(PROP_APN_CLASS_ICCID + phoneId, "");
                        SystemProperties.set(PROP_APN_CLASS + phoneId, "");
                    }
                }
            }
        }

        // M: IA-change attach APN
        // VZW required to detach when disabling VZWIMS. So when VZWIMS is MD changed APN
        // but disabling VZWIMS, follow AOSP logic to change IA.
        ApnSetting mdChangedAttachApn = mMdChangedAttachApn;
        if (mMdChangedAttachApn != null && getClassType(mMdChangedAttachApn) == APN_CLASS_1
                && !isMdChangedAttachApnEnabled()) {
            mdChangedAttachApn = null;
        }

        if (mdChangedAttachApn == null && manualChangedAttachApn == null) {
            if (mPreferredApn != null && mPreferredApn.canHandleType(ApnSetting.TYPE_IA)) {
                iaApnSetting = mPreferredApn;
            } else if (mAllApnSettings != null && !mAllApnSettings.isEmpty()) {
                firstApnSetting = mAllApnSettings.get(0);
                log("setInitialApn: firstApnSetting=" + firstApnSetting);

                boolean isSelectOpIa = false;
                // Search for Initial APN setting and the first apn that can handle default
                for (ApnSetting apn : mAllApnSettings) {
                    if (mIsAddMnoApnsIntoAllApnList) {
                        if (!isSimActivated() &&
                                SPRINT_IA_NI.compareToIgnoreCase(apn.getApnName()) == 0) {
                            isSelectOpIa = true;
                        } else {
                            isSelectOpIa = false;
                        }
                    }
                    log("setInitialApn: isSelectOpIa=" + isSelectOpIa);
                    // Can't use apn.canHandleType(),
                    // as that returns true for APNs that have no type.
                    if ((apn.canHandleType(ApnSetting.TYPE_IA) || isSelectOpIa) &&
                            apn.isEnabled() && checkIfDomesticInitialAttachApn(currentMcc)) {
                        // The Initial Attach APN is highest priority so use it if there is one
                        log("setInitialApn: iaApnSetting=" + apn);
                        iaApnSetting = apn;
                        if (ArrayUtils.contains(PLMN_EMPTY_APN_PCSCF_SET, operatorNumeric)) {
                            isIaApn = true;
                        }
                        break;
                    } else if ((defaultApnSetting == null)
                            && (apn.canHandleType(ApnSetting.TYPE_DEFAULT))) {
                        // Use the first default apn if no better choice
                        log("setInitialApn: defaultApnSetting=" + apn);
                        defaultApnSetting = apn;
                    }
                }
            }
        }
        // M: end of change attach APN

        // The priority of apn candidates from highest to lowest is:
        //   1) APN_TYPE_IA (Initial Attach)
        //   2) mPreferredApn, i.e. the current preferred apn
        //   3) The first apn that than handle APN_TYPE_DEFAULT
        //   4) The first APN we can find.

        mInitialAttachApnSetting = null;
        // M: change attach APN for MD changed APN and handover to WIFI
        if (manualChangedAttachApn != null) {
            log("setInitialAttachApn: using manualChangedAttachApn");
            mInitialAttachApnSetting = manualChangedAttachApn;
        } else if (mdChangedAttachApn != null) {
            log("setInitialAttachApn: using mMdChangedAttachApn");
            mInitialAttachApnSetting = mdChangedAttachApn;
        } else if (iaApnSetting != null) {
            if (DBG) log("setInitialAttachApn: using iaApnSetting");
            mInitialAttachApnSetting = iaApnSetting;
        } else if (mPreferredApn != null) {
            if (DBG) log("setInitialAttachApn: using mPreferredApn");
            mInitialAttachApnSetting = mPreferredApn;
        } else if (defaultApnSetting != null) {
            if (DBG) log("setInitialAttachApn: using defaultApnSetting");
            mInitialAttachApnSetting = defaultApnSetting;
        } else if (firstApnSetting != null) {
            if (DBG) log("setInitialAttachApn: using firstApnSetting");
            mInitialAttachApnSetting = firstApnSetting;
        }

        if (mInitialAttachApnSetting == null) {
            if (DBG) log("setInitialAttachApn: X There in no available apn, use empty");
            ApnSetting emptyApnSetting = ApnSetting.makeApnSetting(0, "", "", "", "", 0, null, "",
                    0, "", "", RILConstants.SETUP_DATA_AUTH_NONE, ApnSetting.TYPE_IA,
                    ApnSetting.PROTOCOL_IPV4V6,
                    ApnSetting.PROTOCOL_IPV4V6,
                    true, 0, 0, false, 0, 0, 0, 0, 0, "");
            /// M: Data icon performance enhancement @{
            ((MtkRIL)mPhone.mCi).setInitialAttachApn(createDataProfile(emptyApnSetting,
                    emptyApnSetting.equals(getPreferredApn())),
                    mPhone.getServiceState().getDataRoamingFromRegistration(), null);
            /// @}
        } else {
            if (DBG) log("setInitialAttachApn: X selected Apn=" + mInitialAttachApnSetting);
            String iaApn = mInitialAttachApnSetting.getApnName();
            if (isIaApn) {
                if (DBG) log("setInitialAttachApn: ESM flag false, change IA APN to empty");
                iaApn = "";
            }

            Message msg = null;
            // M: JPN IA Start
            if (needsResumeModem) {
                if (DBG) log("setInitialAttachApn: DCM IA support");
                msg = obtainMessage(MtkDctConstants.EVENT_SET_RESUME);
            }
            // M: JPN IA End
            /// M: Data icon performance enhancement @{
            ((MtkRIL)mPhone.mCi).setInitialAttachApn(createDataProfile(mInitialAttachApnSetting,
                            mInitialAttachApnSetting.equals(getPreferredApn())),
                    mPhone.getServiceState().getDataRoamingFromRegistration(), msg);
            /// @}
        }
        if (DBG) log("setInitialAttachApn: new attach Apn [" + mInitialAttachApnSetting + "]");
    }

    @Override
    protected void onApnChanged() {
        DctConstants.State overallState = getOverallState();
        boolean isDisconnected = (overallState == DctConstants.State.IDLE ||
                overallState == DctConstants.State.FAILED);

        if (mPhone instanceof GsmCdmaPhone) {
            // The "current" may no longer be valid.  MMS depends on this to send properly. TBD
            ((GsmCdmaPhone)mPhone).updateCurrentCarrierInProvider();
        }

        // TODO: It'd be nice to only do this if the changed entrie(s)
        // match the current operator.
        if (DBG) log("onApnChanged: createAllApnList and cleanUpAllConnections");
        createAllApnList();
        setDataProfilesAsNeeded();
        setInitialAttachApn();
        cleanUpConnectionsOnUpdatedApns(!isDisconnected, Phone.REASON_APN_CHANGED);

        /// M: postpone setup actions in changed done event @{
        sendOnApnChangedDone(false);
        /// @}
    }

    @Override
    protected boolean isHigherPriorityApnContextActive(ApnContext apnContext) {
        if (apnContext.getApnType().equals(PhoneConstants.APN_TYPE_IMS)) {
            return false;
        }

        for (ApnContext otherContext : mPrioritySortedApnContextsEx) {
            if (otherContext.getApnType().equals(PhoneConstants.APN_TYPE_IMS)) {
                continue;
            }
            if (apnContext.getApnType().equalsIgnoreCase(otherContext.getApnType())) return false;
            if (otherContext.isEnabled() && otherContext.getState() != DctConstants.State.FAILED) {
                return true;
            }
        }
        return false;
    }

    @Override
    protected boolean isOnlySingleDcAllowed(int rilRadioTech) {

        // MTK START [ALPS01540105]
        if (mDataConnectionExt != null) {
            try {
                // default is false
                boolean onlySingleDcAllowed = mDataConnectionExt.isOnlySingleDcAllowed();
                if (onlySingleDcAllowed == true) {
                    log("isOnlySingleDcAllowed: " + onlySingleDcAllowed);
                    return true;
                }
            } catch (Exception ex) {
                loge("Fail to create or use plug-in");
                ex.printStackTrace();
            }
        }
        // MTK END [ALPS01540105]

        return super.isOnlySingleDcAllowed(rilRadioTech);
    }

    @Override
    protected boolean retryAfterDisconnected(ApnContext apnContext) {
        boolean retry = true;
        String reason = apnContext.getReason();

        if (Phone.REASON_RADIO_TURNED_OFF.equals(reason) ||
                /// M: Add more reasons to disallow retry @{
                MtkGsmCdmaPhone.REASON_FDN_ENABLED.equals(reason) ||
                /// @}
                (isOnlySingleDcAllowed(getDataRat())
                && isHigherPriorityApnContextActive(apnContext))) {
            retry = false;
        }
        return retry;
    }

    @Override
    protected void onRecordsLoadedOrSubIdChanged() {
        // M @{
        // mcc/mnc will be empty if sim is pin locked
        IccRecords r = mIccRecords.get();
        String operatorNumeric = mtkGetOperatorNumeric(r);
        if (TextUtils.isEmpty(operatorNumeric)) {
            logd("onRecordsLoadedOrSubIdChanged: empty operator numeric, return");
            mIsOperatorNumericEmpty = true;
            return;
        }
        // M @}
        // M: CDMA 3G dual mode card re-attach
        if (MtkDcHelper.isCdma3GDualModeCard(mPhone.getPhoneId())) {
            DctConstants.State overallState = getOverallState();
            boolean isDisconnected = (overallState == DctConstants.State.IDLE ||
                    overallState == DctConstants.State.FAILED);
            if (!isDisconnected && mAllApnSettings != null
                    && !mAllApnSettings.isEmpty()) {
                String numeric = mAllApnSettings.get(0).getOperatorNumeric();
                if (numeric.length() > 0 && !numeric.equals(operatorNumeric)) {
                    logd("CDMA 3G dual mode card numeric change, clean up.");
                    cleanUpAllConnectionsInternal(true, Phone.REASON_APN_CHANGED);
                }
            }
        }

        int defaultSubId = SubscriptionController.getInstance().getDefaultDataSubId();
        int slotId = MtkSubscriptionController.getMtkInstance().getSlotIndex(defaultSubId);

        // M: Reset FDN check flag @{
        mIsFdnChecked = false;
        // M: Reset FDN check flag @}

        /// M: Data Retry requirement for Telcel and Telstra operators @{
        mDataRetryOperator = getDataRetryOperator();
        if (isDataRetryRestrictEnabled()) {
            ((MtkRIL)mPhone.mCi).setRemoveRestrictEutranMode(true, null);
        }
        /// @}

        getImsiFromRil();

        super.onRecordsLoadedOrSubIdChanged();
    }

    @Override
    protected void onSimNotReady() {
        super.onSimNotReady();
        setIsPcoAllowedDefault(true);
        mImsiGsm = null;
        mImsiCdma = null;
        mPhoneType = PhoneConstants.PHONE_TYPE_NONE;
    }

    @Override
    public void enableApn(@ApnType int apnType, @RequestNetworkType int requestType,
                          Message onCompleteMsg) {
        /// M: Data icon performance enhancement @{
        if ((apnType == ApnSetting.TYPE_DEFAULT && requestType == REQUEST_TYPE_NORMAL) ||
                (apnType == ApnSetting.TYPE_EMERGENCY && requestType == REQUEST_TYPE_NORMAL)) {
            ApnContext apnContext = mApnContextsByType.get(apnType);
            if (apnContext != null && apnContext.isDependencyMet() && !apnContext.isReady()) {
                DctConstants.State state = apnContext.getState();
                if (state == DctConstants.State.IDLE) {
                    log("Enable 'default' apn type in advance");
                    apnContext.setReason(Phone.REASON_DATA_ENABLED);
                    apnContext.setEnabled(true);
                    apnContext.resetErrorCodeRetries();
                    if (trySetupData(apnContext, requestType)) {
                        addRequestNetworkCompleteMsg(onCompleteMsg, apnType);
                    } else {
                        sendRequestNetworkCompleteMsg(onCompleteMsg, false, mTransportType,
                                requestType);
                    }
                    return;
                }
            }
        }
        /// @}
        super.enableApn(apnType, requestType, onCompleteMsg);
    }

    @Override
    protected void onDataRoamingOff() {
        if (DBG) {
            log("onDataRoamingOff getDataRoamingEnabled=" + getDataRoamingEnabled()
                    + ", mUserDataEnabled=" + mDataEnabledSettings.isUserDataEnabled());
        }

        /// M: For USCC(OP236) to revert roaming data setting.
        setRoamingDataWithRoamingType(ServiceState.ROAMING_TYPE_NOT_ROAMING);

        reevaluateDataConnections();

        if (isUniqueRoamingFeatureEnabled()) {
            // M: [General Operator] Data Framework - WWOP requirements @{
            boolean bDomDataOnRoamingEnabled = getDomesticDataRoamingEnabledFromSettings();
            boolean bIntDataOnRoamingEnabled = getInternationalDataRoamingEnabledFromSettings();

            log("onDomOrIntRoamingOn bDomDataOnRoamingEnabled=" + bDomDataOnRoamingEnabled
                    + ", bIntDataOnRoamingEnabled=" + bIntDataOnRoamingEnabled
                    + ", currentRoamingType=" + mPhone.getServiceState().getDataRoamingType());

            if (!bDomDataOnRoamingEnabled || !bIntDataOnRoamingEnabled) {
                if (DBG) log("onDomOrIntRoamingOn: setup data for HOME.");
                // TODO: Remove this once all old vendor RILs are gone.
                // We don't need to set initial apn
                // attach and send the data profile again as the modem should have both roaming and
                // non-roaming protocol in place. Modem should choose the right protocol based on
                // the roaming condition.
                setDataProfilesAsNeeded();
                setInitialAttachApn();

                // If the user did not enable data roaming, now when we transit from roaming to
                // non-roaming, we should try to reestablish the data connection.

                setupDataOnAllConnectableApns(Phone.REASON_ROAMING_OFF, RetryFailures.ALWAYS);
            } else {
                mPhone.notifyDataConnection();
            }
            // M: [General Operator] Data Framework - WWOP requirements @}
        } else {
            if (!getDataRoamingEnabled()) {
                // TODO: Remove this once all old vendor RILs are gone.
                // We don't need to set initial apn
                // attach and send the data profile again as the modem should have both roaming and
                // non-roaming protocol in place. Modem should choose the right protocol based on
                // the roaming condition.
                // 90 doesn't support EGDCONT and need to set IA APN and data profile.
                boolean bHasOperatorIaCapability = hasOperatorIaCapability();
                if (DBG) {
                    log("onDataRoamingOff: bHasOperatorIaCapability=" + bHasOperatorIaCapability);
                }
                if (!bHasOperatorIaCapability) {
                    setDataProfilesAsNeeded();
                    setInitialAttachApn();
                }

                // If the user did not enable data roaming, now when we transit from roaming to
                // non-roaming, we should try to reestablish the data connection.

                setupDataOnAllConnectableApns(Phone.REASON_ROAMING_OFF, RetryFailures.ALWAYS);
            } else {
                mPhone.notifyDataConnection();
            }
        }

        if (false == hasOperatorIaCapability()) {
            if (isOp18Sim()) {
                setInitialAttachApn();
            }
        }
    }

    @Override
    protected void onDataRoamingOnOrSettingsChanged(int messageType) {
        /// M: For USCC(OP236)
        int currentRoamingType = mPhone.getServiceState().getDataRoamingType();
        boolean settingChanged = (messageType == DctConstants.EVENT_ROAMING_SETTING_CHANGE);

        if (DBG) {
            log("onDataRoamingOnOrSettingsChanged getDataRoamingEnabled = "
                    + getDataRoamingEnabled()
                    + ", mUserDataEnabled = " + mDataEnabledSettings.isUserDataEnabled()
                    + ", settingChanged = " + settingChanged
                    + ", currentRoamingType = " + currentRoamingType);
        }

        // Check if the device is actually data roaming
        if (!mPhone.getServiceState().getDataRoaming()) {
            if (DBG) log("Device is not roaming, ignore the request.");
            return;
        }

        /// M: For USCC(OP236) to set roaming data setting.
        // When setRoamingDataWithRoamingType return true means there is no need to
        // handle roaming on event and waiting for roaming data setting change event. @{
        if (!settingChanged
                && setRoamingDataWithRoamingType(currentRoamingType)) return;
        /// M: @}

        checkDataRoamingStatus(settingChanged);

        if (!hasOperatorIaCapability() && isOp18Sim()) {
            setInitialAttachApn();
        }

        // M: [General Operator] Data Framework - WWOP requirements @{
        if (isUniqueRoamingFeatureEnabled()) {
            if (checkDomesticDataRoamingEnabled() || checkInternationalDataRoamingEnabled()) {
                log("onDataRoamingOnOrSettingsChanged: setup data on roaming");
                setupDataOnAllConnectableApns(Phone.REASON_ROAMING_ON, RetryFailures.ALWAYS);
                mPhone.notifyDataConnection();
            } else {
                log("onDataRoamingOnOrSettingsChanged: Tear down data connection on roaming.");
                cleanUpAllConnectionsInternal(true, Phone.REASON_ROAMING_ON);
            }
        // M: [General Operator] Data Framework - WWOP requirements @}
        } else if (getDataRoamingEnabled()
                // M: Data on domestic roaming.
                || getDomesticRoamingEnabled()) {
            // If the restricted data was brought up when data roaming is disabled, and now users
            // enable data roaming, we need to re-evaluate the conditions and possibly change the
            // network's capability.
            if (settingChanged) {
                reevaluateDataConnections();
            }
            if (DBG) log("onDataRoamingOnOrSettingsChanged: setup data on roaming");
            setupDataOnAllConnectableApns(Phone.REASON_ROAMING_ON, RetryFailures.ALWAYS);
            mPhone.notifyDataConnection();
        } else {
            // If the user does not turn on data roaming, when we transit from non-roaming to
            // roaming, we need to tear down the data connection otherwise the user might be
            // charged for data roaming usage.
            if (DBG) log("onDataRoamingOnOrSettingsChanged: Tear down data connection on roaming.");
            MtkIccCardConstants.VsimType type =
                    MtkUiccController.getVsimCardType(mPhone.getPhoneId());
            if (type == MtkIccCardConstants.VsimType.REMOTE_SIM) {
                log("RSim, not tear down any data connection since ignore data roaming");
            } else {
                cleanUpAllConnectionsInternal(true, Phone.REASON_ROAMING_ON);
            }
        }
    }

    @Override
    protected void onDisconnectDone(ApnContext apnContext) {
        /// SSC Mode 3 @{
        if (onSkipDisconnectDone(apnContext)) return;
        /// @}

        super.onDisconnectDone(apnContext);

        // M: IMS E911 Bearer Management @{
        if (apnContext == null) return;
        if (!hasMdAutoSetupImsCapability()) {
            if (Phone.MTK_REASON_PDN_OCCUPIED.equals(apnContext.getReason())) {
                if (DBG) log("try setup emergency PDN");
                ApnContext eImsContext =
                        mApnContextsByType.get(ApnSetting.TYPE_EMERGENCY);
                trySetupData(eImsContext, REQUEST_TYPE_NORMAL);
            }
        }
        /// @}
    }

    @Override
    protected void setDataProfilesAsNeeded() {
        if (DBG) log("setDataProfilesAsNeeded");
        /// M: Data icon performance enhancement @{
        IccRecords r = mIccRecords.get();
        String operator = mtkGetOperatorNumeric(r);
        if (mIsSimNotReady && (operator == null || operator.length() == 0)) {
            log("setDataProfilesAsNeeded: ignore, sim not ready and no operator numeric");
            return;
        }
        /// @}

        ArrayList<DataProfile> dataProfileList = new ArrayList<>();

        for (ApnSetting apn : mAllApnSettings) {
            /// M: encode 'inactiveTimer' into 'authType' @{
            ApnSetting apnEncode = encodeInactiveTimer(apn);
            DataProfile dp = createDataProfile(apnEncode, apn.equals(getPreferredApn()));
            /// @}
            if (!dataProfileList.contains(dp)) {
                dataProfileList.add(dp);
            }
        }

        /// M: Add 'dun' apn setting if needed @{
        ArrayList<ApnSetting> dunApns = fetchDunApns();
        if (dunApns != null && dunApns.size() > 0) {
            for (ApnSetting dun : dunApns) {
                DataProfile dpDun = createDataProfile(dun, dun.equals(getPreferredApn()));
                if (!dataProfileList.contains(dpDun)) {
                    dataProfileList.add(dpDun);
                    log("setDataProfilesAsNeeded: add DUN apn setting: " + dun);
                }
            }
        }
        /// @}

        // Check if the data profiles we are sending are same as we did last time. We don't want to
        // send the redundant profiles to the modem. Also if there the list is empty, we don't
        // send it to the modem.

        // M: if "flight mode power off modem" is enabled, profiles in modem will be cleaned up
        // after turning on then off flight mode. So, profiles need to be set to modem.
        // If "flight mode power off modem" is disabled, don't send redundant profiles.
        if (!dataProfileList.isEmpty()
                && (dataProfileList.size() != mLastDataProfileList.size()
                || !mLastDataProfileList.containsAll(dataProfileList)
                /// M: Add more pass conditions to set data profiles @{
                || RadioManager.isFlightModePowerOffModemEnabled()
                || mIsModemReset
                || "OP01".equals(SystemProperties.get("persist.vendor.operator.optr"))
                || "OP09".equals(SystemProperties.get("persist.vendor.operator.optr")))) {
                /// @}
            /// M: Data icon performance enhancement @{
            ((MtkRIL)mPhone.mCi).setDataProfile(dataProfileList.toArray(
                    new DataProfile[dataProfileList.size()]),
                    mPhone.getServiceState().getDataRoamingFromRegistration(), null);
            /// @}
            /// M: Fix AOSP bug to store the current data profile list @{
            mLastDataProfileList.clear();
            mLastDataProfileList.addAll(dataProfileList);
            /// @}
            /// M: Clear modem reset flag @{
            if (mIsModemReset) {
                mIsModemReset = false;
            }
            /// @}
        } else {
            if (DBG) log("setDataProfilesAsNeeded: ignore the empty or same data profile list");
        }
    }

    private ArrayList<ApnSetting> createApnList(Cursor cursor) {
        ArrayList<ApnSetting> mnoApns = new ArrayList<ApnSetting>();
        ArrayList<ApnSetting> mvnoApns = new ArrayList<ApnSetting>();
        IccRecords r = mIccRecords.get();

        if (cursor.moveToFirst()) {
            do {
                /// M: Make ApnSetting with extra fields @{
                ApnSetting apn = makeApnSetting(cursor);
                /// @}
                if (apn == null) {
                    continue;
                }

                if (apn.hasMvnoParams()) {
                    /// M: Data icon performance enhancement @{
                    if (isMvnoMatches(apn.getMvnoType(), apn.getMvnoMatchData())
                    /// @}
                            || (r != null && ApnSettingUtils.mvnoMatches(r,
                            apn.getMvnoType(), apn.getMvnoMatchData()))) {
                        mvnoApns.add(apn);
                    }
                } else {
                    mnoApns.add(apn);
                }
            } while (cursor.moveToNext());
        }

        ArrayList<ApnSetting> result;
        if (mvnoApns.isEmpty()) {
            result = mnoApns;
        } else {
            result = mvnoApns;
            /// M: Sprint special requriement @{
            if (mIsAddMnoApnsIntoAllApnList) {
                if (DBG) log("mnoApns=" + mnoApns);
                result.addAll(mnoApns);
            }
            /// @}
        }
        if (DBG) log("createApnList: X result=" + result);
        return result;
    }

    @Override
    protected void createAllApnList() {
        IccRecords r = mIccRecords.get();
        /// M: Data icon performance enhancement @{
        String operator = mtkGetOperatorNumeric(r);
        if (mIsSimNotReady && (operator == null || operator.length() == 0)) {
            log("createAllApnList: ignore, sim not ready and no operator numeric");
            return;
        }
        /// @}

        synchronized (mRefCountLock) {
            mAllApnSettings.clear();
            if (operator != null) {
                String selection = Telephony.Carriers.NUMERIC + " = '" + operator + "'";
                if (DBG) log("createAllApnList: selection=" + selection);

                // ORDER BY Telephony.Carriers._ID ("_id")
                Cursor cursor = mPhone.getContext().getContentResolver().query(
                        Uri.withAppendedPath(Telephony.Carriers.CONTENT_URI, "filtered"),
                        null, selection, null, Telephony.Carriers._ID);

                if (cursor != null) {
                    if (cursor.getCount() > 0) {
                        mAllApnSettings = createApnList(cursor);
                    }
                    cursor.close();
                } else {
                    if (DBG) log("createAllApnList: cursor is null");
                    mApnSettingsInitializationLog.log("cursor is null for carrier, operator: "
                            + operator);
                }
            }

            /// M: to support vsim apn type @{
            addVsimApnTypeToDefaultApnSetting();
            /// @}
        }

        addEmergencyApnSetting();

        dedupeApnSettings();

        if (mAllApnSettings.isEmpty()) {
            log("createAllApnList: No APN found for carrier, operator: " + operator);
            mApnSettingsInitializationLog.log("no APN found for carrier, operator: "
                    + operator);
            mPreferredApn = null;
            // TODO: What is the right behavior?
            //notifyNoData(DataConnection.FailCause.MISSING_UNKNOWN_APN);
        } else {
            mPreferredApn = getPreferredApn();
            if (mPreferredApn != null && !mPreferredApn.getOperatorNumeric().equals(operator)) {
                mPreferredApn = null;
                setPreferredApn(-1);
            }
            if (DBG) log("createAllApnList: mPreferredApn=" + mPreferredApn);
        }
        if (DBG) log("createAllApnList: X mAllApnSettings=" + mAllApnSettings);
    }

    @Override
    protected ApnSetting mergeApns(ApnSetting dest, ApnSetting src) {
        int id = dest.getId();
        if ((src.getApnTypeBitmask() & ApnSetting.TYPE_DEFAULT) == ApnSetting.TYPE_DEFAULT) {
            id = src.getId();
        }
        final int resultApnType = src.getApnTypeBitmask() | dest.getApnTypeBitmask();
        Uri mmsc = (dest.getMmsc() == null ? src.getMmsc() : dest.getMmsc());
        String mmsProxy = TextUtils.isEmpty(dest.getMmsProxyAddressAsString())
                ? src.getMmsProxyAddressAsString() : dest.getMmsProxyAddressAsString();
        int mmsPort = dest.getMmsProxyPort() == -1 ? src.getMmsProxyPort() : dest.getMmsProxyPort();
        String proxy = TextUtils.isEmpty(dest.getProxyAddressAsString())
                ? src.getProxyAddressAsString() : dest.getProxyAddressAsString();
        int port = dest.getProxyPort() == -1 ? src.getProxyPort() : dest.getProxyPort();
        int protocol = src.getProtocol() == ApnSetting.PROTOCOL_IPV4V6 ? src.getProtocol()
                : dest.getProtocol();
        int roamingProtocol = src.getRoamingProtocol() == ApnSetting.PROTOCOL_IPV4V6
                ? src.getRoamingProtocol() : dest.getRoamingProtocol();
        int networkTypeBitmask = (dest.getNetworkTypeBitmask() == 0
                || src.getNetworkTypeBitmask() == 0)
                ? 0 : (dest.getNetworkTypeBitmask() | src.getNetworkTypeBitmask());

        /// M: add 'inactiveTimer' @{
        return new MtkApnSetting(id, dest.getOperatorNumeric(), dest.getEntryName(),
                dest.getApnName(), proxy, port, mmsc, mmsProxy, mmsPort, dest.getUser(),
                dest.getPassword(), dest.getAuthType(), resultApnType, protocol, roamingProtocol,
                dest.isEnabled(), networkTypeBitmask, dest.getProfileId(),
                (dest.isPersistent() || src.isPersistent()), dest.getMaxConns(),
                dest.getWaitTime(), dest.getMaxConnsTime(), dest.getMtu(), dest.getMvnoType(),
                dest.getMvnoMatchData(), dest.getApnSetId(), dest.getCarrierId(),
                dest.getSkip464Xlat(), ((MtkApnSetting) dest).inactiveTimer);
        /// @}
    }

    @Override
    protected String apnListToString (ArrayList<ApnSetting> apns) {
        try {
            return super.apnListToString(apns);
        } catch (NullPointerException ex) {
            ex.printStackTrace();
            return null;
        }
    }

    @Override
    protected void setPreferredApn(int pos) {
        if (mCanSetPreferApn) {
            log("setPreferredApn: insert pos=" + pos + ", subId=" + mPhone.getSubId());
        }
        super.setPreferredApn(pos);
    }

    @Override
    public void handleMessage (Message msg) {
        if (VDBG) log("handleMessage msg=" + msg);
        AsyncResult ar;
        switch (msg.what) {
            case MtkDctConstants.EVENT_APN_CHANGED_DONE:
                logd("EVENT_APN_CHANGED_DONE");
                // default changed
                onApnChangedDone();
                break;
            case DctConstants.EVENT_PS_RESTRICT_DISABLED:
                /**
                 * When PS restrict is removed, we need setup PDP connection if
                 * PDP connection is down.
                 */
                // M: Wifi only
                ConnectivityManager cnnm = (ConnectivityManager) mPhone.getContext()
                        .getSystemService(Context.CONNECTIVITY_SERVICE);

                if (DBG) log("EVENT_PS_RESTRICT_DISABLED " + mIsPsRestricted);
                mIsPsRestricted  = false;
                if (isConnected()) {
                    startNetStatPoll();
                    startDataStallAlarm(DATA_STALL_NOT_SUSPECTED);
                } else {
                    // TODO: Should all PDN states be checked to fail?
                    if (mState == DctConstants.State.FAILED) {
                        cleanUpAllConnectionsInternal(false, Phone.REASON_PS_RESTRICT_ENABLED);
                        mReregisterOnReconnectFailure = false;
                    }
                    ApnContext apnContext = mApnContextsByType.get(ApnSetting.TYPE_DEFAULT);
                    if (apnContext != null) {
                        // M: Fix dual DataConnection issue. For the case that PS is detached but
                        //    the EVENT_DATA_CONNECTION_DETACHED haven't received yet. In this case,
                        //    isDataAllow returns true and will try to setup data. Then, the detach
                        //    message received and set mAttached as false. After this,
                        //    onDisconnectDone() called and will set ApnContext idle and DCAC null.
                        //    It will make DCAC can not re-use when setup data at the second time.
                        if (mPhone.getServiceStateTracker().getCurrentDataConnectionState()
                                == ServiceState.STATE_IN_SERVICE) {
                            apnContext.setReason(Phone.REASON_PS_RESTRICT_ENABLED);
                            trySetupData(apnContext, REQUEST_TYPE_NORMAL);
                        } else {
                            log("EVENT_PS_RESTRICT_DISABLED, data not attached, skip.");
                        }
                    } else {
                        loge("**** Default ApnContext not found ****");
                        // M: Wifi only
                        if (Build.IS_DEBUGGABLE && cnnm.isNetworkSupported(
                                ConnectivityManager.TYPE_MOBILE)) {
                            throw new RuntimeException("Default ApnContext not found");
                        }
                    }

                    /// M: try to setup MMS data connection @{
                    apnContext = mApnContextsByType.get(ApnSetting.TYPE_MMS);
                    if (apnContext != null && apnContext.isConnectable()) {
                        apnContext.setReason(Phone.REASON_PS_RESTRICT_ENABLED);
                        trySetupData(apnContext, REQUEST_TYPE_NORMAL);
                    } else {
                        loge("**** MMS ApnContext not found ****");
                    }
                    /// @}
                }
                break;
            //M: FDN Support
            case MtkDctConstants.EVENT_FDN_CHANGED:
                onFdnChanged();
                break;
            case MtkDctConstants.EVENT_RESET_PDP_DONE:
                logd("EVENT_RESET_PDP_DONE cid=" + msg.arg1);
                // Notify Default PDN state as this message is only for default APN currently.
                mPhone.notifyDataConnection(PhoneConstants.APN_TYPE_DEFAULT);
                break;
            /// M: Data Retry requirement for Telcel and Telstra operators @{
            case MtkDctConstants.EVENT_REMOVE_RESTRICT_EUTRAN:
                if (isDataRetryRestrictEnabled()) {
                    logd("EVENT_REMOVE_RESTRICT_EUTRAN");
                    mReregisterOnReconnectFailure = false;
                    setupDataOnAllConnectableApns(Phone.REASON_PS_RESTRICT_DISABLED,
                            RetryFailures.ALWAYS);
                }
                break;
            /// @}
            // M: Data Framework - Data Retry enhancement
            case MtkDctConstants.EVENT_MD_DATA_RETRY_COUNT_RESET:
                logd("EVENT_MD_DATA_RETRY_COUNT_RESET");
                if (mIsOperatorNumericEmpty) {
                    mIsOperatorNumericEmpty = false;
                    if (SubscriptionManager.isValidSubscriptionId(mPhone.getSubId())) {
                        onRecordsLoadedOrSubIdChanged();
                    }
                } else {
                    setupDataOnAllConnectableApns(MtkGsmCdmaPhone.REASON_MD_DATA_RETRY_COUNT_RESET,
                            RetryFailures.ALWAYS);
                }
                break;
            // M: [pending data call during located plmn changing] @}
            case MtkDctConstants.EVENT_SETUP_PENDING_DATA:
                onProcessPendingSetupData();
                break;
            // M: [pending data call during located plmn changing] @}
            // M: IA-change attach APN
            case MtkDctConstants.EVENT_ATTACH_APN_CHANGED:
                onMdChangedAttachApn((AsyncResult) msg.obj);
                break;
            // M: JPN IA Start
            case MtkDctConstants.EVENT_REG_PLMN_CHANGED:
                log("handleMessage : <EVENT_REG_PLMN_CHANGED>");
                if (isOp129IaSupport() || isOp17IaSupport()) {
                    handlePlmnChange((AsyncResult) msg.obj);
                }
                break;
            case MtkDctConstants.EVENT_REG_SUSPENDED:
                log("handleMessage : <EVENT_REG_SUSPENDED>");
                if (isOp129IaSupport() || isOp17IaSupport()) {
                    if (isNeedToResumeMd()) {
                        handleRegistrationSuspend((AsyncResult) msg.obj);
                    }
                }
                break;
            case MtkDctConstants.EVENT_SET_RESUME:
                log("handleMessage : <EVENT_SET_RESUME>");
                if (isOp129IaSupport() || isOp17IaSupport()) {
                    handleSetResume();
                }
                break;
            // M: JPN IA End
            //Reset Attach Apn
            case MtkDctConstants.EVENT_RESET_ATTACH_APN: {
                if (mAllApnSettings != null && !mAllApnSettings.isEmpty()) {
                    if (mAllApnSettings.size() == 1 &&
                            mAllApnSettings.get(0).getOperatorNumeric().length() == 0) {
                        logw("Only common Emergency APN, skip set initial attach apn.");
                    } else {
                        setInitialAttachApn();
                    }
                } else {
                    if (DBG) {
                        log("EVENT_RESET_ATTACH_APN: Ignore due to null APN list");
                    }
                }
                break;
            }
            // M: Reset MD changed APN
            case MtkDctConstants.EVENT_RIL_CONNECTED:
                logd("EVENT_RIL_CONNECTED");
                mIsModemReset = true;
                SystemProperties.set(PROP_APN_CLASS_ICCID + mPhone.getPhoneId(), "");
                SystemProperties.set(PROP_APN_CLASS + mPhone.getPhoneId(), "");
                getImsiFromRil();
                break;
            // M: [LTE][Low Power][UL traffic shaping] @{
            case MtkDctConstants.EVENT_LTE_ACCESS_STRATUM_STATE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    int[] ints = (int[]) ar.result;
                    int lteAccessStratumDataState = ints.length > 0 ? ints[0]
                            : DctConstants.INVALID;
                    int networkType = (ints.length > 1) ? ints[1] : DctConstants.INVALID;
                    if (lteAccessStratumDataState != LTE_AS_CONNECTED) { // LTE AS Disconnected
                        notifyPsNetworkTypeChanged(networkType);
                    } else { // LTE AS Connected
                        broadcastPsNetworkTypeChanged(TelephonyManager.NETWORK_TYPE_LTE);
                    }
                    logd("EVENT_LTE_ACCESS_STRATUM_STATE lteAccessStratumDataState = "
                            + lteAccessStratumDataState + ", networkType = " + networkType);
                    notifyLteAccessStratumChanged(lteAccessStratumDataState);
                } else {
                    loge("LteAccessStratumState exception: " + ar.exception);
                }
                break;

            case MtkDctConstants.EVENT_DEFAULT_APN_REFERENCE_COUNT_CHANGED: {
                int newDefaultRefCount = msg.arg1;
                onSharedDefaultApnState(newDefaultRefCount);
                break;
            }
            // M: [LTE][Low Power][UL traffic shaping] @}
            // M: Data on domestic roaming.
            case MtkDctConstants.EVENT_ROAMING_TYPE_CHANGED:
                onRoamingTypeChanged();
                break;
            // M: Multi-PS Attach Start
            case MtkDctConstants.EVENT_DATA_ALLOWED:
                ar = (AsyncResult) msg.obj;
                if (ar != null && ar.result != null) {
                    int[] ints = (int[]) ar.result;
                    boolean allowed = ints[0] == 1 ? true : false;
                    onAllowChanged(allowed);
                } else {
                    loge("Parameter error: ret should not be NULL");
                }
                break;
            // M: Multi-PS Attach End
            /// M: Fix CarrierConfigLoader timing issue. @{
            case MtkDctConstants.EVENT_CARRIER_CONFIG_LOADED:
                setupDataOnAllConnectableApns(MtkGsmCdmaPhone.REASON_CARRIER_CONFIG_LOADED,
                        RetryFailures.ALWAYS);
                break;
            /// @}
            // M: [VzW] Data Framework @{
            case MtkDctConstants.EVENT_DATA_ATTACHED_PCO_STATUS:
                handlePcoDataAfterAttached((AsyncResult) msg.obj);
                break;
            // M: [VzW] Data Framework @}
            /// Ims Data Framework {@
            case MtkDctConstants.EVENT_DEDICATED_BEARER_ACTIVATED:
                mDedicatedBearerCount = mDedicatedBearerCount + 1;
                ar = (AsyncResult) msg.obj;
                if (ar.result instanceof MtkDedicateDataCallResponse) {
                    MtkDedicateDataCallResponse rs = (MtkDedicateDataCallResponse) ar.result;
                    onDedecatedBearerActivated(rs);
                }
                break;
            case MtkDctConstants.EVENT_DEDICATED_BEARER_MODIFIED:
                ar = (AsyncResult) msg.obj;
                if (ar.result instanceof MtkDedicateDataCallResponse) {
                    MtkDedicateDataCallResponse rs = (MtkDedicateDataCallResponse) ar.result;
                    onDedecatedBearerModified(rs);
                }
                break;
             case MtkDctConstants.EVENT_DEDICATED_BEARER_DEACTIVATED:
                if (mDedicatedBearerCount > 0) {
                    mDedicatedBearerCount = mDedicatedBearerCount - 1;
                }
                ar = (AsyncResult) msg.obj;
                int cid = (int)ar.result;
                onDedecatedBearerDeactivated(cid);
                break;
            /// @}
            case DctConstants.EVENT_DATA_RAT_CHANGED:
                if (mRilRat != mPhone.getServiceState().getRilDataRadioTechnology()) {
                    /// M: Data icon performance enhancement @{
                    mTurboSS = null;
                    /// @}
                    mRilRat = mPhone.getServiceState().getRilDataRadioTechnology();
                    super.handleMessage(msg);
                }
                break;
            case MtkDctConstants.EVENT_NETWORK_REJECT:
                ar = (AsyncResult) msg.obj;
                onNetworkRejectReceived(ar);
                break;
            case MtkDctConstants.CMD_TEAR_DOWN_PDN_BY_TYPE:
                onTearDownPdnByApnId(msg.arg1);
                break;
            /// SSC Mode 3 @{
            case MtkDctConstants.EVENT_DATA_SETUP_SSC_MODE3:
                onDataSetupSscMode3(msg.arg1, msg.arg2);
                break;
            /// @}
            case MtkDctConstants.EVENT_MODEM_RESET:
                logd("EVENT_MODEM_RESET");
                mIsModemReset = true;
                break;
            case MtkDctConstants.EVENT_IMSI_QUERY_DONE:
                logd("EVENT_IMSI_QUERY_DONE");
                ar = (AsyncResult) msg.obj;
                if (ar.exception != null) {
                    loge("Exception querying IMSI, Exception:" + ar.exception);
                    break;
                }
                if (msg.arg1 == PhoneConstants.PHONE_TYPE_CDMA) {
                    mImsiCdma = IccUtils.stripTrailingFs((String) ar.result);
                } else {
                    mImsiGsm = IccUtils.stripTrailingFs((String) ar.result);
                }
                break;
            // M: [Data Usage Update][Network Latency Optimization] @{
            case MtkDctConstants.EVENT_MOBILE_DATA_USAGE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null && ar.result != null) {
                    updateMobileDataUsage(ar);
                } else {
                   loge("EVENT_MOBILE_DATA_USAGE, exception: "+ ar.exception);
                }
                break;
            // M: [Data Usage Update][Network Latency Optimization] @}
            case MtkDctConstants.EVENT_RECORDS_OVERRIDE:
                logd("EVENT_RECORDS_OVERRIDE");
                mIsRecordsOverride = true;
                break;
            case MtkDctConstants.EVENT_NW_LIMIT_STATE:
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    int[] ints = (int[]) ar.result;
                    int nwLimitState = ints.length > 0 ? ints[0] : DctConstants.INVALID;
                    logd("nwLimitState = " + nwLimitState);
                } else {
                    loge("EVENT_NW_LIMIT_STATE, exception: " + ar.exception);
                }
                break;
            default:
                super.handleMessage(msg);
                break;
        }
    }

    @Override
    protected int getApnProfileID(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_IMS)) {
            return RILConstants.DATA_PROFILE_IMS;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_FOTA)) {
            return RILConstants.DATA_PROFILE_FOTA;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_CBS)) {
            return RILConstants.DATA_PROFILE_CBS;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_IA)) {
            return RILConstants.DATA_PROFILE_DEFAULT; // DEFAULT for now
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DUN)) {
            return RILConstants.DATA_PROFILE_TETHERED;
        // M @{
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_MMS)) {
            return MtkRILConstants.DATA_PROFILE_MMS;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_SUPL)) {
            return MtkRILConstants.DATA_PROFILE_SUPL;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_HIPRI)) {
            return MtkRILConstants.DATA_PROFILE_HIPRI;
        } else if (TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_WAP)) {
            return MtkRILConstants.DATA_PROFILE_WAP;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_EMERGENCY)) {
            return MtkRILConstants.DATA_PROFILE_EMERGENCY;
        } else if (TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_XCAP)) {
            return MtkRILConstants.DATA_PROFILE_XCAP;
        } else if (TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_RCS)) {
            return MtkRILConstants.DATA_PROFILE_RCS;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DEFAULT)) {
            return RILConstants.DATA_PROFILE_DEFAULT;
        } else if (TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_BIP)) {
            return MtkRILConstants.DATA_PROFILE_BIP;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_ALL)) {
            return MtkRILConstants.DATA_PROFILE_ALL;
        } else if (TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_VSIM)) {
            return MtkRILConstants.DATA_PROFILE_VSIM;
        } else if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_MCX)) {
            return MtkRILConstants.DATA_PROFILE_MCX;
        } else {
            return RILConstants.DATA_PROFILE_INVALID;
        // M @}
        }
    }

    @Override
    protected void onUpdateIcc() {
        if (mUiccController == null) {
            return;
        }

        IccRecords newIccRecords = null;
        int phoneType = PhoneConstants.PHONE_TYPE_NONE;
        if (MtkDcHelper.isCdma3GDualModeCard(mPhone.getPhoneId())
                || MtkDcHelper.isCdma3GCard(mPhone.getPhoneId())) {
            newIccRecords = mPhone.getIccRecords();
            phoneType = mPhone.getPhoneType();

        } else {
            newIccRecords = getUiccRecords(UiccController.APP_FAM_3GPP);
            phoneType = PhoneConstants.PHONE_TYPE_GSM;
        }

        IccRecords r = mIccRecords.get();
        logd("onUpdateIcc: newIccRecords=" + newIccRecords + ", r=" + r);
        if (r != newIccRecords) {
            if (r != null) {
                log("Removing stale icc objects.");
                r.unregisterForRecordsLoaded(this);
                /// M: Data icon performance enhancement @{
                r.unregisterForRecordsOverride(this);
                /// @}
                mIccRecords.set(null);
            }
            if (newIccRecords != null) {
                if (SubscriptionManager.isValidSubscriptionId(mPhone.getSubId())) {
                    log("New records found.");
                    /// M: Data icon performance enhancement @{
                    mPhoneType = phoneType;
                    mIsSimNotReady = false;
                    mIsRecordsOverride = false;
                    /// @}
                    mIccRecords.set(newIccRecords);
                    newIccRecords.registerForRecordsLoaded(
                            this, DctConstants.EVENT_RECORDS_LOADED, null);
                    /// M: Data icon performance enhancement @{
                    newIccRecords.registerForRecordsOverride(
                            this, MtkDctConstants.EVENT_RECORDS_OVERRIDE, null);
                    /// @}
                }
            } else {
                /// M: Data icon performance enhancement @{
                mIsSimNotReady = true;
                mIsRecordsOverride = false;
                /// @}
                onSimNotReady();
            }
        }
        // M: FDN Support @{
        if (mUiccCardApplication == null) {
            mUiccCardApplication = new AtomicReference<UiccCardApplication>();
        }
        UiccCardApplication app = mUiccCardApplication.get();
        UiccCardApplication newUiccCardApp = ((MtkUiccController) mUiccController).
                getUiccCardApplication(mPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA ?
                UiccController.APP_FAM_3GPP2 : UiccController.APP_FAM_3GPP);

        if (app != newUiccCardApp) {
            if (app != null) {
                log("Removing stale UiccCardApplication objects.");
                ((MtkUiccCardApplication) app).unregisterForFdnChanged(this);
                mUiccCardApplication.set(null);
            }

            if (newUiccCardApp != null) {
                log("New UiccCardApplication found");
                ((MtkUiccCardApplication) newUiccCardApp).registerForFdnChanged(
                        this, MtkDctConstants.EVENT_FDN_CHANGED, null);
                mUiccCardApplication.set(newUiccCardApp);
            }
        }
        // M: FDN Support @}
    }

    @Override
    protected void onDataEnabledChanged(boolean enable,
                                      @DataEnabledChangedReason int enabledChangedReason) {
        if (DBG) {
            log("onDataEnabledChanged: enable=" + enable + ", enabledChangedReason="
                    + enabledChangedReason);
        }

        if (enable) {
            reevaluateDataConnections();
            setupDataOnAllConnectableApns(Phone.REASON_DATA_ENABLED, RetryFailures.ALWAYS);
        } else {
            String cleanupReason;
            switch (enabledChangedReason) {
                case DataEnabledSettings.REASON_INTERNAL_DATA_ENABLED:
                    cleanupReason = Phone.REASON_DATA_DISABLED_INTERNAL;
                    break;
                case DataEnabledSettings.REASON_DATA_ENABLED_BY_CARRIER:
                    cleanupReason = Phone.REASON_CARRIER_ACTION_DISABLE_METERED_APN;
                    break;
                case DataEnabledSettings.REASON_USER_DATA_ENABLED:
                case DataEnabledSettings.REASON_POLICY_DATA_ENABLED:
                case DataEnabledSettings.REASON_PROVISIONED_CHANGED:
                case DataEnabledSettings.REASON_PROVISIONING_DATA_ENABLED_CHANGED:
                default:
                    cleanupReason = Phone.REASON_DATA_SPECIFIC_DISABLED;
                    break;
            }
            /// M: Only clean up the connections that not allow data as off  @{
            //cleanUpAllConnectionsInternal(true, cleanupReason);
            for (ApnContext apnContext : mApnContexts.values()) {
                if (!isDataAllowedAsOff(apnContext.getApnType())) {
                    apnContext.setReason(cleanupReason);
                    cleanUpConnectionInternal(true, RELEASE_TYPE_DETACH, apnContext);
                }
            }
            /// @}
        }
    }

    public void log(String s) {
        // AOSP by default using Rlog.d()
        logd(s);
    }

    private void loge(String s) {
        Rlog.e(mLogTag, "[Mtk] " + s);
    }

    private void logw(String s) {
        Rlog.w(mLogTag, "[Mtk] " + s);
    }

    private void logi(String s) {
        // default user/userdebug debug level set as INFO
        Rlog.i(mLogTag, "[Mtk] " + s);
    }

    private void logd(String s) {
        // default eng debug level set as DEBUG
        Rlog.d(mLogTag, "[Mtk] " + s);
    }

    private void logv(String s) {
        Rlog.v(mLogTag, "[Mtk] " + s);
    }

    //M : RCS over 2/3G @{
    @Override
    public String[] getPcscfAddress(String apnType) {
        String[] result = null;
        result = super.getPcscfAddress(apnType);
        log("getPcscfAddress() for RCS, apnType=" + apnType);
        ApnContext apnContext = null;

        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DEFAULT)) {
            apnContext = mApnContextsByType.get(ApnSetting.TYPE_DEFAULT);
        }

        if (apnContext == null) {
            log("apnContext is null for RCS, return null");
            return null;
        }

        DataConnection dataConnection = apnContext.getDataConnection();

        if (dataConnection != null) {
            result = dataConnection.getPcscfAddresses();

            if (result != null) {
                for (int i = 0; i < result.length; i++) {
                    log("Pcscf[" + i + "]: " + result[i]);
                }
            }
            return result;
        }
        return null;
    }
    //M : RCS over 2/3G @}

    @Override
    protected void cleanUpConnectionsOnUpdatedApns(boolean detach, String reason) {
        if (DBG) log("cleanUpConnectionsOnUpdatedApns: detach=" + detach);
        if (mAllApnSettings.isEmpty()) {
            cleanUpAllConnectionsInternal(detach, Phone.REASON_APN_CHANGED);
        } else {
            if (getDataRat() == ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
                // unknown rat is an exception for data rat change. Its only received when out of
                // service and is not applicable for apn bearer bitmask. We should bypass the check
                // of waiting apn list and keep the data connection on.
                return;
            }
            for (ApnContext apnContext : mApnContexts.values()) {
                /// M: In new design, MD should take care eims pdn act and deact @{
                if (hasMdAutoSetupImsCapability() && Phone.REASON_NW_TYPE_CHANGED.equals(reason) &&
                        PhoneConstants.APN_TYPE_EMERGENCY.equals(apnContext.getApnType())) {
                    log("cleanUpConnectionsOnUpdatedApns(): skip emergency due to " + reason);
                    continue;
                }
                /// @}
                /// M: To avoid rebuild waitingApns everytime, we skip clean up connections flow
                /// if currentApnSetting is null. @{
                ApnSetting currentApnSetting = apnContext.getApnSetting();
                if (currentApnSetting == null) {
                    log("cleanUpConnectionsOnUpdatedApns(): currentApnSetting is null");
                    continue;
                }
                /// @}
                /// M: Compare the current apn setting with the new waiting APNs. @{
                ArrayList<ApnSetting> waitingApns = buildWaitingApns(
                        apnContext.getApnType(), getDataRat());
                if (VDBG) log("new waitingApns:" + waitingApns);
                boolean canHandle = false;
                for (ApnSetting newApnSetting : waitingApns) {
                    if (currentApnSetting.equals(newApnSetting,
                            mPhone.getServiceState().getDataRoamingFromRegistration())) {
                        canHandle = true;
                        break;
                    }
                }
                if (!canHandle || (isRestorePreferredApn() && apnContext.getWaitingApns() != null
                        && waitingApns.size() != apnContext.getWaitingApns().size())) {
                /// @}
                    if (VDBG) log("new waiting apn is different for " + apnContext);
                    apnContext.setWaitingApns(waitingApns);
                    if (!apnContext.isDisconnected()) {
                        if (VDBG) log("cleanUpConnectionsOnUpdatedApns for " + apnContext);
                        apnContext.setReason(reason);
                        cleanUpConnectionInternal(true, RELEASE_TYPE_DETACH, apnContext);
                    }
                }
            }
        }

        if (!isConnected()) {
            stopNetStatPoll();
            stopDataStallAlarm();
        }

        mRequestedApnType = ApnSetting.TYPE_DEFAULT;

        if (DBG) log("mDisconnectPendingCount = " + mDisconnectPendingCount);
        if (detach && mDisconnectPendingCount == 0) {
            notifyAllDataDisconnected();
        }
    }

    // M: [skip data stall] @{
    @Override
    protected void startDataStallAlarm(boolean suspectedStall) {
        if (mtkSkipDataStallAlarm()) {
            log("onDataStallAlarm: switch data-stall off, " + "skip it!");
        } else {
            super.startDataStallAlarm(suspectedStall);
        }
    }
    // M: [skip data stall] @}

    // M: [VzW] Data Framework @{
    private void handlePcoDataAfterAttached(AsyncResult ar) {
        if (mDataConnectionExt != null) {
            mDataConnectionExt.handlePcoDataAfterAttached(ar, mPhone, mAllApnSettings);
        }
    }
    // M: [VzW] Data Framework @}

    /*
     * MTK added methods start from here
     */
    private boolean isDataAllowedExt(DataConnectionReasons dataConnectionReasons,
            ApnContext apnContext) {
        if (apnContext == null) {
            if (DBG) log("isDataAllowedExt: apnContext is null, return false");
            return false;
        }
        String apnType = apnContext.getApnType();

        // M: [pending data call during located plmn changing] @{
        if (dataConnectionReasons.contains(
                    DataDisallowedReasonType.MTK_LOCATED_PLMN_CHANGED)) {
            log("isDataAllowedExt: located plmn changed, setSetupDataPendingFlag");
            mPendingDataCall = true;
            return false;
        }
        // M: [pending data call during located plmn changing] @}

        if (dataConnectionReasons.contains(
                DataDisallowedReasonType.DEFAULT_DATA_UNSELECTED)) {
            if (ignoreDefaultDataUnselected(apnType)) {
                dataConnectionReasons.mDataDisallowedReasonSet.remove(
                        DataDisallowedReasonType.DEFAULT_DATA_UNSELECTED);
            } else {
                return false;
            }
        }

        if (dataConnectionReasons.contains(
                DataDisallowedReasonType.ROAMING_DISABLED)) {
            if (ignoreDataRoaming(apnType)
                    // M: Data on domestic roaming.
                    || getDomesticRoamingEnabled()) {
                dataConnectionReasons.mDataDisallowedReasonSet.remove(
                        DataDisallowedReasonType.ROAMING_DISABLED);
            } else {
                return false;
            }
        }

        if (dataConnectionReasons.contains(
                DataDisallowedReasonType.MTK_NOT_ALLOWED)) {
            if (ignoreDataAllow(apnType)) {
                dataConnectionReasons.mDataDisallowedReasonSet.remove(
                        DataDisallowedReasonType.MTK_NOT_ALLOWED);
            } else {
                return false;
            }
        }

        // M: Vsim
        if (dataConnectionReasons.contains(
                DataDisallowedReasonType.MTK_NON_VSIM_PDN_NOT_ALLOWED)) {
            if (TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_VSIM)) {
                dataConnectionReasons.mDataDisallowedReasonSet.remove(
                        DataDisallowedReasonType.MTK_NON_VSIM_PDN_NOT_ALLOWED);
            } else {
                return false;
            }
        }

        // M: IMS
        if (dataConnectionReasons.contains(
                DataDisallowedReasonType.MTK_FDN_ENABLED)) {
            if (PhoneConstants.APN_TYPE_EMERGENCY.equals(apnType) ||
                    PhoneConstants.APN_TYPE_IMS.equals(apnType)) {
                if (DBG) log("isDataAllowedExt allow IMS/EIMS for reason FDN_ENABLED");
                dataConnectionReasons.mDataDisallowedReasonSet.remove(
                        DataDisallowedReasonType.MTK_FDN_ENABLED);
            } else {
                return false;
            }
        }

        /// SSC Mode 3 @{
        if (dataConnectionReasons.contains(
                DataDisallowedReasonType.APN_NOT_CONNECTABLE)) {
            if ((apnContext.getState() == DctConstants.State.CONNECTED) &&
                    MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3.equals(apnContext.getReason())) {
                if (DBG) log("isDataAllowedExt allow SSC mode3 for reason APN_NOT_CONNECTABLE");
                dataConnectionReasons.mDataDisallowedReasonSet.remove(
                        DataDisallowedReasonType.APN_NOT_CONNECTABLE);
            } else {
                return false;
            }
        }
        /// @}

        if (VDBG) log("isDataAllowedExt: " + dataConnectionReasons.allowed());

        return dataConnectionReasons.allowed();
    }

    private int getDefaultMtuConfig(Context context){
        int mtu = 0;
        IccRecords r = mIccRecords.get();
        String operator = mtkGetOperatorNumeric(r);
        int mcc = 0;
        int mnc = 0;
        if (operator != null && operator.length() > 3) {
            try {
                mcc = Integer.parseInt(operator.substring(0, 3));
                mnc = Integer.parseInt(operator.substring(3, operator.length()));
            } catch (NumberFormatException e) {
                e.printStackTrace();
                loge("operator numeric is invalid");
            }
        }

        Resources sysResource = context.getResources();
        int sysMcc = sysResource.getConfiguration().mcc;
        int sysMnc = sysResource.getConfiguration().mnc;
        Resources resource = null;
        try {
            Configuration configuration = new Configuration();
            configuration = context.getResources().getConfiguration();
            configuration.mcc = mcc;
            configuration.mnc = mnc;
            Context resc = context.createConfigurationContext(configuration);
            resource = resc.getResources();
        } catch (Exception e) {
            e.printStackTrace();
            loge("getResourcesUsingMccMnc fail");
        }

        if (resource == null) {
            mtu = sysResource.getInteger(com.android.internal.R.integer.config_mobile_mtu);
            logd("getDefaultMtuConfig: get sysResource sysMcc = " + sysMcc + ", sysMnc = " + sysMnc
                    + ", mcc = " + mcc +", mnc = " + mnc + ", mtu = " + mtu);
        } else {
            mtu = resource.getInteger(com.android.internal.R.integer.config_mobile_mtu);
            logd("getDefaultMtuConfig: get resource sysMcc = " + sysMcc + ", sysMnc = " + sysMnc
                    + ", mcc = " + mcc +", mnc = " + mnc + ", mtu = " + mtu);
        }

        return mtu;
    }

    // M: IA-change attach APN
    private void onMdChangedAttachApn(AsyncResult ar) {
        logv("onMdChangedAttachApn");
        int apnId = (int) ar.result;

        if (apnId != APN_CLASS_1 && apnId != APN_CLASS_3) {
            logw("onMdChangedAttachApn: Not handle APN Class:" + apnId);
            return;
        }

        // Save MD requested APN class in property, for cases that DCT object disposed.
        int phoneId = mPhone.getPhoneId();
        if (SubscriptionManager.isValidPhoneId(phoneId)) {
            String iccId = SystemProperties.get(PROPERTY_ICCID[phoneId], "");
            SystemProperties.set(PROP_APN_CLASS_ICCID + phoneId, iccId);
            SystemProperties.set(PROP_APN_CLASS + phoneId, String.valueOf(apnId));
            log("onMdChangedAttachApn, set " + iccId + ", " + apnId);
        }

        updateMdChangedAttachApn(apnId);

        if (mMdChangedAttachApn != null) {
            setInitialAttachApn();
        } else {
            // Before createAllApnList, the mMdChangedAttachApn will be null
            // after updateMdChangedAttachApn(), it will be set in
            // onRecordsLoaded->setInitialAttachApn()
            logw("onMdChangedAttachApn: MdChangedAttachApn is null, not found APN");
        }
    }

    // M: IA-change attach APN
    private void updateMdChangedAttachApn(int apnId) {
        if (mAllApnSettings != null && !mAllApnSettings.isEmpty()) {
            for (ApnSetting apn : mAllApnSettings) {
                if (apnId == APN_CLASS_1 && apn.canHandleType(ApnSetting.TYPE_IMS)) {
                    mMdChangedAttachApn = apn;
                    log("updateMdChangedAttachApn: MdChangedAttachApn=" + apn);
                    break;
                } else if (apnId == APN_CLASS_3 && apn.canHandleType(ApnSetting.TYPE_DEFAULT)) {
                    mMdChangedAttachApn = apn;
                    log("updateMdChangedAttachApn: MdChangedAttachApn=" + apn);
                    break;
                }
            }
        }
    }

    // M: IA-change attach APN
    private boolean isMdChangedAttachApnEnabled() {
        if (mMdChangedAttachApn != null && mAllApnSettings != null && !mAllApnSettings.isEmpty()) {
            for (ApnSetting apn : mAllApnSettings) {
                if (TextUtils.equals(mMdChangedAttachApn.getApnName(), apn.getApnName())) {
                    log("isMdChangedAttachApnEnabled: " + apn);
                    return apn.isEnabled();
                }
            }
        }
        return false;
    }

    private void sendOnApnChangedDone(boolean bImsApnChanged) {
        Message msg = obtainMessage(MtkDctConstants.EVENT_APN_CHANGED_DONE);
        msg.arg1 = bImsApnChanged ? 1 : 0;
        sendMessage(msg);
    }

    private void onApnChangedDone() {
        //Fixed:[ALPS01670132] Data icon cannot shows and data service cannot work
        //after change default APN some times.
        logd("onApnChangedDone: subId = " + mPhone.getSubId() + ", default data subId = "
                + SubscriptionManager.getDefaultDataSubscriptionId());

        // FIXME: See bug 17426028 maybe no conditional is needed.
        if (mPhone.getSubId() == SubscriptionManager.getDefaultDataSubscriptionId()) {
            setupDataOnAllConnectableApns(Phone.REASON_APN_CHANGED, RetryFailures.ALWAYS);
        } else {
            ApnContext apnContextDefault = mApnContexts.get(PhoneConstants.APN_TYPE_DEFAULT);
            if (apnContextDefault != null && apnContextDefault.isConnectable()) {
                log("Temp data switch is active , call setupDataOnAllConnectableApns()");
                setupDataOnAllConnectableApns(Phone.REASON_APN_CHANGED, RetryFailures.ALWAYS);
            }
        }
    }

    //MTK START: FDN Support
    private static final String FDN_CONTENT_URI = "content://icc/fdn";
    private static final String FDN_CONTENT_URI_WITH_SUB_ID = "content://icc/fdn/subId/";
    private static final String FDN_FOR_ALLOW_DATA = "*99#";
    private boolean mIsFdnChecked = false;
    private boolean mIsMatchFdnForAllowData = false;
    private boolean mIsPhbStateChangedIntentRegistered = false;
    private BroadcastReceiver mPhbStateChangedIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (DBG) {
                log("onReceive: action=" + action);
            }
            if (action.equals(TelephonyIntents.ACTION_PHB_STATE_CHANGED)) {
                boolean bPhbReady = intent.getBooleanExtra("ready", false);
                if (DBG) {
                    log("bPhbReady: " + bPhbReady);
                }
                if (bPhbReady) {
                    onFdnChanged();
                }
            }
        }
    };

    private void registerFdnContentObserver() {
        if (isFdnEnableSupport()) {
            Uri fdnContentUri;
            if (SubscriptionManager.isValidSubscriptionId(mPhone.getSubId())) {
                fdnContentUri = Uri.parse(FDN_CONTENT_URI_WITH_SUB_ID + mPhone.getSubId());
            } else {
                fdnContentUri = Uri.parse(FDN_CONTENT_URI);
            }
            mSettingsObserver.observe(fdnContentUri, MtkDctConstants.EVENT_FDN_CHANGED);
        }
    }

    private boolean isFdnEnableSupport() {
        boolean isFdnEnableSupport = false;
        if (mDataConnectionExt != null) {
            isFdnEnableSupport = mDataConnectionExt.isFdnEnableSupport();
        }
        return isFdnEnableSupport;
    }

    private boolean isFdnEnabled() {
        boolean bFdnEnabled = false;
        if (isFdnEnableSupport()) {
            IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(
                    ServiceManager.getService("phoneEx"));
            if (telephonyEx != null) {
                try {
                    bFdnEnabled = telephonyEx.isFdnEnabled(mPhone.getSubId());
                    log("isFdnEnabled(), bFdnEnabled = " + bFdnEnabled);
                    if (bFdnEnabled) {
                        if (mIsFdnChecked) {
                            log("isFdnEnabled(), match FDN for allow data = "
                                    + mIsMatchFdnForAllowData);
                            return !mIsMatchFdnForAllowData;
                        } else {
                            boolean bPhbReady = telephonyEx.isPhbReady(mPhone.getSubId());
                            log("isFdnEnabled(), bPhbReady = " + bPhbReady);
                            if (bPhbReady) {
                                mWorkerHandler.sendEmptyMessage(
                                        MtkDctConstants.EVENT_CHECK_FDN_LIST);
                            } else if (!mIsPhbStateChangedIntentRegistered) {
                                IntentFilter filter = new IntentFilter();
                                filter.addAction(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
                                mPhone.getContext().registerReceiver(
                                        mPhbStateChangedIntentReceiver, filter);
                                mIsPhbStateChangedIntentRegistered = true;
                            }
                        }
                    } else if (mIsPhbStateChangedIntentRegistered) {
                        mIsPhbStateChangedIntentRegistered = false;
                        mPhone.getContext().unregisterReceiver(mPhbStateChangedIntentReceiver);
                    }
                } catch (RemoteException ex) {
                    ex.printStackTrace();
                }
            } else {
                loge("isFdnEnabled(), get telephonyEx failed!!");
            }
        }
        return bFdnEnabled;
    }

    private void onFdnChanged() {
        if (isFdnEnableSupport()) {
            log("onFdnChanged()");
            boolean bFdnEnabled = false;
            boolean bPhbReady = false;

            IMtkTelephonyEx telephonyEx = IMtkTelephonyEx.Stub.asInterface(
                    ServiceManager.getService("phoneEx"));
            if (telephonyEx != null) {
                try {
                    bFdnEnabled = telephonyEx.isFdnEnabled(mPhone.getSubId());
                    bPhbReady = telephonyEx.isPhbReady(mPhone.getSubId());
                    log("onFdnChanged(), bFdnEnabled = " + bFdnEnabled
                            + ", bPhbReady = " + bPhbReady);
                } catch (RemoteException ex) {
                    ex.printStackTrace();
                }
            } else {
                loge("onFdnChanged(), get telephonyEx failed!!");
            }

            if (bPhbReady) {
                if (bFdnEnabled) {
                    log("fdn enabled, check fdn list");
                    mWorkerHandler.sendEmptyMessage(MtkDctConstants.EVENT_CHECK_FDN_LIST);
                } else {
                    log("fdn disabled, call setupDataOnAllConnectableApns()");
                    setupDataOnAllConnectableApns(MtkGsmCdmaPhone.REASON_FDN_DISABLED,
                            RetryFailures.ALWAYS);
                }
            } else if (!mIsPhbStateChangedIntentRegistered) {
                IntentFilter filter = new IntentFilter();
                filter.addAction(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
                mPhone.getContext().registerReceiver(
                        mPhbStateChangedIntentReceiver, filter);
                mIsPhbStateChangedIntentRegistered = true;
            }
        } else {
            log("not support fdn enabled, skip onFdnChanged");
        }
    }

    private void cleanOrSetupDataConnByCheckFdn() {
        log("cleanOrSetupDataConnByCheckFdn()");

        Uri uriFdn;
        if (SubscriptionManager.isValidSubscriptionId(mPhone.getSubId())) {
            uriFdn = Uri.parse(FDN_CONTENT_URI_WITH_SUB_ID + mPhone.getSubId());
        } else {
            uriFdn = Uri.parse(FDN_CONTENT_URI);
        }
        ContentResolver cr = mPhone.getContext().getContentResolver();
        Cursor cursor = cr.query(uriFdn, new String[] { "number" }, null, null, null);

        mIsMatchFdnForAllowData = false;
        if (cursor != null) {
            mIsFdnChecked = true;
            if (cursor.getCount() > 0) {
                if (cursor.moveToFirst()) {
                    do {
                        String strFdnNumber = cursor.getString(
                                cursor.getColumnIndexOrThrow("number"));
                        log("strFdnNumber = " + strFdnNumber);
                        if (strFdnNumber.equals(FDN_FOR_ALLOW_DATA)) {
                            mIsMatchFdnForAllowData = true;
                            break;
                        }
                    } while (cursor.moveToNext());
                }
            }
            cursor.close();
        }

        if (mIsMatchFdnForAllowData) {
            log("match FDN for allow data, call setupDataOnAllConnectableApns()");
            setupDataOnAllConnectableApns(MtkGsmCdmaPhone.REASON_FDN_DISABLED,
                    RetryFailures.ALWAYS);
        } else {
            log("not match FDN for allow data, call cleanUpAllConnections(REASON_FDN_ENABLED)");
            cleanUpAllConnectionsInternal(true, MtkGsmCdmaPhone.REASON_FDN_ENABLED);
        }
    }
    //MTK END: Support FDN

    // M: Worker Handler @{
    private void createWorkerHandler() {
        if (mWorkerHandler == null) {
            Thread thread = new Thread() {
                @Override
                public void run() {
                    Looper.prepare();
                    mWorkerHandler = new WorkerHandler();
                    Looper.loop();
                }
            };
            thread.start();
        }
    }

    private class WorkerHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MtkDctConstants.EVENT_CHECK_FDN_LIST:
                    cleanOrSetupDataConnByCheckFdn();
                    break;
            }
        }
    }
    // M: Worker Handler @}

    private boolean ignoreDataRoaming(String apnType) {
        boolean ignoreDataRoaming = false;
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();

        try {
            ignoreDataRoaming = mDataConnectionExt.ignoreDataRoaming(apnType);
        } catch (Exception e) {
            loge("get ignoreDataRoaming fail!");
            e.printStackTrace();
        }
        // Telenor requirement: MMS/XCAP should over ePDG in roaming state even roaming data is off.
        if (dcHelper.isOperatorMccMnc(MtkDcHelper.Operator.OP156, mPhone.getPhoneId())) {
            int iwlanRegState = ((MtkServiceState)mPhone.getServiceState()).getIwlanRegState();
            boolean isOverEpdg = (iwlanRegState == ServiceState.STATE_IN_SERVICE) ? true : false;
            log("ignoreDataRoaming: OP156 check apnType = " + apnType + ", Epdg=" + isOverEpdg);
            if (isOverEpdg && (apnType.equals(PhoneConstants.APN_TYPE_MMS) ||
                    apnType.equals(MtkPhoneConstants.APN_TYPE_XCAP))) {
                ignoreDataRoaming = true;
            }
        }

        if (ignoreDataRoaming) {
            logd("ignoreDataRoaming: " + ignoreDataRoaming + ", apnType = " + apnType);
        } else {
            MtkIccCardConstants.VsimType type =
                    MtkUiccController.getVsimCardType(mPhone.getPhoneId());
            if (type == MtkIccCardConstants.VsimType.REMOTE_SIM) {
                ignoreDataRoaming = true;
                log("RSim, set ignoreDataRoaming as true for any apn type");
            } else if (TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_VSIM)
                    && type == MtkIccCardConstants.VsimType.SOFT_AKA_SIM) {
                ignoreDataRoaming = true;
                log("Aka sim and soft sim, set ignoreDataRoaming as true for vsim type");
            }
        }

        return ignoreDataRoaming;
    }

    private boolean ignoreDefaultDataUnselected(String apnType) {
        boolean ignoreDefaultDataUnselected = false;

        try {
            ignoreDefaultDataUnselected = mDataConnectionExt.ignoreDefaultDataUnselected(apnType);
        } catch (Exception e) {
            loge("get ignoreDefaultDataUnselected fail!");
            e.printStackTrace();
        }

        if (ignoreDefaultDataUnselected) {
            logd("ignoreDefaultDataUnselected: " + ignoreDefaultDataUnselected
                    + ", apnType = " + apnType);
        }
        // M: Vsim
        if (!ignoreDefaultDataUnselected
                && TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_VSIM)) {
            log("Vsim is enabled, set ignoreDefaultDataUnselected as true");
            ignoreDefaultDataUnselected = true;
        }
        return ignoreDefaultDataUnselected;
    }

    private boolean ignoreDataAllow(String apnType) {
        boolean ignoreDataAllow = false;
        if (PhoneConstants.APN_TYPE_IMS.equals(apnType)) {
            ignoreDataAllow = true;
        }

        // M: Vsim
        if (!ignoreDataAllow && TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_VSIM)) {
            log("Vsim is enabled, set ignoreDataAllow as true");
            ignoreDataAllow = true;
        }

        return ignoreDataAllow;
    }

    // M: Data on domestic roaming. @{
    private boolean getDomesticRoamingEnabled() {
        // Even if PS registered on domestic roaming, check if is allowed by specific SIM.
        if (DBG) {
            log("getDomesticRoamingEnabled: isDomesticRoaming=" + isDomesticRoaming()
                    + ", bDomesticRoamingEnabled=" + getDomesticRoamingEnabledBySim());
        }
        return (isDomesticRoaming() && getDomesticRoamingEnabledBySim());
    }

    private boolean getIntlRoamingEnabled() {
        // No matter whether register on international roaming, check if is allowed by specific SIM.
        if (DBG) {
            log("getIntlRoamingEnabled: isIntlRoaming=" + isIntlRoaming()
                    + ", bIntlRoamingEnabled=" + mCcIntlRoamingEnabled);
        }
        return (isIntlRoaming() && mCcIntlRoamingEnabled);
    }

    private boolean isDomesticRoaming() {
        return mPhone.getServiceState().getDataRoamingType() ==
                ServiceState.ROAMING_TYPE_DOMESTIC;
    }

    private boolean isIntlRoaming() {
        return mPhone.getServiceState().getDataRoamingType() ==
                ServiceState.ROAMING_TYPE_INTERNATIONAL;
    }

    private void onRoamingTypeChanged() {
        boolean bDataOnRoamingEnabled = getDataRoamingEnabled();
        boolean bUserDataEnabled = mDataEnabledSettings.isUserDataEnabled();
        boolean bDomesticSpecialSim = getDomesticRoamingEnabledBySim();
        boolean bIntlSpecialSim = mCcIntlRoamingEnabled;
        boolean bDomAndIntRoamingFeatureEnabled = isUniqueRoamingFeatureEnabled();
        boolean trySetup = false;

        if (DBG) {
            log("onRoamingTypeChanged: bDataOnRoamingEnabled = " + bDataOnRoamingEnabled
                    + ", bUserDataEnabled = " + bUserDataEnabled
                    + ", bDomesticSpecialSim = " + bDomesticSpecialSim
                    + ", bIntlSpecialSim = " + bIntlSpecialSim
                    + ", bDomAndIntRoamingFeatureEnabled = " + bDomAndIntRoamingFeatureEnabled
                    + ", bOneSettingForRoamingFeatureEnabled = " + mCcOneSettingForRoaming
                    + ", roamingType = " + mPhone.getServiceState().getDataRoamingType());
        }

        // Check if the device is actually data roaming
        if (!mPhone.getServiceState().getDataRoaming()) {
            if (DBG) log("onRoamingTypeChanged: device is not roaming. ignored the request.");
            return;
        }

        /// M: For USCC(OP236) @{
        // If roaming type changed, call onDataRoamingOnOrSettingsChanged to handle new roaming
        // type, maybe need change roaming data setting and setup or cleanup dataconnection.
        if (mCcOneSettingForRoaming && TelephonyManager.getDefault().getSimCount() == 1) {
            onDataRoamingOnOrSettingsChanged(MtkDctConstants.EVENT_ROAMING_TYPE_CHANGED);
            return;
        }
        /// M: @}

        if (!bDomesticSpecialSim && !bIntlSpecialSim && !bDomAndIntRoamingFeatureEnabled) {
            if (DBG) log("onRoamingTypeChanged: is not specific SIM. ignored the request.");
            return;
        }

        // Only the roaming types changed between DOMESTIC and INTERNATIONAL is expected here,
        // if NOT, there might be something wrong with NW, suggest to discuss with them.
        if (bDomAndIntRoamingFeatureEnabled) {
            if (checkDomesticDataRoamingEnabled()
                    || checkInternationalDataRoamingEnabled()) {
                trySetup = true;
            } else {
                trySetup = false;
            }
        } else if (isDomesticRoaming()) {
            if (bDomesticSpecialSim) {
                if (bUserDataEnabled) {
                    trySetup = true;
                } else {
                    trySetup = false;
                }
            } else {
                // for normal sim, setup data needs both switches on
                trySetup = bUserDataEnabled && bDataOnRoamingEnabled;
            }
        } else if (isIntlRoaming()) {
            if (bIntlSpecialSim) {
                if (bDataOnRoamingEnabled) {
                    trySetup = true;
                } else {
                    trySetup = false;
                }
            } else {
                // for normal sim, setup data needs both switches on
                trySetup = bUserDataEnabled && bDataOnRoamingEnabled;
            }
        } else {
            loge("onRoamingTypeChanged error: unexpected roaming type");
        }

        if (trySetup) {
            if (DBG) log("onRoamingTypeChanged: setup data on roaming");
            setupDataOnAllConnectableApns(Phone.REASON_ROAMING_ON, RetryFailures.ALWAYS);
            mPhone.notifyDataConnection();
        } else {
            if (DBG) log("onRoamingTypeChanged: Tear down data connection on roaming.");
            cleanUpAllConnectionsInternal(true, Phone.REASON_ROAMING_ON);
        }
    }
    // M: Data on domestic roaming. @}

    /**
     * M: Called when EVENT_DISCONNECT_DONE is received.
     * Get retry timer for onDisconnectDone.
     */
    private long getDisconnectDoneRetryTimer(String reason, long delay) {
        long timer = delay;
        if (Phone.REASON_APN_CHANGED.equals(reason)) {
            // M: onApnChanged need retry quickly
            timer = 3000;
        } else if (mDataConnectionExt != null) {
            // M: for other specific reason
            try {
                timer = mDataConnectionExt.getDisconnectDoneRetryTimer(reason, timer);
            } catch (Exception e) {
                loge("DataConnectionExt.getDisconnectDoneRetryTimer fail!");
                e.printStackTrace();
            }
        }

        return timer;
    }

    // M: [OD over ePDG] start
    private ArrayList<ApnSetting> buildWifiApns(String requestedApnType) {
        if (DBG) log("buildWifiApns: E requestedApnType=" + requestedApnType);
        ArrayList<ApnSetting> apnList = new ArrayList<ApnSetting>();

        if (mAllApnSettings != null) {
            if (DBG) log("buildWaitingApns: mAllApnSettings=" + mAllApnSettings);
            int requestedApnTypeBitmask =
                    ApnSetting.getApnTypesBitmaskFromString(requestedApnType);
            for (ApnSetting apn : mAllApnSettings) {
                if (apn.canHandleType(requestedApnTypeBitmask)) {
                    if (isWifiOnlyApn(apn.getNetworkTypeBitmask())) {
                        apnList.add(apn);
                    }
                }
            }
        }
        if (DBG) log("buildWifiApns: X apnList=" + apnList);
        return apnList;
    }

    private boolean isWifiOnlyApn(int networkTypeBitmask) {
        int invertIWLANBitMask = ~(1 << (TelephonyManager.NETWORK_TYPE_IWLAN - 1)) & 0xffffff;

        if (networkTypeBitmask == 0) {
            return false;
        }
        return ((networkTypeBitmask & invertIWLANBitMask) == 0);
    }
    // M: [OD over ePDG] end

    // MTK
    public void deactivatePdpByCid(int cid) {
        mDataServiceManager.deactivateDataCall(cid, DataService.REQUEST_REASON_NORMAL,
                obtainMessage(MtkDctConstants.EVENT_RESET_PDP_DONE, cid, 0));
    }

    // M: [LTE][Low Power][UL traffic shaping] @{
    private void onSharedDefaultApnState(int newDefaultRefCount) {
        logd("onSharedDefaultApnState: newDefaultRefCount = " + newDefaultRefCount
                + ", curDefaultRefCount = " + mDefaultRefCount);

        if(newDefaultRefCount != mDefaultRefCount) {
            if (newDefaultRefCount > 1) {
                mIsSharedDefaultApn = true;
            } else {
                mIsSharedDefaultApn = false;
            }
            mDefaultRefCount = newDefaultRefCount;
            logd("onSharedDefaultApnState: mIsSharedDefaultApn = " + mIsSharedDefaultApn);
            broadcastSharedDefaultApnStateChanged(mIsSharedDefaultApn);
        }
    }

    public void onSetLteAccessStratumReport(boolean enabled, Message response) {
        ((MtkRIL) mPhone.mCi).setLteAccessStratumReport(enabled, response);
    }

    public void onSetLteUplinkDataTransfer(int timeMillis, Message response) {
        for(ApnContext apnContext : mApnContexts.values()) {
            if(PhoneConstants.APN_TYPE_DEFAULT.equals(apnContext.getApnType())) {
                try {
                    DataConnection dataConnection = apnContext.getDataConnection();
                    if (dataConnection != null) {
                        int interfaceId = dataConnection.getCid();
                        ((MtkRIL) mPhone.mCi)
                            .setLteUplinkDataTransfer(timeMillis, interfaceId, response);
                    }
                } catch (Exception e) {
                    loge("getDcAc fail!");
                    e.printStackTrace();
                    if (response != null) {
                        AsyncResult.forMessage(response, null,
                                new CommandException(CommandException.Error.GENERIC_FAILURE));
                        response.sendToTarget();
                    }
                }
            }
        }
    }

    private void notifyLteAccessStratumChanged(int lteAccessStratumDataState) {
        mLteAccessStratumDataState = (lteAccessStratumDataState == LTE_AS_CONNECTED) ?
                MtkPhoneConstants.LTE_ACCESS_STRATUM_STATE_CONNECTED :
                MtkPhoneConstants.LTE_ACCESS_STRATUM_STATE_IDLE;
        logd("notifyLteAccessStratumChanged mLteAccessStratumDataState = "
                + mLteAccessStratumDataState);
        broadcastLteAccessStratumChanged(mLteAccessStratumDataState);
    }

    private void notifyPsNetworkTypeChanged(int newRilNwType) {
        int newNwType = mPhone.getServiceState().rilRadioTechnologyToNetworkType(newRilNwType);
        logd("notifyPsNetworkTypeChanged mNetworkType = " + mNetworkType
                + ", newNwType = " + newNwType
                + ", newRilNwType = " + newRilNwType);
        if (newNwType != mNetworkType) {
            mNetworkType = newNwType;
            broadcastPsNetworkTypeChanged(mNetworkType);
        }
    }

    public String getLteAccessStratumState() {
        return mLteAccessStratumDataState;
    }

    public boolean isSharedDefaultApn() {
        return mIsSharedDefaultApn;
    }

    private void broadcastLteAccessStratumChanged(String state) {
        Intent intent = new Intent(TelephonyIntents.ACTION_LTE_ACCESS_STRATUM_STATE_CHANGED);
        intent.putExtra(MtkPhoneConstants.LTE_ACCESS_STRATUM_STATE_KEY, state);
        mPhone.getContext().sendBroadcastAsUser(intent, UserHandle.ALL,
                android.Manifest.permission.READ_PHONE_STATE);
    }

    private void broadcastPsNetworkTypeChanged(int nwType) {
        Intent intent = new Intent(TelephonyIntents.ACTION_PS_NETWORK_TYPE_CHANGED);
        intent.putExtra(MtkPhoneConstants.PS_NETWORK_TYPE_KEY, nwType);
        mPhone.getContext().sendBroadcastAsUser(intent, UserHandle.ALL,
                android.Manifest.permission.READ_PHONE_STATE);
    }

    private void broadcastSharedDefaultApnStateChanged(boolean isSharedDefaultApn) {
        Intent intent = new Intent(TelephonyIntents.ACTION_SHARED_DEFAULT_APN_STATE_CHANGED);
        intent.putExtra(MtkPhoneConstants.SHARED_DEFAULT_APN_KEY, isSharedDefaultApn);
        mPhone.getContext().sendBroadcastAsUser(intent, UserHandle.ALL,
                android.Manifest.permission.READ_PHONE_STATE);
    }
    // M: [LTE][Low Power][UL traffic shaping] @}

    // M: Is data allowed even if mobile data off
    private boolean isDataAllowedAsOff(String apnType) {
        boolean isDataAllowedAsOff = false;
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();

        if (mDataConnectionExt != null) {
            isDataAllowedAsOff = mDataConnectionExt.isDataAllowedAsOff(apnType);
        }

        // M: Data on domestic roaming. @{
        if (mCcIntlRoamingEnabled) {
            if (DBG) {
                log("isDataAllowedAsOff: getDataRoamingEnabled=" + getDataRoamingEnabled()
                        + ", bIsInternationalRoaming=" + isIntlRoaming());
            }

            if (getDataRoamingEnabled() && isIntlRoaming()) {
                // International data roaming is allowed even if mobile data off.
                isDataAllowedAsOff = true;
            }
        }
        // M: Data on domestic roaming. @}
        // M: Vsim
        if (!isDataAllowedAsOff && TextUtils.equals(apnType, MtkPhoneConstants.APN_TYPE_VSIM)) {
            MtkIccCardConstants.VsimType type =
                    MtkUiccController.getVsimCardType(mPhone.getPhoneId());
            if (type.isUserDataAllowed()) {
                log("Vsim is enabled, set isDataAllowedAsOff true");
                isDataAllowedAsOff = true;
            }
        }

        return isDataAllowedAsOff;
    }

    // M: [General Operator] Data Framework - WWOP requirements @{
    private boolean getDomesticDataRoamingEnabledFromSettings() {
        final int phoneId = mPhone.getPhoneId();

        boolean isDomDataRoamingEnabled = false;  // For Sprint default. Temp solution.
        try {
            isDomDataRoamingEnabled = (Settings.Global.getInt(mResolver,
                    MtkSettingsExt.Global.DOMESTIC_DATA_ROAMING + mPhone.getSubId()) != 0);
        } catch (SettingNotFoundException snfe) {
            if (DBG) log("getDomesticDataRoamingEnabled: SettingNofFoundException snfe=" + snfe);
        }
        if (VDBG) {
            log("getDomesticDataRoamingEnabled: phoneId=" + phoneId +
                    " isDomDataRoamingEnabled=" + isDomDataRoamingEnabled);
        }
        return isDomDataRoamingEnabled;
    }

    private boolean getInternationalDataRoamingEnabledFromSettings() {
        final int phoneId = mPhone.getPhoneId();

        boolean isIntDataRoamingEnabled = true;  // For Sprint default. Temp solution.
        try {
            isIntDataRoamingEnabled = (Settings.Global.getInt(mResolver,
                    MtkSettingsExt.Global.INTERNATIONAL_DATA_ROAMING + mPhone.getSubId()) != 0);
        } catch (SettingNotFoundException snfe) {
            if (DBG) log("getInternationalDataRoamingEnabled: SettingNofFoundException snfe=" +
                snfe);
        }
        if (VDBG) {
            log("getInternationalDataRoamingEnabled: phoneId=" + phoneId +
                    " isIntDataRoamingEnabled=" + isIntDataRoamingEnabled);
        }
        return isIntDataRoamingEnabled;
    }

    private boolean isDataRoamingTypeAllowed() {
        boolean isDataRoamingTypeAllowed = false;

        if (isUniqueRoamingFeatureEnabled()) {
            // Use SIM PLMN instead of Carrier config due to google issue.
            boolean bDomDataOnRoamingEnabled = getDomesticDataRoamingEnabledFromSettings();
            boolean bIntDataOnRoamingEnabled = getInternationalDataRoamingEnabledFromSettings();
            if (DBG) {
                log("isDataRoamingTypeAllowed bDomDataOnRoamingEnabled=" + bDomDataOnRoamingEnabled
                    + ", bIntDataOnRoamingEnabled=" + bIntDataOnRoamingEnabled
                    + ", getDataRoaming=" + mPhone.getServiceState().getDataRoaming()
                    + ", currentRoamingType=" + mPhone.getServiceState().getDataRoamingType()
                    + ", mUserDataEnabled=" + mDataEnabledSettings.isUserDataEnabled());
            }

            if (!mPhone.getServiceState().getDataRoaming()
                || ((bDomDataOnRoamingEnabled && isDomesticRoaming())
                || (bIntDataOnRoamingEnabled && isIntlRoaming()))) {
                isDataRoamingTypeAllowed = true;
            } else {
                isDataRoamingTypeAllowed = false;
            }
        }
        if (DBG) log("isDataRoamingTypeAllowed : " + isDataRoamingTypeAllowed);
        return isDataRoamingTypeAllowed;
    }
    // M: [General Operator] Data Framework - WWOP requirements @}

    // M: [pending data call during located plmn changing] @{
    private boolean mPendingDataCall = false;

    public boolean getPendingDataCallFlag() {
        return mPendingDataCall;
    }

    private boolean isLocatedPlmnChanged() {
        // cdma don't support EVENT_PS_NETWORK_STATE_CHANGED and don't need check this.
        if (mPhone.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            return false;
        }
        MtkServiceStateTracker sst = (MtkServiceStateTracker) mPhone.getServiceStateTracker();
        return sst.willLocatedPlmnChange();
    }

    private void onProcessPendingSetupData() {
        setupDataOnAllConnectableApns(MtkGsmCdmaPhone.REASON_RESUME_PENDING_DATA,
                RetryFailures.ALWAYS);
    }

    public void processPendingSetupData(MtkServiceStateTracker sst) {
        mPendingDataCall = false;
        sendMessage(obtainMessage(MtkDctConstants.EVENT_SETUP_PENDING_DATA));
    }
    // M: [pending data call during located plmn changing] @}

    /**
     * M: getClassType.
     *
     * @param apn ApnSetting
     * @return int for class type
     */
    public int getClassType(ApnSetting apn) {
        int classType = APN_CLASS_3;

        if (apn.canHandleType(ApnSetting.TYPE_EMERGENCY)
            || VZW_EMERGENCY_NI.compareToIgnoreCase(apn.getApnName()) == 0) {
            classType = APN_CLASS_0;
        } else if (apn.canHandleType(ApnSetting.TYPE_IMS)
            || VZW_IMS_NI.compareToIgnoreCase(apn.getApnName()) == 0) {
            classType = APN_CLASS_1;
        } else if (VZW_ADMIN_NI.compareToIgnoreCase(apn.getApnName()) == 0) {
            classType = APN_CLASS_2;
        } else if (VZW_APP_NI.compareToIgnoreCase(apn.getApnName()) == 0) {
            classType = APN_CLASS_4;
        } else if (VZW_800_NI.compareToIgnoreCase(apn.getApnName()) == 0) {
            classType = APN_CLASS_5;
        } else if (apn.canHandleType(ApnSetting.TYPE_DEFAULT)) {
            classType = APN_CLASS_3;
        } else {
            log("getClassType: set to default class 3");
        }

        logd("getClassType:" + classType);
        return classType;
    }

    /**
     * M: getClassTypeApn.
     *
     * @param classType APN class type
     * @return ApnSetting for class type apn
     */
    public ApnSetting getClassTypeApn(int classType) {
        ApnSetting classTypeApn = null;
        String apnName = "";

        if (APN_CLASS_0 == classType) {
            apnName = VZW_EMERGENCY_NI;
        } else if (APN_CLASS_1 == classType) {
            apnName = VZW_IMS_NI;
        } else if (APN_CLASS_2 == classType) {
            apnName = VZW_ADMIN_NI;
        } else if (APN_CLASS_3 == classType) {
            apnName = VZW_INTERNET_NI;
        } else if (APN_CLASS_4 == classType) {
            apnName = VZW_APP_NI;
        } else if (APN_CLASS_5 == classType) {
            apnName = VZW_800_NI;
        } else {
            log("getClassTypeApn: can't handle class:" + classType);
            return null;
        }

        if (mAllApnSettings != null) {
            for (ApnSetting apn : mAllApnSettings) {
                if (apnName.compareToIgnoreCase(apn.getApnName()) == 0) {
                    classTypeApn = apn;
                }
            }
        }

        logd("getClassTypeApn:" + classTypeApn + ", class:" + classType);
        return classTypeApn;
    }

    // M: JPN IA Start
    private void handleSetResume() {
        if (!SubscriptionManager.isValidPhoneId(mPhone.getPhoneId())) return;
        ((MtkRIL)mPhone.mCi).setResumeRegistration(mSuspendId, null);
    }

    private void handleRegistrationSuspend(AsyncResult ar) {
        if (ar.exception == null && ar.result != null) {
            if (DBG) log("handleRegistrationSuspend: createAllApnList and set initial attach APN");
            mSuspendId = ((int[]) ar.result)[0];
            log("handleRegistrationSuspend: suspending with Id=" + mSuspendId);
            synchronized (mNeedsResumeModemLock) {
                mNeedsResumeModem = true;
            }
            createAllApnList();
            setInitialAttachApn();
        } else {
            log("handleRegistrationSuspend: AsyncResult is wrong " + ar.exception);
        }
    }


    private void handlePlmnChange(AsyncResult ar) {
        if (ar.exception == null && ar.result != null) {
            String[] plmnString = (String[]) ar.result;

            for (int i = 0; i < plmnString.length; i++) {
                logd("plmnString[" + i + "]=" + plmnString[i]);
            }
            mRegion = getRegion(plmnString[0]);

            IccRecords r = mIccRecords.get();
            String operator = mtkGetOperatorNumeric(r);
            if (!TextUtils.isEmpty(operator) &&
                    isNeedToResumeMd() == false &&
                    mPhone.getPhoneId() ==
                            SubscriptionManager.getPhoneId(
                            SubscriptionController.getInstance().getDefaultDataSubId())){
                logd("handlePlmnChange: createAllApnList and set initial attach APN");
                createAllApnList();
                setInitialAttachApn();
            } else {
                logd("No need to update APN for Operator");
            }
        } else {
            log("AsyncResult is wrong " + ar.exception);
        }
    }

    private int getRegion(String plmn) {
        String currentMcc;
        if (plmn == null || plmn.equals("") || plmn.length() < 5) {
            logd("[getRegion] Invalid PLMN");
            return REGION_UNKNOWN;
        }

        currentMcc = plmn.substring(0, 3);
        for (String mcc : MCC_TABLE_TEST) {
            if (currentMcc.equals(mcc)) {
                logd("[getRegion] Test PLMN");
                return REGION_UNKNOWN;
            }
        }

        for (String mcc : MCC_TABLE_DOMESTIC) {
            if (currentMcc.equals(mcc)) {
                logd("[getRegion] REGION_DOMESTIC");
                return REGION_DOMESTIC;
            } else {
                logd("[getRegion] REGION_FOREIGN");
                return REGION_FOREIGN;
            }
        }
        logd("[getRegion] REGION_UNKNOWN");
        return REGION_UNKNOWN;
    }

    public boolean getImsEnabled() {
        //boolean isImsEnabled = (ImsManager.isVolteEnabledByPlatform(mPhone.getContext()) &&
        //        ImsManager.isEnhanced4gLteModeSettingEnabledByUser(mPhone.getContext()));
        //logd("getImsEnabled: getInt isImsEnabled=" + isImsEnabled);
        return false;//isImsEnabled;
    }

    public boolean checkIfDomesticInitialAttachApn(String currentMcc) {
        boolean isMccDomestic = false;

        for (String mcc : MCC_TABLE_DOMESTIC) {
            if (currentMcc.equals(mcc)) {
                isMccDomestic = true;
                break;
            }
        }
        if (isOp17IaSupport()&& isMccDomestic) {
            if (getImsEnabled()) {
                return mRegion == REGION_DOMESTIC;
            } else {
                return false;
            }
        }
        if (enableOpIA()) {
            return mRegion == REGION_DOMESTIC;
        }

        if (DBG) {
            log("checkIfDomesticInitialAttachApn: Not OP129 or MCC is not in domestic for OP129");
        }

        return true;
    }

    public boolean enableOpIA() {
        IccRecords r = mIccRecords.get();
        String operatorNumeric = mtkGetOperatorNumeric(r);
        if (TextUtils.isEmpty(operatorNumeric)) {
            return false;
        }
        String simOperator = operatorNumeric.substring(0, 3);
        log("enableOpIA: currentMcc = " + simOperator);

        for (String mcc : MCC_TABLE_DOMESTIC) {
            if (simOperator.equals(mcc)) {
                return true;
            }
        }
        return false;
    }

    /**
     * M: VzW feature, change IA in EPDG handover case.
     */
    private void onAttachApnChangedByHandover(boolean isImsHandover) {
        mIsImsHandover = isImsHandover;
        log("onAttachApnChangedByHandover: mIsImsHandover = " + mIsImsHandover);
        setInitialAttachApn();
    }

    private boolean isOp17IaSupport() {
        String value = TelephonyManager.getTelephonyProperty(
                mPhone.getPhoneId(), "vendor.gsm.ril.sim.op17", "0");
        return value.equals("1") ? true : false;
    }

    private boolean isOp129IaSupport() {
        return SystemProperties.get("vendor.gsm.ril.sim.op129").equals("1") ? true : false;
    }

    private boolean isNeedToResumeMd() {
        return SystemProperties.get("vendor.gsm.ril.data.op.suspendmd").equals("1") ? true : false;
    }

    private boolean isOp18Sim() {
        IccRecords r = mIccRecords.get();
        String operator = mtkGetOperatorNumeric(r);

        if (operator != null) {
            for (int i = 0; i < MCCMNC_OP18.length; i++) {
                if (operator.startsWith(MCCMNC_OP18[i])) {
                    return true;
                }
            }
        }
        return false;
    }

    private boolean hasOperatorIaCapability() {
        if (mTelDevController != null &&
                mTelDevController.getModem(0) != null &&
                ((MtkHardwareConfig)mTelDevController.getModem(0))
                        .hasOperatorIaCapability() == true) {
            log("hasOpIaCapability: true");
            return true;
        }
        return false;
    }

    // M: Multi-PS Attach @{
    private void onAllowChanged(boolean allow) {
        if (DBG) {
            log("onAllowChanged: Allow = " + allow);
        }

        mAllowConfig = allow;
        if (allow) {
            setupDataOnAllConnectableApns(MtkGsmCdmaPhone.REASON_DATA_ALLOWED,
                    RetryFailures.ALWAYS);
        }
    }

    protected boolean getAllowConfig() {
        if(!MtkDcHelper.getInstance().isMultiPsAttachSupport()) {
            return true;
        } else {
            return hasModemDeactPdnCapabilityForMultiPS() ? true : mAllowConfig;
        }
    }
    // M: Multi-PS Attach @}

    // M: PS/CS concurrent @{
    public void onVoiceCallStartedEx() {
        if (DBG) log("onVoiceCallStartedEx");
        mInVoiceCall = true;
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();
        mIsSupportConcurrent = (dcHelper == null) ? false :
                dcHelper.isDataAllowedForConcurrent(mPhone.getPhoneId());

        if (isConnected() && !mIsSupportConcurrent) {
            if (DBG) log("onVoiceCallStarted stop polling");
            stopNetStatPoll();
            stopDataStallAlarm();
            mPhone.notifyDataConnection();
        }
        notifyVoiceCallEventToDataConnection(mInVoiceCall, mIsSupportConcurrent);
    }

    public void onDsdaStateChanged() {
        if (DBG) log("onDsdaStateChanged");
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();
        mIsSupportConcurrent = (dcHelper == null) ? false :
                dcHelper.isDataAllowedForConcurrent(mPhone.getPhoneId());

        if (isConnected()) {
            if (!mIsSupportConcurrent) {
                stopNetStatPoll();
                stopDataStallAlarm();
            } else {
                startNetStatPoll();
                startDataStallAlarm(DATA_STALL_NOT_SUSPECTED);
            }
            mPhone.notifyDataConnection();
            notifyVoiceCallEventToDataConnection(mInVoiceCall, mIsSupportConcurrent);
        }
    }

    private void onWifiStateChanged(boolean enabled) {
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();
        mIsSupportConcurrent = (dcHelper == null) ? false :
                dcHelper.isDataAllowedForConcurrent(mPhone.getPhoneId());

        if (DBG) log("onWifiStateChanged, wifi enabled = " + enabled
                + ", mInVoiceCall = " + mInVoiceCall
                + ", mIsSupportConcurrent = " + mIsSupportConcurrent);

        if (mInVoiceCall && isConnected()) {
            if (!enabled && !mIsSupportConcurrent) {
                if (DBG) log("onWifiStateChanged: wifi disabled and not support concurrent");
                stopNetStatPoll();
                stopDataStallAlarm();
                mPhone.notifyDataConnection();
                notifyVoiceCallEventToDataConnection(mInVoiceCall, mIsSupportConcurrent);
            } else if (enabled && mIsSupportConcurrent) {
                if (DBG) log("onWifiStateChanged: wifi enabled and support concurrent");
                startNetStatPoll();
                startDataStallAlarm(DATA_STALL_NOT_SUSPECTED);
                mPhone.notifyDataConnection();
                notifyVoiceCallEventToDataConnection(mInVoiceCall, mIsSupportConcurrent);
            }
        }
    }

    public void onVoiceCallEndedEx() {
        if (DBG) log("onVoiceCallEndedEx");
        mInVoiceCall = false;
        boolean prevIsSupportConcurrent = mIsSupportConcurrent;
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();
        mIsSupportConcurrent = (dcHelper == null) ? false :
                dcHelper.isDataAllowedForConcurrent(mPhone.getPhoneId());

        if (isConnected()) {
            if (!prevIsSupportConcurrent || mIsSupportConcurrent) {
                startNetStatPoll();
                startDataStallAlarm(DATA_STALL_NOT_SUSPECTED);
                mPhone.notifyDataConnection();
            } else {
                // clean slate after call end.
                resetPollStats();
            }
        }

        /* M: svlte denali, after one call end, it maybe in call&concurrent, should update.
         *    keep this after if(isConnected()) to ensure onVoiceCallStartedEx/EndedEx
         *    notifyDataConnection is invoked in a couple.
         */
        if (MtkDcHelper.MTK_SVLTE_SUPPORT) {
            mIsSupportConcurrent = (dcHelper == null) ? false :
                    dcHelper.isDataSupportConcurrent(mPhone.getPhoneId());
            if (dcHelper != null && !dcHelper.isAllCallingStateIdle()) {
                mInVoiceCall = true;
                if (DBG) log("SVLTE denali dual call one end, left one call.");
            }
        }
        // reset reconnect timer
        setupDataOnAllConnectableApns(Phone.REASON_VOICE_CALL_ENDED, RetryFailures.ALWAYS);
        notifyVoiceCallEventToDataConnection(mInVoiceCall, mIsSupportConcurrent);
    }

    private void notifyVoiceCallEventToDataConnection(boolean bInVoiceCall,
            boolean bSupportConcurrent) {
        logd("notifyVoiceCallEventToDataConnection: bInVoiceCall = " + bInVoiceCall
                + ", bSupportConcurrent = " + bSupportConcurrent);
        for (DataConnection dc : mDataConnections.values()) {
                ((MtkDataConnection) dc).notifyVoiceCallEvent(bInVoiceCall, bSupportConcurrent);
        }
    }
    // M: PS/CS concurrent @}

    // M: Data on domestic roaming. @{
    private boolean getDomesticRoamingEnabledBySim() {
        if (mCcDomesticRoamingEnabled) {
            if (mCcDomesticRoamingSpecifiedNw != null) {
                return ArrayUtils.contains(mCcDomesticRoamingSpecifiedNw,
                        TelephonyManager.getDefault().getNetworkOperatorForPhone(
                            mPhone.getPhoneId()));
            }
            return true;
        } else {
            return false;
        }
    }
    // M: Data on domestic roaming. @}

    private boolean isImsApnSettingChanged(ArrayList<ApnSetting> prevApnList,
                                           ArrayList<ApnSetting> currApnList)
    {
        boolean bImsApnChanged = false;
        String prevImsApn = getImsApnSetting(prevApnList);
        String currImsApn = getImsApnSetting(currApnList);

        if (!prevImsApn.isEmpty()) {
            if (!TextUtils.equals(prevImsApn, currImsApn)) {
                bImsApnChanged = true;
            }
        }

        return bImsApnChanged;
    }

    private String getImsApnSetting(ArrayList<ApnSetting> apnSettings) {
        if (apnSettings == null || apnSettings.size() == 0) {
            return "";
        }

        StringBuilder sb = new StringBuilder();
        for (ApnSetting apnSetting : apnSettings) {
            if (apnSetting.canHandleType(ApnSetting.TYPE_IMS)) {
                sb.append(((MtkApnSetting)apnSetting).toStringIgnoreName(true));
            }
        }
        log("getImsApnSetting, apnsToStringIgnoreName: sb = " + sb.toString());
        return sb.toString();
    }

    // M: [General Operator] Data Framework - WWOP requirements @{
    private boolean checkDomesticDataRoamingEnabled() {
        if (DBG) {
            log("checkDomesticDataRoamingEnabled: getDomesticDataRoamingFromSettings="
                    + getDomesticDataRoamingEnabledFromSettings()
                    + ", isDomesticRoaming=" + isDomesticRoaming());
        }

        return getDomesticDataRoamingEnabledFromSettings() && isDomesticRoaming();
    }

    private boolean checkInternationalDataRoamingEnabled() {
        if (DBG) {
            log("checkInternationalDataRoamingEnabled: getInternationalDataRoamingFromSettings="
                    + getInternationalDataRoamingEnabledFromSettings()
                    + ", isIntlRoaming=" + isIntlRoaming());
        }

        return getInternationalDataRoamingEnabledFromSettings() && isIntlRoaming();
    }
    // M: [General Operator] Data Framework - WWOP requirements @}

    private boolean hasModemDeactPdnCapabilityForMultiPS() {
        // fetch for the first call
        // and use cache for the consecutive calls
        if (!mHasFetchModemDeactPdnCapabilityForMultiPS) {
            if (mTelDevController != null &&
                    mTelDevController.getModem(0) != null &&
                    ((MtkHardwareConfig)mTelDevController.getModem(0))
                    .hasModemDeactPdnCapabilityForMultiPS() == true) {
                mModemDeactPdnCapabilityForMultiPS = true;
            } else {
                mModemDeactPdnCapabilityForMultiPS = false;
            }
            mHasFetchModemDeactPdnCapabilityForMultiPS = true;
            log("hasModemDeactPdnCapabilityForMultiPS: " + mModemDeactPdnCapabilityForMultiPS);
        }
        return mModemDeactPdnCapabilityForMultiPS;
    }

    // M: IMS E911 Bearer Management @{
    private void teardownDataByEmergencyPolicy() {
        CarrierConfigManager configManager = (CarrierConfigManager) mPhone.getContext()
                .getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = null;
        String[] disConnectApns;
        boolean isDeactPdn = false;

        if (configManager != null) {
            int subId = mPhone.getSubId();
            b = configManager.getConfigForSubId(subId);
        }

        if (b != null) {
            disConnectApns = b.getStringArray(MtkCarrierConfigManager
                    .KEY_EMERGENCY_BEARER_MANAGEMENT_POLICY);
            for (String name : disConnectApns) {
                for (ApnContext apnContext : mApnContexts.values()) {
                    if (!apnContext.isDisconnected()) {
                        ApnSetting apnSetting = apnContext.getApnSetting();
                        log("compare apn: " + apnSetting.getApnName()+ " by filter: " + name);
                        if (apnSetting.getApnName().equalsIgnoreCase(name)) {
                            isDeactPdn = true;
                            apnContext.setReason(Phone.MTK_REASON_PDN_OCCUPIED);
                            cleanUpConnection(apnContext);
                            break;
                        }
                    }
                }
            }
        } else {
            loge("Couldn't find CarrierConfigService.");
        }
    }
    /// @}

    // M: Data Framework - common part enhancement @{
    private boolean isApnSettingExist(ApnSetting apnSetting) {
        if (apnSetting != null && mAllApnSettings != null && !mAllApnSettings.isEmpty()) {
            for (ApnSetting apn : mAllApnSettings) {
                if (TextUtils.equals(((MtkApnSetting) apnSetting).toStringIgnoreName(false),
                        ((MtkApnSetting) apn).toStringIgnoreName(false))) {
                    log("isApnSettingExist: " + apn);
                    return true;
                }
            }
        }
        return false;
    }
    // M: Data Framework - common part enhancement @}

    /**
     * Anchor method of DcTracer constructor to copy the reference of the handler thread.
     */
    @Override
    protected void mtkCopyHandlerThread(HandlerThread t) {
        mDcHandlerThread = t;
    }

    /**
     * Anchor method of createApnList
     *
     * @return whether the current operator if need to updata total Tx/Rx packets.
     */
    @Override
    protected void mtkUpdateTotalTxRxSum() {
        IccRecords r = mIccRecords.get();
        String strOperatorNumeric = mtkGetOperatorNumeric(r);

        if (strOperatorNumeric != null) {
            for (int i = 0; i < PRIVATE_APN_OPERATOR.length; i++) {
                if (strOperatorNumeric.startsWith(PRIVATE_APN_OPERATOR[i])) {
                    mDataStallTxRxSum.updateTotalTxRxSum();
                    break;
                }
            }
        }
    }

    /**
     * Anchor method of onDisconnectDone to modify inter apn delay based on current type.
     */
    @Override
    protected long mtkModifyInterApnDelay(long delay, ApnContext apnContext) {
        // 1. Reconnect vsim PDN as sooner as possible, the delay timer currently is 1s.
        // 2. For apn changed case, we rely on the retry, thus reduce the delay timer to 1s.
        if (MtkPhoneConstants.APN_TYPE_VSIM.equals(apnContext.getApnType())
                || (PhoneConstants.APN_TYPE_DEFAULT.equals(apnContext.getApnType())
                    && Phone.REASON_APN_CHANGED.equals(apnContext.getReason()))) {
            delay = 1000;
        }
        return delay;
    }

    private void addVsimApnTypeToDefaultApnSetting() {
        if (ExternalSimManager.isNonDsdaRemoteSimSupport() && mAllApnSettings != null) {
            for (int i = 0; i < mAllApnSettings.size(); i++) {
                ApnSetting apnSetting = mAllApnSettings.get(i);
                if (apnSetting.canHandleType(ApnSetting.TYPE_DEFAULT)) {
                    apnSetting = new MtkApnSetting(apnSetting.getId(),
                            apnSetting.getOperatorNumeric(), apnSetting.getEntryName(),
                            apnSetting.getApnName(), apnSetting.getProxyAddressAsString(),
                            apnSetting.getProxyPort(), apnSetting.getMmsc(),
                            apnSetting.getMmsProxyAddressAsString(), apnSetting.getMmsProxyPort(),
                            apnSetting.getUser(), apnSetting.getPassword(),
                            apnSetting.getAuthType(),
                            apnSetting.getApnTypeBitmask() | MtkApnSetting.TYPE_VSIM,
                            apnSetting.getProtocol(), apnSetting.getRoamingProtocol(),
                            apnSetting.isEnabled(), apnSetting.getNetworkTypeBitmask(),
                            apnSetting.getProfileId(), apnSetting.isPersistent(),
                            apnSetting.getMaxConns(), apnSetting.getWaitTime(),
                            apnSetting.getMaxConnsTime(), apnSetting.getMtu(),
                            apnSetting.getMvnoType(), apnSetting.getMvnoMatchData(),
                            apnSetting.getApnSetId(), apnSetting.getCarrierId(),
                            apnSetting.getSkip464Xlat(),
                            ((MtkApnSetting) apnSetting).inactiveTimer);
                    mAllApnSettings.set(i, apnSetting);
                }
            }
        }
    }

    /**
     * Anchor method of notifyOffApnsOfAvailability to decide if skipping notification.
     */
    @Override
    protected boolean mtkIsNeedNotify(ApnContext apnContext) {
        if (TextUtils.equals(apnContext.getApnType(), PhoneConstants.APN_TYPE_DEFAULT)
                && apnContext.getState() == DctConstants.State.CONNECTED) {
            logd("mtkIsNeedNotify: do not notify state for default apn");
            return false;
        }
        return ((MtkApnContext) apnContext).isNeedNotify();
    }

    /**
     * Anchor method of onDataSetupComplete to decide if the cause is permanent.
     */
    protected boolean mtkIsPermanentFailure(@DataFailCause.FailCause int dcFailCause) {
        boolean bPermanent = false;

        if (mDataConnectionExt != null) {
            try {
                bPermanent = mDataConnectionExt.isPermanentCause(dcFailCause);
            } catch (Exception e) {
                logd("mDataConnectionExt.isPermanentCause exception");
                e.printStackTrace();
            }
        }
        if (dcFailCause == DataFailCause.MTK_TCM_ESM_TIMER_TIMEOUT) {
            IccRecords r = mIccRecords.get();
            String strOperatorNumeric = mtkGetOperatorNumeric(r);
            if (strOperatorNumeric != null) {
                for (int i = 0; i < KDDI_OPERATOR.length; i++) {
                    if (strOperatorNumeric.startsWith(KDDI_OPERATOR[i])) {
                        bPermanent = true;
                        break;
                    }
                }
            }
        }
        return bPermanent;
    }

    /**
     * Anchor method of initEmergencyApnSetting to get emergency apn selection.
     */
    protected String mtkGetEmergencyApnSelection(String selection) {
        return selection + " and numeric=''";
    }

    private void isDataAllowedForRoamingFeature(DataConnectionReasons dataConnectionReasons) {
        // M: [General Operator] Data Framework - WWOP requirements {@
        if (isUniqueRoamingFeatureEnabled()) {
            // The criteria is not the same as AOSP for some operators which need to check
            // Domestic roaming and international roaming.
            // So remove the roaming disable reason first from AOSP.
            if (dataConnectionReasons.contains(
                    DataDisallowedReasonType.ROAMING_DISABLED)) {
                dataConnectionReasons.mDataDisallowedReasonSet.remove(
                        DataDisallowedReasonType.ROAMING_DISABLED);
            }

            if (!isDataRoamingTypeAllowed()) {
                dataConnectionReasons.add(DataDisallowedReasonType.ROAMING_DISABLED);
            }
        }
        // M: [General Operator] Data Framework - WWOP requirements @}
    }

    /// Ims Data Framework @{
    private void onDedecatedBearerActivated(
                MtkDedicateDataCallResponse dataResponse) {
        log("onDedecatedBearerActivated, dataInfo: " + dataResponse);
        notifyDedicateDataConnection(
            dataResponse.mCid, DctConstants.State.CONNECTED, dataResponse,
            DataFailCause.NONE, MtkDedicateDataCallResponse.REASON_BEARER_ACTIVATION);
    }

    private void onDedecatedBearerModified(
                MtkDedicateDataCallResponse dataResponse) {
        log("onDedecatedBearerModified, dataInfo: " + dataResponse);
        notifyDedicateDataConnection(
            dataResponse.mCid, DctConstants.State.CONNECTED, dataResponse,
            DataFailCause.NONE, MtkDedicateDataCallResponse.REASON_BEARER_MODIFICATION);
    }

    private void onDedecatedBearerDeactivated(int cid) {
        log("onDedecatedBearerDeactivated, Cid: " + cid);
        notifyDedicateDataConnection(
            cid, DctConstants.State.IDLE, null, DataFailCause.NONE,
            MtkDedicateDataCallResponse.REASON_BEARER_DEACTIVATION);
    }

    private void notifyDedicateDataConnection(
        int ddcId, DctConstants.State state, MtkDedicateDataCallResponse dataInfo,
        @DataFailCause.FailCause int failCause, String reason)
    {
        //reason is used to know if the notification is triggered
        //by bearer activation or deactivation
        log("notifyDedicateDataConnection ddcId=" + ddcId +
            ", state=" + state + ", failCause=" + failCause + ", reason="
            + reason + ", dataInfo=" + dataInfo);
        Intent intent = new Intent(
            TelephonyIntents.ACTION_ANY_DEDICATE_DATA_CONNECTION_STATE_CHANGED);
        intent.putExtra("DdcId", ddcId);
        if (dataInfo != null && dataInfo.mCid >= 0)
            intent.putExtra(PhoneConstants.DATA_LINK_PROPERTIES_KEY, dataInfo);

        intent.putExtra(PhoneConstants.STATE_KEY, state);
        intent.putExtra("cause", failCause);
        //intent.putExtra(PhoneConstants.STATE_CHANGE_REASON_KEY, reason);
        //For state is connected, use reason to know the
        //notification is triggered by modification or activation
        intent.putExtra(PhoneConstants.PHONE_KEY, mPhone.getPhoneId());
        mPhone.getContext().sendBroadcastAsUser(intent, UserHandle.ALL,
                android.Manifest.permission.READ_PRECISE_PHONE_STATE);
    }
    /// @}

    private void loadCarrierConfig(int subId) {
        CarrierConfigManager configManager = (CarrierConfigManager) mPhone.getContext()
                .getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = null;

        if (configManager != null) {
            b = configManager.getConfigForSubId(subId);
        }
        if (b != null) {
            mCcDomesticRoamingEnabled = b.getBoolean(MtkCarrierConfigManager.
                    KEY_DOMESTIC_ROAMING_ENABLED_ONLY_BY_MOBILE_DATA_SETTING);
            mCcDomesticRoamingSpecifiedNw = b.getStringArray(MtkCarrierConfigManager.
                    KEY_DOMESTIC_ROAMING_ENABLED_ONLY_BY_MOBILE_DATA_SETTING_CHECK_NW_PLMN);
            mCcIntlRoamingEnabled = b.getBoolean(MtkCarrierConfigManager.
                    KEY_INTL_ROAMING_ENABLED_ONLY_BY_ROAMING_DATA_SETTING);
            // Two roaming data settings for domestic and international roaming.
            // Should add domestic and international roaming data settings in UI
            // and will hide default roaming data setting.
            mCcUniqueSettingsForRoaming = b.getBoolean(MtkCarrierConfigManager.
                    KEY_UNIQUE_SETTINGS_FOR_DOMESTIC_AND_INTL_ROAMING);
            // One roaming data setting for domestic and international roaming.
            // Set roaming data setting enabled in domestic roaming area, disabled in
            // international roaming area. Currently only for USCC(OP236).
            mCcOneSettingForRoaming = b.getBoolean(MtkCarrierConfigManager.
                    MTK_KEY_ONE_SETTING_FOR_DOMESTIC_AND_INTL_ROAMING_DATA);
            mIsAddMnoApnsIntoAllApnList = b.getBoolean(MtkCarrierConfigManager.
                    MTK_KEY_ADD_MNOAPNS_INTO_ALLAPNLIST);

            log("loadCarrierConfig: DomesticRoamingEnabled " + mCcDomesticRoamingEnabled
                    + ", SpecifiedNw " + (mCcDomesticRoamingSpecifiedNw != null)
                    + ", IntlRoamingEnabled " + mCcIntlRoamingEnabled
                    + ", UniqueSettingsForRoaming " + mCcUniqueSettingsForRoaming
                    + ", OneSettingForRoaming " + mCcOneSettingForRoaming
                    + ", IsAddMnoApnsIntoAllApnList " + mIsAddMnoApnsIntoAllApnList);

            /// M: For USCC(OP236) @{
            // If roaming type changed before carrier config loaded,
            // call onDataRoamingOnOrSettingsChanged to handle roaming data event.
            if (mCcOneSettingForRoaming && TelephonyManager.getDefault().getSimCount() == 1) {
                onDataRoamingOnOrSettingsChanged(MtkDctConstants.EVENT_CARRIER_CONFIG_LOADED);
            }
            /// M: @}
        }
    }

    private boolean hasMdAutoSetupImsCapability() {
        if (!mHasFetchMdAutoSetupImsCapability) {
            if (mTelDevController != null && mTelDevController.getModem(0) != null
                    && ((MtkHardwareConfig) mTelDevController.getModem(0))
                    .hasMdAutoSetupImsCapability() == true) {
                mMdAutoSetupImsCapability = true;
            }
            mHasFetchMdAutoSetupImsCapability = true;
            logd("hasMdAutoSetupImsCapability: " + mMdAutoSetupImsCapability);
        }
        return mMdAutoSetupImsCapability;
    }

    private void reloadOpCustomizationFactory() {
        try {
            if (mDataConnectionExt != null) {
                mDataConnectionExt.stopDataRoamingStrategy();
            }

            mTelephonyCustomizationFactory =
                    OpTelephonyCustomizationUtils.getOpFactory(mPhone.getContext());
            mDataConnectionExt =
                    mTelephonyCustomizationFactory.makeDataConnectionExt(mPhone.getContext());

            if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WWAN) {
                mDataConnectionExt.startDataRoamingStrategy(mPhone);
            }
        } catch (Exception e) {
            if (DBG) {
                log("mDataConnectionExt init fail");
            }
            e.printStackTrace();
        }
    }

    private boolean isSimActivated() {
        // For sprint, the value of gid1 will be all F when sim is non-activation
        String gid1 = SystemProperties.get(PROP_RIL_DATA_GID1+mPhone.getPhoneId(), "");
        if (DBG) log("gid1: " + gid1);
        if (GID1_DEFAULT.compareToIgnoreCase(gid1) == 0) {
            return false;
        }
        return true;
    }

    /**
     * Anchor method of onSubscriptionsChanged to decide if skipping registerSettingsObserver.
     * @param pSubId the previous subId.
     * @param subId the current subId.
     *
     * @return {@code true} if the current subId has been changed from the previous subId.
     */
    @Override
    protected boolean mtkIsNeedRegisterSettingsObserver(int pSubId, int subId) {
        if (DBG) log("mtkIsNeedRegisterSettingsObserver: pSubId=" + pSubId + ", subId=" + subId);
        return (pSubId != subId);
    }

    private ApnSetting encodeInactiveTimer(ApnSetting apn) {
        if (apn == null) {
            loge("encodeInactiveTimer apn is null");
            return null;
        }

        if (apn.getAuthType() > MtkPhoneConstants.APN_AUTH_TYPE_MAX_NUM
                || apn.getAuthType() < -1) {
            loge("encodeInactiveTimer invalid authType: " + apn.getAuthType());
        } else if (apn instanceof MtkApnSetting) {
            int inactTimer = (((MtkApnSetting)apn).inactiveTimer < 0) ? 0 :
                    (((MtkApnSetting)apn).inactiveTimer > MtkPhoneConstants.APN_MAX_INACTIVE_TIMER)
                    ? MtkPhoneConstants.APN_MAX_INACTIVE_TIMER : ((MtkApnSetting)apn).inactiveTimer;
            int authType = ((apn.getAuthType() == -1) ? TextUtils.isEmpty(apn.getUser()) ?
                    RILConstants.SETUP_DATA_AUTH_NONE : RILConstants.SETUP_DATA_AUTH_PAP_CHAP
                    : apn.getAuthType()) + (inactTimer << MtkPhoneConstants.APN_INACTIVE_TIMER_KEY);

            return ApnSetting.makeApnSetting(apn.getId(),
                    apn.getOperatorNumeric(), apn.getEntryName(),
                    apn.getApnName(), apn.getProxyAddressAsString(),
                    apn.getProxyPort(), apn.getMmsc(),
                    apn.getMmsProxyAddressAsString(), apn.getMmsProxyPort(),
                    apn.getUser(), apn.getPassword(), authType,
                    apn.getApnTypeBitmask(),
                    apn.getProtocol(), apn.getRoamingProtocol(),
                    apn.isEnabled(), apn.getNetworkTypeBitmask(),
                    apn.getProfileId(), apn.isPersistent(),
                    apn.getMaxConns(), apn.getWaitTime(),
                    apn.getMaxConnsTime(), apn.getMtu(),
                    apn.getMvnoType(), apn.getMvnoMatchData(),
                    apn.getApnSetId(), apn.getCarrierId(),
                    apn.getSkip464Xlat());
        }

        return apn;
    }

    @Override
    protected String mtkGetOperatorNumeric(IccRecords r) {
        // Get mcc/mnc from property firstly, if empty, then get it from IccRecords
        String operatorNumeric = null;
        if (mPhoneType == PhoneConstants.PHONE_TYPE_NONE) {
            if (MtkDcHelper.isCdma4GDualModeCard(mPhone.getPhoneId())) {
                mPhoneType = PhoneConstants.PHONE_TYPE_GSM;
            } else {
                mPhoneType = mPhone.getPhoneType();
            }
        }
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            operatorNumeric = SystemProperties.get(PROP_RIL_DATA_CDMA_MCC_MNC+mPhone.getPhoneId());
        } else {
            operatorNumeric = SystemProperties.get(PROP_RIL_DATA_GSM_MCC_MNC+mPhone.getPhoneId());
        }
        if (DBG) {
            log("mtkGetOperatorNumeric: operator from IccRecords = "
                    + ((r != null) ? r.getOperatorNumeric() : "")
                    + ", operator from RIL = " + operatorNumeric);
        }
        if (TextUtils.isEmpty(operatorNumeric)) {
            operatorNumeric = (r != null) ? r.getOperatorNumeric() : "";
            if (!TextUtils.isEmpty(operatorNumeric) && hasOperatorIaCapability()) {
                log("mtkGetOperatorNumeric: invalid case for gen93, return null");
                return null;
            }
        }
        if (mDataConnectionExt != null) {
            return mDataConnectionExt.getOperatorNumericFromImpi(operatorNumeric,
                    mPhone.getPhoneId());
        }
        return operatorNumeric;
    }

    private boolean isMvnoMatches(int mvnoType, String mvnoMatchData) {
        if (DBG) log("mvnoMatchData=" + mvnoMatchData);
        if (mvnoType == ApnSetting.MVNO_TYPE_SPN) {
            String strHexSpn = null;
            if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                strHexSpn = SystemProperties.get(PROP_RIL_DATA_CDMA_SPN+mPhone.getPhoneId(), "");
            } else {
                strHexSpn = SystemProperties.get(PROP_RIL_DATA_GSM_SPN+mPhone.getPhoneId(), "");
            }
            if (strHexSpn.length() == 0) {
                return false;
            }
            String strSpn = null;
            if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                strSpn = MtkIccUtilsEx.parseSpnToString(UiccController.APP_FAM_3GPP2,
                        IccUtils.hexStringToBytes(strHexSpn));
            } else {
                strSpn = MtkIccUtilsEx.parseSpnToString(UiccController.APP_FAM_3GPP,
                        IccUtils.hexStringToBytes(strHexSpn));
            }
            if (DBG) log("strSpn=" + strSpn);
            if ((strSpn != null) && strSpn.equalsIgnoreCase(mvnoMatchData)) {
                return true;
            }
        } else if (mvnoType == ApnSetting.MVNO_TYPE_IMSI) {
            String strImsi = null;
            if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                strImsi = SystemProperties.get(PROP_RIL_DATA_CDMA_IMSI+mPhone.getPhoneId(), "");
            } else {
                strImsi = SystemProperties.get(PROP_RIL_DATA_GSM_IMSI+mPhone.getPhoneId(), "");
            }
            if ((strImsi != null) && ApnSettingUtils.imsiMatches(mvnoMatchData, strImsi)) {
                return true;
            }
        } else if (mvnoType == ApnSetting.MVNO_TYPE_GID) {
            String gid1 = SystemProperties.get(PROP_RIL_DATA_GID1+mPhone.getPhoneId(), "");
            if (DBG) log("gid1=" + gid1);
            int mvno_match_data_length = mvnoMatchData.length();
            if ((gid1 != null) && (gid1.length() >= mvno_match_data_length) &&
                    gid1.substring(0, mvno_match_data_length).equalsIgnoreCase(mvnoMatchData)) {
                return true;
            }
        } else if (mvnoType == ApnSetting.MVNO_TYPE_ICCID) {
            String iccId = SystemProperties.get(PROP_RIL_DATA_ICCID+(mPhone.getPhoneId()+1), "");
            if (DBG) log("iccId=" + iccId);
            if ((iccId != null) && ApnSettingUtils.iccidMatches(mvnoMatchData, iccId)) {
                return true;
            }
        } else if (mvnoType == MtkApnSetting.MVNO_TYPE_PNN) {
            String strHexPnn = SystemProperties.get(PROP_RIL_DATA_PNN+mPhone.getPhoneId(), "");
            if (strHexPnn.length() == 0) {
                return false;
            }
            String strPnn = null;
            strPnn = MtkIccUtilsEx.parsePnnToString(IccUtils.hexStringToBytes(strHexPnn));
            if (DBG) log("strPnn=" + strPnn);
            if ((strPnn != null) && strPnn.equalsIgnoreCase(mvnoMatchData)) {
                return true;
            }
        }
        return false;
    }

    private DataRetryOperator getDataRetryOperator() {
        IccRecords r = mIccRecords.get();
        String operator = mtkGetOperatorNumeric(r);

        if (TextUtils.isEmpty(operator)) {
            operator = TelephonyManager.getDefault().getSimOperatorNumeric(mPhone.getSubId());
        }

        if (operator != null) {
            for (int i = 0; i < MCCMNC_TELCEL.length; i++) {
                if (operator.startsWith(MCCMNC_TELCEL[i])) {
                    return DataRetryOperator.TELCEL;
                }
            }
            for (int i = 0; i < MCCMNC_TELSTRA.length; i++) {
                if (operator.startsWith(MCCMNC_TELSTRA[i])) {
                    return DataRetryOperator.TELSTRA;
                }
            }
            for (int i = 0; i < MCCMNC_EE.length; i++) {
                if (operator.startsWith(MCCMNC_EE[i])) {
                    return DataRetryOperator.EE;
                }
            }
        }

        return DataRetryOperator.UNKNOWN;
    }

    private boolean isDataRetryRestrictEnabled() {
        return (mDataRetryOperator == DataRetryOperator.TELCEL
                || mDataRetryOperator == DataRetryOperator.TELSTRA
                || mDataRetryOperator == DataRetryOperator.EE);
    }

    private void onNetworkRejectReceived(AsyncResult ar) {
        if (ar.exception != null || ar.result == null) {
           loge("onNetworkRejectReceived exception");
        } else {
            // result[0]: <emm_cause>
            // result[1]: <esm_cause>
            // result[2]: <event_type>
            //            0 EMM_CAUSE_SOURCE_OTHER
            //            1 EMM_CAUSE_SOURCE_ATTACH_REJECT
            //            2 EMM_CAUSE_SOURCE_TAU_REJECT
            //            3 EMM_CAUSE_SOURCE_NW_DETAC
            //
            int[] ints = (int[]) ar.result;
            if (ints.length < 3) {
               loge("onNetworkRejectReceived urc format error");
               return;
            }
            int emm_cause = ints[0];
            int esm_cause = ints[1];
            int event = ints[2];
            log("onNetworkRejectReceived emm_cause:" + emm_cause + ", esm_cause:"
                    + esm_cause + ", event_type:" + event);

            Intent intent = new Intent(TelephonyIntents.ACTION_NETWORK_REJECT_CAUSE);
            SubscriptionManager.putPhoneIdAndSubIdExtra(intent, mPhone.getPhoneId());
            intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
            intent.putExtra(TelephonyIntents.EXTRA_EMM_CAUSE, emm_cause);
            intent.putExtra(TelephonyIntents.EXTRA_ESM_CAUSE, esm_cause);
            intent.putExtra(TelephonyIntents.EXTRA_REJECT_EVENT_TYPE, event);

            mPhone.getContext().sendStickyBroadcastAsUser(intent, UserHandle.ALL);
        }
    }

    private ApnSetting makeApnSetting(Cursor cursor) {
        // M: [Inactive Timer] @{
        int inactiveTimer = 0;
        try {
            inactiveTimer = cursor.getInt(
                    cursor.getColumnIndexOrThrow("inactive_timer"));
            if (inactiveTimer != 0) {
                logd("makeApnSetting: inactive_timer=" + inactiveTimer);
            }
        } catch (IllegalArgumentException e) {
            // set default value
        }
        // M: [Inactive Timer] @}

        // M: get default mtu config from resource @{
        int mtu = 0;
        try {
            mtu = cursor.getInt(
                    cursor.getColumnIndexOrThrow(Telephony.Carriers.MTU));
            if (mtu != 0) {
                logd("makeApnSetting: mtu=" + mtu);
            }else{
                mtu = getDefaultMtuConfig(mPhone.getContext());
            }
        } catch (IllegalArgumentException e) {
            // set default value
        }
        // M: get default mtu config from resource @}

        return MtkApnSetting.makeApnSetting(cursor, mtu, inactiveTimer);
    }

    /**
     * Anchor method of onDisableApn
     *
     * @param apnContext the specific apn context
     * @param releaseType the release type
     */
    @Override
    protected void mtkSyncApnContextDisableState(ApnContext apnContext,
            @ReleaseNetworkType int releaseType) {
        if (apnContext != null && apnContext.isReady()) {
            if (apnContext.getState() != DctConstants.State.CONNECTED) {
                log("Apn type '" + apnContext.getApnType() + "' is not connected,"
                        + " we need to sync the disable state with RIL");
                int nProfileId = getApnProfileID(apnContext.getApnType());
                nProfileId *= -1;
                mDataServiceManager.deactivateDataCall(nProfileId,
                        DataService.REQUEST_REASON_NORMAL, null);
            } else if (!PhoneConstants.APN_TYPE_DUN.equals(apnContext.getApnType())
                    && releaseType == RELEASE_TYPE_NORMAL) {
                ApnSetting apnSetting = apnContext.getApnSetting();
                if (apnSetting != null && apnSetting.canHandleType(ApnSetting.TYPE_DEFAULT)) {
                    log("This apn context has INTERNET capability and CS will not tear it down");
                    apnContext.setEnabled(false);
                    cleanUpConnectionInternal(true, releaseType, apnContext);
                }
            }
        }
    }

    public void mtkDeactivateDataCall(ApnContext apnContext) {
        if (apnContext != null) {
            log("mtkDeactivateDataCall: sync apn disable state for multi-purpose apn setting");
            int nProfileId = getApnProfileID(apnContext.getApnType());
            nProfileId *= -1;
            mDataServiceManager.deactivateDataCall(nProfileId,
                    DataService.REQUEST_REASON_NORMAL, null);
        }
    }

    private void setIsPcoAllowedDefault(boolean allowed) {
        if (mDataConnectionExt != null) {
            mDataConnectionExt.setIsPcoAllowedDefault(allowed);
        }
    }

    private boolean getIsPcoAllowedDefault() {
        if (mDataConnectionExt != null) {
            return mDataConnectionExt.getIsPcoAllowedDefault();
        } else {
            return true;
        }
    }

    /**
     * Anchor method of handlePcoData to check PCO values and perform operators' required actions
     *
     * @param apnContext the corresponding APN context of the PCO
     * @param pcoData the content of the PCO
     */
    @Override
    protected void mtkHandlePcoByOp(ApnContext apnContext, PcoData pcoData) {
        if (mDataConnectionExt != null) {
            int pcoAction = mDataConnectionExt.getPcoActionByApnType(apnContext, pcoData);
            switch (pcoAction) {
                case 1:
                    log("mtkHandlePcoByOp action1: teardown default apn");
                    cleanUpConnectionInternal(true, RELEASE_TYPE_DETACH, apnContext);
                    break;
                default:
                    break;
            }
        }
    }

    public boolean tearDownPdnByType(String type) {
        Message msg = obtainMessage(MtkDctConstants.CMD_TEAR_DOWN_PDN_BY_TYPE);
        msg.arg1 = ApnSetting.getApnTypesBitmaskFromString(type);
        log("tearDownPdnByType: sendMessage: type=" + msg.arg1);
        sendMessage(msg);
        return true;
    }

    public boolean setupPdnByType(String type) {
        log("setupPdnByType: sendMessage: type=" + type);
        sendMessage(obtainMessage(DctConstants.EVENT_TRY_SETUP_DATA, mApnContexts.get(type)));
        return true;
    }

    private void onTearDownPdnByApnId(int apnId) {
        cleanUpConnectionInternal(true, RELEASE_TYPE_DETACH, mApnContextsByType.get(apnId));
    }

    /// SSC Mode 3 @{
    private void setSscMode() {
        // Set vendor setting SSC mode '3' as the default value. It means that
        // if a new PDU has been allocated, MtkDcT will be notified to establish a new data call.
        String mode = Integer.toString(3);
        if (DBG) log("setSscMode: " + mode);
        ((MtkRIL)mPhone.mCi).setVendorSetting(VendorSetting.VENDOR_SETTING_DATA_SSC_MODE, mode,
                null);
    }

    public void trySetupDataOnEvent(int event, int cid, int lifetime) {
        Message msg = obtainMessage(event);
        msg.arg1 = cid;
        msg.arg2 = lifetime;
        sendMessage(msg);
    }

    private void onDataSetupSscMode3(int cid, int lifetime) {
        ApnContext apnContext = null;

        for (ApnContext apnCtx : mApnContexts.values()) {
            if (apnCtx.getDataConnection() != null) {
                if (apnCtx.getDataConnection().getCid() == cid) {
                    apnContext = apnCtx;
                    if (DBG) log("onDataSetupSscMode3: found apnContext=" + apnContext);
                    break;
                } else {
                    loge("onDataSetupSscMode3: cid does not match! (APN type="
                            + apnCtx.getApnType() + ", cid=" +
                            + apnCtx.getDataConnection().getCid() + ")");
                }
            }
        }

        if (apnContext == null) {
            loge("onDataSetupSscMode3: couldn't find corresponding apnContext");
            return;
        }

        DataConnection dc = apnContext.getDataConnection();
        ((MtkDataConnection) dc).updateNetworkAgentSscMode3(lifetime, 49);
        apnContext.setReason(MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3);
        ((MtkApnContext) apnContext).setDataConnectionSscMode3(dc);

        setupDataOnAllConnectableApns(MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3,
                RetryFailures.ALWAYS);
    }

    private boolean onSkipDisconnectDone(ApnContext apnContext) {
        if (apnContext == null) return false;

        DataConnection dc = apnContext.getDataConnection();
        DataConnection dc3 = ((MtkApnContext) apnContext).getDataConnectionSscMode3();

        boolean skipDisconnect = true;
        if ((dc == null) || (dc3 == null)) {
            skipDisconnect = false;
        } else if (dc.isInactive() && !dc3.isInactive()) {
            apnContext.setDataConnection(dc3);
            ((MtkDataConnection) dc3).updateNetworkAgentSscMode3(0, 50);
            ((MtkApnContext) apnContext).setDataConnectionSscMode3(null);
        } else if (!dc.isInactive() && dc3.isInactive()) {
            ((MtkApnContext) apnContext).setDataConnectionSscMode3(null);
        } else {
            skipDisconnect = false;
        }

        if (MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3.equals(apnContext.getReason())) {
            apnContext.setReason(Phone.REASON_CONNECTED);
        }

        if (skipDisconnect) {
            if (DBG) {
                log("onDisconnectDone: skip due to SSC mode3, dc=(" + dc + "), dc3=(" + dc3 + ")");
            }
        }
        return skipDisconnect;
    }

    /**
     * Anchor method of cleanUpConnectionInternal to check if disconnect SSC mode3 PDU.
     */
    @Override
    protected void mtkTearDown(ApnContext apnContext, Message msg) {
        if (((MtkApnContext) apnContext).getDataConnectionSscMode3() != null) {
            log("cleanUpConnectionInternal: tearing down SSC mode3 PDU, apnContext=" + apnContext);
            ((MtkApnContext) apnContext).getDataConnectionSscMode3()
                    .tearDown(apnContext, MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3, msg);
        }
    }

    /**
     * Anchor method of checkForCompatibleConnectedApnContext to check if disconnect SSC mode3
     * PDU.
     */
    @Override
    protected boolean mtkSkipCheckForCompatibleConnectedApnContext(ApnContext apnContext) {
        if ((apnContext != null) &&
                MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3.equals(apnContext.getReason()) &&
                (apnContext.getState() == DctConstants.State.CONNECTED)) {
            log("checkForCompatibleConnectedApnContext: skip check for SSC mode3 PDU, apnContext="
                    + apnContext);
            return true;
        }
        return false;
    }

    /**
     * Anchor method of onDataSetupComplete to handler data setup result for SSC mode3 PDU.
     */
    @Override
    protected boolean mtkCanHandleOnDataSetupComplete(ApnContext apnContext, boolean success,
            int cause, @RequestNetworkType int requestType) {
        if (success) {
            return false;
        } else if (MtkGsmCdmaPhone.REASON_DATA_SETUP_SSC_MODE3.equals(apnContext.getReason())) {
            if (DBG) {
                ApnSetting apn = apnContext.getApnSetting();
                log("onDataSetupComplete: SSC mode3 PDU, error apn=" + apn.getApnName()
                        + ", cause=" + cause + ", requestType=" + requestTypeToString(requestType));
            }
            DataConnection dc3 = ((MtkApnContext) apnContext).getDataConnectionSscMode3();
            apnContext.setDataConnection(dc3);
            ((MtkApnContext) apnContext).setDataConnectionSscMode3(null);
            if (dc3 == null) {
                log("onDataSetupComplete: SSC mode3 PDU, no connection to original DC, it's wired");
            } else {
                ((MtkDataConnection) dc3).updateNetworkAgentSscMode3(0, 50);
            }
            return true;
        }
        return false;
    }

    /**
     * Anchor method of findFreeDataConnection to check if dataconnection is inuse for SSC mode3
     * PDU.
     */
    @Override
    protected boolean mtkIsInUse(ApnContext apnContext, DataConnection dc) {
        if (((MtkApnContext) apnContext).getDataConnectionSscMode3() == dc) {
            log("findFreeDataConnection: APN type " + apnContext.getApnType() +
                    ", dc=(" + dc + ") is inuse for SSC mode3 PDU");
            return true;
        }
        return false;
    }
    /// @}

    @Override
    protected boolean mtkIsAddRequestNetworkCompleteMsg(@ApnType int apnType,
            @RequestNetworkType int requestType) {
        if (requestType == REQUEST_TYPE_HANDOVER
                && getDataRat() == ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
            log("Remember the handover apn type when data rat is unknown");
            mHandoverApnType = apnType;
            return true;
        }
        return false;
    }

    public void notifyMtkServiceStateChanged(MtkServiceState ss) {
        if (!mIsNotifyDataAttached && ss.getDataRegState() == ServiceState.STATE_IN_SERVICE
                && ss.getDataRoaming() == false) {
            log("notifyMtkServiceStateChanged: send EVENT_DATA_CONNECTION_ATTACHED");
            mIsNotifyDataAttached = true;
            mTurboSS = ss;
            sendMessage(obtainMessage(DctConstants.EVENT_DATA_CONNECTION_ATTACHED));
        }
    }

    public MtkServiceState getTurboSS() {
        return mTurboSS;
    }

    @Override
    protected void mtkFakeDataConnection(ApnContext apnContext) {
        log("mtkFakeDataConnection: apn type = " + apnContext.getApnType());

        if (TextUtils.equals(apnContext.getApnType(), PhoneConstants.APN_TYPE_MMS)) {
            MtkDataConnection conn = (MtkDataConnection) findFreeDataConnection();
            if (conn == null) {
                int id = mUniqueIdGenerator.getAndIncrement();
                conn = (MtkDataConnection) DataConnection.makeDataConnection(mPhone,
                        id, this, mDataServiceManager, mDcTesterFailBringUpAll, mDcc);
                mDataConnections.put(id, conn);
            }
            conn.fakeNetworkAgent(apnContext);
        }
    }

    public String getImsi() {
        if (mIsRecordsOverride) {
            return null;
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            return mImsiCdma;
        }
        return mImsiGsm;
    }

    private void getImsiFromRil() {
        if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WLAN) {
            log("skip getImsiFromRil() for DCT-I-x");
            return;
        }
        if (mPhoneType == PhoneConstants.PHONE_TYPE_CDMA) {
            mPhone.mCi.getIMSIForApp("A0000000000C2K",
                    obtainMessage(MtkDctConstants.EVENT_IMSI_QUERY_DONE,
                    PhoneConstants.PHONE_TYPE_CDMA, 0));
        } else if (mPhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            mPhone.mCi.getIMSIForApp("A0000000000GSM",
                    obtainMessage(MtkDctConstants.EVENT_IMSI_QUERY_DONE,
                    PhoneConstants.PHONE_TYPE_GSM, 0));
        } else {
            mPhone.mCi.getIMSIForApp("A0000000000GSM",
                    obtainMessage(MtkDctConstants.EVENT_IMSI_QUERY_DONE,
                    PhoneConstants.PHONE_TYPE_GSM, 0));
            if ("1".equals(SystemProperties.get("ro.vendor.mtk_c2k_support", ""))) {
                mPhone.mCi.getIMSIForApp("A0000000000C2K",
                        obtainMessage(MtkDctConstants.EVENT_IMSI_QUERY_DONE,
                        PhoneConstants.PHONE_TYPE_CDMA, 0));
            }
        }
    }

    public boolean isDataServiceBound() {
        return mDataServiceBound;
    }

    @Override
    protected boolean mtkSkipDataStallAlarm() {
        boolean skipStall = true;
        boolean isTestSim = false;
        int phoneId = mPhone.getPhoneId();
        MtkDcHelper dcHelper = MtkDcHelper.getInstance();

        if (SubscriptionManager.isValidPhoneId(phoneId) &&
                dcHelper != null && dcHelper.isTestIccCard(phoneId)) {
            isTestSim = true;
        }

        if (isTestSim) {
            if (SystemProperties.get(SKIP_DATA_STALL_ALARM).equals("0")) {
                skipStall = false;
            } else {
                // majority behavior
                skipStall = true;
            }
        } else {
            if (SystemProperties.get(SKIP_DATA_STALL_ALARM).equals("1")) {
                skipStall = true;
            } else {
                // majority behavior
                skipStall = false;
            }
        }

        return skipStall;
    }

    /// M: For USCC(OP236) @{
    // Set roaming data setting with roaming type.
    // Set roaming data setting as true in domestic roaming area, as false in
    // international roaming area.
    // Return true when roaming data setting is really changed.
    private boolean setRoamingDataWithRoamingType(int roamingType) {
        if (roamingType == ServiceState.ROAMING_TYPE_DOMESTIC) {
            return setDataRoamingEnabledByDefault(true);
        }

        if (roamingType == ServiceState.ROAMING_TYPE_INTERNATIONAL
                || roamingType == ServiceState.ROAMING_TYPE_NOT_ROAMING
                || roamingType == ServiceState.ROAMING_TYPE_UNKNOWN) {
            return setDataRoamingEnabledByDefault(false);
        }

        return false;
    }

    // Write roaming data setting value into settings database.
    // Return true when current roaming data setting value is different.
    private boolean setDataRoamingEnabledByDefault(boolean enabled) {
        if (TelephonyManager.getDefault().getSimCount() == 1
                && mCcOneSettingForRoaming && !isDataRoamingFromUserAction()) {
            boolean oldEnabled = getDataRoamingEnabled();

            if (DBG) {
                log("setDataRoamingEnabledByDefault: oldEnabled = " + oldEnabled
                        + " enabled = " + enabled);
            }

            if (oldEnabled != enabled) {
                mDataEnabledSettings.setDataRoamingEnabled(enabled);
                return true;
            }
        }

        return false;
    }

    @Override
    protected boolean mtkIsUseCarrierRoamingData() {
        return !getDataRoamingEnabled();
    }

    @Override
    protected boolean mtkIsSetFalseForUserAction() {
        return mCcOneSettingForRoaming;
    }
    /// M: @}

    // Only Sprint SIM and Sprint project support this feature.
    private boolean isUniqueRoamingFeatureEnabled() {
        return (mCcUniqueSettingsForRoaming &&
                "OP20".equals(SystemProperties.get("persist.vendor.operator.optr", "")));
    }

    // M: [Data Usage Update][Network Latency Optimization] @{
    private void updateMobileDataUsage(AsyncResult ar) {
        int txBytes, txPkts, rxBytes, rxPkts = -1;
        int[] ints = (int[]) ar.result;

        txBytes = (ints.length > 0) ? ints[0] : 0;
        txPkts = (ints.length > 1) ? ints[1] : 0;
        rxBytes = (ints.length > 2) ? ints[2] : 0;
        rxPkts = (ints.length > 3) ? ints[3] : 0;

        long currentTime = SystemClock.elapsedRealtime();
        int isRoaming = mPhone.getServiceState().getDataRoaming() ? 1 : 0;
        LinkProperties linkProperties = getLinkProperties(PhoneConstants.APN_TYPE_DEFAULT);
        String ifacename = (linkProperties == null) ? "" : linkProperties.getInterfaceName();

        // Create mobile data usage of total.
        NetworkStats mobileDataUsage = new NetworkStats(currentTime, 1);
        mobileDataUsage.combineAllValues(mMobileDataUsage);
        mobileDataUsage.combineValues(new NetworkStats.Entry(
                ifacename, -10, NetworkStats.SET_DEFAULT,
                NetworkStats.TAG_NONE, NetworkStats.METERED_YES, isRoaming,
                NetworkStats.DEFAULT_NETWORK_YES, rxBytes,
                rxPkts, txBytes, txPkts, 0));
        log("updateMobileDataUsage ifacename:" + ifacename
                + ", txBytes:" + txBytes
                + ", txPkts:" + txPkts
                + ", rxBytes:" + rxBytes
                + ", rxPkts:" + rxPkts);
        mMobileDataUsage = mobileDataUsage;

        Intent intent = new Intent(TelephonyIntents.ACTION_BACKGROUND_MOBILE_DATA_USAGE);
        mPhone.getContext().sendBroadcastAsUser(intent, UserHandle.ALL);
    }

    public NetworkStats getMobileDataUsage() {
        return mMobileDataUsage;
    }

    public void setMobileDataUsageSum(long txBytes, long txPkts, long rxBytes, long rxPkts) {
        // mMobileDataUsage record all history stats, put them into it.
        long currentTime = SystemClock.elapsedRealtime();
        int isRoaming = mPhone.getServiceState().getDataRoaming() ? 1 : 0;
        LinkProperties linkProperties = getLinkProperties(PhoneConstants.APN_TYPE_DEFAULT);
        String ifacename = (linkProperties == null) ? "" : linkProperties.getInterfaceName();

        // Create mobile data usage of total.
        NetworkStats mobileDataUsage = new NetworkStats(currentTime, 1);
        mobileDataUsage.combineAllValues(mMobileDataUsage);
        mobileDataUsage.combineValues(new NetworkStats.Entry(
                ifacename, -10, NetworkStats.SET_DEFAULT,
                NetworkStats.TAG_NONE, NetworkStats.METERED_YES, isRoaming,
                NetworkStats.DEFAULT_NETWORK_YES, rxBytes,
                rxPkts, txBytes, txPkts, 0));
        mMobileDataUsage = mobileDataUsage;
    }
    // M: [Data Usage Update][Network Latency Optimization] @}

    @Override
    protected boolean mtkIsApnCanSupportNetworkType(ApnSetting apn, int radioTech) {
        // [Scenario] Modem told AP to setup "ims" over wifi and "emergency" over cellular.
        //   However, device only camp on IWLAN but not cellular.
        //   So data framework fail to create emergency PDN over cellular and didn't
        //   reply EIMSPDN confirm to modem in the end.
        // [Solution] To handle radioTech is UNKNOWN but modem prefer establish
        //   emergency APN over specific RAT.
        // [Reference] ALPS04810553
        boolean canHandle = false;
        int networkType = ServiceState.rilRadioTechnologyToNetworkType(radioTech);
        if ((apn.getApnTypeBitmask() == ApnSetting.TYPE_EMERGENCY) &&
                networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WWAN) {
                canHandle = apn.canSupportNetworkType(TelephonyManager.NETWORK_TYPE_LTE);
            } else if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WLAN) {
                canHandle = apn.canSupportNetworkType(TelephonyManager.NETWORK_TYPE_IWLAN);
            }
            logi("[mtkIsApnCanSupportNetworkType] return:" + canHandle);
        }

        return canHandle;
    }

    /**
     * OP129 Spec APN_2_1_2_8 & APN_2_1_2_6
     * isOp129GsmUmts to check if UE is under 2G/3G or roaming
     */
    protected boolean isOp129GsmUmts(int radioTechnology) {
        return radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_GPRS
                || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_EDGE
                || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
                || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
                || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
                || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
                || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP
                || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_GSM;
    }

    /**
     * OP129 Spec APN_2_1_2_8 & APN_2_1_2_6
     * isOp129DunChange to check if UE is under 2G/3G or roaming
     */
    public boolean isOp129DunChange() {
        if (MtkDcHelper.getInstance().isOperatorMccMnc(MtkDcHelper.Operator.OP129,
                    mPhone.getPhoneId())) {
            if (isOp129GsmUmts(getDataRat()) ||
                mPhone.getServiceState().getDataRoamingFromRegistration()) {
                log("isOp129DunChange");
                return true;
            }
        }
        return false;
    }

    private boolean isRestorePreferredApn() {
        String subId = Long.toString(mPhone.getSubId());
        Uri uri = Uri.withAppendedPath(PREFERAPN_NO_UPDATE_URI_USING_SUBID, subId);
        Cursor cursor = mPhone.getContext().getContentResolver().query(
                uri, new String[] { "_id", "name", "apn" },
                null, null, Telephony.Carriers.DEFAULT_SORT_ORDER);
        boolean bRestore = false;
        if (cursor != null) {
            bRestore = (cursor.getCount() == 0);
            cursor.close();
        }
        return bRestore;
    }
}
