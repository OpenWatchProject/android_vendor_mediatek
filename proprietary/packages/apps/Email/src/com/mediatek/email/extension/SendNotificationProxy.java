package com.mediatek.email.extension;

import android.content.Context;

import com.mediatek.email.ext.ISendNotification;
import com.mediatek.email.ext.IServerProviderExt;

/**
 * Proxy responsible for showing/cancelling/suspending the sending notification
 */
public class SendNotificationProxy {
    private static final SendNotificationProxy sSendNotificationProxy = new SendNotificationProxy();
    private static Context sContext;

    public static SendNotificationProxy getInstance(Context context) {
        sContext = context;
        return sSendNotificationProxy;
    }

    /**
     * Show the send notfication
     * @param accountId the account's id
     * @param eventType mail send status(fail,success)
     * @param messageCount message's count
     */
    public void showSendingNotification(long accountId, int eventType,
            int messageCount) {
        ISendNotification sendNotifer = OPExtensionFactory.getSendingNotifyExtension(sContext);
        if (sendNotifer != null) {
            sendNotifer.showSendingNotification(sContext, accountId, eventType, messageCount);
        }
    }

    /**
     * Suspend the send notification when open the related outbox
     * @param accountId the account's id
     */
    public void suspendSendFailedNotification(long accountId) {
        ISendNotification sendNotifer = OPExtensionFactory.getSendingNotifyExtension(sContext);
        if (sendNotifer != null) {
            sendNotifer.suspendSendFailedNotification(accountId);
        }
    }

    /**
     * If the account is 189 account, the notification icon should use the relevant icon.
     * @param emailAddress the account's email address
     * @return true when the account is 189 count or false
     */
    public boolean isShowSpecialNotificationIcon(String emailAddress) {
        IServerProviderExt extension = OPExtensionFactory.getProviderExtension(sContext);
        return extension.isSupportProviderList()
                && AccountSetupChooseMailProvider.isSpecialMailAccount(extension, emailAddress);
    }
}
