package com.mediatek.widget;

import java.util.List;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.util.Log;

import com.mediatek.widget.CustomAccountRemoteViews.AccountInfo;

import com.mediatek.internal.R;

public class DefaultAccountSelectionBar {

    private final static String TAG = "DefaultAccountSelectionBar";
    private CustomAccountRemoteViews mCustomAccountRemoteViews;
    private Context mContext;
    private String mPackageName;
    private NotificationManager mNotificationManager;
    private Notification mNotification;

    /**
     * 
     * @param context The context where the view associated with the notification is posted. 
     * @param packageName The package name of the component which posts the notification.
     * @param data The content of the Account {@link AccountInfo} which will be shown in the notification.
     */
    public DefaultAccountSelectionBar(Context context, String packageName, List<AccountInfo> data) {
        mContext = context;
        mPackageName = packageName;

        configureView(data);

        mNotificationManager = (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);

        mNotification = new Notification.Builder(mContext)
                .setSmallIcon(R.drawable.account_select_notification)
                .setWhen(System.currentTimeMillis())
                .setPriority(Notification.PRIORITY_MAX).build();

        mNotification.flags = Notification.FLAG_NO_CLEAR;
    }

    /**
     * Update the content of the account{@link AccountInfo}.
     * @see configureView(List<AccountInfo> data)
     * @param data The content of the Account {@link AccountInfo} which will be shown in the notification.
     */
    public void updateData(List<AccountInfo> data) {
        configureView(data);
    }

    /**
     * Show the customized notification.
     */
    public void show() {

        mNotification.contentView = mCustomAccountRemoteViews.getNormalRemoteViews();
        mNotification.bigContentView = mCustomAccountRemoteViews.getBigRemoteViews();

        mNotificationManager.notify(R.id.custom_select_default_account_notification_container, mNotification);
        Log.d(TAG, "In package show accountBar: " + mPackageName);

    }

    /**
     * Hide the customized notification.
     */
    public void hide() {
        mNotificationManager.cancel(R.id.custom_select_default_account_notification_container);
        Log.d(TAG, "In package hide accountBar: " + mPackageName);
    }

    private void configureView(List<AccountInfo> data) {
        mCustomAccountRemoteViews = new CustomAccountRemoteViews(mContext, mPackageName, data);
        mCustomAccountRemoteViews.configureView();
    }
}
