package com.mediatek.settings.cdma;

import android.content.Context;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.SubscriptionInfo;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.settings.R;
import com.android.settings.deviceinfo.simstatus.SimStatusDialogFragment;

public class CdmaSimStatus {

    private static final String TAG = "CdmaSimStatus";

    private static final int MCC_LENGTH = 3;

    private SimStatusDialogFragment mFragment;
    private ServiceState mServiceState;
    private TelephonyManager mTelephonyManager;
    private SubscriptionInfo mSubInfo;

    public CdmaSimStatus(SimStatusDialogFragment fragment, SubscriptionInfo subInfo) {
        mFragment = fragment;
        mSubInfo = subInfo;
        mTelephonyManager = (TelephonyManager) fragment.getContext()
                .getSystemService(Context.TELEPHONY_SERVICE);
    }

    public void setServiceState(ServiceState state) {
        Log.d(TAG, "setServiceState, serviceState=" + state);
        mServiceState = state;
    }

    public void updateSignalStrength(SignalStrength signal, int viewId, String viewValue) {
        Log.d(TAG, "updateSignalStrength, signalStrength=" + signal);
        if (CdmaUtils.isSupportCdma(mSubInfo.getSubscriptionId())) {
            if (!signal.isGsm() && isRegisterUnderLteNetwork()) {
                // For ALPS02050467, when network is under LTE,
                // and cdma network is 1x (CS),
                // need to re-get signal strength of cdma (CS)
                String cdmaSignal = getCdmaSignalStrength(signal, viewValue);

                int lteSignalDbm = signal.getLteDbm();
                int lteSignalAsu = signal.getLteAsuLevel();

                if (lteSignalDbm == -1) {
                    lteSignalDbm = 0;
                }

                if (lteSignalAsu == -1) {
                    lteSignalAsu = 0;
                }

                Log.d(TAG, "updateSignalStrength, lteSignalDbm=" + lteSignalDbm
                        + ", lteSignalAsu=" + lteSignalAsu);
                String lteSignal = mFragment.getString(
                        R.string.sim_signal_strength, lteSignalDbm, lteSignalAsu);
                Log.d(TAG, "updateSignalStrength, cdmaSignal=" + cdmaSignal
                        + ", lteSignal=" + lteSignal);
                String summary = mFragment.getString(
                        R.string.status_cdma_signal_strength, cdmaSignal, lteSignal);
                Log.d(TAG, "updateSignalStrength, summary=" + summary);
                setSummaryText(viewId, summary);
            }
        }
    }

    private String getCdmaSignalStrength(SignalStrength signalStrength, String viewValue) {
        ServiceState serviceState = getServiceState();
        Log.d(TAG, "setCdmaSignalStrength, serviceState=" + serviceState);

        if (serviceState != null
                && serviceState.getVoiceNetworkType() == TelephonyManager.NETWORK_TYPE_1xRTT) {
            int signalDbm = signalStrength.getCdmaDbm();
            int signalAsu = signalStrength.getCdmaAsuLevel();

            if (signalDbm == -1) {
                signalDbm = 0;
            }

            if (signalAsu == -1) {
                signalAsu = 0;
            }

            Log.d(TAG, "setCdmaSignalStrength, 1xRTT signalDbm=" + signalDbm
                    + ", signalAsu=" + signalAsu);
            return mFragment.getString(R.string.sim_signal_strength,
                    signalDbm, signalAsu);
        }

        return viewValue;
    }

    private ServiceState getServiceState() {
        return mServiceState;
    }

    private boolean isRegisterUnderLteNetwork() {
        ServiceState serviceState = getServiceState();
        Log.d(TAG, "isRegisterUnderLteNetwork, serviceState=" + serviceState);
        boolean isLteNetwork = false;

        if (serviceState != null
                && serviceState.getDataNetworkType() == TelephonyManager.NETWORK_TYPE_LTE
                && serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE) {
            isLteNetwork = true;
        }

        Log.d(TAG, "isRegisterUnderLteNetwork, lteNetwork=" + isLteNetwork);
        return isLteNetwork;
    }

    private void setSummaryText(int viewId, String text) {
        mFragment.setText(viewId, text);
    }
}
