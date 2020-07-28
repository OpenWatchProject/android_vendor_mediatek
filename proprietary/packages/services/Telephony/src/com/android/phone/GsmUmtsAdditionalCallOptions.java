package com.android.phone;

import android.app.ActionBar;
import android.app.Dialog;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.preference.Preference;
import android.preference.PreferenceScreen;
import android.telephony.CarrierConfigManager;
import android.util.Log;
import android.view.MenuItem;

import com.android.internal.telephony.Phone;
import com.android.phone.settings.SuppServicesUiUtil;
import com.mediatek.phone.ext.ExtensionManager;

import java.util.ArrayList;

public class GsmUmtsAdditionalCallOptions extends TimeConsumingPreferenceActivity
        implements PhoneGlobals.SubInfoUpdateListener {
    private static final String LOG_TAG = "GsmUmtsAdditionalCallOptions";
    private final boolean DBG = true; //(PhoneGlobals.DBG_LEVEL >= 2);

    public static final String BUTTON_CLIR_KEY  = "button_clir_key";
    public static final String BUTTON_CW_KEY    = "button_cw_key";

    private static final int CW_WARNING_DIALOG = 201;
    private static final int CALLER_ID_WARNING_DIALOG = 202;

    private CLIRListPreference mCLIRButton;
    private CallWaitingSwitchPreference mCWButton;

    private final ArrayList<Preference> mPreferences = new ArrayList<Preference>();
    private int mInitIndex = 0;
    private Phone mPhone;
    private SubscriptionInfoHelper mSubscriptionInfoHelper;

    private boolean mShowCLIRButton = true;
    private boolean mShowCWButton = true;
    private boolean mCLIROverUtPrecautions = false;
    private boolean mCWOverUtPrecautions = false;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.gsm_umts_additional_options);

        mSubscriptionInfoHelper = new SubscriptionInfoHelper(this, getIntent());
        mSubscriptionInfoHelper.setActionBarTitle(
                getActionBar(), getResources(), R.string.additional_gsm_call_settings_with_label);
        mPhone = mSubscriptionInfoHelper.getPhone();

        /// M: Add for MTK hotswap @{
        PhoneGlobals.getInstance().addSubInfoUpdateListener(this);
        if (mPhone == null) {
            Log.d(LOG_TAG, "onCreate: mPhone is null, finish!!!");
            finish();
            return;
        }
        /// @}

        PreferenceScreen prefSet = getPreferenceScreen();
        mCLIRButton = (CLIRListPreference) prefSet.findPreference(BUTTON_CLIR_KEY);
        mCWButton = (CallWaitingSwitchPreference) prefSet.findPreference(BUTTON_CW_KEY);

        PersistableBundle b = null;
        if (mSubscriptionInfoHelper.hasSubId()) {
            b = PhoneGlobals.getInstance().getCarrierConfigForSubId(
                    mSubscriptionInfoHelper.getSubId());
        } else {
            b = PhoneGlobals.getInstance().getCarrierConfig();
        }

        if (b != null) {
            mShowCLIRButton = b.getBoolean(
                    CarrierConfigManager.KEY_ADDITIONAL_SETTINGS_CALLER_ID_VISIBILITY_BOOL);
            mShowCWButton = b.getBoolean(
                    CarrierConfigManager.KEY_ADDITIONAL_SETTINGS_CALL_WAITING_VISIBILITY_BOOL);
            mCLIROverUtPrecautions = mShowCLIRButton && b.getBoolean(
                    CarrierConfigManager.KEY_CALLER_ID_OVER_UT_WARNING_BOOL);
            mCWOverUtPrecautions = mShowCWButton && b.getBoolean(
                    CarrierConfigManager.KEY_CALL_WAITING_OVER_UT_WARNING_BOOL);
            if (DBG) {
                Log.d(LOG_TAG, "mCLIROverUtPrecautions:" + mCLIROverUtPrecautions
                        + ",mCWOverUtPrecautions:" + mCWOverUtPrecautions);
            }
        }

        boolean isSsOverUtPrecautions = SuppServicesUiUtil.isSsOverUtPrecautions(this, mPhone);

        if (mCLIRButton != null) {
            if (mShowCLIRButton) {
                if (mCLIROverUtPrecautions && isSsOverUtPrecautions) {
                    mCLIRButton.setEnabled(false);
                } else {
                    mPreferences.add(mCLIRButton);
                }
            } else {
                prefSet.removePreference(mCLIRButton);
            }
        }

        if (mCWButton != null) {
            if (mShowCWButton) {
                if (mCWOverUtPrecautions && isSsOverUtPrecautions) {
                    mCWButton.setEnabled(false);
                } else {
                    mPreferences.add(mCWButton);
                }
            } else {
                prefSet.removePreference(mCWButton);
            }
        }

        /// M: adjust the waiting dialog show time firstly
        mIsForeground = true;

        /// M: Add for plug-in, customize the additional settings @{
        ExtensionManager.getCallFeaturesSettingExt().customizeAdditionalSettings(this, mPhone);
        /// @}

        if (mPreferences.size() != 0) {
            if (icicle == null) {
                if (DBG) Log.d(LOG_TAG, "start to init ");
                doPreferenceInit(mInitIndex);
            } else {
                if (DBG) Log.d(LOG_TAG, "restore stored states");
                mInitIndex = mPreferences.size();
                ///M: Save enabled state of CW and CLIR @{
                if (mShowCWButton && mCWButton != null) {
                ///@}
                    mCWButton.init(this, true, mPhone);
                    ///M: Save enabled state of CW and CLIR @{
                    Bundle bundle = icicle.getParcelable(mCWButton.getKey());
                    if (bundle != null) {
                        mCWButton.setChecked(bundle.getBoolean(KEY_TOGGLE));
                        mCWButton.setEnabled(bundle.getBoolean(KEY_STATE));
                    }
                    ///M: @}
                }
                ///M: Save enabled state of CW and CLIR @{
                if (mShowCLIRButton && mCLIRButton != null) {
                ///@}
                    mCLIRButton.init(this, true, mPhone);
                    int[] clirArray = icicle.getIntArray(mCLIRButton.getKey());
                    if (clirArray != null) {
                        if (DBG) {
                            Log.d(LOG_TAG, "onCreate:  clirArray[0]="
                                    + clirArray[0] + ", clirArray[1]=" + clirArray[1]);
                        }
                        mCLIRButton.handleGetCLIRResult(clirArray);
                    } else {
                        mCLIRButton.init(this, false, mPhone);
                    }
                }
            }
        }

        ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            // android.R.id.home will be triggered in onOptionsItemSelected()
            actionBar.setDisplayHomeAsUpEnabled(true);
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        int indexOfStartInit = mPreferences.size();
        boolean isPrecaution = SuppServicesUiUtil.isSsOverUtPrecautions(this, mPhone);
        dismissWarningDialog();

        if (mShowCLIRButton && mCLIROverUtPrecautions && mCLIRButton != null) {
            if (isPrecaution) {
                showWarningDialog(CW_WARNING_DIALOG);
                if (mCLIRButton.isEnabled()) {
                    if (mPreferences.contains(mCLIRButton)) {
                        mPreferences.remove(mCLIRButton);
                    }
                    mCLIRButton.setEnabled(false);
                }
            } else {
                if (!mPreferences.contains(mCLIRButton)) {
                    mCLIRButton.setEnabled(true);
                    mPreferences.add(mCLIRButton);
                }
            }
        }
        if (mShowCWButton && mCWOverUtPrecautions && mCWButton != null) {
            if (isPrecaution) {
                showWarningDialog(CALLER_ID_WARNING_DIALOG);
                if (mCWButton.isEnabled()) {
                    if (mPreferences.contains(mCWButton)) {
                        mPreferences.remove(mCWButton);
                    }
                    mCWButton.setEnabled(false);
                }
            } else {
                if (!mPreferences.contains(mCWButton)) {
                    mCWButton.setEnabled(true);
                    mPreferences.add(mCWButton);
                }
            }
        }

        if (indexOfStartInit < mPreferences.size()) {
            mInitIndex = indexOfStartInit;
            doPreferenceInit(indexOfStartInit);
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        if (mShowCLIRButton && mCLIRButton.clirArray != null) {
            outState.putIntArray(mCLIRButton.getKey(), mCLIRButton.clirArray);
        }
        ///M: Save enabled state of CW and CLIR @{
        if (mShowCWButton && mCWButton != null) {
            Bundle bundle = new Bundle();
            bundle.putBoolean(KEY_TOGGLE, mCWButton.isChecked());
            bundle.putBoolean(KEY_STATE, mCWButton.isEnabled());
            outState.putBundle(mCWButton.getKey(), bundle);
        }
        ///@}
    }

    @Override
    public void onFinished(Preference preference, boolean reading) {
        if (mInitIndex < mPreferences.size() - 1 && !isFinishing()) {
            mInitIndex++;
            doPreferenceInit(mInitIndex);
        }
        super.onFinished(preference, reading);
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

    private void doPreferenceInit(int index) {
        if (mPreferences.size() > index) {
            Preference pref = mPreferences.get(index);
            if (pref instanceof CallWaitingSwitchPreference) {
                ((CallWaitingSwitchPreference) pref).init(this, false, mPhone);
            } else if (pref instanceof CLIRListPreference) {
                ((CLIRListPreference) pref).init(this, false, mPhone);
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        if (id == CW_WARNING_DIALOG) {
            return SuppServicesUiUtil.showBlockingSuppServicesDialog(this, mPhone, BUTTON_CW_KEY);
        } else if (id == CALLER_ID_WARNING_DIALOG) {
            return SuppServicesUiUtil.showBlockingSuppServicesDialog(this, mPhone, BUTTON_CLIR_KEY);
        }
        return super.onCreateDialog(id);
    }

    private void showWarningDialog(int id) {
        showDialog(id);
    }

    private void dismissWarningDialog() {
        dismissDialogSafely(CW_WARNING_DIALOG);
        dismissDialogSafely(CALLER_ID_WARNING_DIALOG);
    }

    // -------------------------------MTK-----------------------------
    private static final String KEY_TOGGLE = "toggle";
    private static final String KEY_STATE = "state";

    @Override
    public void onDestroy() {
        PhoneGlobals.getInstance().removeSubInfoUpdateListener(this);
        super.onDestroy();
    }

    @Override
    public void handleSubInfoUpdate() {
        finish();
    }
}
