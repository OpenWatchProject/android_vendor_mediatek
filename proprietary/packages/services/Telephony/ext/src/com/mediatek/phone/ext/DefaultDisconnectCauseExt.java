package com.mediatek.phone.ext;

/**
 * Disconnect cause to telecomm.
 */
public class DefaultDisconnectCauseExt implements IDisconnectCauseExt {

    @Override
    public int toTelecomDisconnectCauseCode(int telephonyDisconnectCause, int error) {
        return error;
    }

    @Override
    public CharSequence toTelecomDisconnectCauseLabel(int telephonyDisconnectCause) {
        return "";
    }

    @Override
    public CharSequence toTelecomDisconnectCauseDescription(int telephonyDisconnectCause) {
        return "";
    }

}
