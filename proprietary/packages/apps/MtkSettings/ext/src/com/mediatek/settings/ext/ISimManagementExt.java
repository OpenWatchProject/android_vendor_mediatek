package com.mediatek.settings.ext;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telecom.PhoneAccountHandle;
import android.telephony.SubscriptionInfo;
import android.view.View;
import android.widget.ImageView;

import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

import java.util.ArrayList;
import java.util.List;

public interface ISimManagementExt {

    /**
     * Called when SimSettings fragment onResume
     * @internal
     */
    void onResume(Context context);

    /**
     * Called when SimSettings fragment onPause
     * @internal
     */
    void onPause();

    /**
     * hide SIM color view.
     * @param view view
     * @param context Context
     * @internal
     */
    void hideSimEditorView(View view, Context context);

    /**
     *Show change data connection dialog.
     *
     * @Param PreferenceFragmentCompat
     * @Param isResumed
     * @internal
     */
    void showChangeDataConnDialog(PreferenceFragmentCompat prefFragment,
            boolean isResumed);

    /**
     * Called when update SMS summary
     * @internal
     */
    void updateDefaultSmsSummary(Preference pref);

    /**
     * Called before setDefaultDataSubId
     * @param subid
     * @internal
     */
    void setDataState(int subId);

    /**
     * Called after setDefaultDataSubId
     * @param subid
     * @internal
     */
    void setDataStateEnable(int subId);

    /**
     * Called when create list for "pick default call sub" or "pick default sms sub"
     * @param strings  default summary list
     * @internal
     */
    void customizeListArray(List<String> strings);

    /**
     * Called when create list for "pick default call sub" or "pick default sms sub"
     * @param strings  default sub list
     * @internal
     */
    void customizeSubscriptionInfoArray(List<SubscriptionInfo> subscriptionInfo);

    /**
     * Called when SIM dialog is about to show for SIM info changed
     * @return false if plug-in do not need SIM dialog
     * @internal
     */
    boolean isSimDialogNeeded();

    /**
     * Called when update mobile network settings enable state, to check whether used CT
     * Test SIM
     * @return
     * @internal
     */
    boolean useCtTestcard();

    /**
     * Called when set radio power state for a specific sub
     * @param subId  the slot to set radio power state
     * @param turnOn  on or off
     * @internal
     */
    void setRadioPowerState(int subId, boolean turnOn);

    /**
     * Called when set default subId for sms or data
     * @param context
     * @param sir
     * @param type sms type or data type
     * @return
     * @internal
     */
    SubscriptionInfo setDefaultSubId(Context context, SubscriptionInfo sir, String type);

    /**
     * Called when set default phoneAccount for call
     * @param phoneAccount
     * @return
     * @internal
     */
    PhoneAccountHandle setDefaultCallValue(PhoneAccountHandle phoneAccount);

    /**
     * configSimPreferenceScreen.
     * @param simPref simPref
     * @param type type
     * @param size size
     * @internal
     */
    void configSimPreferenceScreen(Preference simPref, String type, int size);

    /**
     * updateList.
     * @param list list to add the string
     * @param smsSubInfoList type
     * @param selectableSubInfoLength size
     * @internal
     */
    public void updateList(final ArrayList<String> list,
            ArrayList<SubscriptionInfo> smsSubInfoList, final int selectableSubInfoLength);

    /**
     * simDialogOnClick.
     * @param id type of sim prefrence
     * @param value value of position selected
     * @param context context
     * @return handled by plugin or not
     * @internal
     */
    public boolean simDialogOnClick(int id, int value, Context context);

    /**
     * setCurrNetworkIcon.
     * @param icon imagview to be filled
     * @param id type of sim prefrence
     * @param position value of position selected
     * @internal
     */
    public void setCurrNetworkIcon(ImageView icon, int id, int position);

    /**
     * setPrefSummary.
     * @param simPref sim prefrence
     * @param type type of sim prefrence
     * @internal
     */
    public void setPrefSummary(Preference simPref, String type);

     /** Initialize plugin with essential values.
     * @param pf PreferenceFragmentCompat
     * @return
     */
    public void initPlugin(PreferenceFragmentCompat pf);

    /** handleEvent.
     * @param context setting context
     * @return
     */
    public void handleEvent(PreferenceFragmentCompat pf, Context context, final Preference preference);

    /** updatePrefState.
     * @param enable preference
     * @return
     */
    public void updatePrefState();

    /**
     * Called when SimSettings fragment onDestroy
     * @internal
     */
    void onDestroy();

    /**
     * Called when SimSettings fragment onCreate
     */
    void onCreate();

    /**
     * for receive broadcast.
     * @param mIntentFilter sim selectService
     */
    void customRegisteBroadcast(IntentFilter mIntentFilter);

    /**
     * for dual action broadcast.
     * @param intent
     */
    void customBroadcast(Intent intent);

    /**
     * for opeator required, maybe update main capability
     * @param isChecked  when radio off means false
     * @param subid subId
     */
    void customizeMainCapabily(boolean isChecked, int subId);

    /**
     * Judge whether or not reserve the ask first item.
     * @return true if operator want to follow the host flow.
     * @internal
     */
    boolean isNeedAskFirstItemForSms();

    /**
     * Get default SMS click content.
     * @param subInfoList SubscriptionInfo
     * @param value Value
     * @param subId Subid
     * @return subId.
     * @internal
     */
    int getDefaultSmsClickContentExt(final List<SubscriptionInfo> subInfoList,
            int value, int subId);


    /** Initialize Primary Sim preference.
     * @param pf PreferenceFragmentCompat
     * @return
     */
    public void initPrimarySim(PreferenceFragmentCompat pf);

    /** Perform operation if preference is selected.
     * @param context UI context
     * @return
     */
    public void onPreferenceClick(Context context);

    /** Perform operation Primary SIM change.
     * @return
     */
    public void subChangeUpdatePrimarySIM();

    /**
     * For China telecom bug fix.
     * when insert two sim card,
     * use sim1 call, and radio off sim2
     * because CT C project , when radio off sim2,
     * radio will set default data to sim1,
     * will cause problem, so add this interface for disable simsetting items
     * @return true if CT C project
     */
    public boolean customizeSimCardForPhoneState();

    /**
     * For China telecom bug fix.
     * when insert two sim card,
     * use sim1 call, and radio off sim2
     * because CT C project , when radio off sim2,
     * radio will set default data to sim1,
     * will cause problem, so add this interface for disable simsetting items
     * @return true if OM load, CT C load need judgement is calling or not.
     */
    public boolean customizeCallStateNotInCall();
}
