/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

package com.mediatek.dialer.util;

import java.util.ArrayList;
import java.util.List;

import android.Manifest.permission;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.provider.Settings;
import android.telecom.PhoneAccount;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.widget.Toast;

import com.android.dialer.R;
import com.android.dialer.callintent.CallInitiationType;
import com.android.dialer.callintent.CallIntentBuilder;
import com.android.dialer.common.LogUtil;
import com.android.dialer.util.CallUtil;
import com.android.dialer.util.DialerUtils;
import com.android.dialer.util.PermissionsUtil;
import com.mediatek.contacts.util.ContactsIntent;
import com.mediatek.dialer.compat.TelecomCompat;
import com.mediatek.dialer.compat.TelecomCompat.PhoneAccountCompat;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import mediatek.telephony.MtkCarrierConfigManager;
import mediatek.telecom.MtkTelecomManager;

/**
 * M: [VoLTE ConfCall] A util class for supporting the VOLTE features
 */
public class DialerVolteUtils {
    private static final String TAG = "VolteUtils";
    public static final int ACTIVITY_REQUEST_CODE_PICK_PHONE_CONTACTS = 101;
    public static final String EXTRA_CALL_CREATED_TIME_MILLIS =
                           "android.telecom.extra.CALL_CREATED_TIME_MILLIS";

    /**
     * [VoLTE ConfCall] Launch the contacts choice activity to pick participants.
     */
    public static void handleMenuVolteConfCall(Activity activity) {
        Intent intent = new Intent();
        intent.setAction(ContactsIntent.LIST.ACTION_PICK_MULTI_PHONEANDIMSANDSIPCONTACTS);
        intent.setType(Phone.CONTENT_TYPE);
        intent.putExtra(ContactsIntent.CONFERENCE_CALL_LIMIT_NUMBER,
                ContactsIntent.CONFERENCE_CALL_LIMITES);
        DialerUtils.startActivityForResultWithErrorToast(activity, intent,
                ACTIVITY_REQUEST_CODE_PICK_PHONE_CONTACTS);
    }

    /**
     * [VoLTE ConfCall] Launch volte conference call according the picked contacts.
     */
    public static void launchVolteConfCall(Activity activity, Intent data) {
        final long[] dataIds = data.getLongArrayExtra(
                ContactsIntent.CONFERENCE_CALL_RESULT_INTENT_EXTRANAME);
        LogUtil.i(TAG, "launchVolteConfCall enter, data = " + data);

        // Add contacts permission check
        if (!PermissionsUtil.hasContactsReadPermissions(activity)) {
            Toast.makeText(activity, R.string.missing_required_permission,
                    Toast.LENGTH_SHORT).show();
            LogUtil.i(TAG, "No Contacts Read Permissions, return!");
            return;
        }
        if (dataIds == null || dataIds.length <= 0) {
            LogUtil.i(TAG, "Volte conf call, the selected contacts is empty");
            return;
        }
        new LaunchVolteConfCallTask(activity).execute(dataIds);
    }

