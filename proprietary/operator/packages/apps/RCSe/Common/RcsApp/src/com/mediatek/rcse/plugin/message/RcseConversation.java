package com.mediatek.rcse.plugin.message;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import mediatek.telephony.MtkTelephony;
import mediatek.telephony.MtkTelephony.MtkThreads;
import mediatek.telephony.MtkTelephony.MtkThreadSettings;
import android.util.Log;

import com.mediatek.mms.ipmessage.DefaultIpConversationExt;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageConsts.ContactStatus;
import com.mediatek.rcse.plugin.message.IpMessageConsts.RemoteActivities;

public class RcseConversation extends DefaultIpConversationExt {
    public String mContactNumber = null;
    public static final Uri sAllThreadsUriExtend =
            Uri.parse("content://mms-sms/conversations/extend").buildUpon()
                    .appendQueryParameter("simple", "true").build();
        public static final Uri sAllUnreadMessagesUri = Uri.parse("content://mms-sms/unread_count");

        /// M: use this instead of the google default to query more columns in thread_settings
        public static final String[] ALL_THREADS_PROJECTION_EXTEND = {
            MtkThreads._ID, MtkThreads.DATE, MtkThreads.MESSAGE_COUNT, MtkThreads.RECIPIENT_IDS,
            MtkThreads.SNIPPET, MtkThreads.SNIPPET_CHARSET, MtkThreads.READ, MtkThreads.ERROR,
            MtkThreads.HAS_ATTACHMENT
            /// M:
            , MtkThreads.TYPE , MtkTelephony.MtkThreads.READ_COUNT , MtkTelephony.MtkThreads.STATUS,
            MtkTelephony.MtkThreadSettings._ID, /// M: add for common
            MtkTelephony.MtkThreadSettings.NOTIFICATION_ENABLE,
            MtkTelephony.MtkThreadSettings.SPAM, MtkTelephony.MtkThreadSettings.MUTE,
            MtkTelephony.MtkThreadSettings.MUTE_START,
            MtkTelephony.MtkThreads.DATE_SENT
        };

        /// M: this is used as a cache flag for performance.
        private static boolean sIsActivated = false;
        
        private boolean mSpamStatus;
        /// M: New feature for rcse, adding IntegrationMode and Spam date. @{
        private boolean mIsFullIntegrationMode = false;
        private long mSpamDate;

        /** M: this type is used by mType, a convenience for identify a group conversation.
         *  so currently mType value maybe 0 sms 1 mms 2 wappush 3 cellbroadcast 10 guide 110 group
         *  the matched number maybe not right, but the type is as listed.
         */
        public static final int TYPE_GROUP = 110;

        public static final int SPAM           = 14;

        public int onIpFillFromCursor(Context context, Cursor c, int recipientSize, String number, int type, long date) {
            Log.d("RcseConversation", "onIpFillFromCursor" + number);
            mContactNumber = number;
            /// M: if ipmessage is activated, we get extra columns by another query.
            if (c.getColumnCount() > SPAM && sIsActivated) {
                int spam = c.getInt(SPAM);
                mSpamStatus = (spam == 1) ? true : false;
            }

            if (sIsActivated && recipientSize != 0 &&
                    number.startsWith(IpMessageConsts.GROUP_START)) {
                    return TYPE_GROUP;
                }
            Log.d("RcseConversation", "onIpFillFromCursor" + number);
            //setRCSMode(context,number);
            /// M: New feature for rcse, adding IntegrationMode. @{
            if (c.getColumnCount() > SPAM && sIsActivated && recipientSize == 1) {
                if (IpMessageContactManager.getInstance(context).getIntegratedModeForContact(
                        number) == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                    mIsFullIntegrationMode = true;
                }
                if (mSpamStatus) {
                    long spamDate = IpMessageContactManager.getInstance(context).getSpamTime(number);
                    if (spamDate == 0) {
                        mSpamDate = date;
                    } else {
                        mSpamDate = spamDate;
                    }
                }
            }
            return type;
    }
        public void setRCSMode(Context context, String number)
        {
            Logger.d("RcseConversation", "setRCSMode number :" + number);
            boolean mIsIpMessageRecipients = false;
            boolean status = isCurrentIpmessageSendable(context,number);

            if (IpMessageServiceMananger.getInstance(context).getIntegrationMode() == IpMessageConsts.IntegrationMode.FULLY_INTEGRATED) {
                Logger.d("RcseConversation", "is full integrated mode");
                if (IpMessageContactManager.getInstance(context).isIpMessageNumber(number)) {
                    mIsIpMessageRecipients = true;
                }
                if (mIsIpMessageRecipients && status) {
                    RcseComposeActivity.mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
                } else {
                    RcseComposeActivity.mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                }
            } else {
                if (number.startsWith(IpMessageConsts.JOYN_START)) {
                    RcseComposeActivity.mCurrentChatMode = IpMessageConsts.ChatMode.JOYN;
                } else {
                    RcseComposeActivity.mCurrentChatMode = IpMessageConsts.ChatMode.XMS;
                }
            }
            Logger.d("RcseConversation", "setRCSMode exit :" + RcseComposeActivity.mCurrentChatMode);
        
        }
     // / M: Fix ipmessage bug ,fix bug ALPS 01556382@{
        private boolean isCurrentIpmessageSendable(Context context, String number) {      
            if (IpMessageContactManager.getInstance(context).getStatusByNumber(number) == ContactStatus.OFFLINE) {
                Logger.d("RcseConversation", "isCurrentIpmessageSendable(): false ");
                return false;
            } else {
                return true;
            }
        }
        
        /**
         * Returns true if this conversation is a spam thread.
         */
        public synchronized boolean isSpam() {
            return mSpamStatus;
        }

        public synchronized void setSpam(boolean isSpam) {
            this.mSpamStatus = isSpam;
        }

        /**
         * note: invoke this method in UI thread.
         * and the variable is used in UI thead. keep this and we needn't a lock.
         */
        public static void setActivated(boolean activated) {
            sIsActivated = activated;
        }

        public static boolean getActivated() {
            return sIsActivated;
        }

        /// M: New feature for rcse, adding IntegrationMode. @{
        public boolean getIsFullIntegrationMode() {
            return mIsFullIntegrationMode;
        }

        public synchronized long getSpamDate() {
            return mSpamDate;
        }
        /// @}
}
