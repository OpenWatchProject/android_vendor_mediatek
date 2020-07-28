package com.mediatek.systemui.op01;

import android.content.Context;
import android.content.res.ColorStateList;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.util.Log;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.ViewGroup.MarginLayoutParams;
import android.widget.LinearLayout;
import android.widget.FrameLayout;
import android.widget.ImageView;

import com.android.internal.telephony.ITelephony;

import com.mediatek.systemui.op01.R;
import com.mediatek.systemui.ext.DefaultSystemUIStatusBarExt;
import com.mediatek.systemui.op01.PhoneStateListenerController;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * M: Op01 implementation of Plug-in definition of Status bar.
 */
public class Op01SystemUIStatusBarExt extends DefaultSystemUIStatusBarExt {

    private static final String TAG = "Op01SystemUIStatusBarExt";
    private static final boolean DEBUG = !isUserLoad();

    private Context mContext;

    private final int mCount;
    private int mSlotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
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
    private static HashMap<Integer, Integer> mCommonSignalIdMap =
            new HashMap<Integer, Integer>();

    private PhoneStateListenerController mPhoneStateController;
    private ViewGroup mRoot = null;
    private ImageView mDataTypeView;
    private ImageView mNetworkTypeView;
    private ImageView mDataActivityView;
    private ImageView mSignalStrengthView;
    private ImageView mVolteView;
    private ServiceState mServiceState;
    private static HashMap<Integer, ServiceState> mServiceStateMap =
            new HashMap<Integer, ServiceState>();

    // Network type icons
    private final int NETWORK_TYPE_G = R.drawable.stat_sys_network_type_g;
    private final int NETWORK_TYPE_E = R.drawable.stat_sys_network_type_e;
    /// for CMCC L+C
    private final int NETWORK_TYPE_2G = R.drawable.stat_sys_network_type_2g;
    private final int NETWORK_TYPE_3G = R.drawable.stat_sys_network_type_3g;
    private final int NETWORK_TYPE_4G = R.drawable.stat_sys_network_type_4g;

    // Data type icons
    private final int DATA_TYPE_G = R.drawable.stat_sys_data_fully_connected_g;
    private final int DATA_TYPE_E = R.drawable.stat_sys_data_fully_connected_e;
    /// for CMCC L+C
    private final int DATA_TYPE_2G = R.drawable.stat_sys_data_fully_connected_2g;
    private final int DATA_TYPE_3G = R.drawable.stat_sys_data_fully_connected_3g;
    private final int DATA_TYPE_4G = R.drawable.stat_sys_data_fully_connected_4g;
    private final int DATA_TYPE_4G_PLUS = R.drawable.stat_sys_data_fully_connected_4ga;

    // Data activity icons
    private final int DATA_ACTIVITY_NOT_INOUT = R.drawable.stat_sys_signal_not_inout;
    private final int DATA_ACTIVITY_IN = R.drawable.stat_sys_signal_in;
    private final int DATA_ACTIVITY_OUT = R.drawable.stat_sys_signal_out;
    private final int DATA_ACTIVITY_INOUT = R.drawable.stat_sys_signal_inout;

    // Signal strength icons
    private final int SIGNAL_STRENGTH_OFFLINE = R.drawable.stat_sys_signal_radio_off;

    // Data roaming indicator icon
    private final int DATA_ROAMING_INDICATOR = R.drawable.stat_sys_data_fully_connected_roam;

   // Customize volte icon
    private final int VOLTE_ICON = R.drawable.stat_sys_volte_icon;

    private final int[] SIGNAL_STRENGTH_SINGLE = {
        R.drawable.stat_sys_signal_0_fully,
        R.drawable.stat_sys_signal_1_fully,
        R.drawable.stat_sys_signal_2_fully,
        R.drawable.stat_sys_signal_3_fully,
        R.drawable.stat_sys_signal_4_fully
    };

    /**
     * Constructs a new Op01SystemUIStatusBarExt instance with Context.
     * @param context The Context object
     */
    public Op01SystemUIStatusBarExt(Context context) {
        super(context);
        mContext = context;
        mCount = TelephonyManager.getDefault().getSimCount();
        if (DEBUG) {
            Log.d(TAG, "mCount=" + mCount);
        }
        mPhoneStateController = PhoneStateListenerController.getInstance(context);
    }

