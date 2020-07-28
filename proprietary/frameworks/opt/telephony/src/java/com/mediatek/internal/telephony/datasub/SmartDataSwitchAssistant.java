package com.mediatek.internal.telephony.datasub;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.ConnectivityManager.NetworkCallback;
import android.net.NetworkRequest;
import android.net.NetworkCapabilities;
import android.net.Network;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.PreciseCallState;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.TelephonyManager;

import com.android.ims.ImsException;

import com.android.internal.telephony.GsmCdmaCall;
import com.android.internal.telephony.GsmCdmaCallTracker;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCallTracker;
import com.android.internal.telephony.imsphone.ImsPhoneCall;

import com.android.internal.telephony.IOnSubscriptionsChangedListener;
import com.android.internal.telephony.ITelephonyRegistry;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.SettingsObserver;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.ims.internal.MtkImsManagerEx;

import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.WifiOffloadManager;
import com.mediatek.wfo.IMwiService;
import com.mediatek.wfo.MwisConstants;

import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkHardwareConfig;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import com.mediatek.provider.MtkSettingsExt;

import static com.mediatek.internal.telephony.datasub.DataSubConstants.*;

public class SmartDataSwitchAssistant extends Handler {
    private static final boolean DBG = true;
    private static final String LOG_TAG = "SmartDataSwitch";

    private static SmartDataSwitchAssistant sSmartDataSwitchAssistant = null;
    private ISmartDataSwitchAssistantOpExt mSmartDataOpExt  = null;
    protected ContentResolver mResolver;
    protected Phone[] mPhones;
    protected int mPhoneNum;
    protected int mDefaultDataPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
    protected boolean isTemporaryDataServiceSettingOn = false;

    protected int mInCallPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;

    // Reset settings value of temporary data service when sim hot plug and data switch
    protected boolean isResetTdsSettingsByFwk = true;

    private static String mOperatorSpec;
    private Context mContext = null;

    // system service
    private OpTelephonyCustomizationFactoryBase mTelephonyCustomizationFactory = null;
    private TelephonyManager mTelephonyManager = null;
    private ConnectivityManager mConnectivityManager;
    private ConnectivityManager.NetworkCallback mNetworkCallback;
    private IWifiOffloadService mWfoService;

    private HandoverStateListener mHandoverStateListener;

    // event id must be multiple of EVENT_ID_INTVL
    private static final int EVENT_ID_INTVL = 10;
    private static final int EVENT_CALL_STARTED = EVENT_ID_INTVL * 1;
    private static final int EVENT_CALL_ENDED = EVENT_ID_INTVL * 2;
    private static final int EVENT_SRVCC_STATE_CHANGED = EVENT_ID_INTVL * 3;
    private static final int EVENT_TEMPORARY_DATA_SERVICE_SETTINGS = EVENT_ID_INTVL * 4;
    private static final int EVENT_SERVICE_STATE_CHANGED = EVENT_ID_INTVL * 5;

    // Settings URI of tempoary data service enable
    private static final String TEMP_DATA_SERVICE = MtkSettingsExt.Global.DATA_SERVICE_ENABLED;
    private static final String PROPERTY_DEFAULT_DATA_SELECTED
            = "persist.vendor.radio.default.data.selected";

    private final RegistrantList mReEvalueRegistrants;

