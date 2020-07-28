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

public class DefaultSimManagementExt implements ISimManagementExt {

    @Override
    public void onResume(Context context) {
    }

    public void onPause() {
    }

    public void updateSimEditorPref(PreferenceFragmentCompat pref) {
        return;
    }

    @Override
    public void updateDefaultSmsSummary(Preference pref) {
    }

    @Override
    public void showChangeDataConnDialog(PreferenceFragmentCompat prefFragment, boolean isResumed) {
        return;
    }

    @Override
    public void hideSimEditorView(View view, Context context) {
    }

    /**
     * Called before setDefaultDataSubId
     * @param subid
     */
    @Override
    public void setDataState(int subId) {
    }

    /**
     * Called after setDefaultDataSubId
     * @param subid
     */
    @Override
    public void setDataStateEnable(int subId) {
    }

    @Override
    public void customizeListArray(List<String> strings){
    }

    @Override
    public void customizeSubscriptionInfoArray(List<SubscriptionInfo> subscriptionInfo){
    }

    /**
     * Called when SIM dialog is about to show for SIM info changed
     * @return false if plug-in do not need SIM dialog
     */
    public boolean isSimDialogNeeded() {
        return true;
    }

    @Override
    public boolean useCtTestcard() {
        return false;
    }

    /**
     * Called when set radio power state for a specific sub
     * @param subId  the slot to set radio power state
     * @param turnOn  on or off
     */
    public void setRadioPowerState(int subId, boolean turnOn) {
    }

    /**
     * Called when set default subId for sms or data
     * @param context
     * @param sir
     * @param type sms type or data type
     * @return
     */
    public SubscriptionInfo setDefaultSubId(Context context, SubscriptionInfo sir, String type) {
        return sir;
    }

    /**
     * Called when set default phoneAccount for call
     * @param phoneAccount
     * @return
     */
    public PhoneAccountHandle setDefaultCallValue(PhoneAccountHandle phoneAccount) {
        return phoneAccount;
    }

    /**
     * config SimPreferenceScreen.
     * @param simPref simPref
     * @param type type
     * @param size size
     */
    public void configSimPreferenceScreen(Preference simPref, String type, int size) {
    }

    /**
     * updateList.
     * @param list list to add the string
     * @param smsSubInfoList type
     * @param selectableSubInfoLength size
     */
    public void updateList(final ArrayList<String> list,
            ArrayList<SubscriptionInfo> smsSubInfoList, final int selectableSubInfoLength) {
    }

    /**
     * simDialogOnClick.
     * @param id type of sim prefrence
     * @param value value of position selected
     * @param context context
     * @return handled by plugin or not
     */
    public boolean simDialogOnClick(int id, int value, Context context) {
        return false;
    }

    /**
     * setCurrNetworkIcon.
     * @param icon imagview to be filled
     * @param id type of sim prefrence
     * @param position value of position selected
     */
    public void setCurrNetworkIcon(ImageView icon, int id, int position){
    }

    /**
     * setPrefSummary.
     * @param simPref sim prefrence
     * @param type type of sim prefrence
     */
    public void setPrefSummary(Preference simPref, String type){
    }
     /** Initialize plugin with essential values.
     * @param pf PreferenceFragmentCompat
     * @return
     */
    public void initPlugin(PreferenceFragmentCompat pf) {
        return;
    }

    /** handleEvent.
     * @param context service Context
     * @return
     */
    public void handleEvent(PreferenceFragmentCompat pf, Context context, final Preference preference) {
        return;
    }

   /** updatePrefState.
     * @param enable preference
     * @return
     */
    public void updatePrefState() {
        return;
    }

    @Override
    public void onDestroy() {
    }

    @Override
    public void onCreate() {
    }

    @Override
    public void customBroadcast(Intent intent) {
    }
    @Override
    public void customRegisteBroadcast(IntentFilter mIntentFilter) {
    }

    @Override
    public void customizeMainCapabily(boolean isChecked, int subId) {
    }

    @Override
    public boolean isNeedAskFirstItemForSms() {
        return true;
    }

    @Override
    public int getDefaultSmsClickContentExt(final List<SubscriptionInfo> subInfoList,
            int value, int subId) {
        return subId;
    }

    @Override
    public void initPrimarySim(PreferenceFragmentCompat pf) {
    }

    @Override
    public void onPreferenceClick(Context context) {
    }

    @Override
    public void subChangeUpdatePrimarySIM() {
    }

    @Override
    public boolean customizeSimCardForPhoneState() {
        return false;
    }

    @Override
    public boolean customizeCallStateNotInCall() {
        return true;
    }
}