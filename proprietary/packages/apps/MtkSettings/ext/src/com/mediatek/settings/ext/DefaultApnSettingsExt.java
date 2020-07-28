package com.mediatek.settings.ext;

import java.util.ArrayList;

import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.view.Menu;

import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;


public class DefaultApnSettingsExt implements IApnSettingsExt {
    private static final String TAG = "DefaultApnSettingsExt";

    /**
     * Called when ApnSettings fragment or ApnEditor fragment onDestory
     */
    @Override
    public void onDestroy() {
    }

    @Override
    public void initTetherField(PreferenceFragmentCompat pref) {
    }

    /** the default implementation is not null ,so when operator part
     *  extends this class to over write it , must need to think that
     *  whether call super class's implementation or not.
     */
     @Override
    public boolean isAllowEditPresetApn(int subId, String type, String apn, int sourcetype) {
        Log.d(TAG, "isAllowEditPresetApn");
        return true;
    }

    @Override
    public void customizeTetherApnSettings(PreferenceScreen root) {
    }

    /**
     * Called when forge a query string for APN
     * @param where the original query string from host
     * @param mccmnc current mccmnc used in query string
     * @return the original string where, or a customized query string by plug-in
     */
    @Override
    public String getFillListQuery(String where, String mccmnc) {
        return where;
    }

    /**
     * Called when ApnSettings fragment onResume
     */
    @Override
    public void updateTetherState() {
    }

    /**
     * Called when ApnSettings update prefer carrier Uri
     * @param defaultUri the original Uri of host
     * @param subId which sub on updating
     * @return
     */
    @Override
    public Uri getPreferCarrierUri(Uri defaultUri, int subId) {
        return defaultUri;
    }

    /**
     * set APN type preference state (enable or disable), called at update UI in
     * ApnEditor
     * @param preference
     *            The preference to set state
     */
    @Override
    public void setApnTypePreferenceState(Preference preference, String apnType) {
    }

    /**
     * Called when ApnEditor insert a new APN item
     * @param defaultUri the Uri host used
     * @param context
     * @param intent
     * @return inserted Uri, return a customized one if necessary or just return the default Uri
     */
    @Override
    public Uri getUriFromIntent(Uri defaultUri, Context context, Intent intent) {
        return defaultUri;
    }

    /**
     * Called when host forge the APN type array
     * @param defaultApnArray the default array host used
     * @param context
     * @param apnType
     * @return a customized array if necessary, or just return the defaultApnArray
     */
    @Override
    public String[] getApnTypeArray(String[] defaultApnArray, Context context, String apnType) {
        return defaultApnArray;
    }

    @Override
    public boolean isSelectable(String type) {
        return true;
    }

    @Override
    public boolean getScreenEnableState(int subId, Activity activity) {
        return true;
    }

    @Override
    public void addApnTypeExtra(Intent it) {
    }

    /**
     * Called when host update the screen state
     * @param subId
     * @param sourceType
     * @param root
     * @param apnType
     */
    @Override
    public void updateFieldsStatus(int subId, int sourceType,
            PreferenceScreen root, String apnType) {
    }

    @Override
    public void setPreferenceTextAndSummary(int subId, String text) {
    }

    @Override
    public void customizePreference(int subId, PreferenceScreen root) {
    }

    @Override
    public String[] customizeApnProjection(String[] projection) {
        return projection;
    }

    @Override
    public void saveApnValues(ContentValues contentValues) {
    }

    @Override
    public String updateApnName(String name, int sourcetype){
        return name;
    }

    /**
     * Called when OmacpApnReceiver update APN
     * @param defaultReplacedNum
     * @param context
     * @param uri
     * @param apn
     * @param name
     * @param values
     * @param numeric
     * @return
     */
    @Override
    public long replaceApn(long defaultReplaceNum, Context context, Uri uri, String apn,
            String name, ContentValues values, String numeric) {
        return defaultReplaceNum;
    }

    /**
     * For CT A feature, clear the 46011 mms and (mms,supl).
     *
     * @param type mms, or (mms,supl) or (default,mms,supl),etc
     * @param mvnoType mvno type
     * @param mvnoMatchData mvno data value
     * @param mnoApnList the unselectable mno apn list
     * @param mvnoApnList the unselectable mvno apn list
     * @param subId
     */
    @Override
    public void customizeUnselectableApn(String type,
            String mvnoType,
            String mvnoMatchData,
            Object mnoApnList,
            Object mvnoApnList, int subId) {
    }

    /**
     * set MVNO preference state (enable or disable), called at
     * update UI in ApnEditor
     * @param preference The preference to set state
     */
    @Override
    public void setMvnoPreferenceState(Preference mvnoType, Preference mvnoMatchData) {
    }

    /**
     * for CU feature ,if sort APN by the name.
     * @param order sort by the name.
     * @return the sort string.
     */
    @Override
    public String getApnSortOrder(String order) {
        return order;
    }

    /**
     * To get mcc&mnc from IMPI, see TelephonyManagerEx.getIsimImpi().
     * @param defaultValue default value.
     * @param phoneId phoneId used to get IMPI.
     * @return
     */
    @Override
    public String getOperatorNumericFromImpi(String defaultValue, int phoneId) {
        return defaultValue;
    }

    @Override
    public boolean customerUserEditable(int subId) {
        return true;
    }
    
    /**
     * Should select first APN when reset.
     * @return
     */
    public boolean shouldSelectFirstApn() {
        Log.d(TAG, "shouldSelectFirstApn");
        return true;
    }

    /**
     * Apn Settings event.
     * @param event resume/pause
     */
    public void onApnSettingsEvent(int event) {
        Log.d(TAG, "onApnSettingsEvent");
    }

    /**
     * For China telecom feature.
     * remove ctwap apn,
     * remove ims apn if volte support.
     * @param subId subid for judge is China telecom sim card or not
     * @param apnType for judge apn type
     * @param apnList apnlist common settings use ArrayList<ApnPreference>
     */
    @Override
    public void customizeApnState(int subId, String apnType, Object apnList) {
    }
}