    private static class LaunchVolteConfCallTask extends
            AsyncTask<long[], Void, ArrayList<String>> {

        Activity mActivity;
        LaunchVolteConfCallTask(Activity activity) {
            mActivity = activity;
        }
        @Override
        protected ArrayList<String> doInBackground(long[]... arg0) {
            return getPhoneNumberByDataIds(mActivity, arg0[0]);
        }

        @Override
        protected void onPostExecute(ArrayList<String> result) {
            if (mActivity.isFinishing()) {
                LogUtil.i(TAG, "Volte conf call, Activity has finished");
                return;
            }
            if (result.size() <= 0) {
                LogUtil.i(TAG, "Volte conf call, No phone numbers");
                return;
            }

            LogUtil.i(TAG, "LaunchVolteConfCallTask onPostExecute enter");

            //Intent confCallIntent = new CallIntentBuilder(result.get(0),
            //    CallInitiationType.Type.DIALPAD).build();
            //confCallIntent.putExtra(TelecomCompat.EXTRA_VOLTE_CONF_CALL_DIAL, true);
            //confCallIntent.putStringArrayListExtra(
            //    TelecomCompat.EXTRA_VOLTE_CONF_CALL_NUMBERS, result);
            /// M: Fix for ALPS03647027 @{
            Intent confCallIntent =
              MtkTelecomManager.createConferenceInvitationIntent(mActivity.getApplicationContext());
            Uri uri = CallUtil.getCallUri(result.get(0));
            confCallIntent.setData(uri);
            confCallIntent.putExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                VideoProfile.STATE_AUDIO_ONLY);

            Bundle extras = new Bundle();
            extras.putLong(EXTRA_CALL_CREATED_TIME_MILLIS,SystemClock.elapsedRealtime());
            confCallIntent.putExtra(TelecomManager.EXTRA_OUTGOING_CALL_EXTRAS, extras);
            /// @}
            confCallIntent.putStringArrayListExtra(
                TelecomCompat.EXTRA_VOLTE_CONF_CALL_NUMBERS, result);
            DialerUtils.startActivityWithErrorToast(mActivity, confCallIntent);
        }
    }

    private static ArrayList<String> getPhoneNumberByDataIds(
            Context context, long[] dataIds) {
        ArrayList<String> phoneNumbers = new ArrayList<String>();
        if (dataIds == null || dataIds.length <= 0) {
            return phoneNumbers;
        }
        StringBuilder selection = new StringBuilder();
        selection.append(Data._ID);
        selection.append(" IN (");
        selection.append(dataIds[0]);
        for (int i = 1; i < dataIds.length; i++) {
            selection.append(",");
            selection.append(dataIds[i]);
        }
        selection.append(")");
        LogUtil.i(TAG, "getPhoneNumberByDataIds dataIds " + selection.toString());
        Cursor c = null;
        try {
            c = context.getContentResolver().query(Data.CONTENT_URI,
                    new String[]{Data._ID, Data.DATA1},
                    selection.toString(), null, null);
            if (c == null) {
                return phoneNumbers;
            }
            while (c.moveToNext()) {
                LogUtil.i(TAG, "getPhoneNumberByDataIds got"
                        + " _ID=" + c.getInt(0)
                        + ", NUMBER=" + LogUtil.sanitizePhoneNumber(c.getString(1)));
                phoneNumbers.add(c.getString(1));
            }
        } finally {
            if (c != null) {
                c.close();
            }
        }
        return phoneNumbers;
    }

    /**
     * Returns whether the VoLTE conference call enabled.
     * @param context the context
     * @return true if the VOLTE is supported and has Volte phone account
     */
    public static boolean isVolteConfCallEnable(Context context) {
        if (!DialerFeatureOptions.isVolteEnhancedConfCallSupport() || context == null
            ///M:configManager.getConfigForSubId need READ_PHONE_STATE permission
            || !PermissionsUtil.hasPermission(context, permission.READ_PHONE_STATE)) {
            LogUtil.i(TAG, "DialerFeatureOptions.isVolteEnhancedConfCallSupport() = " +
                DialerFeatureOptions.isVolteEnhancedConfCallSupport());
            LogUtil.i(TAG, "PermissionsUtil.hasPermission(context, permission.READ_PHONE_STATE) = " +
                PermissionsUtil.hasPermission(context, permission.READ_PHONE_STATE));
            return false;
        }
        final TelecomManager telecomManager = (TelecomManager) context
                .getSystemService(Context.TELECOM_SERVICE);
        TelephonyManager telephonyManager = context.getSystemService(TelephonyManager.class);
        CarrierConfigManager configManager = (CarrierConfigManager) context
            .getSystemService(Context.CARRIER_CONFIG_SERVICE);

        List<PhoneAccount> phoneAccouts = telecomManager.getAllPhoneAccounts();
        for (PhoneAccount phoneAccount : phoneAccouts) {
            int subId = telephonyManager.getSubIdForPhoneAccount(phoneAccount);
            boolean isVolteEnabled = MtkTelephonyManagerEx.getDefault().isVolteEnabled(subId);
            LogUtil.i(TAG, "isVolteEnabled = " + isVolteEnabled);
            PersistableBundle bundle = configManager.getConfigForSubId(subId);
            LogUtil.i(TAG, "isVolteConfCallEnable subId:" + subId);
            if (isVolteEnabled
                && bundle != null
                && bundle.getBoolean(
                  MtkCarrierConfigManager.MTK_KEY_VOLTE_CONFERENCE_ENHANCED_ENABLE_BOOL)) {
                  LogUtil.i(TAG,
                     "isVolteConfCallEnable MTK_KEY_VOLTE_CONFERENCE_ENHANCED_ENABLE_BOOL TRUE");
                return true;
            }
        }
        LogUtil.i(TAG, "return false ");
        return false;
    }
    /// M: For RTT call @{
    private static boolean isRttSettingOn(Context context) {
        return Settings.Secure.getInt(context.getContentResolver(),
                 Settings.Secure.RTT_CALLING_MODE,0) != 0;
    }

    private static boolean isSimAbsent(Context context) {
        TelephonyManager tm = (TelephonyManager)
            context.getSystemService(Context.TELEPHONY_SERVICE);
        for (int i = 0; i < tm.getPhoneCount(); i++) {
            if (tm.getSimState(i) != tm.SIM_STATE_ABSENT) {
                return false;
            }
        }
        return true;
    }
    /// @}
    ///M: For RTT call @{
    public static boolean isRttCallSupport(Context context) {
        TelephonyManager tm = (TelephonyManager)
            context.getSystemService(Context.TELEPHONY_SERVICE);
        boolean isRttSupport = tm.isRttSupported();
        boolean isRttSetOn = isRttSettingOn(context);
        boolean isSimAbsent = isSimAbsent(context);
        LogUtil.i(TAG, "isRttSupport = " + isRttSupport + " isRttSetOn = " + isRttSetOn);
        LogUtil.i(TAG, "isSimAbsent = " + isSimAbsent);
        return (isRttSupport || isSimAbsent) && isRttSetOn;

    }
    ///@}

  ///M: Add for or ALPS04286464 , for GCF test @ {
  public static boolean isTestSim() {
    boolean isTestSim = false;
    isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
    LogUtil.i(TAG, "isTestSim = " + isTestSim);
    return isTestSim;
  }
  /// @}

   ///M: @{
   public static void handleMenuEmptyVideoConfCall(Activity activity) {
     if (isTestSim()) {
          LogUtil.i(TAG, "handleMenuEmptyVideoConfCall enter");
          ArrayList<String> phoneNumbers = new ArrayList<String>();
          Intent confCallIntent =
             MtkTelecomManager.createConferenceInvitationIntent(activity.getApplicationContext());
          Uri uri = CallUtil.getCallUri("");
          confCallIntent.setData(uri);
          confCallIntent.putExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
               VideoProfile.STATE_BIDIRECTIONAL);
           Bundle extras = new Bundle();
          extras.putLong(EXTRA_CALL_CREATED_TIME_MILLIS, SystemClock.elapsedRealtime());
          confCallIntent.putExtra(TelecomManager.EXTRA_OUTGOING_CALL_EXTRAS, extras);
	  confCallIntent.putStringArrayListExtra(
               TelecomCompat.EXTRA_VOLTE_CONF_CALL_NUMBERS, phoneNumbers);
          DialerUtils.startActivityWithErrorToast(activity, confCallIntent);
         return;
      }
   }
   /// @}


  ///M: @{
  public static void handleMenuEmptyVoiceConfCall(Activity activity) {
       if (isTestSim()) {
           LogUtil.i(TAG, "handleMenuEmptyVoiceConfCall enter");
           ArrayList<String> phoneNumbers = new ArrayList<String>();
           Intent confCallIntent =
              MtkTelecomManager.createConferenceInvitationIntent(activity.getApplicationContext());
           Uri uri = CallUtil.getCallUri("");
           confCallIntent.setData(uri);
           confCallIntent.putExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                VideoProfile.STATE_AUDIO_ONLY);

           Bundle extras = new Bundle();
           extras.putLong(EXTRA_CALL_CREATED_TIME_MILLIS, SystemClock.elapsedRealtime());
           confCallIntent.putExtra(TelecomManager.EXTRA_OUTGOING_CALL_EXTRAS, extras);
       confCallIntent.putStringArrayListExtra(
                TelecomCompat.EXTRA_VOLTE_CONF_CALL_NUMBERS, phoneNumbers);
           DialerUtils.startActivityWithErrorToast(activity, confCallIntent);
          return;
       }
  }
  ///@}
}

