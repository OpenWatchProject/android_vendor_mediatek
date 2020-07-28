package com.mediatek.mms.ext;

import android.app.PendingIntent;
import android.content.Context;

import java.util.ArrayList;

public interface IOpSmsSingleRecipientSenderExt {

    /**
     * @internal
     */
    public boolean sendMessage(Context context, int subId, String dest, String serviceCenter,
            ArrayList<String> messages, ArrayList<PendingIntent> sentIntents,
            ArrayList<PendingIntent> deliveryIntents);

    /**
     * @internal
     */
    public boolean sendOpMessage(Context context, int subId,
            ArrayList<PendingIntent> deliveryIntents);

    /**
     * @internal
     */
    public int sendMessagePrepare(Context context, int codingType);
}
