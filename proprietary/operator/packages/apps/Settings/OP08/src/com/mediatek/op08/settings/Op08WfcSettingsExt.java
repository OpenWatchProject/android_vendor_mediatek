package com.mediatek.op08.settings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;

import android.provider.Settings;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragment;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;
import com.android.settings.SettingsActivity;
import com.android.settings.widget.SwitchBar;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.ims.internal.MtkImsManagerEx;
import com.mediatek.op08.settings.R;
import com.mediatek.settings.ext.DefaultWfcSettingsExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Plugin implementation for WFC Settings plugin
 */
public class Op08WfcSettingsExt extends DefaultWfcSettingsExt {

    private static final String TAG = "Op08WfcSettingsExt";
    private static final String TUTORIALS = "Tutorials";
    private static final String TOP_QUESTIONS = "Top_questions";
    private static final String WFC_MODE = "Wfc_mode";
    private static final String AOSP_BUTTON_WFC_MODE = "wifi_calling_mode";
    private static final String TUTORIAL_ACTION = "mediatek.settings.WFC_TUTORIALS";
    private static final String QA_ACTION = "mediatek.settings.WFC_QA";
    private static final String WFC_MODE_DIALOG_ACTION = "mediatek.settings.SHOW_WFC_MODE_DIALOG";
    private static final int FEATURE_CONFIGURED = 99;
    private static final int FEATURE_UNCONFIGURED = 100;
    private static final int WIFI_SIGNAL_STRENGTH_THRESHOLD = -75;

    private Context mContext;
    private PreferenceFragmentCompat mPreferenceFragment;
    private Preference mWfcModePreference;
    private SwitchBar mSwitchBar;
    private WifiManager mWifiManager;

    private ContentObserver mWfcModeChangeObserver;
    private ConnectivityManager mConnectivityManager;
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
    private WfcSummaryState mWfcSummaryState = WfcSummaryState.SUMMARY_NON_WFC_STATE;

    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {
        @Override
        public void onCallStateChanged(int state, String incomingNumber) {
            boolean isWfcEnabled = ImsManager.isWfcEnabledByPlatform(mContext);
            Log.d(TAG, "Phone state:" + state);
            switch (state) {
                case TelephonyManager.CALL_STATE_IDLE:
                   if (mSwitchBar == null) {
                        Log.d(TAG, "mWfcSwitchBar is null, so return");
                        return;
                    }
                    mSwitchBar.setEnabled(true);
                    Log.d(TAG, "WFC mode Enabled :" + isWfcEnabled);
                    mWfcModePreference.setEnabled(isWfcEnabled);
                    break;
                case TelephonyManager.CALL_STATE_OFFHOOK:
                case TelephonyManager.CALL_STATE_RINGING:
                    if (mSwitchBar != null) {
                        mSwitchBar.setEnabled(false);
                        Log.d(TAG, "mWfcSwitchBar set disabled");
                    }
                    mWfcModePreference.setEnabled(false);
                    Log.d(TAG, "mWfcModePreference set disabled");
                    break;
                default:
                    break;
            }
        }
    };

