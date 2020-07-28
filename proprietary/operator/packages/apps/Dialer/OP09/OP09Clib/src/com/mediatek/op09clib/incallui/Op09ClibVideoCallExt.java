package com.mediatek.op09clib.incallui;


import android.content.Context;
import android.telecom.VideoProfile;
import android.util.Log;
import android.widget.Toast;
import android.os.SystemProperties;


import android.telecom.Call;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.TelephonyManager;


import com.mediatek.op09clib.dialer.R;
import com.mediatek.incallui.ext.DefaultVideoCallExt;

import java.util.Arrays;
import java.util.List;


/**
 * Plugin implementation for VideoCall.
 */
public class Op09ClibVideoCallExt extends DefaultVideoCallExt {
    private static final String TAG = "Op09ClibVideoCallExt";
    private Context mContext;
    private int SESSION_EVENT_BAD_DATA_BITRATE = 4008;
    private int WAITING_FOR_DOWNGRADE_RESPONSE = 6;
    private final static List<String> OP09_MCCMNC_LIST = Arrays.asList(
               "45502", "46003", "46011", "46012", "46013");
    /** Constructor.
     * @param context context
     */
    public Op09ClibVideoCallExt(Context context) {
        super();
        mContext = context;
    }

    @Override
    public void onCallSessionEvent(Object call, int event) {
        Log.d(TAG, "[onCallSessionEvent]downgrade due to insufficient bandwidth event"
                + event);
        if (call == null )
            return;
        com.android.incallui.call.DialerCall incallui = (com.android.incallui.call.DialerCall)call;
        Call telecomCall = incallui.getTelecomCall();
        if (telecomCall != null && (OP09_MCCMNC_LIST.contains(getOperatorName(telecomCall))
                || isTestSim())) {
            if (event == SESSION_EVENT_BAD_DATA_BITRATE && telecomCall.getVideoCall() != null) {
                Log.d(TAG, "[onCallSessionEvent]downgrade to audio for CT requirement");
                telecomCall.getVideoCall().sendSessionModifyRequest(
                    new VideoProfile(VideoProfile.STATE_AUDIO_ONLY));
                Toast.makeText(mContext,
                    mContext.getString(R.string.video_call_downgrade_insufficient_bandwidth),
                    Toast.LENGTH_SHORT).show();
        }
    }
}

      /**
     * FIXME: If some operator do not support some video features, the info should be passed
     * by PhoneAccount instead of hard coding in InCallUI.
     * M: The way to get operator name from call.
     */
    private String getOperatorName(Call telecomCall) {
        PhoneAccountHandle phoneAccountHandle = telecomCall.getDetails().getAccountHandle();
        if (phoneAccountHandle == null) {
            return "";
        }
        TelecomManager telecomManager = (TelecomManager) mContext
                .getSystemService(Context.TELECOM_SERVICE);
        PhoneAccount phoneAccount = telecomManager.getPhoneAccount(phoneAccountHandle);
        if (phoneAccount == null) {
            return "";
        }
        TelephonyManager telephonyManager = (TelephonyManager) mContext
                .getSystemService(Context.TELEPHONY_SERVICE);
        int subId = telephonyManager.getSubIdForPhoneAccount(phoneAccount);
        String mccMnc = telephonyManager.getSimOperator(subId);
        Log.d(TAG, "getOperatorName : " + mccMnc);
        return mccMnc;
    }

     private boolean isTestSim() {
        boolean isTestSim = false;
        isTestSim = SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                   SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1");
        Log.d(TAG, "isTestSim :" + isTestSim);
        return isTestSim;
    }
}