    @Override
    public boolean disableHostFunction() {
        return true;
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
    public int getNetworkTypeIcon(int subId, int iconId, int networkType,
                    ServiceState serviceState) {
        int newType = getNetworkTypeFromServiceState(networkType, serviceState);
        if (newType != networkType) {
            Log.d(TAG, "getNetworkTypeIcon(), revise networkType,"
                + " subId=" + subId
                + ", oldType=" + networkType
                + ", newType=" + newType);
            networkType = newType;
        }

        if (isSimOffline(subId, serviceState) ||
                !hasService(serviceState)) {
            mNetworkTypeIconId = 0;
        } else {
            mNetworkTypeIconId = getNetworkTypeIconId(networkType);
        }

        if (!mLastNetworkTypeIconId.containsKey(subId) ||
                (mLastNetworkTypeIconId.containsKey(subId)
                        && mLastNetworkTypeIconId.get(subId) != mNetworkTypeIconId)) {
            Log.d(TAG, "getNetworkTypeIcon(), subId=" + subId
                    + ", networkType=" + networkType
                    + ", iconId=" + mNetworkTypeIconId
                    + ", serviceState=" + serviceState);
        }
        mLastNetworkTypeIconId.put(subId, mNetworkTypeIconId);

        return mNetworkTypeIconId;
    }

    @Override
    public int getDataTypeIcon(int subId, int iconId,
                    int dataType, int dataState, ServiceState serviceState) {
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
        updateServiceStateMap(subId, serviceState);
        if (isSimOffline(subId, serviceState) ||
                !hasService(serviceState)) {
            mSignalStrengthIconId = SIGNAL_STRENGTH_OFFLINE;
        } else {
            mSignalStrengthIconId = getSignalStrengthIconId(signalStrength);
        }

        if (!mLastSignalStrengthIconId.containsKey(subId) ||
                (mLastSignalStrengthIconId.containsKey(subId) &&
                        mLastSignalStrengthIconId.get(subId) != mSignalStrengthIconId)) {
            Log.d(TAG, "getCustomizeSignalStrengthIcon(), subId=" + subId
                    + ", networkType=" + networkType
                    + ", iconId=" + mSignalStrengthIconId
                    + ", signalStrength=" + signalStrength);
        }
        mLastSignalStrengthIconId.put(subId, mSignalStrengthIconId);

        return mSignalStrengthIconId;
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

        mPhoneStateController.addDataActivityView(subId, mDataActivityView);
        // Add the data group view in the forefront of the view group
        root.addView(dataGroup, 1);

        if (DEBUG) {
            Log.d(TAG, "addCustomizedView(), subId=" + subId
                + ", slotId=" + slotId
                + ", root=" + root);
        }
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
            if (networkTypeId != 0) {
                // Disable the data view first
                if (mRoot != null) {
                    mDataTypeView.setVisibility(View.GONE);
                    mDataActivityView.setVisibility(View.GONE);
                }

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
            if (dataTypeId != 0) {
                mDataTypeView.setImageDrawable(
                    mContext.getResources().getDrawable(dataTypeId));
                mDataTypeView.setVisibility(View.VISIBLE);

                int dataActivityId = getDataActivityIconId(dataIn, dataOut);
                mDataActivityView.setImageDrawable(
                    mContext.getResources().getDrawable(dataActivityId));
                /// APN type not show data activity icon
                if (mPhoneStateController.isShowDataActyIconByIntent(subId)) {
                    mDataActivityView.setVisibility(View.VISIBLE);
                } else {
                    mDataActivityView.setVisibility(View.GONE);
                }
                mNetworkTypeView.setVisibility(View.GONE);
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
            boolean isOffline = isSimOffline(subId, mServiceState);
            boolean isRoaming = isSimRoaming(subId, mServiceState);

            if (!isOffline && isRoaming && hasService(mServiceState)) {
                mobileTypeView.setImageDrawable(
                    mContext.getResources().getDrawable(DATA_ROAMING_INDICATOR));
                mobileTypeView.setVisibility(View.VISIBLE);
            } else {
                mobileTypeView.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public void setCustomizedSignalStrengthView(int subId,
                    int signalStrengthId, ImageView signalStrengthView) {
        mSignalStrengthView = signalStrengthView;
        if (signalStrengthView != null) {
            if (signalStrengthId != 0) {
                signalStrengthView.setImageDrawable(
                    mContext.getResources().getDrawable(signalStrengthId));
                signalStrengthView.setVisibility(View.VISIBLE);
                signalStrengthView.setPaddingRelative(0, 0, 0, 0);
            } else {
                signalStrengthView.setVisibility(View.GONE);
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
    public void registerOpStateListener() {
        //mPhoneStateController.registerStateListener();
    }

    public static final boolean isUserLoad() {
        return SystemProperties.get("ro.build.type").equals("user") ||
                    SystemProperties.get("ro.build.type").equals("userdebug");
    }

    public static final boolean isMtkSupport(String type) {
        return SystemProperties.get(type).equals("1");
    }

    private static ServiceState getServiceState(int subId) {
        ServiceState serviceState = TelephonyManager.getDefault().
                getServiceStateForSubscriber(subId);
        return serviceState;
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

    private boolean isSimRoaming(int subId, ServiceState serviceState) {
        boolean isRoaming = false;

        if (serviceState != null) {
            isRoaming = serviceState.getRoaming();
        }

        if (DEBUG) {
            Log.d(TAG, "isSimRoaming: roaming=" + isRoaming
                + ", subId=" + subId
                + ", serviceState=" + serviceState);
        }

        return isRoaming;
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
        }

        return serviceState;
    }

    private boolean isSimOffline(int subId, ServiceState serviceState) {
        boolean ret;

        boolean isService = hasService(serviceState);
        int slotId = SubscriptionManager.getSlotIndex(subId);

        if (slotId < 0) {
            ret = true;
        } else {
            ret = !isService;
        }

        if (mLastIsSimOffline.containsKey(subId) && mLastIsSimOffline.get(subId) != ret) {
            if (DEBUG) {
                Log.d(TAG, "isSimOffline: offline=" + ret
                        + ", subId=" + subId + ",slotId=" + slotId
                        + ", isService=" + isService
                        + ", serviceState=" + serviceState);
            }
        }
        mLastIsSimOffline.put(subId, ret);

        return ret;
    }

    private int getNetworkTypeIconId(int networkType) {
        int iconId = 0;

        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                break;

            case TelephonyManager.NETWORK_TYPE_GSM:
            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_CDMA:
                iconId = NETWORK_TYPE_G;
                break;

            case TelephonyManager.NETWORK_TYPE_1xRTT:
                iconId = NETWORK_TYPE_2G;
                break;

            case TelephonyManager.NETWORK_TYPE_EDGE:
                iconId = NETWORK_TYPE_E;
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
            case TelephonyManager.NETWORK_TYPE_LTE_CA:
                iconId = NETWORK_TYPE_4G;
                break;

            default:
                break;
        }

        return iconId;
    }

    private int getDataTypeIconId(int dataType, int dataState) {
        int iconId = 0;

        if (dataState != TelephonyManager.DATA_CONNECTED) {
            return iconId;
        }

        switch (dataType) {
            case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                break;

            case TelephonyManager.NETWORK_TYPE_GSM:
            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_CDMA:
                iconId = DATA_TYPE_G;
                break;

            case TelephonyManager.NETWORK_TYPE_1xRTT:
                iconId = DATA_TYPE_2G;
                break;

            case TelephonyManager.NETWORK_TYPE_EDGE:
                iconId = DATA_TYPE_E;
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

        return iconId;
    }

    private int getDataActivityIconId(boolean dataIn, boolean dataOut) {
        int iconId = DATA_ACTIVITY_NOT_INOUT;

        if (dataIn && dataOut) {
            iconId = DATA_ACTIVITY_INOUT;
        } else if (dataIn) {
            iconId = DATA_ACTIVITY_IN;
        } else if (dataOut) {
            iconId = DATA_ACTIVITY_OUT;
        }

        return iconId;
    }

    private int getSignalStrengthIconId(SignalStrength signalStrength) {
        int level = (signalStrength != null ? signalStrength.getLevel() : 0);

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
            type = serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE &&
                    serviceState.getDataNetworkType() != TelephonyManager.NETWORK_TYPE_UNKNOWN
                ? serviceState.getDataNetworkType()
                : serviceState.getVoiceNetworkType();
        }
        return type;
    }

}