    protected final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Message msg = null;
            if (action == null) {
                return;
            }
            if (action.equals(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED)) {
                // update mDefaultDataPhoneId
                int defaultDataSubId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                logd("onReceive: DEFAULT_DATA_SUBSCRIPTION_CHANGED defaultDataSubId=" +
                        defaultDataSubId);
                updateDefaultDataPhoneId(defaultDataSubId, "DataSubChanged");
            } else if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                int status = intent.getIntExtra(MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS,
                        MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE);
                // check if reset settings is handle by framework when sim plug out.
                if (status == MtkSubscriptionManager.EXTRA_VALUE_REMOVE_SIM ||
                            status == MtkSubscriptionManager.EXTRA_VALUE_NEW_SIM) {
                    logd("onSubInfoRecordUpdated: Detecct Status:" + status);
                    resetTdsSettingsByFwk();
                }
            } else {
                // unknown action
            }
        }
    };

    private class HandoverStateListener extends WifiOffloadManager.Listener {
        @Override
        public void onHandover(int simIdx, int stage, int ratType) {
            logd("onHandover() simIdx:" + simIdx + " stage:" + stage + " ratType:" + ratType);
            Message msg = null;
            if (stage == WifiOffloadManager.HANDOVER_END) {
                if (ratType == WifiOffloadManager.RAN_TYPE_WIFI) {
                    onHandoverToWifi();
                    mReEvalueRegistrants.notifyRegistrants();
                } else if (ratType == WifiOffloadManager.RAN_TYPE_MOBILE_3GPP ||
                    ratType == WifiOffloadManager.RAN_TYPE_MOBILE_3GPP2){
                    onHandoverToCellular();
                    mReEvalueRegistrants.notifyRegistrants();
                }
            }
        }
    }

    private final OnSubscriptionsChangedListener mSubscriptionsChangedListener =
            new OnSubscriptionsChangedListener() {
        @Override
        public void onSubscriptionsChanged() {
            // update default dat sub id
            int defaultDataSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            updateDefaultDataPhoneId(defaultDataSubId, "SubscriptionsChanged");
        }
    };

    protected final SettingsObserver mSettingsObserver;
    protected void regSettingsObserver() {
        mSettingsObserver.unobserve();
        mSettingsObserver.observe(
                Settings.Global.getUriFor(TEMP_DATA_SERVICE),
                EVENT_TEMPORARY_DATA_SERVICE_SETTINGS);
    }

    public static SmartDataSwitchAssistant makeSmartDataSwitchAssistant(Context context,
            Phone[] phones) {
        if (context == null || phones == null) {
            throw new RuntimeException("param is null");
        }

        if (sSmartDataSwitchAssistant == null) {
            sSmartDataSwitchAssistant = new SmartDataSwitchAssistant(context, phones);
        }
        logd("makeSDSA: X sSDSA =" + sSmartDataSwitchAssistant);
        return sSmartDataSwitchAssistant;
    }

    public static SmartDataSwitchAssistant getInstance() {
        if (sSmartDataSwitchAssistant == null) {
            throw new RuntimeException("Should not be called before sSmartDataSwitchAssistant");
        }
        return sSmartDataSwitchAssistant;
    }

    private SmartDataSwitchAssistant(Context context, Phone[] phones) {
        logd(" is created");
        mPhones = phones;
        mPhoneNum = phones.length;
        mContext = context;
        mResolver = mContext.getContentResolver();
        mOperatorSpec = SystemProperties.get(PROPERTY_OPERATOR_OPTR, OPERATOR_OM);
        int tempDataSettings = 0;
        mSettingsObserver = new SettingsObserver(mContext, this);

        if (isSmartDataSwitchSupport()) {
            registerEvents();
        }

        initOpSmartDataSwitchAssistant(context);
        if (mSmartDataOpExt == null) {
            mSmartDataOpExt = new SmartDataSwitchAssistantOpExt(context);
        }
        mSmartDataOpExt.init(this);

        mReEvalueRegistrants = new RegistrantList();

        tempDataSettings = Settings.Global.getInt(mResolver, TEMP_DATA_SERVICE, 0);
        if (tempDataSettings != 0) {
            isTemporaryDataServiceSettingOn = true;
        } else {
            isTemporaryDataServiceSettingOn = false;
        }
        logd("init isTemporaryDataServiceSettingOn=" + isTemporaryDataServiceSettingOn);
    }

    private void initOpSmartDataSwitchAssistant(Context context) {
       try {
           mTelephonyCustomizationFactory =
                   OpTelephonyCustomizationUtils.getOpFactory(context);
           mSmartDataOpExt =
                   mTelephonyCustomizationFactory.makeSmartDataSwitchAssistantOpExt(context);
       } catch (Exception e) {
           if (DBG) loge("mSmartDataOpExt init fail");
           e.printStackTrace();
       }
    }

    public void dispose() {
        logd("SmartDataSwitchAssistant.dispose");
        if (isSmartDataSwitchSupport()) {
            unregisterEvents();
        }
    }

    private void registerEvents() {
        logd("registerEvents");

        regSettingsObserver();

        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        //filter.addAction(TelephonyManager.ACTION_PRECISE_CALL_STATE_CHANGED);
        //filter.addAction(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        if (isResetTdsSettingsByFwk) {
            filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        }
        mContext.registerReceiver(mBroadcastReceiver, filter);

        SubscriptionManager.from(mContext).addOnSubscriptionsChangedListener(
                mSubscriptionsChangedListener);
    }

    private void unregisterEvents() {
        logd("unregisterEvents");
        TelephonyManager tm = getTelephonyManager();

        SubscriptionManager.from(mContext).removeOnSubscriptionsChangedListener(
                mSubscriptionsChangedListener);
    }

    @Override
    public void handleMessage(Message msg) {
        // msg_id = n * EVENT_ID_INTVL + phone_id, use mod operator to get phone_id
        // event_id must be multiple of EVENT_ID_INTVL => n * EVENT_ID_INTVL
        int phoneId = msg.what % EVENT_ID_INTVL;
        int eventId = msg.what - phoneId;
        AsyncResult ar;
        switch (eventId) {
            case EVENT_SRVCC_STATE_CHANGED:
                logd("SRVCC, phoneId=" + phoneId);
                onSrvccStateChanged();
                mReEvalueRegistrants.notifyRegistrants();
                break;
            case EVENT_TEMPORARY_DATA_SERVICE_SETTINGS:
                boolean oldSettings = isTemporaryDataServiceSettingOn;
                boolean newSettings = false;
                int tdsSettings = Settings.Global.getInt(mResolver, TEMP_DATA_SERVICE, 0);
                if (tdsSettings != 0){
                    newSettings = true;
                } else {
                    newSettings = false;
                }

                if (oldSettings != newSettings) {
                    isTemporaryDataServiceSettingOn = newSettings;
                    logd("TemporaryDataSetting changed newSettings=" + newSettings);
                    onTemporaryDataSettingsChanged();
                }
                break;
            case EVENT_SERVICE_STATE_CHANGED:
                if(onServiceStateChanged(phoneId)) {
                    logd("EVENT_SERVICE_STATE_CHANGED: notify");
                    mReEvalueRegistrants.notifyRegistrants();
                }
                break;
            default:
                logd("Unhandled message with number: " + msg.what);
                break;
        }
    }

    public void regSrvccEvent() {
        for (int i = 0; i < mPhoneNum; i++) {
            // register SRVCC event
            mPhones[i].registerForHandoverStateChanged(this, EVENT_SRVCC_STATE_CHANGED + i, null);
        }
    }

    public void unregSrvccEvent() {
        for (int i = 0; i < mPhoneNum; i++) {
            // unregister SRVCC event
            mPhones[i].unregisterForHandoverStateChanged(this);
        }
    }

    public void regServiceStateChangedEvent() {
        for (int i = 0; i < mPhoneNum; i++) {
            // register service state changed event
            mPhones[i].registerForServiceStateChanged(this, EVENT_SERVICE_STATE_CHANGED + i, null);
        }
    }

    public void unregServiceStateChangedEvent() {
        for (int i = 0; i < mPhoneNum; i++) {
            // unregister service state changed event
            mPhones[i].unregisterForServiceStateChanged(this);
        }
    }

    public void regImsHandoverEvent() {
        // register handover event. Ex: VoLTE handover to VoWifi.
        if (mWfoService == null) {
            mWfoService = getWifiOffLoadService();
        }
        if (mWfoService != null) {
            try {
                if (mHandoverStateListener == null) {
                    mHandoverStateListener = new HandoverStateListener();
                }
                mWfoService.registerForHandoverEvent(mHandoverStateListener);
            } catch (Exception e) {
                loge("regImsHandoverEvent(): RemoteException mWfoService()");
            }
        }
    }

    public void unregImsHandoverEvent() {
        try {
            // unregister handover event
            mWfoService.unregisterForHandoverEvent(mHandoverStateListener);
        } catch (Exception e) {
            loge("unregImsHandoverEvent: RemoteException mWfoService()");
        }
    }

    public boolean isWifcCalling(int phoneId) {
        return mPhones[phoneId].isWifiCallingEnabled();
    }

    public int getVoiceNetworkType(int phoneId) {
        if (phoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
            loge("updateCallType() invalid Phone Id!");
            return ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
        }
        return mPhones[phoneId].getServiceStateTracker().mSS.getRilVoiceRadioTechnology();
    }

    public void updateDefaultDataPhoneId(int currDataSubId, String reason) {
        int newDefaultDataPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;

        if (SubscriptionManager.isValidSubscriptionId(currDataSubId)) {
            newDefaultDataPhoneId = SubscriptionManager.getPhoneId(currDataSubId);
            if (mDefaultDataPhoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
                // First time receive event the mDefaultDataPhoneId is invalid
                mDefaultDataPhoneId = newDefaultDataPhoneId;
                logd("first time to update mDefaultDataPhoneId=" + mDefaultDataPhoneId
                        + " reason:" + reason);
                // check if reset settings is handle by framework when default sub changed.
                if (!isDefaultDataSelectedBeforeReboot()) {
                    resetTdsSettingsByFwk();
                }
            } else {
                // Default Data phone Id has chaged after bootup
                if (newDefaultDataPhoneId != mDefaultDataPhoneId) {
                    mDefaultDataPhoneId = newDefaultDataPhoneId;
                    logd("updateDefaultDataPhoneId() mDefaultDataPhoneId=" +
                            mDefaultDataPhoneId + " reason:" + reason);
                    // check if reset settings is handle by framework when default sub changed.
                    resetTdsSettingsByFwk();
                }
            }
            setDefaultDataSelectedProperty(1);
        } else {
            setDefaultDataSelectedProperty(0);
        }
    }

    public void onSrvccStateChanged() {
        logd("onSrvccStateChanged()");
        mSmartDataOpExt.onSrvccStateChanged();
    }

    public boolean onServiceStateChanged(int phoneId) {
        return mSmartDataOpExt.onServiceStateChanged(phoneId);
    }

    public void onHandoverToWifi() {
        logd("onHandoverToWifi()");

    }

    public void onHandoverToCellular() {
        logd("onHandoverToCellular()");
    }

    public void onTemporaryDataSettingsChanged() {
        logd("onTemporaryDataSettingsChanged() newSettings=" + isTemporaryDataServiceSettingOn);
        mReEvalueRegistrants.notifyRegistrants();
    }

    public void resetTdsSettingsByFwk() {
        if (isResetTdsSettingsByFwk) {
            // reset settings value of Tempoary Data Service
            Settings.Global.putInt(mResolver, TEMP_DATA_SERVICE, 0);
            logd("reset settings of Tempoary Data Service!");
        }
    }

    private boolean isDefaultDataPhoneIdValid(){
        // In AOSP, getPhoneId would returns DEFAULT_PHONE_INDEX if
        // subId can not found phoneId in sub module.
        if(mDefaultDataPhoneId == SubscriptionManager.INVALID_PHONE_INDEX ||
           mDefaultDataPhoneId == SubscriptionManager.DEFAULT_PHONE_INDEX) {
            return false;
        } else {
            return true;
        }
    }

    private ConnectivityManager getConnectivityManager() {
        if (mConnectivityManager == null) {
            mConnectivityManager = (ConnectivityManager) mContext.getSystemService(
                    Context.CONNECTIVITY_SERVICE);
        }
        return mConnectivityManager;
    }

    private TelephonyManager getTelephonyManager() {
        if (mTelephonyManager == null) {
            mTelephonyManager = (TelephonyManager) mContext.getSystemService(
                    Context.TELEPHONY_SERVICE);
        }
        return mTelephonyManager;
    }

    private IWifiOffloadService getWifiOffLoadService() {
        if (mWfoService == null) {
            IBinder binder = ServiceManager.getService(WifiOffloadManager.WFO_SERVICE);
            if (binder != null) {
                mWfoService = IWifiOffloadService.Stub.asInterface(binder);
            } else {
                binder = ServiceManager.getService(MwisConstants.MWI_SERVICE);
                try {
                    if (binder != null) {
                        mWfoService = IMwiService.Stub.asInterface(binder).getWfcHandlerInterface();
                    } else {
                        loge("getWifiOffLoadService: No MwiService exist");
                    }
                } catch (Exception e) {
                    loge("getWifiOffLoadService: can't get MwiService error:"+ e);
                }
            }

        }
        return mWfoService;
    }

    private void setDefaultDataSelectedProperty(int selected) {
        String defaultDataSelected = SystemProperties.get(PROPERTY_DEFAULT_DATA_SELECTED);
        if (!defaultDataSelected.equals(String.valueOf(selected))) {
            SystemProperties.set(PROPERTY_DEFAULT_DATA_SELECTED, String.valueOf(selected));
            logd("setDefaultDataSelectedProperty() selected=" + String.valueOf(selected));
        }
    }

    private boolean isDefaultDataSelectedBeforeReboot() {
        String defaultDataSelected = SystemProperties.get(PROPERTY_DEFAULT_DATA_SELECTED);
        logd("isDefaultDataSelectedBeforeReboot() property=" + defaultDataSelected);
        return defaultDataSelected.equals("1");
    }

    private boolean isSmartDataSwitchSupport() {
        return SystemProperties.get("persist.vendor.radio.smart.data.switch").equals("1");
    }

    public void registerReEvaluateEvent(Handler h, int what, Object obj, int phoneId) {
        if (!isSmartDataSwitchSupport()) {
            logd("registerReEvaluateEvent: not have TempDataSwitchCapability");
            return;
        }
        Registrant r = new Registrant(h, what, obj);
        logd("registerReEvaluateEvent()");
        setInCallPhoneId(phoneId);
        mReEvalueRegistrants.add(r);
        regImsHandoverEvent();
        mSmartDataOpExt.onCallStarted();
    }

    public void unregisterReEvaluateEvent(Handler h) {
        if (!isSmartDataSwitchSupport()) {
            logd("unregisterReEvaluateEvent: not have TempDataSwitchCapability");
            return;
        }
        logd("unregisterReEvaluateEvent()");
        setInCallPhoneId(SubscriptionManager.INVALID_PHONE_INDEX);
        mReEvalueRegistrants.remove(h);
        unregImsHandoverEvent();
        mSmartDataOpExt.onCallEnded();
    }

    public boolean checkIsSwitchAvailable(int phoneId) {
        if (!isSmartDataSwitchSupport()) {
            logd("checkIsSwitchAvailable: not have TempDataSwitchCapability");
            return false;
        }
        return mSmartDataOpExt.checkIsSwitchAvailable(phoneId);
    }

    public void onDsdaStateChanged(){
        logd("onDsdaStateChanged: notify");
        mReEvalueRegistrants.notifyRegistrants();
    }

    public boolean getTemporaryDataSettings(){
        return isTemporaryDataServiceSettingOn;
    }

    private void setInCallPhoneId(int phoneId) {
        mInCallPhoneId = phoneId;
    }

    public int getInCallPhoneId() {
        return mInCallPhoneId;
    }

    protected static void logv(String s) {
        if (DBG) {
            Rlog.v(LOG_TAG, s);
        }
    }

    protected static void logd(String s) {
        if (DBG) {
            Rlog.d(LOG_TAG, s);
        }
    }

    protected static void loge(String s) {
        if (DBG) {
            Rlog.e(LOG_TAG, s);
        }
    }

    protected static void logi(String s) {
        if (DBG) {
            Rlog.i(LOG_TAG, s);
        }
    }

}
