package com.mediatek.op18.settings;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.TextView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

import android.widget.ArrayAdapter;
import com.android.ims.ImsManager;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkDefaultSmsSimSettings;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.settings.ext.DefaultSimManagementExt;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * Handle selection of non-lte sim for default data and disable SIM dialog at bootup.
 */
public class Op18SimManagementExt extends DefaultSimManagementExt {

    private static final String TAG = "Op18SimManagementExt";
    private static final String KEY_CALLS = "sim_calls";
    private static final String KEY_SMS = "sim_sms";
    private static final int DATA_PICK = 0;
    private static final int CALLS_PICK = 1;
    private static final int SMS_PICK = 2;
    private static final float OPACITY = 0.54f;
    private static TelecomManager telecomManager;

    private Context mContext;
    PreferenceFragmentCompat mPrefFragment;
    private AlertDialog mAlertDlg;
    private int mToCloseSlot = -1;
    private int mSimMode;
    private boolean mIsItemAdded = false;
    Preference smartCallPref;

    private static final String LTE_SUPPORT = "1";
    private static final String VOLTE_SUPPORT = "1";

    // For Primary SIM
    private PreferenceFragmentCompat mHostPf;
    private static final String SYSTEM_PROPERTY_MAIN_PROTOCOL =
            "persist.vendor.radio.simswitch";
    private static final String SYSTEM_PROPERTY_SIM1 =
            "gsm.ril.uicctype";
    private static final String SYSTEM_PROPERTY_SIM2 =
            "gsm.ril.uicctype.2";

    private static final String MULTI_IMS_SUPPORT =
        "persist.vendor.mims_support";

    private IntentFilter mImsIntentFilter;

    private static final String SMART_CALL_FWD_KEY = "smart_call_fwd_key";
    private final String SMART_CALL_PREF_CAT_KEY = "smart_call_activities";
    private final String SMART_CALL_FWD_SETTINGS = "mediatek.settings.SMART_CALL_FWD_SETTINGS";

    /**
     * Action to broadcast when Primary Sim is changed.
     */
    public static final String ACTION_PRIMARY_SIM_CHANGED =
            "com.mediatek.settings.PRIMARY_SIM_CHANGED";
    /**
     * Part of the ACTION_PRIMARY_SIM_CHANGED intents.
     * A long value; the phone ID corresponding to the new primary Sim.
     */
    public static final String EXTRA_SUBSCRIPTION_ID = "subscription_id";

