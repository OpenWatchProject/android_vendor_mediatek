package com.mediatek.presence.service;

import com.gsma.services.rcs.Intents;

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.CoreException;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.support.v4.content.LocalBroadcastManager;
import android.preference.PreferenceManager;

import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;

/**
 * Device boot event receiver: automatically starts the RCS service
 * 
 * @author jexa7410
 */
public class DataSimChanged extends BroadcastReceiver {
    private static Logger logger = Logger.getLogger(DataSimChanged.class.getSimpleName());
    public static String IS_SIM_PLUG_OUT = "isSimPlugOut";
    public static String IS_SIM_PLUG_IN = "isSimPlugIn";
    
    @Override
    public void onReceive(Context context, Intent intent) {
        if (logger.isActivated()) {
            logger.debug("Data Sim Changed:" + intent.getAction());
        }

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(
                intent.getAction())) {
            String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
            logger.debug("Data Sim Changed sim state:" + simState + " slot = " + slotId);
            if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_ABSENT)) {
                Boolean isSimPlugInBefore = prefs.getBoolean(IS_SIM_PLUG_IN + slotId, false);
                if (isSimPlugInBefore) {
                    prefs.edit().putBoolean(IS_SIM_PLUG_OUT + slotId, true).apply();
                    stopImsModule(slotId);
                }
                prefs.edit().putBoolean(IS_SIM_PLUG_IN + slotId, false).apply();
            } else if (simState.equals(IccCardConstants.INTENT_VALUE_ICC_LOADED)) {
                Boolean isSimPlugOutBefore = prefs.getBoolean(IS_SIM_PLUG_OUT + slotId, false);
                if (isSimPlugOutBefore) {
                    startImsModule(slotId);
                }
                prefs.edit().putBoolean(IS_SIM_PLUG_OUT + slotId, false).apply();
                prefs.edit().putBoolean(IS_SIM_PLUG_IN + slotId, true).apply();
            }
        }

    }

    private void startImsModule(int slotId) {
        if (Core.getInstance() == null) {
            return;
        }
        try {
            Core.getInstance().initImsModule(slotId);
        } catch (CoreException e) {
            logger.error("initImsModule Fail" + e);
        }
        Core.getInstance().startImsModule(slotId);

    }

    private void stopImsModule(int slotId) {
        if (Core.getInstance() == null) {
            return;
        }
        Core.getInstance().stopImsModule(slotId);
    }
    
}
