package com.mediatek.phone.ext;

import android.util.Log;


public class DefaultTtyModeListPreferenceExt implements ITtyModeListPreferenceExt {

    /**
     * For Show popup when WFC is enabled.
     *
     * @param buttonTtyMode current TTY mode
     */
    @Override
     public void handleWfcUpdateAndShowMessage(int buttonTtyMode) {
        Log.d("DefaultTtyModeListPreferenceExt", "handleWfcUpdateAndShowMessage");
     }
}