    private static final String[] RJIL_NUMERIC = {
            "405840", "405854", "405855", "405856", "405857", "405858",
            "405859", "405860", "405861", "405862", "405863", "405864",
            "405865", "405866", "405867", "405868", "405869", "405870",
            "405871", "405872", "405873", "405874", "22201"};
    // Subinfo record change listener.
    private BroadcastReceiver mSubReceiver = new BroadcastReceiver() {

            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(TelephonyIntents.ACTION_SUBINFO_CONTENT_CHANGE)
                        || action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                        int[] subids = SubscriptionManager.from(mContext).
                                getActiveSubscriptionIdList();
                        if (subids == null || subids.length <= 1) {
                            if (mAlertDlg != null && mAlertDlg.isShowing()) {
                                Log.d(TAG, "onReceive dealWithDataConnChanged dismiss AlertDlg");
                                mAlertDlg.dismiss();
                                if (mPrefFragment != null) {
                                    mPrefFragment.getActivity().unregisterReceiver(mSubReceiver);
                                }
                            }
                        }
                }
            }
    };

    private BroadcastReceiver mImsReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "OP18 Primary SIM mImsReceiver action = " + action);
            if (action.equalsIgnoreCase("com.mediatek.ims.notify_ims_switch_done")) {
                PreferenceCategory mPreferenceCategoryActivities =
                    (PreferenceCategory) mHostPf.findPreference("sim_activities");
                Preference primarySimPref = mPreferenceCategoryActivities.getPreference(0);
                Log.d(TAG, "OP18 Primary SIM mImsReceiver enable the preference");
                //primarySimPref.setSummary(subInfo.getDisplayName());

                // Enable after receiving this broadcast
                primarySimPref.setEnabled(true);

            }
        }
    };

    /**
     * Initialize plugin context.
     * @param context context
     */
    public Op18SimManagementExt(Context context) {
        super();
        mContext = context;
        telecomManager = TelecomManager.from(context);
    }

    /**
     * Disable SIM dialog.
     * @return false To disable sim dialog.
     */
    @Override
    public boolean isSimDialogNeeded() {
        setDefaultPickForSmsAndCall();
        return false;
    }

    /**
     * Set default pick for sms and call.
     */
    private void setDefaultPickForSmsAndCall() {
        //final ArrayList<String> list = new ArrayList<String>();
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        final List<SubscriptionInfo> subInfoList =
                subscriptionManager.getActiveSubscriptionInfoList();
        final int selectableSubInfoLength = subInfoList == null ? 0 : subInfoList.size();
        int subId = subscriptionManager.getDefaultSubscriptionId();
        Log.d(TAG, "subId: " + subId + "selectableSubInfoLength: " + selectableSubInfoLength);

        if (selectableSubInfoLength == 1) {
            // Set default SMS pick
            subscriptionManager.setDefaultSmsSubId(subId);
            Log.d(TAG, "set Default SMS PICK");

            //Set default Call pick
            PhoneAccountHandle selectedAccountHandle =
                    telecomManager.getUserSelectedOutgoingPhoneAccount();
            final List<PhoneAccountHandle> accountHandles =
                    telecomManager.getCallCapablePhoneAccounts();
            Log.d(TAG, "selectedAccountHandle: " + selectedAccountHandle);
            for (PhoneAccountHandle handle : accountHandles) {
                final PhoneAccount account = telecomManager.getPhoneAccount(handle);
                if (account != null) {
                    telecomManager.setUserSelectedOutgoingPhoneAccount(handle);
                    Log.d(TAG, "set Default CALL PICK");
                    break;
                }
            }
        }
    }

    public boolean switchDefaultDataSub(Context context, int subId) {
        boolean defaultSub = false;
        if (isShowConfirmDialog(subId)) {
            Intent start = new Intent(mContext, DataSwitchDialog.class);
            start.putExtra("subId", subId);
            //start.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(start);
            defaultSub = true;
        }
        Log.d(TAG, "switchDefaultDataSub(), " + subId + "defaultSub" + defaultSub);
        return defaultSub;
    }

    /**
     * app use to judge LTE open.
     * @return true is LTE open
     */
    private boolean isLTESupport() {
        boolean isSupport = LTE_SUPPORT.equals(
                SystemProperties.get("ro.vendor.mtk_lte_support")) ? true : false;
        Log.d(TAG, "isLTESupport(): " + isSupport);
        return isSupport;
    }

    /**
     * app use to judge LTE open.
     * @return true is LTE open
     */
    private boolean isVoLTESupport() {
        boolean isSupport = VOLTE_SUPPORT.equals(
                SystemProperties.get("persist.vendor.volte_support")) ? true : false;
        Log.d(TAG, "isVoLTESupport(): " + isSupport);
        return isSupport;
    }

    /**
     * get the LTE Capability subId.
     * @return the LTE Capability subId
     */
    private int getLTECapabilitySubId() {
        int subId = -1;
        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (iTelEx != null) {
            try {
                int phoneId = iTelEx.getMainCapabilityPhoneId();
                Log.d("@M_" + TAG, "subId : " + subId + ", PhoneId : " + phoneId);
                if (phoneId >= 0) {
                    subId = MtkSubscriptionManager.getSubIdUsingPhoneId(iTelEx
                                    .getMainCapabilityPhoneId());
                    }
            } catch (RemoteException e) {
                Log.d("@M_" + TAG, "getLTECapabilitySubId FAIL to getSubId" + e.getMessage());
            }
        }
        return subId;
    }

   /**
    * app use to judge if need confirm before switch data.
    * @return false is no need confirm
    */
    private boolean isShowConfirmDialog(int switchtoSubId) {
        boolean imsRegStatus = false;

        Log.d(TAG, "isShowConfirmDialog(): MULTI_IMS_SUPPORT = " +
            SystemProperties.getInt(MULTI_IMS_SUPPORT, 1));

        // in MIMS, no need to show switch dialog
        if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) != 1) {
            return false;
        }

        if (!isLTESupport() || !isVoLTESupport()) {
            Log.d(TAG, "isShowConfirmDialog(): isLTESupport() or isVoLTESupport() not support");
            return imsRegStatus;
        } else if (TelecomManager.from(mContext).isInCall()) {
            Log.d(TAG, "isShowConfirmDialog(): inCall, don't switch");
            return imsRegStatus;
        }

        try {
            int curDataSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            if (!isSimTypeJio(curDataSubId)) {
                return false;
            } else {
                String ratProtocol2 = SystemProperties.get("persist.vendor.radio.mtk_ps2_rat");
                if (ratProtocol2.equalsIgnoreCase("L/W/G")) {
                    return false;
                }
            }
            ImsManager imsManager = ImsManager.getInstance(
                        mContext, SubscriptionManager.getPhoneId(curDataSubId));
            IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                    .getService("phoneEx"));
            imsRegStatus = iTelEx.isImsRegistered(curDataSubId);
            Log.d(TAG, "imsRegStatus = " + imsRegStatus + " curDataSubId: " + curDataSubId);
            if (imsRegStatus == false) {
                return false;
            }
        } catch (Exception ex) {
            Log.e(TAG, "Fail to get Ims Status");
        }
        int subId = getLTECapabilitySubId();
        int switchToSlotId = SubscriptionManager.getPhoneId(switchtoSubId);
        Log.d(TAG, "subId:" + subId + "switchtoSubId:" + switchtoSubId);
        //currently no major 4G card
        if (subId < 0) {
            return false;
        } else if (imsRegStatus && switchtoSubId != subId) {
            //switch data to default 4G card, only data switch, 4G isn't switch
            Log.d(TAG, "switch Data from LTE SIM to 2G/3G SIM");
            return true;
        } else {
            Log.d(TAG, "default case");
            return false;
        }
        //return true;
    }

    /**
     * Check if the Jio sim is insterted in shared subid or not.
     * @param subId provided subId of slot
     * @return true if Sim is of Jio operator
     */
    public static boolean isSimTypeJio(int subId) {
        boolean result = false;
        String numeric = TelephonyManager.getDefault().getSimOperator(subId);
        for (String simJio : RJIL_NUMERIC) {
            if (simJio.equals(numeric)) {
                result = true;
                break;
            }
        }
        Log.d(TAG, "isSimTypeJio: subId " + subId + " mcc mnc " + numeric + " result "  + result);
        return result;
    }

    /**
     * Set data state.
     * @param subid subscription id
     */
    public void setDataState(int subid) {
        if (subid != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            int curConSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            TelephonyManager tm = TelephonyManager.from(mContext);
            Log.d(TAG, "setDataState,curConSubId: " + curConSubId + "subid:" + subid);
            if (curConSubId == subid) {
                return;
            }
            if (tm.getDataEnabled(curConSubId) || tm.getDataEnabled(subid)) {
                Log.d(TAG, "setDataState: setDataEnabled curConSubId false");
                tm.setDataEnabled(curConSubId, false);
                tm.setDataEnabled(subid, true);
            }
         }
     }

    /**
     * Sets subId in dataUsage true.
     * @param subid subscription id
     */
    public void setDataStateEnable(int subid) {
        TelephonyManager tm = TelephonyManager.from(mContext);
        if (tm.getDataEnabled(subid)) {
            Log.d(TAG, "setDataStateEnable true subId:" + subid);
            tm.setDataEnabled(subid, true);
        }
    }

    @Override
    public void updateList(final ArrayList<String> list,
            ArrayList<SubscriptionInfo> smsSubInfoList, final int selectableSubInfoLength) {
        int result = Settings.Global.getInt(mContext.getContentResolver(),
                                    MtkSettingsExt.Global.CURRENT_NETWORK_CALL, 0);
        int resultSms = Settings.Global.getInt(mContext.getContentResolver(),
                                    MtkSettingsExt.Global.CURRENT_NETWORK_SMS, 0);
        Log.d(TAG, "updateList result call = " + result + "result sms = " + resultSms);

        if (selectableSubInfoLength > 1) {
            list.add(mContext.getResources().getString(R.string.current_network));
            smsSubInfoList.add(null);
            mIsItemAdded = true;
        }
    }

    @Override
    public boolean simDialogOnClick(int id, int value, Context context) {
        Log.d(TAG, "simDialogOnClick, id = " + id + "value = " + value +
                "mIsItemAdded = " + mIsItemAdded);
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        final List<SubscriptionInfo> subInfoList =
            subscriptionManager.getActiveSubscriptionInfoList();

        switch (id) {
            case DATA_PICK:
                final SubscriptionInfo sir = subInfoList.get(value);
                int targetSub = (sir == null ? null : sir.getSubscriptionId());
                return switchDefaultDataSub(mContext, targetSub);

            case SMS_PICK:
                if (mIsItemAdded) {
                    Log.d(TAG, "current network sms" + value);
                    if (value == 0) {
                    // set the current network value to true
                        Settings.Global.putInt(mContext.getContentResolver(),
                                                MtkSettingsExt.Global.CURRENT_NETWORK_SMS, 1);

                        // Set default SMS pick
                        subscriptionManager.setDefaultSmsSubId(
                                         MtkDefaultSmsSimSettings.ASK_USER_SUB_ID);
                    } else {
                        Settings.Global.putInt(mContext.getContentResolver(),
                                                MtkSettingsExt.Global.CURRENT_NETWORK_SMS, 0);
                        // add host app handling
                        value = value - 1;
                        int subId = getPickSmsDefaultSub(subInfoList, value);
                        setDefaultSmsSubId(mContext, subId);
                    }
                    return true;
                }
                break;

            case CALLS_PICK:
                if (mIsItemAdded) {
                    Log.d(TAG, "current network call" + value);
                    if (value == 0) {
                    // set the current network value to true
                        Settings.Global.putInt(mContext.getContentResolver(),
                                                MtkSettingsExt.Global.CURRENT_NETWORK_CALL, 1);
                        setMainCapabilityAccount();
                    } else {
                        Settings.Global.putInt(mContext.getContentResolver(),
                                                MtkSettingsExt.Global.CURRENT_NETWORK_CALL, 0);
                        // add host app handling
                        value = value - 1;
                        handleHostAppCall(value);
                    }
                    return true;
                }
                break;

            default:
                return false;
        }
        return false;

        // if the operator had done some changes do the screen then
        // all option need to be handled at the OP end only.
        // this is done to avoid coupling b/w host & OP
    }

    @Override
    public void setCurrNetworkIcon(ImageView icon, int id, int position) {
        Log.d(TAG, "setCurrNetworkIcon" + id);
        if ((id == CALLS_PICK || id == SMS_PICK)
                    && mIsItemAdded && position == 0) {
            icon.setImageDrawable(mContext.getResources()
                            .getDrawable(R.drawable.current_network));
            icon.setAlpha(OPACITY);

        }
    }


    @Override
    public void setPrefSummary(Preference simPref, String type) {
        Log.d(TAG, "setPrefSummary" + type);
        int result = 0;
        switch (type) {
            case KEY_CALLS:
                result = Settings.Global.getInt(mContext.getContentResolver(),
                                            MtkSettingsExt.Global.CURRENT_NETWORK_CALL, 0);
                break;

            case KEY_SMS:
                result = Settings.Global.getInt(mContext.getContentResolver(),
                                            MtkSettingsExt.Global.CURRENT_NETWORK_SMS, 0);
        }
        if (result == 1) {
            simPref.setSummary(mContext.getResources().getString(R.string.current_network));
        }
    }

          /**
         * Sets phone account of selected subId.
         * @param subId subId
         * @return
         */
    public void setPhoneAccount(int subId) {
        PhoneAccountHandle phoneAccountHandle = subscriptionIdToPhoneAccountHandle(subId);
        if (phoneAccountHandle != null) {
            telecomManager.setUserSelectedOutgoingPhoneAccount(phoneAccountHandle);
            Log.d(TAG, "account set:" + phoneAccountHandle);
        } else {
            Log.d(TAG, "account not set");
        }

    }

    /**
         * Sets Main capability phone account.
         * @return subId
         */
    public int setMainCapabilityAccount() {
        int masterPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        int subId = MtkSubscriptionManager.getSubIdUsingPhoneId(masterPhoneId);
        Log.d(TAG, "main capability account subId:" + subId);
        setPhoneAccount(subId);
        return subId;
    }

    private PhoneAccountHandle subscriptionIdToPhoneAccountHandle(int subId) {
        TelephonyManager sTelephonyManager = TelephonyManager.from(mContext);

        final Iterator<PhoneAccountHandle> phoneAccounts =
                telecomManager.getCallCapablePhoneAccounts().listIterator();

        while (phoneAccounts.hasNext()) {
            final PhoneAccountHandle phoneAccountHandle = phoneAccounts.next();
            final PhoneAccount phoneAccount = telecomManager.getPhoneAccount(phoneAccountHandle);
            if (subId == sTelephonyManager .getSubIdForPhoneAccount(phoneAccount)) {
                return phoneAccountHandle;
            }
        }
        return null;
    }

    private void handleHostAppCall(int value) {
        final List<PhoneAccountHandle> phoneAccountsList =
                telecomManager.getCallCapablePhoneAccounts();
        Log.d(TAG, "phoneAccountsList = " + phoneAccountsList.toString() +
                "value = " + value);

        /// M: for ALPS02320816 @{
        // phone account may changed in background
        if (value > phoneAccountsList.size()) {
            Log.w(TAG, "phone account changed, do noting! value = " +
                    value + ", phone account size = " +
                    phoneAccountsList.size());
            return;
        }
        /// @}
        setUserSelectedOutgoingPhoneAccount(value < 1 ?
                null : phoneAccountsList.get(value - 1));
    }

    private void setUserSelectedOutgoingPhoneAccount(PhoneAccountHandle phoneAccount) {
        Log.d(TAG, "setUserSelectedOutgoingPhoneAccount phoneAccount = " + phoneAccount);
        telecomManager.setUserSelectedOutgoingPhoneAccount(phoneAccount);
    }

    private int getPickSmsDefaultSub(final List<SubscriptionInfo> subInfoList,
            int value) {
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        if (value < 1) {
            int length = subInfoList == null ? 0 : subInfoList.size();
            if (length == 1) {
                subId = subInfoList.get(value).getSubscriptionId();
            } else {
                subId = MtkDefaultSmsSimSettings.ASK_USER_SUB_ID;
            }
        } else if (value >= 1 && value < subInfoList.size() + 1) {
            subId = subInfoList.get(value - 1).getSubscriptionId();
        }
        //subId = mRCSExt.getDefaultSmsClickContentExt(subInfoList, value, subId);
        Log.d(TAG, "getPickSmsDefaultSub, value: " + value + ", subId: " + subId);
        return subId;
    }

    private static void setDefaultSmsSubId(final Context context, final int subId) {
        Log.d(TAG, "setDefaultSmsSubId, sub = " + subId);
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
        subscriptionManager.setDefaultSmsSubId(subId);
    }


    private CharSequence getDisplayNameFromMainProtocol() {
        Log.d(TAG, "OP18 Primary SIM getDisplayNameFromMainProtocol");
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        final List<SubscriptionInfo> subInfoList =
                subscriptionManager.getActiveSubscriptionInfoList();

        int mainProtocol = SystemProperties.getInt(SYSTEM_PROPERTY_MAIN_PROTOCOL, -1);
        Log.d(TAG, "OP18 Primary SIM getDisplayNameFromMainProtocol main_protocol = "
            + mainProtocol);
        if (mainProtocol != -1) {
            if (subInfoList != null && subInfoList.size() > 0) {
                int subId = 0;
                if (subInfoList.size() == 1) {
                    subId = subInfoList.get(0).getSubscriptionId();
                } else {
                    subId = subInfoList.get(mainProtocol - 1).getSubscriptionId();
                }
                Log.d(TAG, "OP18 Primary SIM getDisplayNameFromMainProtocol subId = " + subId);
                SubscriptionInfo subInfo =
                    subscriptionManager.getActiveSubscriptionInfo(subId);
                Log.d(TAG,
                    "OP18 Primary SIM getDisplayNameFromMainProtocol subInfo.getDisplayName() = "
                    + subInfo.getDisplayName());
                return subInfo.getDisplayName();
            } else {
                Log.d(TAG, "OP18 Primary SIM getDisplayNameFromMainProtocol subInfoList size is 0");
                return "";
            }
        }
        Log.d(TAG, "OP18 Primary SIM getDisplayNameFromMainProtocol empty");
        return "";
    }

    private int getSubIdFromMainProtocol() {
        Log.d(TAG, "OP18 Primary SIM getSubIdFromMainProtocol");
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        final List<SubscriptionInfo> subInfoList =
                subscriptionManager.getActiveSubscriptionInfoList();

        int mainProtocol = SystemProperties.getInt(SYSTEM_PROPERTY_MAIN_PROTOCOL, -1);
        Log.d(TAG, "OP18 Primary SIM getSubIdFromMainProtocol main_protocol = " + mainProtocol);
        if (mainProtocol != -1) {
            if (subInfoList != null && subInfoList.size() > 0) {
                int subId = 0;
                if (subInfoList.size() == 1) {
                    subId = subInfoList.get(0).getSubscriptionId();
                } else {
                    subId = subInfoList.get(mainProtocol - 1).getSubscriptionId();
                }
                Log.d(TAG, "OP18 Primary SIM getSubIdFromMainProtocol subId = " + subId);
                return subId;
            } else {
                Log.d(TAG, "OP18 Primary SIM getSubIdFromMainProtocol subInfoList size is 0");
                return -1;
            }
        }
        Log.d(TAG, "OP18 Primary SIM getSubIdFromMainProtocol -1");
        return -1;
    }

    /**
     * Initialize primary sim selection Prefernce.
     * @param pf activity's Preference Fragment
     */
    public void initPrimarySim(PreferenceFragmentCompat pf) {
        String ratProtocol2 = SystemProperties.get("persist.vendor.radio.mtk_ps2_rat");
        if (ratProtocol2.equalsIgnoreCase("L/W/G")) {
            return;
        }
        Log.d(TAG, "PrimarySim plugin");
        mHostPf = pf;
        CharSequence prefSummary = "";
        PreferenceScreen ps = pf.getPreferenceScreen();
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(ps.getContext());
        final List<SubscriptionInfo> subInfoList =
                subscriptionManager.getActiveSubscriptionInfoList();
        Log.d(TAG, "PrimarySim plugin initPrimarySim ps = " + ps);

        PreferenceCategory mPreferenceCategoryActivities =
            (PreferenceCategory) pf.findPreference("sim_activities");

        Preference primarySimPref = new Preference(ps.getContext());
        primarySimPref.setKey("primary_SIM_key");
        primarySimPref.setTitle(mContext.getText(R.string.primary_sim_settings_title));
        primarySimPref.setOrder(-10);

        // Value currently saved in global Setting
        int savedValue = Settings.Global.getInt(mContext.getContentResolver(),
            MtkSettingsExt.Global.PRIMARY_SIM, -1);
        Log.d(TAG, "OP18 Primary SIM initPrimarySim saved_value = " + savedValue);

        // First time entry
        if (savedValue == -1) {
            prefSummary = getDisplayNameFromMainProtocol();
            //Put value in Setting
            Settings.Global.putInt(mContext.getContentResolver(),
                                    MtkSettingsExt.Global.PRIMARY_SIM, getSubIdFromMainProtocol());

        // Fetch value from saved setting
        } else {
            Log.d(TAG, "OP18 Primary SIM initPrimarySim value shown from saved setting");
            SubscriptionInfo subInfo =
                subscriptionManager.getActiveSubscriptionInfo(savedValue);
            if (subInfo != null) {
                prefSummary = subInfo.getDisplayName();
            }

        }
        primarySimPref.setSummary(prefSummary);

        // Disable when single SIM
        final int selectableSubInfoLength = subInfoList == null ? 0 : subInfoList.size();
        int subId = subscriptionManager.getDefaultSubscriptionId();
        Log.d(TAG, "subId: " + subId + "selectableSubInfoLength: " + selectableSubInfoLength);

        if (selectableSubInfoLength == 0) {
            primarySimPref.setEnabled(false);
            primarySimPref.setSummary(mContext.getText(R.string.no_sim_card));
        } else if (selectableSubInfoLength == 1) {
            primarySimPref.setEnabled(false);
            // In case of single SIM, no harm is using the only SIM
            primarySimPref.setSummary(getDisplayNameFromMainProtocol());
        }

        //Disable if any SIM is non-USIM
        String simType1 = SystemProperties.get(SYSTEM_PROPERTY_SIM1, "");
        String simType2 = SystemProperties.get(SYSTEM_PROPERTY_SIM2, "");
        if (simType1.equals("SIM") || simType2.equals("SIM")) {
            primarySimPref.setEnabled(false);
            primarySimPref.setSummary(getDisplayNameFromMainProtocol());
        }

        mPreferenceCategoryActivities.addPreference(primarySimPref);
        //ImsServiceExtOP18 obj = new ImsServiceExtOP18(mContex
        int status = android.provider.Settings.System.getInt(
                mContext.getContentResolver(), "imssim", 1);
        Log.d("@M_" + TAG, "android.provider.Settings IMS_SIM = " + status);
        if (status == 1) {
            Log.d(TAG, "OP18 Primary SIM initPrimarySim getImsSwitchCompleteStatus is TRUE");
            primarySimPref.setEnabled(true);
        } else {
            primarySimPref.setEnabled(false);
        }
        mImsIntentFilter = new IntentFilter();
        mImsIntentFilter.addAction("com.mediatek.ims.notify_ims_switch_done");
        mHostPf.getActivity().registerReceiver(mImsReceiver, mImsIntentFilter);
    }

    /**
     * Handle primary sim selection preference.
     * @param context activity context
     */
    public void onPreferenceClick(Context context) {
        Log.d(TAG, "OP18 Primary SIM onPreferenceClick");
        String ratProtocol2 = SystemProperties.get("persist.vendor.radio.mtk_ps2_rat");
        if (ratProtocol2.equalsIgnoreCase("L/W/G")) {
            return;
        }
        final ArrayList<String> list = new ArrayList<String>();
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
        final List<SubscriptionInfo> subInfoList =
            subscriptionManager.getActiveSubscriptionInfoList();
        final int selectableSubInfoLength = subInfoList == null ? 0 : subInfoList.size();
        //final SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(context);

        final DialogInterface.OnClickListener selectionListener =
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int value) {

                        int selectedSubId;
                        selectedSubId = subInfoList.get(value).getSubscriptionId();
                        Log.d(TAG, "OP18 Primary SIM onPreferenceClick selectedSubId = "
                            + selectedSubId);
                        Log.d(TAG, "OP18 Primary SIM onPreferenceClick value = " + value);

                        int selectedSubIdFromSetting =
                         Settings.Global.getInt(mContext.getContentResolver(),
                        MtkSettingsExt.Global.PRIMARY_SIM, -1);
                        Log.d(TAG, "OP18 Primary SIM updatePrimarySim selectedSubIdFromSetting = "
                         + selectedSubIdFromSetting);

                        if (selectedSubIdFromSetting != selectedSubId) {
                        //Put value in Setting
                        Settings.Global.putInt(mContext.getContentResolver(),
                                                MtkSettingsExt.Global.PRIMARY_SIM, selectedSubId);
                        // Show on UI
                        updatePrimarySim(mHostPf);
                    }
                    }
                };


        for (int i = 0; i < selectableSubInfoLength; ++i) {
            final SubscriptionInfo sir = subInfoList.get(i);
            subInfoList.add(sir);
            CharSequence displayName = sir.getDisplayName();
            if (displayName == null) {
                displayName = "";
            }
            list.add(displayName.toString());
        }

        String[] arr = list.toArray(new String[0]);
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        ListAdapter adapter = new SelectAccountListAdapter(
                subInfoList,
                //builder.getContext(),
                context,
                R.layout.select_account_list_item,
                arr);

        builder.setTitle(mContext.getResources().getString(R.string.select_primary_sim));

        Dialog dialog = builder.setAdapter(adapter, selectionListener).create();
        dialog.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialogInterface) {
                //finish();
            }
        });
        dialog.show();

    }

    /**
     * class to show List Adapter for selecting primary SIM.
     */
    private class SelectAccountListAdapter extends ArrayAdapter<String> {
        //private Context mContext;
        private int mResId;
        private final float mOPACITY = 0.54f;
        private List<SubscriptionInfo> mSubInfoList;

        public SelectAccountListAdapter(List<SubscriptionInfo> subInfoList,
                Context context, int resource, String[] arr) {
            super(context, resource, arr);
            //mContext = context;
            mResId = resource;
            mSubInfoList = subInfoList;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            LayoutInflater inflater = LayoutInflater.from(mContext);
            View rowView;
            final ViewHolder holder;

            if (convertView == null) {
                // Cache views for faster scrolling
                rowView = inflater.inflate(mResId, null);
                holder = new ViewHolder();
                holder.mTitle = (TextView) rowView.findViewById(R.id.title);
                holder.mIcon = (ImageView) rowView.findViewById(R.id.icon);
                rowView.setTag(holder);
            } else {
                rowView = convertView;
                holder = (ViewHolder) rowView.getTag();
            }

            final SubscriptionInfo sir = mSubInfoList.get(position);
            if (sir == null) {
                holder.mTitle.setText("");
                holder.mIcon.setAlpha(mOPACITY);
            } else {
                holder.mTitle.setText(sir.getDisplayName());
                holder.mIcon.setImageBitmap(sir.createIconBitmap(mContext));
                /// M: when item numbers is over the screen, should set alpha 1.0f.
                holder.mIcon.setAlpha(1.0f);
            }
            return rowView;
        }

        /**
         * Vew Holder class to show primary SIM.
         */
        private class ViewHolder {
            TextView mTitle;
            ImageView mIcon;
        }
    }

    /**
     * Handle Priary sim subid Change.
     * @param pf Activity's Preference Fragment
     */
    public void updatePrimarySim(PreferenceFragmentCompat pf) {
        PreferenceScreen ps = pf.getPreferenceScreen();

        // Get value from setting
        int selectedSubId = Settings.Global.getInt(mContext.getContentResolver(),
            MtkSettingsExt.Global.PRIMARY_SIM, -1);
        Log.d(TAG, "OP18 Primary SIM updatePrimarySim selectedSubId = " + selectedSubId);

        // Get display name for value
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(ps.getContext());
        SubscriptionInfo subInfo =
            subscriptionManager.getActiveSubscriptionInfo(selectedSubId);
        if (subInfo != null) {
            Log.d(TAG, "OP18 Primary SIM updatePrimarySim subInfo.getDisplayName() = "
                + subInfo.getDisplayName());

            //Show display name on UI
        PreferenceCategory mPreferenceCategoryActivities =
            (PreferenceCategory) pf.findPreference("sim_activities");
        Preference primarySimPref = mPreferenceCategoryActivities.getPreference(0);
        primarySimPref.setSummary(subInfo.getDisplayName());

            //Toast.makeText(mContext, "Switching Primary SIM...Please wait", Toast.LENGTH_SHORT)
              //  .show();

            // Disable till the time we get response for enabling
            primarySimPref.setEnabled(false);
            if (ps.getContext() != null) {
                Context context = ps.getContext();
                Intent intent = new Intent(ACTION_PRIMARY_SIM_CHANGED);
                intent.putExtra(EXTRA_SUBSCRIPTION_ID, selectedSubId);
                context.sendBroadcast(new Intent(intent));
            }
        } else {
            Log.d(TAG, "OP18 Primary SIM updatePrimarySim subinfo is null");
        }
    }

    /**
     * Update Priary sim subid data.
     */
    public void subChangeUpdatePrimarySIM() {
        String ratProtocol2 = SystemProperties.get("persist.vendor.radio.mtk_ps2_rat");
        if (ratProtocol2.equalsIgnoreCase("L/W/G")) {
            return;
        }
        // Get value from setting
        int selectedSubId = Settings.Global.getInt(mContext.getContentResolver(),
            MtkSettingsExt.Global.PRIMARY_SIM, -1);
        Log.d(TAG, "OP18 Primary SIM subChangeUpdatePrimarySIM selectedSubId = " + selectedSubId);


        if (selectedSubId == -1) {
            //primarySimPref.setSummary(getDisplayNameFromMainProtocol());
            Settings.Global.putInt(mContext.getContentResolver(),
                                    MtkSettingsExt.Global.PRIMARY_SIM, getSubIdFromMainProtocol());
            Log.d(TAG, "OP18 Primary SIM subChangeUpdatePrimarySIM getSubIdFromMainProtocol() = "
                + getSubIdFromMainProtocol());
        }

        // Get value from setting
        selectedSubId = Settings.Global.getInt(mContext.getContentResolver(),
            MtkSettingsExt.Global.PRIMARY_SIM, -1);
        Log.d(TAG, "OP18 Primary SIM subChangeUpdatePrimarySIM selectedSubId again = "
            + selectedSubId);

        // Get display name for value
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        SubscriptionInfo subInfo =
            subscriptionManager.getActiveSubscriptionInfo(selectedSubId);

        //Show display name on UI
        PreferenceCategory mPreferenceCategoryActivities =
            (PreferenceCategory) mHostPf.findPreference("sim_activities");
        Preference primarySimPref = mPreferenceCategoryActivities.getPreference(0);
        if (subInfo != null) {
            Log.d(TAG, "OP18 Primary SIM subChangeUpdatePrimarySIM subInfo.getDisplayName() = "
                + subInfo.getDisplayName());
            primarySimPref.setSummary(subInfo.getDisplayName());
        } else {
            Log.d(TAG, "OP18 Primary SIM subChangeUpdatePrimarySIM subInfo is null");
        }
    }

    @Override
    /** Common method to check what prefernce has been clicked.
    * @param
    * @return
    */
    public void handleEvent(PreferenceFragmentCompat pf, Context context,
            final Preference preference) {
        if (pf.findPreference(SMART_CALL_FWD_KEY) == preference) {
            handleEventSmartCall(context);
        }
        else {
            // Other preference code can be added here
        }
}
/********************* Smart Call forward feature ***********************/

    @Override
     /** Initialize plugin with essential values.
      * @param pf preference fragment UI
      * @return
      */
    public void initPlugin(PreferenceFragmentCompat pf) {
        PreferenceScreen ps = pf.getPreferenceScreen();
        final TelecomManager telecomManager = TelecomManager.from(mContext);
        int accoutSum = telecomManager.getCallCapablePhoneAccounts().size();
        Log.d(TAG, "accoutSum:" + accoutSum);
        /* Add smart call forward preference */
        if (accoutSum > 1) {
            PreferenceCategory prefCat = new PreferenceCategory(ps.getContext());
            prefCat.setKey(SMART_CALL_PREF_CAT_KEY);
            prefCat.setTitle("General");
            ps.addPreference(prefCat);
            smartCallPref = new Preference(ps.getContext());
            smartCallPref.setKey(SMART_CALL_FWD_KEY);
            smartCallPref.setTitle(mContext.getText(R.string.smart_call_fwd_settings_title));
            smartCallPref.setSummary(mContext.getText(R.string.smart_call_fwd_settings_summary));
            prefCat.addPreference(smartCallPref);
            boolean mIsAirplaneMode = isAirplaneModeOn(mContext);
            Log.d(TAG, "Add smartCallPref mIsAirplaneMode" + mIsAirplaneMode);
            smartCallPref.setEnabled(!mIsAirplaneMode);
        }
    }

    public void handleEventSmartCall(Context context) {
        Log.d(TAG, "handleEvent");
        final Context ctx = context;
        final Intent i = new Intent(SMART_CALL_FWD_SETTINGS);
        //intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(mContext.getText(R.string.smart_call_fwd_enable_alert_dlg_title));
        builder.setMessage(mContext.getText(R.string.smart_call_fwd_alert_dlg_new));
        builder.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface dialog, int id) {
            Log.d(TAG, "launch smartCallFwd activity");
            ctx.startActivity(i);
        }
        });
        builder.setNegativeButton(android.R.string.no, null);
        builder.show();
    }

    /** updatePrefState.
     * @param enable preference
     * @return
     */
    public void updatePrefState() {
        boolean mIsAirplaneMode = isAirplaneModeOn(mContext);
        Log.d(TAG, "updatePrefState airplane status: " + mIsAirplaneMode
            + "smartCallPref" + smartCallPref);
        if (smartCallPref != null) {
            smartCallPref.setEnabled(!mIsAirplaneMode);
        }
        return;
    }

    private static boolean isAirplaneModeOn(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }
}
