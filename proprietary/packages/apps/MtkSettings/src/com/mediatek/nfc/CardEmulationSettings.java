/*
 * Copyright (C) 2011 The Android Open Source Project
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceScreen;
import androidx.preference.PreferenceViewHolder;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.RadioButton;
import android.widget.Switch;
import android.widget.TextView;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.core.InstrumentedFragment;
import com.android.settings.R;
import com.android.settings.SettingsActivity;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.widget.SwitchBar;

import com.mediatek.provider.MtkSettingsExt;

import java.util.ArrayList;
import java.util.List;

public class CardEmulationSettings extends SettingsPreferenceFragment implements
    Preference.OnPreferenceChangeListener, SwitchBar.OnSwitchChangeListener {
    private static final String TAG = "CardEmulationSettings";

    private static final String DEFAULT_MODE = "SIM1";
    private static final String CATEGORY_KEY = "card_emulation_settings_category";
    private String EMULATION_OFF = null;

    private CardEmulationProgressCategory mProgressCategory;
    private TextView mEmptyView;
    private SecurityItemPreference mActivePref;
    //private Switch mActionBarSwitch;
    private SwitchBar mSwitchBar;
    private final List<SecurityItemPreference> mItemPreferences =
        new ArrayList<SecurityItemPreference>();

    private final List<String> mItemKeys = new ArrayList<String>();
    private boolean mUpdateStatusOnly = false;
    private IntentFilter mIntentFilter;

    /**
     * The broadcast receiver is used to handle RF_FIELD status changed.
     */
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (NfcSettings.ACTION_RF_FIELD_ON_DETECTED.equals(action)) {
                getPreferenceScreen().setEnabled(false);
                Log.d("@M_" + TAG, "Receive broadcast: RF field on detected");
            } else if (NfcSettings.ACTION_RF_FIELD_OFF_DETECTED.equals(action)) {
                getPreferenceScreen().setEnabled(true);
                Log.d("@M_" + TAG, "Receive broadcast: RF field off detected");
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.card_emulation_settings);

        mProgressCategory = (CardEmulationProgressCategory) findPreference(CATEGORY_KEY);
        getCardEmulationList();
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(NfcSettings.ACTION_RF_FIELD_ON_DETECTED);
        mIntentFilter.addAction(NfcSettings.ACTION_RF_FIELD_OFF_DETECTED);
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);



        mEmptyView = (TextView) getView().findViewById(android.R.id.empty);
        setEmptyView(mEmptyView);
    }

    @Override
    public void onStart() {
        super.onStart();

        Log.d("@M_" + TAG, "onCreate, mSwitchBar addOnSwitchChangeListener ");
        // On/off switch
        final SettingsActivity activity = (SettingsActivity) getActivity();
        mSwitchBar = activity.getSwitchBar();
        mSwitchBar.addOnSwitchChangeListener(this);
        mSwitchBar.show();
    }

    @Override
    public void onStop() {
        super.onStop();

        Log.d("@M_" + TAG, "onStop, mSwitchBar removeOnSwitchChangeListener ");
        mSwitchBar.removeOnSwitchChangeListener(this);
        mSwitchBar.hide();
    }

    @Override
    public void onSwitchChanged(Switch switchView, boolean desiredState) {
        Log.d("@M_" + TAG, "onCheckedChanged, desiredState " + desiredState
                + " mUpdateStatusOnly " + mUpdateStatusOnly);
        //turn off card emulation, set the active mode off and clear the screen
        if (mUpdateStatusOnly) {
            return;
        }
        if (!desiredState) {
            Settings.Global.putString(getContentResolver(), MtkSettingsExt.Global.NFC_MULTISE_ACTIVE,
                    EMULATION_OFF);
            Log.d("@M_" + TAG, "onCheckedChanged, "
                    + "set Settings.Global.NFC_MULTISE_ACTIVE EMULATION_OFF"
                    + EMULATION_OFF);
        } else {
            //set the active mode is the list first elment and add preference
            String previousMode = Settings.Global.getString(getContentResolver(),
                    MtkSettingsExt.Global.NFC_MULTISE_PREVIOUS);
            if (previousMode == null) {
                String[] emulationList = getCardEmulationList();
                previousMode = (emulationList.length > 0) ? emulationList[0] : DEFAULT_MODE;
            }
            Settings.Global.putString(getContentResolver(), MtkSettingsExt.Global.NFC_MULTISE_ACTIVE,
                    previousMode);
            Log.d("@M_" + TAG, "onCheckedChanged, set active mode to " + previousMode);
        }
        mSwitchBar.setEnabled(false);
    }

    private void removeAll() {
        mProgressCategory.removeAll();
        getPreferenceScreen().removeAll();
        mProgressCategory.setProgress(false);
        mItemPreferences.clear();
        mItemKeys.clear();
    }
    /**
     * update the preference according to the status of NfcAdapter settings.
     */
    private void updatePreferences() {

        removeAll();

        String activeMode = Settings.Global.getString(getContentResolver(),
                MtkSettingsExt.Global.NFC_MULTISE_ACTIVE);
        String previousMode = Settings.Global.getString(getContentResolver(),
                MtkSettingsExt.Global.NFC_MULTISE_PREVIOUS);
        int transactionStatus = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.NFC_MULTISE_IN_TRANSACTION, 0);
        int switchingStatus = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.NFC_MULTISE_IN_SWITCHING, 0);
        Log.d("@M_" + TAG, "updatePreferences(),EMULATION_OFF " + EMULATION_OFF
                + ", active mode: " + activeMode + ", previous mode is " + previousMode);
        Log.d("@M_" + TAG, "updatePreferences, transactionStatus is " + transactionStatus
                + " switchingStatus is " + switchingStatus);

        if (EMULATION_OFF.equals(activeMode)) {
            mUpdateStatusOnly = true;
            mSwitchBar.setChecked(false);
            mUpdateStatusOnly = false;
            if (getCardEmulationList().length == 0) {
                Log.d("@M_" + TAG, "no available security elment found "
                        + "and the active mode is off");

                mEmptyView.setText(R.string.card_emulation_settings_no_element_found);
            } else {
                if (switchingStatus == 0) {
                    mEmptyView.setText(R.string.card_emulation_settings_off_text);
                } else {
                    mEmptyView.setText(R.string.card_emulation_turning_off_text);
                }
            }
            mSwitchBar.setEnabled(transactionStatus == 0 && switchingStatus == 0);
        } else {
            mUpdateStatusOnly = true;
            mSwitchBar.setChecked(true);
            mUpdateStatusOnly = false;
            if (switchingStatus == 1 && EMULATION_OFF.equals(previousMode)) {
                mSwitchBar.setEnabled(false);
                mEmptyView.setText(R.string.card_emulation_turning_on_text);
            } else {
                mSwitchBar.setEnabled(transactionStatus == 0 && switchingStatus == 0);
                addItemPreference();
                int prefCount = mProgressCategory.getPreferenceCount();
                getPreferenceScreen().addPreference(mProgressCategory);
                SecurityItemPreference itemPref =
                        (SecurityItemPreference) findPreference(activeMode);
                if (itemPref != null) {
                    itemPref.setChecked(true);
                    mActivePref = itemPref;
                } else {
                    Log.d("@M_" + TAG, "Activie mode is " + activeMode
                            + ", can not find it on screen");
                }
                mProgressCategory.setProgress(switchingStatus == 1);
                mProgressCategory.setEnabled(transactionStatus == 0 && switchingStatus == 0);
            }
        }
        int fieldActive = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.NFC_RF_FIELD_ACTIVE, 0);
        getPreferenceScreen().setEnabled(fieldActive == 0);
        Log.d("@M_" + TAG, "Read the value Global.NFC_RF_FIELD_ACTIVE : " + fieldActive);
    }

    private void addItemPreference() {
        String[] list = getCardEmulationList();
        if (list != null) {
            for (String key : list) {
                SecurityItemPreference pref = new SecurityItemPreference(getActivity());
                pref.setTitle(key);
                pref.setKey(key);
                pref.setOnPreferenceChangeListener(this);
                mProgressCategory.addPreference(pref);

                mItemPreferences.add(pref);
                mItemKeys.add(key);
            }
        }
    }

    /**
     * parse the card emulation list.
     */
    private String[] getCardEmulationList() {
        String list = Settings.Global.getString(getContentResolver(),
                MtkSettingsExt.Global.NFC_MULTISE_LIST);
        String delims = "[,]";
        String[] tokens = list.split(delims);
        int length = tokens.length;
        Log.d("@M_" + TAG, "getCardEmulationList, length = " + length);
        if (EMULATION_OFF == null) {
            EMULATION_OFF = tokens[length - 1];
            Log.d("@M_" + TAG, "EMULATION_OFF is " + EMULATION_OFF);
        }
        String[] emulationList = new String[length - 1];
        if (tokens != null) {
            for (int i = 0; i < tokens.length - 1 ; i++) {
                emulationList[i] = tokens[i];
                Log.d("@M_" + TAG, "emulation list item is " + emulationList[i]);
            }
        }
        return emulationList;
    }

    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (preference != null && preference instanceof SecurityItemPreference) {
            Log.d("@M_" + TAG, "onPreferenceChange, select " + preference.getKey() + " active");
            Settings.Global.putString(getContentResolver(), MtkSettingsExt.Global.NFC_MULTISE_ACTIVE,
                    preference.getKey());
            mProgressCategory.setProgress(true);
            mSwitchBar.setEnabled(false);
            for (SecurityItemPreference pref : mItemPreferences) {
                pref.setEnabled(false);
            }
            return true;
        }
        return false;
    }

    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        if (preference != null && preference instanceof SecurityItemPreference) {
            Log.d("@M_" + TAG, "onPreferenceTreeClick " + preference.getKey());
            String activeMode = Settings.Global.getString(getContentResolver(),
                    MtkSettingsExt.Global.NFC_MULTISE_ACTIVE);
            String prefKey = preference.getKey();
            if (prefKey != null && !(prefKey.equals(activeMode))) {
                Settings.Global.putString(getContentResolver(), MtkSettingsExt.Global.NFC_MULTISE_ACTIVE,
                        preference.getKey());
                mProgressCategory.setProgress(true);
                mSwitchBar.setEnabled(false);
                for (SecurityItemPreference pref : mItemPreferences) {
                    pref.setEnabled(false);
                }
                return true;
            }
        }
        return false;
    }

    public void onResume() {
        super.onResume();
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(
                MtkSettingsExt.Global.NFC_MULTISE_ACTIVE), false, mActiveCardModeObserver);
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(
                MtkSettingsExt.Global.NFC_MULTISE_LIST), false, mCardModeListObserver);
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(
                MtkSettingsExt.Global.NFC_MULTISE_IN_TRANSACTION), false, mCardtransactionObserver);
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(
                MtkSettingsExt.Global.NFC_MULTISE_IN_SWITCHING), false, mCardSwitchingObserver);
        getActivity().registerReceiver(mReceiver, mIntentFilter);
        updatePreferences();
    }

    public void onPause() {
        super.onPause();
        getContentResolver().unregisterContentObserver(mActiveCardModeObserver);
        getContentResolver().unregisterContentObserver(mCardModeListObserver);
        getContentResolver().unregisterContentObserver(mCardtransactionObserver);
        getContentResolver().unregisterContentObserver(mCardSwitchingObserver);
        getActivity().unregisterReceiver(mReceiver);
    }

    private final ContentObserver mActiveCardModeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            Log.d("@M_" + TAG, "mActiveCardModeObserver, onChange()");
            updatePreferences();
        }
    };

    private final ContentObserver mCardModeListObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            Log.d("@M_" + TAG, "mCardModeListObserver, onChange()");
            updatePreferences();
        }
    };

    private final ContentObserver mCardtransactionObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            //1 in transaction, 0 not in
            Log.d("@M_" + TAG, "mCardtransactionObserver, onChange()");
            updatePreferences();
        }
    };

    private final ContentObserver mCardSwitchingObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            //1 is in switching, 0 is not in
            Log.d("@M_" + TAG, "mCardSwitchingObserver, onChange()");
            updatePreferences();
        }
    };

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.NFC_PAYMENT;
    }
}

