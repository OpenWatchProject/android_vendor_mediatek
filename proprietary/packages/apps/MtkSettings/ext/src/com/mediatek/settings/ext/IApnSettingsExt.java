package com.mediatek.settings.ext;

import java.util.ArrayList;

import android.R.integer;
import android.app.Activity;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.view.Menu;

import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

public interface IApnSettingsExt {

    int RESUME = 0;
    int PAUSE = 1;

    /**
     * Called when ApnSettings fragment or ApnEditor fragment onDestory
     * @internal
     */
    void onDestroy();

    /**
     * Called when ApnSettings created
     * @param pref
     * @internal
     */
    void initTetherField(PreferenceFragmentCompat pref);

    /**
     * whether allowed to edit the present apn, called at fill apn list,
     * in ApnSettings
     * @param subId sub id
     * @param type the apn type
     * @param apn name to query
     * @param sourcetype
     * @return if the specified apn could be edited.
     * @internal
     */
     boolean isAllowEditPresetApn(int subId, String type, String apn, int sourcetype);

    /**
     * customize Tethering APN setting, called at init Tether settings in TetherSettins
     * @param root through root PreferenceScreen, you can customize your preference.
     * @internal
     */
    void customizeTetherApnSettings(PreferenceScreen root);

    /**
     * Called when forge a query string for APN
     * @param where the original query string from host
     * @param mccmnc current mccmnc used in query string
     * @return the original string where, or a customized query string by plug-in
     * @internal
     */
    String getFillListQuery(String where, String mccmnc);

    /**
     * Called when ApnSettings update prefer carrier uri
     * @param subId which sub on updating
     * @return
     * @internal
     */
    Uri getPreferCarrierUri(Uri defaultUri, int subId);

    /**
     * Called when ApnSettings fragment onResume
     * @internal
     */
    void updateTetherState();

    /**
     * set APN type preference state (enable or disable), called at update UI in
     * ApnEditor
     * @param preference
     *            The preference to set state
     * @internal
     */
    void setApnTypePreferenceState(Preference preference, String apnType);

    /**
     * Called when ApnEditor insert a new APN item
     * @param defaultUri the Uri host used
     * @param context
     * @param intent
     * @return inserted Uri, return a customized one if necessary or just return the default Uri
     * @internal
     */
    Uri getUriFromIntent(Uri defaultUri, Context context, Intent intent);

    /**
     * Called when host forge the APN type array
     * @param defaultApnArray the default array host used
     * @param context
     * @param apnType
     * @return a customized array if necessary, or just return the defaultApnArray
     * @internal
     */
    String[] getApnTypeArray(String[] defaultApnArray, Context context, String apnType);

    /**
     * Judge the APN can be selected or not, called in ApnSettings.
     * @param type the apn type
     * @internal
     */
    boolean isSelectable(String type);

    /**
     * add intent extra for start ApnEditor.
     * @param it  the intent to start ApnEditor
     * @internal
     */
    void addApnTypeExtra(Intent it);

    /**
     * judge the screen if enable or disable.
     * @param subId sub id
     * @param activity for the screen
     * @return true screen should enable.
     * @return false screen should disable.
     * @internal
     */
    boolean getScreenEnableState(int subId, Activity activity);

    /**
     * Called when host update the screen state
     * @param subId
     * @param sourceType
     * @param root
     * @param apnType
     * @internal
     */
    void updateFieldsStatus(int subId, int sourceType, PreferenceScreen root, String apnType);

    /**
     * set the preference text and summary according to the slotId.
     * called at update UI, in ApnEditor.
     * @param subId sub id.
     * @param text the text for the preference
     * @internal
     */
    void setPreferenceTextAndSummary(int subId, String text);

    /**
     * customize Preference according to the slotId. called at onCreate once.
     * in ApnEditor.
     * @param subId sub id
     * @param root though the root PreferenceScreen , you can customize any preference
     * @internal
     */
     void customizePreference(int subId, PreferenceScreen root);

     /**
      * customize apn projection, such as add Telephony.Carriers.PPP
      * Called at onCreate in ApnEditor.
      * @param projection: the default source String[]
      * @return customized projection
      * @internal
      */
     String[] customizeApnProjection(String[] projection);

     /**
      * save the added apn values called when save the added apn vaule,
      * in ApnEditor.
      * @param contentValues the default content vaules
      * @internal
      */
     void saveApnValues(ContentValues contentValues);

   /**
    * for CT feature ,update CTWAP,CTNET display.
    * @param name
    * @param source type
    * @internal
    */
    public String updateApnName(String name, int sourcetype);

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
     * @internal
     */
    long replaceApn(long defaultReplaceNum, Context context, Uri uri, String apn, String name,
            ContentValues values, String numeric);

    /**
     * For CT A feature, clear the 46011 mms and (mms,supl).
     * @param type mms, or (mms,supl) or (default,mms,supl),etc
     * @param mvnoType mvno type
     * @param mvnoMatchData mvno data value
     * @param mnoApnList the unselectable mno apn list
     * @param mvnoApnList the unselectable mvno apn list
     * @param subId
     * maybe can remove it from Q, china telecom used for apn
     * @internal
     */
    void customizeUnselectableApn(String type, String mvnoType,
            String mvnoMatchData, Object mnoApnList,
            Object mvnoApnList, int subId);

    /**
     * set MVNO preference state (enable or disable), called at
     * update UI in ApnEditor
     * @param preference The preference to set state
     * @internal
     */
    void setMvnoPreferenceState(Preference mvnoType, Preference mvnoMatchData);

    /**
     * for CU feature ,if sort APN by the name.
     * @param order sort by the name.
     * @return the sort string.
     * @internal
     */
    public String getApnSortOrder(String order);

    /**
     * To get mcc&mnc from IMPI, see TelephonyManagerEx.getIsimImpi().
     * @param defaultValue default value.
     * @param phoneId phoneId used to get IMPI.
     * @return
     */
    String getOperatorNumericFromImpi(String defaultValue, int phoneId);

    /**
     * customer need user can editor apn
     * @param subId subid
     * @return
     */
    boolean customerUserEditable(int subId);

    /**
     * Should select first APN when reset.
     * @return
     */
    boolean shouldSelectFirstApn();

    /**
     * Apn Settings event.
     * @param event resume/pause
     */
    void onApnSettingsEvent(int event);

    /**
     * For China telecom feature.
     * remove ctwap apn,
     * remove ims apn if volte support.
     * @param subId subid for judge is China telecom sim card or not
     * @param apnType for judge apn type
     * @param apnList apnlist common settings use ArrayList<ApnPreference>
     */
    void customizeApnState(int subId, String apnType, Object apnList);
}
