package com.mediatek.email.ext;

import android.content.Context;

/**
 * Interface definition of sending notification plugin.
 */
public interface ISendNotification {
    /**
     * M: Show sending notification when sending mails.
     * @param context
     * @param accountId
     * @param eventType
     * @param messageCount
     * @internal
     */
    public void showSendingNotification(Context context, long accountId, int eventType, int messageCount);

    /**
     * M: Failed-Notification may cancelled when user opening the outbox of
     * the last failed account
     * @param accountId
     * @internal
     */
    public void suspendSendFailedNotification(long accountId);

    /**
     * M: Cancel the notification and reset all the related variables
     */
    public void cancelSendingNotification();
}
