package com.mediatek.mail.vip;

import java.lang.reflect.Field;
import java.util.ArrayList;

import com.android.emailcommon.Logging;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.Message;
import com.android.emailcommon.provider.EmailContent.MessageColumns;

import com.android.mail.R;
import com.android.mail.utils.Utils;

import android.app.Notification;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.widget.RemoteViews;

/**
 * M: The VIP notification style
 *
 */
public class VipNotificationStyle {
    private ArrayList<VIPIconRequest> mVIPIconRequest = new ArrayList<VIPIconRequest>();
    private ArrayList<Long> mVipMessageIds = new ArrayList<Long>();

    public static class VIPIconRequest {
        public static final int POSITION_LEFT = 1;
        public static final int POSITION_RIGHT = 2;
        public CharSequence mVipString;
        public int mIconPosition;
    }

    /**
     * Check the notification should add VIP icon
     * @param context the context
     * @param accountId the account id
     * @param fromList the sender address and display name
     * @param multipleUnseen is multiple message unseen
     * @param title the notification title
     * @param messageCursor the cursor of notify messages
     * @return the messages cursor
     */
    public void checkVips(Context context, long accountId, String fromList,
            boolean multipleUnseen, CharSequence title, Cursor messageCursor) {
        if (!multipleUnseen) {
            if (VipMemberCache.isVIP(fromList)) {
                VIPIconRequest request = new VIPIconRequest();
                request.mIconPosition = VIPIconRequest.POSITION_LEFT;
                request.mVipString = title;
                mVIPIconRequest.add(request);
            }
            return;
        }
        if (messageCursor == null) {
            return;
        }
        // multiple Unseen
        boolean hasVip = false;
        // check all unseen message
        int originalPosition = messageCursor.getPosition();
        String ids = builderSelectIds(messageCursor, EmailContent.ID_PROJECTION_COLUMN);
        messageCursor.moveToPosition(originalPosition);
        Cursor newMessages = null;
        try {
            newMessages = context.getContentResolver().query(Message.CONTENT_URI,
                    new String[]{MessageColumns._ID, MessageColumns.FROM_LIST},
                    MessageColumns._ID + " in " + ids, null, MessageColumns._ID + " DESC");
            if (newMessages == null) {
                return;
            }
            while (newMessages.moveToNext()) {
                long id = newMessages.getLong(0);
                String from = newMessages.getString(1);
                if (VipMemberCache.isVIP(from)) {
                    hasVip = true;
                    mVipMessageIds.add(id);
                }
            }
        } finally {
            if (newMessages != null) {
                newMessages.close();
            }
        }
        // has unseen vip message
        if (hasVip) {
            // the title need add vip icon
            VIPIconRequest request = new VIPIconRequest();
            request.mIconPosition = VIPIconRequest.POSITION_LEFT;
            request.mVipString = title;
            mVIPIconRequest.add(request);
        }
    }

    /**
     * Check the message should be add VIP icon
     * @param messageId the message id
     * @param VipString the display string of the message in notification
     */
    public void checkVipMessage(long messageId, CharSequence VipString) {
        if (mVipMessageIds.contains(messageId)) {
            VIPIconRequest request = new VIPIconRequest();
            request.mIconPosition = VIPIconRequest.POSITION_LEFT;
            request.mVipString = VipString;
            mVIPIconRequest.add(request);
        }
    }

    /// M: The VIP title need add a VIP icon. this method will create a
    /// request that adding the VIP icon, and put it into the request list.
    public void addVipTitle(CharSequence title) {
        VIPIconRequest request = new VIPIconRequest();
        request.mIconPosition = VIPIconRequest.POSITION_LEFT;
        request.mVipString = title;
        mVIPIconRequest.add(request);
    }

    private String builderSelectIds(Cursor c, int icColumnIndex) {
        if (c == null || c.getCount() <= 0) {
            return null;
        }
        StringBuilder idBuilder = new StringBuilder();
        idBuilder.append("(");
        c.moveToFirst();
        long messageId = c.getLong(icColumnIndex);
        idBuilder.append(messageId);
        while (c.moveToNext()) {
            messageId = c.getLong(icColumnIndex);
            idBuilder.append(", ");
            idBuilder.append(messageId);
        }
        idBuilder.append(")");
        return idBuilder.toString();
    }

    /**
     * Setup the vip notification
     * @param context the context
     * @param account the account
     * @param notification the original notification
     */
    public void setupVipNotification(Context context,
            Notification notification) {
        updateVipIcon(context, notification);
        updateVipSoundAndVibration(context, notification);
    }

