package com.mediatek.op08.phone;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsServiceClass;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.ims.internal.MtkImsManagerEx;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.op08.phone.R;

/**
 * Class to provide summary for wfc settings according to wfc state.
 */
public class WfcSummary {

    private static final String TAG = "WfcSummaryState";
    // TODO: need to read the value configured via engineering mode
    private static final int WIFI_SIGNAL_STRENGTH_THRESHOLD = -75;

    /** Enum defines various states which wfc can be in.
     * Is used to determine wfc summary string
     */
    private enum WfcSummaryState {
        SUMMARY_NON_WFC_STATE,
        SUMMARY_ENABLING,
        SUMMARY_WFC_ON,
        SUMMARY_READY_TO_CALL,
        SUMMARY_ERROR,
        SUMMARY_DISABLING,
        SUMMARY_WFC_OFF,
        SUMMARY_UNKNOWN_STATE
    };

    private Context mContext;
    private WfcSummaryState mWfcSummaryState = WfcSummaryState.SUMMARY_NON_WFC_STATE;
    private ImsManager mImsManager;
    private WifiManager mWifiManager;
    private ConnectivityManager mConnectivityManager;
    private WfcSummaryChangeListener mListener = null;
    private Phone mPhone;
    private int mPhoneId;
    private boolean mIsWfc = false;

