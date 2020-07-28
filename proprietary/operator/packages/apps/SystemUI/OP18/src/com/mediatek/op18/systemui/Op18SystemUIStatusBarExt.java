package com.mediatek.op18.systemui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.ColorStateList;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.TelephonyIntents;

//import com.mediatek.internal.telephony.ITelephonyEx;
import com.mediatek.op18.systemui.R;
import com.mediatek.systemui.ext.DefaultSystemUIStatusBarExt;

import java.util.HashMap;

/**
 * M: Op18 implementation of Plug-in definition of Status bar.
 */
public class Op18SystemUIStatusBarExt extends DefaultSystemUIStatusBarExt {

    private static final String TAG = "Op18SystemUIStatusBarExt";
    private static final boolean DEBUG = false; //!isUserLoad();
    private static final int POSITION = 2;

    private Context mContext;

    private final int mCount;
    private int mSlotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
    private int mSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    private int mNetworkTypeIconId = 0;
    private int mDataTypeIconId = 0;
    private int mSignalStrengthIconId = 0;
    private static int sImsSlotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
    private static HashMap<Integer, ServiceState> mServiceStateMap =
            new HashMap<Integer, ServiceState>();
    private static boolean mSimInserted[] = null;
    private static int mSimCount = 2;

    private PhoneStateListenerController mPhoneStateController;
    private ViewGroup mRoot = null;
    private ImageView mDataTypeView;
    private ImageView mNetworkTypeView;
    private ImageView mDataActivityView;
    private ServiceState mServiceState;
    private static Context sInstance = null;


    // Data type icons
    private final int DATA_TYPE_G = R.drawable.stat_sys_data_fully_connected_g;
    private final int DATA_TYPE_E = R.drawable.stat_sys_data_fully_connected_e;
    private final int DATA_TYPE_H = R.drawable.stat_sys_data_fully_connected_h;
    private final int DATA_TYPE_3G = R.drawable.stat_sys_data_fully_connected_3g;
    private final int DATA_TYPE_4G = R.drawable.stat_sys_data_fully_connected_lte;
    private final int DATA_TYPE_LTE_PLUS = R.drawable.stat_sys_data_fully_connected_ltea;


    // Data activity icons
    private final int DATA_ACTIVITY_NOT_INOUT = R.drawable.stat_sys_signal_not_inout;
    private final int DATA_ACTIVITY_IN = R.drawable.stat_sys_signal_in;
    private final int DATA_ACTIVITY_OUT = R.drawable.stat_sys_signal_out;
    private final int DATA_ACTIVITY_INOUT = R.drawable.stat_sys_signal_inout;

    // Data roaming indicator icon
    private final int DATA_ROAMING_INDICATOR = R.drawable.stat_sys_data_fully_connected_roam;
    //

    // Clock 12hr format style
    private static final int AM_PM_STYLE_NORMAL  = 0;

    //StatusBarManager mStatusBarManager;

    /**
     * Constructs a new Op18SystemUIStatusBarExt instance with Context.
     * @param context The Context object
     */
    public Op18SystemUIStatusBarExt(Context context) {
        super(context);
        mContext = context;
        mCount = TelephonyManager.getDefault().getSimCount();
        if (DEBUG) {
            Log.d(TAG, "mCount=" + mCount);
        }
        initSimInserted(mCount);
        mPhoneStateController = PhoneStateListenerController.getInstance(context);
    }

    @Override
    public void addCustomizedView(int subId,
                    Context context, ViewGroup root) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        LinearLayout dataVolteGroup = (LinearLayout) LayoutInflater.from(mContext)
            .inflate(R.layout.mobile_data_volte_group, null);
        FrameLayout dataGroup = (FrameLayout) LayoutInflater.from(mContext)
            .inflate(R.layout.mobile_data_group, null);
        mRoot = root;
        mDataTypeView = (ImageView) dataGroup.findViewById(R.id.data_type);
        mDataActivityView = (ImageView)
            dataGroup.findViewById(R.id.data_activity);
        mPhoneStateController.addDataActivityView(subId, mDataActivityView);

