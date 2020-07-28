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
package com.mediatek.rcse.plugin.message;

import java.util.ArrayList;
import java.util.List;

import android.content.ContentValues;
import android.content.Intent;
import android.database.SQLException;
import android.net.Uri;
import android.os.RemoteException;

import com.mediatek.rcse.activities.InvitationDialog;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage.Status;
import com.mediatek.rcse.mvc.ChatMessage;
import com.mediatek.rcse.mvc.ModelImpl;
import com.mediatek.rcse.mvc.One2OneChat;
import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.RcsNotification;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;
import com.mediatek.rcse.service.binder.ThreadTranslater;

import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;

//import com.gsma.services.rcs.chat.ChatMessage;

/**
 * Plugin Sent ChatWindow message.
 */
public class PluginSentChatMessage extends IRemoteSentChatMessage.Stub {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "PluginSentChatMessage";
    /**
     * The Constant ACTION_MESSAGE_STATUS.
     */
    public static final String ACTION_MESSAGE_STATUS = "com.mediatek.mms.ipmessage.messageStatus";
    /**
     * The Constant STATUS.
     */
    public static final String STATUS = "status";
    /**
     * The Constant IP_MESSAGE_ID.
     */
    public static final String IP_MESSAGE_ID = "com.mediatek.mms.ipmessage.IpMessageRecdId";
    /**
     * The m date.
     */
    private long mDate;
    /**
     * The m status.
     */
    private Status mStatus;
    /**
     * The m message tag.
     */
    private int mMessageTag = -1;
    /**
     * The newgroup invite.
     */
    private boolean mNewgroupInvite = false;

    /**
     * Checks if is newgroup invite.
     *
     * @return true, if is newgroup invite
     */
    public boolean isNewgroupInvite() {
        return mNewgroupInvite;
    }
    /**
     * Sets the newgroup invite.
     *
     * @param newgroupInvite the new newgroup invite
     */
    public void setNewgroupInvite(boolean newgroupInvite) {
        this.mNewgroupInvite = newgroupInvite;
    }

    /**
     * The m plugin chat window message.
     */
    private final PluginChatWindowMessage mPluginChatWindowMessage;
    /**
     * The m message manager.
     */
    private IpMessageManager mMessageManager;

