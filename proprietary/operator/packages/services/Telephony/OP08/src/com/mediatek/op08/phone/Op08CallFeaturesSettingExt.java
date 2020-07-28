package com.mediatek.op08.phone;

import android.content.Context;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.util.Log;

import com.android.internal.telephony.Phone;
import com.android.phone.CallFeaturesSetting;
import com.mediatek.op08.phone.WfcSummary.WfcSummaryChangeListener;
import com.mediatek.phone.ext.DefaultCallFeaturesSettingExt;

/**
 * Plugin implementation for CallfeatureSettings.
 */
public class Op08CallFeaturesSettingExt extends DefaultCallFeaturesSettingExt
        implements WfcSummaryChangeListener {
    private static final String TAG = "Op08CallFeaturesSettingExt";
    private static final String KEY_WFC_SETTINGS = "button_wifi_calling_settings_key";

    private Context mContext;
    private Preference mWfcPreference = null;
    private WfcSummary mWfcSummary;
    private Phone mPhone;

    /** Constructor.
     * @param context context
     */
    public Op08CallFeaturesSettingExt(Context context) {
        super();
        mContext = context;
        mWfcSummary = new WfcSummary(context);
    }

    @Override
    public void initOtherCallFeaturesSetting(PreferenceActivity activity, Object phone) {
        mWfcPreference = ((CallFeaturesSetting) activity).getPreferenceScreen()
                .findPreference(KEY_WFC_SETTINGS);
        mPhone = (Phone) phone;
        mWfcSummary.onCreate(mPhone);
    }

    /**
        * On wfc summary changed.
        * @param summary The current wfc summary
        */
    @Override
    public void onWfcSummaryChanged(String summary) {
        if (mWfcPreference != null) {
            mWfcPreference.setSummary(summary);
        }
    }

    /** Called from onPause
     * @param activity
     * @return
     */
    @Override
    public void onCallFeatureSettingsEvent(int event) {
        Log.d(TAG, "wfcPreference:" + mWfcPreference + ", event:" + event);
        switch (event) {
            case DefaultCallFeaturesSettingExt.RESUME:
                if (mWfcPreference != null && mPhone != null) {
                    boolean isWfcRegistered = mPhone.isWifiCallingEnabled();
                    Log.d(TAG, "isWfcRegistered:" + isWfcRegistered);
                    mWfcSummary.setWfcRegistered(isWfcRegistered);
                    int wfcState = WfcReasonInfo.CODE_WFC_DEFAULT;
                    if (isWfcRegistered) {
                        wfcState = WfcReasonInfo.CODE_WFC_SUCCESS;
                    }
                    mWfcPreference.setSummary(mWfcSummary.getWfcSummaryText(wfcState));
                }
                mWfcSummary.onResume(this);
                break;
            case DefaultCallFeaturesSettingExt.PAUSE:
                mWfcSummary.onPause(this);
                break;

            default:
                break;
        }
    }
}
