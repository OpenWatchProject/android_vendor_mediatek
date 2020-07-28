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
import android.content.res.Resources;
import android.database.ContentObserver;
import android.net.Uri;
import android.nfc.NfcAdapter;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.UserHandle;
import android.provider.Settings;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceScreen;
import androidx.preference.PreferenceViewHolder;
import androidx.preference.SwitchPreference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.settings.R;
import com.android.settings.SettingsActivity;
import com.android.settings.SettingsPreferenceFragment;
import com.android.settings.core.InstrumentedFragment;
import com.android.settings.search.BaseSearchIndexProvider;
import com.android.settings.search.Indexable;
import com.android.settings.search.SearchIndexableRaw;
import com.android.settings.widget.SwitchBar;

import com.mediatek.nfcsettingsadapter.NfcSettingsAdapter;
import com.mediatek.provider.MtkSettingsExt;
import com.mediatek.settings.FeatureOption;
import com.android.settings.nfc.AndroidBeam;
import com.android.settings.core.SubSettingLauncher;

import java.util.ArrayList;
import java.util.List;

public class NfcSettings extends SettingsPreferenceFragment implements Indexable {
    private static final String TAG = "NfcSettings";

    private static final String KEY_NFC_TAG_RW = "nfc_rw_tag";
    private static final String KEY_NFC_P2P_MODE = "nfc_p2p_mode";
    private static final String KEY_ANDROID_BEAM = "nfc_android_beam";
    private static final String KEY_CARD_EMULATION = "nfc_card_emulation";
    private static final String KEY_CARD_EMULATION_CATEGORY = "nfc_card_emulation_category";
    private static final String KEY_NFC_TAP_PAY = "nfc_hce_pay";

    protected static final String ACTION_RF_FIELD_ON_DETECTED =
        "com.android.nfc_extras.action.RF_FIELD_ON_DETECTED";
    protected static final String ACTION_RF_FIELD_OFF_DETECTED =
        "com.android.nfc_extras.action.RF_FIELD_OFF_DETECTED";

    private SettingsActivity mActivity;
    private MtkNfcEnabler mNfcEnabler;
    private NfcAdapter mNfcAdapter;
    private NfcServiceHelper mNfcServiceHelper;

    private NfcSettingsAdapter mNfcSettingsAdapter;
    private SwitchPreference mNfcRwTagPref;
    private SwitchPreference mNfcP2pModePref;
    private SwitchPreference mNfcTapPayPref;
    private Preference mAndroidBeam;
    private Preference mNfcServiceStatusPref;
    private SwitchBar mSwitchBar;

    private IntentFilter mIntentFilter;
    private Preference mCardEmulationPref;
    private boolean mCardEmulationExist = true;
    private boolean mNfcBeamOpen = false;
    private int mNfcState = NfcAdapter.STATE_OFF;
    private QueryTask mQueryTask = null;

    private String EMULATION_OFF = null;

