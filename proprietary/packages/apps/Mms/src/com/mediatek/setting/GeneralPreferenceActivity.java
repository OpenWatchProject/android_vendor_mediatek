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

import android.Manifest;
import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.ExifInterface;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.provider.MediaStore;
import android.provider.Telephony;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.GridView;
import android.widget.SimpleAdapter;
import android.widget.Toast;


import com.android.internal.telephony.PhoneConstants;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.ui.AsyncDialog;
import com.android.mms.ui.FontSizeDialogAdapter;
import com.android.mms.ui.MessageUtils;
import com.android.mms.ui.NumberPickerDialog;
import com.android.mms.ui.UriImage;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsLog;
import com.android.mms.util.Recycler;

import com.mediatek.cb.cbsettings.CellBroadcastSettings;
import com.mediatek.mms.ext.IOpGeneralPreferenceActivityExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SimStateMonitor.SimStateListener;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;

import mediatek.telephony.MtkTelephony;

/**
 * With this activity, users can set preferences for MMS and SMS and
 * can access and manipulate SMS messages stored on the SIM.
 */
public class GeneralPreferenceActivity extends PreferenceActivity
        implements Preference.OnPreferenceChangeListener, SimStateListener {
    private static final String TAG = "GeneralPreferenceActivity";

    // Symbolic names for the keys used for preference lookup

    public static final String SHOW_EMAIL_ADDRESS = "pref_key_show_email_address";

    public static final String BACKUP_MESSAGE = "pref_key_backup_message";

    public static final String RESTORE_MESSAGE = "pref_key_restore_message";

    public static final String AUTO_DELETE = "pref_key_auto_delete";

    public static final String CELL_BROADCAST = "pref_key_cell_broadcast";

    public static final String STORAGE_SETTING = "pref_key_storage_settings";

    public static final String DISPLAY_PREFERENCE = "pref_key_display_preference_settings";

    public static final String MMS_DELETE_LIMIT = "pref_key_mms_delete_limit";

    public static final String SMS_DELETE_LIMIT = "pref_key_sms_delete_limit";

    public static final String WAPPUSH_AUTO_DOWNLOAD = "pref_key_wappush_sl_autoloading";

    public static final String WAPPUSH_SETTING = "pref_key_wappush_settings";

    public static final String WAPPUSH_ENABLED = "pref_key_wappush_enable";

    private static final String MAX_SMS_PER_THREAD = "MaxSmsMessagesPerThread";

    private static final String MAX_MMS_PER_THREAD = "MaxMmsMessagesPerThread";

    private static final String MMS_PREFERENCE = "com.android.mms_preferences";

    private IOpGeneralPreferenceActivityExt mOpGeneralPreferenceActivityExt;

    // Menu entries
    private static final int MENU_RESTORE_DEFAULTS = 1;

    private Preference mSmsLimitPref;

    private Preference mMmsLimitPref;

    private Recycler mSmsRecycler;

    private Recycler mMmsRecycler;

    private Preference mCBsettingPref;

    private Preference mFontSize;

    private AlertDialog mFontSizeDialog;

    private String[] mFontSizeChoices;

    private String[] mFontSizeValues;

    private CheckBoxPreference mShowEmailPref;

    private static final int FONT_SIZE_DIALOG = 10;

    public static final String FONT_SIZE_SETTING = "pref_key_message_font_size";

    public static final String TEXT_SIZE = "message_font_size";

    public static final int TEXT_SIZE_DEFAULT = 18;

    private Preference mCellBroadcastMultiSub;

    private NumberPickerDialog mSmsDisplayLimitDialog;

    private NumberPickerDialog mMmsDisplayLimitDialog;

    private static final String LOCATION_PHONE = "Phone";

    private static final String LOCATION_SIM = "Sim";

    private Handler mSMSHandler = new Handler();

    private Handler mMMSHandler = new Handler();

    private ProgressDialog mProgressDialog = null;

    public String SUB_TITLE_NAME = "sub_title_name";

    boolean mNeedRequestPermissins = false;
  /// M: fix bug ALPS01523754.set google+ pic as wallpaper.@{
    private AsyncDialog mAsyncDialog;
/// @}

    /// M: add for plugin
    @Override
    protected void onPause() {
        super.onPause();
        if (mSmsDisplayLimitDialog != null) {
            mSmsDisplayLimitDialog.dismiss();
        }
        if (mMmsDisplayLimitDialog != null) {
            mMmsDisplayLimitDialog.dismiss();
        }
    }

    @Override
    protected void onDestroy() {
        if (null != mProgressDialog && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
        super.onDestroy();
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        MmsLog.d(TAG, "onCreate");

        if (requestPermissions()) {
            mNeedRequestPermissins = true;
            return;
        }
        ActionBar actionBar = getActionBar();
        actionBar.setTitle(getResources().getString(R.string.actionbar_general_setting));
        actionBar.setDisplayHomeAsUpEnabled(true);
        /// M: add for plugin
        setMessagePreferences();
    }

    /// KK migration, for default MMS function. @{
    @Override
    protected void onResume() {
        super.onResume();
        if (mNeedRequestPermissins) {
            MmsLog.d(TAG, "onResume mNeedRequestPermissins " + mNeedRequestPermissins);
            return;
        }
        MmsLog.d(TAG, "onResume");
        setMultiCardPreference();
        boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
        MmsLog.d(TAG, "onResume sms enable? " + isSmsEnabled);
        if (!isSmsEnabled) {
            finish();
        }
    }
    /// @}

    @Override
    protected void onStart() {
        super.onStart();
        if (mNeedRequestPermissins) {
            MmsLog.d(TAG, "onStart mNeedRequestPermissins " + mNeedRequestPermissins);
            return;
        }
        SimStateMonitor.getInstance().addListener(this);
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (mNeedRequestPermissins) {
            MmsLog.d(TAG, "onStop mNeedRequestPermissins " + mNeedRequestPermissins);
            return;
        }
        SimStateMonitor.getInstance().removeListener(this);
    }


    private void setMessagePreferences() {
        addPreferencesFromResource(R.xml.generalpreferences);
        mShowEmailPref = (CheckBoxPreference) findPreference(SHOW_EMAIL_ADDRESS);
        PreferenceCategory displayOptions =
            (PreferenceCategory) findPreference(DISPLAY_PREFERENCE);
        displayOptions.removePreference(mShowEmailPref);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
        MmsLog.dpi(TAG, "email address check = " + sp.getBoolean(mShowEmailPref.getKey(), true));
        if (mShowEmailPref != null) {
            mShowEmailPref.setChecked(sp.getBoolean(mShowEmailPref.getKey(), true));
        }

        PreferenceCategory storageCategory = (PreferenceCategory) findPreference(STORAGE_SETTING);
        mOpGeneralPreferenceActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpGeneralPreferenceActivityExt();
        mOpGeneralPreferenceActivityExt.setMessagePreferences(
                GeneralPreferenceActivity.this, storageCategory);

        mFontSize = (Preference) findPreference(FONT_SIZE_SETTING);
        mFontSizeChoices = getResourceArray(R.array.pref_message_font_size_choices);
        mFontSizeValues = getResourceArray(R.array.pref_message_font_size_values);
        mFontSize = (Preference) findPreference(FONT_SIZE_SETTING);
        mFontSize.setSummary(mFontSizeChoices[getPreferenceValueInt(FONT_SIZE_SETTING, 0)]);

        mCBsettingPref = findPreference(CELL_BROADCAST);
        if (SimStateMonitor.getInstance().getSubCount() < 1) {
            mCBsettingPref.setEnabled(false);
        }
        mSmsLimitPref = findPreference(SMS_DELETE_LIMIT);
        mMmsLimitPref = findPreference(MMS_DELETE_LIMIT);
        if (!MmsConfig.getMmsEnabled()) {
            PreferenceCategory storageOptions
                    = (PreferenceCategory) findPreference(STORAGE_SETTING);
            storageOptions.removePreference(findPreference(MMS_DELETE_LIMIT));
        }
        enablePushSetting();
        mSmsRecycler = Recycler.getSmsRecycler();
        mMmsRecycler = Recycler.getMmsRecycler();
        // Fix up the recycler's summary with the correct values
        setSmsDisplayLimit();
        setMmsDisplayLimit();
        // Change the key to the SIM-related key, if has one SIM card, else set default value.
        if (SimStateMonitor.getInstance().getSubCount() > 1) {
            setMultiCardPreference();
        }
    }

    private void setMultiCardPreference() {
        // MTK_OP02_PROTECT_END
        if (SimStateMonitor.getInstance().getSubCount() > 1) {
            mCellBroadcastMultiSub = findPreference(CELL_BROADCAST);
        } else {
            mCBsettingPref = findPreference(CELL_BROADCAST);
            mCellBroadcastMultiSub = null;
        }
    }

    private void setMmsDisplayLimit() {
        mMmsLimitPref.setSummary(getString(R.string.pref_summary_delete_limit,
                mMmsRecycler.getMessageLimit(this)));
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        menu.clear();
        menu.add(0, MENU_RESTORE_DEFAULTS, 0, R.string.restore_default);
        return true;
    }

    private void setSmsDisplayLimit() {
        mSmsLimitPref.setSummary(getString(R.string.pref_summary_delete_limit,
                mSmsRecycler.getMessageLimit(this)));
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case MENU_RESTORE_DEFAULTS:
            restoreDefaultPreferences();
            return true;
        case android.R.id.home:
            finish();
            return true;
        default:
            break;
        }
        return false;
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        if (preference == mSmsLimitPref) {
            mSmsDisplayLimitDialog = new NumberPickerDialog(this, mSmsLimitListener,
                    mSmsRecycler.getMessageLimit(this), mSmsRecycler.getMessageMinLimit(),
                    mSmsRecycler.getMessageMaxLimit(), R.string.pref_title_sms_delete);
            mSmsDisplayLimitDialog.show();
        } else if (preference == mCellBroadcastMultiSub) {
            Intent it = new Intent();
            it.setClass(this, SubSelectActivity.class);
            it.putExtra(SmsPreferenceActivity.PREFERENCE_KEY, preference.getKey());
            it.putExtra(SmsPreferenceActivity.PREFERENCE_TITLE_ID, R.string.cell_broadcast);
            startActivity(it);
        } else if (preference == mMmsLimitPref) {
            mMmsDisplayLimitDialog = new NumberPickerDialog(this, mMmsLimitListener,
                    mMmsRecycler.getMessageLimit(this), mMmsRecycler.getMessageMinLimit(),
                    mMmsRecycler.getMessageMaxLimit(), R.string.pref_title_mms_delete);
            mMmsDisplayLimitDialog.show();
        } else if (preference == mCBsettingPref) {
            if (SimStateMonitor.getInstance().getSubCount() == 0) {
                MmsLog.d(TAG, "there is no sim card");
                return true;
            }
            int subId = SimStateMonitor.getInstance().getSubInfoList().get(0).getSubscriptionId();
            MmsLog.d(TAG, "mCBsettingPref subId is : " + subId);

            Intent it = new Intent();
            it.setClass(this, CellBroadcastSettings.class);
            it.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
            it.putExtra(SUB_TITLE_NAME, SimStateMonitor.getInstance()
                    .getSubInfoList().get(0).getDisplayName().toString());
            startActivity(it);

        } else if (preference == mFontSize) {
            showDialog(FONT_SIZE_DIALOG);
        }
        return super.onPreferenceTreeClick(preferenceScreen, preference);
    }

    private void restoreDefaultPreferences() {
        SharedPreferences.Editor editor
                = PreferenceManager.getDefaultSharedPreferences(
                        GeneralPreferenceActivity.this).edit();
        editor.putInt(FONT_SIZE_SETTING, 0);
        editor.putFloat(TEXT_SIZE, Float.parseFloat(mFontSizeValues[0]));
        editor.putBoolean(AUTO_DELETE, false);
        editor.putInt(MAX_SMS_PER_THREAD, MmsConfig.getDefaultSMSMessagesPerThread());
        editor.putInt(MAX_MMS_PER_THREAD, MmsConfig.getDefaultMMSMessagesPerThread());
        editor.putBoolean(CELL_BROADCAST, false);
        /// M: fix bug ALPS00759844, WAPPUSH_ENABLED should be true.
        editor.putBoolean(WAPPUSH_ENABLED, true);
        editor.putBoolean(WAPPUSH_AUTO_DOWNLOAD, false);
        /// M: fix bug ALPS00432361, restore default preferences
        /// about GroupMms and ShowEmailAddress @{
        editor.putBoolean(SHOW_EMAIL_ADDRESS, true);
        /// @}
        editor.apply();
        setPreferenceScreen(null);
        setMessagePreferences();
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case FONT_SIZE_DIALOG:
            FontSizeDialogAdapter adapter = new FontSizeDialogAdapter(
                    GeneralPreferenceActivity.this, mFontSizeChoices, mFontSizeValues);
            mFontSizeDialog = new AlertDialog.Builder(GeneralPreferenceActivity.this).setTitle(
                R.string.message_font_size_dialog_title).setNegativeButton(
                        R.string.message_font_size_dialog_cancel,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        mFontSizeDialog.dismiss();
                    }
                }).setSingleChoiceItems(adapter, getFontSizeCurrentPosition(),
                        new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    SharedPreferences.Editor editor =
                        PreferenceManager.getDefaultSharedPreferences(
                        GeneralPreferenceActivity.this).edit();
                    editor.putInt(FONT_SIZE_SETTING, which);
                    editor.putFloat(TEXT_SIZE, Float.parseFloat(mFontSizeValues[which]));
                    editor.apply();
                    mFontSizeDialog.dismiss();
                    mFontSize.setSummary(mFontSizeChoices[which]);
                }
            }).create();
            mFontSizeDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
                public void onDismiss(DialogInterface dialog) {
                    GeneralPreferenceActivity.this.removeDialog(FONT_SIZE_DIALOG);
                }
            });
            return mFontSizeDialog;
        }
        return super.onCreateDialog(id);
    }

    /*
     * Notes: if wap push is not support, wap push setting should be removed
     */
    private void enablePushSetting() {
        PreferenceCategory wapPushOptions = (PreferenceCategory) findPreference(WAPPUSH_SETTING);
        if (FeatureOption.MTK_WAPPUSH_SUPPORT) {
            if (!MmsConfig.getSlAutoLanuchEnabled()) {
                wapPushOptions.removePreference(findPreference(WAPPUSH_AUTO_DOWNLOAD));
            }
        } else {
            if (getPreferenceScreen() != null) {
                getPreferenceScreen().removePreference(wapPushOptions);
            }
        }
    }

    @Override
    public boolean onPreferenceChange(Preference arg0, Object arg1) {
        // / for Sms&Mms
        return true;
    }

    private CharSequence getVisualTextName(
            String enumName, int choiceNameResId, int choiceValueResId) {
        CharSequence[] visualNames = getResources().getTextArray(choiceNameResId);
        CharSequence[] enumNames = getResources().getTextArray(choiceValueResId);
        // Sanity check
        if (visualNames.length != enumNames.length) {
            return "";
        }
        for (int i = 0; i < enumNames.length; i++) {
            if (enumNames[i].equals(enumName)) {
                return visualNames[i];
            }
        }
        return "";
    }

    NumberPickerDialog.OnNumberSetListener mSmsLimitListener
            = new NumberPickerDialog.OnNumberSetListener() {
        public void onNumberSet(int limit) {
            if (limit <= mSmsRecycler.getMessageMinLimit()) {
                limit = mSmsRecycler.getMessageMinLimit();
            } else if (limit >= mSmsRecycler.getMessageMaxLimit()) {
                limit = mSmsRecycler.getMessageMaxLimit();
            }
            mSmsRecycler.setMessageLimit(GeneralPreferenceActivity.this, limit);
            setSmsDisplayLimit();
            if (mProgressDialog == null || !mProgressDialog.isShowing()) {
                mProgressDialog = ProgressDialog.show(
                        GeneralPreferenceActivity.this, "", getString(R.string.deleting), true);
            }
            mSMSHandler.post(new Runnable() {
                public void run() {
                    new Thread(new Runnable() {
                        public void run() {
                            Recycler.getSmsRecycler().deleteOldMessages(getApplicationContext());
                            if (FeatureOption.MTK_WAPPUSH_SUPPORT) {
                                Recycler.getWapPushRecycler().deleteOldMessages(
                                        getApplicationContext());
                            }
                            if (null != mProgressDialog && mProgressDialog.isShowing()) {
                                mProgressDialog.dismiss();
                            }
                        }
                    }, "DeleteSMSOldMsgAfterSetNum").start();
                }
            });
        }
    };

    NumberPickerDialog.OnNumberSetListener mMmsLimitListener
            = new NumberPickerDialog.OnNumberSetListener() {
        public void onNumberSet(int limit) {
            if (limit <= mMmsRecycler.getMessageMinLimit()) {
                limit = mMmsRecycler.getMessageMinLimit();
            } else if (limit >= mMmsRecycler.getMessageMaxLimit()) {
                limit = mMmsRecycler.getMessageMaxLimit();
            }
            mMmsRecycler.setMessageLimit(GeneralPreferenceActivity.this, limit);
            setMmsDisplayLimit();
            if (mProgressDialog == null || !mProgressDialog.isShowing()) {
                mProgressDialog = ProgressDialog.show(
                        GeneralPreferenceActivity.this, "", getString(R.string.deleting),
                    true);
            }
            mMMSHandler.post(new Runnable() {
                public void run() {
                    new Thread(new Runnable() {
                        public void run() {
                            MmsLog.d("Recycler", "mMmsLimitListener");
                            Recycler.getMmsRecycler().deleteOldMessages(getApplicationContext());
                            if (null != mProgressDialog && mProgressDialog.isShowing()) {
                                mProgressDialog.dismiss();
                            }
                        }
                    }, "DeleteMMSOldMsgAfterSetNum").start();
                }
            });
        }
    };

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        MmsLog.dpi(TAG, "onConfigurationChanged: newConfig = " + newConfig + ",this = " + this);
        super.onConfigurationChanged(newConfig);
        setTheme(R.style.MmsTheme);
