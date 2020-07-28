package com.android.phone;

import android.app.ActionBar;
import android.app.Dialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.database.Cursor;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.telephony.CarrierConfigManager;
import android.util.Log;
import android.view.MenuItem;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.mediatek.settings.vtss.GsmUmtsVTUtils;

import java.util.ArrayList;

public class GsmUmtsCallForwardOptions extends TimeConsumingPreferenceActivity
        implements PhoneGlobals.SubInfoUpdateListener {
    private static final String LOG_TAG = "GsmUmtsCallForwardOptions";

    private static final String NUM_PROJECTION[] = {
        android.provider.ContactsContract.CommonDataKinds.Phone.NUMBER
    };

    private static final String BUTTON_CFU_KEY   = "button_cfu_key";
    private static final String BUTTON_CFB_KEY   = "button_cfb_key";
    private static final String BUTTON_CFNRY_KEY = "button_cfnry_key";
    private static final String BUTTON_CFNRC_KEY = "button_cfnrc_key";

    private static final String KEY_TOGGLE = "toggle";
    private static final String KEY_STATUS = "status";
    private static final String KEY_NUMBER = "number";
    private static final String KEY_ENABLE = "enable";

    private CallForwardEditPreference mButtonCFU;
    private CallForwardEditPreference mButtonCFB;
    private CallForwardEditPreference mButtonCFNRy;
    private CallForwardEditPreference mButtonCFNRc;

    private final ArrayList<CallForwardEditPreference> mPreferences =
            new ArrayList<CallForwardEditPreference>();
    private int mInitIndex = 0;

    private boolean mFirstResume;
    private Bundle mIcicle;
    private Phone mPhone;
    private SubscriptionInfoHelper mSubscriptionInfoHelper;
    private boolean mReplaceInvalidCFNumbers;
    private boolean mCallForwardByUssd;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.callforward_options);

        mSubscriptionInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        mSubscriptionInfoHelper.setActionBarTitle(
                getActionBar(), getResources(), R.string.call_forwarding_settings_with_label);
        mPhone = mSubscriptionInfoHelper.getPhone();

        /// M: Add for sub info update
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
        /// M: Add for airplane mode or radio changed
        registerCallBacks();
        /// M: Add for MTK hotswap @{
        if (mPhone == null) {
            Log.d(LOG_TAG, "onCreate: mPhone is null, finish!!!");
            finish();
            return;
        }
        /// @}

        PersistableBundle b = null;
        if (mSubscriptionInfoHelper.hasSubId()) {
            b = PhoneGlobals.getInstance().getCarrierConfigForSubId(
                    mSubscriptionInfoHelper.getSubId());
        } else {
            b = PhoneGlobals.getInstance().getCarrierConfig();
        }
        if (b != null) {
            mReplaceInvalidCFNumbers = b.getBoolean(
                    CarrierConfigManager.KEY_CALL_FORWARDING_MAP_NON_NUMBER_TO_VOICEMAIL_BOOL);
            mCallForwardByUssd = b.getBoolean(
                    CarrierConfigManager.KEY_USE_CALL_FORWARDING_USSD_BOOL);
        }

        PreferenceScreen prefSet = getPreferenceScreen();
        mButtonCFU = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFU_KEY);
        mButtonCFB = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFB_KEY);
        mButtonCFNRy = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFNRY_KEY);
        mButtonCFNRc = (CallForwardEditPreference) prefSet.findPreference(BUTTON_CFNRC_KEY);

        /// M: Add for vt ss @{
        Intent intent = getIntent();
        int serviceClass = intent.getIntExtra(GsmUmtsVTUtils.SERVICE_CLASS,
                GsmUmtsVTUtils.VOICE_SERVICE_CLASS);
        if (serviceClass != GsmUmtsVTUtils.VOICE_SERVICE_CLASS) {
            GsmUmtsVTUtils.setCFServiceClass(prefSet, serviceClass);
            int resId = GsmUmtsVTUtils.getActionBarResId(serviceClass, GsmUmtsVTUtils.CF_TYPE);
            mSubscriptionInfoHelper.setActionBarTitle(getActionBar(), getResources(), resId);
        }
        /// @}

        mButtonCFU.setParentActivity(this, mButtonCFU.reason);
        mButtonCFB.setParentActivity(this, mButtonCFB.reason);
        mButtonCFNRy.setParentActivity(this, mButtonCFNRy.reason);
        mButtonCFNRc.setParentActivity(this, mButtonCFNRc.reason);

        mPreferences.add(mButtonCFU);
        mPreferences.add(mButtonCFB);
        mPreferences.add(mButtonCFNRy);
        mPreferences.add(mButtonCFNRc);

        if (mCallForwardByUssd) {
            //the call forwarding ussd command's behavior is similar to the call forwarding when
            //unanswered,so only display the call forwarding when unanswered item.
            prefSet.removePreference(mButtonCFU);
            prefSet.removePreference(mButtonCFB);
            prefSet.removePreference(mButtonCFNRc);
            mPreferences.remove(mButtonCFU);
            mPreferences.remove(mButtonCFB);
            mPreferences.remove(mButtonCFNRc);
            mButtonCFNRy.setDependency(null);
        }

        // we wait to do the initialization until onResume so that the
        // TimeConsumingPreferenceActivity dialog can display as it
        // relies on onResume / onPause to maintain its foreground state.

        mFirstResume = true;
        mIcicle = icicle;

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
        /// M: Set toggled state before onPrepareDialogBuilder called. @{
        if (mIcicle != null) {
            for (CallForwardEditPreference pref : mPreferences) {
                Bundle bundle = mIcicle.getParcelable(pref.getKey());
                boolean isToggled = bundle.getBoolean(KEY_TOGGLE);
                pref.setToggled(isToggled);
                /// M: Need update preferences dependency state. @{
                if (pref.getKey().equals(BUTTON_CFU_KEY) && isToggled) {
                    Log.d(LOG_TAG, "Need update other CF items");
                    mUpdated = true;
                }
                /// @}
            }
        }
        /// @}
    }

    @Override
    public void onResume() {
        super.onResume();

        if (mFirstResume) {
            if (mIcicle == null) {
                Log.d(LOG_TAG, "start to init ");
                CallForwardEditPreference pref = mPreferences.get(mInitIndex);
                pref.init(this, mPhone, mReplaceInvalidCFNumbers, mCallForwardByUssd);
                pref.startCallForwardOptionsQuery();

            } else {
                mInitIndex = mPreferences.size();

                for (CallForwardEditPreference pref : mPreferences) {
                    Bundle bundle = mIcicle.getParcelable(pref.getKey());
                    pref.setToggled(bundle.getBoolean(KEY_TOGGLE));
                    pref.setEnabled(bundle.getBoolean(KEY_ENABLE));
                    CallForwardInfo cf = new CallForwardInfo();
                    cf.number = bundle.getString(KEY_NUMBER);
                    cf.status = bundle.getInt(KEY_STATUS);
                    pref.init(this, mPhone, mReplaceInvalidCFNumbers, mCallForwardByUssd);
                    pref.restoreCallForwardInfo(cf);
                }
            }
            mFirstResume = false;
            mIcicle = null;
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        for (CallForwardEditPreference pref : mPreferences) {
            Bundle bundle = new Bundle();
            bundle.putBoolean(KEY_TOGGLE, pref.isToggled());
            bundle.putBoolean(KEY_ENABLE, pref.isEnabled());
            if (pref.callForwardInfo != null) {
                bundle.putString(KEY_NUMBER, pref.callForwardInfo.number);
                bundle.putInt(KEY_STATUS, pref.callForwardInfo.status);
            }
            outState.putParcelable(pref.getKey(), bundle);
        }
    }

    @Override
    public void onFinished(Preference preference, boolean reading) {
        /// M: [CT VOLTE] stop init other preference if there is ut error. @{
        boolean hasUtError = false;
        if (preference instanceof CallForwardEditPreference) {
            CallForwardEditPreference cfPreference = (CallForwardEditPreference) preference;
            hasUtError = cfPreference.hasUtError();
        }
        if (mInitIndex < mPreferences.size() - 1 && !isFinishing() && !hasUtError) {
        /// @}
            mInitIndex++;
            CallForwardEditPreference pref = mPreferences.get(mInitIndex);
            pref.init(this, mPhone, mReplaceInvalidCFNumbers, mCallForwardByUssd);
            pref.startCallForwardOptionsQuery();
        }

        super.onFinished(preference, reading);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(LOG_TAG, "onActivityResult: done");
        if (resultCode != RESULT_OK) {
            Log.d(LOG_TAG, "onActivityResult: contact picker result not OK.");
            return;
        }
        Cursor cursor = null;
        try {
            cursor = getContentResolver().query(data.getData(),
                NUM_PROJECTION, null, null, null);
            if ((cursor == null) || (!cursor.moveToFirst())) {
                Log.d(LOG_TAG, "onActivityResult: bad contact data, no results found.");
                return;
            }

            switch (requestCode) {
                case CommandsInterface.CF_REASON_UNCONDITIONAL:
                    mButtonCFU.onPickActivityResult(cursor.getString(0));
                    break;
                case CommandsInterface.CF_REASON_BUSY:
                    mButtonCFB.onPickActivityResult(cursor.getString(0));
                    break;
                case CommandsInterface.CF_REASON_NO_REPLY:
                    mButtonCFNRy.onPickActivityResult(cursor.getString(0));
                    break;
                case CommandsInterface.CF_REASON_NOT_REACHABLE:
                    mButtonCFNRc.onPickActivityResult(cursor.getString(0));
                    break;
                default:
                    // TODO: may need exception here.
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int itemId = item.getItemId();
        if (itemId == android.R.id.home) {  // See ActionBar#setDisplayHomeAsUpEnabled()
            CallFeaturesSetting.goUpToTopLevelSetting(this, mSubscriptionInfoHelper);
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    // -------------------------------MTK-----------------------------
    private IntentFilter mIntentFilter;
    private boolean mUpdated = false;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
                if (intent.getBooleanExtra("state", false)) {
                    Log.d(LOG_TAG, "finish activity in airplane mode");
                    finish();
                }
            }
        }
    };

    @Override
    public void onUpdated(Preference preference) {
        Log.d(LOG_TAG, "onUpdated, preference=" + preference.getKey());
        if ((preference instanceof CallForwardEditPreference) && mUpdated) {
            CallForwardEditPreference cfPreference = (CallForwardEditPreference) preference;
            if (cfPreference.reason == mButtonCFU.reason &&
                    cfPreference.callForwardInfo != null &&
                    cfPreference.callForwardInfo.status == 0) {
                Log.d(LOG_TAG, "Update other CF items");
                mButtonCFB.setEnabled(true);
                mButtonCFNRy.setEnabled(true);
                mButtonCFNRc.setEnabled(true);
                mUpdated = false;
            }
        }
        super.onUpdated(preference);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        boolean shouldDismissDialog = newConfig.orientation == Configuration.ORIENTATION_PORTRAIT;
        Log.d(LOG_TAG, "onConfigurationChanged: shouldDismissDialog=" + shouldDismissDialog +
                ", mIsForeground=" + mIsForeground);
        if (shouldDismissDialog && !mIsForeground) {
            Dialog dialog = mButtonCFU.getDialog();
            if (dialog != null && dialog.isShowing()) {
                Log.d(LOG_TAG, "onConfigurationChanged: showPhoneNumberDialog for CFU");
                dialog.dismiss();
                mButtonCFU.showPhoneNumberDialog();
                return;
            }
            dialog = mButtonCFB.getDialog();
            if (dialog != null && dialog.isShowing()) {
                Log.d(LOG_TAG, "onConfigurationChanged: showPhoneNumberDialog for CFB");
                dialog.dismiss();
                mButtonCFB.showPhoneNumberDialog();
                return;
            }
            dialog = mButtonCFNRy.getDialog();
            if (dialog != null && dialog.isShowing()) {
                Log.d(LOG_TAG, "onConfigurationChanged: showPhoneNumberDialog for CFNRy");
                dialog.dismiss();
                mButtonCFNRy.showPhoneNumberDialog();
                return;
            }
            dialog = mButtonCFNRc.getDialog();
            if (dialog != null && dialog.isShowing()) {
                Log.d(LOG_TAG, "onConfigurationChanged: showPhoneNumberDialog for mButtonCFNRc");
                dialog.dismiss();
                mButtonCFNRc.showPhoneNumberDialog();
                return;
            }
        }
    }

    private void registerCallBacks() {
        mIntentFilter = new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        registerReceiver(mReceiver, mIntentFilter);
    }

    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
        super.onDestroy();
    }

    @Override
    public void handleSubInfoUpdate() {
        finish();
    }
}
