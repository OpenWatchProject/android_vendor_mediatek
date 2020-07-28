package com.mediatek.mms.plugin;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.preference.CheckBoxPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.mediatek.mms.ext.DefaultOpSmsPreferenceActivityExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import android.telephony.TelephonyManager;

import java.util.List;

/**
 * Op09SmsPreferenceActivityExt.
 *
 */
public class Op09SmsPreferenceActivityExt extends
        DefaultOpSmsPreferenceActivityExt {

    private static final String TAG = "Op09SmsPreferenceActivityExt";
    private static final String SMS_DELIVERY_REPORT_MODE = "pref_key_sms_delivery_reports";
    private PreferenceActivity mActivity;
    private CheckBoxPreference mSmsDeliveryReport;
    private int mSubId;


    @Override
    public void onCreate(PreferenceActivity activity) {
        mActivity = activity;
    }

    @Override
    public void changeSingleCardKeyToSimRelated() {
        List<SubscriptionInfo> subInfoList =
                SubscriptionManager.from(mActivity.getApplicationContext())
                        .getActiveSubscriptionInfoList();
        if (subInfoList == null || subInfoList.isEmpty()) {
            return;
        }
        mSubId = subInfoList.get(0).getSubscriptionId();
        // CT6M, Disable Usim card delivery report setting if is international roaming
        if (isUSimType(mSubId) && isInternationalRoamingStatus(mActivity, mSubId)) {
            mSmsDeliveryReport = (CheckBoxPreference) mActivity.findPreference(
                Integer.toString(mSubId) + "_" + SMS_DELIVERY_REPORT_MODE);
            if (mSmsDeliveryReport != null) {
                mSmsDeliveryReport.setEnabled(false);
                mSmsDeliveryReport.setChecked(false);
            }
        }
    }

   @Override
   public void onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference) {
       if (preference == mSmsDeliveryReport) {
           Log.d(TAG, "onPreferenceTreeClick SmsDeliveryReport");
           // CT6M, Disable Usim card delivery report setting if is international roaming
           if (isUSimType(mSubId) && isInternationalRoamingStatus(mActivity, mSubId)) {
               if (mSmsDeliveryReport.isChecked()) {
                   mSmsDeliveryReport.setChecked(false);
               //    mActivity.showToast(R.string.disable_delivery_report);
               }
           }
           return;
       }
   }

   /**
     * M: check the subId sim is whether in internation roaming status or not.
     * @param context the Context.
     * @param subId the subId.
     * @return true: in international romaing. false : not in.
     */
    private boolean isInternationalRoamingStatus(Context context, long subId) {
        boolean isRoaming = false;
        int simCount = SubscriptionManager.from(context.getApplicationContext())
                        .getActiveSubscriptionInfoCount();
        if (simCount <= 0) {
            Log.e(TAG, "isInternationalRoamingStatus(): Wrong subId!");
            return false;
        }
        /* P0 migration*/
        TelephonyManager telephonyManager = context.getSystemService(TelephonyManager.class);
        TelephonyManager telephonyManagerEx = telephonyManager.createForSubscriptionId((int)subId);
        isRoaming = telephonyManagerEx.isNetworkRoaming(/*(int) subId*/);
        Log.d(TAG, "isInternationalRoamingStatus() isRoaming: " + isRoaming);
        return isRoaming;
    }

    /**
     * M: For EVDO: check the sim is whether UIM or not.
     * @param subId the sim's sub id.
     * @return true: UIM; false: not UIM.
     */
    private boolean isUSimType(int subId) {
        String phoneType = MtkTelephonyManagerEx.getDefault().getIccCardType(subId);
        if (phoneType == null) {
            Log.d(TAG, "[isUIMType]: phoneType = null");
            return false;
        }
        Log.d(TAG, "[isUIMType]: phoneType = " + phoneType);
        return phoneType.equalsIgnoreCase("CSIM") || phoneType.equalsIgnoreCase("UIM")
            || phoneType.equalsIgnoreCase("RUIM");
    }
}
