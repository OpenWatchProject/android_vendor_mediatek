package com.mediatek.op18.mms;

import android.app.PendingIntent;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;

import com.mediatek.mms.ext.DefaultOpSmsSingleRecipientSenderExt;

import java.util.ArrayList;

import mediatek.telephony.MtkSmsManager;

public class  Op18SmsSingleRecipientSenderExt extends DefaultOpSmsSingleRecipientSenderExt {

    private static final String TAG = "Mms/ Op18SmsSingleRecipientSenderExt";

    @Override
    public boolean sendMessage(Context context, int subId, String dest, String serviceCenter,
            ArrayList<String> messages, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents) {
        Bundle extra = getSmsValidityParamBundleWhenSend(context, subId);
        if (extra != null) {
            MtkSmsManager smsManager = MtkSmsManager.getSmsManagerForSubscriptionId(subId);
            smsManager.sendMultipartTextMessageWithExtraParams(dest, serviceCenter, messages,
                    extra, sentIntents, deliveryIntents);
            return true;
        }
        return false;
    }

    /**
     * getSmsValidityParamBundleWhenSend.
     * @param context Context
     * @param subId subId
     * @return Bundle
     */
    private Bundle getSmsValidityParamBundleWhenSend(Context context, int subId) {
        Log.d(TAG, "getSmsValidityParamBundleWhenSend, subId = " + subId);
        Bundle extra = new Bundle();
        int valid = getSmsValiditybySubId(context, subId);
        extra.putInt("validity_period", valid);
        return extra;
    }

    private int getSmsValiditybySubId(Context context, int subId) {

        final int[] validities = {//SmsManager.VALIDITY_PERIOD_NO_DURATION,
                                    MtkSmsManager.VALIDITY_PERIOD_ONE_HOUR,
                                    MtkSmsManager.VALIDITY_PERIOD_SIX_HOURS,
                                    MtkSmsManager.VALIDITY_PERIOD_TWELVE_HOURS,
                                    MtkSmsManager.VALIDITY_PERIOD_ONE_DAY,
                                    MtkSmsManager.VALIDITY_PERIOD_MAX_DURATION};

        SharedPreferences spref = PreferenceManager.getDefaultSharedPreferences(context);
        final String validityKey = Integer.toString(subId) + "_" +
                                Op18MmsPreference.SMS_VALIDITY_PERIOD_PREFERENCE_KEY;
        int index = spref.getInt(validityKey, 0);
        if (index > validities.length) {
            index = 0;
        }
        Log.d(TAG, "getSmsValiditybysubId: result index = " + index + " validityKey = "
                                    + validityKey + ", validity =" + validities[index]);
        return validities[index];
    }
}