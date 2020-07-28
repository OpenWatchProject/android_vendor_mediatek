package com.mediatek.systemui.op02;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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

import com.mediatek.systemui.op02.R;
import com.mediatek.systemui.ext.DefaultSystemUIStatusBarExt;
import com.mediatek.systemui.ext.ISystemUIStatusBarExt.StatusBarCallback;
import com.mediatek.systemui.op02.PhoneStateListenerController;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * M: Op02 implementation of Plug-in definition of Status bar.
 */
public class Op02SystemUIStatusBarExt extends DefaultSystemUIStatusBarExt {

    private static final String TAG = "Op02SystemUIStatusBarExt";
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

    private static final String SLOT_HD_VOICE = "hdvoice";
    private static final String SLOT_NO_SIM = "nosim";
    private static StatusBarCallback sStatusbarCallback;
    private PhoneStateListenerController mPhoneStateController;
    private ViewGroup mRoot = null;
    private ImageView mHdVoiceView;
    private ImageView mDataTypeView;
    private ImageView mDataActivityView;
    private ImageView mNetworkTypeView;
    private ImageView mSignalStrengthView;
    /// google default nosim view
    private ImageView mNoSim;
    private NoSimView mNoSimView;
    private ImageView mVolteView;
    private ServiceState mServiceState;
    private boolean mNoSimChange = false;
    private View mNoSimsCombo = null;
    private static boolean mSetNoSimsComboGone = false;
    private static boolean mRegisterDone = false;
    private static HashMap<Integer, ServiceState> mServiceStateMap =
            new HashMap<Integer, ServiceState>();
    private static HashMap<Integer, Integer> mSlotIdMap =
            new HashMap<Integer, Integer>();
    private static boolean mSimInserted[] = null;
    private static int mSimCount = 2;

    // Network type icons
    private final int NETWORK_TYPE_G = R.drawable.stat_sys_network_type_g;
    private final int NETWORK_TYPE_3G = R.drawable.stat_sys_network_type_3g;
    private final int NETWORK_TYPE_4G = R.drawable.stat_sys_network_type_4g;

    // Data type icons
    private final int DATA_TYPE_G = R.drawable.stat_sys_data_fully_connected_g;
    private final int DATA_TYPE_E = R.drawable.stat_sys_data_fully_connected_e;
    private final int DATA_TYPE_H = R.drawable.stat_sys_data_fully_connected_h;
    private final int DATA_TYPE_H_PLUS = R.drawable.stat_sys_data_fully_connected_h_plus;
    private final int DATA_TYPE_3G = R.drawable.stat_sys_data_fully_connected_3g;
    private final int DATA_TYPE_4G = R.drawable.stat_sys_data_fully_connected_4g;
    private final int DATA_TYPE_4G_PLUS = R.drawable.stat_sys_data_fully_connected_4ga;

    // Data activity icons
    private final int DATA_ACTIVITY_NOT_INOUT = R.drawable.stat_sys_signal_not_inout;
    private final int DATA_ACTIVITY_IN = R.drawable.stat_sys_signal_in;
    private final int DATA_ACTIVITY_OUT = R.drawable.stat_sys_signal_out;
    private final int DATA_ACTIVITY_INOUT = R.drawable.stat_sys_signal_inout;

    // Signal strength icons
    private final int SIGNAL_STRENGTH_NULL_ONE_SIM = R.drawable.stat_sys_signal_null_one_sim;
    private final int SIGNAL_STRENGTH_NULL_TWO_SIMS = R.drawable.stat_sys_signal_null_two_sims;
    private final int SIGNAL_STRENGTH_NULL_SIM1 = R.drawable.stat_sys_signal_null_sim1;
    private final int SIGNAL_STRENGTH_NULL_SIM2 = R.drawable.stat_sys_signal_null_sim2;

    // Data roaming indicator icon
    private final int DATA_ROAMING_INDICATOR = R.drawable.stat_sys_data_fully_connected_roam;

