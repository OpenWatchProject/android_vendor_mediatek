/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
package com.mediatek.rcse.service;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.widget.Toast;

import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.ChatScreenActivity;
import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.interfaces.ChatModel.IChatManager;
//import com.mediatek.rcse.mvc.ContactId;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.mvc.ChatMessage;
import com.mediatek.rcse.plugin.message.PluginGroupChatWindow;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;
//import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.chat.OneToOneChat;
import com.gsma.services.rcs.chat.ChatIntent;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
//import com.gsma.services.rcs.chat.ChatMessage;
import com.gsma.services.rcs.chat.ChatService;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.ft.FileTransferIntent;
import com.gsma.services.rcs.ft.FileTransferService;
import com.gsma.services.rcs.sharing.image.ImageSharingIntent;
import com.gsma.services.rcs.sharing.video.VideoSharingIntent;

/**
 * The Class RcsNotification.
 */
public class RcsNotification {
    /**
     * Notification ID for chat.
     */
    private static final int NOTIFICATION_ID_CHAT = 1000;
    /**
     * Notification ID for file transfer.
     */
    private static final int NOTIFICATION_ID_FILE_TRANSFER = 1001;
    /**
     * The Constant NOTIFICATION_ID_UNREAD_MESSAGE.
     */
    private static final int NOTIFICATION_ID_UNREAD_MESSAGE = 1004;
    /**
     * The Constant NOTIFICATION_TITLE_LENGTH_MAX.
     */
    private static final int NOTIFICATION_TITLE_LENGTH_MAX = 20;
    /**
     * The Constant NOTIFICATION_CHAR_LENGTH_CONDITION.
     */
    private static final int NOTIFICATION_CHAR_LENGTH_CONDITION = 0xFF;
    /**
     * The Constant NOTIFICATION_CHAR_LENGTH_SINGLE_BYTE.
     */
    private static final int NOTIFICATION_CHAR_LENGTH_SINGLE_BYTE = 1;
    /**
     * The Constant NOTIFICATION_CHAR_LENGTH_DOUBLE_BYTE.
     */
    private static final int NOTIFICATION_CHAR_LENGTH_DOUBLE_BYTE = 2;
    /**
     * The Constant FORCE_SCROLLTO_CHAT.
     */
    public static final String FORCE_SCROLLTO_CHAT = "forceScroll";
    /**
     * The Constant TAG.
     */
    private static final String TAG = "RcsNotification";
    /**
     * The Constant FIRST_MESSAGE.
     */
    private static final String FIRST_MESSAGE = "firstMessage";
    /**
     * The Constant GROUP_SUBJECT.
     */
    private static final String GROUP_SUBJECT = "subject";
    /**
     * The Constant CONTACT.
     */
    public static final String CONTACT = "contact";
    /**
     * The Constant MESSAGES.
     */
    private static final String MESSAGES = "messages";
    /**
     * The Constant SESSION_ID.
     */
    public static final String SESSION_ID = "sessionId";
    /**
     * The Constant DISPLAY_NAME.
     */
    public static final String DISPLAY_NAME = "contactDisplayname";
    /**
     * The Constant CHAT_SESSION_ID.
     */
    public static final String CHAT_SESSION_ID = "chatSessionId";
    /**
     * The Constant ISGROUPTRANSFER.
     */
    public static final String ISGROUPTRANSFER = "isGroupTransfer";
    /**
     * The Constant ISHTTPTRANSFER.
     */
    public static final String ISHTTPTRANSFER = "isHttpTransfer";
    /**
     * Chat id.
     */
    public static final String CHAT_ID = "chatId";
    /**
     * Chat id.
     */
    public static final String MSG_ID = "msgid";
    /**
     * Auto accept group invitation.
     */
    public static final String AUTO_ACCEPT = "autoAccept";
    // The RcsNotification instance
    /**
     * The Constant INSTANCE.
     */
    private static final RcsNotification INSTANCE = new RcsNotification();;
    // The GroupInviteChangeListener instance
    /**
     * The m listener.
     */
    private GroupInviteChangedListener mListener = null;
    // The blank space text
    /**
     * The Constant BLANK_SPACE.
     */
    private static final String BLANK_SPACE = " ";
    // The empty text
    /**
     * The Constant EMPTY_STRING.
     */
    private static final String EMPTY_STRING = "";
    // The seprator text
    /**
     * The Constant SEPRATOR.
     */
    private static final String SEPRATOR = ",";
    // Used for storing the title of a specify notification
    /**
     * The Constant NOTIFY_TITLE.
     */
    public static final String NOTIFY_TITLE = "notify_title";
    // Used for storing the content of a specify notification
    /**
     * The Constant NOTIFY_CONTENT.
     */
    public static final String NOTIFY_CONTENT = "notify_content";
    // Used for storing the additional information of a specify notification
    /**
     * The Constant NOTIFY_INFORMATION.
     */
    public static final String NOTIFY_INFORMATION = "notify_information";
    /**
     * The Constant NOTIFY_SIZE.
     */
    public static final String NOTIFY_SIZE = "notify_size";
    /**
     * The Constant NOTIFY_FILE_NAME.
     */
    public static final String NOTIFY_FILE_NAME = "notify_file_name";
    // Indicates the single group invitation
    /**
     * The Constant SINGLE_GROUP_INVITATION.
     */
    private static final int SINGLE_GROUP_INVITATION = 1;
    // The tag of unread message notification
    /**
     * The Constant UNREAD_MESSAGE.
     */
    private static final String UNREAD_MESSAGE = "UnreadMessage";
    /**
     * The Constant FILE_TRANSFER.
     */
    private static final String FILE_TRANSFER = "FileTransfer";
    /**
     * The m group invitation infos.
     */
    private ConcurrentHashMap<String, GroupInvitationInfo> mGroupInvitationInfos =
            new ConcurrentHashMap<String, GroupInvitationInfo>();
    /**
     * The m temp group invitation infos.
     */
    private ConcurrentHashMap<GroupChat, Intent> mTempGroupInvitationInfos =
            new ConcurrentHashMap<GroupChat, Intent>();
    /**
     * The Constant GROUP_PARTICIPANT_SIZE_TWO.
     */
    private static final int GROUP_PARTICIPANT_SIZE_TWO = 2;
    /**
     * The Constant GROUP_PARTICIPANT_SIZE_THREE.
     */
    private static final int GROUP_PARTICIPANT_SIZE_THREE = 3;
    /**
     * The Constant GROUP_INVITATION_TAG.
     */
    private static final String GROUP_INVITATION_TAG = "groupInvitation";
    /**
     * The m un read messages chat infos.
     */
    private ConcurrentHashMap<Object, ChatReceivedInfo> mUnReadMessagesChatInfos =
            new ConcurrentHashMap<Object, ChatReceivedInfo>();
    /**
     * The m file invitation infos.
     */
    private List<FileInvitationInfo> mFileInvitationInfos =
            new CopyOnWriteArrayList<FileInvitationInfo>();
    /**
     * The Constant FILE_INVITATION_INDEX_ZERO.
     */
    public static final int FILE_INVITATION_INDEX_ZERO = 0;
    /**
     * The s is store and forward message notified.
     */
    public boolean mIsStoreAndForwardMessageNotified = false;
    /**
     * The m is in chat main activity.
     */
    private boolean mIsInChatMainActivity = false;

    /**
     * set is in ChatMainActivity.
     *
     * @param isInChatMainActivity the new checks if is in chat main activity
     */
    public void setIsInChatMainActivity(boolean isInChatMainActivity) {
        mIsInChatMainActivity = isInChatMainActivity;
    }
    /**
     * Constructor.
     */
    private RcsNotification() {
    }
    /**
     * Get the RcsNotification singleton instance.
     *
     * @return The RcsNotification instance
     */
    public static RcsNotification getInstance() {
        return INSTANCE;
    }
    /**
     * Register the GroupInviteChangeListener.
     *
     * @param listener            The GroupInviteChangeListener to register
     */
    public synchronized void registerGroupInviteChangedListener(
            GroupInviteChangedListener listener) {
        Logger.d(TAG,
                "registerGroupInviteChangedListener() entry listener is "
                        + listener);
        mListener = listener;
        Logger.d(TAG, "registerGroupInviteChangedListener() exit");
    }
    /**
     * Unregister the GroupInviteChangeListener.
     */
    public synchronized void unregisterGroupInviteChangedListener() {
        Logger.d(TAG,
                "unregisterGroupInviteChangedListener() entry listener is "
                        + mListener);
        if (mListener != null) {
            mListener = null;
        }
        Logger.d(TAG,
                "unregisterGroupInviteChangedListener() exit listener is "
                        + mListener);
    }

    /**
     * The listener used to notify that the invitation has changed.
     *
     * @see GroupInviteChangedEvent
     */
    public interface GroupInviteChangedListener {
        /**
         * Add group invitation.
         *
         * @param chatSession
         *            The chatSession used to add.
         * @param intent
         *            The invite intent used to add.
         */
        void onAddedGroupInvite(final GroupChat chatSession,
                final Intent intent);
        /**
         * Remove group invitation.
         *
         * @param chatId the chat id
         */
        void onRemovedGroupInvite(final String chatId);
    }

