package com.mediatek.phone.ext;


public interface ITtyModeListPreferenceExt {

    /**
     * For Show popup when WFC is enabled.
     *
     * @param buttonTtyMode current TTY mode
     */
    void handleWfcUpdateAndShowMessage(int buttonTtyMode);
}
