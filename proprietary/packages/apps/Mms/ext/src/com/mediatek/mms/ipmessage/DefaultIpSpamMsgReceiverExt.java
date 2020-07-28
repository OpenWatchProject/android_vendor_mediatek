package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.content.Intent;


public class DefaultIpSpamMsgReceiverExt implements IIpSpamMsgReceiverExt {

    public boolean onIpReceiveSpamMsg(Context context, Intent intent, boolean isMmsPush) {
        return false;
    }

}
