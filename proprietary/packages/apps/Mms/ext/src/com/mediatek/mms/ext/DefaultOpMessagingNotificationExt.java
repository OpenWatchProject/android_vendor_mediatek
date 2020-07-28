package com.mediatek.mms.ext;

import android.app.Notification.Builder;
import android.content.Context;
import android.content.ContextWrapper;
import android.net.Uri;

public class DefaultOpMessagingNotificationExt extends ContextWrapper implements
        IOpMessagingNotificationExt {

    public DefaultOpMessagingNotificationExt(Context base) {
        super(base);
    }

    @Override
    public Uri getUndeliveredMessageCount(Uri uri) {
        return uri;
    }

    @Override
    public boolean updateNotification(Context context, int uniqueThreadCount,
            long mostRecentThreadId, Class<?> cls, int messageCount, boolean isMostRecentSms,
            Uri mostRecentUri, Builder noti) {
        return false;
    }

    @Override
    public boolean notifyFailed(Context context, Class<?> cls,
            boolean allFailedInSameThread, boolean isDownload, long threadId,
            Builder noti, String title, String description) {
        return false;
    }

    @Override
    public void onUpdateNotification(boolean isNew) {
    }


    @Override
    public String notifyClassZeroMessage(Context context, int subId, String defaultRingtone) {
        return defaultRingtone;
    }

    @Override
    public String blockingUpdateNewMessageIndicator(Context context, long newMsgThreadId,
                            String defaultTone, boolean appNotificationEnabled) {
        return defaultTone;
    }

     @Override
    public String updateNotification(Context context, Uri mostRecentUri, String defaultRingtone) {
        return defaultRingtone;
    }

    @Override
    public String notifyFailed(Context context, long threadId, String defaultRingtone) {
        return defaultRingtone;
    }
}
