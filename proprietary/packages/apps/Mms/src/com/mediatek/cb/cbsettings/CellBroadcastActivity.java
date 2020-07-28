package com.mediatek.cb.cbsettings;

import android.app.ActionBar;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.os.Bundle;
import android.os.Handler;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.view.MenuItem;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.provider.MtkSettingsExt;


public class CellBroadcastActivity extends TimeConsumingPreferenceActivity {
    private static final String BUTTON_CB_CHECKBOX_KEY     = "enable_cellBroadcast";
    private static final String BUTTON_CB_SETTINGS_KEY     = "cbsettings";
    private static final String LOG_TAG = "MmsCB/CellBroadcastActivity";
    private static final String SUB_TITLE_NAME = "sub_title_name";
    private ServiceState mServiceState;

    private CellBroadcastCheckBox mCBCheckBox = null;
    private Preference mCBSetting = null;

    private boolean mAirplaneModeEnabled = false;
    private int mDualSimMode = -1;
    private int mSubId;
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            MmsLog.d(LOG_TAG, "[action = " + action + "]");
            //if (TelephonyIntents.ACTION_SIM_INFO_UPDATE.equals(action)) {
            if ("mediatek.intent.action.SIM_INFO_UPDATE".equals(action)) {
                setScreenEnabled();
            } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                mAirplaneModeEnabled = intent.getBooleanExtra("state", false);
                MmsLog.d(LOG_TAG, "mAirplaneModeEnabled: " + mAirplaneModeEnabled);
                /// M: ALPS00740653 @{
                // when airplane mode is on, the phone state must not be out of service,
                // but ,but when airplane mode is off, the phone state may be out of service,
                // so airplane mode is off, we not enable screen until phone state is in service.
                if (mAirplaneModeEnabled) {
                    setScreenEnabled();
                }
                /// @}
            } else if (action.equals(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED)) {
                setScreenEnabled();
            }
        }
    };

    private final ContentObserver mMsimObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfUpdate) {
            mDualSimMode = Settings.System.getInt(getContentResolver(),
                    MtkSettingsExt.System.MSIM_MODE_SETTING, -1);
            setScreenEnabled();
        }
    };

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        addPreferencesFromResource(R.xml.cell_broad_cast);

        mCBCheckBox = (CellBroadcastCheckBox) findPreference(BUTTON_CB_CHECKBOX_KEY);
        mCBSetting = findPreference(BUTTON_CB_SETTINGS_KEY);

        if (null != getIntent().getStringExtra(SUB_TITLE_NAME)) {
            setTitle(getIntent().getStringExtra(SUB_TITLE_NAME));
        }
        //mSubId = getIntent().getLongExtra(PhoneConstants.SUBSCRIPTION_KEY, 1);
        mSubId = (int) getIntent().getExtras().get(PhoneConstants.SUBSCRIPTION_KEY);
        if (mCBCheckBox != null) {
            mCBCheckBox.init(mSubId, this, false);
        }
        /// M: ALPS00670751 @{
        registerBroadcast();
        /// @}
        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        getContentResolver().registerContentObserver(
                Settings.System.getUriFor(MtkSettingsExt.System.MSIM_MODE_SETTING),
                false, mMsimObserver);
    }

    @Override
    public boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen,
            Preference preference) {
        if (preference == mCBSetting) {
            Intent intent = new Intent(this, CellBroadcastSettings.class);
            intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
            this.startActivity(intent);
            return true;
        }
        return false;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        switch (itemId) {
        case android.R.id.home:
            finish();
            return true;
        default:
            break;
        }
        return super.onOptionsItemSelected(item);
    }

    protected void onDestroy() {
        super.onDestroy();

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        unregisterReceiver(mReceiver);
        getContentResolver().unregisterContentObserver(mMsimObserver);
    }

    private void setScreenEnabled() {
        ///M: add for hot swap {
        handleSimHotSwap(mSubId, this);
        ///@}

        /// M: ALPS00670751 @{
        // when no airplane , exist sim card and  have dual sim mode
        // set screen disable
        enableScreen();
        /// @}
    }

    @Override
    public void onResume() {
        super.onResume();
        mAirplaneModeEnabled = android.provider.Settings.System.getInt(getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, -1) == 1;
        mDualSimMode = android.provider.Settings.System.getInt(getContentResolver(),
                MtkSettingsExt.System.MSIM_MODE_SETTING, -1);
        MmsLog.d(LOG_TAG, "onResume(), mDualSimMode=" + mDualSimMode);
        SubscriptionInfo subInfo =
            SubscriptionManager.from(this).getActiveSubscriptionInfo(mSubId);
        if (subInfo != null) {
            if (subInfo.getDisplayName() != null) {
                setTitle(subInfo.getDisplayName());
            }
            setScreenEnabled();
        }
    }

    private void enableScreen() {
        boolean isShouldEnabled = false;
        boolean isHasSimCard = SubscriptionManager
                .from(MmsApp.getApplication()).getActiveSubscriptionInfoCount() > 0;
        isShouldEnabled = (!mAirplaneModeEnabled) && (mDualSimMode != 0) && isHasSimCard;
        getPreferenceScreen().setEnabled(isShouldEnabled);
    }

    private void registerBroadcast() {
        /*
        IntentFilter intentFilter = new IntentFilter("mediatek.intent.action.SIM_INFO_UPDATE");
        intentFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mReceiver, intentFilter);
        */
    }

    public static void handleSimHotSwap(int subId, Activity activity) {
        MmsLog.d(LOG_TAG, "subId = " + subId);
        if (subId < 1) {
            activity.finish();
        }
    }
}