    // HD Voice icon
    private final int mHdVoiceViewIconId = R.drawable.stat_sys_hd_voice_call;
    // Customize volte icon
    private final int VOLTE_ICON = R.drawable.stat_sys_volte_icon;

    private final int[] SIGNAL_STRENGTH_SINGLE_SS_LOAD = {
            R.drawable.stat_sys_signal_0_fully,
            R.drawable.stat_sys_signal_1_fully,
            R.drawable.stat_sys_signal_2_fully,
            R.drawable.stat_sys_signal_3_fully,
            R.drawable.stat_sys_signal_4_fully
    };

    private final int[][] SIGNAL_STRENGTH_SINGLE = {
        {
            R.drawable.stat_sys_signal_0_fully_sim1,
            R.drawable.stat_sys_signal_1_fully_sim1,
            R.drawable.stat_sys_signal_2_fully_sim1,
            R.drawable.stat_sys_signal_3_fully_sim1,
            R.drawable.stat_sys_signal_4_fully_sim1
        },
        {
            R.drawable.stat_sys_signal_0_fully_sim2,
            R.drawable.stat_sys_signal_1_fully_sim2,
            R.drawable.stat_sys_signal_2_fully_sim2,
            R.drawable.stat_sys_signal_3_fully_sim2,
            R.drawable.stat_sys_signal_4_fully_sim2
        }
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

    /**
     * Constructs a new Op02SystemUIStatusBarExt instance with Context.
     * @param context The Context object
     */
    public Op02SystemUIStatusBarExt(Context context) {
        super(context);
        mContext = context;
        mCount = TelephonyManager.getDefault().getSimCount();
        if (DEBUG) {
            Log.d(TAG, "mCount=" + mCount);
        }

        mPhoneStateController = PhoneStateListenerController.getInstance(context);
        initSimInserted(mCount);

        if (!mRegisterDone) {
            IntentFilter intentFilter =  new IntentFilter(ACTION_SHOW_DISMISS_HDVOICE_ICON);
            mContext.registerReceiver(mReceiver, intentFilter,
                    "com.mediatek.permission.BROADCAST_HD_INFO", null);
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
        String[] opSlots = new String[N + 2];
        boolean found = false;
        for (int i=0; i < N; i++) {
            if (slots[i].equals("mobile")) {
                found = true;
                opSlots[i] = SLOT_HD_VOICE;
                opSlots[i+1] = slots[i];
                opSlots[i+2] = SLOT_NO_SIM;
            } else {
                if (found) {
                    opSlots[i+2] = slots[i];
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
        if (mNoSimView != null && mNoSimView.mNoSimView != null) {
            mNoSimView.mNoSimView.setImageTintList(tint);
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

        mNetworkTypeIconId = getNetworkTypeIconId(networkType);

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
        int slotId = SubscriptionManager.getSlotIndex(subId);
        mSignalStrengthIconId = getSignalStrengthIconId(signalStrength, slotId);

        if (!mLastSignalStrengthIconId.containsKey(subId) ||
                (mLastSignalStrengthIconId.containsKey(subId) &&
                        mLastSignalStrengthIconId.get(subId) != mSignalStrengthIconId)) {
            Log.d(TAG, "getCustomizeSignalStrengthIcon(), subId=" + subId
                    + ", networkType=" + networkType
                    + ", iconId=" + mSignalStrengthIconId
                    + ", signalStrength=" + signalStrength);
        }
        mLastSignalStrengthIconId.put(subId, mSignalStrengthIconId);

        updateServiceStateMap(subId, serviceState);

        return mSignalStrengthIconId;
    }

    @Override
    public void addCustomizedView(int subId,
                    Context context, ViewGroup root) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        mSlotIdMap.put(subId, slotId);
        FrameLayout dataGroup = (FrameLayout) LayoutInflater.from(mContext)
            .inflate(R.layout.mobile_data_group, null);

        mRoot = root;
        mDataTypeView = (ImageView) dataGroup.findViewById(R.id.data_type);
        mDataActivityView = (ImageView)
            dataGroup.findViewById(R.id.data_activity);

        mPhoneStateController.addDataActivityView(subId, mDataActivityView);
        // Add the data group view in the forefront of the view group
        root.addView(dataGroup, 1);

        // Add the no sim view for another slot
        if (mCount == 2) {
            mNoSimView = new NoSimView(mContext);
            if (slotId == 0) {
                mNoSimView.mSlotId = 1;
                mNoSimView.mNoSimView.setImageDrawable(
                    mContext.getResources().getDrawable(
                        SIGNAL_STRENGTH_NULL_SIM2));
                root.addView(mNoSimView.mNoSimView);
            } else if (slotId == 1) {
                mNoSimView.mSlotId = 0;
                mNoSimView.mNoSimView.setImageDrawable(
                    mContext.getResources().getDrawable(
                        SIGNAL_STRENGTH_NULL_SIM1));
                root.addView(mNoSimView.mNoSimView, 0);
            }
        }

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
            if (!mLastNetworkTypeIconId.containsKey(subId) ||
                    (mLastNetworkTypeIconId.containsKey(subId) &&
                            mLastNetworkTypeIconId.get(subId) != networkTypeId)) {
                if (DEBUG) {
                    Log.d(TAG, "NetworkTypeView, subId=" + subId
                            + " networkTypeId=" + networkTypeId);
                }
            }

            if (isSimOffline(subId, mServiceState) || !isSimInsert(subId)) {
                networkTypeView.setVisibility(View.GONE);
                return;
            }

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
            if (!mLastDataTypeIconId.containsKey(subId) || (mLastDataTypeIconId.containsKey(subId)
                    && mLastDataTypeIconId.get(subId) != dataTypeId)) {
                if (DEBUG) {
                    Log.d(TAG, "DataTypeView, subId=" + subId + " dataTypeId=" + dataTypeId);
                }
            }

            if (isSimOffline(subId, mServiceState) || !isSimInsert(subId)) {
                mDataTypeView.setVisibility(View.GONE);
                mDataActivityView.setVisibility(View.GONE);
                return;
            }

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

            if (!isOffline && isRoaming) {
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
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (DEBUG) {
            Log.d(TAG, "SignalView, mNoSimChange=" + mNoSimChange);
        }
        if ((mNoSimChange == true) && (mCount == 2)) {
            if (slotId == 0) {
                mNoSimView.mNoSimView.setImageDrawable(
                    mContext.getResources().getDrawable(
                        SIGNAL_STRENGTH_NULL_SIM2));
            } else if (slotId == 1) {
                mNoSimView.mNoSimView.setImageDrawable(
                    mContext.getResources().getDrawable(
                        SIGNAL_STRENGTH_NULL_SIM1));
            }
            mNoSimChange = false;
        }

        if (signalStrengthView != null) {
            if (!mLastSignalStrengthIconId.containsKey(subId) ||
                    (mLastSignalStrengthIconId.containsKey(subId) &&
                            mLastSignalStrengthIconId.get(subId) != signalStrengthId)) {
                if (DEBUG) {
                    Log.d(TAG, "SignalView, subId=" + subId + " signalId=" + signalStrengthId);
                }
            }

            if (!isSimInsert(subId)) {
                signalStrengthView.setVisibility(View.GONE);
                return;
            }
            if (isSimOffline(subId, mServiceState)) {
                if (slotId == 0) {
                    signalStrengthId = SIGNAL_STRENGTH_NULL_SIM1;
                } else if (slotId == 1) {
                    signalStrengthId = SIGNAL_STRENGTH_NULL_SIM2;
                } else {
                    signalStrengthId = SIGNAL_STRENGTH_NULL_ONE_SIM;
                }
            }

            if (signalStrengthId != 0) {
                //set mNoSimsCombo gone when any sim insert
                if (mNoSimsCombo != null) {
                    mNoSimsCombo.setVisibility(View.GONE);
                    mSetNoSimsComboGone = true;
                }
                signalStrengthView.setImageDrawable(
                    mContext.getResources().getDrawable(signalStrengthId));
                signalStrengthView.setVisibility(View.VISIBLE);
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
    public void setCustomizedView(int subId) {
        if (mNoSimView != null) {
            // Show the no sim for other slot when needed
            if (SubscriptionManager.isValidSlotIndex(mNoSimView.mSlotId)) {
                boolean inserted = getSimInserted(mNoSimView.mSlotId);

                if (!inserted) {
                    if (mCount == 2) {
                        int slodId = 0;
                        if (mNoSimView.mSlotId == 0) {
                            slodId = 1;
                        }
                        boolean hasCard = getSimInserted(slodId);
                        if (!hasCard) {
                            if (mNoSimView.mSlotId == 0) {
                                mNoSimView.mNoSimView.setImageDrawable(
                                    mContext.getResources().getDrawable(
                                        SIGNAL_STRENGTH_NULL_SIM2));
                            } else if (mNoSimView.mSlotId == 1) {
                                mNoSimView.mNoSimView.setImageDrawable(
                                    mContext.getResources().getDrawable(
                                        SIGNAL_STRENGTH_NULL_SIM1));
                            }
                            mNoSimChange = true;
                        }
                        if (DEBUG) {
                            Log.d(TAG, "setCustomizedView(), slodId=" + slodId
                                + " ,mSlotId=" + mNoSimView.mSlotId + ", hasCard=" + hasCard);
                        }
                    }

                    if (mNoSimsCombo != null) {
                        mNoSimsCombo.setVisibility(View.GONE);
                        mSetNoSimsComboGone = true;
                    }
                    mNoSimView.mNoSimView.setVisibility(View.VISIBLE);

                    if (DEBUG) {
                        Log.d(TAG, "setCustomizedView(), subId=" + subId
                            + ", another SIM slotId=" + mNoSimView.mSlotId
                            + ", inserted=" + inserted);
                    }
                } else {
                    mNoSimView.mNoSimView.setVisibility(View.GONE);
                }
            } else {
                mNoSimView.mNoSimView.setVisibility(View.GONE);
            }
        }
    }

    @Override
    public void setCustomizedNoSimView(boolean noSimsVisible) {
        sStatusbarCallback.setSystemIcon(SLOT_NO_SIM, 0, SLOT_NO_SIM, noSimsVisible);
    }

    public class NoSimView {
        public int mSlotId;
        public ImageView mNoSimView;

        public NoSimView(Context context) {
            mSlotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
            mNoSimView = new ImageView(mContext);
        }
    }

    @Override
    public void setSimInserted(int slotId, boolean insert) {
        updateSimInserted(slotId, insert);
    }

    @Override
    public void registerOpStateListener() {
        //mPhoneStateController.registerStateListener();
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
            case TelephonyManager.NETWORK_TYPE_GPRS:
            case TelephonyManager.NETWORK_TYPE_EDGE:
            case TelephonyManager.NETWORK_TYPE_CDMA:
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                iconId = NETWORK_TYPE_G;
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
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                iconId = DATA_TYPE_G;
                break;

            case TelephonyManager.NETWORK_TYPE_EDGE:
                iconId = DATA_TYPE_E;
                break;

            case TelephonyManager.NETWORK_TYPE_HSDPA:
            case TelephonyManager.NETWORK_TYPE_HSUPA:
            case TelephonyManager.NETWORK_TYPE_HSPA:
                iconId = DATA_TYPE_H;
                break;

            case TelephonyManager.NETWORK_TYPE_HSPAP:
                iconId = DATA_TYPE_H_PLUS;
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

    private int getSignalStrengthIconId(SignalStrength signalStrength, int slotId) {
        if (slotId < 0) {
            return SIGNAL_STRENGTH_NULL_ONE_SIM;
        }

        int level = (signalStrength != null ? signalStrength.getLevel() : 0);
        if (mCount == 1) {
            return SIGNAL_STRENGTH_SINGLE_SS_LOAD[level];
        } else {
            return SIGNAL_STRENGTH_SINGLE[slotId][level];
        }
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

}

