package com.mediatek.rcse.plugin.message;

import com.gsma.services.rcs.chat.GroupChatIntent;

import android.app.Notification;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.AsyncTask;
import android.provider.Telephony;
import android.provider.Telephony.Threads;

import com.mediatek.mms.ipmessage.DefaultIpMessagingNotificationExt;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageConsts.NewMessageAction;
import com.mediatek.rcs.R;

public class RcseMessagingNotification extends DefaultIpMessagingNotificationExt {
    private static String TAG = "RcseMessagingNotification";
    
    private static String NEW_MESSAGE_ACTION = "com.mediatek.mms.ipmessage.newMessage";

    private Context mContext;
    private IpMessageReceiver mIpMessageReceiver;
    private IntentFilter mIntentFilter;

    @Override
    public boolean ipMessagingNotificationInit(Context context) {
        mContext = context;
        mIpMessageReceiver = new IpMessageReceiver();
        mIntentFilter = new IntentFilter();
        mIntentFilter.addAction(NEW_MESSAGE_ACTION);
        context.registerReceiver(mIpMessageReceiver, mIntentFilter);
        return true;
    }

    @Override
    public String onIpFormatBigMessage(String number, String sender) {
        String ipSender = IpMessageContactManager.getInstance(mContext).getNameByNumber(number);
        if (!ipSender.isEmpty()) {
            sender = ipSender;
        }
        return sender;
    }
    
    @Override
    public boolean isIpAttachMessage(long msgId, Cursor cursor) {
        IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(msgId);
        if (null != ipMessage) {
            int ipMessageType = ipMessage.getType();
            if (ipMessageType != IpMessageConsts.IpMessageType.TEXT) {
                return false;
            } else {
                return true;
            }
        }
        return false;
    }
    
    @Override
    public Bitmap getIpBitmap(long msgId, Cursor cursor) {
        IpMessage ipMessage = IpMessageManager.getInstance(mContext).getIpMsgInfo(msgId);
        if (null != ipMessage) {
            int ipMessageType = ipMessage.getType();
            if (ipMessageType != IpMessageConsts.IpMessageType.TEXT) {
                if (ipMessageType == IpMessageConsts.IpMessageType.PICTURE) {
                    BitmapFactory.Options options = new BitmapFactory.Options();
                    options.inJustDecodeBounds = true;
                    String filePath = ((IpImageMessage) ipMessage).getPath();
                    return BitmapFactory.decodeFile(filePath);
                }
            } else {
                return null;
            }
        }
        return null;
    }
    
    @Override
    public boolean onIpgetNewMessageNotificationInfo(String number, long threadId) {
        if (number != null && number.startsWith(IpMessageConsts.GROUP_START)) {
            Logger.d(TAG, "new group message received.");
            Intent clickIntent = new Intent(IpMessageConsts.ACTION_GROUP_NOTIFICATION_CLICKED);
            clickIntent.putExtra(IpMessageConsts.RemoteActivities.KEY_THREAD_ID, threadId);
            clickIntent.putExtra(IpMessageConsts.RemoteActivities.KEY_BOOLEAN, false);
            return true;
        }
        return false;
    }
    
    @Override
    public String getIpNotificationTitle(String number, long threadId, String title) {
        if (number != null && number.startsWith(IpMessageConsts.GROUP_START)) {
            String displayName = IpMessageContactManager.getInstance(mContext).getNameByThreadId(
                    threadId, number);
            if (!displayName.isEmpty()) {
                Logger.d(TAG, "displayName:" + displayName);
                return displayName;
            }
        }
        return title;
    }

    @Override
    public boolean setIpSmallIcon(Notification.Builder noti, String number) {
        // / M: modify for ip message
        return super.setIpSmallIcon(noti, number);
    }

    @Override
    public String ipBuildTickerMessage(String address, String displayAddress) {
        if ((address.startsWith(IpMessageConsts.GROUP_START) || address
                .startsWith(IpMessageConsts.JOYN_START))) {
            return IpMessageContactManager.getInstance(mContext).getNameByNumber(address);
        } else {
            return displayAddress;
        }
    }

    /**
     * NotificationReceiver receives some kinds of ip message notification and
     * notify the listeners.
     */
    public class IpMessageReceiver extends BroadcastReceiver {
        private static final String TAG = "IpMessageReceiver";

        @Override
        public void onReceive(Context context, Intent intent) {
            Logger.d(TAG, "onReceive: " + "Action = " + intent.getAction());
            new ReceiveNotificationTask(context).execute(intent);
        }

        private class ReceiveNotificationTask extends AsyncTask<Intent, Void, Void> {
            private Context mContext;

            ReceiveNotificationTask(Context context) {
                mContext = context;
            }

            @Override
            protected Void doInBackground(Intent... intents) {
                Intent intent = intents[0];
                String action = intent.getAction();
                if (NewMessageAction.ACTION_NEW_MESSAGE.equals(action)) {
                    handleNewMessage(intent);
                } else {
                    Logger.w(TAG, "unknown notification type.");
                }
                return null;
            }

            private void handleNewMessage(Intent intent) {
                Logger.d(TAG, "handleNewMessage");
                long messageId = intent.getLongExtra(NewMessageAction.IP_MESSAGE_KEY, 0);
                if (messageId <= 0) {
                    Logger.e(TAG, "get ip message failed.");
                    return;
                }
                Logger.d(TAG, "new message id:" + messageId);

                String from = IpMessageContactManager.getInstance(mContext).getNumberByMessageId(
                        messageId);
                Logger.d(TAG, "\t displyFrom:" + from);
                long threadId = Telephony.Threads.getOrCreateThreadId(mContext, from);
                Logger.d(TAG, "\t threadid:" + threadId);
                // add for ipmessage
                IpMessageUtils.blockingIpUpdateNewMessageIndicator(mContext, threadId, false,
                        null);
                IpMessageUtils.notifyIpWidgetDatasetChanged(mContext);
            }
        }
    }

}
