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

import android.annotation.NonNull;
import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
/// M: E911 Wifi control. @{
import android.net.wifi.WifiManager;
/// @}
import android.os.AsyncResult;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.PowerManager;
import android.os.RegistrantList;
import android.os.ResultReceiver;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.preference.PreferenceManager;
import android.telecom.VideoProfile;
import android.telephony.AccessNetworkConstants;
import android.telephony.AccessNetworkConstants.TransportType;
import android.telephony.CarrierConfigManager;
import android.telephony.MtkRadioAccessFamily;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.text.TextUtils;
import android.util.Log;

import com.android.ims.ImsManager;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.CarrierInfoManager;
import com.android.internal.telephony.CarrierKeyDownloadManager;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Connection;
import com.android.internal.telephony.GsmCdmaCall;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.MmiCode;
import com.android.internal.telephony.OperatorInfo;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneNotifier;
import com.android.internal.telephony.RadioCapability;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.UUSInfo;
// PHB START @{
import com.android.internal.telephony.uicc.IccFileHandler;
// PHB END @}
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.RuimRecords;
import com.android.internal.telephony.uicc.SIMRecords;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccProfile;
import com.android.internal.telephony.cdma.EriManager;
import com.android.internal.telephony.cdma.CdmaMmiCode;
import com.android.internal.telephony.gsm.GsmMmiCode;
import com.android.internal.telephony.gsm.SuppServiceNotification;
import com.android.internal.telephony.test.SimulatedRadioControl;
import com.android.ims.ImsConfig;

import com.mediatek.ims.internal.MtkImsManager;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCallTracker;
import com.android.internal.telephony.imsphone.ImsPhoneMmiCode;
import com.android.internal.telephony.TelephonyDevController;

/// M: CC: Proprietary CRSS handling
import com.mediatek.internal.telephony.gsm.MtkSuppServiceNotification;
import com.mediatek.internal.telephony.gsm.MtkSuppCrssNotification;

// MTK: SS {
import com.android.ims.ImsException;

import com.mediatek.ims.MtkImsReasonInfo;
import com.mediatek.internal.telephony.MtkCallForwardInfo;
import com.mediatek.internal.telephony.MtkOperatorUtils;
import com.mediatek.internal.telephony.MtkOperatorUtils.OPID;
import com.mediatek.internal.telephony.gsm.MtkGsmMmiCode;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.*;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_NO_REPLY;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_NOT_REACHABLE;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_BUSY;
import static com.android.internal.telephony.CommandsInterface.CF_REASON_UNCONDITIONAL;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_VOICE;
// MTK: SS }

import com.mediatek.internal.telephony.cdma.MtkCdmaSubscriptionSourceManager;
import com.mediatek.internal.telephony.dataconnection.MtkDcHelper;
import com.mediatek.internal.telephony.dataconnection.MtkDcTracker;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;
import com.mediatek.internal.telephony.imsphone.MtkLocalPhoneNumberUtils;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkPhoneNotifier;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.internal.telephony.MtkSSRequestDecisionMaker;
import com.mediatek.internal.telephony.MtkTelephonyComponentFactory;
import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.PseudoCellInfo;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;
// MTK-START: MVNO
import com.mediatek.internal.telephony.uicc.MtkSIMRecords;
import com.mediatek.internal.telephony.MtkPhoneConstants;
// MTK-END
/// M: CSIM PHB handling @{
import com.mediatek.internal.telephony.phb.CsimPhbUtil;
/// @}
import com.mediatek.internal.telephony.MtkSuppServHelper;
import com.mediatek.internal.telephony.NetworkInfoWithAcT;
import com.mediatek.internal.telephony.scbm.ISCBMManager;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;

import com.mediatek.telephony.MtkTelephonyManagerEx;

import mediatek.telephony.MtkCarrierConfigManager;
import mediatek.telephony.MtkServiceState;

import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.Deque;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.TimeZone;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.annotation.ProductApi;

public class MtkGsmCdmaPhone extends GsmCdmaPhone {
    public static final String LOG_TAG = "MtkGsmCdmaPhone";
    private static final boolean DBG = true;
    public MtkRIL mMtkCi;

    private int mNewVoiceTech = -1;

    private final Object mLock = new Object();

    /// M: CC: ECC for Fusion RIL @{
    TelephonyDevController mTelDevController = TelephonyDevController.getInstance();
    private boolean hasC2kOverImsModem() {
        if (mTelDevController != null &&
                mTelDevController.getModem(0) != null &&
                ((MtkHardwareConfig) mTelDevController.getModem(0)).hasC2kOverImsModem() == true) {
                    return true;
        }
        return false;
    }
    /// @}

    /// M: [Network][C2K]Add the new network type. @{
    public static final int NT_MODE_LTE_GSM = MtkRILConstants.NETWORK_MODE_LTE_GSM;
    public static final int NT_MODE_LTE_TDD_ONLY = MtkRILConstants.NETWORK_MODE_LTE_TDD_ONLY;
    /// @}

    /* M: Network part */
    public MtkServiceStateTracker mMtkSST;
    public static final String GSM_INDICATOR   = "2G";
    public static final String UTRAN_INDICATOR = "3G";
    public static final String LTE_INDICATOR   = "4G";
    public static final String ACT_TYPE_GSM    = "0";
    public static final String ACT_TYPE_UTRAN  = "2";
    public static final String ACT_TYPE_LTE    = "7";

    public static final String REASON_RA_FAILED = "raFailed";
    public static final String REASON_FDN_ENABLED = "FdnEnabled";
    public static final String REASON_FDN_DISABLED = "FdnDisabled";
    /// M: P-CSCF error handling.
    public static final String REASON_PCSCF_ADDRESS_FAILED = "pcscfFailed";
    public static final String REASON_RESUME_PENDING_DATA = "resumePendingData";

    // M: Data Framework - Data Retry enhancement
    // Modem data retry count reset
    public static final String REASON_MD_DATA_RETRY_COUNT_RESET = "modemDataCountReset";
    // M: CarrierConfigLoader error handling
    public static final String REASON_CARRIER_CONFIG_LOADED = "carrierConfigLoaded";

    // M: Multi-PS Attach
    public static final String REASON_DATA_ALLOWED = "dataAllowed";

    /// M: SSC Mode 3 @{
    public static final String REASON_DATA_SETUP_SSC_MODE3 = "dataSetupSscMode3";
    /// @}

    protected static final int EVENT_MTK_BASE                       = 1000;
    /// M: CC: GSM 02.07 B.1.26 Ciphering Indicator support
    protected static final int EVENT_CIPHER_INDICATION              = EVENT_MTK_BASE + 0;
    protected static final int EVENT_GET_APC_INFO                   = EVENT_MTK_BASE + 1;
    /// M: CC: Proprietary CRSS handling
    protected static final int EVENT_SSN_EX                         = EVENT_MTK_BASE + 2;
    protected static final int EVENT_CRSS_IND                       = EVENT_MTK_BASE + 3;
    /// M: SS: Set supplementary service property
    protected static final int EVENT_SET_SS_PROPERTY                = EVENT_MTK_BASE + 4;

    /// M: CC: GSM 02.07 B.1.26 Ciphering Indicator support
    protected final RegistrantList mCipherIndicationRegistrants
            = new RegistrantList();

    /// M: CC: Proprietary CRSS handling @{
    RegistrantList mCallRelatedSuppSvcRegistrants = new RegistrantList();
    private AsyncResult mCachedSsn = null;
    private AsyncResult mCachedCrssn = null;
    /// @}


    private ISelfActivation mSelfActInstance = null;

    private ISCBMManager mScbmManager = null;

    /* M: SS
     * Define the mapping between UT/XCAP ERROR CODE and
     * OEM ERROR CODE in CommandException
     *
     * UT_XCAP_403_FORBIDDEN             = CommandException.Error.OPERATION_NOT_ALLOWED
     * UT_XCAP_404_NOT_FOUND             = CommandException.Error.NO_SUCH_ELEMENT
     * SPECAIL_UT_COMMAND_NOT_SUPPORTED  = CommandException.Error.REQUEST_NOT_SUPPORTED
     * UT_UNKNOWN_HOST                   = CommandException.Error.OEM_ERROR_3
     * UT_XCAP_409_CONFLICT              = CommandException.Error.OEM_ERROR_25
     * CALL_BARRED                       = CommandException.Error.OEM_ERROR_5
     */

    private static final String SS_SERVICE_CLASS_PROP = "vendor.gsm.radio.ss.sc";
    private static final String CFU_TIME_SLOT = "persist.vendor.radio.cfu.timeslot.";

    private static final String[] PROPERTY_RIL_FULL_UICC_TYPE  = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    private static final String PROP_MTK_CDMA_LTE_MODE = "ro.vendor.mtk_c2k_lte_mode";
    public static final boolean MTK_SVLTE_SUPPORT = (SystemProperties.getInt(
            PROP_MTK_CDMA_LTE_MODE, 0) == 1);

    private static final String CFB_KEY = "CFB";
    private static final String CFNR_KEY = "CFNR";
    private static final String CFNRC_KEY = "CFNRC";

    public static final String IMS_DEREG_PROP = "vendor.gsm.radio.ss.imsdereg";
    public static final String IMS_DEREG_ON = "1";
    public static final String IMS_DEREG_OFF = "0";
    public static final int MESSAGE_SET_CF = 1;

    /* EVENT numver for SS part */
    public static final int EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE = 109;
    public static final int EVENT_SET_CALL_FORWARD_TIME_SLOT_DONE = 110;

    /* for SIM switch to early get capability */
    public static final int EVENT_UNSOL_RADIO_CAPABILITY_CHANGED  = 111;

    public static final int EVENT_GET_CALL_WAITING_DONE = 301;
    public static final int EVENT_SET_CALL_WAITING_DONE = 302;
    public static final int EVENT_IMS_UT_DONE           = 2000;
    public static final int EVENT_IMS_UT_CSFB           = 2001;
    public static final int EVENT_GET_CLIR_COMPLETE          = 2004;
    public static final int EVENT_SET_CALL_BARRING_COMPLETE = 2005;
    public static final int EVENT_GET_CALL_BARRING_COMPLETE = 2006;

    /* The mode for Terminal-based Call Waiting
     * TBCW_UNKNOWN: Default value. If the sim card was inserted first time and have not
     *               queried yet, we are not able to know the mode to TBCW.
     * TBCW_VOLTE_USER: For a VoLTE SIM card, Set & Get Terminal-based CW.
     * TBCW_NOT_VOLTE_USER: For a non USIM-type sim card, Set & Get CS CW.
     * TBCW_WITH_CS: For those operators support CS CW. We will also Set Termianl-based CW.
     */
    public static final int TBCW_UNKNOWN = 0;
    public static final int TBCW_VOLTE_USER = 1;
    public static final int TBCW_NOT_VOLTE_USER = 2;
    public static final int TBCW_WITH_CS = 3;
    /* The mode for Terminal-based Call Waiting, it will be
     * initialized first when query call waiting.
     */
    private int mTbcwMode = TBCW_UNKNOWN;
    private MtkSSRequestDecisionMaker mMtkSSReqDecisionMaker;
    private CountDownLatch mCallbackLatch;
    private ExecutorService mExecutorService = Executors.newSingleThreadExecutor();
    private static final int OPERATION_TIME_OUT_MILLIS = 3000;

    /* CS Fallback status */
    private int mCSFallbackMode = 0;

    /// M: E911 Wifi control. @{
    private boolean mWifiIsEnabledBeforeE911 = false;
    private static final String PROPERTY_WFC_ENABLE = "persist.vendor.mtk.wfc.enable";
    /// @}

    /// M: VzW device type @{
    private static final String PROP_VZW_DEVICE_TYPE = "persist.vendor.vzw_device_type";
    /// @}

    /// M: SS IOT EAZY CONFIG @{
    private static final int PROPERTY_MODE_INT = 0;
    private static final int PROPERTY_MODE_BOOL = 1;
    private static final int PROPERTY_MODE_STRING = 2;
    /// @}

    /* AOSP
     * Create Cfu (Call forward unconditional) so that dialing number &
     * mOnComplete (Message object passed by client) can be packed &
     * given as a single Cfu object as user data to RIL.
     */
    private static class Cfu {
        final String mSetCfNumber;
        final Message mOnComplete;
        final int mServiceClass;

        Cfu(String cfNumber, Message onComplete, int serviceClass) {
            mSetCfNumber = cfNumber;
            mOnComplete = onComplete;
            mServiceClass = serviceClass;
        }
    }

    public MtkGsmCdmaPhone(Context context, CommandsInterface ci, PhoneNotifier notifier,
            boolean unitTestMode, int phoneId, int precisePhoneType,
            TelephonyComponentFactory telephonyComponentFactory) {
        super(context, ci, notifier, unitTestMode, phoneId, precisePhoneType,
                telephonyComponentFactory);

        Rlog.d(LOG_TAG, "constructor: sub = " + phoneId);
        mMtkCi = (MtkRIL)ci;
        mMtkSST = (MtkServiceStateTracker) mSST;

        /// M: SS Ut part @{
        mMtkSSReqDecisionMaker = new MtkSSRequestDecisionMaker(mContext, this);
        mMtkSSReqDecisionMaker.starThread();
        /// @}

        /// M: CC: GSM 02.07 B.1.26 Ciphering Indicator support
        mMtkCi.registerForCipherIndication(this, EVENT_CIPHER_INDICATION, null);

        // Self-activation feature @{
        OpTelephonyCustomizationFactoryBase telephonyCustomizationFactory = null;
        telephonyCustomizationFactory =
                OpTelephonyCustomizationUtils.getOpFactory(context);
        mSelfActInstance =
                telephonyCustomizationFactory.makeSelfActivationInstance(phoneId);
        mSelfActInstance
                .setContext(context)
                .setCommandsInterface(ci)
                .buildParams();
        // @}

        /* register the intents for BroadcastReceiver */
        mScbmManager = telephonyCustomizationFactory.makeSCBMManager(context, phoneId, ci);
    }

    public MtkGsmCdmaPhone(Context context, CommandsInterface ci, PhoneNotifier notifier,
            int phoneId, int precisePhoneType,
            TelephonyComponentFactory telephonyComponentFactory) {
        this(context, ci, notifier, false, phoneId, precisePhoneType, telephonyComponentFactory);
    }

    public ISelfActivation getSelfActivationInstance() {
        return mSelfActInstance;
    }

    public ISCBMManager getScbmManagerInstance() {
        return mScbmManager;
    }

    @Override
    public ServiceState getServiceState() {
        /// IMS service state is reliable only when data registration state is in service
        if (mMtkSST == null
                || (mMtkSST.mSS.getState() != ServiceState.STATE_IN_SERVICE
                && mMtkSST.mSS.getDataRegState() == ServiceState.STATE_IN_SERVICE)) {
            if (mImsPhone != null) {
                return MtkServiceState.mergeMtkServiceStates(
                        (mMtkSST == null) ? new MtkServiceState() : (MtkServiceState)mMtkSST.mSS,
                        (MtkServiceState)mImsPhone.getServiceState());
            }
        }

        if (mSST != null) {
            return mSST.mSS;
        } else {
            // avoid potential NPE in EmergencyCallHelper during Phone switch
            return new MtkServiceState();
        }
    }

    @Override
    public PhoneConstants.DataState getDataConnectionState(String apnType) {
        PhoneConstants.DataState ret = PhoneConstants.DataState.DISCONNECTED;

        /// M: Data icon performance enhancement @{
        MtkDcTracker dct = (MtkDcTracker)getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        MtkServiceState turboSS = dct.getTurboSS();
        /// @}

        if (mSST == null) {
            // Radio Technology Change is ongoning, dispose() and removeReferences() have
            // already been called

            ret = PhoneConstants.DataState.DISCONNECTED;
        } else if (turboSS == null
                && mSST.getCurrentDataConnectionState() != ServiceState.STATE_IN_SERVICE
                && (isPhoneTypeCdma() || isPhoneTypeCdmaLte() ||
                (isPhoneTypeGsm() && !apnType.equals(PhoneConstants.APN_TYPE_EMERGENCY)))) {
            // If we're out of service, open TCP sockets may still work
            // but no data will flow

            // Emergency APN is available even in Out Of Service
            // Pass the actual State of EPDN

            ret = PhoneConstants.DataState.DISCONNECTED;
        } else { /* mSST.gprsState == ServiceState.STATE_IN_SERVICE */
            int currentTransport = mTransportManager.getCurrentTransport(
                    ApnSetting.getApnTypesBitmaskFromString(apnType));
            if (getDcTracker(currentTransport) != null) {
                switch (getDcTracker(currentTransport).getState(apnType)) {
                    case CONNECTED:
                    case DISCONNECTING:
                        /// M: For call state, both CS and PS call need to be concerned. @{
                        //     Create a new method to check if CS/PS concurrent is allowed
                        //     based on isDataAllowedForConcurrent() method.
                        //if ( mCT.mState != PhoneConstants.State.IDLE
                        //        && !mSST.isConcurrentVoiceAndDataAllowed()) {
                        if (MtkDcHelper.getInstance().isDataAllowedForConcurrent(getPhoneId())) {
                            ret = PhoneConstants.DataState.CONNECTED;
                        } else {
                            ret = PhoneConstants.DataState.SUSPENDED;
                        }
                        /// @}
                        break;
                    case CONNECTING:
                        ret = PhoneConstants.DataState.CONNECTING;
                        break;
                    default:
                        ret = PhoneConstants.DataState.DISCONNECTED;
                }
            }
        }

        logd("getDataConnectionState apnType=" + apnType + " ret=" + ret);
        return ret;
    }

    protected void initOnce(CommandsInterface ci) {
        super.initOnce(ci);

        // for SIM switch to early get capability
        mCi.registerForRadioCapabilityChanged(this, EVENT_UNSOL_RADIO_CAPABILITY_CHANGED, null);

        /// M: CC: Proprietary CRSS handling
        if (mMtkCi == null) {
            mMtkCi = (MtkRIL)ci;
        }
        mMtkCi.setOnSuppServiceNotificationEx(this, EVENT_SSN_EX, null);
        mMtkCi.setOnCallRelatedSuppSvc(this, EVENT_CRSS_IND, null);
    }


    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();