class CardEmulationProgressCategory extends PreferenceCategory {
    private boolean mProgress = false;

    public CardEmulationProgressCategory(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_progress_category);
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder holder) {
        super.onBindViewHolder(holder);
        final View progressBar = holder.findViewById(R.id.scanning_progress);
        progressBar.setVisibility(mProgress ? View.VISIBLE : View.GONE);
    }

    public void setProgress(boolean progressOn) {
        mProgress = progressOn;
        notifyChanged();
    }

}

class SecurityItemPreference extends Preference implements View.OnClickListener {
    private static final String TAG = "SecurityItemPreference";
    private TextView mPreferenceTitle = null;
    private RadioButton mPreferenceButton = null;
    private CharSequence mTitleValue = "";
    private boolean mChecked = false;

    public SecurityItemPreference(Context context) {
        super(context);
        setLayoutResource(R.layout.card_emulation_item);
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder holder) {
        super.onBindViewHolder(holder);
        mPreferenceTitle = (TextView) holder.findViewById(R.id.preference_title);
        mPreferenceTitle.setText(mTitleValue);
        mPreferenceButton = (RadioButton) holder.findViewById(R.id.preference_radiobutton);
        mPreferenceButton.setOnClickListener(this);
        mPreferenceButton.setChecked(mChecked);
    }

    @Override
    public void setTitle(CharSequence title) {
        if (null == mPreferenceTitle) {
            mTitleValue = title;
        }
        if (!title.equals(mTitleValue)) {
            mTitleValue = title;
            mPreferenceTitle.setText(mTitleValue);
        }
    }

    @Override
    public void onClick(View v) {
        boolean newValue = !isChecked();

        if (!newValue) {
            Log.d("@M_" + TAG, "button.onClick return");
            return;
        }

        if (setChecked(newValue)) {
            callChangeListener(newValue);
            Log.d("@M_" + TAG, "button.onClick");
        }
    }

    public boolean isChecked() {
        return mChecked;
    }

    public boolean setChecked(boolean checked) {
        if (null == mPreferenceButton) {
            Log.d("@M_" + TAG, "setChecked return");
            mChecked = checked;
            return false;
        }

        if (mChecked != checked) {
            mPreferenceButton.setChecked(checked);
            mChecked = checked;
            return true;
        }
        return false;
    }
}