    /**
     * Instantiates a new plugin sent chat message.
     *
     * @param messageManager the message manager
     * @param message the message
     * @param messageTag the message tag
     */
    public PluginSentChatMessage(IpMessageManager messageManager,
            ChatMessage message, int messageTag) {
        mPluginChatWindowMessage = new PluginChatWindowMessage(message);
        mMessageManager = messageManager;
        Logger.d(TAG, "PluginSentChatMessage(), messageManager: "
                + messageManager + "message = " + message + " ,messageTag: "
                + messageTag);
        Long messageIdInMms = null;
        mMessageTag = messageTag;
        if (-1 != messageTag
                && PluginUtils.getIdInMmsDb(messageTag) != -1) {
            messageIdInMms = PluginUtils.getIdInMmsDb(messageTag);
            Logger.d(TAG,
                    "PluginSentChatMessage() this is a present message, we'll update it."
                            + " messageIdInMms: " + messageIdInMms);
            Logger.v(TAG, "updateStatus(), status = DELIVERED");
            if (mMessageManager != null) {
                mMessageManager.removePresentMessage(mMessageTag);
            }
            PluginUtils
                    .updatePreSentMessageInMmsDb(message.getId(), messageTag);
        } else {
            messageIdInMms = PluginUtils.storeMessageInDatabase(
                    message.getId(), message.getContent(),
                    message.getRemoteContact().toString(), PluginUtils.OUTBOX_MESSAGE, 0, messageTag);
        }
        mPluginChatWindowMessage.storeInCache(messageIdInMms);
    }
    /**
     * Insert thread id in db.
     *
     * @param threadId the thread id
     * @param groupSubject the group subject
     */
    private void insertThreadIDInDB(long threadId, String groupSubject) {
        ContentValues values = new ContentValues();
        values.put(IntegratedMessagingData.KEY_INTEGRATED_MODE_THREAD_ID,
                threadId);
        values.put(IntegratedMessagingData.KEY_INTEGRATED_MODE_GROUP_SUBJECT,
                groupSubject);
        try {
            Uri uri = MediatekFactory
                    .getApplicationContext()
                    .getContentResolver()
                    .insert(IntegratedMessagingData.CONTENT_URI_INTEGRATED,
                            values);
        } catch (SQLException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /**
     * Instantiates a new plugin sent chat message.
     *
     * @param messageManager the message manager
     * @param message the message
     * @param messageTag the message tag
     * @param groupSubject the group subject
     */
    public PluginSentChatMessage(IpMessageManager messageManager,
            ChatMessage message, int messageTag, String groupSubject) {
        mPluginChatWindowMessage = new PluginChatWindowMessage(message);
        mMessageManager = messageManager;
        Logger.d(TAG, "PluginSentChatMessage(), messageManager: "
                + messageManager + "message = " + message + " ,messageTag: "
                + messageTag);
        Long messageIdInMms = null;
        if (-1 != messageTag && PluginUtils.getIdInMmsDb(messageTag) != -1
                && mMessageManager.removePresentMessage(messageTag)) {
            messageIdInMms = PluginUtils.getIdInMmsDb(messageTag);
            Logger.d(TAG,
                    "PluginSentChatMessage() this is a present message, we'll update it."
                            + " messageIdInMms: " + messageIdInMms);
            PluginUtils
                    .updatePreSentMessageInMmsDb(message.getId(), messageTag);
        } else {
            messageIdInMms = PluginUtils.storeMessageInDatabase(
                    message.getId(), message.getContent(),
                    message.getRemoteContact().toString(), PluginUtils.OUTBOX_MESSAGE, 0,messageTag);
            if (ThreadTranslater.tagExistInCache(message.getRemoteContact().toString())) {
                Logger.d(TAG,
                        "addSentMessage() Tag exists" + message.getRemoteContact().toString());
                Long thread = ThreadTranslater.translateTag(message
                        .getRemoteContact().toString());
                insertThreadIDInDB(thread, groupSubject);
            }
        }
        mPluginChatWindowMessage.storeInCache(messageIdInMms);
    }
    /**
     * Gets the plugin chat window message.
     *
     * @return the plugin chat window message
     */
    public PluginChatWindowMessage getPluginChatWindowMessage() {
        return mPluginChatWindowMessage;
    }
    /**
     * Update date.
     *
     * @param date the date
     * @throws RemoteException the remote exception
     */
    @Override
    public void updateDate(long date) throws RemoteException {
        Logger.v(TAG, "updateDate(), date = " + date);
        mDate = date;
    }
    /**
     * Update status.
     *
     * @param status the status
     * @throws RemoteException the remote exception
     */
    @Override
    public void updateStatus(String status) throws RemoteException {
        Logger.v(TAG, "updateStatus(), status = " + status);
        String messageId = mPluginChatWindowMessage.getChatMessage().getId();
        Long messageIdInMms = IpMessageManager.getMessageId(messageId);
        mStatus = Status.valueOf(status);
        IpMessage ipMessage = IpMessageManager.getMessage(messageIdInMms);
        if (ipMessage == null) {
            return;
        }
        if (ipMessage instanceof PluginIpTextMessage) {
            Logger.w(TAG, "updateStatus() setStatus to " + mStatus);
            ((PluginIpTextMessage) ipMessage).setStatus(mStatus);
        } else {
            Logger.w(TAG, "updateStatus() ipMessage is " + ipMessage);
        }
        Intent it = new Intent();
        it.setAction(ACTION_MESSAGE_STATUS);
        it.putExtra(STATUS, mStatus.ordinal());
        it.putExtra(IP_MESSAGE_ID, messageIdInMms);
        IpNotificationsManager.notify(it);
        if ((PluginUtils.getMessagingMode() == 1) && (mStatus == Status.FAILED)) {
            Logger.v(TAG, "updateStatus(), status failed = " + status);
            if(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()) {
               Logger.v(TAG, "OP08/OP07 Operator, Don't Confirm User About MMS, Just Fallback");
               Intent intent = new Intent(ApiService.MMS_FALLBACK_ACTION);
               intent.putExtra(RcsNotification.CONTACT, ipMessage.getFrom());               
               PluginUtils.saveThreadandTag(1, ipMessage.getFrom());
               intent.putExtra("send_by_sms_text",
                       ((PluginIpTextMessage) ipMessage).getBody());               
               MediatekFactory.getApplicationContext().sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
               return;
            }
            Intent intent = new Intent(InvitationDialog.ACTION);
            intent.putExtra(RcsNotification.CONTACT, ipMessage.getFrom());
            intent.putExtra(InvitationDialog.KEY_STRATEGY,
                    InvitationDialog.STRATEGY_IPMES_SEND_BY_SMS);
            PluginUtils.saveThreadandTag(1, ipMessage.getFrom());
            intent.putExtra("send_by_sms_text",
                    ((PluginIpTextMessage) ipMessage).getBody());
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK
                    | Intent.FLAG_ACTIVITY_NEW_TASK);
            MediatekFactory.getApplicationContext().startActivity(intent);
        }
    }
    
    public void updateMessage(ChatMessage message) {
        Logger.v(TAG, "updateMessage(), messageID = " + message.getId() + "&messagetag=" + mMessageTag);
        PluginUtils.updatePreSentMessageInMmsDb(message.getId(), mMessageTag);
    }
    
    /**
     * Gets the date.
     *
     * @return the date
     */
    public long getDate() {
        Logger.v(TAG, "getDate(), mDate = " + mDate);
        return mDate;
    }
    /**
     * Gets the status.
     *
     * @return the status
     */
    public Status getStatus() {
        Logger.v(TAG, "getStatus(), mStatus = " + mStatus);
        return mStatus;
    }
    /**
     * Gets the id.
     *
     * @return the id
     * @throws RemoteException the remote exception
     */
    @Override
    public String getId() throws RemoteException {
        // TODO Auto-generated method stub
        return null;
    }
}