package com.mediatek.ipmsg.util;

import android.content.AsyncQueryHandler;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.util.Log;

import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.MmsMessageSender;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;
import com.android.mms.widget.MmsWidgetProvider;

//import com.mediatek.common.MPlugin;
import com.mediatek.mms.callback.IUtilsCallback;
import com.mediatek.mms.ipmessage.DefaultIpMessagePluginImplExt;
import com.mediatek.mms.ipmessage.IIpMessagePluginExt;
import com.mediatek.mms.ipmessage.IIpUtilsExt;
import com.mediatek.mms.ipmessage.IpMmsCustomizationUtils;

import java.util.Collection;

public class IpMessageUtils implements IUtilsCallback {
    private static final String TAG = "Mms/ipmsg/utils";

    /// M: add for ipmessage {@
    public static IIpMessagePluginExt sIpMessagePlugin = null;

    private static IpMessageUtils sIpMessageUtils = null;

    private static IIpUtilsExt mIpUtils = null;

    public static final class IpMessageType {
        public static final int TEXT = 0;
        public static final int GROUP_CREATE_CFG = 1;
        public static final int GROUP_ADD_CFG = 2;
        public static final int GROUP_QUIT_CFG = 3;
        public static final int PICTURE = 4;
        public static final int VOICE = 5;
        public static final int VCARD = 6;
        public static final int VIDEO = 9;
        public static final int CALENDAR = 10;
        public static final int UNKNOWN_FILE = 11;
        public static final int COUNT = 12;
        // / M: add for ipmessage readburn
        public static final int READEDBURN = 13;
    }
    /// M: add for ipmessage {@
    public static final String SELECTION_CONTACT_RESULT = "contactId";
    /// @}

    public static synchronized IIpMessagePluginExt getIpMessagePlugin(Context context) {
        if (sIpMessagePlugin == null) {
            initIpMessagePlugin(context);
        }
        return sIpMessagePlugin;
    }

    private static synchronized void initIpMessagePlugin(Context context) {
        if (sIpMessagePlugin == null) {
           sIpMessageUtils = new IpMessageUtils();
/*           sIpMessagePlugin = (IIpMessagePluginExt) MPlugin.createInstance(
                    IIpMessagePluginExt.class.getName(), context);
    */       MmsLog.d(TAG, "sIpMessagePlugin = " + sIpMessagePlugin);
            if (sIpMessagePlugin == null) {
//                sIpMessagePlugin = new DefaultIpMessagePluginImplExt(context);
                sIpMessagePlugin = IpMmsCustomizationUtils.getOpFactory(context)
                        .makeIpMessagePluginExt();
                MmsLog.d(TAG, "default sIpMessagePlugin = " + sIpMessagePlugin);
            }
            mIpUtils = sIpMessagePlugin.getIpUtils();
            mIpUtils.initIpUtils(sIpMessageUtils);
        }
    }

    public static void onIpBootCompleted(Context context) {
        if (mIpUtils == null) {
            initIpMessagePlugin(context);
        }
        mIpUtils.onIpBootCompleted(context);
    }

    public static void onIpMmsCreate(Context context) {
        if (mIpUtils == null) {
            initIpMessagePlugin(context);
        }
        mIpUtils.onIpMmsCreate(context);
    }


    public static void onIpDeleteMessage(Context context, Collection<Long> threadIds, int maxSmsId,
            boolean deleteLockedMessages) {
        mIpUtils.onIpDeleteMessage(context, threadIds, maxSmsId, deleteLockedMessages);
    }

    @Override
    public void blockingIpUpdateNewMessageIndicator(Context context, long newMsgThreadId,
            boolean isStatusMessage, Uri statusMessageUri) {
        MessagingNotification.blockingUpdateNewMessageIndicator(context, newMsgThreadId,
                isStatusMessage, statusMessageUri);
    }

    @Override
    public int getNotificationResourceId() {
        return R.drawable.stat_notify_sms;
    }

    @Override
    public void notifyIpWidgetDatasetChanged(Context context) {
        MmsWidgetProvider.notifyDatasetChanged(context);
    }

    @Override
    public boolean isIpHome(Context context) {
        return MessageUtils.isHome(context);
    }

    public Drawable getContactDrawalbeFromNumber(Context context,
            Drawable defaultValue, String number, boolean needRequery) {
        Contact c = Contact.get(number, needRequery);
        c.getName();
        c.existsInDatabase();
        return c.getAvatar(context, defaultValue, 0);
    }

    public static CharSequence formatIpMessage(CharSequence body, boolean showImg,
            CharSequence buf) {
        return mIpUtils.formatIpMessage(body, showImg, buf);
    }

    public static long getKey(String type, long id) {
        return mIpUtils.getKey(type, id);
    }

    @Override
    public boolean sendMms(Context context, Uri location,
            long messageSize, int subId, long threadId) {
        try {
            MmsMessageSender sender = new MmsMessageSender(context, location, messageSize, subId);
            return sender.sendMessage(threadId);
        } catch (Exception e) {
            // TODO: handle exception
            Log.e(TAG, "[sendMms] exception = " + e);
        }
        return false;
    }

    @Override
    public String formatIpTimeStampString(Context context, long when, boolean fullFormat) {
        return MessageUtils.formatTimeStampString(context, when, fullFormat);
    }

    public static boolean startQueryForConversation(AsyncQueryHandler handler, String[] projection,
            int token, String selection) {
        return mIpUtils.startQueryForConversation(handler, projection, token, selection);
    }

    public static boolean startQueryHaveLockedMessages(AsyncQueryHandler handler, int token,
            Object cookie, Uri uri, String[] projection,
            String selection, String[] selectionArgs) {
        return mIpUtils.startQueryHaveLockedMessages(handler, token, cookie, uri, projection,
                                    selection, selectionArgs);
    }

    public static void blockingMarkAllIpMessageAsSeen(Context context) {
        mIpUtils.blockingMarkAllIpMessageAsSeen(context);
    }

    public static Uri startDeleteForConversation(Uri uri) {
        return mIpUtils.startDeleteForConversation(uri);
    }
}
