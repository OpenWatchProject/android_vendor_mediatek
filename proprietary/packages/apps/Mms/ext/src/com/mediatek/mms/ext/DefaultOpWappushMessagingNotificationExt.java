package com.mediatek.mms.ext;

import android.app.Notification.Builder;
import android.content.Context;
import android.content.ContextWrapper;

public class DefaultOpWappushMessagingNotificationExt extends ContextWrapper
        implements IOpWappushMessagingNotificationExt {

    public DefaultOpWappushMessagingNotificationExt(Context base) {
        super(base);
    }

    @Override
    public boolean updateNotification(Context context, Builder noti) {
        return false;
    }
}