    /**
     * Remove the stored data of one specific session in notification manager,
     * provided.
     *
     * @param chatId the chat id
     */
    public synchronized void removeGroupInvite(String chatId) {
        Logger.d(TAG, "removeGroupInvite() entry, mListener: "
                + mListener);
        if (mListener != null) {
            mListener.onRemovedGroupInvite(chatId);
        }
        updateTempGroupInvitationInfos(chatId);
        mGroupInvitationInfos.remove(chatId);
        updateGroupInvitationNotification();
        Logger.d(TAG, "removeGroupInvite() exit");
    }
    /**
     * remove a item from mTempGroupInvitationInfos by sessionId.
     *
     * @param chatId the chat id
     */
    private void updateTempGroupInvitationInfos(String chatId) {
        Logger.d(TAG,
                "updateTempGroupInvitationInfos() entry, chatId: "
                        + chatId + " size: "
                        + mTempGroupInvitationInfos.size());
        try {
            for (Map.Entry<GroupChat, Intent> entry : mTempGroupInvitationInfos
                    .entrySet()) {
                if (chatId.equals(entry.getKey().getChatId())) {
                    Logger.d(TAG,
                            "updateTempGroupInvitationInfos() find session id equal");
                    mTempGroupInvitationInfos.remove(entry.getKey());
                    break;
                }
            }
        } catch (Exception e) {
            Logger.e(TAG,
                    "updateTempGroupInvitationInfos() Get chat session failed");
            e.printStackTrace();
        }
        Logger.v(TAG, "updateTempGroupInvitationInfos() exit, size: "
                + mTempGroupInvitationInfos.size());
    }
    /**
     * Get all the group chat invitations map.
     *
     * @return The group chat invitations map.
     */
    public synchronized ConcurrentHashMap<GroupChat, Intent> getTempGroupInvite() {
        Logger.d(TAG,
                "getTempGroupInvite() entry, mTempGroupInvitationInfos size: "
                        + mTempGroupInvitationInfos.size());
        return mTempGroupInvitationInfos;
    }
    /**
     * Store the group chat invitation with a specific session in notification
     * manager.
     *
     * @param chatSession the chat session
     * @param info            The group invitation information.
     */
    private synchronized void addGroupInvite(GroupChat chatSession,
            GroupInvitationInfo info) {
        Logger.d(TAG, "addGroupInvite entry, mListener: " + mListener);
        if (mListener != null) {
            Logger.d(TAG, "addGroupInvite mListener is not null");
            mListener.onAddedGroupInvite(chatSession, info.intent);
        }
        mTempGroupInvitationInfos.put(chatSession, info.intent);
        try {
            String chatId = chatSession.getChatId();
            mGroupInvitationInfos.put(chatId, info);
            Logger.d(TAG, "addGroupInvite() sessionId: " + chatId);
        } catch (Exception e) {
            Logger.e(TAG, "addGroupInvite getSessionID fail");
            e.printStackTrace();
        }
    }
    /**
     * Cancel group invitation notification.
     */
    private void cancelGroupInviteNotification() {
        int size = mGroupInvitationInfos.size();
        Logger.d(TAG, "cancelGroupInviteNotification() entry, size: "
                + size);
        if (size == 0) {
            Context context = ApiManager.getInstance().getContext();
            Logger.e(TAG, "cancelGroupInviteNotification() context: "
                    + context);
            if (context != null) {
                NotificationManager groupInviteNotification = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                groupInviteNotification.cancel(GROUP_INVITATION_TAG,
                        NOTIFICATION_ID_CHAT);
            }
        }
        Logger.d(TAG, "cancelGroupInviteNotification exit");
    }
    /**
     * Cancel File transfer notification when cancel the notification.
     *
     * @param contact            the notification related to.
     * @param cancelSize the cancel size
     */
    public void cancelFileTransferNotificationWithContact(
            String contact, long cancelSize) {
        Logger.d(TAG,
                "cancelFileTransferNotificationWithContact entry  the contact is "
                        + contact + "cancleSize is " + cancelSize);
        int size = mFileInvitationInfos.size();
        for (int i = 0; i < size; i++) {
            if (contact.equals(mFileInvitationInfos.get(i).mContact)) {
                if (cancelSize >= 0) {
                    Logger.d(
                            TAG,
                            "cancelFileTransferNotificationWithContact()"
                                    + " with param cancelsize find the relavent contact");
                    mFileInvitationInfos.get(i).mInviteNumber--;
                    mFileInvitationInfos.get(i).mFileSize -= cancelSize;
                    if (mFileInvitationInfos.get(i).mInviteNumber == 0) {
                        mFileInvitationInfos.remove(i);
                    }
                } else {
                    Logger.e(
                            TAG,
                            "cancelFileTransferNotificationWithContact the" +
                            " cancel size is illegal");
                }
                updateFileTansferNotification(null);
                break;
            }
        }
        Logger.d(
                TAG,
                "cancelFileTransferNotificationWithContact with param cancelsize exit");
    }
    /**
     * Cancel File transfer notification.
     *
     * @param contact
     *            the notification related to.
     */
    public void cancelFileTransferNotificationWithContact(
            String contact) {
        Logger.d(TAG,
                "cancelFileTransferNotificationWithContact() entry with the contact "
                        + contact);
        int size = mFileInvitationInfos.size();
        Logger.d(
                TAG,
                "cancelFileTransferNotificationWithContact() mFileInvitationInfos size is "
                        + size);
        for (int i = 0; i < size; i++) {
            if (contact.equals(mFileInvitationInfos.get(i).mContact)) {
                Logger.d(
                        TAG,
                        "cancelFileTransferNotificationWithContact() find the relavent contact");
                mFileInvitationInfos.remove(i);
                updateFileTansferNotification(null);
                break;
            }
        }
        Logger.d(TAG,
                "cancelFileTransferNotificationWithContact() exit");
    }
    /**
     * Gets the chat session.
     *
     * @param intent the intent
     * @return the chat session
     */
    private OneToOneChat getChatSession(Intent intent) {
        ApiManager instance = ApiManager.getInstance();
        String msgId = intent
                .getStringExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID);
        String sessionId = null;
        Logger.d(TAG, "getChatSession() msgid:" + msgId);
        if (ApiManager.getInstance() != null){
            ChatService chatService = ApiManager.getInstance().getChatApi();
            if(chatService != null){
                try {
                    com.gsma.services.rcs.chat.ChatMessage msg = chatService.getChatMessage(msgId);
                    sessionId = msg.getRemoteContact().toString();
                    
                    Logger.d(TAG, " getChatSession sessionId "+ sessionId);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.d(TAG, "chatService is null ");
            }
        } else {
            Logger.d(TAG, "ApiManager is null ");
        }
        /*String sessionId = intent
                .getStringExtra(ChatIntent.EXTRA_CONTACT);*/
        if (instance == null) {
            Logger.i(TAG, "ApiManager instance is null");
            return null;
        }
        ChatService chatApi = instance.getChatApi();
        if (chatApi == null || sessionId == null) {
            Logger.d(TAG, "MessagingApi instance is null or sessionid is null");
            return null;
        }
        OneToOneChat chatSession = null;
        Logger.d(TAG, "The chat session is null1");
        try {
            chatSession = chatApi.getOneToOneChat(ContactIdUtils.createContactIdFromTrustedData(sessionId));
        } catch (Exception e) {
            Logger.e(TAG, "Get chat session failed");
            Logger.d(TAG, "The chat session is null2");
            e.printStackTrace();
            chatSession = null;
        }
        /*if (chatSession != null) {
            return chatSession;
        }
        try {
            // Set<Chat> totalChats = null;
            Set<Chat> totalChats = chatApi.getChats();
            if (totalChats != null) {
                Logger.w(TAG, "aaa getChatSession size: "
                        + totalChats.size());
                Logger.d(TAG, "The chat session is null3 : "
                        + totalChats.size());
                for (Chat setElement : totalChats) {
                    if (setElement.getRemoteContact().equals(
                            sessionId)) {
                        Logger.e(TAG, "Get chat session finally");
                        // might work or might throw exception, Java calls it
                        // indefined behaviour:
                        chatSession = setElement;
                        break;
                    }
                }
            } else {
                Logger.w(TAG, "aaa getChatSession size: null");
            }
        } catch (JoynServiceException e) {
            Logger.e(TAG, "Get chat session xyz");
            e.printStackTrace();
        }*/
        return chatSession;
    }
    /**
     * Gets the group chat session.
     *
     * @param intent the intent
     * @return the group chat session
     */
    private GroupChat getGroupChatSession(Intent intent) {
        ApiManager instance = ApiManager.getInstance();
        String chatId = intent
                .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
        Logger.d(TAG, "getGroupChatSession chatId" + chatId);
        if (instance == null) {
            Logger.i(TAG, "ApiManager instance is null");
            return null;
        }
        ChatService chatApi = instance.getChatApi();
        if (chatApi == null) {
            Logger.d(TAG, "MessageingApi instance is null");
            return null;
        }
        GroupChat chatSession = null;
        try {
            chatSession = chatApi.getGroupChat(chatId);
        } catch (Exception e) {
            Logger.e(TAG, "Get chat session failed");
            e.printStackTrace();
            chatSession = null;
        } finally {
            return chatSession;
        }
    }
    /**
     * Gets the file transfer session.
     *
     * @param transferId the transfer id
     * @return the file transfer session
     */
    private FileTransfer getFileTransferSession(String transferId) {
        ApiManager instance = ApiManager.getInstance();
        FileTransferService ftApi = instance.getFileTransferApi();
        if (ftApi == null) {
            Logger.d(TAG,
                    "getFileTransferSession() MessageingApi instance is null");
            return null;
        }
        FileTransfer fileTransferSession = null;
        try {
            fileTransferSession = ftApi.getFileTransfer(transferId);
        } catch (Exception e) {
            Logger.e(TAG,
                    "getFileTransferSession() Get file session failed");
            e.printStackTrace();
            fileTransferSession = null;
        } finally {
            return fileTransferSession;
        }
    }
    /**
     * Handle the group chat invitation.
     *
     * @param context
     *            The context.
     * @param invitation
     *            The group chat invitation.
     * @param autoAccept
     *            True if auto accept, otherwise false.
     */
    private void handleGroupChatInvitation(Context context,
            Intent invitation, boolean autoAccept) {
        Logger.v(TAG,
                "handleGroupChatInvitation() entry, autoAccept: "
                        + autoAccept);
        if (autoAccept) {
            handlePluginGroupChatInvitation(context, invitation, true);
            autoAcceptGroupChat(context, invitation);
        } else {
            if (Logger.getIsIntegrationMode()) {
                Logger.d(TAG,
                        "handleGroupChatInvitation integration mode");
                handlePluginGroupChatInvitation(context, invitation,
                        false);
            } else {
                Logger.d(TAG,
                        "handleGroupChatInvitation chat app mode");
            }
            nonAutoAcceptGroupChat(context, invitation);
        }
        Logger.v(TAG, "handleGroupChatInvitation() exit");
    }
    /**
     * Handle plugin group chat invitation.
     *
     * @param context the context
     * @param invitation the invitation
     * @param autoAccept the auto accept
     */
    private void handlePluginGroupChatInvitation(Context context,
            Intent invitation, boolean autoAccept) {
        Logger.d(TAG, "handlePluginGroupChatInvitation entry");
        try {
            GroupChat chatSession = getGroupChatSession(invitation);
            UUID uuid = UUID.randomUUID();
            ParcelUuid parcelUuid = new ParcelUuid(uuid);
            if (chatSession == null) {
                Logger.d(TAG, "by simple The chatSession is null");
                return;
            }
            //String sessionId = invitation.getStringExtra(SESSION_ID);
            String groupSubject = chatSession.getSubject();/*invitation
                    .getStringExtra(GroupChatIntent.EXTRA_SUBJECT);*/
            invitation.putExtra(GroupChatIntent.EXTRA_SUBJECT, groupSubject);
            Logger.v(TAG, "handlePluginGroupChatInvitation() subject: "
                    + groupSubject);
            String chatId = invitation
                    .getStringExtra(GroupChatIntent.EXTRA_CHAT_ID);
            GroupInvitationInfo info = buildNotificationInfo(context,
                    invitation);
            if (info == null) {
                Logger.d(TAG, "notification info  is null");
                return;
            }
            // Add mms db
            String contact = PluginGroupChatWindow
                    .generateGroupChatInvitationContact(chatId);
            if (autoAccept) {
                contact = PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER
                        + parcelUuid.toString();
                Logger.d(TAG, "auto accept is true");
            }
            invitation.putExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT,
                    contact);
            int messageTag = PluginGroupChatWindow.GROUP_CHAT_INVITATION_IPMSG_ID;
            Logger.d(TAG, "notify info is" + info.notifyInfo + "contact="
                    + contact);
            // broadcast intent to mms plugin to insert into mms db
            Intent intent = new Intent();
            if (info != null) {
                intent.putExtra("notify", info.notifyInfo);
            }
            intent.putExtra("contact", contact);
            intent.putExtra("messageTag", messageTag);
            intent.putExtra("subject", groupSubject);
            intent.setAction(IpMessageConsts.JoynGroupInvite.ACTION_GROUP_IP_INVITATION);
            intent.putExtra("groupinvite", 1);
            MediatekFactory.getApplicationContext().sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
            /*
             * Long messageIdInMms = PluginUtils.insertDatabase(info.notifyInfo,
             * contact, messageTag, PluginUtils.INBOX_MESSAGE); if
             * (ThreadTranslater.tagExistInCache(contact)) { Logger.d(TAG,
             * "plugingroupchatinvitation() Tag exists" + contact); Long thread =
             * ThreadTranslater.translateTag(contact); insertThreadIDInDB(thread,
             * groupSubject); }
             */
            if (contact
                    .startsWith(PluginGroupChatWindow.GROUP_CONTACT_STRING_BEGINNER)) {
                contact = contact.substring(4);
            }
            invitation.putExtra(ChatScreenActivity.KEY_CHAT_TAG,
                    parcelUuid);
            invitation.putExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT,
                    contact);
            Logger.d(TAG,
                    "handlePluginGroupChatInvitation parcelUuid is "
                            + parcelUuid.toString() + "contact is"
                            + contact);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    /**
     * Handle the group chat invitation with automatic accept.
     *
     * @param context
     *            The context.
     * @param invitation
     *            The group chat invitation.
     */
    private void autoAcceptGroupChat(Context context,
            Intent invitation) {
        Logger.v(TAG, "autoAcceptGroupChat entry");
        ArrayList<ChatMessage> messages = new ArrayList<ChatMessage>();
        ChatMessage msg = invitation
                .getParcelableExtra(FIRST_MESSAGE);
        if (msg != null) {
            messages.add(msg);
        }
        invitation.putParcelableArrayListExtra(MESSAGES, messages);
        ModelImpl.getInstance().handleNewGroupInvitation(invitation,
                false);
        Logger.v(TAG, "autoAcceptGroupChat exit");
    }
    /**
     * Handle the group chat invitation with non-automatic accept.
     *
     * @param context
     *            The context.
     * @param invitation
     *            The group chat invitation.
     */
    private void nonAutoAcceptGroupChat(Context context,
            Intent invitation) {
        Logger.v(TAG, "handleGroupChatInvitation entry");
        GroupInvitationInfo info = buildNotificationInfo(context,
                invitation);
        if (info == null) {
            Logger.d(TAG, "handleGroupChatInvitation info is null");
            return;
        }
        // Should increase the number of unread message
        Logger.d(TAG, "Has receive a group chat invitation");
        UnreadMessageManager.getInstance().changeUnreadMessageNum(
                UnreadMessageManager.MIN_STEP_UNREAD_MESSAGE_NUM,
                true);
        int size = mGroupInvitationInfos.size();
        StringBuilder notifyTitle = new StringBuilder(
                info.notifyTitle);
        String notifyContent = null;
        Intent intent = info.intent;
        if (size == SINGLE_GROUP_INVITATION) {
            notifyTitle
                    .append(context
                            .getString(R.string.group_invitation_notify_title));
            notifyTitle.append(BLANK_SPACE);
            notifyTitle.append(info.sender);
            notifyContent = info.notifyInfo;
            ParcelUuid tag = (ParcelUuid) invitation
                    .getParcelableExtra(ChatScreenActivity.KEY_CHAT_TAG);
            intent.putExtra(ChatScreenActivity.KEY_CHAT_TAG, tag);
            intent.setClass(context, InvitationDialog.class);
            if (Logger.getIsIntegrationMode()) {
                intent.putExtra(
                        InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
                String contact = invitation
                        .getStringExtra(PluginGroupChatWindow.GROUP_CHAT_CONTACT);
                if (null != contact) {
                    intent.putExtra(
                            PluginGroupChatWindow.GROUP_CHAT_CONTACT,
                            contact);
                } else {
                    Logger.w(TAG,
                            "nonAutoAcceptGroupChat() contact is null");
                }
            } else {
                intent.putExtra(InvitationDialog.KEY_STRATEGY,
                        InvitationDialog.STRATEGY_GROUP_INVITATION);
            }
        } else {
            notifyTitle.append(size);
            notifyTitle.append(BLANK_SPACE);
            notifyTitle
                    .append(context
                            .getString(R.string.group_multi_invitation_title));
            notifyContent = context
                    .getString(R.string.group_multi_invitation_content);
            if (Logger.getIsIntegrationMode()) {
                Logger.d(TAG, "nonAutoAcceptGroupChat start mms");
                intent.setClassName(
                        PluginGroupChatWindow.MMS_PACKAGE_STRING,
                        PluginGroupChatWindow.MMS_BOOT_ACTIVITY_STRING);
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            } else {
                intent.setClass(context, ChatMainActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP
                        | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            }
        }
        intent.putExtra(NOTIFY_TITLE, notifyTitle.toString());
        intent.putExtra(NOTIFY_CONTENT, notifyContent);
        PendingIntent contentIntent = PendingIntent
                .getActivity(context, 0, intent,
                        PendingIntent.FLAG_UPDATE_CURRENT);
        Notification.Builder builder = new Notification.Builder(
                context);
        builder.setContentTitle(notifyTitle);
        builder.setContentIntent(contentIntent);
        builder.setSmallIcon(info.icon);
        builder.setContentText(notifyContent);
        builder.setWhen(System.currentTimeMillis());
        builder.setAutoCancel(false);
        Notification notification = builder.getNotification();
        Logger.v(
                TAG,
                "handleGroupChatInvitation SettingsFragment." +
                "IS_NOTIFICATION_CHECKED.get() is "
                        + SettingsFragment.IS_NOTIFICATION_CHECKED
                                .get());
        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.d(TAG,
                    "handleGroupChatInvitation notification is built, with size: "
                            + size);
            String ringtone = RcsSettings.getInstance()
                    .getChatInvitationRingtone();
            if (ringtone != null && ringtone.length() != 0) {
                notification.sound = Uri.parse(ringtone);
            }
            if (RcsSettings.getInstance()
                    .isPhoneVibrateForChatInvitation()) {
                notification.defaults |= Notification.DEFAULT_VIBRATE;
            }
            NotificationManager groupInviteNotification = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            groupInviteNotification.notify(GROUP_INVITATION_TAG,
                    NOTIFICATION_ID_CHAT, notification);
        } else {
            Logger.v(
                    TAG,
                    "handleGroupChatInvitation SettingsFragment." +
                    "IS_NOTIFICATION_CHECKED.get() is false ");
        }
        Logger.v(TAG, "handleGroupChatInvitation exit");
    }
    /**
     * Builds the notification info.
     *
     * @param context the context
     * @param invitation the invitation
     * @return the group invitation info
     */
    private GroupInvitationInfo buildNotificationInfo(
            Context context, Intent invitation) {
        Logger.v(TAG, "buildNotificationInfo entry");
        if (invitation == null) {
            Logger.d(TAG,
                    "buildNotificationInfo current invitation is null");
            return null;
        }
        StringBuffer notifyTitle;
        StringBuffer notifyInfo;
        // Notification information
        String sender;
        try {
            GroupChat chatSession = getGroupChatSession(invitation);
            if (chatSession == null) {
                Logger.d(TAG,
                        "buildNotificationInfo current chat session is null");
                return null;
            }
            ArrayList<ChatMessage> messages = new ArrayList<ChatMessage>();
            messages.add((ChatMessage) invitation
                    .getParcelableExtra(FIRST_MESSAGE));
            invitation.putParcelableArrayListExtra(MESSAGES, messages);
            notifyTitle = new StringBuffer();
            notifyInfo = new StringBuffer();
            sender = chatSession.getRemoteContact().toString();
            invitation.putExtra(GroupChatIntent.EXTRA_CONTACT, sender);
            sender = ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(sender);
            List<String> participants = null;
            /*participants = new ArrayList<String>(
                    chatSession.getAllParticipants());*/
            participants = convertIdSetToList(chatSession
                    .getParticipants().keySet());
            if (participants != null) {
                Logger.e(TAG,
                        "buildNotificationInfo getAllParticipants"
                                + participants);
            }
            if (participants == null) {
                Logger.i(TAG,
                        "buildNotificationInfo paticipants list is null");
                return null;
            } else {
                int count = participants.size();
                if (count >= GROUP_PARTICIPANT_SIZE_TWO) {
                    String notify = null;
                    String contact = participants.get(0);
                    if (Utils.isANumber(contact)) {
                        contact = ContactsListManager.getInstance()
                                .getDisplayNameByPhoneNumber(contact);
                    }
                    if(contact.equals(sender)){
                        contact = participants.get(1);
                        if (Utils.isANumber(contact)) {
                            contact = ContactsListManager.getInstance()
                                    .getDisplayNameByPhoneNumber(contact);
                        }
                    }
                    if (count >= GROUP_PARTICIPANT_SIZE_THREE) {
                        notify = context
                                .getString(
                                        R.string.notify_invitation_multi_participants,
                                        sender, contact, count - 1);
                    } else {
                        notify = context
                                .getString(
                                        R.string.notify_invitation_two_participants,
                                        sender, contact);
                    }
                    notifyInfo.append(notify);
                } else {
                    Logger.i(TAG,
                            "buildNotificationInfo paticipants list is invalid");
                    return null;
                }
            }
            invitation.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            invitation
                    .putExtra(NOTIFY_INFORMATION, notifyInfo.toString());
            invitation.putExtra(FORCE_SCROLLTO_CHAT, true);
            GroupInvitationInfo info = new GroupInvitationInfo();
            info.context = context;
            info.sender = sender;
            info.icon = R.drawable.rcs_notify_chat_message;
            info.notifyInfo = notifyInfo.toString();
            info.notifyTitle = notifyTitle.toString();
            info.intent = invitation;
            addGroupInvite(chatSession, info);
            Logger.v(TAG, "buildNotificationInfo exit");
            return info;
        } catch (Exception e) {
            Logger.e(TAG,
                    "buildNotificationInfo getInivtedParticipants fail");
            e.printStackTrace();
            return null;
        }
        
    }
    
    private List<String> convertIdSetToList(Set<ContactId> contacts) {
        // create an iterator
        Iterator iterator = contacts.iterator();
        List<String> pList = new ArrayList<String>();

        // check values
        while (iterator.hasNext()) {
            pList.add(iterator.next().toString());
        }
        return pList;
    }
    
    /**
     * Update group invitation notification.
     */
    private void updateGroupInvitationNotification() {
        Set<String> keys = mGroupInvitationInfos.keySet();
        int size = keys.size();
        Logger.d(TAG,
                "updateGroupInvitationNotification entry, with size: "
                        + size);
        if (size == SINGLE_GROUP_INVITATION) {
            String sessionId = keys.iterator().next();
            GroupInvitationInfo info = mGroupInvitationInfos
                    .get(sessionId);
            if (info == null) {
                Logger.v(TAG,
                        "updateGroupInvitationNotification info is null");
                return;
            }
            Context context = info.context;
            StringBuilder titleBuilder = new StringBuilder(
                    info.notifyTitle);
            titleBuilder
                    .append(context
                            .getString(R.string.group_invitation_notify_title));
            titleBuilder.append(BLANK_SPACE);
            titleBuilder.append(info.sender);
            String notifyContent = info.notifyInfo;
            Intent intent = info.intent;
            if (intent != null) {
                intent.setClass(context, InvitationDialog.class);
                if (Logger.getIsIntegrationMode()) {
                    intent.putExtra(
                            InvitationDialog.KEY_STRATEGY,
                            InvitationDialog.STRATEGY_IPMES_GROUP_INVITATION);
                } else {
                    intent.putExtra(
                            InvitationDialog.KEY_STRATEGY,
                            InvitationDialog.STRATEGY_GROUP_INVITATION);
                }
                intent.putExtra(NOTIFY_TITLE, titleBuilder.toString());
                intent.putExtra(NOTIFY_CONTENT, notifyContent);
            }
            PendingIntent contentIntent = PendingIntent.getActivity(
                    context, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Notification.Builder builder = new Notification.Builder(
                    context);
            builder.setContentTitle(titleBuilder.toString());
            builder.setContentText(notifyContent);
            builder.setContentIntent(contentIntent);
            builder.setSmallIcon(info.icon);
            builder.setWhen(System.currentTimeMillis());
            builder.setAutoCancel(false);
            Logger.v(
                    TAG,
                    "updateGroupInvitationNotification notification checked: "
                            + SettingsFragment.IS_NOTIFICATION_CHECKED
                                    .get() + " intent: " + intent);
            if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
                Notification notification = builder.getNotification();
                NotificationManager groupInviteNotification = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                groupInviteNotification.notify(GROUP_INVITATION_TAG,
                        NOTIFICATION_ID_CHAT, notification);
            }
        } else if (size > 1) {
            String sessionId = keys.iterator().next();
            GroupInvitationInfo info = mGroupInvitationInfos
                    .get(sessionId);
            if (info == null) {
                Logger.v(TAG,
                        "updateGroupInvitationNotification info is null");
                return;
            }
            Context context = info.context;
            StringBuilder titleBuilder = new StringBuilder();
            titleBuilder.append(size);
            titleBuilder.append(BLANK_SPACE);
            titleBuilder
                    .append(context
                            .getString(R.string.group_multi_invitation_title));
            String notifyTitle = titleBuilder.toString();
            String notifyContent = context
                    .getString(R.string.group_multi_invitation_content);
            Intent intent = info.intent;
            if (intent != null) {
                intent.setClass(context, ChatMainActivity.class);
                intent.putExtra(NOTIFY_CONTENT, notifyContent);
            }
            PendingIntent contentIntent = PendingIntent.getActivity(
                    context, 0, intent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Notification.Builder builder = new Notification.Builder(
                    context);
            builder.setContentTitle(notifyTitle);
            builder.setContentText(notifyContent);
            builder.setContentIntent(contentIntent);
            builder.setSmallIcon(info.icon);
            builder.setWhen(System.currentTimeMillis());
            builder.setAutoCancel(false);
            Logger.v(
                    TAG,
                    "updateGroupInvitationNotification notification checked: "
                            + SettingsFragment.IS_NOTIFICATION_CHECKED
                                    .get() + " intent: " + intent);
            if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
                Notification notification = builder.getNotification();
                NotificationManager groupInviteNotification = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                groupInviteNotification.notify(GROUP_INVITATION_TAG,
                        NOTIFICATION_ID_CHAT, notification);
            }
        } else {
            cancelGroupInviteNotification();
        }
        Logger.v(TAG, "updateGroupInvitationNotification exit");
    }
    /**
     * Handle file transfer invitation.
     *
     * @param context the context
     * @param invitation the invitation
     */
    private void handleFileTransferInvitation(final Context context,
            Intent invitation) {
        String transferId = invitation
                .getStringExtra(FileTransferIntent.EXTRA_TRANSFER_ID);
        String chatSessionId = invitation
                .getStringExtra(CHAT_SESSION_ID);
        boolean isGroup = invitation.getBooleanExtra(ISGROUPTRANSFER,
                false);
        FileTransfer session = getFileTransferSession(transferId);
        if (session == null) {
            Logger.w(TAG,
                    "handleFileTransferInvitation() session is null");
            return;
        }
        long fileSize = invitation.getLongExtra(Utils.FILE_SIZE, 0);
        long availabeSize = Utils.getFreeStorageSize();
        long maxFileSize = ApiManager.getInstance()
                .getMaxSizeforFileThransfer();
        Logger.d(TAG, "handleFileTransferInvitation() fileSize: "
                + fileSize + " availabeSize: " + availabeSize
                + " maxFileSize: " + maxFileSize);
        if (fileSize > availabeSize) {
            // check if there is enough storage.
            Handler handler = new Handler(Looper.getMainLooper());
            final String toastText;
            if (availabeSize == -1) {
                toastText = context
                        .getString(R.string.rcse_no_external_storage_for_file_transfer);
            } else {
                toastText = context
                        .getString(R.string.rcse_no_enough_storage_for_file_transfer);
            }
            handler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(context, toastText,
                            Toast.LENGTH_LONG).show();
                }
            });
            try {
                session.rejectInvitation();
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            // check if it exceeds max size.
            if (fileSize >= maxFileSize && maxFileSize != 0) {
                boolean integration = Logger.getIsIntegrationMode();
                Logger.d(TAG,
                        "handleFileTransferInvitation() integration mode: "
                                + integration);
                if (!integration) {
                    showLargeFileNofification(context, invitation,
                            session);
                }
            } else {
                showFileTransferNotification(context, invitation,
                        session);
            }
        }
    }
    /**
     * Show large file nofification.
     *
     * @param context the context
     * @param invitation the invitation
     * @param session the session
     */
    private void showLargeFileNofification(Context context,
            Intent invitation, FileTransfer session) {
        Logger.d(TAG, "showLargeFileNofification(), invitation() is "
                + invitation);
        String contact = formatCallerId(invitation);
        try {
            session.rejectInvitation();
        } catch (Exception e) {
            e.printStackTrace();
        }
        String notifyTitle = context.getString(
                R.string.file_size_notification, contact);
        Notification.Builder builder = new Notification.Builder(
                context);
        builder.setPriority(Notification.PRIORITY_HIGH);
        builder.setSmallIcon(R.drawable.rcs_notify_file_transfer)
                .setTicker(notifyTitle).setContentTitle(notifyTitle);
        Notification notification = builder.getNotification();
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        PendingIntent pendingIntent = PendingIntent.getActivity(
                context, 0, new Intent(), 0);
        notification.contentIntent = pendingIntent;
        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.v(
                    TAG,
                    "showLargeFileNofification SettingsFragment." +
                    "IS_NOTIFICATION_CHECKED.get() is "
                            + SettingsFragment.IS_NOTIFICATION_CHECKED
                                    .get());
            // Set ringtone
            String ringtone = RcsSettings.getInstance()
                    .getChatInvitationRingtone();
            if (ringtone != null && ringtone.length() != 0) {
                notification.sound = Uri.parse(ringtone);
            }
            // Set vibration
            if (RcsSettings.getInstance()
                    .isPhoneVibrateForChatInvitation()) {
                notification.defaults |= Notification.DEFAULT_VIBRATE;
            }
            Logger.i(
                    TAG,
                    "showLargeFileNofification(), the new file transfer invitation title is"
                            + notifyTitle);
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.notify(FILE_TRANSFER,
                    NOTIFICATION_ID_FILE_TRANSFER, notification);
        } else {
            Logger.d(TAG,
                    "showLargeFileNofification(), IS_NOTIFICATION_CHECKED.get() is false");
        }
    }
    /**
     * Update file tansfer notification.
     *
     * @param invitation the invitation
     */
    private void updateFileTansferNotification(Intent invitation) {
        Logger.d(TAG,
                "updateFileTansferNotification() entry, invitation is "
                        + invitation);
        int ftContactCount = mFileInvitationInfos.size();
        Intent updateIntent = null;
        Context context = ApiManager.getInstance().getContext();
        Logger.d(TAG,
                "updateFileTansferNotification() ftContactCount is "
                        + ftContactCount + " context: " + context);
        if (ftContactCount <= 0) {
            if (context != null) {
                NotificationManager fileTransferNotification =
                        (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                if (fileTransferNotification != null) {
                    fileTransferNotification.cancel(FILE_TRANSFER,
                            NOTIFICATION_ID_FILE_TRANSFER);
                }
            }
        } else {
            int ftCount = 0;
            long totalSize = 0;
            if (invitation == null) {
                Logger.d(TAG,
                        "updateFileTansferNotification(),invitation is null");
                updateIntent = new Intent();
            } else {
                updateIntent = invitation;
            }
            for (int i = 0; i < ftContactCount; i++) {
                ftCount += mFileInvitationInfos.get(i).mInviteNumber;
                totalSize += mFileInvitationInfos.get(i).mFileSize;
            }
            String notifyTitle = null;
            String notifyContent = null;
            String contactName = ContactsListManager
                    .getInstance()
                    .getDisplayNameByPhoneNumber(
                            mFileInvitationInfos
                                    .get(FILE_INVITATION_INDEX_ZERO).mContact);
            if (ftCount == 1) {
                Logger.d(TAG,
                        "updateFileTansferNotification() ftCount is "
                                + ftCount);
                String fileName = mFileInvitationInfos
                        .get(FILE_INVITATION_INDEX_ZERO).mLastFileName;
                notifyTitle = context.getString(
                        R.string.ft_notify_title, contactName);
                notifyContent = context
                        .getString(
                                R.string.ft_notify_content,
                                fileName,
                                Utils.formatFileSizeToString(
                                        mFileInvitationInfos.get(0).mFileSize,
                                        Utils.SIZE_TYPE_TOTAL_SIZE));
                String title = context
                        .getString(R.string.file_transfer_from)
                        + contactName;
                updateIntent
                        .setClass(context, InvitationDialog.class);
                updateIntent
                        .putExtra(
                                SESSION_ID,
                                mFileInvitationInfos
                                        .get(FILE_INVITATION_INDEX_ZERO).mLastSessionId);
                updateIntent
                        .putExtra(
                                InvitationDialog.KEY_STRATEGY,
                                InvitationDialog.STRATEGY_FILE_TRANSFER_INVITATION);
                updateIntent.putExtra(CONTACT, mFileInvitationInfos
                        .get(FILE_INVITATION_INDEX_ZERO).mContact);
                updateIntent.putExtra(RcsNotification.NOTIFY_CONTENT,
                        notifyContent.toString());
                updateIntent.putExtra(RcsNotification.NOTIFY_SIZE,
                        String.valueOf(totalSize));
                updateIntent.putExtra(
                        RcsNotification.NOTIFY_FILE_NAME, fileName);
                updateIntent.putExtra(RcsNotification.NOTIFY_TITLE,
                        title);
            } else if (ftCount > 1) {
                notifyContent = context.getString(
                        R.string.ft_notify_multiple_content, Utils
                                .formatFileSizeToString(totalSize,
                                        Utils.SIZE_TYPE_TOTAL_SIZE));
                if (ftContactCount > 1) {
                    notifyTitle = context
                            .getString(
                                    R.string.ft_notify_from_dif_contact_title,
                                    ftCount);
                    updateIntent.setClass(context,
                            ChatMainActivity.class);
                } else {
                    notifyTitle = context.getString(
                            R.string.ft_notify_multiple_title,
                            ftCount, contactName);
                    getChatScreen(context, updateIntent,
                            mFileInvitationInfos.get(0).mContact,
                            contactName);
                }
            }
            updateIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            PendingIntent pendingIntent = PendingIntent.getActivity(
                    context, 0, updateIntent,
                    PendingIntent.FLAG_UPDATE_CURRENT);
            Notification.Builder builder = new Notification.Builder(
                    context);
            builder.setPriority(Notification.PRIORITY_HIGH);
            builder.setSmallIcon(R.drawable.rcs_notify_file_transfer)
                    .setContentText(notifyContent)
                    .setContentTitle(notifyTitle)
                    .setContentIntent(pendingIntent);
            if (invitation == null) {
                Logger.d(
                        TAG,
                        "updateFileTansferNotification() no need" +
                        " to notify in the notification bar");
            } else {
                builder.setTicker(notifyTitle);
            }
            builder.setAutoCancel(false);
            Notification notification = builder.getNotification();
            if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()
                    && invitation != null) {
                Logger.v(
                        TAG,
                        "updateFileTansferNotification SettingsFragment." +
                        "IS_NOTIFICATION_CHECKED.get() is "
                                + SettingsFragment.IS_NOTIFICATION_CHECKED
                                        .get());
                // Set ringtone
                String ringtone = RcsSettings.getInstance()
                        .getChatInvitationRingtone();
                if (ringtone != null && ringtone.length() != 0) {
                    notification.sound = Uri.parse(ringtone);
                }
                // Set vibration
                if (RcsSettings.getInstance()
                        .isPhoneVibrateForChatInvitation()) {
                    notification.defaults |= Notification.DEFAULT_VIBRATE;
                }
            } else {
                Logger.d(
                        TAG,
                        "updateFileTansferNotification() SettingsFragment." +
                        "IS_NOTIFICATION_CHECKED.get()"
                                + SettingsFragment.IS_NOTIFICATION_CHECKED
                                        .get()
                                + " invitation is "
                                + invitation);
            }
            Logger.i(
                    TAG,
                    "updateFileTansferNotification() notify a new file transfer" +
                    " invitation title is"
                            + notifyTitle
                            + " content is "
                            + notifyContent);
            NotificationManager fileTransferNotification = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            if (!Logger.getIsIntegrationMode()) {
                fileTransferNotification.notify(FILE_TRANSFER,
                        NOTIFICATION_ID_FILE_TRANSFER, notification);
            }
        }
    }
    /**
     * Show file transfer notification.
     *
     * @param context the context
     * @param invitation the invitation
     * @param session the session
     */
    private void showFileTransferNotification(Context context,
            Intent invitation, FileTransfer session) {
        Logger.d(TAG,
                "showFileTransferNotification() entry withe invitation is "
                        + invitation);
        String chatSessionId = null;
        String chatId = null;
        String msgId = null;
        boolean isGroup = false;
        isGroup = invitation.getBooleanExtra(ISGROUPTRANSFER, false);
        String contactNum = null;//invitation.getStringExtra(CONTACT);
        String sessionId = invitation
                .getStringExtra(FileTransferIntent.EXTRA_TRANSFER_ID);
        Uri fileUri = null;
        byte[] thumbNail = null;
        if (ApiManager.getInstance() != null){
            FileTransferService filetransferService = ApiManager.getInstance().getFileTransferApi();
            if(filetransferService != null){
                try {
                    FileTransfer ft = filetransferService.getFileTransfer(sessionId);
                    contactNum = ft.getRemoteContact().toString();
                    chatId = ft.getChatId();
                    fileUri = ft.getFileIcon();
                    Logger.d(TAG, " showFileTransferNotification remote "+ contactNum + ",fileUri:" + fileUri);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.d(TAG, "fileService is null ");
            }
        } else {
            Logger.d(TAG, "ApiManager is null ");
        }
        boolean isAutoAccept = invitation.getBooleanExtra(
                AUTO_ACCEPT, false);
        /*if(fileUri != null){
            File file = new File(fileUri.getPath());
            int size = (int) file.length();
            thumbNail = new byte[size];
            try {
                BufferedInputStream buf = new BufferedInputStream(new FileInputStream(file));
                buf.read(thumbNail, 0, thumbNail.length);
                buf.close();
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }*/
        //byte[] thumbNail = invitation.getByteArrayExtra("thumbnail");
        String isHttpTransfer = invitation
                .getStringExtra(ISHTTPTRANSFER);
        chatSessionId = invitation.getStringExtra(CHAT_SESSION_ID);
        
        //chatId = invitation.getStringExtra(CHAT_ID);
        msgId = invitation.getStringExtra(FileTransferIntent.EXTRA_MSG_ID);
        Logger.d(TAG,
                "showFileTransferNotification(), isAutoAccept = "
                        + isAutoAccept + "isHttpTransfer:"
                        + isHttpTransfer);
        if (isGroup) {
            Logger.d(TAG, "IsGroupTransfer: true");
        } else {
            Logger.d(TAG, "IsGroupTransfer: false");
        }
        String fileName = null;
        long fileSize = 0;
        try {
            fileName = session.getFileName();
            fileSize = session.getFileSize();
            Logger.i(TAG,
                    "showFileTransferNotification(), the fileName is"
                            + fileName + " and the fileSize is "
                            + fileSize);
        } catch (Exception exception) {
            exception.printStackTrace();
        }
        IChatManager chatManager = ModelImpl.getInstance();
        if (chatManager.handleFileTransferInvitation(sessionId,
                isAutoAccept, isGroup, chatSessionId, chatId,msgId)) {
            Logger.d(
                    TAG,
                    "showFileTransferNotification(), chatManager "
                            + "has decided to handle the file transfer invitation itself, "
                            + "so no need to notify the user");
        } else {
            Logger.d(TAG,
                    "showFileTransferNotification(), needs to notify the user");
            if (isAutoAccept) {
                getChatScreen(context, invitation, contactNum,
                        contactNum);
                addAutoAcceptFileTransferNotification(context,
                        invitation, fileName, contactNum, fileSize);
            } else {
                int size = mFileInvitationInfos.size();
                Logger.d(TAG,
                        "showFileTransferNotification(), size of mFileInvitationInfos is"
                                + size);
                boolean exist = false;
                for (int i = 0; i < size; i++) {
                    FileInvitationInfo fileIvitationInfo = mFileInvitationInfos
                            .get(i);
                    if (fileIvitationInfo.mContact.equals(contactNum)) {
                        fileIvitationInfo.mFileSize += fileSize;
                        fileIvitationInfo.mInviteNumber++;
                        fileIvitationInfo.mLastFileName = fileName;
                        fileIvitationInfo.mLastSessionId = sessionId;
                        exist = true;
                        break;
                    }
                }
                if (!exist) {
                    FileInvitationInfo fileIvitationInfo = new FileInvitationInfo(
                            contactNum, fileSize, fileName, sessionId);
                    mFileInvitationInfos.add(fileIvitationInfo);
                } else {
                    Logger.d(
                            TAG,
                            "showFileTransferNotification the contactNum has" +
                            " exist in the mFileInvitationInfos "
                                    + contactNum);
                }
                updateFileTansferNotification(invitation);
            }
        }
    }
    
    
    /**
     * Adds the auto accept file transfer notification.
     *
     * @param context the context
     * @param invitation the invitation
     * @param fileName the file name
     * @param contact the contact
     * @param fileSize the file size
     */
    private void addAutoAcceptFileTransferNotification(
            Context context, Intent invitation, String fileName,
            String contact, long fileSize) {
        Logger.d(TAG, "addAutoAcceptFileTransferNotification() entry");
        String notifyTitle = null;
        String notifyContent = null;
        String contactName = ContactsListManager.getInstance()
                .getDisplayNameByPhoneNumber(contact);
        notifyTitle = context.getString(R.string.ft_notify_title,
                contactName);
        notifyContent = context.getString(R.string.ft_notify_content,
                fileName, Utils.formatFileSizeToString(fileSize,
                        Utils.SIZE_TYPE_TOTAL_SIZE));
        Notification.Builder builder = new Notification.Builder(
                context);
        builder.setPriority(Notification.PRIORITY_HIGH);
        builder.setSmallIcon(R.drawable.rcs_notify_file_transfer)
                .setContentText(notifyContent)
                .setContentTitle(notifyTitle);
        Notification notification = builder.getNotification();
        notification.flags |= Notification.FLAG_AUTO_CANCEL;
        PendingIntent pendingIntent = PendingIntent.getActivity(
                context, 0, invitation,
                PendingIntent.FLAG_UPDATE_CURRENT);
        notification.contentIntent = pendingIntent;
        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.v(
                    TAG,
                    "addAutoAcceptFileTransferNotification SettingsFragment." +
                    "IS_NOTIFICATION_CHECKED.get() is "
                            + SettingsFragment.IS_NOTIFICATION_CHECKED
                                    .get());
            // Set ringtone
            String ringtone = RcsSettings.getInstance()
                    .getChatInvitationRingtone();
            if (ringtone != null && ringtone.length() != 0) {
                notification.sound = Uri.parse(ringtone);
            }
            // Set vibration
            if (RcsSettings.getInstance()
                    .isPhoneVibrateForChatInvitation()) {
                notification.defaults |= Notification.DEFAULT_VIBRATE;
            }
            Logger.i(
                    TAG,
                    "addAutoAcceptFileTransferNotification(), the new file" +
                    " transfer invitation title is"
                            + notifyTitle);
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            if (!Logger.getIsIntegrationMode()) {
                notificationManager.notify(FILE_TRANSFER,
                        NOTIFICATION_ID_FILE_TRANSFER, notification);
            }
        } else {
            Logger.d(
                    TAG,
                    "addAutoAcceptFileTransferNotification(), IS_NOTIFICATION_CHECKED" +
                    ".get() is false");
        }
    }
    /**
     * Gets the chat screen.
     *
     * @param context the context
     * @param invitation the invitation
     * @param number the number
     * @param name the name
     * @return the chat screen
     */
    private void getChatScreen(Context context, Intent invitation,
            String number, String name) {
        One2OneChat chat = null;
        if (null != number) {
            Participant contact = new Participant(number, name);
            List<Participant> participantList = new ArrayList<Participant>();
            participantList.add(contact);
            chat = (One2OneChat) ModelImpl.getInstance().addChat(
                    participantList, null, null);
            if (null != chat) {
                invitation
                        .setClass(context, ChatScreenActivity.class);
                invitation.putExtra(ChatScreenActivity.KEY_CHAT_TAG,
                        (ParcelUuid) chat.getChatTag());
            } else {
                Logger.e(TAG, "getChatScreen(), chat is null!");
            }
        } else {
            Logger.e(TAG,
                    "getChatScreen(), fileSessionSession is null");
        }
    }
    /**
     * Start to handle invitation.
     *
     * @param context            The Context instance
     * @param intent            The invitation Intent instance
     */
    public static synchronized void handleInvitation(Context context,
            Intent intent) {
        Logger.d(TAG,
                "handleInvitation() entry with intend action is "
                        + intent.getAction());
        String action = intent.getAction();
        if (action == null) {
            return;
        }
        
        if (OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE.equalsIgnoreCase(action) ||
                OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_STANDALONE_CHAT_MESSAGE.equalsIgnoreCase(action)) {
            /*OneToOneChat chatSession = RcsNotification.getInstance()
                    .getChatSession(intent);
            if (chatSession == null) {
                Logger.d(TAG, "The chat session is null");
                return;
            }*/
            try {
                String contact = formatCallerId(intent);
                intent.putExtra(DISPLAY_NAME, contact);
                handleChatInvitation(context, intent);
            } catch (Exception e) {
                Logger.d(TAG, "Chat operation error");
                e.printStackTrace();
            }
        } else if (FileTransferIntent.ACTION_NEW_INVITATION
                .equalsIgnoreCase(action)) {
            getInstance().handleFileTransferInvitation(context,
                    intent);
        } else if (GroupChatIntent.ACTION_NEW_INVITATION
                .equalsIgnoreCase(action)) {
            Logger.d(TAG, "Group Chat invitation arrived");
            try {
                if (!(intent.getBooleanExtra("isGroupChatExist",
                        false))) {
                    boolean autoAccept = intent.getBooleanExtra(
                            AUTO_ACCEPT, false);
                    RcsNotification.getInstance()
                            .handleGroupChatInvitation(context,
                                    intent, autoAccept);
                }
            } catch (Exception e) {
                Logger.d(TAG, "Group Chat operation error");
                e.printStackTrace();
            }
        } else if (ImageSharingIntent.ACTION_NEW_INVITATION
                .equalsIgnoreCase(action)) {
            handleImageSharingInvitation(context, intent);
        } else if (VideoSharingIntent.ACTION_NEW_INVITATION
                .equalsIgnoreCase(action)) {
            handleVideoSharingInvitation(context, intent);
        } /*
           * else if (ChatIntent.CHAT_SESSION_REPLACED.equalsIgnoreCase(action))
           * { handleChatInvitation(context, intent); }
           */
        Logger.v(TAG, "handleInvitation() exit");
    }
    
    public static synchronized void handleNewMessage(Context context,
            Intent intent) {
        Logger.d(TAG,
                "handleNewMessage() entry with intend action is "
                        + intent.getAction());
        String action = intent.getAction();
        if (action == null) {
            return;
        }
        
        if (GroupChatIntent.ACTION_NEW_GROUP_CHAT_MESSAGE
                .equalsIgnoreCase(action)) {
            String msgId = intent.getStringExtra(GroupChatIntent.EXTRA_MESSAGE_ID);
            Logger.v(TAG, "handleNewMessage() msgid:" + msgId);
            ModelImpl.getInstance().handleNewMessage(msgId, intent);
        }
    }
    /**
     * Handle chat invitation.
     *
     * @param context the context
     * @param invitation the invitation
     */
    private static void handleChatInvitation(Context context,
            Intent invitation) {
        Logger.v(TAG, "handleChatInvitation entry");
        ArrayList<ChatMessage> messages = new ArrayList<ChatMessage>();
        ChatMessage msg = invitation
                .getParcelableExtra(FIRST_MESSAGE);
        if (msg != null) {
            messages.add(msg);
        }
        invitation.putParcelableArrayListExtra(FIRST_MESSAGE,
                messages);
        ModelImpl.getInstance()
                .handleO2OInvitation(invitation, false);
        Logger.v(TAG, "handleChatInvitation exit");
    }
    /**
     * Handle image sharing invitation.
     *
     * @param context the context
     * @param invitation the invitation
     */
    private static void handleImageSharingInvitation(Context context,
            Intent invitation) {
    }
    /**
     * Handle video sharing invitation.
     *
     * @param context the context
     * @param invitation the invitation
     */
    private static void handleVideoSharingInvitation(Context context,
            Intent invitation) {
    }
    /**
     * On receive message in background.
     *
     * @param context the context
     * @param newTag the new tag
     * @param message the message
     * @param participantList the participant list
     * @param id the id
     */
    public void onReceiveMessageInBackground(Context context,
            Object newTag, ChatMessage message,
            List<Participant> participantList, final int id) {
        onReceiveMessageInBackground(context, newTag, message,
                participantList, id, false);
    }
    /**
     * Showing a notifications of a new incoming message when the chat window is
     * in background.
     *
     * @param context            The context
     * @param newTag the new tag
     * @param message the message
     * @param participantList the participant list
     * @param id the id
     * @param isStoreAndFoward the is store and foward
     */
    public void onReceiveMessageInBackground(Context context,
            Object newTag, ChatMessage message,
            List<Participant> participantList, final int id,
            boolean isStoreAndFoward) {
        Logger.d(TAG,
                "onReceiveMessageInBackground(), the newTag is "
                        + newTag + " isStoreAndFoward "
                        + isStoreAndFoward + "messageid is "
                        + message.getId());
        if (mIsInChatMainActivity) {
            Logger.e(TAG,
                    "onReceiveMessageInBackground() mIsInChatMainActivity is true)");
            return;
        }
        String ticker = null;
        String contact = getRemoteContact(message);
        if (null != contact) {
            if (Utils.isANumber(contact)) {
                contact = ContactsListManager.getInstance()
                        .getDisplayNameByPhoneNumber(contact);
            } else {
                Logger.d(TAG,
                        "onReceiveMessageInBackground(), contact is null");
            }
        }
        if (contact != null) {
            ticker = contact + ":" + message.getContent();
        }
        String notifyTitle = contact;
        if (participantList != null && participantList.size() > 1) {
            notifyTitle = getGroupNotificationTitle(participantList);
        }
        String description = message.getContent();
        if (mUnReadMessagesChatInfos.containsKey(newTag)) {
            ChatReceivedInfo chatInfo = mUnReadMessagesChatInfos
                    .get(newTag);
            if (chatInfo != null) {
                chatInfo.updateMessage(notifyTitle);
            }
        } else {
            mUnReadMessagesChatInfos.put(newTag,
                    new ChatReceivedInfo(notifyTitle, description));
        }
        if (!isStoreAndFoward) {
            Logger.d(TAG,
                    "onReceiveMessageInBackground(), isStoreAndFoward is "
                            + isStoreAndFoward);
            updateReceiveMessageNotification(ticker, true);
        } else if (!RcsNotification.getInstance().mIsStoreAndForwardMessageNotified) {
            Logger.d(TAG, "onReceiveMessageInBackground(), "
                    + "mIsStoreAndForwardMessageNotified is false ");
            updateReceiveMessageNotification(ticker, true);
            RcsNotification.getInstance().mIsStoreAndForwardMessageNotified = true;
        } else {
            Logger.d(TAG, "onReceiveMessageInBackground(), "
                    + "mIsStoreAndForwardMessageNotified is true ");
            updateReceiveMessageNotification(ticker, false);
        }
    }
    /**
     * Update receive message notification.
     *
     * @param context the context
     * @param requestCode the request code
     * @param intent the intent
     * @param ticker the ticker
     * @param notifyTitle the notify title
     * @param description the description
     * @param icon the icon
     * @param isNewMessageNotification the is new message notification
     */
    private void updateReceiveMessageNotification(Context context,
            int requestCode, Intent intent, String ticker,
            String notifyTitle, String description, int icon,
            boolean isNewMessageNotification) {
        Logger.d(
                TAG,
                "updateReceiveMessageNotification() entry eight parameters" +
                " isNewMessageNotification "
                        + isNewMessageNotification);
        PendingIntent contentIntent = PendingIntent.getActivity(
                context, requestCode, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        Notification.Builder builder = new Notification.Builder(
                context);
        builder.setContentTitle(notifyTitle);
        builder.setContentText(description);
        builder.setContentIntent(contentIntent);
        if (isNewMessageNotification) {
            builder.setTicker(ticker);
        }
        builder.setSmallIcon(icon);
        builder.setWhen(System.currentTimeMillis());
        builder.setAutoCancel(true);
        if (SettingsFragment.IS_NOTIFICATION_CHECKED.get()) {
            Logger.v(
                    TAG,
                    "updateReceiveMessageNotification SettingsFragment." +
                    "IS_NOTIFICATION_CHECKED.get() is "
                            + SettingsFragment.IS_NOTIFICATION_CHECKED
                                    .get());
            if (isNewMessageNotification) {
                Logger.v(TAG,
                        "updateReceiveMessageNotification isNewMessageNotification is "
                                + isNewMessageNotification);
                // Set ringtone
                String ringtone = RcsSettings.getInstance()
                        .getChatInvitationRingtone();
                if (ringtone != null && ringtone.length() != 0) {
                    Logger.v(TAG,
                            "updateReceiveMessageNotification set rintone");
                    builder.setSound(Uri.parse(ringtone));
                } else {
                    Logger.v(TAG,
                            "updateReceiveMessageNotification not set rintone");
                }
                // Set vibrate
                if (RcsSettings.getInstance()
                        .isPhoneVibrateForChatInvitation()) {
                    Logger.v(TAG,
                            "updateReceiveMessageNotification set vibarate");
                    builder.setDefaults(Notification.DEFAULT_VIBRATE);
                } else {
                    Logger.v(TAG,
                            "updateReceiveMessageNotification not set vibarate");
                }
            } else {
                Logger.v(TAG,
                        "updateReceiveMessageNotification isNewMessageNotification is "
                                + isNewMessageNotification);
            }
            Notification notification = builder.getNotification();
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.notify(UNREAD_MESSAGE,
                    NOTIFICATION_ID_UNREAD_MESSAGE, notification);
        } else {
            Logger.v(
                    TAG,
                    "updateReceiveMessageNotification SettingsFragment." +
                    "IS_NOTIFICATION_CHECKED.get() is false");
        }
    }
    /**
     * Update receive message notification.
     *
     * @param ticker the ticker
     * @param isNewMessageNotification the is new message notification
     */
    private void updateReceiveMessageNotification(String ticker,
            boolean isNewMessageNotification) {
        Logger.d(
                TAG,
                "updateReceiveMessageNotification() two parameters entry" +
                " isNewMessageNotification is "
                        + isNewMessageNotification);
        Context context = ApiManager.getInstance().getContext();
        if (context != null) {
            if (mUnReadMessagesChatInfos.size() == 0) {
                Logger.d(TAG,
                        "updateReceiveMessageNotification() the size is 0");
                NotificationManager notificationManager = (NotificationManager) context
                        .getSystemService(Context.NOTIFICATION_SERVICE);
                if (notificationManager != null) {
                    notificationManager.cancel(UNREAD_MESSAGE,
                            NOTIFICATION_ID_UNREAD_MESSAGE);
                }
            } else {
                String notifyTitle = null;
                String description = null;
                Intent intent = new Intent();
                intent.setClass(context, ChatScreenActivity.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
                if (mUnReadMessagesChatInfos.size() > 1) {
                    Logger.d(TAG,
                            "updateReceiveMessageNotification() the size >1");
                    intent.setClass(context, ChatMainActivity.class);
                    intent.putExtra(FORCE_SCROLLTO_CHAT, true);
                    notifyTitle = context
                            .getString(
                                    R.string.notification_multipleChats_title,
                                    Integer.toString(mUnReadMessagesChatInfos
                                            .size()));
                    int count = 0;
                    Collection<ChatReceivedInfo> chatInfos = mUnReadMessagesChatInfos
                            .values();
                    Iterator<ChatReceivedInfo> iterator = chatInfos
                            .iterator();
                    while (iterator.hasNext()) {
                        ChatReceivedInfo chatInfo = iterator.next();
                        count += chatInfo.getMessageNum();
                    }
                    if (count > 1) {
                        description = context.getString(
                                R.string.notification_multiple,
                                Integer.toString(count));
                    }
                } else if (mUnReadMessagesChatInfos.size() == 1) {
                    Logger.d(TAG,
                            "updateReceiveMessageNotification() the size = 1");
                    Object tag = mUnReadMessagesChatInfos.keys()
                            .nextElement();
                    if (tag == null) {
                        Logger.e(TAG,
                                "updateReceiveMessageNotification the chat tag is null");
                        return;
                    }
                    if (tag instanceof UUID) {
                        ParcelUuid parcelUuid = new ParcelUuid(
                                (UUID) tag);
                        intent.putExtra(
                                ChatScreenActivity.KEY_CHAT_TAG,
                                parcelUuid);
                    } else {
                        intent.putExtra(
                                ChatScreenActivity.KEY_CHAT_TAG,
                                (ParcelUuid) tag);
                    }
                    ChatReceivedInfo chatInfo = mUnReadMessagesChatInfos
                            .get(tag);
                    if (chatInfo == null) {
                        Logger.e(TAG,
                                "updateReceiveMessageNotification return chatInfo is null");
                        return;
                    }
                    notifyTitle = chatInfo.getDisplayName();
                    description = chatInfo.getFirstMessage();
                    int count = chatInfo.getMessageNum();
                    if (count > 1) {
                        description = context.getString(
                                R.string.notification_multiple,
                                Integer.toString(count));
                    }
                }
                int requestCode = 0;
                int icon = R.drawable.rcs_notify_chat_message;
                Logger.d(TAG,
                        "updateReceiveMessageNotification() mode: "
                                + Logger.getIsIntegrationMode());
                if (!Logger.getIsIntegrationMode()) {
                    updateReceiveMessageNotification(context,
                            requestCode, intent, ticker, notifyTitle,
                            description, icon,
                            isNewMessageNotification);
                }
            }
        } else {
            Logger.d(TAG,
                    "updateReceiveMessageNotification() the context is null");
        }
    }
    /**
     * cancel one message notification when user open this chat window(one2one
     * or group chat).
     *
     * @param newTag the new tag
     */
    public void cancelReceiveMessageNotification(Object newTag) {
        if (newTag == null) {
            Logger.d(TAG,
                    "cancelReceiveMessageNotification chat tag is null");
            return;
        }
        if (mUnReadMessagesChatInfos != null
                && mUnReadMessagesChatInfos.containsKey(newTag)) {
            mUnReadMessagesChatInfos.remove(newTag);
            String ticker = null;
            boolean isNewMessageNotification = false;
            updateReceiveMessageNotification(ticker,
                    isNewMessageNotification);
        } else {
            Logger.d(
                    TAG,
                    "cancelReceiveMessageNotification mUnReadMessagesChatInfos" +
                    " didn't contain this chat tag");
        }
    }
    /**
     * Cancel a previously shown notification with a special notification ID. If
     * it's transient, the view will be hidden. If it's persistent, it will be
     * removed from the status bar.
     */
    public void cancelNotification() {
        Context context = ApiManager.getInstance().getContext();
        if (context != null) {
            NotificationManager notificationManager = (NotificationManager) context
                    .getSystemService(Context.NOTIFICATION_SERVICE);
            if (notificationManager != null) {
                Logger.d(TAG, "cancelNotification() entry");
                notificationManager.cancel(UNREAD_MESSAGE,
                        NOTIFICATION_ID_UNREAD_MESSAGE);
                notificationManager.cancel(FILE_TRANSFER,
                        NOTIFICATION_ID_FILE_TRANSFER);
            } else {
                Logger.e(TAG,
                        "cancelNotification the notificationManager is null");
            }
        } else {
            Logger.e(TAG,
                    "cancelGroupInviteNotification the context is null");
        }
        mUnReadMessagesChatInfos.clear();
        mFileInvitationInfos.clear();
        Logger.d(TAG, "cancelNotification() exit");
    }
    /**
     * Gets the group notification title.
     *
     * @param participantList the participant list
     * @return the group notification title
     */
    private String getGroupNotificationTitle(
            List<Participant> participantList) {
        String notifyTitle = ChatFragment
                .getParticipantsName(participantList
                        .toArray(new Participant[1]));
        char[] chars = notifyTitle.toCharArray();
        int length = 0;
        int index = 0;
        for (int i = 0; i < chars.length; i++) {
            char c = chars[i];
            if (c > NOTIFICATION_CHAR_LENGTH_CONDITION) {
                length += NOTIFICATION_CHAR_LENGTH_DOUBLE_BYTE;
            } else {
                length += NOTIFICATION_CHAR_LENGTH_SINGLE_BYTE;
            }
            if (length >= NOTIFICATION_TITLE_LENGTH_MAX) {
                Logger.d(TAG,
                        "getGroupNotificationTitle(), length > maxlenght" +
                        " will cut the string");
                index = i;
                break;
            }
        }
        if (index > 0) {
            notifyTitle = notifyTitle.substring(0, index) + "...("
                    + participantList.size() + ")";
        } else {
            notifyTitle = notifyTitle + " (" + participantList.size()
                    + ")";
        }
        return notifyTitle;
    }
    /**
     * Format caller id.
     *
     * @param invitation the invitation
     * @return the string
     */
    private static String formatCallerId(Intent invitation) {
        //String number = invitation.getStringExtra(CONTACT);
        String msgId = invitation
                .getStringExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID);
        String number = null;
        String name = null;
        Logger.d(TAG, "formatCallerId() msgid:" + msgId);
        if (ApiManager.getInstance() != null){
            ChatService chatService = ApiManager.getInstance().getChatApi();
            if(chatService != null){
                try {
                    com.gsma.services.rcs.chat.ChatMessage msg = chatService.getChatMessage(msgId);
                    number = msg.getRemoteContact().toString();
                    Logger.d(TAG, " formatCallerId number "+ number);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else {
                Logger.d(TAG, "chatService is null ");
            }
        } else {
            Logger.d(TAG, "ApiManager is null ");
        }
        Logger.d(TAG, "formatCallerId, number is " + number);
        String displayName;
        if(ContactsListManager.getInstance() == null) {
            Logger.d(TAG,
                    "formatCallerId() ContactsListManager is null ");
            ContactsListManager.initialize(MediatekFactory
                    .getApplicationContext());
        }
        if (null != number && ContactsListManager.getInstance() != null) {
            String tmpContact = ContactsListManager.getInstance()
                    .getDisplayNameByPhoneNumber(number);
            if (tmpContact != null) {
                displayName = tmpContact;
            } else {
                displayName = number;
            }
        } else {
            displayName = EMPTY_STRING;
            Logger.e(TAG, "formatCallerId, number is null!");
        }
        Logger.d(TAG, "formatCallerId, displayName is " + displayName);
        return displayName;
    }
    /**
     * Gets the remote contact.
     *
     * @param message the message
     * @return the remote contact
     */
    private static String getRemoteContact(ChatMessage message) {
        Logger.d(TAG, "formatRemote() entry, message.getRemote() is "
                + message.getRemoteContact().toString());
        String remote = message.getRemoteContact().toString();
        String displayName = Utils
                .extractDisplayNameFromUri(remote);
        if ((displayName != null) && (displayName.length() > 0)) {
            return displayName;
        }
        String number = Utils.extractNumberFromUri(remote);
        return number;
    }

    /**
     * The Class GroupInvitationInfo.
     */
    public static class GroupInvitationInfo {
        /**
         * The context.
         */
        public Context context;
        /**
         * The sender.
         */
        public String sender;
        /**
         * The icon.
         */
        public int icon;
        /**
         * The notify title.
         */
        public String notifyTitle;
        /**
         * The notify info.
         */
        public String notifyInfo;
        /**
         * The intent.
         */
        public Intent intent;
    }

    /**
     * The Class FileInvitationInfo.
     */
    public static class FileInvitationInfo {
        /**
         * The m contact.
         */
        String mContact;
        /**
         * The m last session id.
         */
        String mLastSessionId;
        /**
         * The m invite number.
         */
        int mInviteNumber = 1;
        /**
         * The m file size.
         */
        long mFileSize = 0;
        /**
         * The m last file name.
         */
        String mLastFileName;

        /**
         * Instantiates a new file invitation info.
         *
         * @param contact the contact
         * @param filesize the filesize
         * @param filename the filename
         * @param sessionId the session id
         */
        public FileInvitationInfo(String contact, long filesize,
                String filename, String sessionId) {
            mContact = contact;
            mFileSize = filesize;
            mLastFileName = filename;
            mLastSessionId = sessionId;
        }
    }

    /**
     * The Class ChatReceivedInfo.
     */
    private static class ChatReceivedInfo {
        /**
         * The m display name.
         */
        private String mDisplayName;
        /**
         * The m first message.
         */
        private String mFirstMessage;
        /**
         * The m message num.
         */
        private int mMessageNum;

        /**
         * Instantiates a new chat received info.
         *
         * @param displayName the display name
         * @param firstMessage the first message
         */
        public ChatReceivedInfo(String displayName,
                String firstMessage) {
            this.mDisplayName = displayName;
            this.mFirstMessage = firstMessage;
            this.mMessageNum = 1;
        }
        /**
         * Gets the display name.
         *
         * @return the display name
         */
        public String getDisplayName() {
            return mDisplayName;
        }
        /**
         * Gets the first message.
         *
         * @return the first message
         */
        public String getFirstMessage() {
            return mFirstMessage;
        }
        /**
         * Gets the message num.
         *
         * @return the message num
         */
        public int getMessageNum() {
            return mMessageNum;
        }
        /**
         * Update message.
         *
         * @param displayName the display name
         */
        public void updateMessage(String displayName) {
            this.mDisplayName = displayName;
            this.mMessageNum++;
        }
    }
}
