package com.mediatek.email.ext;

import android.content.Context;

/**
 * Dummy implementation of sending notification plugin,
 * will be used if the plugin could not be found
 */
public class DefaultSendNotification implements ISendNotification {

    @Override
    public synchronized void showSendingNotification(Context context, long accountId, int eventType,
            int messageCount) {
    }
    @Override
    public void suspendSendFailedNotification(long accountId) {

    }

    @Override
    public synchronized void cancelSendingNotification() {
    }
}
