/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.email.preferences;

import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;

import com.android.email.Preferences;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.provider.EmailContent.AccountColumns;
import com.android.mail.preferences.BasePreferenceMigrator;
import com.android.mail.preferences.FolderPreferences;
import com.android.mail.preferences.MailPrefs;
import com.android.mail.providers.Account;
import com.android.mail.providers.Folder;
import com.android.mail.providers.UIProvider;
import com.android.mail.utils.LogUtils;
import com.mediatek.mail.vip.VipPreferences;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * Migrates Email settings to UnifiedEmail
 */
public class EmailPreferenceMigrator extends BasePreferenceMigrator {
    private static final String LOG_TAG = "EmailPrefMigrator";

    @Override
    protected void migrate(final Context context, final int oldVersion, final int newVersion) {
        final List<Account> accounts = new ArrayList<Account>();

        final Cursor accountCursor = context.getContentResolver().query(Uri.parse(
                EmailContent.CONTENT_URI + "/uiaccts"),
                UIProvider.ACCOUNTS_PROJECTION_NO_CAPABILITIES, null, null, null);

        if (accountCursor == null) {
            LogUtils.e(LOG_TAG,
                    "Null cursor returned from query to %s when migrating accounts from %d to %d",
                    EmailContent.CONTENT_URI + "/uiaccts",
                    oldVersion, newVersion);
            return;
        } else {
            try {
                while (accountCursor.moveToNext()) {
                    accounts.add(Account.builder().buildFrom(accountCursor));
                }
            } finally {
                accountCursor.close();
            }
        }

        migrate(context, oldVersion, newVersion, accounts);
    }

    @SuppressWarnings("deprecation")
    protected static void migrate(final Context context, final int oldVersion, final int newVersion,
            final List<Account> accounts) {
        final Preferences preferences = Preferences.getPreferences(context);
        final MailPrefs mailPrefs = MailPrefs.get(context);
        if (oldVersion < 1) {
            // Move global settings

            final boolean hasSwipeDelete = preferences.hasSwipeDelete();
            if (hasSwipeDelete) {
                final boolean swipeDelete = preferences.getSwipeDelete();
                mailPrefs.setConversationListSwipeEnabled(swipeDelete);
            }

            // Move reply-all setting
            final boolean isReplyAllSet = preferences.hasReplyAll();
            if (isReplyAllSet) {
                final boolean replyAll = preferences.getReplyAll();
                mailPrefs.setDefaultReplyAll(replyAll);
            }

            ///M: Upgrade Email settings from JB2 to KK
            upgradeSettingsFromJB2(context);

            // Move folder notification settings
            for (final Account account : accounts) {
                // Get the emailcommon account
                final Cursor ecAccountCursor = context.getContentResolver().query(
                        com.android.emailcommon.provider.Account.CONTENT_URI,
                        com.android.emailcommon.provider.Account.CONTENT_PROJECTION,
                        AccountColumns.EMAIL_ADDRESS + " = ?",
                        new String[] { account.getEmailAddress() },
                        null);
                final com.android.emailcommon.provider.Account ecAccount =
                        new com.android.emailcommon.provider.Account();


                if (ecAccountCursor == null) {
                    LogUtils.e(LOG_TAG, "Null old account cursor for mailbox %s",
                            LogUtils.sanitizeName(LOG_TAG, account.getEmailAddress()));
                    continue;
                }

                try {
                    if (ecAccountCursor.moveToFirst()) {
                        ecAccount.restore(ecAccountCursor);
                    } else {
                        LogUtils.e(LOG_TAG, "Couldn't load old account for mailbox %s",
                                LogUtils.sanitizeName(LOG_TAG, account.getEmailAddress()));
                        continue;
                    }
                } finally {
                    ecAccountCursor.close();
                }

                // The only setting in AccountPreferences so far is a global notification toggle,
                // but we only allow Inbox notifications, so it will remain unused
                final Cursor folderCursor =
                        context.getContentResolver().query(account.settings.defaultInbox,
                                UIProvider.FOLDERS_PROJECTION, null, null, null);

                if (folderCursor == null) {
                    LogUtils.e(LOG_TAG, "Null folder cursor for mailbox %s",
                            LogUtils.sanitizeName(LOG_TAG,
                                    account.settings.defaultInbox.toString()));
                    continue;
                }

                Folder folder = null;
                try {
                    /** M: [Klocwork] Check the folder to avoid Non pointer exception. @{ */
                    if (folderCursor.moveToFirst()) {
                        folder = new Folder(folderCursor);
                    } else {
                        LogUtils.e(LOG_TAG, "Empty folder cursor for mailbox %s",
                                LogUtils.sanitizeName(LOG_TAG,
                                        account.settings.defaultInbox.toString()));
                        continue;
                    }
                    /** @} */
                } finally {
                    folderCursor.close();
                }

                final FolderPreferences folderPreferences =
                        new FolderPreferences(context, account.getEmailAddress(), folder,
                                true /* inbox */);

                final boolean notify = (ecAccount.getFlags()
                        & com.android.emailcommon.provider.Account.FLAGS_NOTIFY_NEW_MAIL) != 0;
                folderPreferences.setNotificationsEnabled(notify);

                final String ringtoneUri = ecAccount.getRingtone();
                folderPreferences.setNotificationRingtoneUri(ringtoneUri);

                final boolean vibrate = (ecAccount.getFlags()
                        & com.android.emailcommon.provider.Account.FLAGS_VIBRATE) != 0;
                folderPreferences.setNotificationVibrateEnabled(vibrate);

                folderPreferences.commit();
            }
        }

        if (oldVersion < 2) {
            final Set<String> whitelistedAddresses = preferences.getWhitelistedSenderAddresses();
            mailPrefs.setSenderWhitelist(whitelistedAddresses);
        }

        if (oldVersion < 3) {
            // The default for the conversation list icon is the sender image.
            final boolean showSenderImages = !TextUtils.equals(
                    Preferences.CONV_LIST_ICON_NONE, preferences.getConversationListIcon());
            mailPrefs.setShowSenderImages(showSenderImages);
        }
        if (oldVersion < 4) {
            final boolean confirmDelete = preferences.getConfirmDelete();
            mailPrefs.setConfirmDelete(confirmDelete);

            final boolean confirmSend = preferences.getConfirmSend();
            mailPrefs.setConfirmSend(confirmSend);

            final int autoAdvance = preferences.getAutoAdvanceDirection();
            switch(autoAdvance) {
                case Preferences.AUTO_ADVANCE_OLDER:
                    mailPrefs.setAutoAdvanceMode(UIProvider.AutoAdvance.OLDER);
                case Preferences.AUTO_ADVANCE_NEWER:
                    mailPrefs.setAutoAdvanceMode(UIProvider.AutoAdvance.NEWER);
                case Preferences.AUTO_ADVANCE_MESSAGE_LIST:
                default:
                    mailPrefs.setAutoAdvanceMode(UIProvider.AutoAdvance.LIST);
            }
        }
    }