        dataVolteGroup.addView(dataGroup, POSITION);
        root.addView(dataVolteGroup, 0);
    }

    @Override
    public boolean disableHostFunction() {
        //Log.d(TAG, "disable host functions");
        return true;
    }

    @Override
    public void SetHostViewInvisible(ImageView view) {
        Log.d(TAG, "SetHostViewInvisible view = "+view);
        view.setVisibility(View.GONE);
    }

    @Override
    public void setCustomizedDataTypeView(int subId,
                    int dataTypeId, boolean dataIn, boolean dataOut) {
        if (mRoot != null) {
            if(DEBUG) {
                Log.d(TAG, "DataTypeView, subId=" + subId + " dataTypeId=" + dataTypeId);
            }

           if (!isSimInsert(subId)) {
                mDataTypeView.setVisibility(View.GONE);
                mDataActivityView.setVisibility(View.GONE);
                if(DEBUG) {
                    Log.d(TAG, "data view not show");
                }
                return;
            }

            if (mNetworkTypeIconId != 0 && mNetworkTypeView != null) {
                if(DEBUG) {
                    Log.d(TAG, "removing host app nw view");
                }
                mNetworkTypeView.setVisibility(View.GONE);
            }

            ConnectivityManager cm =
                    (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo wifi = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
            if (DEBUG) {
                Log.d(TAG, "wifi connected:" + wifi.isConnected());
            }

            // ALPS02862723: Hide data icon when wifi connected(even if data pdn connected).
            if (dataTypeId != 0 && !wifi.isConnected()) {
                mDataTypeView.setImageDrawable(
                    mContext.getResources().getDrawable(dataTypeId));
                mDataTypeView.setVisibility(View.VISIBLE);

                int dataActivityId = getDataActivityIconId(dataIn, dataOut);
                mDataActivityView.setImageDrawable(
                    mContext.getResources().getDrawable(dataActivityId));
                /// APN type not show data activity icon
                if (mPhoneStateController.isShowDataActyIconByIntent(subId)) {
                    mDataActivityView.setVisibility(View.VISIBLE);
                    mDataTypeView.setVisibility(View.VISIBLE);
                } else {
                    mDataActivityView.setVisibility(View.GONE);
                    mDataTypeView.setVisibility(View.GONE);
                }
                //mNetworkTypeView.setVisibility(View.GONE);
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
    public void setCustomizedNetworkTypeView(int subId,
                    int networkTypeId, ImageView networkTypeView) {
        mNetworkTypeView = networkTypeView;
        mNetworkTypeIconId = networkTypeId;
    }

    @Override
    public int getDataTypeIcon(int subId, int iconId,
                    int dataType, int dataState, ServiceState serviceState) {
        mDataTypeIconId = getDataTypeIconId(dataType, dataState);

        if (DEBUG) {
            Log.d(TAG, "getDataTypeIcon(), subId=" + subId
                + ", dataType=" + dataType
                + ", dataState=" + dataState
                + ", iconId=" + mDataTypeIconId);
        }

        return mDataTypeIconId;
    }

    @Override
    public void getServiceStateForCustomizedView(int subId) {
        //mServiceState = getServiceState(subId);
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
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                iconId = DATA_TYPE_G;
                break;

            case TelephonyManager.NETWORK_TYPE_EDGE:
                iconId = DATA_TYPE_E;
                break;

            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                iconId = DATA_TYPE_H;
                break;

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
                iconId = DATA_TYPE_LTE_PLUS;
                break;

            default:
                break;
        }

        return iconId;
    }

    @Override
    public void registerOpStateListener() {
        //mPhoneStateController.registerStateListener();
    }

    @Override
    public void setImsSlotId(final int slotId) {
        sImsSlotId = slotId;
        //Log.d(TAG, "setSlotId =" + slotId);
    }

    @Override
    public void setSimInserted(int slotId, boolean insert) {
        updateSimInserted(slotId, insert);
    }

    /**
     * Customize style to display 12 hr format am/pm text .
     * NORMAL = 0, SMALL = 1, GONE = 2.
     * @param defaultAmPmStyle default style being used by host app.
     * @return customized style
     */
    @Override
    public int getClockAmPmStyle(int defaultAmPmStyle) {
        return AM_PM_STYLE_NORMAL;
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

    private boolean isSimOffline(int subId, ServiceState serviceState) {
        boolean ret;

        boolean isEmergencyOnly =
            serviceState != null ? serviceState.isEmergencyOnly() : false;
        boolean isRadioOn = isSimRadioOn(subId);

        if (isEmergencyOnly) {
            ret = true;
        } else {
            ret = !(isRadioOn || (serviceState != null ?
                    serviceState.getDataRegState() != ServiceState.STATE_POWER_OFF
                    : false));
        }

        if (DEBUG) {
            Log.d(TAG, "isSimOffline: offline=" + ret
                + ", subId=" + subId + ",isRadioOn: " + isRadioOn
                + ", serviceState=" + serviceState);
        }

        return ret;
    }

    private boolean isSimInsert(int subId) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        boolean inserted = false;
        if (slotId >= 0) {
            inserted = getSimInserted(slotId);
        }
        if (DEBUG) {
            Log.d(TAG, "isSimInsert: slotId=" + slotId + " inserted=" + inserted);
        }

        return inserted;
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

    private static boolean getSimInserted(int slotId) {
        if (DEBUG) {
            Log.d(TAG, "getSimInserted() slotId=" + slotId + " mSimCount=" + mSimCount);
        }

        if (slotId < mSimCount) {
            return mSimInserted[slotId];
        }

        return false;
    }

/*    private static void updateServiceStateMap(int subId, ServiceState serviceState) {
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

    private static ServiceState getServiceState(int subId) {
        ServiceState serviceState = null;
        Bundle bd = null;

        try {
            ITelephonyEx phoneEx = ITelephonyEx.Stub.asInterface(
                    ServiceManager.checkService("phoneEx"));

            if (phoneEx != null) {
                bd = phoneEx.getServiceState(subId);
                if (bd != null) {
                    serviceState = ServiceState.newFromBundle(bd);
                }
            }
        } catch (RemoteException e) {
            Log.e(TAG, "getServiceState error e:" + e.getMessage());
        }

        return serviceState;
    }*/

    private static void updateSimInserted(int slotId, boolean insert) {
        if (DEBUG) {
            Log.d(TAG, "updateSimInserted() slotId=" + slotId + " insert=" + insert);
        }
        if (slotId < mSimCount) {
            mSimInserted[slotId] = insert;
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
    }

    private void setTint(ImageView v, int tint) {
        v.setImageTintList(ColorStateList.valueOf(tint));
    }
}
