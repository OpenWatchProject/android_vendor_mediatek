package com.mediatek.mms.ipmessage;

public interface IIpRecipientsEditorExt {
    /**
     * isValidAddress
     * @param return true if is a valid address, add for "MYPC"
     * @internal
     */
    boolean isValidAddress(boolean commonValidValue, String number);
}
