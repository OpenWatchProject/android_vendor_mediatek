package com.mediatek.systemui.op09clip;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.ColorStateList;
import android.text.TextUtils;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;
import android.widget.LinearLayout;
import android.widget.FrameLayout;
import android.widget.ImageView;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.cdma.EriInfo;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkIccCardConstants.CardType;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.systemui.op09clip.R;
import com.mediatek.systemui.ext.DefaultSystemUIStatusBarExt;
import com.mediatek.systemui.ext.ISystemUIStatusBarExt.StatusBarCallback;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import mediatek.telecom.MtkTelecomManager;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * M: Op09 implementation of Plug-in definition of Status bar for C lib.
 */
public class Op09SystemUIStatusBarExt extends DefaultSystemUIStatusBarExt {

    private static final String TAG = "Op09SystemUIStatusBarExt";
    private static final boolean DEBUG = !isUserLoad();

    private Context mContext;

    private final int mCount;
    private int mNetworkTypeIconId = 0;
    private int mDataTypeIconId = 0;
    private int mSignalStrengthIconId = 0;

    //add for log print
    private static HashMap<Integer, Boolean> mLastIsSimOffline =
            new HashMap<Integer, Boolean>();
    private static HashMap<Integer, Integer> mLastNetworkTypeIconId =
            new HashMap<Integer, Integer>();
    private static HashMap<Integer, Integer> mLastDataTypeIconId =
            new HashMap<Integer, Integer>();
    private static HashMap<Integer, Integer> mLastSignalStrengthIconId =
            new HashMap<Integer, Integer>();

    private static final String SLOT_HD_VOICE = "hdvoice";
    private static StatusBarCallback sStatusbarCallback;
    private ViewGroup mRoot = null;
    private ImageView mHdVoiceView;
    private ImageView mDataTypeView;
    private ImageView mDataActivityView;
    private ImageView mNetworkTypeView;
    private ImageView mSignalStrengthView;
    private ImageView mVolteView;
    private ServiceState mServiceState;
    private boolean mIsRoaming;
    private boolean mInCsCall;

    private static HashMap<Integer, Boolean> sIsRoamingMap =
            new HashMap<Integer, Boolean>();
    private static HashMap<Integer, Boolean> sIsDataDisabledMap =
            new HashMap<Integer, Boolean>();
    private static HashMap<Integer, ServiceState> mServiceStateMap =
            new HashMap<Integer, ServiceState>();
    private static HashMap<Integer, Integer> mCommonSignalIdMap =
            new HashMap<Integer, Integer>();
    private static boolean mSimInserted[] = null;
    private static boolean mRegisterDone = false;
    private static int mSimCount = 2;

    // Network type icons
    private final int NETWORK_TYPE_G  = R.drawable.stat_sys_network_type_g;
    private final int NETWORK_TYPE_2G = R.drawable.stat_sys_network_type_2g;
    private final int NETWORK_TYPE_1X = R.drawable.stat_sys_network_type_1x;
    private final int NETWORK_TYPE_3G = R.drawable.stat_sys_network_type_3g;
    private final int NETWORK_TYPE_4G = R.drawable.stat_sys_network_type_4g;
    // CA update
    private final int NETWORK_TYPE_4G_PLUS = R.drawable.stat_sys_network_type_4g_plus;

    // Data type icons
    private final int DATA_TYPE_2G = R.drawable.stat_sys_data_fully_connected_2g;
    private final int DATA_TYPE_1X = R.drawable.stat_sys_data_fully_connected_1x;
    private final int DATA_TYPE_3G = R.drawable.stat_sys_data_fully_connected_3g;
    private final int DATA_TYPE_4G = R.drawable.stat_sys_data_fully_connected_4g;
    private final int DATA_TYPE_4G_PLUS = R.drawable.stat_sys_data_fully_connected_4ga;

    // Data activity icons
    private final int DATA_ACTIVITY_NOT_INOUT = R.drawable.stat_sys_signal_not_inout;
    private final int DATA_ACTIVITY_IN = R.drawable.stat_sys_signal_in;
    private final int DATA_ACTIVITY_OUT = R.drawable.stat_sys_signal_out;
    private final int DATA_ACTIVITY_INOUT = R.drawable.stat_sys_signal_inout;

    // Data primary/roaming indicator icon
    private final int DATA_INDICATOR_PRIMARY =
                        R.drawable.stat_sys_data_primary_simcard;
    private final int DATA_INDICATOR_ROAMING =
                        R.drawable.stat_sys_data_fully_connected_roam;
    private final int DATA_INDICATOR_PRIMARY_ROAMING =
                        R.drawable.stat_sys_data_primary_simcard_roam;

    private final int SIGNAL_STRENGTH_NULL = R.drawable.stat_sys_signal_null;

    private static final String PROPERTY_3G_SIM = "persist.vendor.radio.simswitch";
    //private static final String MULTI_IMS_SUPPORT = "ro.mtk_multiple_ims_support";
    private static final String[] CT_NUMERIC = { "45502", "45507", "46003", "46011", "46012",
    "46013" };
    // for dis volte icon
    private static HashMap<Integer, CardType> sCardTypeMap =
            new HashMap<Integer, CardType>();
    private static HashMap<Integer, Integer> sImsRegStateMap =
            new HashMap<Integer, Integer>();
    private static HashMap<Integer, Boolean> sIsOverwfcMap =
            new HashMap<Integer, Boolean>();
    private static HashMap<Integer, Boolean> sIsImsUseEnabledMap =
            new HashMap<Integer, Boolean>();
    private final Object mLock = new Object();
    private final int mDisVolteIconId = R.drawable.stat_sys_volte_dis;

    private static HashMap<Integer, ArrayList> sUpdateDisVolteIconMap =
            new HashMap<Integer, ArrayList>();
    // HD Voice icon
    private final int mHdVoiceViewIconId = R.drawable.stat_sys_hd_voice_call;

    // Customize volte icon
    private final int VOLTE_ICON = R.drawable.stat_sys_volte_icon;

