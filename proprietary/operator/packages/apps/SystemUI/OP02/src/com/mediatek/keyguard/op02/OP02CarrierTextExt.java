package com.mediatek.keyguard.op02;

import android.content.Context;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.keyguard.ext.DefaultCarrierTextExt;

/**
 * Customize carrier text.
 */
public class OP02CarrierTextExt extends DefaultCarrierTextExt {
    public static final String TAG = "OP02CarrierTextExt";
    private static final boolean DEBUG = !isUserLoad();

    @Override
    public CharSequence customizeCarrierText(CharSequence carrierText, CharSequence simMessage,
            int simId) {
        final int[] subIds = SubscriptionManager.getSubId(simId);
        if (DEBUG) {
            Log.i(TAG, "customizeCarrierText, slotId = " + simId + ",subIds = " + subIds);
        }
        if (subIds != null && subIds.length != 0) {
            int subId = subIds[0];
            ServiceState ss = TelephonyManager.getDefault().getServiceStateForSubscriber(subId);
            if (ss != null) {
                if (DEBUG) {
                    Log.i(TAG, "customizeCarrierText, slotId = " + simId + ",subId = " + subId
                            + "ss.isEmergencyOnly()=" + ss.isEmergencyOnly());
                }
                if (ss.isEmergencyOnly()) {
                    return simMessage;
                }
            }
        }
        return super.customizeCarrierText(carrierText, simMessage, simId);
    }

    @Override
    public boolean showCarrierTextWhenSimMissing(boolean isSimMissing, int simId) {
        ///init sim card info
        int simcardNumber = getSimNumber();
        if (DEBUG) {
            Log.i(TAG, "showCarrierTextWhenSimMissing, simcardNumber = " + simcardNumber);
        }
        if (simcardNumber == 0 || simcardNumber == 1) {
            return isSimMissing;
        } else if (simcardNumber == 2) {
            if (hasPinPukLock()) {
                return isSimMissing;
            }

            boolean oneCardOutOfService = isOneCardOutOfService();
            if (DEBUG) {
                Log.i(TAG, "oneCardOutOfService = " + oneCardOutOfService);
            }
            if (!oneCardOutOfService) {
                return isSimMissing;
            } else {
                // / M: when one sim state is not in service && !
                // emerycallonly,sim1 must show ,so reture false
                if (DEBUG) {
                    Log.i(TAG, "simId = " + simId + " return = "
                            + (PhoneConstants.SIM_ID_1 != simId));
                }
                return PhoneConstants.SIM_ID_1 != simId;
            }
        }
        return isSimMissing;
    }

    /**
     * The customized divider of carrier text.
     *
     * @param divider the current carrier text divider string.
     *
     * @return the customized carrier text divider string.
     */
    @Override
    public String customizeCarrierTextDivider(String divider) {
        String carrierDivider = " | ";
        return carrierDivider;
    }

    /**
     * Get the valid sim count.
     * @return the valid sim count.
     */
    private int getSimNumber() {
        if (DEBUG) {
            Log.i(TAG, "getSimNumber() start ");
        }
        int simNumber = 0;
        final int simNum = getNumOfSim();
        if (DEBUG) {
            Log.i(TAG, "getSimNumber() simNum = " + simNum);
        }
        for (int i = PhoneConstants.SIM_ID_1; i < simNum; i++) {
            final int[] subIds = SubscriptionManager.getSubId(i);
            if (DEBUG) {
                Log.i(TAG, "getSimNumber() slotId = " + i + ",subIds = " + subIds);
            }
            if (subIds != null && subIds.length != 0) {
                int subId = subIds[0];
                int simState = TelephonyManager.getDefault().getSimState(i);
                boolean bSimInserted = TelephonyManager.getDefault().hasIccCard(i);
                if (DEBUG) {
                    Log.i(TAG, "getSimNumber() slotId = " + i + " subId = " + subId +
                            "bSimInserted = " + bSimInserted + " simState=" + simState);
                }

                boolean processed = false;
                if (bSimInserted
                        && ((TelephonyManager.SIM_STATE_PIN_REQUIRED == simState
                                || TelephonyManager.SIM_STATE_PUK_REQUIRED == simState
                                || TelephonyManager.SIM_STATE_READY == simState
                                || TelephonyManager.SIM_STATE_NETWORK_LOCKED == simState))) {
                    simNumber++;
                    processed = true;
                    continue;
                }

                ServiceState ss = TelephonyManager.getDefault().getServiceStateForSubscriber(subId);
                if (ss != null) {
                    if (DEBUG) {
                        Log.i(TAG,
                                "getSimNumber() slotId = " + i + " subId = " + subId
                                        + "bSimInserted = " + bSimInserted
                                        + " ss.isEmergencyOnly() = " + ss.isEmergencyOnly()
                                        + " ss.getState() = " + ss.getState());
                    }

                    if (!processed && bSimInserted &&
                            ss.getState() == ServiceState.STATE_POWER_OFF) {
                        simNumber++;
                    }
                }
            }
        }

        if (DEBUG) {
            Log.i(TAG, "getSimNumber() end: simNumber = " + simNumber);
        }
        return simNumber;
    }

    private boolean hasPinPukLock() {
        final int simNum = getNumOfSim();
        for (int i = PhoneConstants.SIM_ID_1; i < simNum; i++) {
            int simState = TelephonyManager.getDefault().getSimState(i);
            Log.i(TAG, "slot[" + i + "] simState = " + simState);
            if (simState == TelephonyManager.SIM_STATE_PIN_REQUIRED
                    || simState == TelephonyManager.SIM_STATE_PUK_REQUIRED) {
                Log.i(TAG, "return hasPinPukLock slotId = " + i + "simstate = PIN or PUK");
                return true;
            }
        }
        return false;
    }

    private static int getNumOfSim() {
        return TelephonyManager.getDefault().getSimCount();
    }

    private boolean isOneCardOutOfService() {
        final int simNum = getNumOfSim();
        for (int i = PhoneConstants.SIM_ID_1; i < simNum; i++) {
            final int[] subIds = SubscriptionManager.getSubId(i);
            if (DEBUG) {
                Log.i(TAG, "isOneCardOutOfService, slotId = " + i + ",subIds = " + subIds);
            }
            if (subIds != null && subIds.length != 0) {
                int subId = subIds[0];
                ServiceState ss = TelephonyManager.getDefault().getServiceStateForSubscriber(subId);
                if (ss != null) {
                    if (DEBUG) {
                        Log.i(TAG, "isOneCardOutOfService, slotId = " + i + ",subId = " + subId
                                + " sim has service: " + hasService(ss));
                    }
                    if (!hasService(ss)) {
                        return true;
                    }
                } else {
                    return true;
                }
            }
        }
        return false;
    }

    public static final boolean isUserLoad() {
        return SystemProperties.get("ro.build.type").equals("user") ||
                    SystemProperties.get("ro.build.type").equals("userdebug");
    }

    private boolean hasService(ServiceState serviceState) {
        if (serviceState != null) {
            // Consider the device to be in service if either voice or data
            // service is available. Some SIM cards are marketed as data-only
            // and do not support voice service, and on these SIM cards, we
            // want to show signal bars for data service as well as the "no
            // service" or "emergency calls only" text that indicates that voice
            // is not available.
            switch (serviceState.getVoiceRegState()) {
                case ServiceState.STATE_POWER_OFF:
                    return false;
                case ServiceState.STATE_OUT_OF_SERVICE:
                case ServiceState.STATE_EMERGENCY_ONLY:
                    return serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE;
                default:
                    return true;
            }
        } else {
            return false;
        }
    }
}