    /**
     * Update the VIP Icon of the notification
     * @param context the context
     * @param notification the notification
     */
    public void updateVipIcon(Context context, Notification notification) {
        int vipIconId = R.drawable.ic_notification_vip_holo_light;
        updateVipIcon(notification.contentView, vipIconId);
        updateVipIcon(notification.bigContentView, vipIconId);
        if (hasVip()) {
            notification.icon = R.drawable.stat_notify_email_vip;
        }
    }

    private void updateVipIcon(RemoteViews remoteViews, int iconId) {
        if (remoteViews == null) {
            Logging.w(VipMemberCache.TAG, "The remoteViews is null.");
            return;
        }
        ArrayList<Object> actionList = null;
        try {
            /**
             * The notification's contentView and bigContentView are changed to use RemoteViews's
             * subclass BuilderRemoteViews since version LOLLIPOP.
             */
            actionList = (ArrayList<Object>) getValue(remoteViews, "mActions",
                    Utils.isRunningLOrLater());
        } catch (Exception e) {
            Logging.w(VipMemberCache.TAG, "The mActions of remoteViews may be not ArrayList");
            actionList = null;
        }
        if (actionList == null) {
            Logging.w(VipMemberCache.TAG, "Can not get mActions from " + remoteViews);
            return;
        }

        ArrayList<Object> cloneList = new ArrayList<Object>(actionList);
        for (Object obj : cloneList) {
            // find the ReflectionAction
            String className = obj.getClass().getSimpleName();
            if (!"ReflectionAction".equals(className)) {
                continue;
            }
            // find the setText method
            String method = (String) getValue(obj, "methodName");
            if (!"setText".equals(method)) {
                continue;
            }
            // get value and viewId
            CharSequence text = (CharSequence) getValue(obj, "value");
            VIPIconRequest req = getVipIconRequest(text);
            /// M: The viewId be moved from ReflectionAction to its
            /// super class Action from android 4.2
            Integer viewId = (Integer) getValue(obj, "viewId", true);
            if (req != null && viewId != null && viewId > 0) {
                int leftIconid = req.mIconPosition == VIPIconRequest.POSITION_LEFT ? iconId : 0;
                int rightIconid = req.mIconPosition == VIPIconRequest.POSITION_RIGHT ? iconId : 0;
                remoteViews.setTextViewCompoundDrawables(viewId, leftIconid, 0, rightIconid, 0);
            } else if (viewId != null && viewId > 0) {
                remoteViews.setTextViewCompoundDrawables(viewId, 0, 0, 0, 0);
            }
        }
    }

    public boolean hasVip() {
        return mVIPIconRequest.size() > 0;
    }

    private void updateVipSoundAndVibration(Context context, Notification n) {
        if (!hasVip()) {
            return;
        }
        final String ringtoneUri = VipPreferences.get(context).getVipRingtone();
        final boolean vipVibrate = VipPreferences.get(context).getVipVebarate();
        if (vipVibrate) {
            int defaults = Notification.DEFAULT_LIGHTS;
            defaults |= Notification.DEFAULT_VIBRATE;
            n.defaults = defaults;
        }
        n.audioStreamType = Notification.STREAM_DEFAULT;
        n.sound = (ringtoneUri == null) ? null : Uri.parse(ringtoneUri);
    }

    private VIPIconRequest getVipIconRequest(CharSequence text) {
        for (VIPIconRequest req : mVIPIconRequest) {
            if (req.mVipString.equals(text)) {
                return req;
            }
        }
        return null;
    }

    /**
     * M: Get the value of the field of the object through reflect
     * @param obj the object
     * @param fieldName the field name of the object
     * @return the value or null
     */
    private static Object getValue(Object obj, String fieldName) {
        return getValue(obj, fieldName, false);
    }

    /**
     * M: Get the value of the field of the current object or its super class through reflect
     * @param obj the object
     * @param fieldName the field name
     * @param inSuperClass is the field come from the super class
     * @return the value or null
     */
    private static Object getValue(Object obj, String fieldName, boolean inSuperClass) {
        Field field = null;
        try {
            if (inSuperClass) {
                field = obj.getClass().getSuperclass().getDeclaredField(fieldName);
            } else {
                field = obj.getClass().getDeclaredField(fieldName);
            }
        } catch (NoSuchFieldException e) {
            Logging.d(VipMemberCache.TAG, "getDeclaredField failed because has no " + fieldName);
            return null;
        }
        try {
            field.setAccessible(true);
            return (Object) field.get(obj);
        } catch (IllegalArgumentException e) {
            Logging.d(VipMemberCache.TAG, "get value failed because IllegalArgumentException");
        } catch (IllegalAccessException e) {
            Logging.d(VipMemberCache.TAG, "get value failed because IllegalAccessException");
        }
        return null;
    }
}
