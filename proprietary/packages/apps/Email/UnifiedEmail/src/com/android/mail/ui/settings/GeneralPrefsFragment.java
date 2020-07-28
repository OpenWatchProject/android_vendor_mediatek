/*******************************************************************************
 *      Copyright (C) 2014 Google Inc.
 *      Licensed to The Android Open Source Project.
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *           http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 *******************************************************************************/

package com.android.mail.ui.settings;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.DialogInterface.OnClickListener;
import android.database.ContentObserver;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.preference.CheckBoxPreference;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.RingtonePreference;
import android.preference.Preference.OnPreferenceChangeListener;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.android.emailcommon.Logging;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.android.mail.preferences.MailPrefs;
import com.android.mail.preferences.MailPrefs.PreferenceKeys;
import com.android.mail.providers.SuggestionsProvider;
import com.android.mail.providers.UIProvider.AutoAdvance;
import com.android.mail.ui.settings.SettingsUtils;
import com.android.mail.utils.LogUtils;
import com.android.mail.R;
import com.google.common.annotations.VisibleForTesting;
import com.mediatek.mail.vip.VipMember;
import com.mediatek.mail.vip.VipPreferences;
import com.mediatek.mail.vip.activity.VipListActivity;

/**
 * This fragment shows general app preferences.
 */