    /**
     * The broadcast receiver is used to handle the nfc adapter state changed.
     */
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (NfcAdapter.ACTION_ADAPTER_STATE_CHANGED.equals(action)) {
                Log.d("@M_" + TAG, "Receive nfc change " + intent.getIntExtra(
                        NfcAdapter.EXTRA_ADAPTER_STATE, NfcAdapter.STATE_OFF));
                if (mNfcAdapter != null) {
                    mQueryTask = new QueryTask();
                    mQueryTask.execute();
                }
            } else if (ACTION_RF_FIELD_ON_DETECTED.equals(action)) {
                getPreferenceScreen().setEnabled(false);
                Log.d("@M_" + TAG, "Receive broadcast: RF field on detected");
            } else if (ACTION_RF_FIELD_OFF_DETECTED.equals(action)) {
                getPreferenceScreen().setEnabled(true);
                Log.d("@M_" + TAG, "Receive broadcast: RF field off detected");
            }
        }
    };

    private final ContentObserver mActiveCardModeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange, Uri uri) {
            Log.d("@M_" + TAG, "mActiveCardModeObserver, onChange()");
            if (mCardEmulationExist) {
                String activeMode = Settings.Global.getString(
                        getContentResolver(),
                        MtkSettingsExt.Global.NFC_MULTISE_ACTIVE);
                Log.d("@M_" + TAG, "updatePreferences, active mode is "
                        + activeMode + " EMULATION_OFF is " + EMULATION_OFF);
                // if active mode is off, set the summary as "Off", need MUI
                if (EMULATION_OFF != null && EMULATION_OFF.equals(activeMode)) {
                    mCardEmulationPref.setSummary(R.string.android_beam_off_summary);
                } else {
                    mCardEmulationPref.setSummary(activeMode);
                }
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        addPreferencesFromResource(R.xml.nfc_settings);

        mActivity = (SettingsActivity) getActivity();

        mNfcAdapter = NfcAdapter.getDefaultAdapter(mActivity);
        if (mNfcAdapter == null) {
            Log.d("@M_" + TAG, "Nfc adapter is null, finish Nfc settings");
            getActivity().finish();
            return;
        }
        if (FeatureOption.MTK_NFC_ADDON_SUPPORT) {
            mNfcSettingsAdapter = NfcSettingsAdapter.getDefaultAdapter(mActivity);
        }
        mIntentFilter = new IntentFilter(
                NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
        mIntentFilter.addAction(ACTION_RF_FIELD_ON_DETECTED);
        mIntentFilter.addAction(ACTION_RF_FIELD_OFF_DETECTED);
        initPreferences();
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Log.d("@M_" + TAG, "onActivityCreated() ");
        // On/off switch
        mSwitchBar = mActivity.getSwitchBar();
        mNfcEnabler = new MtkNfcEnabler(mActivity, mSwitchBar, mNfcAdapter);
    }

    @Override
    public void onDestroyView() {
        Log.d("@M_" + TAG, "onDestroyView, mSwitchBar removeOnSwitchChangeListener");

//        getActivity().getActionBar().setCustomView(null);
        if (mNfcEnabler != null) {
            mNfcEnabler.teardownSwitchBar();
        }
        super.onDestroyView();
    }

    /**
     * According to the key find the corresponding preference.
     */
    private void initPreferences() {
        mNfcP2pModePref = (SwitchPreference) findPreference(KEY_NFC_P2P_MODE);
        mAndroidBeam = findPreference(KEY_ANDROID_BEAM);
        mNfcRwTagPref = (SwitchPreference) findPreference(KEY_NFC_TAG_RW);

        //card emulation item
        mCardEmulationPref = findPreference(KEY_CARD_EMULATION);
        PreferenceCategory cardCategory = (PreferenceCategory) findPreference(
                KEY_CARD_EMULATION_CATEGORY);
        //if the nfc_multise_on is off, remove card emulation pref
        int cardExist = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.NFC_MULTISE_ON, 0);
        Log.d("@M_" + TAG,  "NFC_MULTISE_ON is " + cardExist);
        // Card emulation is only for owner
        if (UserHandle.myUserId() != UserHandle.USER_OWNER
                || (cardCategory != null && cardExist == 0)) {
            getPreferenceScreen().removePreference(cardCategory);
            mCardEmulationExist = false;
        } else {
            getEmulationOffConstant();
        }

        //nfc tap&pay item
        mNfcTapPayPref = (SwitchPreference) findPreference(KEY_NFC_TAP_PAY);

        //nfc service status
        if (mNfcSettingsAdapter.isShowOverflowMenu()) {
            addNfcServiceStatusPref();
        }
    }

    private void addNfcServiceStatusPref() {
        mNfcServiceHelper = new NfcServiceHelper(mActivity);
        mNfcServiceStatusPref = new Preference(getPreferenceScreen().getPreferenceManager().getContext());
        mNfcServiceStatusPref.setTitle(getString(R.string.nfc_service_status_title));
        mNfcServiceStatusPref.setFragment(NfcServiceStatus.class.getName());
        getPreferenceScreen().addPreference(mNfcServiceStatusPref);
    }

    private void updatePreferenceEnabledStatus(int state) {
        Log.d("@M_" + TAG, "updatePreferenceEnabledStatus nfc state :"  + state);
        // if nfc is on, set enabled, else set disenabled.
        if (state == NfcAdapter.STATE_ON) {
            mNfcP2pModePref.setEnabled(true);
            mNfcRwTagPref.setEnabled(true);
            if (mNfcBeamOpen) {
                mAndroidBeam.setSummary(R.string.android_beam_on_summary);
            } else {
                mAndroidBeam.setSummary(R.string.android_beam_off_summary);
            }
            if (mCardEmulationExist) {
                mCardEmulationPref.setEnabled(true);
            }
            mNfcTapPayPref.setEnabled(true);
        } else {
            mNfcP2pModePref.setEnabled(false);
            mNfcRwTagPref.setEnabled(false);
            mAndroidBeam.setSummary(R.string.android_beam_off_summary);
            if (mCardEmulationExist) {
                mCardEmulationPref.setEnabled(false);
            }
            mNfcTapPayPref.setEnabled(false);
        }
    }

    /**
     * update the preference according to the status of NfcAdapter settings.
     */
    private void updatePreferences() {
        Log.d("@M_" + TAG, "updatePreferences");
        // update p2p mode , android beam and RW tag preference enabled status
        updatePreferenceEnabledStatus(mNfcState);

        // update p2p mode preference checked status
        mNfcP2pModePref.setChecked(mNfcSettingsAdapter
                .getModeFlag(NfcSettingsAdapter.MODE_P2P) == NfcSettingsAdapter.FLAG_ON);
        // update RW tag preference checked status
        mNfcRwTagPref.setChecked(mNfcSettingsAdapter
                .getModeFlag(NfcSettingsAdapter.MODE_READER) == NfcSettingsAdapter.FLAG_ON);
        if (mCardEmulationExist) {
            String activeMode = Settings.Global.getString(getContentResolver(),
                    MtkSettingsExt.Global.NFC_MULTISE_ACTIVE);
            Log.d("@M_" + TAG, "updatePreferences, active mode is "
                    + activeMode + " EMULATION_OFF is " + EMULATION_OFF);
            // if active mode is off, set the summary as "Off", need MUI
            if (EMULATION_OFF != null && EMULATION_OFF.equals(activeMode)) {
                mCardEmulationPref.setSummary(R.string.android_beam_off_summary);
            } else {
                mCardEmulationPref.setSummary(activeMode);
            }
        }

        int hceFlg = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.NFC_HCE_ON, 0);
        if (1 == hceFlg) {
            mNfcTapPayPref.setChecked(true);
        } else {
            mNfcTapPayPref.setChecked(false);
        }

        int fieldActive = Settings.Global.getInt(getContentResolver(),
                MtkSettingsExt.Global.NFC_RF_FIELD_ACTIVE, 0);
        getPreferenceScreen().setEnabled(fieldActive == 0);
        Log.d("@M_" + TAG, "Read the value Global.NFC_RF_FIELD_ACTIVE : " + fieldActive);

        if (mNfcServiceStatusPref != null) {
            mNfcServiceHelper.initServiceList();
            mNfcServiceStatusPref.setSummary(getString(R.string.nfc_service_status_summary,
                    mNfcServiceHelper.getSelectServiceCount(),
                    mNfcServiceHelper.getAllServiceCount()));
         }
    }

    /*
     * get EMULATION_OFF Constant
     */
    private void getEmulationOffConstant() {
        String list = Settings.Global.getString(getContentResolver(),
                MtkSettingsExt.Global.NFC_MULTISE_LIST);
        if (list == null) {
            EMULATION_OFF = "Off";
        } else {
            String delims = "[,]";
            String[] tokens = list.split(delims);
            int length = tokens.length;
            if (EMULATION_OFF == null) {
                EMULATION_OFF = tokens[length - 1];
                Log.d("@M_" + TAG, "NFC_MULTISE_LIST is" + list
                        + ", EMULATION_OFF is " + EMULATION_OFF);
            }
        }
    }

    public boolean onPreferenceTreeClick(Preference preference) {
        if (preference.equals(mAndroidBeam)) {
            new SubSettingLauncher(getContext())
            .setDestination(AndroidBeam.class.getCanonicalName())
            .setTitleRes(R.string.android_beam_settings_title)
            .setSourceMetricsCategory(getMetricsCategory())
            .setArguments(null)
            .launch();
        } else if (preference.equals(mNfcP2pModePref)) {
            Log.d("@M_" + TAG, "p2p mode");
            int flag = mNfcP2pModePref.isChecked() ? NfcSettingsAdapter.FLAG_ON
                    : NfcSettingsAdapter.FLAG_OFF;
            mNfcSettingsAdapter.setModeFlag(NfcSettingsAdapter.MODE_P2P, flag);
        } else if (preference.equals(mNfcRwTagPref)) {
            Log.d("@M_" + TAG, "tag rw mode");
            int flag = mNfcRwTagPref.isChecked() ? NfcSettingsAdapter.FLAG_ON
                    : NfcSettingsAdapter.FLAG_OFF;
            mNfcSettingsAdapter.setModeFlag(NfcSettingsAdapter.MODE_READER, flag);
        } else if (preference.equals(mCardEmulationPref)) {
            Log.d("@M_" + TAG, "card emulation mode");
            new SubSettingLauncher(getContext())
            .setDestination(CardEmulationSettings.class.getCanonicalName())
            .setTitleRes(R.string.nfc_card_emulation)
            .setSourceMetricsCategory(getMetricsCategory())
            .setArguments(null)
            .launch();
        } else if (preference.equals(mNfcTapPayPref)) {
            boolean flag = mNfcTapPayPref.isChecked();
            Log.d("@M_" + TAG, "pay tap " + flag);
            Settings.Global.putInt(getContentResolver(),
                    MtkSettingsExt.Global.NFC_HCE_ON, flag ? 1 : 0);
        }
        return super.onPreferenceTreeClick(preference);
    }

    public void onResume() {
        super.onResume();
        Log.d("@M_" + TAG, "onResume ");
        if (mNfcEnabler != null) {
            mNfcEnabler.resume();
        }
        if (mNfcAdapter != null) {
            mQueryTask = new QueryTask();
            mQueryTask.execute();
        }
        getActivity().registerReceiver(mReceiver, mIntentFilter);
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(
                MtkSettingsExt.Global.NFC_MULTISE_ACTIVE), false, mActiveCardModeObserver);

        updatePreferences();
    }

    public void onPause() {
        super.onPause();
        Log.d("@M_" + TAG, "onPause rm observer ");

        getContentResolver().unregisterContentObserver(mActiveCardModeObserver);
        if (mQueryTask != null) {
            mQueryTask.cancel(true);
            Log.d("@M_" + TAG, "mQueryTask.cancel(true)");
        }
        getActivity().unregisterReceiver(mReceiver);
        if (mNfcEnabler != null) {
            mNfcEnabler.pause();
        }
    }

    public static final SearchIndexProvider SEARCH_INDEX_DATA_PROVIDER =
        new BaseSearchIndexProvider() {
            @Override
            public List<SearchIndexableRaw> getRawDataToIndex(Context context,
                    boolean enabled) {
                final List<SearchIndexableRaw> result = new ArrayList<SearchIndexableRaw>();
                final Resources res = context.getResources();
                if (NfcAdapter.getDefaultAdapter(context) != null
                        && FeatureOption.MTK_NFC_ADDON_SUPPORT) {
                    Log.d("@M_" + TAG, "SearchIndexProvider add NFC");
                    // Add fragment title
                    SearchIndexableRaw data = new SearchIndexableRaw(context);
                    data.title = res.getString(R.string.nfc_quick_toggle_title);
                    data.screenTitle = res
                            .getString(R.string.nfc_quick_toggle_title);
                    data.keywords = res.getString(R.string.nfc_quick_toggle_title);
                    result.add(data);
                } else {
                    Log.d("@M_" + TAG, "NfcAdapter is null or it is default NFC");
                }
                return result;
            }
        };

    private class QueryTask extends AsyncTask<Void, Void, Integer> {

        @Override
        protected Integer doInBackground(Void... params) {
            mNfcState = mNfcAdapter.getAdapterState();
            mNfcBeamOpen = mNfcAdapter.isNdefPushEnabled();

            Log.d("@M_" + TAG, "doInBackground  mNfcState: " + mNfcState);
            Log.d("@M_" + TAG, "doInBackground  mNfcBeamOpen: " + mNfcBeamOpen);
            return mNfcState;
        }

        @Override
        protected void onPostExecute(Integer result) {
            Log.d("@M_" + TAG, "onPostExecute");
            updatePreferenceEnabledStatus(result);
        }
    }

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.NFC_PAYMENT;
    }
}

class NfcDescriptionPreference extends Preference {
    public NfcDescriptionPreference(Context context, AttributeSet attrs,
            int defStyle) {
        super(context, attrs, defStyle);
    }

    public NfcDescriptionPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder holder) {
        super.onBindViewHolder(holder);

        TextView title = (TextView) holder.findViewById(android.R.id.title);
        if (title != null) {
            title.setSingleLine(false);
            title.setMaxLines(3);
        }
    }

}
