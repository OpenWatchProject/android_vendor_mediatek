package com.mediatek.apmonitor.kpi;

import android.telephony.ServiceState;

public class NetworkServiceState extends KpiObjBaseRoot {
    private static final int VERSION = 1;

    public static class ApmServiceState {
        public static final int STATE_EMERGENCY_ONLY = 0;
        public static final int STATE_IN_SERVICE = 1;
        public static final int STATE_OUT_OF_SERVICE = 2;
        public static final int STATE_RADIO_OFF = 3;
        public static final int STATE_UNKNOWN = 4;

        public static Integer mapServiceState(int state) {
            Integer apmState;
            switch(state) {
                case ServiceState.STATE_EMERGENCY_ONLY:
                    apmState = ApmServiceState.STATE_EMERGENCY_ONLY;
                    break;
                case ServiceState.STATE_IN_SERVICE:
                    apmState = ApmServiceState.STATE_IN_SERVICE;
                    break;
                case ServiceState.STATE_OUT_OF_SERVICE:
                    apmState = ApmServiceState.STATE_OUT_OF_SERVICE;
                    break;
                case ServiceState.STATE_POWER_OFF:
                    apmState = ApmServiceState.STATE_RADIO_OFF;
                    break;
                default:
                    apmState = ApmServiceState.STATE_UNKNOWN;
                    break;
            }
            return apmState;
        }
    }

    // Fields
    public Integer mApmServiceState; // Reference to ServiceState
    public boolean mIsRoaming;
    public String mOperatorNumeric;
    public String mOperatorAlpha;

    public NetworkServiceState(
            int state, boolean isRoaming, String operatorNumeric, String operatorAlpha) {
        // Must call super constructor and assgin version.
        super(VERSION);
        mApmServiceState = ApmServiceState.mapServiceState(state);
        mIsRoaming = isRoaming;
        mOperatorNumeric = operatorNumeric;
        mOperatorAlpha = operatorAlpha;
    }

    // Implement to flat KPI fields to byte stream.
    @Override
    public void serialize() {
        writeInteger(mApmServiceState);
        writeBoolean(mIsRoaming);

        boolean validOperatorNumeric = (mOperatorNumeric != null && mOperatorNumeric.length() > 0);
        if (validOperatorNumeric) {
            writeInteger(mOperatorNumeric.length() + 1);
        } else {
            writeInteger(0);
        }
        boolean validOperatorAlpha = (mOperatorAlpha != null && mOperatorAlpha.length() > 0);
        if (validOperatorAlpha) {
            writeInteger(mOperatorAlpha.length() + 1);
        } else {
            writeInteger(0);
        }

        if (validOperatorNumeric) {
            writeString(mOperatorNumeric);
        }
        if (validOperatorAlpha) {
            writeString(mOperatorAlpha);
        }
    }

    // No need to implement this currently.
    @Override
    public void deserialize() {
        // Not implemented
    }
}