            if (intent.getAction().equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)) {
                sendMessage(obtainMessage(EVENT_CARRIER_CONFIG_CHANGED));
            }
        }
    };

    @Override
    protected void switchPhoneType(int precisePhoneType) {
        synchronized (mLock) {
            super.switchPhoneType(precisePhoneType);
        }
        // M: for SS phone type change {@
        if (mIccRecords.get() != null && precisePhoneType == PhoneConstants.PHONE_TYPE_GSM) {
            logd("Re-register registerForIccRecordEvents due to phonetype change to GSM.");
            unregisterForIccRecordEvents();
            registerForIccRecordEvents();
        }
        // M: for SS phone type change @}
    }

    @Override
    protected void onUpdateIccAvailability() {
        super.onUpdateIccAvailability();
        // Update mIccRecords, mUiccApplication, mIccPhoneBookIntManager
        UiccCardApplication newUiccApplication = getUiccCardApplication();
        UiccCardApplication app = mUiccApplication.get();
        // MTK-START
        IccRecords newIccRecord =
                (newUiccApplication != null) ? newUiccApplication.getIccRecords() : null;
        if ((app == newUiccApplication) && (mIccRecords.get() != newIccRecord)) {
        //if (app != newUiccApplication) {
        // MTK-END
            if (app != null) {
                if (DBG) {
                    logd("Removing stale icc objects.");
                }
                if (mIccRecords.get() != null) {
                    unregisterForIccRecordEvents();
                    mIccPhoneBookIntManager.updateIccRecords(null);
                }
                mIccRecords.set(null);
                mUiccApplication.set(null);
            }
            if (newUiccApplication != null) {
                if (DBG) {
                    logd("New Uicc application found. type = " + newUiccApplication.getType());
                }
                mUiccApplication.set(newUiccApplication);
                mIccRecords.set(newUiccApplication.getIccRecords());
                registerForIccRecordEvents();
                mIccPhoneBookIntManager.updateIccRecords(mIccRecords.get());
            }
        }
        // MTK-START
        // If phoneType is CDMALTE and card is CDMA only, change the phoneType to CDMA;
        // If phoneType is CDMA and card is not CDMA only, change the phoneType to CDMALTE
        Rlog.d(LOG_TAG, "isPhoneTypeCdmaLte:" + isPhoneTypeCdmaLte()+ ", phoneId: " +getPhoneId()
        + " isCdmaWithoutLteCard: " + isCdmaWithoutLteCard() + " mNewVoiceTech: " + mNewVoiceTech);
        if (mNewVoiceTech != -1 &&
                ((isPhoneTypeCdmaLte() && isCdmaWithoutLteCard()) ||
                (isPhoneTypeCdma() && !isCdmaWithoutLteCard()))) {
            updatePhoneObject(mNewVoiceTech);
        }
        // MTK-END
    }

    @Override
    protected boolean correctPhoneTypeForCdma(boolean matchCdma, int newVoiceRadioTech) {
        boolean phoneTypeChanged = false;
        if (matchCdma  &&
                getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
            // Update UiccProfile app type and it is different for cdma 4G and 3G card.
            UiccProfile uiccProfile = getUiccProfile();
            if (uiccProfile != null) {
                uiccProfile.setVoiceRadioTech(newVoiceRadioTech);
            }
            phoneTypeChanged = true;
        }
        // If phoneType is CDMALTE and card is CDMA only, change the phoneType to CDMA;
        // If phoneType is CDMA and card is not CDMA only, change the phoneType to CDMALTE
        if (!((isPhoneTypeCdmaLte() && isCdmaWithoutLteCard()) ||
                (isPhoneTypeCdma() && !isCdmaWithoutLteCard()))) {
            phoneTypeChanged = false;
        }
        Rlog.d(LOG_TAG, "correctPhoneTypeForCdma: change:" + phoneTypeChanged +
                " newVoiceRadioTech=" + newVoiceRadioTech + " mActivePhone=" + getPhoneName());
        return phoneTypeChanged;
    }

    @Override
    protected void switchVoiceRadioTech(int newVoiceRadioTech) {
        logd("Switching Voice Phone : " + getPhoneName() + " >>> "
                + (ServiceState.isGsm(newVoiceRadioTech) ? "GSM" : "CDMA"));
        // MTK-START: add CDMA type.
        if (ServiceState.isCdma(newVoiceRadioTech) && isCdmaWithoutLteCard()) {
            switchPhoneType(PhoneConstants.PHONE_TYPE_CDMA);
            return;
        }
        // MTK-END: add CDMA type.
        super.switchVoiceRadioTech(newVoiceRadioTech);
    }

    protected void logd(String s) {
        Rlog.d(LOG_TAG, "[MtkGsmCdmaPhone] " + s);
    }

    /**
     * Check if card type is cdma only card.
     * @return true: cdma only card; false: not cdma only card.
     */
    private boolean isCdmaWithoutLteCard() {

        int iccFamily = MtkTelephonyManagerEx.getDefault().getIccAppFamily(getPhoneId());

        if (iccFamily == UiccController.APP_FAM_3GPP2) {
            return true;
        }
        return false;
    }

    /**
     * Trigger mode switch for ECC
     *
     * @param mode the target mode: 04: CARD_TYPE_CSIM, 01: CARD_TYPE_SIM.
     * @param response the responding message.
     */
    public void triggerModeSwitchByEcc(int mode, Message response) {
        mMtkCi.triggerModeSwitchByEcc(mode, response);
    }

    /**
     * Return the current located PLMN string (ex: "46000") or null (ex: flight mode or no signal
     * area).
     * @return mLocatedPlmn return register PLMN id.
     */
    public String getLocatedPlmn() {
        return mMtkSST.getLocatedPlmn();
    }

    @Override
    public void sendSubscriptionSettings(boolean restoreNetworkSelection) {
        if (mMtkSST != null) {
            mMtkSST.setDeviceRatMode(mPhoneId);
        }

        // M: check restoreSelection
        boolean restoreSelection_config = !mContext.getResources().getBoolean(
                com.android.internal.R.bool.skip_restoring_network_selection);

        // M: restoreSelection_config must be true when restore network selection
        if (restoreNetworkSelection && restoreSelection_config) {
            restoreSavedNetworkSelection(null);
        }
    }

    @Override
    protected void setPreferredNetworkTypeIfSimLoaded() {
        int subId = getSubId();
        if (SubscriptionManager.isValidSubscriptionId(subId)) {
            if (mMtkSST != null) {
                mMtkSST.setDeviceRatMode(mPhoneId);
            }
        }
    }

    /**
     * Requests to set the preferred network type for searching and registering.
     * (CS/PS domain, RAT, and operation mode)
     * @param networkType one of  NT_*_TYPE
     * @param response is callback message
     */
    @Override
    public void setPreferredNetworkType(int networkType, Message response) {
        ///M: [Network][C2K] TDD data only can not filtered by RadioAccessFamily since it's
        // Raf is same as LTE_ONLY, send it derectly. @{
        if (networkType == MtkRILConstants.NETWORK_MODE_LTE_TDD_ONLY) {
            mCi.setPreferredNetworkType(networkType, response);
            return;
        }
        /// @}

        // Only set preferred network types to that which the modem supports
        int modemRaf = getRadioAccessFamily();
        int rafFromType = MtkRadioAccessFamily.getRafFromNetworkType(networkType);

        if (modemRaf == MtkRadioAccessFamily.RAF_UNKNOWN
                || rafFromType == MtkRadioAccessFamily.RAF_UNKNOWN) {
            Rlog.d(LOG_TAG, "setPreferredNetworkType: Abort, unknown RAF: "
                    + modemRaf + " " + rafFromType);
            if (response != null) {
                CommandException ex;

                ex = new CommandException(CommandException.Error.GENERIC_FAILURE);
                AsyncResult.forMessage(response, null, ex);
                response.sendToTarget();
            }
            return;
        }

        int filteredRaf = (rafFromType & modemRaf) > MtkRadioAccessFamily.RAF_UNKNOWN ?
                (rafFromType & modemRaf) : modemRaf;
        int filteredType = MtkRadioAccessFamily.getNetworkTypeFromRaf(filteredRaf);

        Rlog.d(LOG_TAG, "setPreferredNetworkType: networkType = " + networkType
                + " modemRaf = " + modemRaf
                + " rafFromType = " + rafFromType
                + " filteredType = " + filteredType);

        mCi.setPreferredNetworkType(filteredType, response);
    }

    @Override
    public void selectNetworkManually(OperatorInfo network, boolean persistSelection,
            Message response) {
        // wrap the response message in our own message along with
        // the operator's id.
        NetworkSelectMessage nsm = new NetworkSelectMessage();
        nsm.message = response;
        nsm.operatorNumeric = network.getOperatorNumeric();
        nsm.operatorAlphaLong = network.getOperatorAlphaLong();
        nsm.operatorAlphaShort = network.getOperatorAlphaShort();

        Message msg = obtainMessage(EVENT_SET_NETWORK_MANUAL_COMPLETE, nsm);
        if (isPhoneTypeGsm()) {
            Rlog.d(LOG_TAG, "MTK GSMPhone selectNetworkManuallyWithAct:" + network);
            if (network.getOperatorAlphaLong() != null
                    && network.getOperatorAlphaLong().endsWith(GSM_INDICATOR)) {
                mMtkCi.setNetworkSelectionModeManualWithAct(network.getOperatorNumeric(),
                    ACT_TYPE_GSM, 0, msg);
            } else if (network.getOperatorAlphaLong() != null
                    && network.getOperatorAlphaLong().endsWith(UTRAN_INDICATOR)) {
                mMtkCi.setNetworkSelectionModeManualWithAct(network.getOperatorNumeric(),
                    ACT_TYPE_UTRAN, 0, msg);
            } else if (network.getOperatorAlphaLong() != null
                    && network.getOperatorAlphaLong().endsWith(LTE_INDICATOR)) {
                mMtkCi.setNetworkSelectionModeManualWithAct(network.getOperatorNumeric(),
                    ACT_TYPE_LTE, 0, msg);
            } else {
                mCi.setNetworkSelectionModeManual(network.getOperatorNumeric(), msg);
            }
        } else {
            mCi.setNetworkSelectionModeManual(network.getOperatorNumeric(), msg);
        }

        if (persistSelection) {
            updateSavedNetworkOperator(nsm);
        } else {
            clearSavedNetworkSelection();
        }
    }

    /**
     * Set network selection mode by semi automatic.
     * @param network is selection information
     * @param response is callback message
     */
    public void setNetworkSelectionModeSemiAutomatic(OperatorInfo network,
            Message response) {
        // wrap the response message in our own message along with
        // an empty string (to indicate automatic selection) for the
        // operator's id.
        NetworkSelectMessage nsm = new NetworkSelectMessage();
        nsm.message = response;
        nsm.operatorNumeric = "";
        nsm.operatorAlphaLong = "";
        nsm.operatorAlphaShort = "";
        Message msg = obtainMessage(EVENT_SET_NETWORK_AUTOMATIC_COMPLETE, nsm);

        Rlog.d(LOG_TAG, "MTK GSMPhone setNetworkSelectionModeSemiAutomatic:" + network);
        String actype = ACT_TYPE_GSM;
        if (network.getOperatorAlphaLong() != null
                && network.getOperatorAlphaLong().endsWith(UTRAN_INDICATOR)) {
            actype = ACT_TYPE_UTRAN;
        } else if (network.getOperatorAlphaLong() != null
                && network.getOperatorAlphaLong().endsWith(LTE_INDICATOR)) {
            actype = ACT_TYPE_LTE;
        }
        mMtkCi.setNetworkSelectionModeManualWithAct(network.getOperatorNumeric(), actype, 1, msg);
    }

    @Override
    public void getAvailableNetworks(Message response) {
        if (isPhoneTypeGsm() || isPhoneTypeCdmaLte()) {
            if (mMtkCi.showRat) mMtkCi.getAvailableNetworksWithAct(response);
            else mCi.getAvailableNetworks(response);
        } else {
            Rlog.d(LOG_TAG, "getAvailableNetworks: not possible in CDMA");
        }
    }

    /**
     * Cancel scan available networks. This method is synchronous; .
     * @param response is callback message
     */
    public synchronized void cancelAvailableNetworks(Message response) {
        Rlog.d(LOG_TAG, "cancelAvailableNetworks");
        mMtkCi.cancelAvailableNetworks(response);
    }

    // Femtocell (CSG) feature START
    /**
     * Scan available femtocells. This method is asynchronous.
     * @param operatorNumeric is operator mcc/mnc.
     * @param rat is operator rat.
     * @param response is callback message.
     */
    public void getFemtoCellList(Message response) {
        Rlog.d(LOG_TAG, "getFemtoCellList()");
        mMtkCi.getFemtoCellList(response);
    }

    /**
     * Abort scaning femtocell list.
     * @param response is callback message.
     */
    public void abortFemtoCellList(Message response) {
        Rlog.d(LOG_TAG, "abortFemtoCellList()");
        mMtkCi.abortFemtoCellList(response);
    }

    /**
     * Manually selects a femtocell. <code>response</code> is
     * @param femtocell is the specified femtocell to be selected
     * @param response is callback message
     */
    public void selectFemtoCell(FemtoCellInfo femtocell, Message response) {
        Rlog.d(LOG_TAG, "selectFemtoCell(): " + femtocell);
        mMtkCi.selectFemtoCell(femtocell, response);
    }

    /*
     * Query femto cell system selection mode
     * @param response is callback message
     */
    public void queryFemtoCellSystemSelectionMode(Message response) {
        Rlog.d(LOG_TAG, "queryFemtoCellSystemSelectionMode()");
        mMtkCi.queryFemtoCellSystemSelectionMode(response);
    }

    /**
     * Set femto cell system selection mode
     * @param mode specifies the preferred system selection mode
     * @param response is callback message
     */
    public void setFemtoCellSystemSelectionMode(int mode, Message response) {
        Rlog.d(LOG_TAG, "setFemtoCellSystemSelectionMode(), mode=" + mode);
        mMtkCi.setFemtoCellSystemSelectionMode(mode, response);
    }
    // Femtocell (CSG) feature END

    @Override
    protected Connection dialInternal(String dialString, DialArgs dialArgs,
            ResultReceiver wrappedCallback)
            throws CallStateException {

        // Need to make sure dialString gets parsed properly
        String newDialString = PhoneNumberUtils.stripSeparators(dialString);

        if (isPhoneTypeGsm()) {
            // handle in-call MMI first if applicable
            if (handleInCallMmiCommands(newDialString)) {
                return null;
            }

            // Only look at the Network portion for mmi
            String networkPortion = PhoneNumberUtils.extractNetworkPortionAlt(newDialString);
            MtkGsmMmiCode mmi = MtkGsmMmiCode.newFromDialString(networkPortion, this,
                    mUiccApplication.get(), wrappedCallback);
            if (DBG) logd("dialInternal: dialing w/ mmi '" + mmi + "'...");

            if (mmi == null) {
                return mCT.dialGsm(newDialString, dialArgs.uusInfo, dialArgs.intentExtras);
            } else if (mmi.isTemporaryModeCLIR()) {
                return mCT.dialGsm(mmi.mDialingNumber, mmi.getCLIRMode(), dialArgs.uusInfo,
                        dialArgs.intentExtras);
            } else {
                mPendingMMIs.add(mmi);
                /* Print MMI content */
                Rlog.d(LOG_TAG, "dialInternal: " + MtkSuppServHelper.encryptString(dialString)
                        + ", mmi=" + mmi);
                dumpPendingMmi();
                mMmiRegistrants.notifyRegistrants(new AsyncResult(null, mmi, null));
                mmi.processCode();
                return null;
            }
        } else {
            return mCT.dial(newDialString, dialArgs.intentExtras);
        }
    }

    // MTK-START: SIM GBA
    /**
     * Request security context authentication for USIM/SIM/ISIM
     */
    public void doGeneralSimAuthentication(int sessionId, int mode, int tag,
            String param1, String param2, Message result) {
        if (isPhoneTypeGsm()) {
            mMtkCi.doGeneralSimAuthentication(sessionId, mode, tag, param1, param2, result);
        }
    }
    // MTK-END

    // MTK-START: MVNO
    public String getMvnoPattern(String type) {
        String pattern = "";
        synchronized (mLock) {
            if (isPhoneTypeGsm()) {
                if (mIccRecords.get() != null) {
                    if (type.equals(MtkPhoneConstants.MVNO_TYPE_SPN)) {
                    //MTK-START
                        pattern = ((MtkSIMRecords) mIccRecords.get()).getSpNameInEfSpn();
                    //MTK-END
                    } else if (type.equals(MtkPhoneConstants.MVNO_TYPE_IMSI)) {
                    //MTK-START
                        pattern = ((MtkSIMRecords) mIccRecords.get()).isOperatorMvnoForImsi();
                    //MTK-END
                    } else if (type.equals(MtkPhoneConstants.MVNO_TYPE_PNN)) {
                        pattern = ((MtkSIMRecords) mIccRecords.get()).isOperatorMvnoForEfPnn();
                    } else if (type.equals(MtkPhoneConstants.MVNO_TYPE_GID)) {
                        pattern = mIccRecords.get().getGid1();
                    } else {
                        Rlog.d(LOG_TAG, "getMvnoPattern: Wrong type = " + type);
                    }
                }
            }
        }
        return pattern;
    }

    public String getMvnoMatchType() {
        String type = MtkPhoneConstants.MVNO_TYPE_NONE;
        synchronized (mLock) {
            if (isPhoneTypeGsm()) {
                if (mIccRecords.get() != null) {
                 //MTK-START
                    type = ((MtkSIMRecords) mIccRecords.get()).getMvnoMatchType();
                 //MTK-END
                }
                Rlog.d(LOG_TAG, "getMvnoMatchType: Type = " + type);
            }
        }
        return type;
    }

    // M: PS/CS concurrent @{
    @Override
    protected void updateImsPhone() {
        Rlog.d(LOG_TAG, "updateImsPhone");

        MtkDcHelper dcHelper = MtkDcHelper.getInstance();

        if (mImsServiceReady && (mImsPhone == null)) {
            super.updateImsPhone();
            if (dcHelper != null) {
                dcHelper.registerImsEvents(getPhoneId());
            }
        } else if (!mImsServiceReady && (mImsPhone != null)) {
            if (dcHelper != null) {
                dcHelper.unregisterImsEvents(getPhoneId());
            }
            super.updateImsPhone();
        }
    }
    // M: PS/CS concurrent @}
    // MTK-END

    /// M: CC: HangupAll for FTA 31.4.4.2 @{
    public void hangupAll() throws CallStateException {
        ((MtkGsmCdmaCallTracker)mCT).hangupAll();
    }
    /// @}

    /// M: CC: Check GSM call state to avoid InCallMMI dispatching to IMS @{
    // [ALPS02516173],[ALPS02615800]
    public Call getCSRingingCall() {
        return mCT.mRingingCall;
    }

    boolean isInCSCall() {
        MtkGsmCdmaCall.State foregroundCallState = getForegroundCall().getState();
        MtkGsmCdmaCall.State backgroundCallState = getBackgroundCall().getState();
        MtkGsmCdmaCall.State ringingCallState = getCSRingingCall().getState();

       return (foregroundCallState.isAlive() ||
                backgroundCallState.isAlive() ||
                ringingCallState.isAlive());
    }
    /// @}

    /// M: CC: GSM 02.07 B.1.26 Ciphering Indicator support @{
    /**
     * Registers the handler when network reports cipher indication info for the voice call.
     *
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     */
    public void registerForCipherIndication(Handler h, int what, Object obj) {
        mCipherIndicationRegistrants.addUnique(h, what, obj);
    }

    /**
     * Unregister for notifications when network reports cipher indication info for the voice call.
     *
     * @param h Handler to be removed from the registrant list.
     */
    public void unregisterForCipherIndication(Handler h) {
        mCipherIndicationRegistrants.remove(h);
    }
    /// @}

    @Override
    public Connection dial(String dialString, @NonNull DialArgs dialArgs)
            throws CallStateException {
        if (!isPhoneTypeGsm() && dialArgs.uusInfo != null) {
            throw new CallStateException("Sending UUS information NOT supported in CDMA!");
        }

        boolean isEmergency = PhoneNumberUtils.isEmergencyNumber(getSubId(), dialString);
        MtkLocalPhoneNumberUtils.setIsEmergencyNumber(isEmergency);

        Phone imsPhone = mImsPhone;

        /// M: E911 Wifi control. @{
        if (isEmergency) {
            tryTurnOffWifiForE911(isEmergency);
        }
        /// @}

        // M: for improve dial performance.
        int imsServiceState = (imsPhone != null) ?
                imsPhone.getServiceState().getState() : ServiceState.STATE_POWER_OFF;

        // M: for improve dial performance.
        boolean alwaysTryImsForEmergencyCarrierConfig = false;
        if (isEmergency) {
            CarrierConfigManager configManager = (CarrierConfigManager) mContext.getSystemService(
                    Context.CARRIER_CONFIG_SERVICE);
            alwaysTryImsForEmergencyCarrierConfig = configManager.getConfigForSubId(getSubId())
                    .getBoolean(CarrierConfigManager.KEY_CARRIER_USE_IMS_FIRST_FOR_EMERGENCY_BOOL);
        }

        /** Check if the call is Wireless Priority Service call */
        boolean isWpsCall = dialString != null ? dialString.startsWith(PREFIX_WPS) : false;
        // M: for improve dial performance.
        boolean allowWpsOverIms = true;
        if (isWpsCall) {
            CarrierConfigManager configManager = (CarrierConfigManager) mContext.getSystemService(
                    Context.CARRIER_CONFIG_SERVICE);
            allowWpsOverIms = configManager.getConfigForSubId(getSubId())
                    .getBoolean(CarrierConfigManager.KEY_SUPPORT_WPS_OVER_IMS_BOOL);
        }

        boolean useImsForCall = isImsUseEnabled()
                 && imsPhone != null
                 && (imsPhone.isVolteEnabled() || imsPhone.isWifiCallingEnabled() ||
                 (imsPhone.isVideoEnabled() && VideoProfile.isVideo(dialArgs.videoState)))
                 && (imsServiceState == ServiceState.STATE_IN_SERVICE)
                 && (isWpsCall ? allowWpsOverIms : true);

        boolean useImsForEmergency = imsPhone != null
                && isEmergency
                && alwaysTryImsForEmergencyCarrierConfig
                && ImsManager.getInstance(mContext, mPhoneId).isNonTtyOrTtyOnVolteEnabled()
                && (imsServiceState != ServiceState.STATE_POWER_OFF);

        /// M: CC: ECC for Fusion RIL @{
        if (hasC2kOverImsModem()) {
            if (DBG) Rlog.d(LOG_TAG, "keep AOSP");
        } else {
            /// M: CC: Workaround for separate C2K not on IMS modem @{
            if (!isPhoneTypeGsm()) {
                useImsForEmergency = false;
            }
            /// @}
        }
        /// @}

        // ALPS03042143, IMS ECC should be dialed via the phone with main capability
        // TODO: remove this workaround for dual VoLTE project
        if (mPhoneId != getMainCapabilityPhoneId() && MtkImsManager.isSupportMims() == false) {
            useImsForEmergency = false;
        }

        if (shouldProcessSelfActivation() || useImsForPCOChanged()) {
            logd("always use ImsPhone for self activation");
            useImsForCall = true;
        }

        String dialPart = PhoneNumberUtils.extractNetworkPortionAlt(PhoneNumberUtils.
                stripSeparators(dialString));
        boolean isUt = (dialPart.startsWith("*") || dialPart.startsWith("#"))
                && dialPart.endsWith("#");

        boolean useImsForUt = imsPhone != null && imsPhone.isUtEnabled();

        if (DBG) {
            logd("PhoneId = " + mPhoneId
                    + ", useImsForCall=" + useImsForCall
                    + ", useImsForEmergency=" + useImsForEmergency
                    + ", useImsForUt=" + useImsForUt
                    + ", isUt=" + isUt
                    + ", isWpsCall=" + isWpsCall
                    + ", allowWpsOverIms=" + allowWpsOverIms
                    + ", imsPhone=" + imsPhone
                    + ", imsPhone.isVolteEnabled()="
                    + ((imsPhone != null) ? imsPhone.isVolteEnabled() : "N/A")
                    + ", imsPhone.isVowifiEnabled()="
                    + ((imsPhone != null) ? imsPhone.isWifiCallingEnabled() : "N/A")
                    + ", imsPhone.isVideoEnabled()="
                    + ((imsPhone != null) ? imsPhone.isVideoEnabled() : "N/A")
                    + ", imsPhone.getServiceState().getState()="
                    + ((imsPhone != null) ? imsServiceState : "N/A"));
        }

        int setting = SubscriptionManager.getIntegerSubscriptionProperty(getSubId(),
                SubscriptionManager.WFC_IMS_MODE,
                ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED, mContext);
        if (setting == ImsConfig.WfcModeFeatureValueConstants.WIFI_ONLY) {
            Phone.checkWfcWifiOnlyModeBeforeDial(mImsPhone, mPhoneId, mContext);
        }

        if ((useImsForCall && !isUt) || (isUt && useImsForUt) || useImsForEmergency) {
            /// M: CC: Check GSM call state to avoid InCallMMI dispatching to IMS @{
            // [ALPS02516173],[ALPS02615800]
            if (isInCSCall()) {
                if (DBG) Rlog.d(LOG_TAG, "has CS Call. Don't try IMS PS Call!");
            } else {
            /// @}
                try {
                    if (dialArgs.videoState == VideoProfile.STATE_AUDIO_ONLY) {
                        if (DBG) {
                            logd("Trying IMS PS call");
                        }
                        return imsPhone.dial(dialString, dialArgs);
                    } else {
                        if (SystemProperties.get("persist.vendor.vilte_support").equals("1")) {
                            if (DBG) {
                                logd("Trying IMS PS video call");
                            }
                            return imsPhone.dial(dialString, dialArgs);
                        } else {
                            loge("Should not be here. (isInCSCall == false, videoState="
                                    + dialArgs.videoState);
                        }
                    }

                } catch (CallStateException e) {
                    if (DBG) logd("IMS PS call exception " + e +
                            "useImsForCall =" + useImsForCall + ", imsPhone =" + imsPhone);
                    /// M: E911 Wifi control. @{
                    tryTurnOnWifiForE911Finished();
                    /// @}
                    // Do not throw a CallStateException and instead fall back to Circuit switch
                    // for emergency calls and MMI codes.
                    if (Phone.CS_FALLBACK.equals(e.getMessage()) || isEmergency) {
                        logi("IMS call failed with Exception: " + e.getMessage() + ". Falling back "
                                + "to CS.");
                    } else {
                        CallStateException ce = new CallStateException(
                                e.getError(), e.getMessage());
                        ce.setStackTrace(e.getStackTrace());
                        throw ce;
                    }
                }
            /// M: CC: Check GSM call state to avoid InCallMMI dispatching to IMS @{
            }
            /// @}
        }

        /// M: CC: FTA requires call should be dialed out even out of service @{
        if (SystemProperties.getInt("vendor.gsm.gcf.testmode", 0) != 2
                && isCdmaLessDevice() == false) {
            if (mSST != null && mSST.mSS.getState() == ServiceState.STATE_OUT_OF_SERVICE
                    && mSST.mSS.getDataRegState() != ServiceState.STATE_IN_SERVICE
                    && !isEmergency) {
                throw new CallStateException("cannot dial in current state");
            }
        }
        /// @}

        // Check non-emergency voice CS call - shouldn't dial when POWER_OFF
        if (mSST != null && mSST.mSS.getState() == ServiceState.STATE_POWER_OFF /* CS POWER_OFF */
                && !VideoProfile.isVideo(dialArgs.videoState) /* voice call */
                && !isEmergency /* non-emergency call */) {
            throw new CallStateException(
                CallStateException.ERROR_POWER_OFF,
                "cannot dial voice call in airplane mode");
        }
        // Check for service before placing non emergency CS voice call.
        // Allow dial only if either CS is camped on any RAT (or) PS is in LTE service.
        /// M: CC: FTA requires call should be dialed out even out of service @{
        if (SystemProperties.getInt("vendor.gsm.gcf.testmode", 0) != 2
                && isCdmaLessDevice() == false
                && mSST != null
                && mSST.mSS.getState() == ServiceState.STATE_OUT_OF_SERVICE /* CS out of service */
                && !(mSST.mSS.getDataRegState() == ServiceState.STATE_IN_SERVICE
                    && ServiceState.isLte(mSST.mSS.getRilDataRadioTechnology())) /* PS not in LTE */
                && !VideoProfile.isVideo(dialArgs.videoState) /* voice call */
                && !isEmergency /* non-emergency call */
                && !(isUt && useImsForUt) /* not UT */) {
            throw new CallStateException(
                CallStateException.ERROR_OUT_OF_SERVICE,
                "cannot dial voice call in out of service");
        }
        /// @}
        if (DBG) logd("Trying (non-IMS) CS call");

        if (isPhoneTypeGsm()) {
            return dialInternal(dialString, new DialArgs.Builder<>()
                    .setIntentExtras(dialArgs.intentExtras)
                    .build());
        } else {
            return dialInternal(dialString, dialArgs);
        }
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        Message onComplete;

        switch (msg.what) {
            // handle the get apc callbacks.
            case EVENT_GET_APC_INFO:
                Rlog.d(LOG_TAG, "handle EVENT_GET_APC_INFO");
                ar = (AsyncResult) msg.obj;
                PseudoCellInfoResult result = (PseudoCellInfoResult) ar.userObj;
                if (result == null) {
                    Rlog.e(LOG_TAG, "EVENT_GET_APC_INFO: result return null");
                    return;
                }
                synchronized(result.lockObj) {
                    if (ar.exception != null) {
                        Rlog.d(LOG_TAG, "EVENT_GET_APC_INFO: error ret null, e=" + ar.exception);
                        result.infos = null;
                    } else {
                        int[] msgs = (int[]) ar.result;
                        result.infos = new PseudoCellInfo(msgs);
                    }
                    result.lockObj.notify();
                }
                break;

            case EVENT_SSN:
                logd("Event EVENT_SSN Received");
                if (isPhoneTypeGsm()) {
                    ar = (AsyncResult) msg.obj;
                    SuppServiceNotification not = (SuppServiceNotification) ar.result;

                    /// M: CC: Proprietary CRSS handling @{
                    if (not.notificationType == 1) {       // for MT cases
                        if (not.code == SuppServiceNotification.CODE_2_FORWARDED_CALL) {
                            logd("skip AOSP event for MT forwarded call notification");
                            break;
                        }
                    }
                    /// @}

                    ar = new AsyncResult(null, not, null);
                    /// M: CC: Proprietary CRSS handling @{
                    if (mSsnRegistrants.size() == 0) {
                        mCachedSsn = ar;
                    }
                    /// @}
                    mSsnRegistrants.notifyRegistrants(ar);
                }
                break;

            /// M: CC: Proprietary CRSS handling @{
            case EVENT_SSN_EX:
                logd("Event EVENT_SSN_EX Received");
                if (isPhoneTypeGsm()) {
                    ar = (AsyncResult) msg.obj;
                    MtkSuppServiceNotification not = (MtkSuppServiceNotification) ar.result;

                    if (not.notificationType == 1) {       // for MT cases
                        if (not.code == SuppServiceNotification.CODE_2_FORWARDED_CALL ||
                                not.code >= MtkSuppServiceNotification.MT_CODE_FORWARDED_CF) {
                            ar = new AsyncResult(null, not, null);
                            if (mSsnRegistrants.size() == 0) {
                                mCachedSsn = ar;
                            }
                            mSsnRegistrants.notifyRegistrants(ar);
                        } else {
                            logd("Unexpected SSN_EX code:" + not.code);
                        }
                    }
                }
                break;

            case EVENT_CRSS_IND:
                ar = (AsyncResult) msg.obj;
                MtkSuppCrssNotification noti = (MtkSuppCrssNotification) ar.result;

                /// M: CC: number presentation via CLIP is not necessary, should be updated by CLCC
                /// M: CC: Redirecting number via COLP @{
                if (noti.code == MtkSuppCrssNotification.CRSS_CONNECTED_LINE_ID_PREST) {
                /* If the phone number in +COLP is different from the address of connection,
                       store it to connection as redirecting address.
                    */
                    Rlog.d(LOG_TAG, "[COLP]noti.number = " + Rlog.pii(LOG_TAG, noti.number));
                    if (getForegroundCall().getState() != GsmCdmaCall.State.IDLE) {
                        MtkGsmCdmaConnection cn = (MtkGsmCdmaConnection) (getForegroundCall()
                                .getConnections().get(0));
                        if ((cn != null) &&
                            (cn.getAddress() != null) &&
                            !cn.getAddress().equals(noti.number)) {
                           cn.setRedirectingAddress(noti.number);
                           Rlog.d(LOG_TAG, "[COLP]Redirecting address = " +
                                   Rlog.pii(LOG_TAG, cn.getRedirectingAddress()));
                        }
                    }
                }
                /// @}

                if (mCallRelatedSuppSvcRegistrants.size() == 0) {
                    mCachedCrssn = ar;
                }
                mCallRelatedSuppSvcRegistrants.notifyRegistrants(ar);
                break;
                /// @}

            case EVENT_GET_CALL_FORWARD_DONE:
                /* For solving ALPS00997715 */
                Rlog.d(LOG_TAG, "mPhoneId= " + mPhoneId + "subId=" + getSubId());
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    handleCfuQueryResult((CallForwardInfo[])ar.result);
                } else {
                    if (supportMdAutoSetupIms()) {
                        CommandException cmdException = (CommandException) ar.exception;
                        if (cmdException.getCommandError() == CommandException.Error.OEM_ERROR_25) {
                            if (cmdException.getMessage() != null &&
                                    cmdException.getMessage().isEmpty()) {
                                // For 409 error and assign the error message to CommandException.
                                MtkSuppServHelper ssHelper =
                                        MtkSuppServManager.getSuppServHelper(getPhoneId());
                                String errorMsg = null;
                                if (ssHelper != null) {
                                    errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                            CommandException.Error.OEM_ERROR_25);
                                    if (errorMsg != null && !errorMsg.isEmpty()) {
                                        ar.exception = new CommandException(
                                                CommandException.Error.OEM_ERROR_25,
                                                errorMsg);
                                    }
                                }
                            }
                        }
                    }
                }
                onComplete = (Message) ar.userObj;
                if (onComplete != null) {
                    AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                    onComplete.sendToTarget();
                }
                break;

            case EVENT_SET_CALL_FORWARD_DONE:
                /// M: 93 Ims SS native solution @{
                if (supportMdAutoSetupIms()) {
                    ar = (AsyncResult)msg.obj;
                    IccRecords r = mIccRecords.get();
                    Cfu cfu = (Cfu) ar.userObj;
                    if (ar.exception == null && r != null) {
                        if ((cfu.mServiceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) {
                            setVoiceCallForwardingFlag(1, msg.arg1 == 1, cfu.mSetCfNumber);
                        }
                    } else {
                        if (supportMdAutoSetupIms()) {
                            CommandException cmdException = (CommandException) ar.exception;
                            if (cmdException.getCommandError() ==
                                    CommandException.Error.OEM_ERROR_25) {
                                if (cmdException.getMessage() != null &&
                                        cmdException.getMessage().isEmpty()) {
                                    // For 409 error and assign the error message
                                    // to CommandException.
                                    MtkSuppServHelper ssHelper =
                                        MtkSuppServManager.getSuppServHelper(getPhoneId());
                                    String errorMsg = null;
                                    if (ssHelper != null) {
                                        errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                                CommandException.Error.OEM_ERROR_25);
                                        if (errorMsg != null && !errorMsg.isEmpty()) {
                                            ar.exception = new CommandException(
                                                    CommandException.Error.OEM_ERROR_25,
                                                    errorMsg);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (cfu.mOnComplete != null) {
                        AsyncResult.forMessage(cfu.mOnComplete, ar.result, ar.exception);
                        cfu.mOnComplete.sendToTarget();
                    }
                    break;
                }
                /// @}

                ar = (AsyncResult)msg.obj;
                IccRecords r = mIccRecords.get();
                Cfu cfu = (Cfu) ar.userObj;
                if (ar.exception == null && r != null) {
                    /* Only CFU would go in this case.
                     * because only CFU use EVENT_SET_CALL_FORWARD_DONE.
                     * So no need to check it is for CFU.
                     */
                    if (queryCFUAgainAfterSet()) {
                        if (ar.result != null) {
                            CallForwardInfo[] cfinfo = (CallForwardInfo[]) ar.result;
                            if (cfinfo == null || cfinfo.length == 0) {
                                Rlog.d(LOG_TAG, "cfinfo is null or length is 0.");
                            } else {
                                Rlog.d(LOG_TAG, "[EVENT_SET_CALL_FORWARD_DONE] check cfinfo");
                                for (int i = 0 ; i < cfinfo.length ; i++) {
                                    if ((cfinfo[i].serviceClass & SERVICE_CLASS_VOICE) != 0) {
                                        setVoiceCallForwardingFlag(1, (cfinfo[i].status == 1),
                                                cfinfo[i].number);
                                        break;
                                    }
                                }
                            }
                        } else {
                            Rlog.e(LOG_TAG, "EVENT_SET_CALL_FORWARD_DONE: ar.result is null.");
                        }
                    } else {
                        setVoiceCallForwardingFlag(1, msg.arg1 == 1, cfu.mSetCfNumber);
                    }
                }
                if (cfu.mOnComplete != null) {
                    AsyncResult.forMessage(cfu.mOnComplete, ar.result, ar.exception);
                    cfu.mOnComplete.sendToTarget();
                }
                break;

            case EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE:
                Rlog.d(LOG_TAG, "mPhoneId = " + mPhoneId + ", subId = " + getSubId());
                ar = (AsyncResult) msg.obj;
                Rlog.d(LOG_TAG, "[EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE]ar.exception = "
                        + ar.exception);
                if (ar.exception == null) {
                    handleCfuInTimeSlotQueryResult((MtkCallForwardInfo[]) ar.result);
                }
                Rlog.d(LOG_TAG, "[EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE]msg.arg1 = "
                        + msg.arg1);
                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    CommandException cmdException = (CommandException) ar.exception;
                    Rlog.d(LOG_TAG, "[EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE] cmdException error:" +
                            cmdException.getCommandError());

                    if ((msg.arg1 == 1) && (cmdException != null) &&
                            (cmdException.getCommandError() ==
                                    CommandException.Error.REQUEST_NOT_SUPPORTED)) {
                        if (mSST != null && mSST.mSS != null
                                && (mSST.mSS.getState() == ServiceState.STATE_IN_SERVICE)) {
                            getCallForwardingOption(CF_REASON_UNCONDITIONAL,
                                    obtainMessage(EVENT_GET_CALL_FORWARD_DONE));
                        }
                    }

                    if (supportMdAutoSetupIms()) {
                        if ((cmdException != null) &&
                            (cmdException.getCommandError() == CommandException.Error.OEM_ERROR_2)){
                            Rlog.d(LOG_TAG, "return REQUEST_NOT_SUPPORTED");
                            ar.exception = new CommandException(
                                    CommandException.Error.REQUEST_NOT_SUPPORTED);
                        }
                    }
                }
                onComplete = (Message) ar.userObj;
                if (onComplete != null) {
                    AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                    onComplete.sendToTarget();
                }
                break;

            case EVENT_SET_CALL_FORWARD_TIME_SLOT_DONE:
                ar = (AsyncResult) msg.obj;
                IccRecords records = mIccRecords.get();
                CfuEx cfuEx = (CfuEx) ar.userObj;
                if (ar.exception == null && records != null) {
                    records.setVoiceCallForwardingFlag(1, msg.arg1 == 1, cfuEx.mSetCfNumber);
                    saveTimeSlot(cfuEx.mSetTimeSlot);
                }
                if (cfuEx.mOnComplete != null) {
                    AsyncResult.forMessage(cfuEx.mOnComplete, ar.result, ar.exception);
                    cfuEx.mOnComplete.sendToTarget();
                }
                break;

            case EVENT_GET_CALL_WAITING_DONE:
                ar = (AsyncResult) msg.obj;
                Rlog.d(LOG_TAG, "[EVENT_GET_CALL_WAITING_]ar.exception = " + ar.exception);

                onComplete = (Message) ar.userObj;
                if (ar.exception == null) {
                    int[] cwArray = (int[]) ar.result;
                    // If cwArray[0] is = 1, then cwArray[1] must follow,
                    // with the TS 27.007 service class bit vector of services
                    // for which call waiting is enabled.
                    try {
                        Rlog.d(LOG_TAG, "EVENT_GET_CALL_WAITING_DONE cwArray[0]:cwArray[1] = "
                                + cwArray[0] + ":" + cwArray[1]);

                        boolean csEnable = ((cwArray[0] == 1) &&
                            ((cwArray[1] & 0x01) == SERVICE_CLASS_VOICE));

                        if (!supportMdAutoSetupIms()) {
                            setTerminalBasedCallWaiting(csEnable, null);
                        }

                        if (onComplete != null) {
                            AsyncResult.forMessage(onComplete, ar.result, null);
                            onComplete.sendToTarget();
                            break;
                        }
                    } catch (ArrayIndexOutOfBoundsException e) {
                        Rlog.e(LOG_TAG, "EVENT_GET_CALL_WAITING_DONE: improper result: err ="
                                + e.getMessage());
                        if (onComplete != null) {
                            AsyncResult.forMessage(onComplete, ar.result, null);
                            onComplete.sendToTarget();
                            break;
                        }
                    }
                } else {
                    if (supportMdAutoSetupIms()) {
                        CommandException cmdException = (CommandException) ar.exception;
                        if (cmdException.getCommandError() == CommandException.Error.OEM_ERROR_25) {
                            if (cmdException.getMessage() != null &&
                                    cmdException.getMessage().isEmpty()) {
                                // For 409 error and assign the error message to CommandException.
                                MtkSuppServHelper ssHelper =
                                        MtkSuppServManager.getSuppServHelper(getPhoneId());
                                String errorMsg = null;
                                if (ssHelper != null) {
                                    errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                            CommandException.Error.OEM_ERROR_25);
                                    if (errorMsg != null && !errorMsg.isEmpty()) {
                                        ar.exception = new CommandException(
                                                CommandException.Error.OEM_ERROR_25,
                                                errorMsg);
                                    }
                                }
                            }
                        }
                    }

                    if (onComplete != null) {
                        AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                        onComplete.sendToTarget();
                        break;
                    }
                }
                break;

            case EVENT_SET_CALL_WAITING_DONE:
                ar = (AsyncResult) msg.obj;
                onComplete = (Message) ar.userObj;
                Rlog.d(LOG_TAG, "EVENT_SET_CALL_WAITING_DONE: ar.exception=" + ar.exception);

                if (ar.exception != null) {
                    if (supportMdAutoSetupIms()) {
                        CommandException cmdException = (CommandException) ar.exception;
                        if (cmdException.getCommandError() == CommandException.Error.OEM_ERROR_25) {
                            if (cmdException.getMessage() != null &&
                                    cmdException.getMessage().isEmpty()) {
                                // For 409 error and assign the error message to CommandException.
                                MtkSuppServHelper ssHelper =
                                        MtkSuppServManager.getSuppServHelper(getPhoneId());
                                String errorMsg = null;
                                if (ssHelper != null) {
                                    errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                            CommandException.Error.OEM_ERROR_25);
                                    if (errorMsg != null && !errorMsg.isEmpty()) {
                                        ar.exception = new CommandException(
                                                CommandException.Error.OEM_ERROR_25,
                                                errorMsg);
                                    }
                                }
                            }
                        }
                    }

                    if (onComplete != null) {
                        AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                        onComplete.sendToTarget();
                        break;
                    }
                } else {
                    if (!supportMdAutoSetupIms()) {
                        boolean enable = msg.arg1 == 1 ? true : false;
                        setTerminalBasedCallWaiting(enable, onComplete);
                    } else {
                        if (onComplete != null) {
                            AsyncResult.forMessage(onComplete, null, null);
                            onComplete.sendToTarget();
                        }
                    }
                }
                break;

            case EVENT_ICC_RECORD_EVENTS: {
                Rlog.d(LOG_TAG, "EVENT_ICC_RECORD_EVENTS");
                ar = (AsyncResult)msg.obj;
                processIccRecordEvents((Integer)ar.result);

                MtkSuppServHelper ssHelper = MtkSuppServManager.getSuppServHelper(getPhoneId());
                if (ssHelper != null) {
                    ssHelper.setIccRecordsReady();
                }
                break;
            }
            case EVENT_SET_CLIR_COMPLETE:
                Rlog.d(LOG_TAG, "EVENT_SET_CLIR_COMPLETE");
                ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    saveClirSetting(msg.arg1);
                }

                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    CommandException cmdException = (CommandException) ar.exception;
                    Rlog.d(LOG_TAG, "EVENT_SET_CLIR_COMPLETE: cmdException error:" +
                            cmdException.getCommandError());

                    if (supportMdAutoSetupIms()) {
                        if (cmdException != null) {
                            if ((isOp(OPID.OP01) || isOp(OPID.OP02)) &&
                                        isUtError(cmdException.getCommandError())) {
                                Rlog.d(LOG_TAG, "return REQUEST_NOT_SUPPORTED");
                                ar.exception = new CommandException(
                                        CommandException.Error.REQUEST_NOT_SUPPORTED);
                            } else if (cmdException.getCommandError() ==
                                        CommandException.Error.OEM_ERROR_25) {
                                if (cmdException.getMessage() != null &&
                                        cmdException.getMessage().isEmpty()) {
                                    // For 409 error and assign the error message
                                    // to CommandException.
                                    MtkSuppServHelper ssHelper =
                                        MtkSuppServManager.getSuppServHelper(getPhoneId());
                                    String errorMsg = null;
                                    if (ssHelper != null) {
                                        errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                                CommandException.Error.OEM_ERROR_25);
                                        if (errorMsg != null && !errorMsg.isEmpty()) {
                                            ar.exception = new CommandException(
                                                    CommandException.Error.OEM_ERROR_25,
                                                    errorMsg);
                                        }
                                    }
                                }
                            } else {
                                Rlog.d(LOG_TAG, "return Original Error");
                            }
                        }
                    }
                }

                onComplete = (Message) ar.userObj;
                if (onComplete != null) {
                    AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                    onComplete.sendToTarget();
                }
                break;

            case EVENT_GET_CLIR_COMPLETE:
                Rlog.d(LOG_TAG, "EVENT_GET_CLIR_COMPLETE");
                ar = (AsyncResult)msg.obj;

                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    CommandException cmdException = (CommandException) ar.exception;
                    Rlog.d(LOG_TAG, "EVENT_GET_CLIR_COMPLETE: cmdException error:" +
                            cmdException.getCommandError());

                    if (supportMdAutoSetupIms()) {
                        if (cmdException != null) {
                            if (isOp(OPID.OP01) || isOp(OPID.OP02)) {
                                if (isUtError(cmdException.getCommandError())) {
                                    Rlog.d(LOG_TAG, "return REQUEST_NOT_SUPPORTED");
                                    ar.exception = new CommandException(
                                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                                } else {
                                    Rlog.d(LOG_TAG, "return Original Error");
                                }
                            } else if (cmdException.getCommandError() ==
                                    CommandException.Error.OEM_ERROR_25) {
                                Rlog.d(LOG_TAG, "cmdException.getMessage():" +
                                        cmdException.getMessage());
                                if (cmdException.getMessage() != null &&
                                        cmdException.getMessage().isEmpty()) {
                                    // For 409 error and assign the error message
                                    // to CommandException.
                                    MtkSuppServHelper ssHelper =
                                        MtkSuppServManager.getSuppServHelper(getPhoneId());
                                    String errorMsg = null;
                                    if (ssHelper != null) {
                                        errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                                CommandException.Error.OEM_ERROR_25);
                                        if (errorMsg != null && !errorMsg.isEmpty()) {
                                            ar.exception = new CommandException(
                                                    CommandException.Error.OEM_ERROR_25,
                                                    errorMsg);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                onComplete = (Message) ar.userObj;
                if (onComplete != null) {
                    AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                    onComplete.sendToTarget();
                }
                break;

            case EVENT_SET_CALL_BARRING_COMPLETE:
                Rlog.d(LOG_TAG, "EVENT_SET_CALL_BARRING_COMPLETE");
                ar = (AsyncResult)msg.obj;

                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    CommandException cmdException = (CommandException) ar.exception;
                    Rlog.d(LOG_TAG, "EVENT_SET_CALL_BARRING_COMPLETE: cmdException error:" +
                            cmdException.getCommandError());

                    if (supportMdAutoSetupIms()) {
                        if (cmdException != null) {
                            if (isOp(OPID.OP01)) {
                                if (isUtError(cmdException.getCommandError())) {
                                    Rlog.d(LOG_TAG, "return REQUEST_NOT_SUPPORTED");
                                    ar.exception = new CommandException(
                                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                                } else {
                                    Rlog.d(LOG_TAG, "return Original Error");
                                }
                            } else if (cmdException.getCommandError() ==
                                    CommandException.Error.OEM_ERROR_25) {
                                if (cmdException.getMessage() != null &&
                                        cmdException.getMessage().isEmpty()) {
                                    // For 409 error and assign the error message
                                    // to CommandException.
                                    MtkSuppServHelper ssHelper =
                                            MtkSuppServManager.getSuppServHelper(getPhoneId());
                                    String errorMsg = null;
                                    if (ssHelper != null) {
                                        errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                                CommandException.Error.OEM_ERROR_25);
                                        if (errorMsg != null && !errorMsg.isEmpty()) {
                                            ar.exception = new CommandException(
                                                    CommandException.Error.OEM_ERROR_25,
                                                    errorMsg);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                onComplete = (Message) ar.userObj;
                if (onComplete != null) {
                    AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                    onComplete.sendToTarget();
                }
                break;

            case EVENT_GET_CALL_BARRING_COMPLETE:
                Rlog.d(LOG_TAG, "EVENT_GET_CALL_BARRING_COMPLETE");
                ar = (AsyncResult)msg.obj;

                if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                    CommandException cmdException = (CommandException) ar.exception;
                    Rlog.d(LOG_TAG, "EVENT_GET_CALL_BARRING_COMPLETE: cmdException error:" +
                            cmdException.getCommandError());

                    if (supportMdAutoSetupIms()) {
                        if (cmdException != null) {
                            if (isOp(OPID.OP01) || isOp(OPID.OP09)) {
                                if (isUtError(cmdException.getCommandError())) {
                                    Rlog.d(LOG_TAG, "return REQUEST_NOT_SUPPORTED");
                                    ar.exception = new CommandException(
                                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                                } else {
                                    Rlog.d(LOG_TAG, "return Original Error");
                                }
                            } else if (cmdException.getCommandError() ==
                                    CommandException.Error.OEM_ERROR_25) {
                                if (cmdException.getMessage() != null &&
                                        cmdException.getMessage().isEmpty()) {
                                    // For 409 error and assign the error message
                                    // to CommandException.
                                    MtkSuppServHelper ssHelper =
                                            MtkSuppServManager.getSuppServHelper(getPhoneId());
                                    String errorMsg = null;
                                    if (ssHelper != null) {
                                        errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                                CommandException.Error.OEM_ERROR_25);
                                        if (errorMsg != null && !errorMsg.isEmpty()) {
                                            ar.exception = new CommandException(
                                                    CommandException.Error.OEM_ERROR_25,
                                                    errorMsg);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                onComplete = (Message) ar.userObj;
                if (onComplete != null) {
                    AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                    onComplete.sendToTarget();
                }
                break;

            case EVENT_IMS_UT_DONE:
                Rlog.d(LOG_TAG, "EVENT_IMS_UT_DONE: Enter");

                handleImsUtDone(msg);
                break;

            case EVENT_IMS_UT_CSFB:
                handleImsUtCsfb(msg);
                break;

            // M: Restore to auto mode when manual setting is fail
            case EVENT_SET_NETWORK_MANUAL_COMPLETE:
                super.handleMessage(msg);
                if (isPhoneTypeGsm()) {
                    ar = (AsyncResult) msg.obj;
                    boolean restoreSelection = !mContext.getResources().getBoolean(
                                com.android.internal.R.bool.skip_restoring_network_selection);
                    boolean exception = ((ar == null) ? false :
                            ((ar.exception == null) ? false : true));
                    Rlog.d(LOG_TAG, "EVENT_SET_NETWORK_MANUAL_COMPLETE, restoreSelection="
                            + restoreSelection + " exception=" + exception);
                    if (!restoreSelection && exception) {
                        // clear the saved network selection by AOSP API
                        clearSavedNetworkSelection();
                        mCi.setNetworkSelectionModeAutomatic(null);
                    }
                }
                break;
            case EVENT_SET_NETWORK_AUTOMATIC_COMPLETE:
                /// insert expection if it's not in service now
                if (isPhoneTypeGsm()) {
                    ar = (AsyncResult) msg.obj;
                    MtkServiceState mss = (MtkServiceState) getServiceState();
                    Rlog.d(LOG_TAG, "EVENT_SET_NETWORK_AUTOMATIC_COMPLETE, getCellularRegState="
                            + mss.getCellularRegState());
                    if (mss.getCellularRegState() != ServiceState.STATE_IN_SERVICE) {
                        // both CS and PS are not in serivce
                        ar.exception = new CommandException(CommandException.Error.ABORTED);
                    }
                }
                super.handleMessage(msg);
                break;
            /// M: CC: Timing issue for ECC ended but not in ECM. @{
            case EVENT_EMERGENCY_CALLBACK_MODE_ENTER:
                if (!isPhoneTypeGsm()) {
                    boolean inEcm = isInEcm();
                    super.handleMessage(msg);
                    if (!inEcm) {
                        /**
                         * When hanging up ECC call, the call may be ended immediately
                         * but not enter ECM at the same time,
                         * the data will be enabled again in this case.
                         * So, disable data again when entering ECM.
                         */
                        mDataEnabledSettings.setInternalDataEnabled(false);
                        notifyEmergencyCallRegistrants(true);
                    }
                } else {
                    // Keep AOSP
                    super.handleMessage(msg);
                }
                break;
            /// @}

            /// M: To get radio capability earlier @{
            case EVENT_UNSOL_RADIO_CAPABILITY_CHANGED:
                ar = (AsyncResult) msg.obj;
                RadioCapability rc_unsol = (RadioCapability) ar.result;
                if (ar.exception != null) {
                    Rlog.d(LOG_TAG, "RIL_UNSOL_RADIO_CAPABILITY fail, don't change capability");
                } else {
                    radioCapabilityUpdated(rc_unsol);
                }
                Rlog.d(LOG_TAG, "EVENT_UNSOL_RADIO_CAPABILITY_CHANGED: " + "rc: " + rc_unsol);
                break;
            /// @}
            /// M: Set supplementary service property @{
            case EVENT_SET_SS_PROPERTY:
                if (mCallbackLatch != null) {
                    mCallbackLatch.countDown();
                }
                Rlog.d(LOG_TAG, "EVENT_SET_SS_PROPERTY done");
                break;
            /// @}
            case EVENT_SIM_RECORDS_LOADED:
                super.handleMessage(msg);
                updateVoiceMail();
            break;
            default:
                super.handleMessage(msg);
                break;
        }
    }

    public void setApcMode(int apcMode, boolean reportOn, int reportInterval) {
        if (isPhoneTypeGsm()) {
            mMtkCi.setApcMode(apcMode, reportOn, reportInterval, null);
        } else {
            Rlog.d(LOG_TAG, "setApcMode: not possible in CDMA");
        }
    }

    private class PseudoCellInfoResult {
        PseudoCellInfo infos = null;
        Object lockObj = new Object();
    }

    public PseudoCellInfo getApcInfo() {
        if (isPhoneTypeGsm()) {
            PseudoCellInfoResult result = new PseudoCellInfoResult();
            //wait query return and return
            synchronized(result.lockObj) {
                result.infos = null;
                mMtkCi.getApcInfo(obtainMessage(EVENT_GET_APC_INFO, result));
                try {
                    result.lockObj.wait(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            synchronized(result.lockObj) {
                if (result.infos != null) {
                    Rlog.d(LOG_TAG, "getApcInfo return: list.size = " + result.infos.toString());
                    return result.infos;
                } else {
                    Rlog.d(LOG_TAG, "getApcInfo return null");
                }
            }
        } else {
            Rlog.d(LOG_TAG, "getApcInfo: not possible in CDMA");
        }
        return null;
    }

    /// M: CC: Proprietary CRSS handling @{
    public void registerForCrssSuppServiceNotification(
            Handler h, int what, Object obj) {
        mCallRelatedSuppSvcRegistrants.addUnique(h, what, obj);
        if (mCachedCrssn != null) {
            mCallRelatedSuppSvcRegistrants.notifyRegistrants(mCachedCrssn);
            mCachedCrssn = null;
        }
    }

    public void unregisterForCrssSuppServiceNotification(Handler h) {
        mCallRelatedSuppSvcRegistrants.remove(h);
        mCachedCrssn = null;
    }
    /// @}

    @Override
    public void registerForSuppServiceNotification(
            Handler h, int what, Object obj) {
        mSsnRegistrants.addUnique(h, what, obj);
        /// M: CC: Proprietary CRSS handling @{
        // Do not enable or disable CSSN since it is already enabled in RIL initial callback.
        if (mCachedSsn != null) {
            mSsnRegistrants.notifyRegistrants(mCachedSsn);
            mCachedSsn = null;
        }
        /// @}
    }

    @Override
    public void unregisterForSuppServiceNotification(Handler h) {
        mSsnRegistrants.remove(h);
        /// M: CC: Proprietary CRSS handling @{
        // Do not enable or disable CSSN since it is already enabled in RIL initial callback.
        //if (mSsnRegistrants.size() == 0) mCi.setSuppServiceNotifications(false, null);
        mCachedSsn = null;
        /// @}
    }

    @Override
    public boolean handleInCallMmiCommands(String dialString) throws CallStateException {
        if (!isPhoneTypeGsm()) {
            loge("method handleInCallMmiCommands is NOT supported in CDMA!");
            return false;
        }

        Phone imsPhone = mImsPhone;
        if (imsPhone != null
                && imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE) {
            /// M: CC: Check GSM call state to avoid InCallMMI dispatching to IMS @{
            // [ALPS02516173],[ALPS02615800]
            //return imsPhone.handleInCallMmiCommands(dialString);
            if (!isInCSCall()) {
                return imsPhone.handleInCallMmiCommands(dialString);
            }
        }

        if (!isInCall()) {
            return false;
        }

        if (TextUtils.isEmpty(dialString)) {
            return false;
        }

        boolean result = false;
        char ch = dialString.charAt(0);
        switch (ch) {
            case '0':
                /// M: CC: Use 0+SEND MMI to release held calls or sets UDUB
                // (User Determined User Busy) for a waiting call. @{
                // 3GPP 22.030 6.5.5
                //result = handleCallDeflectionIncallSupplementaryService(dialString);
                result = handleUdubIncallSupplementaryService(dialString);
                ///@}
                break;
            case '1':
                result = handleCallWaitingIncallSupplementaryService(dialString);
                break;
            case '2':
                result = handleCallHoldIncallSupplementaryService(dialString);
                break;
            case '3':
                result = handleMultipartyIncallSupplementaryService(dialString);
                break;
            case '4':
                result = handleEctIncallSupplementaryService(dialString);
                break;
            case '5':
                result = handleCcbsIncallSupplementaryService(dialString);
                break;
            default:
                break;
        }

        return result;
    }

    /// M: CC: Use 0+SEND MMI to release held calls or sets UDUB
    // (User Determined User Busy) for a waiting call. @{
    // 3GPP 22.030 6.5.5
    private boolean handleUdubIncallSupplementaryService(
            String dialString) {
        if (dialString.length() > 1) {
            return false;
        }

        if (getRingingCall().getState() != GsmCdmaCall.State.IDLE ||
                getBackgroundCall().getState() != GsmCdmaCall.State.IDLE) {
            if (DBG) Rlog.d(LOG_TAG,
                    "MmiCode 0: hangupWaitingOrBackground");
            mCT.hangupWaitingOrBackground();
        }

        return true;
    }
    /// @}

    // PHB START
    public void queryPhbStorageInfo(int type, Message response) {
        /// M: CSIM PHB handling @{
        IccFileHandler fh;
        fh = getIccFileHandler();
        if (!CsimPhbUtil.hasModemPhbEnhanceCapability(fh)) {
            CsimPhbUtil.getPhbRecordInfo(response);
        } else {
        /// @}
            mMtkCi.queryPhbStorageInfo(type, response);
        }
    }
    // PHB END

    /**
     * Register for Network info changed.
     * Message.obj will contain an AsyncResult.
     * AsyncResult.result will be a String[ ] instance
     */
    public void registerForNetworkInfo(Handler h, int what, Object obj){
        mMtkCi.registerForNetworkInfo(h, what, obj);
    }

    /**
     * Unregisters for Network info changed notification.
     * Extraneous calls are tolerated silently
     */
    public void unregisterForNetworkInfo(Handler h){
        mMtkCi.unregisterForNetworkInfo(h);
    }

    /**
     * Set RF test Configuration to defautl phone
     * @param AntType The configuration
     *  0: signal information is not available on all Rx chains
     *  1: Rx diversity bitmask for chain 0(primary antenna)
     *  2: Rx diversity bitmask for chain 1(secondary antenna) is available
     *  3: Signal information on both Rx chains is available
     */
    public void setRxTestConfig (int AntType, Message result) {
        Rlog.d(LOG_TAG, "set Rx Test Config");
        mMtkCi.setRxTestConfig(AntType, result);
    }

    /**
     * Query RF Test Result
     */
    public void getRxTestResult (Message result) {
        Rlog.d(LOG_TAG, "get Rx Test Result");
        mMtkCi.getRxTestResult(result);
    }

    /** Get POL capability.
     *
     * @param onComplete a callback message when the action is completed.
     *
     * @internal
     */
    public void getPolCapability(Message onComplete){
        mMtkCi.getPOLCapability(onComplete);
    }

    /** Get Prefered operator list.
     *
     * @param onComplete a callback message when the action is completed.
     *
     * @internal
     */
    @ProductApi
    public void getPol(Message onComplete){
        mMtkCi.getCurrentPOLList(onComplete);
    }

    /** Set POL entry.
     *
     * @param networkWithAct network infor with act.
     * @param onComplete a callback message when the action is completed.
     *
     * @internal
     */
    @ProductApi
    public void setPolEntry(NetworkInfoWithAcT networkWithAct, Message onComplete){
        mMtkCi.setPOLEntry(networkWithAct.getPriority(), networkWithAct.getOperatorNumeric(),
                                    networkWithAct.getAccessTechnology(), onComplete);
    }

    @Override
    public List<? extends MmiCode> getPendingMmiCodes() {
        Rlog.d(LOG_TAG, "getPendingMmiCodes");
        dumpPendingMmi();

        ImsPhone imsPhone = (ImsPhone) mImsPhone;
        ArrayList<MmiCode> imsphonePendingMMIs = new ArrayList<MmiCode>();
        if (imsPhone != null
                && imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE) {
            List<ImsPhoneMmiCode> imsMMIs
                    = (List<ImsPhoneMmiCode>) imsPhone.getPendingMmiCodes();
            for(ImsPhoneMmiCode mmi : imsMMIs) {
                imsphonePendingMMIs.add((MmiCode) mmi);
            }
        }

        ArrayList<MmiCode> allPendingMMIs = new ArrayList<MmiCode>(mPendingMMIs);
        allPendingMMIs.addAll(imsphonePendingMMIs);
        Rlog.d(LOG_TAG, "allPendingMMIs.size() = " + allPendingMMIs.size());
        for (int i=0, s=allPendingMMIs.size(); i<s; i++) {
            Rlog.d(LOG_TAG, "dump allPendingMMIs: " + allPendingMMIs.get(i));
        }
        return allPendingMMIs;
    }

    @Override
    public void notifyCallForwardingIndicator() {
        TelephonyManager tm = TelephonyManager.from(mContext);
        int simState = tm.getSimState(mPhoneId);
        Rlog.d(LOG_TAG, "notifyCallForwardingIndicator: sim state = " + simState);

        /* For solving ALPS03242427
         * Also have to update the CFU icon when PIN LOCK phase
         */
        if (simState == TelephonyManager.SIM_STATE_READY) {
            mNotifier.notifyCallForwardingChanged(this);
        }
    }

    /* To fix ALPS03480449, we need to clean CFU icon directly without
     * checking SIM state when dispose occurs. Or the CFU value in TelephoyRegistry
     * may remains old, and it causes incorrectly CFU icon display.
     */
    public void notifyCallForwardingIndicatorWithoutCheckSimState() {
        Rlog.d(LOG_TAG, "notifyCallForwardingIndicatorWithoutCheckSimState");
        mNotifier.notifyCallForwardingChanged(this);
    }

    @Override
    public boolean handlePinMmi(String dialString) {
        MmiCode mmi;
        if (isPhoneTypeGsm()) {
            mmi = MtkGsmMmiCode.newFromDialString(dialString, this, mUiccApplication.get(), null);
        } else {
            mmi = CdmaMmiCode.newFromDialString(dialString, this, mUiccApplication.get());
        }

        if (mmi != null && mmi.isPinPukCommand()) {
            mPendingMMIs.add(mmi);
            Rlog.d(LOG_TAG, "handlePinMmi: " + dialString + ", mmi=" + mmi);
            dumpPendingMmi();
            mMmiRegistrants.notifyRegistrants(new AsyncResult(null, mmi, null));
            try {
                mmi.processCode();
            } catch (CallStateException e) {
                //do nothing
            }
            return true;
        }
        loge("Mmi is null or unrecognized!");
        return false;
    }

    @Override
    protected boolean isImsUtEnabledOverCdma() {
        if (isGsmSsPrefer()) {
            return true;
        }

        return isPhoneTypeCdmaLte()
            && mImsPhone != null
            && mImsPhone.isUtEnabled();
    }

    @Override
    public void getCallForwardingOption(int commandInterfaceCFReason, Message onComplete) {
        getCallForwardingOptionForServiceClass(commandInterfaceCFReason,
                CommandsInterface.SERVICE_CLASS_VOICE,
                onComplete);
    }

    public void getCallForwardingOptionForServiceClass(int commandInterfaceCFReason,
            int serviceClass,
            Message onComplete) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.getCallForwardingOptionForServiceClass(
                    commandInterfaceCFReason, serviceClass, onComplete, getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, getCallForwardingOptionForServiceClass");
            getCallForwardingOptionInternal(
                    commandInterfaceCFReason, serviceClass, onComplete);
        }
    }

    public void getCallForwardingOptionInternal(int commandInterfaceCFReason,
            int serviceClass,
            Message onComplete) {
        /// M: Support ut ss for ct card @{
        // if (isPhoneTypeGsm()) {
        if (isPhoneTypeGsm() || isImsUtEnabledOverCdma()) {
        /// @}
            Phone imsPhone = mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
            Rlog.d(LOG_TAG, "getCallForwardingOptionForServiceClass enter, " +
                    "CFReason:" + commandInterfaceCFReason + ", serviceClass:" + serviceClass);
                if ((imsPhone != null)
                        && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                        || imsPhone.isUtEnabled())) {

                    ((MtkImsPhone) imsPhone).getCallForwardingOptionForServiceClass(
                            commandInterfaceCFReason, serviceClass, onComplete);
                    return;
                }

                if (isValidCommandInterfaceCFReason(commandInterfaceCFReason)) {
                    if (DBG) {
                        logd("requesting call forwarding query.");
                    }
                    Message resp;
                    if (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL) {
                        resp = obtainMessage(EVENT_GET_CALL_FORWARD_DONE, onComplete);
                    } else {
                        resp = onComplete;
                    }
                    mCi.queryCallForwardStatus(commandInterfaceCFReason, serviceClass, null, resp);
                }

                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    &&(imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                        MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CF,onComplete);
                ss.mParcel.writeInt(commandInterfaceCFReason);
                ss.mParcel.writeInt(serviceClass);
                Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);
                ((MtkImsPhone) imsPhone).getCallForwardingOptionForServiceClass(
                        commandInterfaceCFReason, serviceClass, imsUtResult);
                return;
            }

            if (isValidCommandInterfaceCFReason(commandInterfaceCFReason)) {
                if (DBG) logd("requesting call forwarding query.");
                Message resp;
                if (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL) {
                    resp = obtainMessage(EVENT_GET_CALL_FORWARD_DONE, onComplete);
                } else {
                    resp = onComplete;
                }

                if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                        && isGsmUtSupport()) {
                    if (isInCSCall() && (getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA)) {
                        sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                        return;
                    }
                    mMtkSSReqDecisionMaker.queryCallForwardStatus(commandInterfaceCFReason,
                            serviceClass, null, resp);
                    return;
                }

                if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                    setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
                }

                if (isDuringVoLteCall() || isDuringImsEccCall()) {
                    if (onComplete != null) {
                        sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                        return;
                    }
                }

                // Not support from Ut to cs domain part
                if (isNotSupportUtToCS()) {
                    sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
                } else {
                    Rlog.d(LOG_TAG, "mCi.queryCallForwardStatus.");
                    mCi.queryCallForwardStatus(commandInterfaceCFReason, serviceClass, null, resp);
                }
            }
        } else {
            loge("getCallForwardingOptionForServiceClass: not possible in CDMA");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    @Override
    public void setCallForwardingOption(int commandInterfaceCFAction,
            int commandInterfaceCFReason,
            String dialingNumber,
            int timerSeconds,
            Message onComplete) {
                setCallForwardingOptionForServiceClass(commandInterfaceCFAction,
                commandInterfaceCFReason,
                dialingNumber,
                timerSeconds,
                CommandsInterface.SERVICE_CLASS_VOICE,
                onComplete);
    }

    public void setCallForwardingOptionForServiceClass(int commandInterfaceCFAction,
                                 int commandInterfaceCFReason,
                                 String dialingNumber,
                                 int timerSeconds,
                                 int serviceClass,
                                 Message onComplete) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.setCallForwardingOptionForServiceClass(
                    commandInterfaceCFAction, commandInterfaceCFReason, dialingNumber,
                    timerSeconds, serviceClass, onComplete, getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, setCallForwardingOptionForServiceClass");
            setCallForwardingOptionInternal(
                    commandInterfaceCFAction, commandInterfaceCFReason, dialingNumber,
                    timerSeconds, serviceClass, onComplete);
        }
    }

    public void setCallForwardingOptionInternal(int commandInterfaceCFAction,
                                 int commandInterfaceCFReason,
                                 String dialingNumber,
                                 int timerSeconds,
                                 int serviceClass,
                                 Message onComplete) {
        // M: Support ut ss for ct card @{
        // if (isPhoneTypeGsm()) {
        if (isPhoneTypeGsm() || isImsUtEnabledOverCdma()) {
        /// @}
            Phone imsPhone = mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "setCallForwardingOptionForServiceClass enter" +
                        ", CFAction:" + commandInterfaceCFAction +
                        ", CFReason:" + commandInterfaceCFReason +
                        ", dialingNumber:" + Rlog.pii(LOG_TAG, dialingNumber) +
                        ", timerSeconds:" + timerSeconds + ", serviceClass:" + serviceClass);
                if ((imsPhone != null)
                        && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                        || imsPhone.isUtEnabled())) {
                    ((MtkImsPhone) imsPhone).setCallForwardingOption(commandInterfaceCFAction,
                            commandInterfaceCFReason, dialingNumber, serviceClass,
                            timerSeconds, onComplete);
                    return;
                }

                if ((isValidCommandInterfaceCFAction(commandInterfaceCFAction)) &&
                        (isValidCommandInterfaceCFReason(commandInterfaceCFReason))) {

                    Message resp;
                    if (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL) {
                        Cfu cfu = new Cfu(dialingNumber, onComplete, serviceClass);
                        resp = obtainMessage(EVENT_SET_CALL_FORWARD_DONE,
                                isCfEnable(commandInterfaceCFAction) ? 1 : 0, 0, cfu);
                    } else {
                        resp = onComplete;
                    }
                    mCi.setCallForward(commandInterfaceCFAction,
                            commandInterfaceCFReason,
                            serviceClass,
                            dialingNumber,
                            timerSeconds,
                            resp);
                }
                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                        MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CF,onComplete);
                ss.mParcel.writeInt(commandInterfaceCFAction);
                ss.mParcel.writeInt(commandInterfaceCFReason);
                ss.mParcel.writeString(dialingNumber);
                ss.mParcel.writeInt(timerSeconds);
                ss.mParcel.writeInt(serviceClass);
                Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);
                ((ImsPhone)imsPhone).setCallForwardingOption(commandInterfaceCFAction,
                        commandInterfaceCFReason, dialingNumber, serviceClass, timerSeconds,
                        imsUtResult);
                return;
            }

            if ((isValidCommandInterfaceCFAction(commandInterfaceCFAction)) &&
                    (isValidCommandInterfaceCFReason(commandInterfaceCFReason))) {

                Message resp;
                if (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL) {
                    Cfu cfu = new Cfu(dialingNumber, onComplete, serviceClass);
                    resp = obtainMessage(EVENT_SET_CALL_FORWARD_DONE,
                                isCfEnable(commandInterfaceCFAction) ? 1 : 0, 0, cfu);
                } else {
                    resp = onComplete;
                }

                if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                        && isGsmUtSupport()) {
                    if (isInCSCall() && (getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA)) {
                        sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                        return;
                    }
                    mMtkSSReqDecisionMaker.setCallForward(commandInterfaceCFAction,
                            commandInterfaceCFReason, serviceClass,
                            dialingNumber, timerSeconds, resp);
                    return;
                }

                if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                    setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
                }

                /// M: Not support from Ut to cs domain part @{
                if (isNotSupportUtToCS()) {
                    sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
                    return;
                }
                /// @}

                if (isDuringVoLteCall() || isDuringImsEccCall()) {
                    if (onComplete != null) {
                        sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                        return;
                    }
                }

                mCi.setCallForward(commandInterfaceCFAction,
                        commandInterfaceCFReason,
                        serviceClass,
                        dialingNumber,
                        timerSeconds,
                        resp);
            }
        } else {
            loge("setCallForwardingOption: not possible in CDMA");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    /// M: SS OP01 Ut @{
    private static class CfuEx {
        final String mSetCfNumber;
        final long[] mSetTimeSlot;
        final Message mOnComplete;

        CfuEx(String cfNumber, long[] cfTimeSlot, Message onComplete) {
            mSetCfNumber = cfNumber;
            mSetTimeSlot = cfTimeSlot;
            mOnComplete = onComplete;
        }
    }

    public void saveTimeSlot(long[] timeSlot) {
        String timeSlotKey = CFU_TIME_SLOT + mPhoneId;
        String timeSlotString = "";
        if (timeSlot != null && timeSlot.length == 2) {
            timeSlotString = Long.toString(timeSlot[0]) + "," + Long.toString(timeSlot[1]);
        }
        SystemProperties.set(timeSlotKey, timeSlotString);
        Rlog.d(LOG_TAG, "timeSlotString = " + timeSlotString);
    }

    public long[] getTimeSlot() {
        String timeSlotKey = CFU_TIME_SLOT + mPhoneId;
        String timeSlotString = SystemProperties.get(timeSlotKey, "");
        long[] timeSlot = null;
        if (timeSlotString != null && !timeSlotString.equals("")) {
            String[] timeArray = timeSlotString.split(",");
            if (timeArray.length == 2) {
                timeSlot = new long[2];
                for (int i = 0; i < 2; i++) {
                    timeSlot[i] = Long.parseLong(timeArray[i]);
                    Calendar calenar = Calendar.getInstance(TimeZone.getDefault());
                    calenar.setTimeInMillis(timeSlot[i]);
                    int hour = calenar.get(Calendar.HOUR_OF_DAY);
                    int min = calenar.get(Calendar.MINUTE);
                    Calendar calenar2 = Calendar.getInstance(TimeZone.getDefault());
                    calenar2.set(Calendar.HOUR_OF_DAY, hour);
                    calenar2.set(Calendar.MINUTE, min);
                    timeSlot[i] = calenar2.getTimeInMillis();
                }
            }
        }
        Rlog.d(LOG_TAG, "timeSlot = " + Arrays.toString(timeSlot));
        return timeSlot;
    }

    public void getCallForwardInTimeSlot(int commandInterfaceCFReason, Message onComplete) {
        if (isPhoneTypeGsm()) {
            ImsPhone imsPhone = (ImsPhone) mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "getCallForwardInTimeSlot enter, CFReason:"
                        + commandInterfaceCFReason);
                if ((imsPhone != null)
                        && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                        || imsPhone.isUtEnabled())) {
                    ((MtkImsPhone)imsPhone).getCallForwardInTimeSlot(
                            commandInterfaceCFReason, onComplete);
                    return;
                }

                if (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL) {
                    if (DBG) {
                        Rlog.d(LOG_TAG, "requesting call forwarding in time slot query.");
                    }
                    Message resp;
                    resp = obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE, onComplete);
                    mMtkCi.queryCallForwardInTimeSlotStatus(commandInterfaceCFReason, 0, resp);
                }
                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                ((MtkImsPhone)imsPhone).getCallForwardInTimeSlot(
                        commandInterfaceCFReason, onComplete);
                return;
            }

            if (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL) {
                if (DBG) {
                    Rlog.d(LOG_TAG, "requesting call forwarding in time slot query.");
                }
                Message resp;
                resp = obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE, onComplete);

                if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                        && isGsmUtSupport()) {
                    mMtkSSReqDecisionMaker.queryCallForwardInTimeSlotStatus(
                            commandInterfaceCFReason,
                            CommandsInterface.SERVICE_CLASS_VOICE, resp);
                } else {
                    sendErrorResponse(onComplete,
                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                }
            } else if (onComplete != null) {
                sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            }
        } else {
            loge("method getCallForwardInTimeSlot is NOT supported in CDMA!");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    public void setCallForwardInTimeSlot(int commandInterfaceCFAction,
            int commandInterfaceCFReason,
            String dialingNumber,
            int timerSeconds,
            long[] timeSlot,
            Message onComplete) {
        if (isPhoneTypeGsm()) {
            ImsPhone imsPhone = (ImsPhone) mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "setCallForwardInTimeSlot enter, CFReason:" +
                        commandInterfaceCFReason + ", CFAction:" + commandInterfaceCFAction +
                        ", dialingNumber:" + dialingNumber +
                        ", timerSeconds:" + timerSeconds);
                if ((imsPhone != null)
                        && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                        || imsPhone.isUtEnabled())) {
                    ((MtkImsPhone)imsPhone).setCallForwardInTimeSlot(commandInterfaceCFAction,
                            commandInterfaceCFReason, dialingNumber, timerSeconds,
                            timeSlot, onComplete);
                    return;
                }

                if ((isValidCommandInterfaceCFAction(commandInterfaceCFAction)) &&
                        (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL)) {
                    Message resp;
                    CfuEx cfuEx = new CfuEx(dialingNumber, timeSlot, onComplete);
                    resp = obtainMessage(EVENT_SET_CALL_FORWARD_TIME_SLOT_DONE,
                            isCfEnable(commandInterfaceCFAction) ? 1 : 0, 0, cfuEx);

                    mMtkCi.setCallForwardInTimeSlot(commandInterfaceCFAction,
                            commandInterfaceCFReason,
                            CommandsInterface.SERVICE_CLASS_VOICE,
                            dialingNumber,
                            timerSeconds,
                            timeSlot,
                            resp);
                }
                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && isOp(OPID.OP01) && (imsPhone != null)
                    && (imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                        MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CF_IN_TIME_SLOT, onComplete);
                ss.mParcel.writeInt(commandInterfaceCFAction);
                ss.mParcel.writeInt(commandInterfaceCFReason);
                ss.mParcel.writeString(dialingNumber);
                ss.mParcel.writeInt(timerSeconds);
                Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);
                ((MtkImsPhone)imsPhone).setCallForwardInTimeSlot(
                        commandInterfaceCFAction, commandInterfaceCFReason,
                        dialingNumber, timerSeconds, timeSlot, imsUtResult);
                return;
            }

            if ((isValidCommandInterfaceCFAction(commandInterfaceCFAction)) &&
                    (commandInterfaceCFReason == CF_REASON_UNCONDITIONAL)) {
                Message resp;
                CfuEx cfuEx = new CfuEx(dialingNumber, timeSlot, onComplete);
                resp = obtainMessage(EVENT_SET_CALL_FORWARD_TIME_SLOT_DONE,
                        isCfEnable(commandInterfaceCFAction) ? 1 : 0, 0, cfuEx);

                if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                        && isGsmUtSupport()) {
                    mMtkSSReqDecisionMaker.setCallForwardInTimeSlot(commandInterfaceCFAction,
                            commandInterfaceCFReason, CommandsInterface.SERVICE_CLASS_VOICE,
                            dialingNumber, timerSeconds, timeSlot, resp);
                } else {
                    sendErrorResponse(onComplete,
                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                }
            } else {
                sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            }
        } else {
            loge("method setCallForwardInTimeSlot is NOT supported in CDMA!");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    private void handleCfuInTimeSlotQueryResult(MtkCallForwardInfo[] infos) {
        IccRecords r = mIccRecords.get();
        if (r != null) {
            if (infos == null || infos.length == 0) {
                // Assume the default is not active
                // Set unconditional CFF in SIM to false
                setVoiceCallForwardingFlag(1, false, null);
            } else {
                for (int i = 0, s = infos.length; i < s; i++) {
                    if ((infos[i].serviceClass & SERVICE_CLASS_VOICE) != 0) {
                        setVoiceCallForwardingFlag(1, (infos[i].status == 1),
                                infos[i].number);
                        saveTimeSlot(infos[i].timeSlot);
                        break;
                    }
                }
            }
        }
    }

    /**
     * Get Terminal-based CLIR.
     * @return Response array by 27.007.
     */
    public int[] getSavedClirSetting() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(getContext());
        int clirSetting = sp.getInt(CLIR_KEY + getPhoneId(), -1);
        int presentationMode;
        int getClirResult;
        if ((clirSetting == 0) || (clirSetting == -1)) {
            //allow CLI presentation
            presentationMode = 4;
            getClirResult = CommandsInterface.CLIR_DEFAULT;
        } else if (clirSetting == 1) {
            //restrict CLI presentation
            presentationMode = 3;
            getClirResult = CommandsInterface.CLIR_INVOCATION;
        } else {
            //allow CLI presentation
            presentationMode = 4;
            getClirResult = CommandsInterface.CLIR_SUPPRESSION;
        }

        int getClirResponse [] = new int[2];
        getClirResponse[0] = getClirResult;
        getClirResponse[1] = presentationMode;

        Rlog.i(LOG_TAG, "getClirResult: " + getClirResult);
        Rlog.i(LOG_TAG, "presentationMode: " + presentationMode);

        return getClirResponse;
    }

    @Override
    public void getOutgoingCallerIdDisplay(Message onComplete) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.getOutgoingCallerIdDisplay(onComplete, getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, getOutgoingCallerIdDisplay");
            getOutgoingCallerIdDisplayInternal(onComplete);
        }
    }

    public void getOutgoingCallerIdDisplayInternal(Message onComplete) {
        if (isPhoneTypeGsm() || isGsmSsPrefer()) {
            Phone imsPhone = mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "getOutgoingCallerIdDisplay enter");

                Message resp = obtainMessage(EVENT_GET_CLIR_COMPLETE, onComplete);
                if ((imsPhone != null) &&
                        (imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)) {
                    imsPhone.getOutgoingCallerIdDisplay(resp);
                    return;
                }
                mCi.getCLIR(resp);

                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                if (isOpNotSupportCallIdentity()) {
                    sendErrorResponse(onComplete,
                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                    return;
                }

                if (isOpTbClir()) {
                    if (onComplete != null) {
                        int[] result = getSavedClirSetting();
                        AsyncResult.forMessage(onComplete, result, null);
                        onComplete.sendToTarget();
                    }
                    return;
                }

                MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                        MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CLIR, onComplete);
                Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);
                imsPhone.getOutgoingCallerIdDisplay(imsUtResult);
                return;
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && isGsmUtSupport()) {
                if (isOpTbClir()) {
                    if (onComplete != null) {
                        int[] result = getSavedClirSetting();
                        AsyncResult.forMessage(onComplete, result, null);
                        onComplete.sendToTarget();
                    }
                    return;
                }

                mMtkSSReqDecisionMaker.getCLIR(onComplete);
                return;
            }

            if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            /// M: Not support from Ut to cs domain part @{
            if (isNotSupportUtToCS()) {
                sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
                return;
            }
            /// @}

            if (isDuringVoLteCall() || isDuringImsEccCall()) {
                if (onComplete != null) {
                    sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                    return;
                }
            }

            mCi.getCLIR(onComplete);
        } else {
            loge("getOutgoingCallerIdDisplay: not possible in CDMA");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    @Override
    public void setOutgoingCallerIdDisplay(int commandInterfaceCLIRMode, Message onComplete) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.setOutgoingCallerIdDisplay(commandInterfaceCLIRMode, onComplete,
                    getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, setOutgoingCallerIdDisplay");
            setOutgoingCallerIdDisplayInternal(commandInterfaceCLIRMode, onComplete);
        }
    }

    public void setOutgoingCallerIdDisplayInternal(int commandInterfaceCLIRMode,
            Message onComplete) {
        if (isPhoneTypeGsm() || isGsmSsPrefer()) {
            Phone imsPhone = mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "setOutgoingCallerIdDisplay enter, CLIRmode:" +
                        commandInterfaceCLIRMode);

                Message resp = obtainMessage(EVENT_SET_CLIR_COMPLETE,
                        commandInterfaceCLIRMode, 0, onComplete);
                if ((imsPhone != null) &&
                        (imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)) {
                    imsPhone.setOutgoingCallerIdDisplay(commandInterfaceCLIRMode, resp);
                    return;
                }
                // Packing CLIR value in the message. This will be required for
                // SharedPreference caching, if the message comes back as part of
                // a success response.
                mCi.setCLIR(commandInterfaceCLIRMode, resp);

                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {

                if (isOpNotSupportCallIdentity()) {
                    sendErrorResponse(onComplete,
                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                    return;
                }

                if (isOpTbClir()) {
                    if (isDuringVoLteCall()|| isDuringImsEccCall()) {
                        if (onComplete != null) {
                            sendErrorResponse(onComplete,
                                    CommandException.Error.GENERIC_FAILURE);
                            return;
                        }
                    }

                    mCi.setCLIR(commandInterfaceCLIRMode,
                        obtainMessage(EVENT_SET_CLIR_COMPLETE, commandInterfaceCLIRMode,
                        0, onComplete));
                    return;
                }

                MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                        MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CLIR,onComplete);
                ss.mParcel.writeInt(commandInterfaceCLIRMode);
                Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);
                imsPhone.setOutgoingCallerIdDisplay(commandInterfaceCLIRMode, imsUtResult);
                return;
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && isGsmUtSupport()) {
                if (isOpTbClir()) {

                    if (isDuringVoLteCall()|| isDuringImsEccCall()) {
                        if (onComplete != null) {
                            sendErrorResponse(onComplete,
                                    CommandException.Error.GENERIC_FAILURE);
                            return;
                        }
                    }

                    mCi.setCLIR(commandInterfaceCLIRMode,
                        obtainMessage(EVENT_SET_CLIR_COMPLETE, commandInterfaceCLIRMode,
                        0, onComplete));
                    return;
                }

                mMtkSSReqDecisionMaker.setCLIR(commandInterfaceCLIRMode,
                        obtainMessage(EVENT_SET_CLIR_COMPLETE,
                                commandInterfaceCLIRMode, 0, onComplete));
                return;
            }

            if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            /// M: Not support from Ut to cs domain part @{
            if (isNotSupportUtToCS()) {
                sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
                return;
            }
            /// @}

            if (isDuringVoLteCall() || isDuringImsEccCall()) {
                if (onComplete != null) {
                    sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                    return;
                }
            }

            // Packing CLIR value in the message. This will be required for
            // SharedPreference caching, if the message comes back as part of
            // a success response.
            mCi.setCLIR(commandInterfaceCLIRMode,
                    obtainMessage(EVENT_SET_CLIR_COMPLETE,
                            commandInterfaceCLIRMode, 0, onComplete));
        } else {
            loge("setOutgoingCallerIdDisplay: not possible in CDMA");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    private void initTbcwMode() {
        if (mTbcwMode != TBCW_UNKNOWN) {
            Rlog.d(LOG_TAG, "initTbcwMode, mTbcwMode is not UNKNOWN, no need to init");
            return;
        }

        /* If the IccCard is not ready, just return. It will retry if the mTbcwMode == UNKNOWN */
        IccCard iccCard = getIccCard();
        String simType = iccCard.getIccCardType();
        if (iccCard == null || !iccCard.hasIccCard() || simType == null || simType.equals("")) {
            Rlog.d(LOG_TAG, "initTbcwMode, IccCard is not ready. mTbcwMode ramains UNKNOWN");
            return;
        }

        mExecutorService.submit(new Runnable() {
            public void run() {
                if (isOpTbcwWithCS()) {
                    setTbcwMode(TBCW_WITH_CS);
                    setTbcwToEnabledOnIfDisabled();
                } else if (!isUsimCard())  {
                    setTbcwMode(TBCW_NOT_VOLTE_USER);
                    setSSPropertyThroughHidl(getPhoneId(), PROPERTY_TBCW_MODE, TBCW_DISABLED);
                }
                Rlog.d(LOG_TAG, "initTbcwMode: " + mTbcwMode);
            }
        });
    }

    public int getTbcwMode() {
        initTbcwMode();
        return mTbcwMode;
    }

    public void setTbcwMode(int newMode) {
        Rlog.d(LOG_TAG, "Set tbcwmode: " + newMode + ", phoneId: " + getPhoneId());
        mTbcwMode = newMode;
    }

    /* Set the system property PROPERTY_TBCW_MODE
     * to TBCW_ON if it is TBCW_DISABLED.
     */
    public void setTbcwToEnabledOnIfDisabled() {
        String tbcwMode = TelephonyManager.getTelephonyProperty(getPhoneId(),
                PROPERTY_TBCW_MODE,
                TBCW_DISABLED);
        Rlog.d(LOG_TAG, "setTbcwToEnabledOnIfDisabled tbcwmode: "
            + tbcwMode + ", status: " + tbcwMode.equals(""));
        if (TBCW_DISABLED.equals(tbcwMode) || tbcwMode.equals("")) {
            setSSPropertyThroughHidl(getPhoneId(), PROPERTY_TBCW_MODE, TBCW_ON);
        }
    }

    /* Return Terminal-based Call Waiting configuration. */
    public void getTerminalBasedCallWaiting(Message onComplete) {
        Future future = mExecutorService.submit(new Callable() {
            public Object call() throws Exception {
                String tbcwMode = TelephonyManager.getTelephonyProperty(getPhoneId(),
                PROPERTY_TBCW_MODE,
                TBCW_DISABLED);
                if (DBG) {
                    Rlog.d(LOG_TAG, "getTerminalBasedCallWaiting(): tbcwMode = " + tbcwMode
                            + ", onComplete = " + onComplete);
                }
                if (TBCW_ON.equals(tbcwMode)) {
                    if (onComplete != null) {
                        Thread.sleep(1000); // For UI display
                        int[] cwInfos = new int[2];
                        cwInfos[0] = 1;
                        cwInfos[1] = SERVICE_CLASS_VOICE;
                        AsyncResult.forMessage(onComplete, cwInfos, null);
                        onComplete.sendToTarget();
                    }
                    return true;
                } else if (TBCW_OFF.equals(tbcwMode)) {
                    if (onComplete != null) {
                        Thread.sleep(1000); // For UI display
                        int[] cwInfos = new int[2];
                        cwInfos[0] = 0;
                        AsyncResult.forMessage(onComplete, cwInfos, null);
                        onComplete.sendToTarget();
                    }
                    return true;
                }
                return false;
            }
        });

        boolean result = false;
        try {
            result = (Boolean) future.get();
            Rlog.d(LOG_TAG, "getTerminalBasedCallWaiting future get = " + result);
        } catch (Exception e) {
            Rlog.e(LOG_TAG, "getTerminalBasedCallWaiting Exception occured");
        }
    }

    @Override
    public void getCallWaiting(Message onComplete) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.getCallWaiting(onComplete, getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, getCallWaiting");
            getCallWaitingInternal(onComplete);
        }
    }

    public void getCallWaitingInternal(Message onComplete) {
        /// M: Support ut ss for ct card @{
        // if (isPhoneTypeGsm()) {
        if (isPhoneTypeGsm() || isImsUtEnabledOverCdma()) {
        /// @}
            Phone imsPhone = mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "getCallWaiting enter");

                if ((imsPhone != null)
                        && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                        || imsPhone.isUtEnabled())) {
                    imsPhone.getCallWaiting(onComplete);
                    return;
                }

                //As per 3GPP TS 24.083, section 1.6 UE doesn't need to send service
                //class parameter in call waiting interrogation  to network
                mCi.queryCallWaiting(CommandsInterface.SERVICE_CLASS_NONE, onComplete);

                return;
            }
            /// @}

            /* isOpNwCW: Call Waiting is configured by Ut interface */
            if (!isOpNwCW()) {
                if (mTbcwMode == TBCW_UNKNOWN) {
                    initTbcwMode();
                }
                if (DBG) {
                    Rlog.d(LOG_TAG, "getCallWaiting(): mTbcwMode = " + mTbcwMode
                            + ", onComplete = " + onComplete);
                }

                switch (mTbcwMode) {
                    case TBCW_VOLTE_USER:
                        getTerminalBasedCallWaiting(onComplete);
                        return;

                    case TBCW_NOT_VOLTE_USER:
                        if (isDuringVoLteCall()|| isDuringImsEccCall()) {
                            if (onComplete != null) {
                                sendErrorResponse(onComplete,
                                        CommandException.Error.GENERIC_FAILURE);
                                return;
                            }
                        }
                        /* Not support from Ut to cs domain part */
                        if (isNotSupportUtToCS()) {
                            sendErrorResponse(onComplete,
                                    CommandException.Error.OPERATION_NOT_ALLOWED);
                        } else {
                            Rlog.d(LOG_TAG, "mCi.queryCallForwardStatus.");
                            mCi.queryCallWaiting(CommandsInterface.SERVICE_CLASS_NONE, onComplete);
                        }
                        return;

                    case TBCW_WITH_CS:
                        if (isDuringVoLteCall()|| isDuringImsEccCall()) {
                            if (onComplete != null) {
                                sendErrorResponse(onComplete,
                                        CommandException.Error.GENERIC_FAILURE);
                                return;
                            }
                        }
                        Message resp = obtainMessage(EVENT_GET_CALL_WAITING_DONE, onComplete);
                        mCi.queryCallWaiting(CommandsInterface.SERVICE_CLASS_NONE, resp);
                        return;
                }
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                if (isOpNwCW()){
                    // TODO: For IMS SS
                    Rlog.d(LOG_TAG, "isOpNwCW(), getCallWaiting() by Ut interface");
                    MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                            MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CW,onComplete);
                    Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);
                    imsPhone.getCallWaiting(imsUtResult);
                } else {
                    Rlog.d(LOG_TAG, "isOpTbCW(), getTerminalBasedCallWaiting");
                    setTbcwMode(TBCW_VOLTE_USER);
                    setTbcwToEnabledOnIfDisabled();
                    getTerminalBasedCallWaiting(onComplete);
                }
                return;
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && isGsmUtSupport()) {
                Rlog.d(LOG_TAG, "mMtkSSReqDecisionMaker.queryCallWaiting");
                mMtkSSReqDecisionMaker.queryCallWaiting(CommandsInterface.SERVICE_CLASS_NONE,
                    onComplete);
                return;
            }

            if (isDuringVoLteCall()|| isDuringImsEccCall()) {
                if (onComplete != null) {
                    sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                    return;
                }
            }

            //As per 3GPP TS 24.083, section 1.6 UE doesn't need to send service
            //class parameter in call waiting interrogation  to network
            /* Not support from Ut to cs domain part */
            if (isNotSupportUtToCS()) {
                sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
            } else {
                Rlog.d(LOG_TAG, "mCi.queryCallForwardStatus.");
                mCi.queryCallWaiting(CommandsInterface.SERVICE_CLASS_NONE, onComplete);
            }
        } else {
            // TODO: getCallWaiting for CDMA part
            mCi.queryCallWaiting(CommandsInterface.SERVICE_CLASS_VOICE, onComplete);
        }
    }

    /**
     * Set Terminal-based Call Waiting configuration.
     * @param enable true if activate Call Waiting. false if deactivate Call Waiting.
     * @param onComplete Message callback
     */
    public void setTerminalBasedCallWaiting(boolean enable, Message onComplete) {
        mExecutorService.submit(new Runnable() {
            public void run() {
                String tbcwMode = TelephonyManager.getTelephonyProperty(getPhoneId(),
                                                                        PROPERTY_TBCW_MODE,
                                                                        TBCW_DISABLED);
                if (DBG) {
                    Rlog.d(LOG_TAG, "setTerminalBasedCallWaiting(): tbcwMode = " + tbcwMode
                            + ", enable = " + enable);
                }
                if (TBCW_ON.equals(tbcwMode)) {
                    if (!enable) {
                        setSSPropertyThroughHidl(getPhoneId(), PROPERTY_TBCW_MODE, TBCW_OFF);
                    }
                    if (onComplete != null) {
                        try {
                            Thread.sleep(1000); // For UI display
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        AsyncResult.forMessage(onComplete, null, null);
                        onComplete.sendToTarget();
                    }
                    return;
                } else if (TBCW_OFF.equals(tbcwMode)) {
                    if (enable) {
                        setSSPropertyThroughHidl(getPhoneId(), PROPERTY_TBCW_MODE, TBCW_ON);
                    }
                    if (onComplete != null) {
                        try {
                            Thread.sleep(1000); // For UI display
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        AsyncResult.forMessage(onComplete, null, null);
                        onComplete.sendToTarget();
                    }
                    return;
                }

                Rlog.e(LOG_TAG, "setTerminalBasedCallWaiting(): ERROR: tbcwMode = " + tbcwMode);
                return;
            }
        });
    }

    /**
     * Sets a per-phone supplementary service property through HIDL with the value specified.
     */
    public void setSSPropertyThroughHidl(int phoneId, String property, String value) {
        Rlog.d(LOG_TAG, "setSSPropertyThroughHidl" +
                ", phoneId = " + phoneId + ", name = " + property + ", value = " + value);
        String propVal = "";
        String p[] = null;
        String prop = SystemProperties.get(property);

        if (value == null) {
            value = "";
        }

        if (prop != null) {
            p = prop.split(",");
        }

        if (!SubscriptionManager.isValidPhoneId(phoneId)) {
            Rlog.d(LOG_TAG, "setSSPropertyThroughHidl: invalid phoneId=" + phoneId +
                    " property=" + property + " value: " + value + " prop=" + prop);
            return;
        }

        for (int i = 0; i < phoneId; i++) {
            String str = "";
            if ((p != null) && (i < p.length)) {
                str = p[i];
            }
            propVal = propVal + str + ",";
        }

        propVal = propVal + value;
        if (p != null) {
            for (int i = phoneId + 1; i < p.length; i++) {
                propVal = propVal + "," + p[i];
            }
        }

        if (propVal.length() > SystemProperties.PROP_VALUE_MAX) {
            Rlog.d(LOG_TAG, "setSSPropertyThroughHidl: property too long phoneId=" + phoneId +
                    " property=" + property + " value: " + value + " propVal=" + propVal);
            return;
        }

        setSuppServProperty(property, propVal);
    }

    @Override
    public void setCallWaiting(boolean enable, Message onComplete) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.setCallWaiting(enable, onComplete, getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, setCallWaiting");
            setCallWaitingInternal(enable, onComplete);
        }
    }

    public void setCallWaitingInternal(boolean enable, Message onComplete) {
        /// M: Support ut ss for ct card @{
        // if (isPhoneTypeGsm()) {
        if (isPhoneTypeGsm() || isImsUtEnabledOverCdma()) {
        /// @}
            Phone imsPhone = mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "setCallWaiting enter, enable:" + enable);

                if ((imsPhone != null)
                        && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                        || imsPhone.isUtEnabled())) {
                    imsPhone.setCallWaiting(enable, onComplete);
                    return;
                }

                int serviceClass = CommandsInterface.SERVICE_CLASS_VOICE;
                CarrierConfigManager configManager = (CarrierConfigManager)
                    getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
                PersistableBundle b = configManager.getConfigForSubId(getSubId());
                if (b != null) {
                    serviceClass = b.getInt(CarrierConfigManager.KEY_CALL_WAITING_SERVICE_CLASS_INT,
                            CommandsInterface.SERVICE_CLASS_VOICE);
                }
                Rlog.d(LOG_TAG, "setCallWaiting serviceClass = " + serviceClass);
                mCi.setCallWaiting(enable, serviceClass, onComplete);
                return;
            }
            /// @}

            /* isOpNwCW: Call Waiting is configured by Ut interface */
            if (!isOpNwCW()) {
                if (mTbcwMode == TBCW_UNKNOWN) {
                    initTbcwMode();
                }
                if (DBG) {
                    Rlog.d(LOG_TAG, "setCallWaiting(): mTbcwMode = " + mTbcwMode
                            + ", onComplete = " + onComplete);
                }

                switch (mTbcwMode) {
                    case TBCW_VOLTE_USER:
                        setTerminalBasedCallWaiting(enable, onComplete);
                        return;

                    case TBCW_NOT_VOLTE_USER:
                        if (isDuringVoLteCall()|| isDuringImsEccCall()) {
                            if (onComplete != null) {
                                sendErrorResponse(onComplete,
                                        CommandException.Error.GENERIC_FAILURE);
                                return;
                            }
                        }

                        /// M: Not support from Ut to cs domain part @{
                        if (isNotSupportUtToCS()) {
                            sendErrorResponse(onComplete,
                                    CommandException.Error.OPERATION_NOT_ALLOWED);
                        } else {
                            mCi.setCallWaiting(
                                    enable, CommandsInterface.SERVICE_CLASS_VOICE, onComplete);
                        }
                        /// @}
                        //mCi.setCallWaiting(enable,CommandsInterface.SERVICE_CLASS_VOICE,
                        //        onComplete);
                        return;

                    case TBCW_WITH_CS:
                        if (isDuringVoLteCall()|| isDuringImsEccCall()) {
                            if (onComplete != null) {
                                sendErrorResponse(onComplete,
                                        CommandException.Error.GENERIC_FAILURE);
                                return;
                            }
                        }
                        Message resp = obtainMessage(EVENT_SET_CALL_WAITING_DONE,
                                enable == true ? 1 : 0, 0, onComplete);
                        mCi.setCallWaiting(enable, CommandsInterface.SERVICE_CLASS_VOICE, resp);
                        return;
                }
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                if (isOpNwCW()) {
                    MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                            MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CW,onComplete);
                    int enableState = enable ? 1 : 0;
                    ss.mParcel.writeInt(enableState);
                    Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);
                    imsPhone.setCallWaiting(enable, imsUtResult);
                } else {
                    Rlog.d(LOG_TAG, "isOpTbCW(), setTerminalBasedCallWaiting(): IMS in service");
                    setTbcwMode(TBCW_VOLTE_USER);
                    setTbcwToEnabledOnIfDisabled();
                    setTerminalBasedCallWaiting(enable, onComplete);
                }
                return;
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && isGsmUtSupport()) {
                Rlog.d(LOG_TAG, "mMtkSSReqDecisionMaker.setCallWaiting");
                mMtkSSReqDecisionMaker.setCallWaiting(enable,
                        CommandsInterface.SERVICE_CLASS_VOICE, onComplete);
                return;
            }

            if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            if (isDuringVoLteCall() || isDuringImsEccCall()) {
                if (onComplete != null) {
                    sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                    return;
                }
            }

            /// M: Not support from Ut to cs domain part @{
            if (isNotSupportUtToCS()) {
                sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
            } else {
                mCi.setCallWaiting(enable, CommandsInterface.SERVICE_CLASS_VOICE, onComplete);
            }
            /// @}
            //  mCi.setCallWaiting(enable, CommandsInterface.SERVICE_CLASS_VOICE, onComplete);
        } else {
            loge("method setCallWaiting is NOT supported in CDMA!");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    @Override
    public void getCallBarring(String facility, String password, Message onComplete,
            int serviceClass) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.getCallBarring(facility, password, serviceClass,
                    onComplete, getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, getCallBarringInternal");
            getCallBarringInternal(facility, password, onComplete, serviceClass);
        }
    }

    public void getCallBarring(String facility, String password, Message onComplete) {
        getCallBarring(facility, password, onComplete, CommandsInterface.SERVICE_CLASS_VOICE);
    }

    public void getCallBarringInternal(String facility, String password, Message onComplete,
            int serviceClass) {
        if (isPhoneTypeGsm() || isGsmSsPrefer()) {
            ImsPhone imsPhone = (ImsPhone) mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "getCallBarringInternal enter, facility:" + facility +
                        ", serviceClass:" + serviceClass + ", password:" + password);

                Message resp = obtainMessage(EVENT_GET_CALL_BARRING_COMPLETE, onComplete);
                if ((imsPhone != null) && ((imsPhone.getServiceState().getState() ==
                        ServiceState.STATE_IN_SERVICE) || imsPhone.isUtEnabled())) {
                    imsPhone.getCallBarring(facility, password, resp, serviceClass);
                    return;
                }

                mCi.queryFacilityLock(facility, password, serviceClass, resp);
                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                if (isOpNotSupportOCB(facility)) {
                    sendErrorResponse(onComplete,
                            CommandException.Error.REQUEST_NOT_SUPPORTED);
                    return;
                }

                MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                        MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CB,onComplete);
                ss.mParcel.writeString(facility);
                ss.mParcel.writeString(password);
                ss.mParcel.writeInt(serviceClass);
                Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);

                imsPhone.getCallBarring(facility, password, imsUtResult, serviceClass);
                return;
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.queryFacilityLock(facility, password,
                        serviceClass, onComplete);
                return;
            }

            if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            /// M: Not support from Ut to cs domain part @{
            if (isNotSupportUtToCS()) {
                sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
                return;
            }
            /// @}

            if (isDuringVoLteCall() || isDuringImsEccCall()) {
                if (onComplete != null) {
                    sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                    return;
                }
            }

            CommandException checkError = checkUiccApplicationForCB();
            if (checkError != null && onComplete != null) {
                sendErrorResponse(onComplete, checkError.getCommandError());
                return;
            }

            mCi.queryFacilityLockForApp(facility, password, serviceClass,
                    mUiccApplication.get().getAid(), onComplete);
        } else {
            loge("method getFacilityLock is NOT supported in CDMA!");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    @Override
    public void setCallBarring(String facility, boolean lockState,
            String password, Message onComplete, int serviceClass) {
        MtkSuppServQueueHelper ssQueueHelper = MtkSuppServManager.getSuppServQueueHelper();
        if (ssQueueHelper != null) {
            ssQueueHelper.setCallBarring(facility, lockState, password,
                    serviceClass, onComplete, getPhoneId());
        } else {
            Rlog.d(LOG_TAG, "ssQueueHelper not exist, setCallBarring");
            setCallBarringInternal(facility, lockState, password, onComplete, serviceClass);
        }
    }

    public void setCallBarring(String facility, boolean lockState,
            String password, Message onComplete) {
        setCallBarring(facility, lockState, password, onComplete,
                CommandsInterface.SERVICE_CLASS_VOICE);
    }

    public void setCallBarringInternal(String facility, boolean lockState,
            String password, Message onComplete, int serviceClass) {
        if (isPhoneTypeGsm() || isGsmSsPrefer()) {
            ImsPhone imsPhone = (ImsPhone) mImsPhone;

            /// M: 93 Ims SS native solution @{
            if (supportMdAutoSetupIms()) {
                Rlog.d(LOG_TAG, "setCallBarring enter, facility:" + facility +
                        ", serviceClass:" + serviceClass + ", password:" + password +
                        ", lockState:" + lockState);

                Message resp = obtainMessage(EVENT_SET_CALL_BARRING_COMPLETE, onComplete);
                if ((imsPhone != null) && ((imsPhone.getServiceState().getState() ==
                        ServiceState.STATE_IN_SERVICE) || imsPhone.isUtEnabled())) {
                    imsPhone.setCallBarring(facility, lockState, password, resp, serviceClass);
                    return;
                }

                mCi.setFacilityLock(facility, lockState, password, serviceClass, resp);
                return;
            }
            /// @}

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && (imsPhone != null)
                    && ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())
                    && (imsPhone.isVolteEnabled()
                    || (imsPhone.isWifiCallingEnabled() && isWFCUtSupport()))) {
                if (isOpNotSupportOCB(facility)) {
                    sendErrorResponse(onComplete, CommandException.Error.REQUEST_NOT_SUPPORTED);
                    return;
                }

                MtkSuppSrvRequest ss = MtkSuppSrvRequest.obtain(
                        MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CB,onComplete);
                ss.mParcel.writeString(facility);
                int enableState = lockState ? 1 : 0;
                ss.mParcel.writeInt(enableState);
                ss.mParcel.writeString(password);
                ss.mParcel.writeInt(serviceClass);
                Message imsUtResult = obtainMessage(EVENT_IMS_UT_DONE, ss);

                imsPhone.setCallBarring(facility, lockState, password, imsUtResult, serviceClass);
                return;
            }

            if ((getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                    && isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.setFacilityLock(facility, lockState, password,
                        serviceClass, onComplete);
                return;
            }

            if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            /// M: Not support from Ut to cs domain part @{
            if (isNotSupportUtToCS()) {
                sendErrorResponse(onComplete, CommandException.Error.OPERATION_NOT_ALLOWED);
                return;
            }
            /// @}
            if (isDuringVoLteCall() || isDuringImsEccCall()) {
                if (onComplete != null) {
                    sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
                    return;
                }
            }

            CommandException checkError = checkUiccApplicationForCB();
            if (checkError != null && onComplete != null) {
                sendErrorResponse(onComplete, checkError.getCommandError());
                return;
            }

            mCi.setFacilityLockForApp(facility, lockState, password, serviceClass,
                    mUiccApplication.get().getAid(), onComplete);
        } else {
            loge("method setFacilityLock is NOT supported in CDMA!");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    // MTK-START
    /* If mUiccApplication is not ready, for example, quickly switch flight mode,
     * during sim switch. We should not execute call barring.
     */
    public CommandException checkUiccApplicationForCB() {
        if (mUiccApplication.get() == null) {
            Rlog.d(LOG_TAG, "checkUiccApplicationForCB: mUiccApplication.get() == null");
            /* Check if the radioState is available */
            if (!isRadioAvailable() || !isRadioOn()) {
                Rlog.d(LOG_TAG, "checkUiccApplicationForCB: radio not available");
                return new CommandException(CommandException.Error.RADIO_NOT_AVAILABLE);
            } else {
                return new CommandException(CommandException.Error.GENERIC_FAILURE);
            }
        }
        return null;
    }
    // MTK-END

    @Override
    public void changeCallBarringPassword(String facility, String oldPwd, String newPwd,
            Message onComplete) {
        if (isPhoneTypeGsm()) {
            if (isDuringImsCall()) {
                /* Prevent CS domain SS request during IMS call */
                if (onComplete != null) {
                    CommandException ce = new CommandException(
                            CommandException.Error.GENERIC_FAILURE);
                    AsyncResult.forMessage(onComplete, null, ce);
                    onComplete.sendToTarget();
                }
            } else {
                mCi.changeBarringPassword(facility, oldPwd, newPwd, onComplete);
            }
        } else {
            loge("method changeBarringPassword is NOT supported in CDMA!");
            sendErrorResponse(onComplete, CommandException.Error.GENERIC_FAILURE);
            return;
        }
    }

    /* MTK */
    public MtkSSRequestDecisionMaker getMtkSSRequestDecisionMaker() {
        return mMtkSSReqDecisionMaker;
    }

    public boolean isDuringImsCall() {
        if (mImsPhone != null) {
            Call.State foregroundCallState = mImsPhone.getForegroundCall().getState();
            Call.State backgroundCallState = mImsPhone.getBackgroundCall().getState();
            Call.State ringingCallState = mImsPhone.getRingingCall().getState();
            boolean isDuringImsCall = (foregroundCallState.isAlive() ||
                    backgroundCallState.isAlive() || ringingCallState.isAlive());
            if (isDuringImsCall) {
                Rlog.d(LOG_TAG, "During IMS call.");
                return true;
            }
        }
        return false;
    }

    public boolean isDuringVoLteCall() {
        boolean isOnLtePdn = (mImsPhone != null && mImsPhone.isVolteEnabled());
        boolean r = isDuringImsCall() && isOnLtePdn;
        Rlog.d(LOG_TAG, "isDuringVoLteCall: " + r);
        return r;
    }

    public boolean isDuringImsEccCall() {
        boolean isInImsEccCall = (mImsPhone != null && mImsPhone.isInEmergencyCall());
        Rlog.d(LOG_TAG, "isInImsEccCall: " + isInImsEccCall);
        return isInImsEccCall;
    }

    private void handleImsUtDone(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        Message onComplete;

        if (ar == null) {
            Rlog.e(LOG_TAG, "EVENT_IMS_UT_DONE: Error AsyncResult null!");
            return;
        }

        MtkSuppSrvRequest ss = (MtkSuppSrvRequest) ar.userObj;

        if (ss == null) {
            Rlog.e(LOG_TAG, "EVENT_IMS_UT_DONE: Error SuppSrvRequest null!");
        } else {
            CommandException cmdException = null;
            if ((ar.exception != null) && (ar.exception instanceof CommandException)) {
                cmdException = (CommandException) ar.exception;
            }

            if ((cmdException != null) && (cmdException.getCommandError()
                    == CommandException.Error.OPERATION_NOT_ALLOWED)) {
                setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_UNTIL_NEXT_BOOT);
                if (isNotSupportUtToCS()) {
                    Rlog.d(LOG_TAG, "UT_XCAP_403_FORBIDDEN.");
                    ar.exception = new CommandException(
                            CommandException.Error.OPERATION_NOT_ALLOWED);
                    onComplete = ss.getResultCallback();
                    if (onComplete != null) {
                        AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                        onComplete.sendToTarget();
                    }
                    ss.mParcel.recycle();
                } else {
                    Rlog.d(LOG_TAG, "Csfallback next_reboot.");
                    Message msgCSFB = obtainMessage(EVENT_IMS_UT_CSFB, ss);
                    sendMessage(msgCSFB);
                }
            } else if ((cmdException != null) && (cmdException.getCommandError()
                    == CommandException.Error.OEM_ERROR_3)) {
                if (isNotSupportUtToCS()) {
                    Rlog.d(LOG_TAG, "CommandException.Error.UT_UNKNOWN_HOST.");
                    ar.exception = new CommandException(
                            CommandException.Error.OPERATION_NOT_ALLOWED);
                    onComplete = ss.getResultCallback();
                    if (onComplete != null) {
                        AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                        onComplete.sendToTarget();
                    }
                    ss.mParcel.recycle();
                } else {
                    Rlog.d(LOG_TAG, "Csfallback once.");
                    setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_ONCE);
                    Message msgCSFB = obtainMessage(EVENT_IMS_UT_CSFB, ss);
                    sendMessage(msgCSFB);
                }
            } else {
                if ((cmdException != null) && (cmdException.getCommandError()
                        == CommandException.Error.NO_SUCH_ELEMENT)) {
                    // Only consider CB && op05 and response 404 status.
                    // Get it from ImsPhone.java
                    // if not CB && op05, then transfer to GENERIC_FAILURE
                    if (isOpTransferXcap404()
                        && (ss.getRequestCode() == MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CB
                        || ss.getRequestCode() == MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CB)) {
                        Rlog.d(LOG_TAG, "GSMPhone get UT_XCAP_404_NOT_FOUND.");
                    } else {
                        ar.exception = new CommandException(
                                CommandException.Error.GENERIC_FAILURE);
                    }
                } else if ((cmdException != null) && (cmdException.getCommandError()
                        == CommandException.Error.OEM_ERROR_25)) {
                    //CommandException.Error.OEM_ERROR_25 is used for 409 conflict.
                    if (!isEnableXcapHttpResponse409()) {
                        // Transfer back to gereric failure.
                        Rlog.d(LOG_TAG, "GSMPhone get UT_XCAP_409_CONFLICT, " +
                                "return GENERIC_FAILURE");
                        ar.exception = new CommandException(
                                CommandException.Error.GENERIC_FAILURE);
                    } else {
                        Rlog.d(LOG_TAG, "GSMPhone get UT_XCAP_409_CONFLICT.");
                    }
                }

                onComplete = ss.getResultCallback();
                if (onComplete != null) {
                    AsyncResult.forMessage(onComplete, ar.result, ar.exception);
                    onComplete.sendToTarget();
                }
                ss.mParcel.recycle();
            }
        }
    }

    private void handleImsUtCsfb(Message msg) {
        MtkSuppSrvRequest ss = (MtkSuppSrvRequest) msg.obj;
        if (ss == null) {
            Rlog.e(LOG_TAG, "handleImsUtCsfb: Error MtkSuppSrvRequest null!");
            return;
        }

        if (isDuringVoLteCall()|| isDuringImsEccCall()) {
            // Prevent CS domain SS request during IMS Ecc call or VOLTE call
            Message resultCallback = ss.getResultCallback();
            if (resultCallback != null) {
                CommandException ce = new CommandException(
                        CommandException.Error.GENERIC_FAILURE);
                AsyncResult.forMessage(resultCallback, null, ce);
                resultCallback.sendToTarget();
            }

            if (getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            ss.setResultCallback(null);
            ss.mParcel.recycle();
            return;
        }

        final int requestCode = ss.getRequestCode();
        ss.mParcel.setDataPosition(0);
        switch(requestCode) {
            case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CF:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_GET_CF");
                int commandInterfaceCFReason = ss.mParcel.readInt();
                int serviceClass = ss.mParcel.readInt();
                getCallForwardingOptionInternal(commandInterfaceCFReason,
                        serviceClass, ss.getResultCallback());
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CF:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_SET_CF");
                int commandInterfaceCFAction = ss.mParcel.readInt();
                int commandInterfaceCFReason = ss.mParcel.readInt();
                String dialingNumber = ss.mParcel.readString();
                int timerSeconds = ss.mParcel.readInt();
                int serviceClass = ss.mParcel.readInt();
                setCallForwardingOptionInternal(commandInterfaceCFAction,
                        commandInterfaceCFReason, dialingNumber, timerSeconds,
                        serviceClass, ss.getResultCallback());
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CLIR:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_GET_CLIR");
                getOutgoingCallerIdDisplayInternal(ss.getResultCallback());
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CLIR:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_SET_CLIR");
                int commandInterfaceCLIRMode = ss.mParcel.readInt();
                setOutgoingCallerIdDisplayInternal(commandInterfaceCLIRMode,
                        ss.getResultCallback());
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CW:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_GET_CW");
                getCallWaitingInternal(ss.getResultCallback());
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CW:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_SET_CW");
                int enableState = ss.mParcel.readInt();
                boolean enable = (enableState != 0);
                setCallWaitingInternal(enable, ss.getResultCallback());
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_GET_CB:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_GET_CB");
                String facility = ss.mParcel.readString();
                String password = ss.mParcel.readString();
                int serviceClass = ss.mParcel.readInt();
                getCallBarringInternal(facility, password, ss.getResultCallback(), serviceClass);
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_SET_CB:
            {
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_SET_CB");
                String facility = ss.mParcel.readString();
                int enableState = ss.mParcel.readInt();
                boolean enable = (enableState != 0);
                String password = ss.mParcel.readString();
                int serviceClass = ss.mParcel.readInt();
                setCallBarringInternal(facility, enable, password, ss.getResultCallback(),
                        serviceClass);
                break;
            }
            case MtkSuppSrvRequest.SUPP_SRV_REQ_MMI_CODE:
            {
                String dialString = ss.mParcel.readString();
                Rlog.d(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_MMI_CODE: dialString = "
                        + dialString);
                try {
                    dial(dialString, new DialArgs.Builder().build());
                } catch (CallStateException ex) {
                    Rlog.e(LOG_TAG, "handleImsUtCsfb: SUPP_SRV_REQ_MMI_CODE: CallStateException!");
                    ex.printStackTrace();
                }
                break;
            }
            default:
                Rlog.e(LOG_TAG, "handleImsUtCsfb: invalid requestCode = " + requestCode);
                break;
        }

        ss.setResultCallback(null);
        ss.mParcel.recycle();
    }

    public  void dumpPendingMmi() {
        int size = mPendingMMIs.size();
        if (size == 0) {
            Rlog.d(LOG_TAG, "dumpPendingMmi: none");
            return;
        }
        for (int i=0; i<size; i++) {
            Rlog.d(LOG_TAG, "dumpPendingMmi: " + mPendingMMIs.get(i));
        }
    }

    @Override
    protected void onIncomingUSSD (int ussdMode, String ussdMessage) {
        if (!isPhoneTypeGsm()) {
            loge("onIncomingUSSD: not expected on GSM");
        }
        boolean isUssdError;
        boolean isUssdRequest;
        boolean isUssdRelease;

        isUssdRequest
            = (ussdMode == CommandsInterface.USSD_MODE_REQUEST);

        isUssdError
            = (ussdMode != CommandsInterface.USSD_MODE_NOTIFY
                && ussdMode != CommandsInterface.USSD_MODE_REQUEST);

        isUssdRelease = (ussdMode == CommandsInterface.USSD_MODE_NW_RELEASE);


        // See comments in GsmMmiCode.java
        // USSD requests aren't finished until one
        // of these two events happen
        GsmMmiCode found = null;
        for (int i = 0, s = mPendingMMIs.size() ; i < s; i++) {
            if(((GsmMmiCode)mPendingMMIs.get(i)).isPendingUSSD()) {
                found = (GsmMmiCode)mPendingMMIs.get(i);
                break;
            }
        }

        if (found != null) {
            super.onIncomingUSSD(ussdMode, ussdMessage);
        } else if (!isUssdError && ussdMessage != null) {
            super.onIncomingUSSD(ussdMode, ussdMessage);
        }  else if (isUssdError) {
                MtkGsmMmiCode mmi;
                mmi = MtkGsmMmiCode.newNetworkInitiatedUssdError(ussdMessage,
                                                   isUssdRequest,
                                                   MtkGsmCdmaPhone.this,
                                                   mUiccApplication.get());
                onNetworkInitiatedUssd(mmi);
        }
    }

    public void setServiceClass(int serviceClass) {
        Rlog.d(LOG_TAG, "setServiceClass: " + serviceClass);
        SystemProperties.set(SS_SERVICE_CLASS_PROP, String.valueOf(serviceClass));
    }

    /**
     * MTK
     * Check whether GSM support UT interface for the
     * supplementary service configuration or not.
     *
     * @return true if support UT interface in GSMPhone
     */
    public boolean isGsmUtSupport() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isGsmUtSupport, ssConf is null, return false");
            return false;
        }

        boolean isRoaming = getServiceState().getRoaming();
        if (SystemProperties.get("persist.vendor.ims_support").equals("1")
                && SystemProperties.get("persist.vendor.volte_support").equals("1")
                && ssConf.isGsmUtSupport(getOperatorNumeric()) && isUsimCard() ) {
            boolean isWfcEnable = (mImsPhone != null) && (mImsPhone.isWifiCallingEnabled());
            boolean isWfcUtSupport = isWFCUtSupport();
            if (DBG) logd("in isGsmUtSupport isWfcEnable -->"+isWfcEnable + ",isWfcUtSupport-->"
                    + isWfcUtSupport);
            if(ssConf.isNeedCheckImsWhenRoaming(getOperatorNumeric()) &&
                    isRoaming && !isIMSRegistered()) {
                if (DBG) logd("in isGsmUtSupport isRoaming -->"+isRoaming + ",isIMSRegistered-->"
                    + isIMSRegistered());
                return false;
            }
            if (isWfcEnable && !isWfcUtSupport) {
                return false;
            }
            return true;
        }
        return false;
    }

    /* Check whether WifiCalling support UT interface for the
     * supplementary service configuration or not.
     *
     * @return true if support UT interface in GSMPhone
     */
    public boolean isWFCUtSupport() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isWFCUtSupport, ssConf is null, return false");
            return false;
        }

        if (SystemProperties.get("persist.vendor.ims_support").equals("1") &&
                SystemProperties.get("persist.vendor.mtk_wfc_support").equals("1")) {
            if (ssConf.isNotSupportWFCUt(getOperatorNumeric())) {
                return false;
            } else {
                return true;
            }
        }
        return false;
    }

    /* MTK
     * Check if it is a USIM card
     */
    private boolean isUsimCard() {
        if (isPhoneTypeGsm() &&  !isOp(OPID.OP09)) {
            boolean r = false;
            String iccCardType = (PhoneFactory.getPhone(getPhoneId()).
                    getIccCard()).getIccCardType();
            if (iccCardType != null && iccCardType.equals("USIM")) {
                r = true;
            }
            Rlog.d(LOG_TAG, "isUsimCard: " + r + ", " + iccCardType);
            return r;
        } else {
            String prop = null;
            String values[] = null;
            int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(getPhoneId());
            int slotId = SubscriptionManager.getSlotIndex(subId);
            if (slotId < 0 || slotId >= PROPERTY_RIL_FULL_UICC_TYPE.length) {
                return false;
            }
            prop = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[slotId], "");
            if ((!prop.equals("")) && (prop.length() > 0)) {
                values = prop.split(",");
            }
            Rlog.d(LOG_TAG, "isUsimCard PhoneId = " + getPhoneId() +
                    " cardType = " + Arrays.toString(values));
            if (values == null) {
                return false;
            }
            for (String s : values) {
                if (s.equals("USIM")) {
                    return true;
                }
            }
            return false;
        }
    }

    private boolean isOp(OPID id) {
        return MtkOperatorUtils.isOperator(getOperatorNumeric(), id);
    }

    public boolean isOpNotSupportOCB(String facility) {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isOpNotSupportOCB, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        boolean isOcb = false;
        if (facility.equals(CommandsInterface.CB_FACILITY_BAOC)
                || facility.equals(CommandsInterface.CB_FACILITY_BAOIC)
                || facility.equals(CommandsInterface.CB_FACILITY_BAOICxH)) {
            isOcb = true;
        }
        if (isOcb && ssConf.isNotSupportOCB(getOperatorNumeric())) {
            r = true;
        }
        Rlog.d(LOG_TAG, "isOpNotSupportOCB: " + r + ", facility=" + facility);
        return r;
    }

    /* For the MD platform which is before 92, set Call Waiting both on CS Network
     * and System Property (TBCW) if the operator does not support XCAP. (default behavior)
     */
    public boolean isOpTbcwWithCS() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isOpTbcwWithCS, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isNotSupportXcap(getOperatorNumeric())) {
            r = true;
        }
        Rlog.d(LOG_TAG, "isOpTbcwWithCS: " + r);
        return r;
    }

    /* Check whether Operator support Terminal-baased CLIR. */
    public boolean isOpTbClir() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isOpTbClir, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isTbClir(getOperatorNumeric())) {
            r = true;
        }
        Rlog.d(LOG_TAG, "isOpTbClir: " + r);
        return r;
    }

    /* Check whether Operator support IMS Network-based Call Waiting. */
    public boolean isOpNwCW() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isOpNwCW, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isImsNwCW(getOperatorNumeric())) {
           r = true;
        }
        Rlog.d(LOG_TAG, "isOpNwCW():" + r);
        return r;
    }

    /* Get the enable/disable for 409 conflict response.
     * @return Operator support 409 response.
     */
    public boolean isEnableXcapHttpResponse409() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isEnableXcapHttpResponse409, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isEnableXcapHttpResponse409(getOperatorNumeric())) {
           r = true;
        }
        Rlog.d(LOG_TAG, "isEnableXcapHttpResponse409: " + r);
        return r;
    }

    public boolean isOpTransferXcap404() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isOpTransferXcap404, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isTransferXcap404(getOperatorNumeric())) {
           r = true;
        }
        Rlog.d(LOG_TAG, "isOpTransferXcap404: " + r);
        return r;
    }

    public boolean isOpNotSupportCallIdentity() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isOpNotSupportCallIdentity, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isNotSupportCallIdentity(getOperatorNumeric())) {
           r = true;
        }
        Rlog.d(LOG_TAG, "isOpNotSupportCallIdentity: " + r);
        return r;
    }

    public boolean isOpReregisterForCF() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isOpReregisterForCF, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isReregisterForCF(getOperatorNumeric())) {
           r = true;
        }
        Rlog.d(LOG_TAG, "isOpReregisterForCF: " + r);
        return r;
    }

    private boolean isIccCardMncMccAvailable(int phoneId) {
        UiccController uiccCtl = UiccController.getInstance();
        IccRecords iccRecords = uiccCtl.getIccRecords(phoneId, UiccController.APP_FAM_3GPP);
        if (iccRecords != null) {
            String mccMnc = iccRecords.getOperatorNumeric();
            Rlog.d(LOG_TAG, "isIccCardMncMccAvailable(): mccMnc is " + mccMnc);
            return (mccMnc != null);
        }
        Rlog.d(LOG_TAG, "isIccCardMncMccAvailable(): false");
        return false;
    }

    /* MTK */
    public boolean isNotSupportUtToCS() {
        boolean r = false;
        if ((((SystemProperties.getInt("persist.vendor.mtk_ct_volte_support", 0) != 0) &&
                isOp(OPID.OP09) && isUsimCard()) || isOp(OPID.OP117))
                && !getServiceState().getRoaming()) {
           r = true;
        }
        Rlog.d(LOG_TAG, "isNotSupportUtToCS: " + r);
        return r;
    }

    /// M: 93 Ims SS native solution @{
    private boolean supportMdAutoSetupIms() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }

    /// M: 93 Ims SS native solution @{
    private boolean isUtError(CommandException.Error error) {
       /*
        * CME_409_CONFLICT                        CommandException.Error.OEM_ERROR_25
        * CME_NETWORK_TIMEOUT                     CommandException.Error.OEM_ERROR_3
        * CME_404_NOT_FOUND                       CommandException.Error.OEM_ERROR_4
        * CME_412_PRECONDITION_FAILED             CommandException.Error.OEM_ERROR_6
       */
       if (error == CommandException.Error.OEM_ERROR_25 ||
            error == CommandException.Error.OEM_ERROR_3 ||
            error == CommandException.Error.OEM_ERROR_4 ||
            error == CommandException.Error.OEM_ERROR_6) {
            return true;
        }
        return false;
    }
    /// @}

    /* MTK */
    /**
     * Check whether Operator support save the cf number to sharedpref.
     * @return true if Operator support save the cf number to sharedpref.
     */
    public boolean isSupportSaveCFNumber() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isSupportSaveCFNumber, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isSupportSaveCFNumber(getOperatorNumeric())) {
            r = true;
        }
        Rlog.d(LOG_TAG, "isSupportSaveCFNumber: " + r);
        return r;
    }

    /**
     * Clear CF number in sharedpref.
     * @param cfReason input call forwarding reason.
     */
    public void clearCFSharePreference(int cfReason) {
        String key = null;
        switch (cfReason) {
            case CF_REASON_BUSY:
                key = CFB_KEY + "_" + String.valueOf(mPhoneId);
                break;
            case CF_REASON_NO_REPLY:
                key = CFNR_KEY + "_" + String.valueOf(mPhoneId);
                break;
            case CF_REASON_NOT_REACHABLE:
                key = CFNRC_KEY + "_" + String.valueOf(mPhoneId);
                break;
            default:
                Rlog.e(LOG_TAG, "No need to store cfreason: " + cfReason);
                return;
        }

        Rlog.e(LOG_TAG, "Read to clear the key: " + key);

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(getContext());
        SharedPreferences.Editor editor = sp.edit();
        editor.remove(key);
        if (!editor.commit()) {
            Rlog.e(LOG_TAG, "failed to commit the removal of CF preference: " + key);
        } else {
            Rlog.e(LOG_TAG, "Commit the removal of CF preference: " + key);
        }
    }

    /**
     * Store the CF number in sharedpref.
     *
     * @param cfReason input cf reason.
     * @param setNumber is numebr.
     * @return true if save success.
     */
    public boolean applyCFSharePreference(int cfReason, String setNumber) {
        String key = null;
        switch (cfReason) {
            case CF_REASON_BUSY:
                key = CFB_KEY + "_" + String.valueOf(mPhoneId);
                break;
            case CF_REASON_NO_REPLY:
                key = CFNR_KEY + "_" + String.valueOf(mPhoneId);
                break;
            case CF_REASON_NOT_REACHABLE:
                key = CFNRC_KEY + "_" + String.valueOf(mPhoneId);
                break;
            default:
                Rlog.d(LOG_TAG, "No need to store cfreason: " + cfReason);
                return false;
        }

        IccRecords r = mIccRecords.get();
        if (r == null) {
            Rlog.d(LOG_TAG, "No iccRecords");
            return false;
        }

        String currentImsi = r.getIMSI();

        if (currentImsi == null || currentImsi.isEmpty()) {
            Rlog.d(LOG_TAG, "currentImsi is empty");
            return false;
        }

        if (setNumber == null || setNumber.isEmpty()) {
            Rlog.d(LOG_TAG, "setNumber is empty");
            return false;
        }

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(getContext());
        SharedPreferences.Editor editor = sp.edit();

        String content = currentImsi + ";" + setNumber;

        if (content == null || content.isEmpty()) {
            Rlog.e(LOG_TAG, "imsi or content are empty or null.");
            return false;
        }

        Rlog.e(LOG_TAG, "key: " + key);
        Rlog.e(LOG_TAG, "content: " + content);

        editor.putString(key, content);
        editor.apply();

        return true;
    }

    /**
     * Get previous CF number.
     *
     * @param cfReason input cf reason.
     * @return cf numebr from previous setting.
     */
    public String getCFPreviousDialNumber(int cfReason) {
        String key = null;
        switch (cfReason) {
            case CF_REASON_BUSY:
                key = CFB_KEY + "_" + String.valueOf(mPhoneId);
                break;
            case CF_REASON_NO_REPLY:
                key = CFNR_KEY + "_" + String.valueOf(mPhoneId);
                break;
            case CF_REASON_NOT_REACHABLE:
                key = CFNRC_KEY + "_" + String.valueOf(mPhoneId);
                break;
            default:
                Rlog.d(LOG_TAG, "No need to do the reason: " + cfReason);
                return null;
        }

        Rlog.d(LOG_TAG, "key: " + key);

        IccRecords r = mIccRecords.get();
        if (r == null) {
            Rlog.d(LOG_TAG, "No iccRecords");
            return null;
        }

        String currentImsi = r.getIMSI();

        if (currentImsi == null || currentImsi.isEmpty()) {
            Rlog.d(LOG_TAG, "currentImsi is empty");
            return null;
        }

        Rlog.d(LOG_TAG, "currentImsi: " + currentImsi);

        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(getContext());
        String info = sp.getString(key, null);

        if (info == null) {
            Rlog.d(LOG_TAG, "Sharedpref not with: " + key);
            return null;
        }

        String[] infoAry = info.split(";");

        if (infoAry == null || infoAry.length < 2) {
            Rlog.d(LOG_TAG, "infoAry.length < 2");
            return null;
        }

        String imsi = infoAry[0];
        String number = infoAry[1];

        if (imsi == null || imsi.isEmpty()) {
            Rlog.d(LOG_TAG, "Sharedpref imsi is empty.");
            return null;
        }

        if (number == null || number.isEmpty()) {
            Rlog.d(LOG_TAG, "Sharedpref number is empty.");
            return null;
        }

        Rlog.d(LOG_TAG, "Sharedpref imsi: " + imsi);
        Rlog.d(LOG_TAG, "Sharedpref number: " + number);

        if (currentImsi.equals(imsi)) {
            Rlog.d(LOG_TAG, "Get dial number from sharepref: " + number);
            return number;
        } else {
            SharedPreferences.Editor editor = sp.edit();
            editor.remove(key);
            if (!editor.commit()) {
                Rlog.e(LOG_TAG, "failed to commit the removal of CF preference: " + key);
            }
        }

        return null;
    }

    public boolean queryCFUAgainAfterSet() {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "queryCFUAgainAfterSet, ssConf is null, return false");
            return false;
        }

        boolean r = false;
        if (ssConf.isQueryCFUAgainAfterSet(getOperatorNumeric())) {
            r = true;
        }
        Rlog.d(LOG_TAG, "queryCFUAgainAfterSet: " + r);
        return r;
    }

    public boolean isSupportCFUTimeSlot() {
        boolean r = false;
        if (isOp(OPID.OP01)) {
            r = true;
        }
        Rlog.d(LOG_TAG, "isSupportCFUTimeSlot: " + r);
        return r;
    }

    public boolean isNotSupportUtToCSforCFUQuery() {
        return isNotSupportUtToCS();
    }

    public boolean isNoNeedToCSFBWhenIMSRegistered() {
        return isOp(OPID.OP01) || isOp(OPID.OP02);
    }

    public boolean isResetCSFBStatusAfterFlightMode() {
        return isOp(OPID.OP02);
    }

    @Override
    protected void processIccRecordEvents(int eventCode) {
        switch (eventCode) {
            case IccRecords.EVENT_CFI:
                Rlog.d(LOG_TAG, "processIccRecordEvents");
                notifyCallForwardingIndicator();
                break;
            default:
                super.processIccRecordEvents(eventCode);
        }
    }

    void sendErrorResponse(Message onComplete, CommandException.Error error) {
        Rlog.d(LOG_TAG, "sendErrorResponse" + error);
        if (onComplete != null) {
            AsyncResult.forMessage(onComplete, null, new CommandException(error));
            onComplete.sendToTarget();
        }
    }

    private boolean isAllowXcapIfDataRoaming(String mccMnc) {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isAllowXcapIfDataRoaming, ssConf is null, return false");
            return false;
        }

        // Check roaming state.
        //     -> if false, return directly.
        //     -> if true, check VOWIFI status.
        //                 -> if true, check skip wifi op list in roaming state.
        //                             -> if true, check VOWIFI state, and return directly.
        //                             -> if false, DONT CARE.
        //                 -> if false, check op list for support roaming nw.
        //                             -> if true, return false.
        //                             -> if false, return true.

        if (!getServiceState().getDataRoaming()) {
            Rlog.d(LOG_TAG, "isAllowXcapIfDataRoaming: true (not roaming state)");
            return true;
        }

        if (!ssConf.isNeedCheckDataRoaming(mccMnc)) {
            Rlog.d(LOG_TAG, "isAllowXcapIfDataRoaming: true (ignore roaming state)");
            return true;
        }

        Rlog.d(LOG_TAG, "isAllowXcapIfDataRoaming: false (roaming state, block SS)");
        return false;
    }

    private boolean isAllowXcapIfDataEnabled(String mccMnc) {
        MtkSuppServConf ssConf = MtkSuppServManager.getSuppServConf(getPhoneId());
        if (ssConf == null) {
            Rlog.d(LOG_TAG, "isAllowXcapIfDataEnabled, ssConf is null, return false");
            return false;
        }

        if (!ssConf.isNeedCheckDataEnabled(mccMnc)) {
            return true;
        }

        if (mDataEnabledSettings.isDataEnabled()) {
            Rlog.d(LOG_TAG, "isAllowXcapIfDataEnabled: true");
            return true;
        }

        Rlog.d(LOG_TAG, "isAllowXcapIfDataEnabled: false");
        return false;
    }

    /**
     * Get the UT CS fallback status.
     *
     * @return The UT CS fallback status:
     *         {@link MtkPhoneConstants.UT_CSFB_PS_PREFERRED}
     *         {@link MtkPhoneConstants.UT_CSFB_ONCE}
     *         {@link MtkPhoneConstants.UT_CSFB_UNTIL_NEXT_BOOT}.
     */
    public int getCsFallbackStatus() {
        if (!isAllowXcapIfDataEnabled(getOperatorNumeric())){
            mCSFallbackMode = MtkPhoneConstants.UT_CSFB_ONCE;
        }

        if (!isAllowXcapIfDataRoaming(getOperatorNumeric())){
            mCSFallbackMode = MtkPhoneConstants.UT_CSFB_ONCE;
        }

        Rlog.d(LOG_TAG, "getCsFallbackStatus is " + mCSFallbackMode);
        return mCSFallbackMode;
    }

    /**
     * Set the UT CS fallback status.
     *
     * @param newStatus The UT CS fallback status:
     *        {@link MtkPhoneConstants.UT_CSFB_PS_PREFERRED} for PS domain UT/XCAP SS request.
     *        {@link MtkPhoneConstants.UT_CSFB_ONCE} to fallback to CS domain SS request only once.
     *        {@link MtkPhoneConstants.UT_CSFB_UNTIL_NEXT_BOOT} to fallback to CS domain SS request
     *        until next reboot.
     */
    public void setCsFallbackStatus(int newStatus) {
        Rlog.d(LOG_TAG, "setCsFallbackStatus to " + newStatus);
        mCSFallbackMode = newStatus;
    }

    // ALPS03008063, IMS ECC should be dialed via the phone with main capability.
    /**
     * To get main capability phone id.
     *
     * @return The phone id with main capability.
     */
    private int getMainCapabilityPhoneId() {
        int phoneId = SystemProperties.getInt(MtkPhoneConstants.PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
            phoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        }
        return phoneId;
    }

    // For VoLTE enhanced conference call
    /**
     * Initiate a new conference host connection.
     *
     * @param numbers The dial numbers.
     * @param videoState The desired video state for the connection.
     * @exception CallStateException if a new outgoing call is not currently possible because
     * no more call slots exist or a call exists that is dialing, alerting, ringing, or waiting.
     * Other errors are handled asynchronously.
     * @return Connection the MO connection.
     * @hide
     */
    public Connection dial(List<String> numbers, int videoState) throws CallStateException {
        if (mImsPhone instanceof MtkImsPhone == false) {
            Rlog.d(LOG_TAG, "mImsPhone must be MtkImsPhone to make enhanced conference dial");
            return null;
        }
        // VoLTE enhanced conference call should never be emergency call.
        MtkLocalPhoneNumberUtils.setIsEmergencyNumber(false);

        MtkImsPhone imsPhone = (MtkImsPhone) mImsPhone;
        boolean imsUseEnabled = isImsUseEnabled()
                && imsPhone != null
                && (imsPhone.isVolteEnabled() || imsPhone.isWifiCallingEnabled() ||
                (imsPhone.isVideoEnabled() && VideoProfile.isVideo(videoState)))
                && (imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE);

        if (!imsUseEnabled) {
            Rlog.w(LOG_TAG, "IMS is disabled and can not dial conference call directly.");
            return null;
        }

        if (imsPhone != null) {
            Rlog.w(LOG_TAG, "service state = " + imsPhone.getServiceState().getState());
        }

        if (imsUseEnabled && imsPhone != null
                && (imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)) {
            try {
                if (DBG) {
                    Rlog.d(LOG_TAG, "Trying IMS PS conference call");
                }
                return imsPhone.dial(numbers, videoState);
            } catch (CallStateException e) {
                if (DBG) {
                    Rlog.d(LOG_TAG, "IMS PS conference call exception " + e);
                }
                if (!Phone.CS_FALLBACK.equals(e.getMessage())) {
                    CallStateException ce = new CallStateException(e.getMessage());
                    ce.setStackTrace(e.getStackTrace());
                    throw ce;
                }
            }
        }
        return null;
    }

    public boolean isIMSRegistered() {
        MtkSuppServHelper ssHelper = MtkSuppServManager.getSuppServHelper(getPhoneId());
        if (ssHelper != null) {
            return ssHelper.getIMSRegistered();
        }
        return false;
    }

    /**
     * Get CDMA subscription active status  by subId.
     * @return active status. 1 is active, 0 is deactive
     */
    public int getCdmaSubscriptionActStatus() {
        return (mCdmaSSM != null) ? ((MtkCdmaSubscriptionSourceManager)mCdmaSSM).getActStatus() : 0;
    }

    /// M: [Network][C2K] Sprint roaming control @{
    /**
     * Set the roaming enabling.
     * @param config the configuration
     *  config[0]: phone id
     *  config[1]: international_voice_text_roaming (0,1)
     *  config[2]: international_data_roaming (0,1)
     *  config[3]: domestic_voice_text_roaming (0,1)
     *  config[4]: domestic_data_roaming (0,1)
     *  config[5]: domestic_LTE_data_roaming (1)
     * @param response is callback message
     */
    public void setRoamingEnable(int[] config, Message response) {
        Rlog.d(LOG_TAG, "set roaming enable");
        // replace the phone id before
        config[0] = mPhoneId;
        mMtkCi.setRoamingEnable(config, response);
    }

    /**
     * Get the roaming enabling.
     * @param response is callback message
     */
    public void getRoamingEnable(Message response) {
        Rlog.d(LOG_TAG, "get roaming enable");
        mMtkCi.getRoamingEnable(mPhoneId, response);
    }
    /// @}

    /// M: Support ut ss for ct card @{
    public boolean isGsmSsPrefer() {
        if (((SystemProperties.getInt("persist.vendor.mtk_ct_volte_support", 0) != 0) &&
                isOp(OPID.OP09)) || isOp(OPID.OP117)) {
            return true;
        }
        return false;
    }
    /// @}

    @Override
    protected void onCheckForNetworkSelectionModeAutomatic(Message fromRil) {
        AsyncResult ar = (AsyncResult)fromRil.obj;
        Message response = (Message)ar.userObj;
        boolean doAutomatic = true;
        if (ar.exception == null && ar.result != null) {
            try {
                int[] modes = (int[])ar.result;
                if (modes[0] == 0) {
                    // already confirmed to be in automatic mode - don't resend
                    doAutomatic = false;
                    // M: show connect_later when OOS and already auto mode
                    MtkServiceState mss = (MtkServiceState) getServiceState();
                    if (mss.getCellularRegState() != ServiceState.STATE_IN_SERVICE) {
                        // both CS and PS are not in serivce
                        ar.exception = new CommandException(CommandException.Error.ABORTED);
                    }
                }
            } catch (Exception e) {
                // send the setting on error
            }
        }

        // wrap the response message in our own message along with
        // an empty string (to indicate automatic selection) for the
        // operator's id.
        NetworkSelectMessage nsm = new NetworkSelectMessage();
        nsm.message = response;
        nsm.operatorNumeric = "";
        nsm.operatorAlphaLong = "";
        nsm.operatorAlphaShort = "";

        if (doAutomatic) {
            Message msg = obtainMessage(EVENT_SET_NETWORK_AUTOMATIC_COMPLETE, nsm);
            mCi.setNetworkSelectionModeAutomatic(msg);
        } else {
            Rlog.d(LOG_TAG, "setNetworkSelectionModeAutomatic - already auto, ignoring");
            ar.userObj = nsm;
            handleSetSelectNetwork(ar);
        }

        updateSavedNetworkOperator(nsm);
    }

    /**
     * @return operator numeric.
     */
    protected String getOperatorNumeric() {
        if (!isPhoneTypeGsm()) {
            /// M: update mCdmaSubscriptionSource to avoid wifi only,radio off case. @{
            mCdmaSubscriptionSource = mCdmaSSM.getCdmaSubscriptionSource();
            /// @}
        }
        return super.getOperatorNumeric();
    }

    public Message getCFCallbackMessage() {
        return obtainMessage(EVENT_GET_CALL_FORWARD_DONE);
    }

    public Message getCFTimeSlotCallbackMessage() {
        return obtainMessage(EVENT_GET_CALL_FORWARD_TIME_SLOT_DONE);
    }

    /**
     * Determines if  IMS is enabled for call.
     *
     * @return {@code true} if IMS calling is enabled.
     */
    @Override
    public boolean isImsUseEnabled() {
        // M: Remove EnabledByPlatform to improve dial performance, and add logs to debug.
        ImsManager imsManager = ImsManager.getInstance(mContext, mPhoneId);
        boolean isEnhanced4gLteModeSettingEnabledByUser =
                imsManager.isEnhanced4gLteModeSettingEnabledByUser();
        boolean isWfcEnabledByUser = true;
        boolean isNonTtyOrTtyOnVolteEnabled = true;

        // M: Only get these values when needed to improve dial performance.
        if (!isEnhanced4gLteModeSettingEnabledByUser) {
            isWfcEnabledByUser = imsManager.isWfcEnabledByUser();
            isNonTtyOrTtyOnVolteEnabled = imsManager.isNonTtyOrTtyOnVolteEnabled();
        }
        boolean imsUseEnabled = (isEnhanced4gLteModeSettingEnabledByUser || isWfcEnabledByUser
                && isNonTtyOrTtyOnVolteEnabled);
        Rlog.d(LOG_TAG, "isImsUseEnabled() VolteEnableByUser: " +
                isEnhanced4gLteModeSettingEnabledByUser + ", WfcEnableByUser: " +
                isWfcEnabledByUser + ", isNonTtyOrTtyOnVolteEnabled: " +
                isNonTtyOrTtyOnVolteEnabled);
        return imsUseEnabled;
    }

    /* Clean the CFU status in share preference */
    public void cleanCallForwardingIndicatorFromSharedPref() {
        setCallForwardingIndicatorInSharedPref(false);
    }

    /// M: CC: VzW Self activation. @{
    public boolean shouldProcessSelfActivation() {
        int selfActivateState = getSelfActivationInstance().getSelfActivateState();

        Rlog.d(LOG_TAG, "shouldProcessSelfActivation() state: " + selfActivateState);

        return (selfActivateState == ISelfActivation.STATE_NOT_ACTIVATED);
    }

    public boolean useImsForPCOChanged() {
        int pcoState = getSelfActivationInstance().getPCO520State();

        Rlog.d(LOG_TAG, "pcoState() state: " + pcoState);

        return (pcoState == ISelfActivation.STATE_520_ACTIVATED);
    }
    /// @}
    // PHB Start
    @Override
    protected boolean needResetPhbIntMgr() {
        return false;
    }
    // PHB End

    @Override
    public String getFullIccSerialNumber() {
        String iccId = super.getFullIccSerialNumber();
        if (iccId != null) {
            return iccId;
        }

        // Get ICCID from SIMRecords, because it maybe not ready in RUIMRecords for CDMA 4G
        if (!isPhoneTypeGsm() && mUiccController != null) {
            IccRecords r = null;
            r = mUiccController.getIccRecords(mPhoneId, UiccController.APP_FAM_3GPP);
            iccId = (r != null) ? r.getFullIccId() : null;
            if (iccId != null) {
                return iccId;
            }
        }

        // Now that all security checks passes, perform the operation as ourselves.
        final long identity = Binder.clearCallingIdentity();
        try {
            // Get ICCID from cache of active SubscriptionInfo
            int subId = getSubId();
            SubscriptionInfo subInfo = SubscriptionManager.from(getContext())
                    .getActiveSubscriptionInfo(subId);
            if (subInfo != null) {
                iccId = subInfo.getIccId();
            }
            return iccId;
        } finally {
            Binder.restoreCallingIdentity(identity);
        }
    }


    @Override
    protected void initRatSpecific(int precisePhoneType) {
        super.initRatSpecific(precisePhoneType);
        if (isPhoneTypeGsm()) {
            // M: CC: Allow ECM under GSM
            mIsPhoneInEcmState = getInEcmMode();
            if (mIsPhoneInEcmState) {
                // Send a message which will invoke handleExitEmergencyCallbackMode
                mCi.exitEmergencyCallbackMode(
                        obtainMessage(EVENT_EXIT_EMERGENCY_CALLBACK_RESPONSE));
            }
        }
    }

    @Override
    public void exitEmergencyCallbackMode() {
        if (DBG) {
            Rlog.d(LOG_TAG, "exitEmergencyCallbackMode: mImsPhone=" + mImsPhone
                    + " isPhoneTypeGsm=" + isPhoneTypeGsm());
        }
        Rlog.d(LOG_TAG, "exitEmergencyCallbackMode()");
        /// M: IMS ECBM,ECBM wifi control. @{
        tryTurnOnWifiForE911Finished();
        /// @}
        if (isPhoneTypeGsm()) {
            if (mImsPhone != null) {
                mImsPhone.exitEmergencyCallbackMode();
            }
        } else {
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
            }
            // Send a message which will invoke handleExitEmergencyCallbackMode
            // M: CC: We don't trigger handleExitEmergencyCallbackMode here because ECBM has not
            // actually exited yet. Instead it will be invoked when ECBM exited URC reported.
            //mCi.exitEmergencyCallbackMode(obtainMessage(EVENT_EXIT_EMERGENCY_CALLBACK_RESPONSE));
            mCi.exitEmergencyCallbackMode(null);
        }
    }

    public void sendExitEmergencyCallbackModeMessage() {
        Rlog.d(LOG_TAG, "sendExitEmergencyCallbackModeMessage()");
        Message message = obtainMessage(EVENT_EXIT_EMERGENCY_CALLBACK_RESPONSE);
        AsyncResult.forMessage(message);
        sendMessage(message);
    }

    /// M: E911 Wifi control. @{
    private void tryTurnOffWifiForE911(boolean isEcc) {
        if (mContext == null) {
            return;
        }

        CarrierConfigManager configManager =
                (CarrierConfigManager) mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        boolean turnOffWifiForEcc = configManager.getConfigForSubId(getSubId())
                .getBoolean(MtkCarrierConfigManager.MTK_KEY_CARRIER_TURN_OFF_WIFI_BEFORE_E911);

        boolean isWfcEnabled = SystemProperties.getInt(PROPERTY_WFC_ENABLE, 0) == 1;
        Rlog.d(LOG_TAG, "tryTurnOffWifiForEcc() carrierConfig: " + turnOffWifiForEcc +
            " isECC: " + isEcc + " isWfcEnable: " + isWfcEnabled);
        if (isEcc == false || turnOffWifiForEcc == false || isWfcEnabled) {
            return;
        }
        WifiManager wifiMngr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        mWifiIsEnabledBeforeE911 = wifiMngr.isWifiEnabled();
        Rlog.d(LOG_TAG, "tryTurnOffWifiForEcc() wifiEnabled: " + mWifiIsEnabledBeforeE911);
        if (mWifiIsEnabledBeforeE911) {
            wifiMngr.setWifiEnabled(false);
        }
    }

    private void tryTurnOnWifiForE911Finished() {
        if (mContext == null) {
            return;
        }

        CarrierConfigManager configManager =
                (CarrierConfigManager) mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        boolean turnOffWifiForEcc = configManager.getConfigForSubId(getSubId())
                .getBoolean(MtkCarrierConfigManager.MTK_KEY_CARRIER_TURN_OFF_WIFI_BEFORE_E911);

        if (turnOffWifiForEcc == false) {
            return;
        }

        WifiManager wifiMngr = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        Rlog.d(LOG_TAG, "tryTurnOnWifiForEcbmFinished() wifiEnabled: " + mWifiIsEnabledBeforeE911);
        if (mWifiIsEnabledBeforeE911) {
            wifiMngr.setWifiEnabled(true);
        }
    }
    /// @}

    @Override
    public String getDeviceSvn() {
        //M: When phone type is CDMA, also need the mImeiSv, so add isPhoneTypeCdma(). @{
        if (isPhoneTypeGsm() || isPhoneTypeCdmaLte() || isPhoneTypeCdma()) {
            /// @}
            return mImeiSv;
        } else {
            loge("getDeviceSvn(): return 0");
            return "0";
        }
    }

    /**
     * Invokes RIL_REQUEST_OEM_HOOK_RAW on RIL implementation.
     *
     * @param data The data for the request.
     * @param response <strong>On success</strong>,
     * (byte[])(((AsyncResult)response.obj).result)
     * <strong>On failure</strong>,
     * (((AsyncResult)response.obj).result) == null and
     * (((AsyncResult)response.obj).exception) being an instance of
     * com.android.internal.telephony.gsm.CommandException
     *
     * @see #invokeOemRilRequestRaw(byte[], android.os.Message)
     */
    public void invokeOemRilRequestRaw(byte[] data, Message response) {
        mMtkCi.invokeOemRilRequestRaw(data, response);
    }

    @Override
    protected void phoneObjectUpdater(int newVoiceRadioTech) {
        mNewVoiceTech = newVoiceRadioTech;
        super.phoneObjectUpdater(newVoiceRadioTech);
    }

    public void setDisable2G(boolean mode, Message result)
    {
        Rlog.d(LOG_TAG, "setDisable2G " + mode);
        mMtkCi.setDisable2G(mode, result);
    }

    public void getDisable2G(Message result)
    {
        Rlog.d(LOG_TAG, "getDisable2G");
        mMtkCi.getDisable2G(result);
    }

    @Override
    public void setVoiceCallForwardingFlag(int line, boolean enable, String number) {
        super.setVoiceCallForwardingFlag(line, enable, number);
        if (getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA && isGsmSsPrefer()) {
            if (mUiccController != null) {
                IccRecords r =
                        mUiccController.getIccRecords(mPhoneId, UiccController.APP_FAM_3GPP);
                if (r != null) {
                    r.setVoiceCallForwardingFlag(line, enable, number);
                }
            }
            notifyCallForwardingIndicator();
        }
    }

    @Override
    public boolean getCallForwardingIndicator() {
        if (getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA && isGsmSsPrefer()) {
            IccRecords r = null;
            if (mUiccController != null) {
                r = mUiccController.getIccRecords(mPhoneId, UiccController.APP_FAM_3GPP);
            }
            int callForwardingIndicator = IccRecords.CALL_FORWARDING_STATUS_UNKNOWN;
            if (r != null) {
                callForwardingIndicator = r.getVoiceCallForwardingFlag();
                Rlog.v(LOG_TAG, "getCallForwardingIndicator: from icc record = " +
                       callForwardingIndicator);
            }
            if (callForwardingIndicator == IccRecords.CALL_FORWARDING_STATUS_UNKNOWN) {
                callForwardingIndicator = getCallForwardingIndicatorFromSharedPref();
            }
            Rlog.v(LOG_TAG, "getCallForwardingIndicator: iccForwardingFlag=" + (r != null
                        ? r.getVoiceCallForwardingFlag() : "null") + ", sharedPrefFlag="
                        + getCallForwardingIndicatorFromSharedPref());
            return (callForwardingIndicator == IccRecords.CALL_FORWARDING_STATUS_ENABLED);
        }
        return super.getCallForwardingIndicator();
    }

    public boolean isCdmaLessDevice() {
        boolean isCdmaLess = false;
        if ("3".equals(SystemProperties.get(PROP_VZW_DEVICE_TYPE, "0")) ||
                "4".equals(SystemProperties.get(PROP_VZW_DEVICE_TYPE, "0"))) {
            isCdmaLess = true;
        }
        Rlog.d(LOG_TAG, "isCdmaLess: " + isCdmaLess);
        return isCdmaLess;
    }

    /**
        * Invokes RIL_REQUEST_OEM_HOOK_STRINGS on RIL implementation.
        *
        * @param strings The strings to make available as the request data.
        * @param response <strong>On success</strong>, "response" bytes is
        * made available as:
        * (String[])(((AsyncResult)response.obj).result).
        * <strong>On failure</strong>,
        * (((AsyncResult)response.obj).result) == null and
        * (((AsyncResult)response.obj).exception) being an instance of
        * com.android.internal.telephony.gsm.CommandException
        *
        * @see #invokeOemRilRequestStrings(java.lang.String[], android.os.Message)
        * @deprecated OEM needs a vendor-extension hal and their apps should use that instead
        */
    public void invokeOemRilRequestStrings(String[] strings, Message response) {
        mMtkCi.invokeOemRilRequestStrings(strings, response);
    }

    /// M: SS IOT EAZY CONFIG @{
    public void setSuppServProperty(String name, String value) {
        Rlog.d(LOG_TAG, "setSuppServProperty, name = " + name + ", value = " + value);
        mCallbackLatch = new CountDownLatch(1);
        mMtkCi.setSuppServProperty(name, value, obtainMessage(EVENT_SET_SS_PROPERTY));
        if (!isCallbackDone()) {
            // do error handling
            Rlog.e(LOG_TAG, "waitForCallback: callback is not done!");
        }
    }

    private boolean isCallbackDone() {
        boolean isDone = false;

        try {
            isDone = mCallbackLatch.await(OPERATION_TIME_OUT_MILLIS,
                    TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            isDone = false;
        }

        Rlog.d(LOG_TAG, "waitForCallback: isDone=" + isDone);
        return isDone;
    }
    // @}


    // M: CC: Add a system property to indicate ECM by slot because AOSP only set
    // one global property. @{
    @Override
    public void setIsInEcm(boolean isInEcm) {
        super.setIsInEcm(isInEcm);
        setSystemProperty(PROPERTY_INECM_MODE_BY_SLOT, String.valueOf(isInEcm));
    }
    // @}

    @Override
    public PhoneConstants.State getState() {
        if (mImsPhone != null) {
            PhoneConstants.State imsState = mImsPhone.getState();
            if (imsState != PhoneConstants.State.IDLE) {
                return imsState;
            }
        }

        /// M: In SRVCC handover state, IMS call tracker and CS call tracker
        //  will be both idle which return wrong value in this case @{
        if (mCT.mState == PhoneConstants.State.IDLE &&
                ((MtkGsmCdmaCallTracker)mCT).getHandoverConnectionSize() > 0) {
            return PhoneConstants.State.OFFHOOK;
        }
        /// @}

        return mCT.mState;
    }

    /**
     * Retrieves the phone number string for line 1 of a subcription.
     * @return String phone number
     */
    public String getLine1PhoneNumber() {
        if (isPhoneTypeGsm()) {
            String optr = SystemProperties.get("persist.vendor.operator.optr");
            if (optr != null && "OP20".equals(optr)) {
                MtkIccCardConstants.CardType mCdmaCardType = MtkTelephonyManagerEx
                        .getDefault().getCdmaCardType(getPhoneId());
                boolean isCdma4g = false;

                if (mCdmaCardType != null) {
                    isCdma4g = mCdmaCardType.is4GCard();
                }
                if (isCdma4g) {
                    RuimRecords rr = (RuimRecords) UiccController.getInstance().getIccRecords(
                            getPhoneId(), UiccController.APP_FAM_3GPP2);
                    IccRecords r = mIccRecords.get();

                    logd("getLine1PhoneNumber, number = " + ((rr != null && rr.getMdn() != null
                            && !rr.getMdn().isEmpty()) ? rr.getMdn() : ((r != null)
                            ? r.getMsisdnNumber() : null)) + ", slot = " + getPhoneId());

                    return (rr != null && rr.getMdn() != null && !rr.getMdn().isEmpty())
                            ? rr.getMdn() : ((r != null) ? r.getMsisdnNumber() : null);
                } else {
                    IccRecords r = mIccRecords.get();
                    return (r != null) ? r.getMsisdnNumber() : null;
                }
            } else {
                IccRecords r = mIccRecords.get();
                return (r != null) ? r.getMsisdnNumber() : null;
            }
        } else {
            return mSST.getMdnNumber();
        }
    }

    /**
     * This is one shot event during boot up or RIL_CONNECTED
     */
    public void notifyMtkServiceStateChanged(MtkServiceState ss) {
        ((MtkPhoneNotifier)mNotifier).notifyMtkServiceState(this, ss);
        // It's too fast to use Registrants. Add any API as listener

        /// M: Data icon performance enhancement @{
        MtkDcTracker dct = (MtkDcTracker)getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (dct != null) {
            dct.notifyMtkServiceStateChanged(ss);
        }
        /// @}
    }

    public void notifyMtkSignalStrength(SignalStrength ss) {
        ((MtkPhoneNotifier)mNotifier).notifyMtkSignalStrength(this, ss);
    }

    @Override
    public String getSubscriberId() {
        String subscriberId = null;
        MtkDcTracker dct = (MtkDcTracker)
                getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
        if (dct != null) {
            subscriberId = dct.getImsi();
        }
        if (TextUtils.isEmpty(subscriberId)) {
            subscriberId = super.getSubscriberId();
        }
        return subscriberId;
    }

    public void iwlanSetRegisterCellularQualityReport(int qualityRegister, int type, int[] values,
            Message result) {
        mMtkCi.iwlanSetRegisterCellularQualityReport(qualityRegister, type, values, result);
    }

    public void getSuggestedPlmnList(int rat, int num, int timer, Message onCompleted){
        mMtkCi.getSuggestedPlmnList(rat, num, timer, onCompleted);
    }

    /**
     * OP129 Spec APN_2_1_2_8 & APN_2_1_2_6
     * isOp129DunChange to check if UE is under 2G/3G or roaming
     */
     public boolean isOp129DunChange() {
         // For KDDI, we only consider DUN over cellular case
         // The spec didn't mention the DUN over Wifi
         MtkDcTracker dct = (MtkDcTracker)
                    getDcTracker(AccessNetworkConstants.TRANSPORT_TYPE_WWAN);
         if (dct != null) {
             return dct.isOp129DunChange();
         }
         return false;
     }
}
