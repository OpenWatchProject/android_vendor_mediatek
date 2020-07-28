package com.mediatek.phone.ext;

/**
 * Disconnect cause to telecomm.
 */
public interface IDisconnectCauseExt {
    /**
     * Convert the telephony disconnect code into a telecom disconnect code.
     * @param telephonyDisconnectCause disconnect cause
     * @param error default error
     * @return error
     */
    int toTelecomDisconnectCauseCode(int telephonyDisconnectCause, int error);

    /**
     * Returns a label for to the disconnect cause to be shown to the user.
     * @param telephonyDisconnectCause disconnect cause
     * @return string
     */
    CharSequence toTelecomDisconnectCauseLabel(int telephonyDisconnectCause);

    /**
     * Returns a description of the disconnect cause to be shown to the user.
     * @param telephonyDisconnectCause disconnect cause
     * @return string
     */
    CharSequence toTelecomDisconnectCauseDescription(int telephonyDisconnectCause);
}
