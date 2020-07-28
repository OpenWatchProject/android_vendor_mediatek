package com.mediatek.mms.ext;

import android.app.Notification;
import android.content.Context;

public interface IOpWappushMessagingNotificationExt {
    /**
     * @internal
     */
    boolean updateNotification(Context context, Notification.Builder noti);
}