    /** Constructor.
     * @param context default summary res id
     */
    public Op08WfcSettingsExt(Context context) {
        super();
        mContext = context;
        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        mConnectivityManager =
            (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        Log.d(TAG, "Op08WfcSettingsExt");
    }

   /**
    * Initialize plugin with essential values.
     * @param
     * @return
     */
    @Override
    public void initPlugin(PreferenceFragmentCompat pf) {
        mPreferenceFragment = pf;
    }

    /** Called from onPause/onResume.
     * @param event event happened
     * @return
     */
    @Override
    public void onWfcSettingsEvent(int event) {
        Log.d(TAG, "WfcSeting event:" + event);
        switch (event) {
            case DefaultWfcSettingsExt.RESUME:
                SettingsActivity settingsActivity = (SettingsActivity) mPreferenceFragment
                        .getActivity();
                mSwitchBar = settingsActivity.getSwitchBar();
                ((TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE))
                        .listen(mPhoneStateListener, PhoneStateListener.LISTEN_CALL_STATE);
                mContext.getContentResolver().registerContentObserver(
                        android.provider.Settings.Global.getUriFor(
                                android.provider.Settings.Global.WFC_IMS_MODE),
                                false, mWfcModeChangeObserver);
                break;
            case DefaultWfcSettingsExt.PAUSE:
                ((TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE))
                        .listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
                mContext.getContentResolver().unregisterContentObserver(mWfcModeChangeObserver);
                break;
            default:
                break;
        }
    }

    /** Add WFC tutorial prefernce, if any.
     * @param
     * @return
     */
    @Override
    public void addOtherCustomPreference() {
        PreferenceScreen ps = mPreferenceFragment.getPreferenceScreen();
        /* remove AOSP wfc mode preference */
        ps.removePreference(mPreferenceFragment.findPreference(AOSP_BUTTON_WFC_MODE));

        /* Add cutomized wfc mode preference */
        mWfcModePreference = new Preference(ps.getContext());
        mWfcModePreference.setKey(WFC_MODE);
        mWfcModePreference.setTitle(mContext.getText(R.string.wfc_mode_preference_title));
        mWfcModePreference.setSummary(WfcUtils.getWfcModeSummary(ImsManager.getWfcMode(mContext)));
        mWfcModePreference.setIntent(new Intent(WFC_MODE_DIALOG_ACTION));
        ps.addPreference(mWfcModePreference);
        /* Register content observer on Wfc Mode to change summary of this pref on mode change */
        registerForWfcModeChange(new Handler());

        Preference wfcTutorialPreference = new Preference(ps.getContext());
        wfcTutorialPreference.setKey(TUTORIALS);
        wfcTutorialPreference.setTitle(mContext.getText(R.string.Tutorials));
        wfcTutorialPreference.setIntent(new Intent(TUTORIAL_ACTION));
        ps.addPreference(wfcTutorialPreference);

        Preference wfcQAPreference = new Preference(ps.getContext());
        wfcQAPreference.setKey(TOP_QUESTIONS);
        wfcQAPreference.setTitle(mContext.getText(R.string.Top_questions));
        wfcQAPreference.setIntent(new Intent(QA_ACTION));
        ps.addPreference(wfcQAPreference);
    }

    /** Takes operator specific action on wfc list preference on switch change:
     * On Switch OFF, disable wfcModePref.
     * @param root
     * @param wfcModePref
     * @return
     */
    @Override
    public void updateWfcModePreference(PreferenceScreen root, ListPreference wfcModePref,
            boolean wfcEnabled, int wfcMode) {
        Log.d(TAG, "wfc_enabled:" + wfcEnabled + " wfcMode:" + wfcMode);
        mWfcModePreference.setSummary(WfcUtils.getWfcModeSummary(wfcMode));
        mWfcModePreference.setEnabled(wfcEnabled);
        // Remove Wfc mode preference because host app adds it if wfc is enabled
        if (wfcEnabled) {
            root.removePreference(wfcModePref);
        }
    }

    /*
    * Observes WFC mode changes to change summary of preference.
    */
    private void registerForWfcModeChange(Handler handler) {
        mWfcModeChangeObserver = new ContentObserver(handler) {

            @Override
            public void onChange(boolean selfChange) {
                this.onChange(selfChange,
                        android.provider.Settings.Global
                        .getUriFor(android.provider.Settings.Global.WFC_IMS_MODE));
            }
            @Override
            public void onChange(boolean selfChange, Uri uri) {
                Uri i = android.provider.Settings.Global
                        .getUriFor(android.provider.Settings.Global.WFC_IMS_MODE);
                Log.d("@M_" + TAG, "wfc mode:" + ImsManager.getWfcMode(mContext));
                if (i != null && i.equals(uri)) {
                        mWfcModePreference.setSummary(WfcUtils
                                .getWfcModeSummary(ImsManager.getWfcMode(mContext)));
                }
            }
        };
    }

    @Override
    public void customizedWfcSummary(Preference preference){
        Log.i(TAG, "customizedWfcSummary");
        boolean isWfcRegistered = MtkTelephonyManagerEx.getDefault()
                .isWifiCallingEnabled(SubscriptionManager.getDefaultVoiceSubscriptionId());
        Log.d(TAG, "[getWfcSummary] isWfcRegistered:" + isWfcRegistered);
        int wfcState = WfcReasonInfo.CODE_WFC_DEFAULT;
        if (isWfcRegistered) {
            wfcState = WfcReasonInfo.CODE_WFC_SUCCESS;
        }
        preference.setSummary(getWfcSummaryText(wfcState));
    }

    /** returns res_id of string to be displayed in Wfc Summary.
     * @param wfcState Current wfc state
     * @return res_id of string to be displayed as wfc summary
     */
    public String getWfcSummaryText(int wfcState) {
        if (!ImsManager.isWfcEnabledByUser(mContext)) {
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
        } else if (ImsManager.getWfcMode(mContext)
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

    private boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0) == 1;
    }

    private boolean isWifiEnabled() {
        int wifiState = mWifiManager.getWifiState();
        Log.d(TAG, "wifi state:" + wifiState);
        return (wifiState != WifiManager.WIFI_STATE_DISABLED);
    }

    private boolean isIsimAppPresent() {
        IMtkTelephonyEx telEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        Log.d(TAG, "TelephonyEx service:" + telEx);
        boolean iSimPresent = false;
        try {
            if (telEx != null) {
                iSimPresent = telEx.isAppTypeSupported(SubscriptionManager.getDefaultVoicePhoneId(),
                    PhoneConstants.APPTYPE_ISIM);
            }
        } catch (RemoteException e) {
            Log.d(TAG, "IMtkTelephonyEx exceptio:" + e);
        }
        Log.d(TAG, "iSimPresent:" + iSimPresent);
        return iSimPresent;
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
            imsState = MtkImsManagerEx.getInstance().getImsState(RadioCapabilitySwitchUtil
                         .getMainCapabilityPhoneId());
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

    private boolean isWfcSwitchOn(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                Settings.System.WHEN_TO_MAKE_WIFI_CALLS,
                TelephonyManager.WifiCallingChoices.ALWAYS_USE)
                == TelephonyManager.WifiCallingChoices.ALWAYS_USE ? true : false;
    }

    @Override
    public boolean customizeBroadcastReceiveIntent() {
        Log.i(TAG, "customizeBroadcastReceiveIntent return true");
        return true;
    }

}