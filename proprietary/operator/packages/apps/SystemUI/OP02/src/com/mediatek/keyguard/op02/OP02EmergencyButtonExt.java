package com.mediatek.keyguard.op02;

import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.keyguard.ext.DefaultEmergencyButtonExt;

/**
 * Customize emergency button.
 */
public class OP02EmergencyButtonExt extends DefaultEmergencyButtonExt {

    private static final String TAG = "OP02EmergencyButtonExt";

    @Override
    public boolean showEccByServiceState(boolean[] isServiceSupportEcc, int slotId) {

        boolean isSimReady = false;

        isSimReady = hasSimReady();

        if (!isSimReady) {
            return false;
        }

        boolean isServiceSupport = false;
        for (int i = 0; i < isServiceSupportEcc.length; i++) {
            if (isServiceSupportEcc[i]) {
                isServiceSupport = true;
                break;
            }
        }
        return isSimReady && isServiceSupport;
    }

    private boolean hasSimReady() {
        final int simNum = getNumOfSim();
        for (int i = PhoneConstants.SIM_ID_1; i < simNum; i++) {
                int simState = TelephonyManager.getDefault().getSimState(i);
                Log.i(TAG, "slotId = " + i + " simState = " + simState);
                if (TelephonyManager.SIM_STATE_READY == simState) {
                    return true;
                }
        }
        return false;
    }

    private int getNumOfSim() {
        return TelephonyManager.getDefault().getSimCount();
    }

    @Override
    public boolean showEccInNonSecureUnlock() {
        Log.d(TAG, "showEccInNonSecureUnlock return true");
        return true;
    }

    @Override
    public void setEmergencyButtonVisibility(View eccButtonView, float alpha) {
        Log.d(TAG, "setEmergencyButtonVisibility: alpha=" + alpha
                + ", eccButton=" + eccButtonView);
        if (eccButtonView != null) {
            if (alpha == 0) {
                eccButtonView.setClickable(false);
            } else {
                eccButtonView.setClickable(true);
            }
        }
    }
}