public class GeneralPrefsFragment extends MailPreferenceFragment
        implements OnClickListener, OnPreferenceChangeListener {

    // Keys used to reference pref widgets which don't map directly to preference entries
    static final String AUTO_ADVANCE_WIDGET = "auto-advance-widget";

    static final String CALLED_FROM_TEST = "called-from-test";

    //M:Notification channel
    static final String NOTIFICATION_CHANNEL_VIP= "email_notification_channel_vip";

    // Category for removal actions
    protected static final String REMOVAL_ACTIONS_GROUP = "removal-actions-group";

    protected MailPrefs mMailPrefs;

    private AlertDialog mClearSearchHistoryDialog;

    private ListPreference mAutoAdvance;
    private static final int[] AUTO_ADVANCE_VALUES = {
            AutoAdvance.NEWER,
            AutoAdvance.OLDER,
            AutoAdvance.LIST
    };

    ///M: add bcc myself
    private CheckBoxPreference mAutoBcc;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setHasOptionsMenu(true);

        mMailPrefs = MailPrefs.get(getActivity());

        // Set the shared prefs name to use prefs auto-persist behavior by default.
        // Any pref more complex than the default (say, involving migration), should set
        // "persistent=false" in the XML and manually handle preference initialization and change.
        getPreferenceManager()
                .setSharedPreferencesName(mMailPrefs.getSharedPreferencesName());

        addPreferencesFromResource(R.xml.general_preferences);

        mAutoAdvance = (ListPreference) findPreference(AUTO_ADVANCE_WIDGET);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        /*
         * We deliberately do not call super because our menu includes the parent's menu options to
         * allow custom ordering.
         */
        menu.clear();
        inflater.inflate(R.menu.general_prefs_fragment_menu, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        if (itemId == R.id.clear_search_history_menu_item) {
            clearSearchHistory();
            return true;
        } else if (itemId == R.id.clear_picture_approvals_menu_item) {
            clearDisplayImages();
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (getActivity() == null) {
            // Monkeys cause bad things. This callback may be delayed if for some reason the
            // preference screen was closed really quickly - just bail then.
            return false;
        }

        final String key = preference.getKey();

        if (PreferenceKeys.REMOVAL_ACTION.equals(key)) {
            final String removalAction = newValue.toString();
            mMailPrefs.setRemovalAction(removalAction);
        } else if (AUTO_ADVANCE_WIDGET.equals(key)) {
            final int prefsAutoAdvanceMode =
                    AUTO_ADVANCE_VALUES[mAutoAdvance.findIndexOfValue((String) newValue)];
            mMailPrefs.setAutoAdvanceMode(prefsAutoAdvanceMode);
        /** M: Check is the MTK feature relevant preference changed @{ */
        } else if (onVipPreferenceChange(preference, newValue)) {
            return true;
        } else if (MailPrefs.PreferenceKeys.BCC_MYSELF_KEY.equals(key)) {
            mMailPrefs.setAutoBccMyself(mAutoBcc.isChecked());
        /** @} */
        } else if (!PreferenceKeys.CONVERSATION_LIST_SWIPE.equals(key) &&
                !PreferenceKeys.SHOW_SENDER_IMAGES.equals(key) &&
                !PreferenceKeys.DEFAULT_REPLY_ALL.equals(key) &&
                !PreferenceKeys.CONVERSATION_OVERVIEW_MODE.equals(key) &&
                !PreferenceKeys.CONFIRM_DELETE.equals(key) &&
                !PreferenceKeys.CONFIRM_ARCHIVE.equals(key) &&
                !PreferenceKeys.CONFIRM_SEND.equals(key)) {
            return false;
        }
        return true;
    }

    private void clearDisplayImages() {
        final ClearPictureApprovalsDialogFragment fragment =
                ClearPictureApprovalsDialogFragment.newInstance();
        fragment.show(getActivity().getFragmentManager(),
                ClearPictureApprovalsDialogFragment.FRAGMENT_TAG);
    }

    private void clearSearchHistory() {
        mClearSearchHistoryDialog = new AlertDialog.Builder(getActivity())
                .setMessage(R.string.clear_history_dialog_message)
                .setTitle(R.string.clear_history_dialog_title)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setPositiveButton(R.string.clear, this)
                .setNegativeButton(R.string.cancel, this)
                .show();
    }


    @Override
    public void onClick(DialogInterface dialog, int which) {
        if (dialog.equals(mClearSearchHistoryDialog)) {
            if (which == DialogInterface.BUTTON_POSITIVE) {
                final Context context = getActivity();
                // Clear the history in the background, as it causes a disk
                // write.
                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... params) {
                        final SuggestionsProvider suggestions =
                                new SuggestionsProvider(context);
                        suggestions.clearHistory();
                        suggestions.cleanup();
                        return null;
                    }
                }.execute();
                Toast.makeText(getActivity(), R.string.search_history_cleared, Toast.LENGTH_SHORT)
                        .show();
            }
        }
    }

    @Override
    public void onStop() {
        super.onStop();
        /// M: add for VIP features
        unregisterVipCountObserver();
        if (mClearSearchHistoryDialog != null && mClearSearchHistoryDialog.isShowing()) {
            mClearSearchHistoryDialog.dismiss();
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        // Manually initialize the preference views that require massaging. Prefs that require
        // massaging include:
        //  1. a prefs UI control that does not map 1:1 to storage
        //  2. a pref that must obtain its initial value from migrated storage, and for which we
        //     don't want to always persist a migrated value
        final int autoAdvanceModeIndex = prefValueToWidgetIndex(AUTO_ADVANCE_VALUES,
                mMailPrefs.getAutoAdvanceMode(), AutoAdvance.DEFAULT);
        mAutoAdvance.setValueIndex(autoAdvanceModeIndex);

        listenForPreferenceChange(
                PreferenceKeys.REMOVAL_ACTION,
                PreferenceKeys.CONVERSATION_LIST_SWIPE,
                PreferenceKeys.SHOW_SENDER_IMAGES,
                PreferenceKeys.DEFAULT_REPLY_ALL,
                PreferenceKeys.CONVERSATION_OVERVIEW_MODE,
                AUTO_ADVANCE_WIDGET,
                PreferenceKeys.CONFIRM_DELETE,
                PreferenceKeys.CONFIRM_ARCHIVE,
                PreferenceKeys.CONFIRM_SEND,
                /// M: Add Auto BCC Myself
                PreferenceKeys.BCC_MYSELF_KEY
        );
        loadMtkSettings();
    }

    protected boolean supportsArchive() {
        return true;
    }

    /**
     * Converts the prefs value into an index useful for configuring the UI widget, falling back to
     * the default value if the value from the prefs can't be found for some reason. If neither can
     * be found, it throws an {@link java.lang.IllegalArgumentException}
     *
     * @param conversionArray An array of prefs values, in widget order
     * @param prefValue Value of the preference
     * @param defaultValue Default value, as a fallback if we can't map the real value
     * @return Index of the entry (or fallback) in the conversion array
     */
    @VisibleForTesting
    static int prefValueToWidgetIndex(int[] conversionArray, int prefValue, int defaultValue) {
        for (int i = 0; i < conversionArray.length; i++) {
            if (conversionArray[i] == prefValue) {
                return i;
            }
        }
        LogUtils.e(LogUtils.TAG, "Can't map preference value " + prefValue);
        for (int i = 0; i < conversionArray.length; i++) {
            if (conversionArray[i] == defaultValue) {
                return i;
            }
        }
        throw new IllegalArgumentException("Can't map default preference value " + prefValue);
    }

    private void listenForPreferenceChange(String... keys) {
        for (String key : keys) {
            Preference p = findPreference(key);
            if (p != null) {
                p.setOnPreferenceChangeListener(this);
            }
        }
    }

    /**M: Support for VIP settings @{*/
    private static final String PERFERENCE_KEY_VIPSETTINGS = "vip_settings";
    private static final String PERFERENCE_KEY_VIP_MEMBERS = "vip_members";

    private VipMemberPreference mVipMembers;
    private VipMemberCountObserver mCountObserver;
    private int mMemberCount;
    private CheckBoxPreference mVipNotification;

    //M: Changes from RingtonePreference to Preference type
    private Preference mVipRingTone;
    //private CheckBoxPreference mVipVibrate;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        PreferenceCategory vipCategory = (PreferenceCategory) findPreference(PERFERENCE_KEY_VIPSETTINGS);
        mVipMembers = new VipMemberPreference(getActivity());
        mVipMembers.setOrder(0);
        vipCategory.addPreference(mVipMembers);
        return super.onCreateView(inflater, container, savedInstanceState);
    }

    /**M: Support for VIP settings. Open VipListActivity @{*/
    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
        if (getActivity() == null) {
            // Guard against monkeys.
            return false;
        }
        String key = preference.getKey();
        if (PERFERENCE_KEY_VIP_MEMBERS.equals(key)) {
            final Intent vipActivityIntent = VipListActivity.createIntent(getActivity(),
                    Account.ACCOUNT_ID_COMBINED_VIEW);
            getActivity().startActivity(vipActivityIntent);
            return true;
        }
        /** @} */
        return false;
    }

    /** M: load MTK relevant features' settings @{ */
    private void loadMtkSettings() {
        /// M: add Auto BCC Myself
        mAutoBcc = (CheckBoxPreference) findPreference(MailPrefs.PreferenceKeys.BCC_MYSELF_KEY);
        mAutoBcc.setChecked(mMailPrefs.getIsAutoBccMyselfEnabled());

        loadVipData();
    }
    /**
     * M: Load the VIP Settings to UI
     */
    private void loadVipData() {
        VipPreferences vipPreferences = VipPreferences.get(getActivity());
        mVipNotification = (CheckBoxPreference) findPreference(VipPreferences.VIP_NOTIFICATION);
        mVipNotification.setChecked(vipPreferences.getVipNotification());
        mVipNotification.setOnPreferenceChangeListener(this);
        mVipRingTone = (Preference) findPreference(VipPreferences.VIP_RINGTONE);
        //SharedPreferences prefs = mVipRingTone.getPreferenceManager().getSharedPreferences();
        //String ringtone = vipPreferences.getVipRingtone();
        //prefs.edit().putString(VipPreferences.VIP_RINGTONE, ringtone).apply();
        //setRingtoneSummary(getActivity(), ringtone);
        mVipRingTone.setOnPreferenceChangeListener(this);
        mVipRingTone.setOnPreferenceClickListener(new OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(final Preference preference) {
                      final Context context = getActivity();
                     String packageName = context.getPackageName();
                     String id = NOTIFICATION_CHANNEL_VIP;
                     LogUtils.d(LogUtils.TAG, "Go to system notification setting for VIP, packages="
                               + packageName + ", id = " + id);

                     Intent intent = new Intent(Settings.ACTION_CHANNEL_NOTIFICATION_SETTINGS);
                     intent.putExtra(Settings.EXTRA_APP_PACKAGE, packageName);
                     intent.putExtra(Settings.EXTRA_CHANNEL_ID, id);
                     startActivity(intent);  // Lanch the Notification category for Vip Email
                     return true;
                }
            });
        /* M: No need this
        mVipVibrate = (CheckBoxPreference) findPreference(VipPreferences.VIP_VIBRATE);
        mVipVibrate.setChecked(vipPreferences.getVipVebarate());
        mVipVibrate.setOnPreferenceChangeListener(this);
        */
        registerVipCountObserver();
    }

    /// M: Deal the the VIP preference change event
    private boolean onVipPreferenceChange(Preference preference, Object newValue) {
        String key = preference.getKey();
        if (VipPreferences.VIP_NOTIFICATION.equals(key)) {
            VipPreferences.get(getActivity()).setVipNotification(
                    (Boolean) newValue);
            return true;
        } else if (VipPreferences.VIP_RINGTONE.equals(key)) {
            //VipPreferences.get(getActivity()).setVipRingtone((String) newValue);
            //setRingtoneSummary(getActivity(), (String) newValue);
            return true;
        } else if (VipPreferences.VIP_VIBRATE.equals(key)) {
            //VipPreferences.get(getActivity())
            //        .setVipVebarate((Boolean) newValue);
            return true;
        }
        return false;
    }

    /**
     * Sets the current ringtone summary.
     */
    private void setRingtoneSummary(Context context, String ringtoneUri) {
        Ringtone ringtone = null;
        if (!TextUtils.isEmpty(ringtoneUri)) {
            ringtone = RingtoneManager.getRingtone(getActivity(), Uri.parse(ringtoneUri));
        }
        final String summary = ringtone != null ? ringtone.getTitle(context)
                : context.getString(R.string.silent_ringtone);
       // mVipRingTone.setSummary(summary);
    }

    /**
     * M: Register the VIP member count observer if it was not registered
     */
    private void registerVipCountObserver() {
        Context context = getActivity();
        if (context != null && mCountObserver == null) {
            mCountObserver = new VipMemberCountObserver(Utility.getMainThreadHandler());
            context.getContentResolver().registerContentObserver(VipMember.NOTIFIER_URI, true,
                    mCountObserver);
            updateVipMemberCount();
        }
    }

    /**
     * M: Unregister the VIP member count observer if it was not unregistered
     */
    private void unregisterVipCountObserver() {
        Context context = getActivity();
        if (context != null && mCountObserver != null) {
            context.getContentResolver().unregisterContentObserver(mCountObserver);
            mCountObserver = null;
        }
    }

    private void updateVipMemberCount() {
        new EmailAsyncTask<Void, Void, Integer>(null) {
            private static final int ERROR_RESULT = -1;
            @Override
            protected Integer doInBackground(Void... params) {
                Context context = getActivity();
                if (context == null) {
                    return ERROR_RESULT;
                }
                return VipMember.countVipMembersWithAccountId(context,
                        Account.ACCOUNT_ID_COMBINED_VIEW);
            }

            @Override
            protected void onSuccess(Integer result) {
                if (result != ERROR_RESULT) {
                    mMemberCount = result;
                    mVipMembers.setCount(result);
                } else {
                    Logging.e("Failed to get the count of the VIP member");
                }
            }
        }.executeParallel();
    }

    private class VipMemberCountObserver extends ContentObserver {

        public VipMemberCountObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange) {
            updateVipMemberCount();
        }
    }

    private class VipMemberPreference extends Preference {
        private TextView mCountView;

        public VipMemberPreference(Context context) {
            super(context);
            setKey(PERFERENCE_KEY_VIP_MEMBERS);
            setTitle(R.string.vip_members);
            setWidgetLayoutResource(R.layout.vip_preference_widget_count);
        }

        @Override
        protected void onBindView(View view) {
            super.onBindView(view);
            // Get the widget view of the member preference
            ViewGroup widgetFrame = (ViewGroup) view.findViewById(com.android.internal.R.id.widget_frame);
            mCountView = (TextView) widgetFrame.findViewById(R.id.vip_count);
            setCount(mMemberCount);
        }

        // Set the count of the VIP member
        public void setCount(int count) {
            if (mCountView != null) {
                mCountView.setText(getContext().getResources().getString(
                        R.string.vip_settings_member_count, count));
            }
        }
    }
    /** @{ */

    /**
     * M: Don't show Help & feedback menu.
     */
    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        MenuItem item = menu.findItem(R.id.help_info_menu_item);
        if (null != item) {
            item.setVisible(false);
        }
    }
    /** @{ */

}
