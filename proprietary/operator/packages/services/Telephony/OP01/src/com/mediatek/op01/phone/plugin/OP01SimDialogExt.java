package com.mediatek.op01.phone.plugin;
import java.util.List;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.phone.ext.DefaultSimDialogExt;
import com.mediatek.phone.ext.ISimDialogExt;

public class OP01SimDialogExt extends DefaultSimDialogExt {
    private static final String TAG = "OP01SimDialogExt";
    private Context mContext;
    private static final String KEY_CELLULAR_DATA = "sim_cellular_data";
    private static final String KEY_CALLS = "sim_calls";
    private static final String KEY_SMS = "sim_sms";
    public static final int DATA_PICK = 0;
    public static final int CALLS_PICK = 1;
    public static final int SMS_PICK = 2;
    private static final String SLOT_PREFIX = "sim_slot_";
    private static final int SLOT_EMPTY = -1;
    private SharedPreferences mPreference;
    private static final String DATA_SIM = "data_sim";
    private static final int INVALID_SLOT = -2;

    public OP01SimDialogExt(Context context) {
        super();
        mContext = context;
    }

    @Override
    public void customBroadcast(Intent intent) {
        Log.d(TAG, "customBroadcast");
        mPreference = mContext.createDeviceProtectedStorageContext()
                 .getSharedPreferences(DATA_SIM, Context.MODE_PRIVATE);
        boolean isAirplaneModeBroadcast = (null != intent)
                 && (null != intent.getAction())
                 && (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(intent.getAction()));
        boolean isSimStateChanged = (null != intent)
                 && (null != intent.getAction())
                 && (TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED.equals(intent.getAction()));
        if (isSimStateChanged || isAirplaneModeBroadcast) {
            List<SubscriptionInfo> subs = SubscriptionManager.from(mContext)
                     .getActiveSubscriptionInfoList();
             final int detectedType = intent.getIntExtra(
                     MtkSubscriptionManager.INTENT_KEY_DETECT_STATUS, 0);
             Log.d(TAG, "sub info update, type = " + detectedType + ", subs = " + subs);
             /// M: Ignore this broadcast when the SIM count is changed again. @{
             final int numSims = intent.getIntExtra(
                     MtkSubscriptionManager.INTENT_KEY_SIM_COUNT, (subs == null ? 0 : subs.size()));
             if (subs != null && numSims != subs.size()) {
                 Log.d(TAG, "SIM count is changed again, extraSimCount=" + numSims
                         + ", currentSimCount=" + subs.size());
                 return;
             }
             /// @}
            boolean airplaneOn = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.AIRPLANE_MODE_ON, 0) == 1;
            Log.d(TAG, "isSimDialogNeeded isSimStateChanged airplaneOn: " + airplaneOn);
            if (airplaneOn) {
                Log.i(TAG, "Return do nothing - airplaneOn true");
                return;
            }
            /// if sim number<2, return.
            int numSlots = TelephonyManager.getDefault().getSimCount();
            if (numSlots < 2) {
                Log.i(TAG, "Return do nothing -  simnumber<2");
                return;
            }
            ///start or not by OP01 logic.
            boolean notificationSent = false;
            int numSIMsDetected = 0;
            for (int i = 0; i < numSlots; i++) {
                final SubscriptionInfo sir = findRecordBySlotId(i);
                Log.d(TAG, "sir = " + sir);
                final String key = SLOT_PREFIX + i;
                final int lastSubId = getLastSubId(key);
                if (sir != null) {
                    numSIMsDetected++;
                    final int currentSubId = sir.getSubscriptionId();
                    if (lastSubId == INVALID_SLOT) {
                        setLastSubId(key, currentSubId);
                        notificationSent = true;
                    } else if (lastSubId != currentSubId) {
                        setLastSubId(key, currentSubId);
                        notificationSent = true;
                    }
                    Log.d(TAG, "customBroadcast return key = " + key
                            + " lastSubId = " + lastSubId
                            + " currentSubId = " + currentSubId);
                } else if (lastSubId != SLOT_EMPTY) {
                    setLastSubId(key, SLOT_EMPTY);
                    notificationSent = false;
                }
            }
            Log.d(TAG, "notificationSent = " + notificationSent
                    + " numSIMsDetected = " + numSIMsDetected);

            if ((!notificationSent) || numSIMsDetected < 2) {
                Log.i(TAG, "OP01 customBroadcast return simactivity<2"
                        +" or notificationSent =false");
                return;
            }
            Log.d(TAG, "Is dialog show? sIsShow = " + OP01SimDialogService.sIsShow);
            if (OP01SimDialogService.sIsShow) {
                Log.i(TAG, "OP01 customBroadcast return"
                        + " OP01DataPickService.sIsShow = true<2");
                return;
            }
            int defaultDataSubId  = SubscriptionManager.getDefaultDataSubscriptionId();
            Log.i(TAG, "defaultDataSubId =  " + defaultDataSubId);
            for (SubscriptionInfo sub : subs) {
                int activitysubId = sub.getSubscriptionId();
                Log.i(TAG, "activitysubId =  " + activitysubId);
                if (activitysubId == defaultDataSubId) {
                    return;
                }
            }
            Intent startDataPicker = new Intent(mContext, OP01SimDialogService.class);
            mContext.startService(startDataPicker);
            Log.i(TAG, "OP01 customBroadcast started OP01SimDialogService");
        }
    }
    private int getLastSubId(String strSlotId) {
        return mPreference.getInt(strSlotId, INVALID_SLOT);
    }
    private SubscriptionInfo findRecordBySlotId(final int slotId) {
        final List<SubscriptionInfo> subInfoList =
                SubscriptionManager.from(mContext).getActiveSubscriptionInfoList();
        if (subInfoList != null) {
            final int subInfoLength = subInfoList.size();

            for (int i = 0; i < subInfoLength; ++i) {
                final SubscriptionInfo sir = subInfoList.get(i);
                if (sir.getSimSlotIndex() == slotId) {
                    //Right now we take the first subscription on a SIM.
                    return sir;
                }
            }
        }
        return null;
    }

    private void setLastSubId(String strSlotId, int value) {
        Editor editor = mPreference.edit();
        editor.putInt(strSlotId, value);
        editor.commit();
    }
}