package com.mediatek.apmonitor.monitor;

import com.mediatek.apmonitor.ApmMsgDefs;
import com.mediatek.apmonitor.IApmKpiMonitor;
import com.mediatek.apmonitor.KpiMonitor;
import com.mediatek.apmonitor.KpiMonitor.*;
import com.mediatek.apmonitor.kpi.NetworkServiceState;

import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

public class NetworkServiceStateKpiMonitor extends KpiMonitor {
    private static final String TAG = "NetworkServiceStateKpiMonitor";
    private static final Short APM_MSG_NETWORK_SERVICE_STATE = ApmMsgDefs.APM_MSG_NETWORK_SERVICE_STATE;
    private static final int MAX_SUPPORT_SIM_SLOT = 2;
    private static final int SIM_SLOT_SIM1 = 0;
    private static final int SIM_SLOT_SIM2 = 1;

    public NetworkServiceStateKpiMonitor(Context context, Handler handler) {
        super(context, handler, TAG);
        Log.d(TAG, "NetworkServiceStateKpiMonitor Ctor");
    }

    @Override
    public boolean handleMessage(Message msg) {
        if (msg.what == MSG_ID_APM_KPI_STATE_CHANGED) {
            Short msgId = (Short)msg.obj;
            if (msgId != APM_MSG_NETWORK_SERVICE_STATE) {
                return false;
            }
            TelephonyManager tm =
                    (TelephonyManager)getContext().getSystemService(Context.TELEPHONY_SERVICE);
            if (tm == null) {
                Log.e(TAG, "Null TelephonyManager with KPI state changed!");
                return false;
            }
            if (msg.arg1 == EVENT_KPI_ADDED) {
                // To submit when first registration
                Log.d(TAG, "Subscribe APM_MSG_NETWORK_SERVICE_STATE");
                registAllServiceState(tm, true);
                return true;
            } else if (msg.arg1 == EVENT_KPI_REMOVED) {
                Log.d(TAG, "Unsubscribe APM_MSG_NETWORK_SERVICE_STATE");
                registAllServiceState(tm, false);
                return true;
            }
        }

        return false;
    }

    private PhoneStateListener mPhoneStateListenerSim1 = new PhoneStateListener() {
        @Override
        public void onServiceStateChanged(ServiceState state) {
            Log.d(TAG, "onServiceStateChanged(SIM 1), Submit ServiceState");
            submitServiceState(state, SIM_SLOT_SIM1);
        }
    };

    private PhoneStateListener mPhoneStateListenerSim2 = new PhoneStateListener() {
        @Override
        public void onServiceStateChanged(ServiceState state) {
            Log.d(TAG, "onServiceStateChanged(SIM 2), Submit ServiceState");
            submitServiceState(state, SIM_SLOT_SIM2);
        }
    };

    private void registAllServiceState(TelephonyManager tm, boolean subscribe) {
        int slotId = 0;
        for (slotId = 0; slotId < MAX_SUPPORT_SIM_SLOT; slotId++) {
            int subId = getSubId(slotId);
            TelephonyManager tmSubId = tm.createForSubscriptionId(subId);
            if (tmSubId != null) {
                PhoneStateListener listener = null;

                if (slotId == SIM_SLOT_SIM1) {
                    listener = mPhoneStateListenerSim1;
                } else if (slotId == SIM_SLOT_SIM2) {
                    listener = mPhoneStateListenerSim2;
                } else {
                    Log.e(TAG, "Invalid slotId = " + slotId);
                    return;
                }

                if (subscribe) {
                    tmSubId.listen(listener, PhoneStateListener.LISTEN_SERVICE_STATE);
                } else {
                    tmSubId.listen(listener, PhoneStateListener.LISTEN_NONE);
                }
            } else {
                Log.e(TAG, "Failed to create TelephonyManager for subId = " + subId);
            }
        }

    }

    private boolean submitServiceState(ServiceState srvState, int simId) {
        boolean handled = false;

        if (srvState != null) {
            int state = srvState.getState();
            Log.d(TAG, "[" + simId + "]submitServiceState(), srvState = " + srvState);
            if (shouldHandleServiceState(state)) {
                NetworkServiceState kpi = new NetworkServiceState(
                        state,
                        srvState.getRoaming(),
                        srvState.getOperatorNumeric(),
                        srvState.getOperatorAlphaLong()
                );

                // Build the kpi and submit
                byte[] payload = kpi.build();
                int len = kpi.currentWriteBytes();
                long timestampMs = System.currentTimeMillis();

                boolean ret = apmSubmitKpiST(APM_MSG_NETWORK_SERVICE_STATE, simId, timestampMs, len, payload);
                Log.d(TAG, "submit APM_MSG_NETWORK_SERVICE_STATE, ret = " + ret);
                kpi.close();
                handled = true;
            } else {
                Log.d(TAG, "Skip submitServiceState()");
            }
        } else {
            Log.e(TAG, "submitServiceState(" + simId + ") failed with null srvState!");
        }
        return handled;
    }

    private boolean shouldHandleServiceState(int state) {
        switch(state) {
            case ServiceState.STATE_EMERGENCY_ONLY:
            case ServiceState.STATE_IN_SERVICE:
            case ServiceState.STATE_OUT_OF_SERVICE:
            case ServiceState.STATE_POWER_OFF:
                return true;
            default:
                // Skip update
        }
        return false;
    }

    private int getSubId(int simSlot) {
        SubscriptionManager sm = (SubscriptionManager)
                getContext().getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
        if (sm != null) {
            int[] subIds = sm.getSubscriptionIds(simSlot);
            if (subIds != null && subIds.length > 0) {
                return subIds[0];
            } else {
                return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
            }
        }
        return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
    }
}
