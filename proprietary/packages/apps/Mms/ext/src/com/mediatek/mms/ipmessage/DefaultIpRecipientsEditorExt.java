package com.mediatek.mms.ipmessage;

public class DefaultIpRecipientsEditorExt implements IIpRecipientsEditorExt {

    @Override
    public boolean isValidAddress(boolean commonValidValue, String number) {
        return false;
    }
}
