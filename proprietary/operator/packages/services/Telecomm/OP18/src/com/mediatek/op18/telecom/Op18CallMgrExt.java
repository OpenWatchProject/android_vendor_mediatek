package com.mediatek.op18.telecom;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.UserHandle;
import android.provider.CallLog.Calls;
import android.support.v4.content.LocalBroadcastManager;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.server.telecom.Call;
import com.android.server.telecom.TelecomSystem;
import com.mediatek.server.telecom.ext.DefaultCallMgrExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Plug in implementation for OP18 Call Mananger interfaces.
 */
public class Op18CallMgrExt extends DefaultCallMgrExt {
    private static final String LOG_TAG = "Op18CallMgrExt";
    private static final int BATTERY_PERCENTAGE = 15;
    private Context mContext = null;
    private Intent mIntent ;
    private boolean mHandleIntent ;
    private static final String DOWNGRADE_TO_AUDIO = "com.mtk.plugin.DOWNGRADE_TO_AUDIO";
    private static final String CONTINUE_AS_VIDEO = "com.mtk.plugin.CONTINUE_AS_VIDEO";
    private static final String SHOW_VIDEO_CALL = "show_video";
    private static final int FEATURES_VoLTE = 0x4;
    private static final int FEATURES_VoWIFI = 0x8;
    private static final int FEATURES_ViWIFI = 0x10;

    /** Constructor.
     * @param context context
     */
    public Op18CallMgrExt(Context context) {
       mContext = context;
       mHandleIntent = false;
    }

    @Override
    public int getCallFeatures(Object callObj, int callFeatures) {
        Log.d(LOG_TAG, "getCallFeatures");
        Call call = (Call) callObj;
        TelecomManager tmgr = TelecomManager.from(mContext);
        TelephonyManager telephonyManager = TelephonyManager.from(mContext);
        PhoneAccountHandle callPhoneAccountHandle =
                               call.getConnectionManagerPhoneAccount();
        if (callPhoneAccountHandle != null) {
            PhoneAccount phoneAccount = tmgr.getPhoneAccount(callPhoneAccountHandle);
            if (phoneAccount == null) {
                return callFeatures;
            }

            int subId = telephonyManager.getSubIdForPhoneAccount(phoneAccount);
            boolean volteEnabled = MtkTelephonyManagerEx.getDefault().isVolteEnabled(subId);
            boolean wifiCallingEnabled = MtkTelephonyManagerEx.getDefault()
                                                   .isWifiCallingEnabled(subId);
            Log.d(LOG_TAG, "subId:" + subId + "\nvolteEnabled:" + volteEnabled +
                           "\nwifiCallingEnabled:" + wifiCallingEnabled);
            if (wifiCallingEnabled && (callFeatures == Calls.FEATURES_VIDEO)) {
                Log.d(LOG_TAG, "logNumber Video Over Wifi:" + FEATURES_ViWIFI);
                callFeatures = FEATURES_ViWIFI;
            } else if (wifiCallingEnabled) {
                Log.d(LOG_TAG, "logNumber Wifi Call:" + FEATURES_VoWIFI);
                callFeatures = FEATURES_VoWIFI;
            } else if (volteEnabled && (callFeatures != Calls.FEATURES_VIDEO)) {
                Log.d(LOG_TAG, "logNumber Volte Call:" + FEATURES_VoLTE);
                callFeatures = FEATURES_VoLTE;
            } else {
                 Log.d(LOG_TAG, "logNumber as Normal call:" + callFeatures);
            }
        }
        return callFeatures;
    }

    @Override
    public boolean shouldPreventVideoCallIfLowBattery(Context context, Intent intent) {
        int intentVideoState = intent.getIntExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                VideoProfile.STATE_AUDIO_ONLY);
        if (!VideoProfile.isVideo(intentVideoState)) {
            return false;
        }
        float batteryLevel = batteryLevel(mContext);
        if (batteryLevel <= BATTERY_PERCENTAGE) {
            mIntent = intent;
            mHandleIntent = true;
            IntentFilter filter = new IntentFilter();
            filter.addAction(DOWNGRADE_TO_AUDIO);
            filter.addAction(CONTINUE_AS_VIDEO);
            LocalBroadcastManager.getInstance(mContext).registerReceiver(mReceiver, filter);

            final Intent errorIntent = new Intent(mContext, ErrorDialogActivity.class);
            errorIntent.putExtra(SHOW_VIDEO_CALL, true);
            errorIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivityAsUser(errorIntent, UserHandle.CURRENT);
            return true;
        } else {
            return false;
        }
    }

    /**
     * log Emergency number or not.
     *
     * @return whether log ECC or not.
     *
     */
    @Override
    public boolean shouldLogEmergencyNumber() {
        Log.d(LOG_TAG, "Log Emergency Number");
        return true;
    }

    private  BroadcastReceiver mReceiver = new BroadcastReceiver() {
       @Override
       public void onReceive(Context context, Intent intent) {
           if (false == mHandleIntent) {
               Log.d(LOG_TAG, "Ignore intent");
               return;
           }
           String action = intent.getAction();
           if (intent.getAction().equals(DOWNGRADE_TO_AUDIO)) {
               Log.d(LOG_TAG, "DOWNGRADE_TO_AUDIO");
               mHandleIntent = false;
               convertCall();
           } else if (intent.getAction().equals(CONTINUE_AS_VIDEO)) {
               Log.d(LOG_TAG, "CONTINUE_AS_VIDEO");
               mHandleIntent = false;
               continueCall();
           }
       }
    };

    private  void convertCall() {
        Log.d(LOG_TAG, "convertCall");
        mIntent.putExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
        VideoProfile.STATE_AUDIO_ONLY);
        /*synchronized (TelecomSystem.getInstance().getLock()) {
                TelecomSystem.getInstance().getCallIntentProcessor().processIntent(mIntent, "");
        }*/
    }

    private  void continueCall() {
        Log.d(LOG_TAG, "continueCall");
        /*synchronized (TelecomSystem.getInstance().getLock()) {
                TelecomSystem.getInstance().getCallIntentProcessor().processIntent(mIntent, "");
        }*/
    }

    private float batteryLevel(Context context) {
        LocalBroadcastManager.getInstance(context).registerReceiver(null,
                new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
        Intent intent  = new Intent(context, ErrorDialogActivity.class);
        int    level   = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
        int    scale   = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 100);
        int    percent = (level * 100) / scale;
        Log.d(LOG_TAG, "batteryLevel percent" + percent);
        return percent;
    }
}