    /** M: Upgrade Email settings from JB2 to KK @{ */
    //JB always add account address to bcc on Compose a mail.
    private static final String JB_BCC_MYSELF_KEY = "bcc_myself";
    private static final boolean JB_BCC_MYSELF_DEFAULT = false;
    //JB ask before deleting
    private static final String JB_ASK_BEFORE_DELETING_KEY = "ask_before_deleting";
    private static final boolean JB_ASK_BEFORE_DELETING_DEFAULT = true;
    //JB VIP settings
    private static final String JB_VIP_NOTIFICATION = "vip_notification";
    private static final String JB_VIP_RINGTONE = "vip_ringtone";
    private static final String JB_VIP_VIBRATE = "vip_vibarate";
    private static final boolean JB_VIP_NOTIFICATION_DEFAULT = true;
    private static final String JB_VIP_RINGTONE_DEFAULT = "content://settings/system/notification_sound";
    private static final boolean JB_VIP_VIBATATE_DEFAULT = false;

    private static void upgradeSettingsFromJB2(Context context) {
        SharedPreferences sharedPreferences = Preferences.getSharedPreferences(context);
        // Auto bcc myself. Preferences changed from "AndroidMail.Main" to
        // "UnifiedEmail"
        boolean hasIsBccMyself = sharedPreferences.contains(JB_BCC_MYSELF_KEY);
        if (hasIsBccMyself) {
            boolean isBccMyself = sharedPreferences.getBoolean(JB_BCC_MYSELF_KEY,
                    JB_BCC_MYSELF_DEFAULT);
            MailPrefs.get(context).setAutoBccMyself(isBccMyself);
            LogUtils.d(LOG_TAG, "AutoBccMyself upgrade to %b", isBccMyself);
        }
        // Conform delete. Key changed from "ask_before_deleting" to
        // "confirm_delete"
        boolean hasConfirmDelete = sharedPreferences.contains(JB_ASK_BEFORE_DELETING_KEY);
        if (hasConfirmDelete) {
            boolean confirmDelete = sharedPreferences.getBoolean(JB_ASK_BEFORE_DELETING_KEY,
                    JB_ASK_BEFORE_DELETING_DEFAULT);
            // Support for confirm_delete upgrade from version below kk
            Preferences.getPreferences(context).setConfirmDelete(confirmDelete);
            LogUtils.d(LOG_TAG, "ConfirmDelete upgrade to %b", confirmDelete);
        }
        // VIP perferences. Preferences changed from "AndroidMail.Main" to
        // "VipPreferences"
        boolean hasVipNotification = sharedPreferences.contains(JB_VIP_NOTIFICATION);
        if (hasVipNotification) {
            boolean vipNotification = sharedPreferences.getBoolean(JB_VIP_NOTIFICATION,
                    JB_VIP_NOTIFICATION_DEFAULT);
            VipPreferences.get(context).setVipNotification(vipNotification);
            LogUtils.d(LOG_TAG, "VipNotification upgrade to %b", vipNotification);
        }
        boolean hasVipRingtone = sharedPreferences.contains(JB_VIP_RINGTONE);
        if (hasVipRingtone) {
            String vipRingtone = sharedPreferences.getString(JB_VIP_RINGTONE,
                    JB_VIP_RINGTONE_DEFAULT);
            VipPreferences.get(context).setVipRingtone(vipRingtone);
            LogUtils.d(LOG_TAG, "VipRingtone upgrade to %s", vipRingtone);
        }
        boolean hasVipVibrate = sharedPreferences.contains(JB_VIP_VIBRATE);
        if (hasVipVibrate) {
            boolean vipVibrate = sharedPreferences.getBoolean(JB_VIP_VIBRATE,
                    JB_VIP_VIBATATE_DEFAULT);
            VipPreferences.get(context).setVipVebarate(vipVibrate);
            LogUtils.d(LOG_TAG, "VipVibrate upgrade to %b", vipVibrate);
        }
    }
    /** @} */
}
