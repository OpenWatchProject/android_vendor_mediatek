package com.richinfo.dm;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.IccCardConstants;

import cn.richinfo.dm.CtmApplication;
import com.dmyk.android.telephony.DmykAbsTelephonyManager;
import com.dmyk.android.telephony.DmykTelephonyManager;
import com.dmyk.android.telephony.DmykTelephonyManager.MLog;

public class CtmReceiver extends BroadcastReceiver {
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        MLog.d("CtmReceiver receives " + action);
        if (action != null && "android.intent.action.SIM_STATE_CHANGED".equals(action)) {
            String simStatus = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
            MLog.d("SIM_STATE_CHANGED: simStatus=" + simStatus + ", phoneId=" + phoneId);
            if (phoneId != -1) {
                // CtmApplication instance should be created after Application is done
                CtmApplication ca = CtmApplication.getInstance(null);
                if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(simStatus)) {
                    int slotFlag = ca.getSlotFlag();
                    slotFlag |= 1 << phoneId;
                    ca.setSlotFlag(slotFlag);
                    MLog.d("sim state loaded, slotFlag=" + slotFlag);
                    if (0x03 == slotFlag) {
                        // both slot are determined, send the last state
                        Intent newIntent =
                            new Intent(DmykAbsTelephonyManager.ACTION_SIM_STATE_CHANGED);
                        newIntent.putExtra(DmykAbsTelephonyManager.EXTRA_SIM_PHONEID, phoneId);
                        newIntent.putExtra(DmykAbsTelephonyManager.EXTRA_SIM_STATE,
                                           DmykAbsTelephonyManager.SIM_STATE_READY);
                         newIntent.setFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
                         context.sendBroadcast(newIntent);
                         MLog.d("Send SIM_STATE_CHANGED");
                    }
                    ca.syncVolteStatus();
                    ca.updateAPNObservers();
                } else if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simStatus)) {
                    int slotFlag = ca.getSlotFlag();
                    slotFlag |= 1 << phoneId;
                    ca.setSlotFlag(slotFlag);
                    MLog.d("sim state absent, slotFlog=" + slotFlag);
                    if (0x03 == slotFlag) {
                        // both slot are determined, send the last state
                        Intent newIntent =
                            new Intent(DmykAbsTelephonyManager.ACTION_SIM_STATE_CHANGED);
                        newIntent.putExtra(DmykAbsTelephonyManager.EXTRA_SIM_PHONEID, phoneId);
                        newIntent.putExtra(DmykAbsTelephonyManager.EXTRA_SIM_STATE,
                                           DmykAbsTelephonyManager.SIM_STATE_ABSENT);
                        newIntent.setFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
                        context.sendBroadcast(newIntent);
                        MLog.d("Send SIM_STATE_CHANGED");
                    }
                    ca.updateAPNObservers();
                }
            }
        } else if (DmykAbsTelephonyManager.ACTION_VOLTE_STATE_SETTING.equals(action)) {
            int slotId = intent.getIntExtra(DmykAbsTelephonyManager.EXTRA_SIM_PHONEID, -1);
            Intent newIntent = new Intent(DmykAbsTelephonyManager.ACTION_VOLTE_STATE_SETTING);
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            newIntent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
            context.startActivity(newIntent);
        }
    }
}

