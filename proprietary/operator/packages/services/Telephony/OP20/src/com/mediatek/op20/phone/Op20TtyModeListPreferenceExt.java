package com.mediatek.op20.phone;

import android.content.Context;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionManager;
import android.util.Log;
import android.widget.Toast;

import com.android.ims.ImsManager;
import com.mediatek.phone.ext.DefaultTtyModeListPreferenceExt;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.op20.phone.R;



public class Op20TtyModeListPreferenceExt extends DefaultTtyModeListPreferenceExt {
    private static final String TAG = "Op20TtyModeListPreferenceExt";
    private static final String WIFICALL_SETTINGS_BY_TTY = "wificall_settings_by_tty";

    private Context mContext;

    public Op20TtyModeListPreferenceExt(Context context) {
        mContext = context;
    }


    /**
     * For OP20 (Sprint) Show popup when WFC is enabled.
     *
     * @param buttonTtyMode current TTY mode
     */
    @Override
    public void handleWfcUpdateAndShowMessage(int buttonTtyMode) {
        Log.d(TAG, "handleWfcUpdateAndShowMessage");
        updateWfcAndShowMessage(buttonTtyMode);
    }


    private int getMainPhoneId() {
        int mainPhoneId = SubscriptionManager.INVALID_PHONE_INDEX;
        IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (iTelEx != null) {
            try {
                mainPhoneId = iTelEx.getMainCapabilityPhoneId();
            } catch (RemoteException e) {
                Log.d(TAG, "getMainCapabilityPhoneId: remote exception");
            }
        } else {
            Log.d(TAG, "IMtkTelephonyEx service not ready!");
            mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        }
        return mainPhoneId;
    }

    /**
     * Disable/Enable WFC based on TTY mode and show dialog about WFC setting
     * @param newPreferredTtyMode current tty mode set
     */
    private void updateWfcAndShowMessage(int newPreferredTtyMode) {
        int mainPhoneId = getMainPhoneId();
        String disableWifiCallingMessage = mContext.getString(R.string.tty_wfc_disable_message);
        String mode = "TTY prefered mode change to  " +
                getTtyModeString(newPreferredTtyMode) + "\n" + disableWifiCallingMessage;
        boolean isWifiCallingChangedByTty = (Settings.System.getInt(
                            mContext.getContentResolver(), WIFICALL_SETTINGS_BY_TTY, 0) == 1);
        ImsManager imsManager = ImsManager.getInstance(mContext, mainPhoneId);
        if (newPreferredTtyMode != TelecomManager.TTY_MODE_OFF &&
                imsManager.isWfcEnabledByUser()) {
            imsManager.setWfcSetting(false);
            Settings.System.putInt(mContext.getContentResolver(), WIFICALL_SETTINGS_BY_TTY, 1);
            Toast.makeText(mContext, mode, Toast.LENGTH_LONG).show();
        } else {
            if (newPreferredTtyMode == TelecomManager.TTY_MODE_OFF && isWifiCallingChangedByTty) {
                imsManager.setWfcSetting(true);
                Settings.System.putInt(mContext.getContentResolver(), WIFICALL_SETTINGS_BY_TTY, 0);
            }
        }
    }

    /**
     * Get Tty mode in String format.
     * @param mode tty mode
     * @return String output for tty
     */
    private String getTtyModeString(int mode) {
        String [] txts = mContext.getResources().getStringArray(R.array.tty_mode_entries);
        switch (mode) {
            case TelecomManager.TTY_MODE_OFF:
            case TelecomManager.TTY_MODE_HCO:
            case TelecomManager.TTY_MODE_VCO:
            case TelecomManager.TTY_MODE_FULL:
                return (txts[mode]);
            default:
                return (txts[TelecomManager.TTY_MODE_OFF]);
        }
    }

}