    private final int[] SIGNAL_STRENGTH_SINGLE = {
        R.drawable.stat_sys_signal_0_fully,
        R.drawable.stat_sys_signal_1_fully,
        R.drawable.stat_sys_signal_2_fully,
        R.drawable.stat_sys_signal_3_fully,
        R.drawable.stat_sys_signal_4_fully
    };

    private static final String ACTION_SHOW_DISMISS_HDVOICE_ICON
                                         = "com.android.incallui.ACTION_SHOW_DISMISS_HD_ICON";

    private static final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(ACTION_SHOW_DISMISS_HDVOICE_ICON)) {
                boolean isShowHdVoiceIcon = intent.getBooleanExtra("FLAG_KEY_VISIBILITY", false);
                Log.d(TAG,"receive ACTION_SHOW_DISMISS_HD_ICON, isShow: " + isShowHdVoiceIcon);
                updateHdVoiceIcon(isShowHdVoiceIcon);
            }
        }
    };

    private final BroadcastReceiver mImsChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)) {
                if (!isDualVolteSupport()) {
                    int newMainPhoneSubId = MtkSubscriptionManager.getSubIdUsingPhoneId(
                            getMainPhoneId());
                    boolean imsUseEnabled = ImsManager.getInstance(mContext,
                            getMainPhoneId()).isEnhanced4gLteModeSettingEnabledByUser();
                    if (!sIsImsUseEnabledMap.containsKey(newMainPhoneSubId) ||
                            (sIsImsUseEnabledMap.containsKey(newMainPhoneSubId) && imsUseEnabled !=
                             sIsImsUseEnabledMap.get(newMainPhoneSubId))) {
                        Log.d(TAG,"singal volte main phone sub id change to :" + newMainPhoneSubId
                                + ", imsUseEnabled: " + imsUseEnabled);
                        sIsImsUseEnabledMap.clear();
                        if (imsUseEnabled) {
                            sIsImsUseEnabledMap.put(newMainPhoneSubId, imsUseEnabled);
                        }

                        if (sUpdateDisVolteIconMap.containsKey(newMainPhoneSubId)) {
                            ArrayList<ImageView> mUpdateViewList =
                                    sUpdateDisVolteIconMap.get(newMainPhoneSubId);
                            for (ImageView view: mUpdateViewList) {
                                updateDisVolteView(newMainPhoneSubId, view);
                            }
                            mUpdateViewList = null;
                        }
                    }
                }
            } else if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                String imsEnabledKey = intent.getStringExtra(
                        MtkSubscriptionManager.INTENT_KEY_PROP_KEY);
                if (SubscriptionManager.ENHANCED_4G_MODE_ENABLED.equals(imsEnabledKey)) {
                    Log.d(TAG,"volte status changed from sub");
                    if (!isDualVolteSupport()) {
                        handleImsSwitchInSingleVolte();
                    } else {
                        handleImsSwitchInDualVolte();
                    }
                }
            }
        }
    };

    /**
     * Constructs a new Op09SystemUIStatusBarExt instance with Context.
     * @param context The Context object
     */
    public Op09SystemUIStatusBarExt(Context context) {
        super(context);
        mContext = context;
        mCount = TelephonyManager.getDefault().getSimCount();
        initSimInserted(mCount);

        if (!mRegisterDone) {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(ACTION_SHOW_DISMISS_HDVOICE_ICON);
            mContext.registerReceiver(mReceiver, intentFilter);

            IntentFilter imsChangeFilter = new IntentFilter();
            if (!isDualVolteSupport()) {
                imsChangeFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
            }
            imsChangeFilter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
            mContext.registerReceiver(mImsChangeReceiver, imsChangeFilter);
            mRegisterDone = true;
        }
    }

    @Override
    public void addCallback(StatusBarCallback cb) {
        sStatusbarCallback = cb;
    }

    @Override
    public String[] addSlot(String[] slots) {
        int N = slots.length;
        String[] opSlots = new String[N + 1];
        boolean found = false;
        for (int i=0; i < N; i++) {
            if (slots[i].equals("mobile")) {
                found = true;
                opSlots[i] = SLOT_HD_VOICE;
                opSlots[i+1] = slots[i];
            } else {
                if (found) {
                    opSlots[i+1] = slots[i];
                } else {
                    opSlots[i] = slots[i];
                }
            }
        }
        return opSlots;
    }

    @Override
    public boolean disableHostFunction() {
        return true;
    }

    public boolean isDualVolteSupport() {
        return MtkImsManager.isSupportMims();
    }

    private int getSubIdBySlotId(int slot) {
        int[] subIds = SubscriptionManager.getSubId(slot);
        if (subIds == null) {
            return SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        }
        return subIds[0];
    }

    private void handleImsSwitchInSingleVolte() {
        int mainPhoneSubId = MtkSubscriptionManager.getSubIdUsingPhoneId(
                getMainPhoneId());
        synchronized (mLock) {
            boolean imsUseEnabled = ImsManager.getInstance(mContext,
                    getMainPhoneId()).isEnhanced4gLteModeSettingEnabledByUser();
            if (DEBUG) {
                Log.d(TAG,"handleImsSwitchInSingleVolte in single volte, mainPhoneSubId: "
                        + mainPhoneSubId + ", imsUseEnabled: " + imsUseEnabled);
            }
            if (!imsUseEnabled) {
                sIsImsUseEnabledMap.clear();
            } else {
                sIsImsUseEnabledMap.put(mainPhoneSubId, imsUseEnabled);
            }
        }
        if (sUpdateDisVolteIconMap.containsKey(mainPhoneSubId)) {
            ArrayList<ImageView> mUpdateViewList = sUpdateDisVolteIconMap.get(mainPhoneSubId);
            for (ImageView view: mUpdateViewList) {
                updateDisVolteView(mainPhoneSubId, view);
            }
        }
    }

    private void handleImsSwitchInDualVolte() {
        if (getSimInserted(PhoneConstants.SIM_ID_1)) {
            int sub1 = MtkSubscriptionManager.getSubIdUsingPhoneId(
                    PhoneConstants.SIM_ID_1);
            if (SubscriptionManager.isValidSubscriptionId(sub1)) {
                synchronized (mLock) {
                    boolean imsUseEnabled = ImsManager.getInstance(mContext,
                            PhoneConstants.SIM_ID_1).isEnhanced4gLteModeSettingEnabledByUser();
                    if (DEBUG) {
                        Log.d(TAG,"handleImsSwitchInDualVolte in dual volte, sub1: "
                                + sub1 + ", imsUseEnabled: " + imsUseEnabled);
                    }
                    sIsImsUseEnabledMap.put(sub1, imsUseEnabled);
                }
                if (sUpdateDisVolteIconMap.containsKey(sub1)) {
                    ArrayList<ImageView> mUpdateViewList = sUpdateDisVolteIconMap.get(sub1);
                    for (ImageView view: mUpdateViewList) {
                        updateDisVolteView(sub1, view);
                    }
                }
            }
        }

        if (getSimInserted(PhoneConstants.SIM_ID_2)) {
            int sub2 = MtkSubscriptionManager.getSubIdUsingPhoneId(
                    PhoneConstants.SIM_ID_2);
            if (SubscriptionManager.isValidSubscriptionId(sub2)) {
                synchronized (mLock) {
                    boolean imsUseEnabled = ImsManager.getInstance(mContext,
                            PhoneConstants.SIM_ID_2).isEnhanced4gLteModeSettingEnabledByUser();
                    if (DEBUG) {
                        Log.d(TAG,"handleImsSwitchInDualVolte in dual volte, sub2: " + sub2 +
                                ", imsUseEnabled: " + imsUseEnabled);
                    }
                    sIsImsUseEnabledMap.put(sub2, imsUseEnabled);
                }
                if (sUpdateDisVolteIconMap.containsKey(sub2)) {
                    ArrayList<ImageView> mUpdateViewList = sUpdateDisVolteIconMap.get(sub2);
                    for (ImageView view: mUpdateViewList) {
                        updateDisVolteView(sub2, view);
                    }
                }
            }
        }
    }

    /**
    * Get the main phone id
    * @return
    */
    public static int getMainPhoneId() {
        int mainPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        String curr3GSim = SystemProperties.get(PROPERTY_3G_SIM, "1");
        if (!TextUtils.isEmpty(curr3GSim)) {
            int curr3GPhoneId = Integer.parseInt(curr3GSim);
            mainPhoneId = curr3GPhoneId - 1;
        }
        return mainPhoneId;
    }

    private void initImsRegisterState(int subId) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        boolean imsRegInfo = MtkTelephonyManagerEx.getDefault().isImsRegistered(subId);

        synchronized (mLock) {
            if (imsRegInfo) {
                sImsRegStateMap.put(subId, ServiceState.STATE_IN_SERVICE);
            } else {
                sImsRegStateMap.put(subId, ServiceState.STATE_OUT_OF_SERVICE);
            }
            sIsOverwfcMap.put(subId, false);
            if (isDualVolteSupport()) {
                if (ImsManager.getInstance(mContext, slotId).isVolteEnabledByPlatform()) {
                    boolean imsUseEnabled = ImsManager.getInstance(mContext,
                            slotId).isEnhanced4gLteModeSettingEnabledByUser();
                    sIsImsUseEnabledMap.put(subId, imsUseEnabled);
                    if (DEBUG) {
                        Log.d(TAG, "initImsRegisterState get volte subId: " + subId +
                                ",ImsUseEnabled: " + imsUseEnabled + ",imsRegInfo: " + imsRegInfo);
                    }
                }
            } else {
                if (slotId == getMainPhoneId()) {
                    boolean imsUseEnabled = ImsManager.getInstance(mContext,
                            slotId).isEnhanced4gLteModeSettingEnabledByUser();
                    if (imsUseEnabled) {
                        sIsImsUseEnabledMap.put(subId, imsUseEnabled);
                    }
                    if (DEBUG) {
                        Log.d(TAG, "initImsRegisterState get volte subId: " + subId +
                                ",ImsUseEnabled: " + imsUseEnabled + ",imsRegInfo: " + imsRegInfo);
                    }
                }
            }
        }
    }

    @Override
    public void setIconTint(ColorStateList tint) {
        if (mNetworkTypeView != null) {
            mNetworkTypeView.setImageTintList(tint);
        }
        if (mDataTypeView != null) {
            mDataTypeView.setImageTintList(tint);
        }
        if (mDataActivityView != null) {
            mDataActivityView.setImageTintList(tint);
        }
        if (mVolteView != null) {
            mVolteView.setImageTintList(tint);
        }
        if (mSignalStrengthView != null) {
            mSignalStrengthView.setImageTintList(tint);
        }
    }

    @Override
    public void getServiceStateForCustomizedView(int subId) {
        mServiceState = getServiceStateMap(subId);
    }

    @Override
    public void isDataDisabled(int subId, boolean dataDisabled) {
        sIsDataDisabledMap.put(subId, dataDisabled);
    }

    @Override
    public int getCustomizeCsState(ServiceState serviceState, int state) {
        if (serviceState != null) {
            return serviceState.getVoiceRegState();
        } else {
            return state;
        }
    }

    @Override
    public int getNetworkTypeIcon(int subId, int iconId, int networkType,
                    ServiceState serviceState) {
        if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            int newType = getNetworkTypeFromServiceState(networkType, serviceState);
            if (newType != networkType) {
                Log.d(TAG, "getNetworkTypeIcon(), revise networkType,"
                    + " subId=" + subId
                    + ", oldType=" + networkType
                    + ", newType=" + newType);
                networkType = newType;
            }
        }

        ///M: modify nw icon to 2G, because c2k cs call, md ps can not fall back. @{
        if (mInCsCall &&
                (serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE &&
                serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE &&
                serviceState.getVoiceNetworkType() == TelephonyManager.NETWORK_TYPE_1xRTT)) {
            networkType = TelephonyManager.NETWORK_TYPE_1xRTT;
            Log.d(TAG, "getNetworkTypeIcon(), in c2k cs call, modify nw icon to 2G");
        }
        /// @}

        if (!hasService(serviceState)) {
            mNetworkTypeIconId = 0;
        } else {
            mNetworkTypeIconId = getNetworkTypeIconId(networkType);
        }

        if (!mLastNetworkTypeIconId.containsKey(subId) ||
                (mLastNetworkTypeIconId.containsKey(subId)
                        && mLastNetworkTypeIconId.get(subId) != mNetworkTypeIconId)) {
            Log.d(TAG, "getNetworkTypeIcon(), subId=" + subId
                    + ", networkType=" + networkType
                    + ", iconId=" + mNetworkTypeIconId);
        }
        mLastNetworkTypeIconId.put(subId, mNetworkTypeIconId);

        return mNetworkTypeIconId;
    }

    @Override
    public int getDataTypeIcon(int subId, int iconId,
                    int dataType, int dataState, ServiceState serviceState) {
        if (iconId == 0) {
            return 0;
        }
        // when data is disabled, om data icon show x, but op do not need show.
        if (iconId != 0 &&
                (sIsDataDisabledMap.containsKey(subId) && sIsDataDisabledMap.get(subId))) {
            Log.d(TAG, "subId: " + subId + ", isDataDisabled is true, set iconId: " + iconId +
                    " to 0");
            return 0;
        }
        int newState = dataState;
        if ((serviceState != null) &&
            (serviceState.getDataRegState() != ServiceState.STATE_IN_SERVICE)) {
            newState = TelephonyManager.DATA_DISCONNECTED;
        }
        if (newState != dataState) {
            Log.d(TAG, "getDataTypeIcon(), revise dataState,"
                + " subId=" + subId
                + ", oldState=" + dataState
                + ", newState=" + newState);
            dataState = newState;
        }

        mDataTypeIconId = getDataTypeIconId(dataType, dataState);

        if (!mLastDataTypeIconId.containsKey(subId) || (mLastDataTypeIconId.containsKey(subId)
                && mLastDataTypeIconId.get(subId) != mDataTypeIconId)) {
            Log.d(TAG, "getDataTypeIcon(), subId=" + subId
                    + ", dataType=" + dataType
                    + ", dataState=" + dataState
                    + ", iconId=" + mDataTypeIconId);
        }
        mLastDataTypeIconId.put(subId, mDataTypeIconId);

        return mDataTypeIconId;
    }

    @Override
    public int getCommonSignalIconId(int subId, int mobileSignalIconId) {
        if (mCommonSignalIdMap.containsKey(subId)) {
            return mCommonSignalIdMap.get(subId);
        } else {
            return mobileSignalIconId;
        }
    }

    @Override
    public int getCustomizeSignalStrengthIcon(int subId, int iconId,
                    SignalStrength signalStrength, int networkType,
                    ServiceState serviceState) {
        mCommonSignalIdMap.put(subId, iconId);
        int newType = getNetworkTypeFromServiceState(networkType, serviceState);
        if (newType != networkType) {
            if (DEBUG) {
                Log.d(TAG, "getCustomizeSignalStrengthIcon(), revise networkType,"
                    + " subId=" + subId
                    + ", oldType=" + networkType
                    + ", newType=" + newType);
            }
            networkType = newType;
        }

        updateServiceStateMap(subId, serviceState);

        ///M: modify signal icon to 1x, because c2k cs call, md ps can not fall back. @{
        if (mInCsCall &&
                (serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE &&
                serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE &&
                serviceState.getVoiceNetworkType() == TelephonyManager.NETWORK_TYPE_1xRTT)) {
            networkType = TelephonyManager.NETWORK_TYPE_1xRTT;
            Log.d(TAG, "getCustomizeSignalStrengthIcon(), in c2k cs call, modify type to 2G");
        }
        /// @}

        if (hasService(serviceState) && signalStrength != null) {
            mSignalStrengthIconId = getSignalStrengthIconId(
                signalStrength, networkType);
        } else {
            mSignalStrengthIconId = SIGNAL_STRENGTH_NULL;
        }

        mIsRoaming  = isRoaming(serviceState, signalStrength);
        updateRoamingStatusMap(subId, mIsRoaming);

        if (!mLastSignalStrengthIconId.containsKey(subId) ||
                (mLastSignalStrengthIconId.containsKey(subId) &&
                        mLastSignalStrengthIconId.get(subId) != mSignalStrengthIconId)) {
            Log.d(TAG, "getCustomizeSignalStrengthIcon(), subId=" + subId
                    + ", networkType=" + networkType
                    + ", iconId=" + mSignalStrengthIconId
                    + ", signalStrength=" + signalStrength + ",mIsRoaming: " + mIsRoaming);
        }
        mLastSignalStrengthIconId.put(subId, mSignalStrengthIconId);

        return mSignalStrengthIconId;
    }

    @Override
    public int getCustomizeSignalStrengthLevel(int signalLevel,
            SignalStrength signalStrength, ServiceState serviceState) {
        int networkType = getNetworkTypeFromServiceState(0, serviceState);

        if (signalStrength != null) {
            switch (networkType) {
                case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                case TelephonyManager.NETWORK_TYPE_GSM:
                case TelephonyManager.NETWORK_TYPE_GPRS:
                case TelephonyManager.NETWORK_TYPE_EDGE:
                    signalLevel = signalStrength.getLevel();
                    break;

                case TelephonyManager.NETWORK_TYPE_CDMA:
                case TelephonyManager.NETWORK_TYPE_1xRTT:
                    signalLevel = signalStrength.getCdmaLevel();
                    break;

                case TelephonyManager.NETWORK_TYPE_HSDPA:
                case TelephonyManager.NETWORK_TYPE_HSUPA:
                case TelephonyManager.NETWORK_TYPE_HSPA:
                case TelephonyManager.NETWORK_TYPE_HSPAP:
                case TelephonyManager.NETWORK_TYPE_UMTS:
                    signalLevel = signalStrength.getLevel();
                    break;

                case TelephonyManager.NETWORK_TYPE_EVDO_0:
                case TelephonyManager.NETWORK_TYPE_EVDO_A:
                case TelephonyManager.NETWORK_TYPE_EVDO_B:
                case TelephonyManager.NETWORK_TYPE_EHRPD:
                    signalLevel = signalStrength.getEvdoLevel();
                    break;

                case TelephonyManager.NETWORK_TYPE_LTE:
                case TelephonyManager.NETWORK_TYPE_LTE_CA:
                    signalLevel = signalStrength.getLteLevel();
                    break;

                default:
                    break;
            }
        }

        if (DEBUG) {
            Log.d(TAG, "getCustomizeSignalStrengthLevel(), networkType=" + networkType
                + ", signalLevel=" + signalLevel);
        }

        return signalLevel;
    }

    @Override
    public void addCustomizedView(int subId,
                    Context context, ViewGroup root) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        FrameLayout dataGroup = (FrameLayout) LayoutInflater.from(mContext)
            .inflate(R.layout.mobile_data_group, null);

        mRoot = root;
        mDataTypeView = (ImageView) dataGroup.findViewById(R.id.data_type);
        mDataActivityView = (ImageView)
            dataGroup.findViewById(R.id.data_activity);

        // Add the data group view in the forefront of the view group
        root.addView(dataGroup, 1);

        initImsRegisterState(subId);
        if (DEBUG) {
            Log.d(TAG, "addCustomizedView(), subId=" + subId
                + ", slotId=" + slotId
                + ", root=" + root);
        }
    }

    @Override
    public boolean handleCallStateChanged(int subId, int state, String incomingNumber,
         ServiceState serviceState) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
         if ((state == TelephonyManager.CALL_STATE_OFFHOOK
                 || state == TelephonyManager.CALL_STATE_RINGING)
              && MtkTelephonyManagerEx.getDefault().isInCsCall(slotId)) {
             mInCsCall = true;
         } else {
             mInCsCall = false;
         }
         if (DEBUG) {
             Log.d(TAG, "handleCallStateChanged(), subId: " + subId + ",state: " + state
                     + ",mInCsCall:" + mInCsCall);
         }
         return true;
    }

    @Override
    public boolean isInCsCall() {
        return mInCsCall;
    }

    @Override
    public void SetHostViewInvisible(ImageView view) {
        view.setVisibility(View.GONE);
    }

    @Override
    public void setCustomizedNetworkTypeView(int subId,
                    int networkTypeId, ImageView networkTypeView) {
        mNetworkTypeView = networkTypeView;
        if (networkTypeView != null) {
            if (!mLastNetworkTypeIconId.containsKey(subId) ||
                    (mLastNetworkTypeIconId.containsKey(subId) &&
                            mLastNetworkTypeIconId.get(subId) != networkTypeId)) {
                if (DEBUG) {
                    Log.d(TAG, "NetworkTypeView, subId=" + subId + " networkTypeId="
                                    + networkTypeId);
                }
            }

            if (!isSimInsert(subId)) {
                networkTypeView.setVisibility(View.GONE);
                return;
            }

            if (networkTypeId != 0) {
                networkTypeView.setImageDrawable(
                    mContext.getResources().getDrawable(networkTypeId));
                networkTypeView.setVisibility(View.VISIBLE);
            } else {
                networkTypeView.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public void setCustomizedDataTypeView(int subId,
                    int dataTypeId, boolean dataIn, boolean dataOut) {
        if (mRoot != null) {
            if (!mLastDataTypeIconId.containsKey(subId) || (mLastDataTypeIconId.containsKey(subId)
                    && mLastDataTypeIconId.get(subId) != dataTypeId)) {
                if (DEBUG) {
                    //Log.d(TAG, "DataTypeView, subId=" + subId + " dataTypeId=" + dataTypeId);
                }
            }

            if (!isSimInsert(subId)) {
                mDataTypeView.setVisibility(View.GONE);
                mDataActivityView.setVisibility(View.GONE);
                return;
            }

            if (dataTypeId != 0) {
                //dataTypeId = getDataNetworkTypeId(dataTypeId);
                mDataTypeView.setImageDrawable(
                    mContext.getResources().getDrawable(dataTypeId));
                mDataTypeView.setVisibility(View.VISIBLE);

                int dataActivityId = getDataActivityIconId(dataIn, dataOut);
                if (dataActivityId != 0) {
                    mDataActivityView.setImageDrawable(
                        mContext.getResources().getDrawable(dataActivityId));
                    mDataActivityView.setVisibility(View.VISIBLE);
                } else {
                    mDataActivityView.setVisibility(View.GONE);
                }
            } else {
                mDataTypeView.setVisibility(View.GONE);
                mDataActivityView.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public void setCustomizedMobileTypeView(int subId,
                    ImageView mobileTypeView) {
        if (mobileTypeView != null) {
            if (!isSimInsert(subId)) {
                mobileTypeView.setVisibility(View.GONE);
                return;
            }

            int iconId = getMobileTypeIconId(subId);

            if (iconId != 0) {
                mobileTypeView.setImageDrawable(
                    mContext.getResources().getDrawable(iconId));
                mobileTypeView.setVisibility(View.VISIBLE);
            } else {
                mobileTypeView.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public void setImsRegInfo(int subId, int imsRegState, boolean isOverwfc,
            boolean isImsOverVoice) {
        synchronized (mLock) {
            sImsRegStateMap.put(subId, imsRegState);
            if (DEBUG) {
                Log.d(TAG, "setImsRegInfo, subId: " + subId + ",imsRegState: " + imsRegState
                        + ",isOverwfc: " + isOverwfc);
            }
            sIsOverwfcMap.put(subId, isOverwfc);
        }
    }

    private void updateCardTypeMap(int slotId) {
        MtkIccCardConstants.CardType cardType =
                MtkTelephonyManagerEx.getDefault().getCdmaCardType(slotId);
        if (!sCardTypeMap.containsKey(slotId) || (sCardTypeMap.containsKey(slotId) &&
                !sCardTypeMap.get(slotId).equals(cardType))) {
            sCardTypeMap.put(slotId, cardType);
        }
    }

    private void cacheVolteView(int subId, ImageView volteView) {
        ArrayList<ImageView> mUpdateViewList;
        if (sUpdateDisVolteIconMap.containsKey(subId)) {
            mUpdateViewList = sUpdateDisVolteIconMap.get(subId);
        } else {
            mUpdateViewList = new ArrayList<>();
        }
        if (!mUpdateViewList.contains(volteView)) {
            mUpdateViewList.add(volteView);
        }
        sUpdateDisVolteIconMap.put(subId, mUpdateViewList);
        mUpdateViewList = null;
    }

    private void updateDisVolteView(int subId, ImageView volteView) {
        ServiceState ss =getServiceStateMap(subId);
        int networkType = getNetworkTypeFromServiceState(0, ss);
        int mainPhoneSubId = MtkSubscriptionManager.getSubIdUsingPhoneId(getMainPhoneId());
        int slotId = SubscriptionManager.getSlotIndex(subId);
        synchronized (mLock) {
            if (sCardTypeMap.containsKey(slotId)) {
                CardType cardType = sCardTypeMap.get(slotId);
                // just for CT card
                if (isCTCardType(cardType) || isCtSim(subId)) {
                    if (getImsUseEnabledStatus(subId)
                            && ((networkType == TelephonyManager.NETWORK_TYPE_LTE
                            || networkType == TelephonyManager.NETWORK_TYPE_LTE_CA)
                            && (ss != null
                            && ss.getVoiceRegState() != ServiceState.STATE_IN_SERVICE))
                            && (sImsRegStateMap.containsKey(subId)
                            && (sImsRegStateMap.get(subId) != ServiceState.STATE_IN_SERVICE))
                            && (sIsOverwfcMap.containsKey(subId) && !sIsOverwfcMap.get(subId))) {
                        volteView.setImageDrawable(
                                mContext.getResources().getDrawable(mDisVolteIconId));
                        volteView.setVisibility(View.VISIBLE);
                        Log.d(TAG, "updateDisVolteView, set dis volte");
                    } else if (getImsUseEnabledStatus(subId)
                            && (networkType != TelephonyManager.NETWORK_TYPE_LTE
                            || networkType != TelephonyManager.NETWORK_TYPE_LTE_CA)
                            && (sImsRegStateMap.containsKey(subId)
                            && (sImsRegStateMap.get(subId) != ServiceState.STATE_IN_SERVICE))
                            && (sIsOverwfcMap.containsKey(subId) && !sIsOverwfcMap.get(subId))) {
                        // network type not LTE, if ims turn on, do not show dis volte
                        volteView.setVisibility(View.GONE);
                        Log.d(TAG, "network type is not LTE if ims turn on, do not show dis volte");
                    } else if (!getImsUseEnabledStatus(subId)) {
                        // CT card is primary sim but ims disable, or CT card is not primary
                        // so it's ims also disable, do not show this icon.
                        volteView.setVisibility(View.GONE);
                    }
                }
            }
        }
    }

    @Override
    public void setCustomizedVolteView(int iconId, ImageView volteView) {
        mVolteView = volteView;
        ViewGroup.MarginLayoutParams mp = new ViewGroup.MarginLayoutParams(
                ViewGroup.MarginLayoutParams.WRAP_CONTENT,
                ViewGroup.MarginLayoutParams.WRAP_CONTENT);
        mp.setMargins(0, 0, 4, 0);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(mp);
        volteView.setLayoutParams(lp);
        if (iconId != 0) {
            volteView.setImageDrawable(
                    mContext.getResources().getDrawable(VOLTE_ICON));
            volteView.setVisibility(View.VISIBLE);
        } else {
            volteView.setVisibility(View.GONE);
        }
    }

    @Override
    public void setDisVolteView(int subId, int iconId, ImageView volteView) {
        cacheVolteView(subId, volteView);
        if (iconId > 0) {
            return;
        } else {
            updateDisVolteView(subId, volteView);
        }
    }

    public static boolean isCtSim(int subId) {
        boolean ctSim = false;
        String numeric = TelephonyManager.getDefault().getSimOperator(subId);
        for (String ct : CT_NUMERIC) {
            if (ct.equals(numeric)) {
                ctSim = true;
                break;
            }
        }
        return ctSim;
    }

    private boolean isCTCardType(CardType cardType) {
        if (((MtkIccCardConstants.CardType.CT_4G_UICC_CARD).equals(cardType) ||
                (MtkIccCardConstants.CardType.CT_3G_UIM_CARD).equals(cardType) ||
                (MtkIccCardConstants.CardType.CT_UIM_SIM_CARD).equals(cardType))) {
            return true;
        } else {
            return false;
        }
    }

    private boolean getImsUseEnabledStatus(int subId) {
        if (sIsImsUseEnabledMap.containsKey(subId)) {
            return sIsImsUseEnabledMap.get(subId);
        } else {
            return false;
        }
    }

    @Override
    public void setCustomizedSignalStrengthView(int subId,
                    int signalStrengthId, ImageView signalStrengthView) {
        mSignalStrengthView = signalStrengthView;
        if (signalStrengthView != null) {
            if (!mLastSignalStrengthIconId.containsKey(subId) ||
                    (mLastSignalStrengthIconId.containsKey(subId) &&
                            mLastSignalStrengthIconId.get(subId) != signalStrengthId)) {
                if (DEBUG) {
                    Log.d(TAG, "SignalView, subId=" + subId + " signalId=" + signalStrengthId);
                }
            }

            if (!isSimInsert(subId) || isSimOffline(subId, mServiceState)) {
                signalStrengthId = SIGNAL_STRENGTH_NULL;
            }
            if (signalStrengthId != 0) {
                signalStrengthView.setImageDrawable(
                    mContext.getResources().getDrawable(signalStrengthId));
                signalStrengthView.setVisibility(View.VISIBLE);
            } else {
                signalStrengthView.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public void setSimInserted(int slotId, boolean insert) {
        updateSimInserted(slotId, insert);
        updateCardTypeMap(slotId);
    }

    public static final boolean isUserLoad() {
        return SystemProperties.get("ro.build.type").equals("user") ||
                    SystemProperties.get("ro.build.type").equals("userdebug");
    }

    private static void updateHdVoiceIcon(boolean isShowHdVoiceIcon) {
        sStatusbarCallback.setSystemIcon(SLOT_HD_VOICE, 0, SLOT_HD_VOICE, isShowHdVoiceIcon);
    }

    private static ServiceState getServiceState(int subId) {
        ServiceState serviceState = TelephonyManager.getDefault().
                getServiceStateForSubscriber(subId);
        return serviceState;
    }

    private boolean isRoaming(ServiceState serviceState, SignalStrength signalStrength) {
        if (serviceState == null) {
            return false;
        }
        if ((signalStrength != null) && !signalStrength.isGsm()) {
            final int iconMode = serviceState.getCdmaEriIconMode();
            return serviceState.getCdmaEriIconIndex() != EriInfo.ROAMING_INDICATOR_OFF
                    && (iconMode == EriInfo.ROAMING_ICON_MODE_NORMAL
                        || iconMode == EriInfo.ROAMING_ICON_MODE_FLASH);
        } else {
            return serviceState != null && serviceState.getRoaming();
        }
    }

    private static void updateRoamingStatusMap(int subId, boolean isRoaming) {
        if (!sIsRoamingMap.containsKey(subId) || isRoaming != sIsRoamingMap.get(subId)) {
            sIsRoamingMap.put(subId, isRoaming);
        }
    }

    private static boolean getRoamingStatusByMap(int subId) {
        if (sIsRoamingMap.containsKey(subId)) {
            return sIsRoamingMap.get(subId);
        } else {
            return false;
        }
    }

    private boolean isSimRadioOn(int subId) {
        ITelephony telephony = ITelephony.Stub.asInterface(ServiceManager
                .getService(Context.TELEPHONY_SERVICE));
        try {
            if (telephony != null) {
                return telephony.isRadioOnForSubscriber(
                            subId, mContext.getPackageName());
            }
        } catch (RemoteException e) {
            Log.e(TAG, "ITelephony.isRadioOnForSubscriber exception");
        }
        return false;
    }

    private static void updateServiceStateMap(int subId, ServiceState serviceState) {
        mServiceStateMap.put(subId, serviceState);
    }

    private static ServiceState getServiceStateMap(int subId) {
        ServiceState serviceState;
        if (mServiceStateMap.containsKey(subId)) {
            serviceState = mServiceStateMap.get(subId);
        } else {
            serviceState = getServiceState(subId);
            mServiceStateMap.put(subId, serviceState);
            if (DEBUG) {
                Log.d(TAG, "getServiceStateMap()");
            }
        }

        return serviceState;
    }

    private static void initSimInserted(int simCount) {
        mSimCount = simCount;
        if (mSimInserted == null) {
            Log.d(TAG, "initSimInserted() mSimCount=" + mSimCount);
            mSimInserted = new boolean[mSimCount + 1];

            for (int i = 0; i < mSimCount; i++) {
                mSimInserted[i] = TelephonyManager.getDefault().hasIccCard(i);
            }
        }
    }

    private static void updateSimInserted(int slotId, boolean insert) {
        if (slotId < mSimCount) {
            mSimInserted[slotId] = insert;
        }
    }

    private static boolean getSimInserted(int slotId) {
        if (slotId < mSimCount) {
            return mSimInserted[slotId];
        }
        return false;
    }

    private boolean isSimOffline(int subId, ServiceState serviceState) {
        boolean ret;
        boolean isRadioOn = ((serviceState != null) &&
                             (serviceState.getState() != ServiceState.STATE_POWER_OFF));
        boolean isService = hasService(serviceState);
        int slotId = SubscriptionManager.getSlotIndex(subId);

        if (slotId < 0) {
            ret = true;
        } else {
            ret = (!isRadioOn || !isService);
        }

        if (mLastIsSimOffline.containsKey(subId) && mLastIsSimOffline.get(subId) != ret) {
            Log.d(TAG, "isSimOffline: offline=" + ret
                    + ", subId=" + subId + ",slotId=" + slotId
                    + ", isService=" + isService
                    + ", serviceState=" + serviceState);
        }
        mLastIsSimOffline.put(subId, ret);

        return ret;
    }

    private boolean isSimInsert(int subId) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        boolean inserted = false;
        if (slotId >= 0) {
            inserted = getSimInserted(slotId);
        }
        return inserted;
    }

    private int getNetworkTypeIconId(int networkType) {
        int iconId = 0;

        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                break;

            case TelephonyManager.NETWORK_TYPE_GSM:
                iconId = NETWORK_TYPE_G;
                break;

            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_EDGE:
            case TelephonyManager.NETWORK_TYPE_CDMA:
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                iconId = NETWORK_TYPE_2G;
                break;

            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
            case TelephonyManager.NETWORK_TYPE_UMTS:
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                iconId = NETWORK_TYPE_3G;
                break;

            case TelephonyManager.NETWORK_TYPE_LTE:
                iconId = NETWORK_TYPE_4G;
                break;

            case TelephonyManager.NETWORK_TYPE_LTE_CA:
                iconId = NETWORK_TYPE_4G_PLUS;
                break;

            default:
                break;
        }

        return iconId;
    }

    private int getDataTypeIconId(int dataType, int dataState) {
        int iconId = 0;

        if (dataState == TelephonyManager.DATA_CONNECTED) {
            switch (dataType) {
                case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                    break;

                case TelephonyManager.NETWORK_TYPE_GSM:
                case TelephonyManager.NETWORK_TYPE_GPRS:
                case TelephonyManager.NETWORK_TYPE_EDGE:
                case TelephonyManager.NETWORK_TYPE_CDMA:
                case TelephonyManager.NETWORK_TYPE_1xRTT:
                    iconId = DATA_TYPE_2G;
                    break;

                case TelephonyManager.NETWORK_TYPE_HSDPA:
                case TelephonyManager.NETWORK_TYPE_HSUPA:
                case TelephonyManager.NETWORK_TYPE_HSPA:
                case TelephonyManager.NETWORK_TYPE_HSPAP:
                case TelephonyManager.NETWORK_TYPE_UMTS:
                case TelephonyManager.NETWORK_TYPE_EVDO_0:
                case TelephonyManager.NETWORK_TYPE_EVDO_A:
                case TelephonyManager.NETWORK_TYPE_EVDO_B:
                case TelephonyManager.NETWORK_TYPE_EHRPD:
                    iconId = DATA_TYPE_3G;
                    break;

                case TelephonyManager.NETWORK_TYPE_LTE:
                    iconId = DATA_TYPE_4G;
                    break;

                case TelephonyManager.NETWORK_TYPE_LTE_CA:
                    iconId = DATA_TYPE_4G_PLUS;
                    break;

                default:
                    break;
            }
        }

        return iconId;
    }

    private int getDataActivityIconId(boolean dataIn, boolean dataOut) {
        int iconId = 0;

        if (dataIn && dataOut) {
            iconId = DATA_ACTIVITY_INOUT;
        } else if (dataIn) {
            iconId = DATA_ACTIVITY_IN;
        } else if (dataOut) {
            iconId = DATA_ACTIVITY_OUT;
        }

        return iconId;
    }

    private int getMobileTypeIconId(int subId) {
        int iconId = 0;
        int defaultDataSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        boolean isRoaming = getRoamingStatusByMap(subId);

        if (mCount > 1 && defaultDataSubId != SubscriptionManager.INVALID_SUBSCRIPTION_ID
            && defaultDataSubId == subId) {
            if (isRoaming) {
                iconId = DATA_INDICATOR_PRIMARY_ROAMING;
            } else {
                iconId = DATA_INDICATOR_PRIMARY;
            }
        } else if (isRoaming) {
            iconId = DATA_INDICATOR_ROAMING;
        }

        return iconId;
    }

    private int getSignalStrengthIconId(
                    SignalStrength signalStrength, int networkType) {
        int level = 0;

        if (signalStrength != null) {
            switch (networkType) {
                case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                case TelephonyManager.NETWORK_TYPE_GSM:
                case TelephonyManager.NETWORK_TYPE_GPRS:
                case TelephonyManager.NETWORK_TYPE_EDGE:
                    level = signalStrength.getLevel();
                    break;

                case TelephonyManager.NETWORK_TYPE_CDMA:
                case TelephonyManager.NETWORK_TYPE_1xRTT:
                    level = signalStrength.getCdmaLevel();
                    break;

                case TelephonyManager.NETWORK_TYPE_HSDPA:
                case TelephonyManager.NETWORK_TYPE_HSUPA:
                case TelephonyManager.NETWORK_TYPE_HSPA:
                case TelephonyManager.NETWORK_TYPE_HSPAP:
                case TelephonyManager.NETWORK_TYPE_UMTS:
                    level = signalStrength.getLevel();
                    break;

                case TelephonyManager.NETWORK_TYPE_EVDO_0:
                case TelephonyManager.NETWORK_TYPE_EVDO_A:
                case TelephonyManager.NETWORK_TYPE_EVDO_B:
                case TelephonyManager.NETWORK_TYPE_EHRPD:
                    level = signalStrength.getEvdoLevel();
                    /// M: fix 3646080 @{
                    if (level == 0) {
                        level = signalStrength.getLevel();
                    }
                    /// @}
                    break;

                case TelephonyManager.NETWORK_TYPE_LTE:
                case TelephonyManager.NETWORK_TYPE_LTE_CA:
                    level = signalStrength.getLteLevel();
                    /// M: fix 3487655 @{
                    if (level == 0) {
                        level = signalStrength.getLevel();
                    }
                    /// @}
                    break;

                default:
                    break;
            }
        }

        return SIGNAL_STRENGTH_SINGLE[level];
    }

    private boolean hasService(ServiceState serviceState) {
        if (serviceState != null) {
            // Consider the device to be in service if either voice or data
            // service is available. Some SIM cards are marketed as data-only
            // and do not support voice service, and on these SIM cards, we
            // want to show signal bars for data service as well as the "no
            // service" or "emergency calls only" text that indicates that voice
            // is not available.
            switch (serviceState.getVoiceRegState()) {
                case ServiceState.STATE_POWER_OFF:
                    return false;
                case ServiceState.STATE_OUT_OF_SERVICE:
                case ServiceState.STATE_EMERGENCY_ONLY:
                    return serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE;
                default:
                    return true;
            }
        } else {
            return false;
        }
    }

    private int getNetworkTypeFromServiceState(
                    int networkType, ServiceState serviceState) {
        int type = networkType;
        if (networkType == TelephonyManager.NETWORK_TYPE_LTE_CA) {
            return type;
        }
        if (serviceState != null) {
            type = serviceState.getDataNetworkType()
                != TelephonyManager.NETWORK_TYPE_UNKNOWN
                ? serviceState.getDataNetworkType()
                : serviceState.getVoiceNetworkType();
        }
        return type;
    }

    private int getDataNetworkTypeId(int dataTypeId) {
        int type;

        if (mServiceState == null) {
            return dataTypeId;
        }

        type = mServiceState.getDataNetworkType();
        /*
        if (type == TelephonyManager.NETWORK_TYPE_LTE
               || type == TelephonyManager.NETWORK_TYPE_LTEA) {
            if (mServiceState != null) {
                type = (mServiceState.getProprietaryDataRadioTechnology() == 0 ?
                    TelephonyManager.NETWORK_TYPE_LTE : TelephonyManager.NETWORK_TYPE_LTEA);
            }
        }
        */
        if (type != TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            dataTypeId = getDataTypeIconId(type, TelephonyManager.DATA_CONNECTED);
        }

        return dataTypeId;
    }
}