    /**
     * Listen to the IMS service state change.
     */
    private ImsConnectionStateListener mImsConnectionStateListener =
        new ImsConnectionStateListener() {
        @Override
        public void onFeatureCapabilityChanged(int serviceClass,
                int[] enabledFeatures, int[] disabledFeatures) {
            if (serviceClass == ImsServiceClass.MMTEL) {
                boolean oldIsWfc = mIsWfc;
                mIsWfc = (enabledFeatures[ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI]
                        == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI);
                Log.d(TAG, "onFeatureCapabilityChanged mIsWfc=" + mIsWfc +
                        ", oldIsWfc=" + oldIsWfc);
                if (mIsWfc != oldIsWfc) {
                    int wfcState = WfcReasonInfo.CODE_WFC_DEFAULT;
                    if (mIsWfc) {
                       wfcState = WfcReasonInfo.CODE_WFC_SUCCESS;
                    }
                    updateWfcSummaryOnUI(wfcState);
                }
            }
        }
    };

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO Auto-generated method stub
            Log.d(TAG, "Intent action:" + intent.getAction());
            if (mPhone == null) {
                Log.e(TAG, "onReceive: phone is null");
                return;
            }
            int wfcState = WfcReasonInfo.CODE_WFC_DEFAULT;
            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(intent.getAction())
                    || WifiManager.WIFI_STATE_CHANGED_ACTION.equals(intent.getAction())
                    || (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())
                        && IccCardConstants.INTENT_VALUE_ICC_LOADED
                            .equals(intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE)))
                    || WifiManager.RSSI_CHANGED_ACTION.equals(intent.getAction())
                    || WifiManager.NETWORK_STATE_CHANGED_ACTION.equals(intent.getAction())) {
                boolean isWfcRegistered = mPhone.isWifiCallingEnabled();
                Log.d(TAG, "onReceive, isWfcRegistered:" + isWfcRegistered);
                wfcState = WfcReasonInfo.CODE_WFC_DEFAULT;
                if (isWfcRegistered) {
                    wfcState = WfcReasonInfo.CODE_WFC_SUCCESS;
                }
                sendWfcSummaryChange(wfcState);
            }
        }
    };

    /** Constructor.
     * @param context Context passed
     */
    public WfcSummary(Context context) {
        mContext = context;
        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        mConnectivityManager = (ConnectivityManager) context.
                getSystemService(Context.CONNECTIVITY_SERVICE);
    }

    /**
     * Called to register wfc summary receiver and listener.
     * @param listener
     *            The WfcSummaryChangeListener used to register
     */
    public void onResume(WfcSummaryChangeListener listener) {
        mListener = listener;

        IntentFilter filter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
        filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
        filter.addAction(WifiManager.RSSI_CHANGED_ACTION);
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);

        try {
            if (mImsManager != null) {
                mImsManager.addRegistrationListener(ImsServiceClass.MMTEL,
                        mImsConnectionStateListener);
            }
        } catch (ImsException e) {
            Log.e(TAG, "ImsException:" + e);
        }
    }

    /**
     * Called to unregister wfc summary receiver and listener.
     * @param listener
     *            The WfcSummaryChangeListener used to unregister
     */
    public void onPause(WfcSummaryChangeListener listener) {
        mListener = null;
        mContext.unregisterReceiver(mReceiver);

        try {
            if (mImsManager != null) {
                mImsManager.removeRegistrationListener(mImsConnectionStateListener);
            }
        } catch (ImsException e) {
            Log.e(TAG, "ImsException:" + e);
        }
    }

    /**
     * Init phone and basic ims values.
     * @param phone The target Phone
     */
    public void onCreate(Phone phone) {
        mPhone = phone;
        if (mPhone != null) {
            mPhoneId = mPhone.getPhoneId();
            mImsManager = ImsManager.getInstance(mPhone.getContext(), mPhoneId);
            Log.d(TAG, "setPhone: mPhoneId=" + mPhoneId + " mImsManager=" + mImsManager);
        }
    }

    /**
     * Set wfc registered state.
     * @param isWfc The wfc registered state.
     */
    public void setWfcRegistered(boolean isWfc) {
        mIsWfc = isWfc;
    }

    /** returns res_id of string to be displayed in Wfc Summary.
     * @param wfcState Current wfc state
     * @return res_id of string to be displayed as wfc summary
     */
    public String getWfcSummaryText(int wfcState) {
        if (!mImsManager.isWfcEnabledByUser()) {
            Log.d(TAG, "WFC OFF");
            return mContext.getResources().getString(R.string.Wfc_off_summary);
        }  else if (wfcState == WfcReasonInfo.CODE_WFC_SUCCESS) {
            Log.d(TAG, "WFC registered");
            return mContext.getResources().getString(R.string.wfc_ready_for_calls_summary);
        } else if (isAirplaneModeOn(mContext) && !isWifiEnabled()) {
            // Show airplane mode only if wifi is off
            // .i.e., if user has enabled wifi after turning Airplane mode ON
            //show summary as per wifi & ims/wfc conditions
            Log.d(TAG, "Airplane mode ON");
            return mContext.getResources().getString(R.string.airplane_mode_on);
        } else if (isWifiEnabled() && !isIsimAppPresent()) {
            // Show ISIM not present error only, when wifi is enabled
            Log.d(TAG, "ISIM not present");
            return mContext.getResources().getString(WfcReasonInfo
                    .getImsStatusCodeString(WfcReasonInfo.CODE_WFC_INCORRECT_SIM_CARD_ERROR));
        } else if (mImsManager.getWfcMode()
                == ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED
                && wfcState  == WfcReasonInfo.CODE_WFC_DEFAULT) {
            Log.d(TAG, "pref cellular preferred");
            return mContext.getResources().getString(R.string.cellular_prefered_summary);
        } else if (!isWifiEnabled()) {
            Log.d(TAG, "wifi OFF");
            return mContext.getResources().getString(R.string.wifi_off_summary);
        } else if (!isWifiConnected()) {
            Log.d(TAG, "wifi not  connected");
            return mContext.getResources().getString(R.string.not_connected_to_wifi_summary);
        } else if (mWifiManager.getConnectionInfo().getRssi() < WIFI_SIGNAL_STRENGTH_THRESHOLD) {
            Log.d(TAG, "poor signal strenth" + mWifiManager.getConnectionInfo().getRssi());
            return mContext.getResources().getString(R.string.poor_wifi_signal_summary);
        } else {
            // Above are summary according to external factors
            // If above factors are satisfied, now get summary acoording to IMS framework state
            return getWfcStateSummary(mContext, wfcState);
        }
    }

    private String getWfcStateSummary(Context context, int wfcState) {

        setWfcSummaryState(context, wfcState);

        switch (mWfcSummaryState) {
            case SUMMARY_ENABLING:
                return context.getResources().getString(R.string.wfc_enabling_summary);

            case SUMMARY_WFC_ON:
                return context.getResources().getString(R.string.wfc_on_summary);

            case SUMMARY_READY_TO_CALL:
                return context.getResources().getString(R.string.wfc_ready_for_calls_summary);

            case SUMMARY_ERROR:
                if (WfcReasonInfo.getImsStatusCodeString(wfcState) != 0) {
                    return context.getResources().getString(WfcReasonInfo
                            .getImsStatusCodeString(wfcState));
                } else {
                    Log.d(TAG, "in error but invalid code:" + wfcState);
                    return context.getResources().getString(R.string.wfc_unknown_error_summary);
                }

            case SUMMARY_DISABLING:
                return context.getResources().getString(R.string.wfc_disabling_summary);

            case SUMMARY_WFC_OFF:
                return context.getResources().getString(R.string.Wfc_off_summary);

            case SUMMARY_UNKNOWN_STATE:
            default:
                return context.getResources().getString(R.string.wfc_unknown_error_summary);

        }
    }

    private void setWfcSummaryState(Context context, int wfcState) {

        int imsState;
        try {
            imsState = MtkImsManagerEx.getInstance().getImsState(mPhoneId);
        } catch (ImsException e) {
            Log.d(TAG, "ImsException:" + e);
            imsState = MtkPhoneConstants.IMS_STATE_DISABLED;
        }
        Log.d(TAG, "ims state:" + imsState);
        Log.d(TAG, "wfc state:" + wfcState);
        Log.d(TAG, "wfc switch ON:" + isWfcSwitchOn(mContext));

        if (isWfcSwitchOn(mContext) && imsState == MtkPhoneConstants.IMS_STATE_ENABLING) {
            mWfcSummaryState = WfcSummaryState.SUMMARY_ENABLING;
        } else if (isWfcSwitchOn(mContext) && wfcState == WfcReasonInfo.CODE_WFC_DEFAULT) {
                    mWfcSummaryState = WfcSummaryState.SUMMARY_WFC_ON;
        }  else if (isWfcSwitchOn(mContext) && wfcState == WfcReasonInfo.CODE_WFC_SUCCESS) {
                    mWfcSummaryState = WfcSummaryState.SUMMARY_READY_TO_CALL;
        } else if (isWfcSwitchOn(mContext) && wfcState > WfcReasonInfo.CODE_WFC_DEFAULT) {
                    mWfcSummaryState = WfcSummaryState.SUMMARY_ERROR;
        } else if (imsState == MtkPhoneConstants.IMS_STATE_DISABLING
                && (wfcState > WfcReasonInfo.CODE_WFC_DEFAULT
                        || wfcState == WfcReasonInfo.CODE_WFC_SUCCESS)) {
                    mWfcSummaryState = WfcSummaryState.SUMMARY_DISABLING;
        } else if (!isWfcSwitchOn(mContext)) {
            mWfcSummaryState = WfcSummaryState.SUMMARY_WFC_OFF;
        } else {
            mWfcSummaryState = WfcSummaryState.SUMMARY_UNKNOWN_STATE;
        }
        Log.d(TAG, "summary state:" + mWfcSummaryState);
    }

    private void updateWfcSummaryOnUI(int wfcState) {
        Handler handler = new Handler(mContext.getMainLooper());
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "updateWfcSummaryOnUI");
                sendWfcSummaryChange(wfcState);
            }
        };
        handler.post(runnable);
    }

    private void sendWfcSummaryChange(int wfcState) {
        if (mListener != null) {
            String summary = getWfcSummaryText(wfcState);
            Log.d(TAG, "Sending change with summary:" + summary);
            mListener.onWfcSummaryChanged(summary);
        }
    }

    private boolean isWfcSwitchOn(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                Settings.System.WHEN_TO_MAKE_WIFI_CALLS,
                TelephonyManager.WifiCallingChoices.ALWAYS_USE)
                == TelephonyManager.WifiCallingChoices.ALWAYS_USE ? true : false;
    }

    private boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0) == 1;
    }

    private boolean isWifiEnabled() {
        int wifiState = mWifiManager.getWifiState();
        Log.d(TAG, "wifi state:" + wifiState);
        return (wifiState != WifiManager.WIFI_STATE_DISABLED);
    }

    private boolean isWifiConnected() {
        NetworkInfo networkInfo = mConnectivityManager
            .getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        Log.d(TAG, "networkInfo:" + networkInfo);
        if (networkInfo != null) {
            Log.d(TAG, "networkInfo:" + networkInfo.isConnected());
            Log.d(TAG, "networkInfo:" + networkInfo.getDetailedState());
        }
        return (networkInfo != null && (networkInfo.isConnected()
                    || networkInfo.getDetailedState() == DetailedState.CAPTIVE_PORTAL_CHECK));
    }

    private boolean isIsimAppPresent() {
        IMtkTelephonyEx telEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        Log.d(TAG, "TelephonyEx service:" + telEx);
        boolean iSimPresent = false;
        try {
            if (telEx != null) {
                iSimPresent = telEx.isAppTypeSupported(mPhoneId, PhoneConstants.APPTYPE_ISIM);
            }
        } catch (RemoteException e) {
            Log.d(TAG, "IMtkTelephonyEx exceptio:" + e);
        }
        Log.d(TAG, "iSimPresent:" + iSimPresent);
        return iSimPresent;
    }

    /**
     * The WfcSummaryChangeListener defined as a listener to notify the
     * specify observer that the wfc summary has been changed.
     */
    public interface WfcSummaryChangeListener {

        /**
            * On wfc summary changed.
            * @param summary The current wfc summary
            */
        void onWfcSummaryChanged(String summary);
    }
}
