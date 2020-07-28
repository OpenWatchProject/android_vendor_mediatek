package com.mediatek.mms.ipmessage;

import android.content.Context;
import android.content.Intent;


public interface IIpSpamMsgReceiverExt {

  /**
     * called on onReceive Spam message
     * @param context: context
     * @param intent: intent
     * @param isMmsPush: Sms: false, Push: true
     * @return boolean
     * @internal
     */
    public boolean onIpReceiveSpamMsg(Context context, Intent intent, boolean isMmsPush);

}
