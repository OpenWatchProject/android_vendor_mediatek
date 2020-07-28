/*
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.setting;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.Bundle;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.telephony.SubscriptionInfo;
import android.text.InputFilter;
import android.view.Menu;
import android.view.MenuItem;
import android.view.inputmethod.EditorInfo;
import android.widget.EditText;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.ui.MessageUtils;
import com.android.mms.ui.SmsTemplateEditActivity;
import com.android.mms.util.MmsLog;
import com.mediatek.mms.ext.IOpSmsPreferenceActivityExt;
import com.mediatek.mms.ext.IOpSubSelectActivityExt;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SimStateMonitor.SimStateListener;
import com.mediatek.simmessage.ManageSimMessages;

import java.util.List;

import mediatek.telephony.MtkSmsManager;

/**
 * With this activity, users can set preferences for MMS and SMS and can access
 * and manipulate SMS messages stored on the SIM.
 */
public class SmsPreferenceActivity extends PreferenceActivity implements
        Preference.OnPreferenceChangeListener, SimStateListener {
    private static final String TAG = "SmsPreferenceActivity";

    private static final boolean DEBUG = false;

    public static final String SMS_DELIVERY_REPORT_MODE = "pref_key_sms_delivery_reports";

    public static final String SMS_QUICK_TEXT_EDITOR = "pref_key_quick_text_editor";

    public static final String SMS_SERVICE_CENTER = "pref_key_sms_service_center";

    public static final String SMS_MANAGE_SIM_MESSAGES = "pref_key_manage_sim_messages";

    public static final String SMS_SAVE_LOCATION = "pref_key_sms_save_location";

    public static final String SMS_INPUT_MODE = "pref_key_sms_input_mode";

    public static final String SMS_SETTINGS = "pref_key_sms_settings";

    public static final String PREFERENCE_KEY = "PREFERENCE_KEY";

    public static final String PREFERENCE_TITLE_ID = "PREFERENCE_TITLE";

    public static final String SETTING_SAVE_LOCATION = "Phone";

    // / M: fix bug ALPS00437648, restoreDefaultPreferences in tablet
    public static final String SETTING_SAVE_LOCATION_TABLET = "Device";

    private static final String MMS_PREFERENCE = "com.android.mms_preferences";

    public static final String SDCARD_DIR_PATH = "//sdcard//message//";

    // Menu entries
    private static final int MENU_RESTORE_DEFAULTS = 1;

    private static final int MAX_EDITABLE_LENGTH = 20;

    private Preference mSmsQuickTextEditorPref;

    private Preference mManageSimPref;

    private Preference mSmsServiceCenterPref;

    // MTK_OP01_PROTECT_END
    // all preferences need change key for single sim card
    private CheckBoxPreference mSmsDeliveryReport;

    private CheckBoxPreference mSmsForwardWithSender;

    // all preferences need change key for multiple sim card
    private Preference mSmsDeliveryReportMultiSim;

    private Preference mSmsServiceCenterPrefMultiSim;

    private Preference mManageSimPrefMultiSim;

    private Preference mSmsSaveLoactionMultiSim;

    private ListPreference mSmsLocation;

    private ListPreference mSmsInputMode;

    private static final String LOCATION_PHONE = "Phone";

    private static final String LOCATION_SIM = "Sim";

    private EditText mNumberText;

    private AlertDialog mNumberTextDialog;

    // / M: fix bug ALPS00455172, add tablet "device" support
    private static final String DEVICE_TYPE = "pref_key_device_type";

    ///M: plug-in for Op
    private IOpSmsPreferenceActivityExt mOpSmsPreferenceActivityExt;
    private IOpSubSelectActivityExt mOpSubSelectActivityExt;

    @Override
    protected void onResume() {
        super.onResume();
        // / KK migration, for default MMS function. @{
        boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
        MmsLog.d(TAG, "onResume sms enable? " + isSmsEnabled);
        if (!isSmsEnabled) {
            finish();
            return;
        }
        // / @}
        setListPrefSummary();
    }

    private void setListPrefSummary() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        // / M: fix bug ALPS00455172, add tablet "device" support
        SharedPreferences.Editor editor = sp.edit();
        if (!getResources().getBoolean(R.bool.isTablet)) {
            editor.putString(DEVICE_TYPE, "Phone");
        } else {
            editor.putString(DEVICE_TYPE, "Device");
        }
        editor.commit();

        // For mSmsLocation;
        String saveLocation = "Phone";

        List<SubscriptionInfo> subInfos = SimStateMonitor.getInstance().getSubInfoList();
        if (subInfos != null && subInfos.size() == 1) {
            int subId = subInfos.get(0).getSubscriptionId();
            saveLocation = sp.getString((Long.toString(subId) + "_" + SMS_SAVE_LOCATION), "Phone");
        }

        if (getResources().getBoolean(R.bool.isTablet)) {
            // /@ M: fix bug ALPS00823808, avoid empty of the sms stroe lactaion
            // at the first time.
            if ("Phone".equals(saveLocation)) {
                saveLocation = "Device";
            }
            mSmsLocation.setSummary(MessageUtils.getVisualTextName(this, saveLocation,
                    R.array.pref_tablet_sms_save_location_choices,
                    R.array.pref_tablet_sms_save_location_values));
            // /@
        } else {
            mSmsLocation.setSummary(MessageUtils.getVisualTextName(this, saveLocation,
                    R.array.pref_sms_save_location_choices,
                    R.array.pref_sms_save_location_values));
        }
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        MmsLog.d(TAG, "onCreate");
        mOpSmsPreferenceActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpSmsPreferenceActivityExt();
        mOpSubSelectActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpSubSelectActivityExt();
        mOpSmsPreferenceActivityExt.onCreate(this);
        ActionBar actionBar = getActionBar();
        actionBar.setTitle(getResources().getString(R.string.actionbar_sms_setting));
        actionBar.setDisplayHomeAsUpEnabled(true);
        setMessagePreferences();
        SimStateMonitor.getInstance().addListener(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mNumberTextDialog != null && mNumberTextDialog.isShowing()) {
            mNumberTextDialog.dismiss();
        }
        SimStateMonitor.getInstance().removeListener(this);
    }

    private void setMessagePreferences() {
        MmsLog.dpi(TAG, "setMessagePreferences SubInfoList :"
                + SimStateMonitor.getInstance().getSubInfoList()
                + " SubCount is :" + SimStateMonitor.getInstance().getSubCount());

        if (SimStateMonitor.getInstance().getSubCount() <= 1) {
            addPreferencesFromResource(R.xml.smspreferences);
        } else {
            addPreferencesFromResource(R.xml.smsmulticardpreferences);
        }
        mSmsLocation = (ListPreference) findPreference(SMS_SAVE_LOCATION);
        mSmsLocation.setOnPreferenceChangeListener(this);
        mSmsQuickTextEditorPref = findPreference(SMS_QUICK_TEXT_EDITOR);
        ///M: do not show sim messages when sim is off. @{
        PreferenceCategory smsCategory = (PreferenceCategory) findPreference(SMS_SETTINGS);
        mManageSimPref = findPreference(SMS_MANAGE_SIM_MESSAGES);
        if (!MessageUtils.isSimMessageAccessable(this)) {
            // If there is no SIM, this item will be disabled and can not be
            // accessed.
            mManageSimPref.setEnabled(false);
        }
        /// @}

        if (SimStateMonitor.getInstance().getSubCount() <= 1) {
            mSmsServiceCenterPref = findPreference(SMS_SERVICE_CENTER);

            // MTK_OP02_PROTECT_END
            if (SimStateMonitor.getInstance().getSubCount() == 0) {
                mSmsDeliveryReport = (CheckBoxPreference) findPreference(SMS_DELIVERY_REPORT_MODE);
                mSmsDeliveryReport.setEnabled(false);
                mSmsServiceCenterPref.setEnabled(false);
                // / M: fix bug ALPS00455172, add tablet "device" support
                if (!getResources().getBoolean(R.bool.isTablet)) {
                    mSmsLocation.setValue("Phone");
                } else {
                    mSmsLocation.setValue("Device");
                }
                mSmsLocation.setEnabled(false);
            } else {
                MmsLog.d(TAG, "single sim");
                changeSingleCardKeyToSimRelated();
            }
        } else {
            setMultiCardPreference();
        }

        addSmsInputModePreference();
        mOpSmsPreferenceActivityExt.setMessagePreferences(SmsPreferenceActivity.this, smsCategory,
                SimStateMonitor.getInstance().getSubCount());

    }

    // add input mode setting for op03 request, if not remove it.
    private void addSmsInputModePreference() {
        if (!mOpSmsPreferenceActivityExt.addSmsInputModePreference(this)) {
            PreferenceCategory smsCategory
                    = (PreferenceCategory) findPreference("pref_key_sms_settings");
            mSmsInputMode = (ListPreference) findPreference(SMS_INPUT_MODE);
            if (mSmsInputMode != null) {
                smsCategory.removePreference(mSmsInputMode);
            }
        }
    }

    private void changeSingleCardKeyToSimRelated() {
        // get to know which one
        if (SimStateMonitor.getInstance().getSubCount() == 0) {
            return;
        }
        SubscriptionInfo singleCardInfo = null;
        singleCardInfo = SimStateMonitor.getInstance().getSubInfoList().get(0);

        if (singleCardInfo == null) {
            return;
        }
        int subId = singleCardInfo.getSubscriptionId();
        MmsLog.d(TAG, "changeSingleCardKeyToSimRelated Got simId = " + subId);
        // translate all key to SIM-related key;
        mSmsDeliveryReport = (CheckBoxPreference) findPreference(SMS_DELIVERY_REPORT_MODE);
        mSmsServiceCenterPref = findPreference(SMS_SERVICE_CENTER);
        mManageSimPref = findPreference(SMS_MANAGE_SIM_MESSAGES);

        mManageSimPrefMultiSim = null;
        PreferenceCategory smsCategory
                = (PreferenceCategory) findPreference("pref_key_sms_settings");

        mSmsLocation = (ListPreference) findPreference(SMS_SAVE_LOCATION);
        mSmsLocation.setKey(Long.toString(subId) + "_" + SMS_SAVE_LOCATION);
        String [] saveLocationDisp = mOpSubSelectActivityExt.setSaveLocation();
        if (saveLocationDisp != null) {
            mSmsLocation.setEntries(saveLocationDisp);
        }

        SharedPreferences spr = getSharedPreferences("com.android.mms_preferences",
                MODE_PRIVATE);
        // /M fix bug ALPS00837998;
        if (!getResources().getBoolean(R.bool.isTablet)) {
            mSmsLocation.setValue(spr.getString((Long.toString(subId) + "_" + SMS_SAVE_LOCATION),
                    "Phone"));
        } else {
            mSmsLocation.setValue(spr.getString((Long.toString(subId) + "_" + SMS_SAVE_LOCATION),
                    "Device"));
        }
        // /@

        // MTK_OP02_PROTECT_END

        // get the stored value
        SharedPreferences sp = getSharedPreferences(MMS_PREFERENCE, MODE_PRIVATE);
        if (mSmsDeliveryReport != null) {
            mSmsDeliveryReport.setKey(Long.toString(subId) + "_" + SMS_DELIVERY_REPORT_MODE);
            mSmsDeliveryReport.setChecked(sp.getBoolean(mSmsDeliveryReport.getKey(), false));
        }

        mOpSmsPreferenceActivityExt.changeSingleCardKeyToSimRelated();
    }

    private void setMultiCardPreference() {
        mSmsDeliveryReportMultiSim = findPreference(SMS_DELIVERY_REPORT_MODE);
        mSmsServiceCenterPrefMultiSim = findPreference(SMS_SERVICE_CENTER);
        mManageSimPrefMultiSim = findPreference(SMS_MANAGE_SIM_MESSAGES);

        mManageSimPref = null;
        PreferenceCategory smsCategory = (PreferenceCategory) findPreference(SMS_SETTINGS);

        if (mSmsLocation != null) {
            smsCategory.removePreference(mSmsLocation);
            Preference saveLocationMultiSim = new Preference(this);
            saveLocationMultiSim.setKey(SMS_SAVE_LOCATION);
            saveLocationMultiSim.setTitle(R.string.sms_save_location);
            saveLocationMultiSim.setSummary(R.string.sms_save_location);
            smsCategory.addPreference(saveLocationMultiSim);
            mSmsSaveLoactionMultiSim = findPreference(SMS_SAVE_LOCATION);
        }

        mSmsDeliveryReportMultiSim.setKey(SMS_DELIVERY_REPORT_MODE);
        mOpSmsPreferenceActivityExt.setMultiCardPreference();
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.clear();
        menu.add(0, MENU_RESTORE_DEFAULTS, 0, R.string.restore_default);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                // The user clicked on the Messaging icon in the action bar.
                // Take them back from
                // wherever they came from
                finish();
                return true;
            case MENU_RESTORE_DEFAULTS:
                restoreDefaultPreferences();
                return true;
            default:
                break;
        }
        return false;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        if (preference == mManageSimPref) {
            if (SimStateMonitor.getInstance().getSubCount() != 0) {
                int subId = SimStateMonitor
                        .getInstance().getSubInfoList().get(0).getSubscriptionId();
                MmsLog.d(TAG, "slotId is : " + subId);
                if (subId != -1) {
                    Intent it = new Intent();
                    it.setClass(this, ManageSimMessages.class);
                    it.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
                    startActivity(it);
                }
            }
        } else if (preference == mSmsQuickTextEditorPref) {
            Intent intent = new Intent();
            intent.setClass(this, SmsTemplateEditActivity.class);
            startActivity(intent);
        } else if (preference == mSmsDeliveryReportMultiSim) {
            Intent it = new Intent();
            it.setClass(this, SubSelectActivity.class);
            it.putExtra(PREFERENCE_KEY, preference.getKey());
            it.putExtra(PREFERENCE_TITLE_ID, R.string.pref_title_sms_delivery_reports);
            startActivity(it);
        } else if (preference == mSmsServiceCenterPref) {
            if (SimStateMonitor.getInstance().getSubCount() == 0) {
                MmsLog.d(TAG, "there is no sim card");
                return true;
            }
            new Thread(new Runnable() {
                public void run() {
                    int id = SimStateMonitor.getInstance().getSubInfoList()
                            .get(0).getSubscriptionId();
                    MmsLog.d(TAG, "onPreferenceTreeClick slotId is : " + id);
                    final Bundle result = MtkSmsManager.getSmsManagerForSubscriptionId(id)
                            .getScAddressWithErroCode();
                    runOnUiThread(new Runnable() {
                        public void run() {
                            if (result != null && result.getByte(
                                    MtkSmsManager.GET_SC_ADDRESS_KEY_RESULT)
                                            == MtkSmsManager.ERROR_CODE_NO_ERROR) {
                                // Means Success to get SC Address
                                AlertDialog.Builder dialog = new AlertDialog.Builder(
                                        SmsPreferenceActivity.this);
                                mNumberText = new EditText(dialog.getContext());
                                mNumberText.setHint(R.string.type_to_compose_text_enter_to_send);
                                mNumberText.computeScroll();
                                mNumberText.setFilters(new InputFilter[] {
                                        new InputFilter.LengthFilter(MAX_EDITABLE_LENGTH) });
                                mNumberText.setInputType(
                                        EditorInfo.TYPE_CLASS_TEXT | EditorInfo.TYPE_CLASS_PHONE);
                                String gotScNumber = (String) result.getCharSequence(
                                        MtkSmsManager.GET_SC_ADDRESS_KEY_ADDRESS);
                                MmsLog.d(TAG, "gotScNumber is: " + gotScNumber);

                                mNumberText.setText(gotScNumber);
                                mNumberText.setTextColor(R.color.black);
                                if (!SmsPreferenceActivity.this.isFinishing()) {
                                    mNumberTextDialog = dialog.setIcon(
                                        R.drawable.ic_dialog_info_holo_light).setTitle(
                                        R.string.sms_service_center).setView(mNumberText)
                                        .setPositiveButton(R.string.OK,
                                                new PositiveButtonListener()).setNegativeButton(
                                                        R.string.Cancel,
                                                        new NegativeButtonListener()).show();
                                }
                            } else if (result != null && result.getByte(
                                    MtkSmsManager.GET_SC_ADDRESS_KEY_RESULT)
                                            == MtkSmsManager.ERROR_CODE_NO_SUPPORT_SC_ADDR) {
                                Toast.makeText(getApplicationContext(),
                                        getString(R.string.cdma_not_support),
                                        Toast.LENGTH_SHORT).show();
                                MmsLog.dpi(TAG, "getScAddress not support error: " + result);
                            } else {
                                MmsLog.dpi(TAG, "getScAddress error: " + result);
                                showToast(R.string.sms_not_ready);
                            }
                        }
                    });
                }
            }).start();

        } else if (preference == mSmsServiceCenterPrefMultiSim
                || preference == mManageSimPrefMultiSim
                || (preference == mSmsSaveLoactionMultiSim
                        && SimStateMonitor.getInstance().getSubCount() > 1)) {
            Intent it = new Intent();
            it.setClass(this, SubSelectActivity.class);
            it.putExtra(PREFERENCE_KEY, preference.getKey());
            if (preference == mSmsServiceCenterPrefMultiSim) {
                it.putExtra(PREFERENCE_TITLE_ID, R.string.sms_service_center);
            } else if (preference == mManageSimPrefMultiSim) {
                it.putExtra(PREFERENCE_TITLE_ID, R.string.pref_title_manage_sim_messages);
            } else if (preference == mSmsSaveLoactionMultiSim) {
                it.putExtra(PREFERENCE_TITLE_ID, R.string.sms_save_location);
            }
            startActivity(it);
        }
        mOpSmsPreferenceActivityExt.onPreferenceTreeClick(preferenceScreen, preference);
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    // / M: added for bug ALPS00314789 begin
    private boolean isValidAddr(String address) {
        boolean ret = true;
        if (address.isEmpty()) {
            return ret;
        }
        if (address.charAt(0) == '+') {
            for (int i = 1, count = address.length(); i < count; i++) {
                if (address.charAt(i) < '0' || address.charAt(i) > '9') {
                    ret = false;
                    break;
                }
            }
        } else {
            for (int i = 0, count = address.length(); i < count; i++) {
                if (address.charAt(i) < '0' || address.charAt(i) > '9') {
                    ret = false;
                    break;
                }
            }
        }
        return ret;
    }

    // / M: added for bug ALPS00314789 end
    private class PositiveButtonListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int which) {
            // write to the SIM Card.
            // / M: added for bug ALPS00314789 begin
            if (!isValidAddr(mNumberText.getText().toString())) {
                String num = mNumberText.getText().toString();
                String strUnSpFormat = getResources().getString(R.string.unsupported_media_format,
                        "");
                Toast.makeText(getApplicationContext(), strUnSpFormat, Toast.LENGTH_SHORT).show();
                return;
            }
            // / M: added for bug ALPS00314789 end
            final int subId;
            subId = SimStateMonitor.getInstance().getSubInfoList().get(0).getSubscriptionId();
            new Thread(new Runnable() {
                public void run() {
                    MtkSmsManager.getSmsManagerForSubscriptionId(subId).setScAddress(
                            mNumberText.getText().toString());
                }
            }).start();
        }
    }

    private class NegativeButtonListener implements OnClickListener {
        public void onClick(DialogInterface dialog, int which) {
            // cancel
            dialog.dismiss();
        }
    }

    private void restoreDefaultPreferences() {
        SharedPreferences.Editor editor = PreferenceManager.getDefaultSharedPreferences(
                SmsPreferenceActivity.this).edit();
        int subCount = SimStateMonitor.getInstance().getSubCount();
        if (subCount > 0) {
            for (int i = 0; i < subCount; i++) {
                int subId = SimStateMonitor.getInstance().getSubInfoList().get(i)
                        .getSubscriptionId();
                editor.putBoolean(Long.toString(subId) + "_" + SMS_DELIVERY_REPORT_MODE, false);
                // / M: fix bug ALPS00437648, restoreDefaultPreferences
                // in tablet
                if (!getResources().getBoolean(R.bool.isTablet)) {
                    editor.putString(Long.toString(subId) + "_" + SMS_SAVE_LOCATION,
                            SETTING_SAVE_LOCATION);
                } else {
                    editor.putString(Long.toString(subId) + "_" + SMS_SAVE_LOCATION,
                            SETTING_SAVE_LOCATION_TABLET);
                }
            }
        }

        mOpSmsPreferenceActivityExt.restoreDefaultPreferences(this, editor);
        editor.apply();
        setPreferenceScreen(null);
        setMessagePreferences();
        setListPrefSummary();
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object arg1) {
        if (mOpSmsPreferenceActivityExt.onPreferenceChange(preference, arg1)) {
            return true;
        }

        final String key = preference.getKey();
        if (preference == mSmsLocation && !(SimStateMonitor.getInstance().getSubCount() > 1)) {
            if (!getResources().getBoolean(R.bool.isTablet)) {
                mSmsLocation.setSummary(MessageUtils.getVisualTextName(this, (String) arg1,
                        R.array.pref_sms_save_location_choices,
                        R.array.pref_sms_save_location_values));
            } else {
                mSmsLocation.setSummary(MessageUtils.getVisualTextName(this, (String) arg1,
                        R.array.pref_tablet_sms_save_location_choices,
                        R.array.pref_tablet_sms_save_location_values));
            }
        }

        return true;
    }

    private void showToast(int id) {
        Toast t = Toast.makeText(getApplicationContext(), getString(id), Toast.LENGTH_SHORT);
        t.show();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        MmsLog.d(TAG, "onConfigurationChanged: newConfig = " + newConfig + ",this = " + this);
        super.onConfigurationChanged(newConfig);
        // for Migration change,maybe need check
//        (this.getListView()).clearScrapViewsIfNeeded();
    }

    @Override
    public void onSimStateChanged() {
        MmsLog.d(TAG, "onSimStateChanged");
        Dialog locationDialog = mSmsLocation.getDialog();
        if (mNumberTextDialog != null && mNumberTextDialog.isShowing()) {
            mNumberTextDialog.dismiss();
        }
        if (locationDialog != null && locationDialog.isShowing()) {
            locationDialog.dismiss();
        }
        setPreferenceScreen(null);
        setMessagePreferences();
        setListPrefSummary();
    }

    public static String getSmsSaveLocation(Context context, int subId) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        // in tablet
        if (!context.getResources().getBoolean(R.bool.isTablet)) {
            return sp.getString((Long.toString(subId) + "_" + SMS_SAVE_LOCATION),
                    "Phone");
        } else {
            return sp.getString((Long.toString(subId) + "_" + SMS_SAVE_LOCATION),
                    "Device");
        }

    }
}
