/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.nfc;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import androidx.preference.Preference;
import androidx.preference.PreferenceScreen;
import androidx.preference.Preference.OnPreferenceClickListener;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.widget.Toast;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.SettingsPreferenceFragment;
import com.mediatek.nfcsettingsadapter.ServiceEntry;
import com.mediatek.settings.FeatureOption;

import java.util.List;


public class NfcServiceStatus extends SettingsPreferenceFragment implements OnPreferenceClickListener{
    private static final String TAG = "NfcServiceStatus";
    private static final String KEY_EDIT_MODE = "nfcEditMode";
    private static final int MENU_ID_EDIT = Menu.FIRST + 1;
    private static final int MENU_ID_OK = Menu.FIRST + 2;
    private Context mContext;
    private Menu mMenu;
    private boolean mEditMode;
    private NfcServiceHelper mNfcServiceHelper;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = getActivity();
        mNfcServiceHelper = new NfcServiceHelper(mContext);
        addPreferencesFromResource(R.xml.nfc_service_status);
        setHasOptionsMenu(true);
    }

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.NFC_PAYMENT;
    }

    @Override
    public void onResume() {
        super.onResume();
        refreshUi(true);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        final MenuItem menuEdit = menu.add(Menu.NONE, MENU_ID_EDIT, 0, null);
        menuEdit.setShowAsAction(MenuItem. SHOW_AS_ACTION_ALWAYS);
        menuEdit.setIcon(R.drawable.ic_edit);
        final MenuItem menuOK = menu.add(Menu.NONE, MENU_ID_OK, 0, R.string.okay);
        menuOK.setShowAsAction(MenuItem. SHOW_AS_ACTION_ALWAYS);
        super.onCreateOptionsMenu(menu, inflater);
        mMenu = menu;
        updateVisibilityOfMenu();
    }

    private void updateVisibilityOfMenu() {
        if (mMenu == null) {
            return;
        }
        final MenuItem menuEdit = mMenu.findItem(MENU_ID_EDIT);
        final MenuItem menuOK = mMenu.findItem(MENU_ID_OK);
        if (menuEdit != null && menuOK != null) {
            menuEdit.setVisible(!mEditMode);
            menuOK.setVisible(mEditMode);
        }
    }

   @Override
    public boolean onOptionsItemSelected(MenuItem menuItem) {
        switch (menuItem.getItemId()) {
            case MENU_ID_EDIT:
                setEditMode(true);
                refreshUi(false);
                return true;
            case MENU_ID_OK:
                mNfcServiceHelper.saveChange();
                setEditMode(false);
                refreshUi(false);
                return true;
            case android.R.id.home:
                if (mEditMode) {
                    setEditMode(false);
                    refreshUi(false);
                    return true;
                }
                break;
        }
        return super.onOptionsItemSelected(menuItem);
    }

    private void setEditMode(boolean editMode) {
        mEditMode = editMode;
        updateVisibilityOfMenu();
        mNfcServiceHelper.setEditMode(editMode);
    }

    private void refreshUi(boolean needRestore) {
        Log.d(TAG, "refreshUi, mEditMode = " + mEditMode + ", needRestore = " + needRestore);
        getPreferenceScreen().removeAll();
        mNfcServiceHelper.initServiceList();
        mNfcServiceHelper.sortList();
        if (needRestore) {
            mNfcServiceHelper.restoreCheckedState();
        }
        initPreferences(mNfcServiceHelper.getServiceList());
    }

    private void initPreferences(List<ServiceEntry> list) {
        for (ServiceEntry serviceEntry : list) {
            getPreferenceScreen().addPreference(createPreference(serviceEntry));
        }
    }

    private NfcServicePreference createPreference (ServiceEntry serviceEntry) {
        NfcServicePreference pref = new NfcServicePreference(mContext, serviceEntry);
        if (mEditMode) {
            pref.setOnPreferenceClickListener(this);
            pref.setShowCheckbox(true);
        } else {
            if (serviceEntry.getWasEnabled().booleanValue()) {
                pref.setEnabled(true);
                pref.setSummary(R.string.nfc_service_summary_enabled);
            } else {
                pref.setEnabled(false);
                pref.setSummary(R.string.nfc_service_summary_disabled);
            }
        }
        return pref;
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (preference instanceof NfcServicePreference) {
            NfcServicePreference nfcPreference = (NfcServicePreference) preference;
            boolean isChecked = nfcPreference.isChecked();
            Log.d(TAG, "onPreferenceClick, isChecked =" + isChecked);
            if(mNfcServiceHelper.setEnabled(nfcPreference, !isChecked)) {
                nfcPreference.setChecked(!isChecked);
            } else {
                Toast.makeText(mContext, R.string.nfc_service_overflow, Toast.LENGTH_SHORT).show();
            }
        }
        return false;
    }

    @Override
    public void onViewStateRestored(Bundle savedInstanceState) {
        super.onViewStateRestored(savedInstanceState);
        if (savedInstanceState != null) {
            mEditMode = savedInstanceState.getBoolean(KEY_EDIT_MODE, false);
            Log.d(TAG, "onViewStateRestored mEditMode = " + mEditMode);
            setEditMode(mEditMode);
            mNfcServiceHelper.restoreState(savedInstanceState);
        }
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        Log.d(TAG, "onSaveInstanceState, mEditMode = " + mEditMode);
        outState.putBoolean(KEY_EDIT_MODE, mEditMode);
        mNfcServiceHelper.saveState(outState);
    }
}