//        this.getListView().clearScrapViewsIfNeeded();
    }
    // MTK_OP01_PROTECT_START
    private String[] getFontSizeArray(int resId) {
        return getResources().getStringArray(resId);
    }

    private int getFontSizeCurrentPosition() {
        SharedPreferences sp = getSharedPreferences(MMS_PREFERENCE, MODE_PRIVATE);
        return sp.getInt(FONT_SIZE_SETTING, 0);
    }

    // MTK_OP01_PROTECT_END
    private void showToast(int id) {
        Toast t = Toast.makeText(getApplicationContext(), getString(id), Toast.LENGTH_SHORT);
        t.show();
    }

    private int getPreferenceValueInt(String key, int defaultValue) {
        SharedPreferences sp = getSharedPreferences(
                "com.android.mms_preferences", MODE_PRIVATE);
        return sp.getInt(key, defaultValue);
    }

    private String[] getResourceArray(int resId) {
        return getResources().getStringArray(resId);
    }

    @Override
    public void onSimStateChanged() {
        MmsLog.d(TAG, "onSimStateChanged");
        if (SimStateMonitor.getInstance().getSubCount() < 1) {
            mCBsettingPref.setEnabled(false);
        } else {
            mCBsettingPref.setEnabled(true);
            setMultiCardPreference();
        }
    }


    private boolean isPermissionGranted() {
         String[] permissions = new String[] {
                    Manifest.permission.READ_SMS,
                    Manifest.permission.WRITE_SMS};
         for (int i = 0; i < permissions.length; i++) {
            if (!PermissionCheckUtil.hasPermission(this, permissions[i])) {
                // Show toast
                if (PermissionCheckUtil.isRequiredPermission(permissions[i])
                        || PermissionCheckUtil.isNeverGrantedPermission(this, permissions[i])) {
                    if (!PermissionCheckUtil.isPermissionChecking()) {
                        PermissionCheckUtil.showNoPermissionsToast(this);
                    }
                }
                MmsLog.d(TAG, "isPermissionGranted return false");
                return false;
            }
        }
        MmsLog.d(TAG, "isPermissionGranted return true");
        return true;
    }

    private boolean requestPermissions() {
        String[] permissions = new String[] {
                    Manifest.permission.READ_SMS,
                    Manifest.permission.WRITE_SMS};

        final ArrayList<String> missingList;
        missingList = PermissionCheckUtil.getMissingPermissions(this, permissions);

        if (missingList.size() == 0) {
        MmsLog.d(TAG, "Permission granted for General activity");
            return false;
        }
        final String[] missingArray = new String[missingList.size()];
        missingList.toArray(missingArray);
        MmsLog.d(TAG, "Request permissions start!");
        PermissionCheckUtil.setPermissionActivityCount(true);
        requestPermissions(missingArray,
            PermissionCheckUtil.REQUIRED_PERMISSIONS_REQUEST_CODE);
        return true;
    }

    @Override
    public void onRequestPermissionsResult(
            final int requestCode, final String permissions[], final int[] grantResults) {
        MmsLog.d(TAG, " onRequestPermissionsResult Activity Count: "
                + PermissionCheckUtil.sPermissionsActivityStarted);
        PermissionCheckUtil.setPermissionActivityCount(false);
        if (isPermissionGranted()) {
            mNeedRequestPermissins = false;
            ActionBar actionBar = getActionBar();
            actionBar.setTitle(getResources().getString(R.string.actionbar_general_setting));
            actionBar.setDisplayHomeAsUpEnabled(true);
            setMessagePreferences();
            SimStateMonitor.getInstance().addListener(this);
            boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
            MmsLog.d(TAG, "onResume sms enable? " + isSmsEnabled);
            if (!isSmsEnabled) {
               finish();
            }
        } else {
            finish();
        }
    }
}
