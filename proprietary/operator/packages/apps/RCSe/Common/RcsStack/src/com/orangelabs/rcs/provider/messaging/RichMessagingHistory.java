/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 ******************************************************************************/

package com.orangelabs.rcs.provider.messaging;

import java.io.File;
import java.io.ByteArrayOutputStream;
import java.io.ObjectOutputStream;
import java.security.spec.EncodedKeySpec;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.List;


import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.provider.messaging.GroupDeliveryInfoLog;
import com.orangelabs.rcs.provider.messaging.GroupDeliveryInfoData;
import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.ChatLog.Message.Type;
import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.ft.FileTransferLog;
import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.chat.ChatMessage;
import com.gsma.services.rcs.RcsService.ReadStatus;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.GroupChat.State;
import com.gsma.services.rcs.chat.GroupChat.ReasonCode;
import com.gsma.services.rcs.chat.ChatLog.GroupChatEvent;
import com.gsma.services.rcs.chat.ChatLog.Message.MimeType;
//import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.Status;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;
import android.util.SparseArray;


import com.orangelabs.rcs.provider.messaging.MessageData;
//import com.orangelabs.rcs.provider.messaging.ContactId;
import com.orangelabs.rcs.provider.messaging.FileTransferData;
//import com.orangelabs.rcs.provider.messaging.ReasonCode;
//import com.orangelabs.rcs.provider.messaging.State;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocPush;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatInfo;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.service.api.PauseResumeFileObject;
import com.orangelabs.rcs.service.api.ServerApiPersistentStorageException;
import com.orangelabs.rcs.utils.ContactUtil;
import com.orangelabs.rcs.utils.ContactUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.provider.CursorUtil;
import com.orangelabs.rcs.provider.settings.RcsSettings;

/**
 * Rich messaging history for chats and file transfers.
 *
 * @author Jean-Marc AUFFRET
 */
public class RichMessagingHistory {
    /**
     * Current instance
     */
    private static RichMessagingHistory instance = null;

    private GroupDeliveryInfoLog mGroupChatDeliveryInfoLog;

    private static final int FIRST_COLUMN_IDX = 0;

    /**
     * Content resolver
     */
    private ContentResolver cr;

    /**
     * Chat database URI
     */
    private Uri chatDatabaseUri = ChatData.CONTENT_URI;

    /**
     * Message database URI
     */
    private Uri msgDatabaseUri = MessageData.CONTENT_URI;

    /*
     * GROUP MEMBER DATABASE URI
     */
    private Uri grpMemberDatabaseUri = GroupMemberData.CONTENT_URI;

    //URI for adding conversation id column in the database in case of CPM
    private final Uri alterChatDatabaseUri = Uri.parse(
            "content://com.orangelabs.rcs.chat/add_chat_conv_id");
    private final Uri alterMsgDatabaseUri = Uri.parse(
            "content://com.orangelabs.rcs.chat/add_msg_conv_id");

    /**
     * File transfer database URI
     */
    private Uri ftDatabaseUri = FileTransferData.CONTENT_URI;

    /**
     * Multi File transfer database URI
     */
    private Uri multiFtDatabaseUri = MultiFileTransferData.CONTENT_URI;

    private static final String SELECTION_BY_NOT_DISPLAYED = MessageData.KEY_STATUS + "<>"
            + Status.DISPLAYED.toInt();

    private static final String SELECTION_BY_NOT_SENT = MessageData.KEY_STATUS + "<>"
            + Status.SENT.toInt();

    private static final String SELECTION_BY_NOT_DELIEVERED = MessageData.KEY_STATUS + "<>"
            + Status.DELIVERED.toInt();

    private static final String SELECTION_FILE_BY_FT_ID =
            new StringBuilder(FileTransferData.KEY_FT_ID).append("=?").toString();

    private static final String PARTICIPANT_INFO_PARTICIPANT_SEPARATOR = ",";

    private static final String PARTICIPANT_INFO_STATUS_SEPARATOR = "=";

    private static final String SELECTION_BY_UNDELIVERED_FTSTATUS = FileTransferData.KEY_STATE
            + " NOT IN(" + FileTransfer.State.DELIVERED.toInt() + "," + FileTransfer.State.DISPLAYED.toInt() + ")";

    private static final String SELECTION_BY_UNDELIVERED_STATUS = MessageData.KEY_STATUS
            + " NOT IN(" + Status.DELIVERED.toInt() + "," + Status.DISPLAYED.toInt() + ")";

    private static final String SELECTION_BY_NOT_READ = FileTransferData.KEY_READ_STATUS + "="
            + ReadStatus.UNREAD.toInt();

    private final static String[] PROJECTION_FILE_TRANSFER_ID = new String[] {
        FileTransferData.KEY_FT_ID
    };

    private static final String SELECTION_BY_EQUAL_CHAT_ID_AND_CONTACT = FileTransferData.KEY_CHAT_ID
            + "=" + FileTransferData.KEY_CONTACT;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Create instance
     *
     * @param ctx Context
     */
    public static synchronized void createInstance(Context ctx) {
        if (instance == null) {
            instance = new RichMessagingHistory(ctx);
        }
    }

    /**
     * Returns instance
     *
     * @return Instance
     */
    public static RichMessagingHistory getInstance() {
        return instance;
    }

    /**
     * Constructor
     *
     * @param ctx Application context
     */
    private RichMessagingHistory(Context ctx) {
        super();

        this.cr = ctx.getContentResolver();
        mGroupChatDeliveryInfoLog = new GroupDeliveryInfoLog(ctx);

        if(true) {
            addColumnConversationID();
        }
    }

    /*--------------------- Group chat methods -----------------------*/

    private enum UserAbortion {

        SERVER_NOTIFIED(0), SERVER_NOT_NOTIFIED(1);

        private final int mValue;

        private static SparseArray<UserAbortion> mValueToEnum = new SparseArray<>();
        static {
            for (UserAbortion entry : UserAbortion.values()) {
                mValueToEnum.put(entry.toInt(), entry);
            }
        }

        UserAbortion(int value) {
            mValue = value;
        }

        public final int toInt() {
            return mValue;
        }
    }

    /**
     * Add group chat session
     *
     * @param chatId Chat ID
     * @param subject Subject
     * @param participants List of participants
     * @param status Status
     * @param direction Direction
     */
    public void addGroupChat(
            String chatId, String subject, List<String> participants, int status, int direction) {
        if (logger.isActivated()) {
            logger.debug("Add group chat entry: chatID=" + chatId);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_CHAT_ID, chatId);
        values.put(ChatData.KEY_STATE, status);
        values.put(ChatData.KEY_SUBJECT, subject);
        values.put(ChatData.KEY_PARTICIPANTS, RichMessagingHistory.getParticipants(participants));
        values.put(ChatData.KEY_DIRECTION, direction);
        values.put(ChatData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
        cr.insert(chatDatabaseUri, values);
    }

    public void addGroupChat(String chatId, String contact, String subject,
            Map<String, ParticipantStatus> participants, int state, int reasonCode,
            int direction, long timestamp) {
        String encodedParticipants = generateEncodedParticipantInfos(participants);
        if (logger.isActivated()) {
            logger.debug("addGroupChat; chatID=" + chatId + ", subject=" + subject + ", state="
                    + state + " reasonCode=" + reasonCode + ", direction=" + direction
                    + ", timestamp=" + timestamp + ", participants=" + encodedParticipants);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_CHAT_ID, chatId);
        if (contact != null) {
            values.put(ChatData.KEY_CONTACT, contact.toString());
        }
        values.put(ChatData.KEY_STATE, state);
        values.put(ChatData.KEY_REASON_CODE, reasonCode);
        values.put(ChatData.KEY_SUBJECT, subject);

        values.put(ChatData.KEY_PARTICIPANTS, encodedParticipants);
        values.put(ChatData.KEY_DIRECTION, direction);
        values.put(ChatData.KEY_TIMESTAMP, timestamp);
        values.put(ChatData.KEY_USER_ABORTION, UserAbortion.SERVER_NOTIFIED.toInt());
        cr.insert(ChatData.CONTENT_URI, values);
    }

    public boolean setGroupChatStateAndReasonCode(String chatId, int state, int reasonCode) {
        if (logger.isActivated()) {
            logger.debug("setGroupChatStateAndReasonCode (chatId=" + chatId + ") (state=" + state
                    + ") (reasonCode=" + reasonCode + ")");
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_STATE, state);
        values.put(ChatData.KEY_REASON_CODE, reasonCode);
        boolean result = cr.update(chatDatabaseUri,
                values,
                ChatData.KEY_CHAT_ID + " = '" + chatId + "'",
                null) > 0;
       logger.debug("setGroupChatStateAndReasonCoder: result: " + result);
       return result;
    }

    /**
     * Add group member detail for a chat
     *
     */
    public void addGroupMember(
            String chatId, String memberName, String memberNumber, String eType) {
        if (logger.isActivated()){
            logger.debug("Add group member: chatID=" + chatId
                    + "  and memeber : " + memberName + " and memberNumber : " + memberNumber);
        }
        ContentValues values = new ContentValues();
        values.put(GroupMemberData.KEY_CHAT_ID, chatId);
        values.put(GroupMemberData.KEY_CONTACT_NUMBER, memberNumber);
        values.put(GroupMemberData.KEY_MEMBER_NAME, memberName);
        values.put(GroupMemberData.KEY_CONTACT_TYPE, eType);

        cr.insert(grpMemberDatabaseUri, values);
    }

    public void removeGroupMember(String chatId, String memberNumber) {

        if (logger.isActivated()) {
            logger.debug("removeGroupMember() : for chatId = "+ chatId
                    + "; and memberNumber : "+memberNumber);
        }
        // Delete entries
        int deletedRows = cr.delete(
                grpMemberDatabaseUri,
                GroupMemberData.KEY_CHAT_ID+ " = '" + chatId +"' AND " +
                        GroupMemberData.KEY_CONTACT_NUMBER+ " = '" + memberNumber + "'",
                null);

    }

    public void updateGroupMemberEtype(String chatId, String memberNumber, String eType){
        if (logger.isActivated()) {
            logger.debug("updateGroupMemberEtype for  chat id : " + chatId
                    + "; member_number : "+ memberNumber + " ; etype : " + eType);
        }
        ContentValues values = new ContentValues();
        values.put(GroupMemberData.KEY_CONTACT_TYPE, eType);
        cr.update(grpMemberDatabaseUri,
                values,
                GroupMemberData.KEY_CHAT_ID + " = '" + chatId +
                "'  AND " +
                GroupMemberData.KEY_CONTACT_NUMBER + " = '"+memberNumber+"'",
                null);
    }

    public void updateGroupMemberName(String chatId, String memberNumber, String memberName){
        if (logger.isActivated()) {
            logger.debug("updateGroupMemberName for  chat id : " + chatId
                    + "; member_number : "+ memberNumber + " ; memberName : " + memberName);
        }
        ContentValues values = new ContentValues();
        values.put(GroupMemberData.KEY_MEMBER_NAME, memberName);
        cr.update(grpMemberDatabaseUri,
                values,
                GroupMemberData.KEY_CHAT_ID + " = '" + chatId +
                "'  AND " +
                GroupMemberData.KEY_CONTACT_NUMBER + " = '"+memberNumber+"'",
                null);
    }

   /* *//**
     * Get list of participants into a string
     *
     * @param participants List of participants
     * @return String (contacts are comma separated)
     *//*
    private static String getParticipants(List<String> participants) {
        StringBuffer result = new StringBuffer();
        for (String contact : participants) {
            if (contact != null) {
                result.append(PhoneUtils.extractNumberFromUri(contact) + ";");
            }
        }
        return result.toString();
    }*/

    /**
     * Convert participants to string representation
     *
     * @param participants the participants
     * @return the string with comma separated values of key pairs formatted as follows: "key=value"
     */
    private String generateEncodedParticipantInfos(Map<String, ParticipantStatus> participants) {
        StringBuilder builder = new StringBuilder();
        int size = participants.size();
        for (Map.Entry<String, ParticipantStatus> participant : participants.entrySet()) {
            builder.append(participant.getKey());
            builder.append(PARTICIPANT_INFO_STATUS_SEPARATOR);
            builder.append(participant.getValue().toInt());
            if (--size != 0) {
                builder.append(PARTICIPANT_INFO_PARTICIPANT_SEPARATOR);
            }
        }
        return builder.toString();
    }

    /**
     * Convert string representation of participants into participants
     *
     * @param participants the participants
     * @return the participants and their individual status
     */
    private Map<String, ParticipantStatus> parseEncodedParticipantInfos(String participants) {
        String[] encodedParticipantInfos = participants
                .split(PARTICIPANT_INFO_PARTICIPANT_SEPARATOR);
        Map<String, ParticipantStatus> participantInfos = new HashMap<>();
        for (String encodedParticipantInfo : encodedParticipantInfos) {
            String[] participantInfo = encodedParticipantInfo
                    .split(PARTICIPANT_INFO_STATUS_SEPARATOR);
            String participant = participantInfo[0];
            ParticipantStatus status = ParticipantStatus.valueOf(Integer
                    .parseInt(participantInfo[1]));
            participantInfos.put(participant, status);
        }
        return participantInfos;
    }



    public String getMessageMimeType(String msgId) {
        Cursor cursor = getMessageData(MessageData.KEY_MIME_TYPE, msgId);
        if (cursor == null) {
            return null;
        }
        return getDataAsString(cursor);
    }

    public com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode getMessageReasonCode(String msgId) {
        Cursor cursor = getMessageData(MessageData.KEY_REASON_CODE, msgId);
        if (cursor == null) {
            return null;
        }
        return com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode.valueOf(getDataAsInteger(cursor));
    }

    public Boolean isMessageRead(String msgId) {
        Cursor cursor = getMessageData(MessageData.KEY_READ_STATUS, msgId);
        if (cursor == null) {
            return null;
        }
        return (getDataAsInteger(cursor) == ReadStatus.READ.toInt());
    }

    public Boolean isChatMessageExpiredDelivery(String msgId) {
        Cursor cursor = getMessageData(MessageData.KEY_EXPIRED_DELIVERY, msgId);
        if (cursor == null) {
            return null;
        }
        return getDataAsBoolean(cursor);
    }

    public Long getMessageSentTimestamp(String msgId) {
        Cursor cursor = getMessageData(MessageData.KEY_TIMESTAMP_SENT, msgId);
        if (cursor == null) {
            return null;
        }
        return getDataAsLong(cursor);
    }

    public Long getMessageTimestamp(String msgId) {
        Cursor cursor = getMessageData(MessageData.KEY_TIMESTAMP, msgId);
        if (cursor == null) {
            return null;
        }
        return getDataAsLong(cursor);
    }

    private String getDataAsString(Cursor cursor) {
        try {
            return cursor.getString(FIRST_COLUMN_IDX);

        } finally {
            CursorUtil.close(cursor);
        }
    }

    private Boolean getDataAsBoolean(Cursor cursor) {
        try {
            if (cursor.isNull(FIRST_COLUMN_IDX)) {
                return null;
            }
            return cursor.getInt(FIRST_COLUMN_IDX) == 1;

        } finally {
            CursorUtil.close(cursor);
        }
    }

    private Long getDataAsLong(Cursor cursor) {
        try {
            if (cursor.isNull(FIRST_COLUMN_IDX)) {
                return null;
            }
            return cursor.getLong(FIRST_COLUMN_IDX);

        } finally {
            CursorUtil.close(cursor);
        }
    }

    private Integer getDataAsInteger(Cursor cursor) {
        try {
            if (cursor.isNull(FIRST_COLUMN_IDX)) {
                return null;
            }
            return cursor.getInt(FIRST_COLUMN_IDX);

        } finally {
            CursorUtil.close(cursor);
        }
    }

    private Cursor getMessageData(String columnName, String msgId) {
        String[] projection = new String[] {
            columnName
        };
        Uri contentUri = Uri.withAppendedPath(MessageData.CONTENT_URI, msgId);
        Cursor cursor = cr.query(contentUri, projection, null, null, null);
        CursorUtil.assertCursorIsNotNull(cursor, contentUri);
        if (!cursor.moveToNext()) {
            CursorUtil.close(cursor);
            return null;
        }
        return cursor;
    }

    public boolean deleteGroupChat(String chatId){
        if (logger.isActivated()) {
            logger.debug("deleteGroupChat() chatId:" + chatId);
        }
        int deletedRows = cr.delete(
                chatDatabaseUri,
                ChatData.KEY_CHAT_ID+ " = '" + chatId + "'",
                null);
        int deletedRowsMsg = cr.delete(
                msgDatabaseUri,
                MessageData.KEY_CHAT_ID+ " = '" + chatId + "'",
                null);
        int deletedRowsFile= cr.delete(
                ftDatabaseUri,
                FileTransferData.KEY_CHAT_ID+ " = '" + chatId + "'",
                null);
        if (logger.isActivated()) {
            logger.debug("deleteGroupChat() chatId:" + chatId + ", rows: " + deletedRows+deletedRowsMsg+deletedRowsFile) ;
        }
        if((deletedRows+deletedRowsMsg+deletedRowsFile)>0)
            return true;
        return false;
    }

    public boolean deleteO2OChat(String chatId){
        if (logger.isActivated()) {
            logger.debug("deleteO2OChat() chatId:" + chatId);
        }
        int deletedRowsMsg = cr.delete(
                msgDatabaseUri,
                MessageData.KEY_CHAT_ID+ " = '" + chatId + "'",
                null);
        int deletedRowsFile= cr.delete(
                ftDatabaseUri,
                FileTransferData.KEY_CHAT_ID+ " = '" + chatId + "'",
                null);
        if (logger.isActivated()) {
            logger.debug("deleteO2OChat() chatId:" + chatId + ", rows: " + deletedRowsMsg+deletedRowsFile) ;
        }
        if((deletedRowsMsg+deletedRowsFile)>0)
            return true;
        return false;
    }

    public void deleteMessage(String msgId){
        if (logger.isActivated()) {
            logger.debug("deleteMessage() msgId:" + msgId);
        }
        // Delete entries
        int deletedRows = cr.delete(
                msgDatabaseUri,
                MessageData.KEY_MESSAGE_ID+ " = '" + msgId + "'",
                null);
    }

    /**
     * Set outgoing group chat message or file to delivered
     *
     * @param chatId Group chat ID
     * @param contact The contact ID for which the entry is to be updated
     * @param msgId Message ID
     * @param timestampDelivered Time for delivery
     */
    public boolean setGroupChatDeliveryInfoDelivered(String chatId, String contact,
            String msgId, long timestampDelivered) {
        return mGroupChatDeliveryInfoLog.setGroupChatDeliveryInfoDelivered(chatId,contact,msgId,timestampDelivered);

    }

    public boolean setGroupChatDeliveryInfoDisplayed(String chatId, String contact,
            String fileTransferId, long timestampDisplayed) {
        return mGroupChatDeliveryInfoLog.setGroupChatDeliveryInfoDisplayed(chatId, contact,
                fileTransferId, timestampDisplayed);
    }

    /**
     * Set outgoing group chat message or file to delivered
     *
     * @param chatId Group chat ID
     * @param contact The contact ID for which the entry is to be updated
     * @param msgId Message ID
     * @param timestampDelivered Time for delivery
     */
    public boolean setDeliveryInfoDisplayed(String chatId, String contact,
            String msgId, long timestampDelivered) {
        return mGroupChatDeliveryInfoLog.setGroupChatDeliveryInfoDisplayed(chatId,contact,
            msgId, timestampDelivered);

    }

    public boolean isDeliveredToAllRecipients(String msgId) {
        return mGroupChatDeliveryInfoLog.isDeliveredToAllRecipients(msgId);

    }

    public boolean isDisplayedByAllRecipients(String msgId) {
        return mGroupChatDeliveryInfoLog.isDisplayedByAllRecipients(msgId);
    }

    public boolean setGroupChatParticipants(String chatId,
            Map<String, ParticipantStatus> participants) {
        String encodedParticipants = generateEncodedParticipantInfos(participants);
        if (logger.isActivated()) {
            logger.debug("updateGroupChatParticipant (chatId=" + chatId + ") (participants="
                    + encodedParticipants + ")");
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_PARTICIPANTS, encodedParticipants);
        return cr.update(
                Uri.withAppendedPath(ChatData.CONTENT_URI, chatId), values, null, null) > 0;
    }

    public boolean setMessageStatusDelivered(String msgId, long timestampDelivered) {
        if (logger.isActivated()) {
            logger.debug("setChatMessageStatusDelivered msgId=" + msgId + ", timestampDelivered="
                    + timestampDelivered);
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_STATUS, Status.DELIVERED.toInt());
        values.put(MessageData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
        values.put(MessageData.KEY_TIMESTAMP_DELIVERED, timestampDelivered);
        values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
        /*boolean value =  cr.update(Uri.withAppendedPath(MessageData.CONTENT_URI, msgId),
                values, SELECTION_BY_NOT_DELIEVERED, null) > 0;*/
        boolean value = cr.update(msgDatabaseUri,
                        values,
                        MessageData.KEY_MESSAGE_ID + " = '" + msgId + "'",
                        null) > 0;
        if (logger.isActivated()) {
            logger.debug("setChatMessageStatusDelivered msgId=" + msgId + ",result: " + value);
        }
        return true;
    }

    public boolean setMessageStatusSent(String msgId, long timestampDelivered) {
         if (logger.isActivated()) {
             logger.debug("setChatMessageStatusSent msgId=" + msgId + ", timestampSent="
                     + timestampDelivered);
         }
         ContentValues values = new ContentValues();
         values.put(MessageData.KEY_STATUS, Status.SENT.toInt());
         values.put(MessageData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
         values.put(MessageData.KEY_TIMESTAMP_SENT, timestampDelivered);
         //values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
         /*boolean value =  cr.update(Uri.withAppendedPath(MessageData.CONTENT_URI, msgId),
                 values, SELECTION_BY_NOT_SENT, null) > 0;*/
         boolean value = cr.update(msgDatabaseUri,
                 values,
                 MessageData.KEY_MESSAGE_ID + " = '" + msgId + "'",
                 null) > 0;
         if (logger.isActivated()) {
             logger.debug("setMessageStatusSent msgId=" + msgId + ",result: " + value);
         }
         return true;
     }

    public boolean setMessageStatusDisplayed(String msgId, long timestampDelivered) {
         if (logger.isActivated()) {
             logger.debug("setMessageStatusDisplayed msgId=" + msgId + ", timestampDelivered="
                     + timestampDelivered);
         }
         ContentValues values = new ContentValues();
         values.put(MessageData.KEY_STATUS, Status.DISPLAYED.toInt());
         values.put(MessageData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
         values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, timestampDelivered);
         //values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
         /*boolean value =  cr.update(Uri.withAppendedPath(MessageData.CONTENT_URI, msgId),
                 values, SELECTION_BY_NOT_DISPLAYED, null) > 0;*/
         boolean value = cr.update(msgDatabaseUri,
                 values,
                 MessageData.KEY_MESSAGE_ID + " = '" + msgId + "'",
                 null) > 0;
         if (logger.isActivated()) {
             logger.debug("setMessageStatusDisplayed msgId=" + msgId + ",result: " + value);
         }
         return true;
     }

    /**
     * Set delivery status for outgoing group chat messages and files. Note that this method should
     * not be used for Status.DELIVERED and Status.DISPLAYED. These states require timestamps and
     * should be set through setGroupChatDeliveryInfoDisplayed and setGroupChatDeliveryInfoDisplayed
     * respectively.
     *
     * @param chatId Group chat ID
     * @param contact The contact ID for which the entry is to be updated
     * @param msgId Message ID
     * @param status Status
     * @param reasonCode Reason code
     * @return true if an entry was updated, otherwise false
     */
    public boolean setGroupChatDeliveryInfoStatusAndReasonCode(String chatId, String contact,
            String msgId, int status, int reasonCode) {
        ContentValues values = new ContentValues();
        return mGroupChatDeliveryInfoLog.setGroupChatDeliveryInfoStatusAndReasonCode(chatId,contact,msgId, status, reasonCode);
    }

    public Uri addGroupChatDeliveryInfoEntry(String chatId, String contact, String msgId,
            int status, int reasonCode, long timestampDelivered, long timestampDisplayed) {
        com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo.Status mStatus = com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo.Status.valueOf(status);
        com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo.ReasonCode mReasonCode = com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo.ReasonCode.valueOf(reasonCode);
        return mGroupChatDeliveryInfoLog.addGroupChatDeliveryInfoEntry(chatId, contact, msgId, mStatus, mReasonCode, timestampDelivered, timestampDisplayed);

    }

    public boolean setChatMessageStatusAndReasonCode(String msgId, int status,
            int reasonCode) {
        if (logger.isActivated()) {
            logger.debug("Update chat message: msgId=" + msgId + ", status=" + status
                    + ", reasonCode=" + reasonCode);
        }
        Status mStatus = Status.valueOf(status);
        switch (mStatus) {
            case DELIVERED:
            case DISPLAYED:
                throw new IllegalArgumentException("Status that requires "
                        + "timestamp passed, use specific method taking timestamp"
                        + " to set status " + mStatus.toString());
            default:
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_STATUS, mStatus.toInt());
        values.put(MessageData.KEY_REASON_CODE, reasonCode);
        boolean value = cr.update(Uri.withAppendedPath(MessageData.CONTENT_URI, msgId),
                values, SELECTION_BY_UNDELIVERED_STATUS, null) > 0;
       logger.debug("setChatMessageStatusAndReasonCode value :" + value);
        return value;
    }

    /**
     * Get list of participants from a string
     *
     * @param String participants (contacts are comma separated)
     * @return String[] contacts or null if
     */
    private static List<String> getMultiFileParticipants(String participants) {
        ArrayList<String> result = new ArrayList<String>();
        if (participants != null && participants.trim().length() > 0) {
            String[] items = participants.split(",", 0);
            for (int i = 0; i < items.length; i++) {
                if (items[i] != null) {
                    result.add(items[i]);
                }
            }
        }
        return result;
    }

    /**
     * Update group chat status
     *
     * @param chatId Chat ID
     * @param status Status
     */
    public void updateGroupChatStatus(String chatId, int status) {
        if (logger.isActivated()) {
            logger.debug("Update group chat status to " + status);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_STATE, status);
        cr.update(chatDatabaseUri,
                values,
                ChatData.KEY_CHAT_ID + " = '" + chatId + "'",
                null);
    }

    /**
     * Update group chat rejoin ID
     *
     * @param chatId Chat ID
     * @param rejoingId Rejoin ID
     * @param status Status
     */
    public void updateGroupChatRejoinId(String chatId, String rejoingId) {
        if (logger.isActivated()) {
            logger.debug("Update group chat rejoin ID to " + rejoingId);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_REJOIN_ID, rejoingId);
        cr.update(chatDatabaseUri,
                values,
                ChatData.KEY_CHAT_ID + " = '" + chatId + "'",
                null);
    }

    /**
     * Get the group chat info
     *
     * @param chatId Chat ID
     * @result Group chat info
     */
    public GroupChatInfo getGroupChatInfo(String chatId) {
        if (logger.isActivated()) {
            logger.debug("Get group chat info for " + chatId);
        }
        GroupChatInfo result = null;
        Cursor cursor = cr.query(chatDatabaseUri,
                new String[] {
                    ChatData.KEY_CHAT_ID,
                    ChatData.KEY_REJOIN_ID,
                    ChatData.KEY_PARTICIPANTS,
                    ChatData.KEY_SUBJECT
                },
                "(" + ChatData.KEY_CHAT_ID + "='" + chatId + "')",
                null,
                ChatData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            String participants = cursor.getString(2);
            Map<String, ParticipantStatus> participantsInfo = parseEncodedParticipantInfos(participants);
            result = new GroupChatInfo(
                    cursor.getString(0),
                    cursor.getString(1),
                    chatId,
                    null,
                    cursor.getString(3));
            result.setParticipantsWithStatus(participantsInfo);

            List<String> participantList = new ArrayList();
            for(String participant:participantsInfo.keySet()){
                participantList.add(participant);
            }
            result.setParticipants(participantList);
        }
        cursor.close();
        return result;
    }

    /**
     * Get the group chat info
     *
     * @param chatId Chat ID
     * @result Group chat info
     */
    public GroupChatInfo getGroupChatInfoByRejoinId(String rejoinId) {
        if (logger.isActivated()) {
            logger.debug("Get group chat info by RejoinId for " + rejoinId);
        }
        GroupChatInfo result = null;
        Cursor cursor = cr.query(chatDatabaseUri,
                new String[] {
                    ChatData.KEY_CHAT_ID,
                    ChatData.KEY_REJOIN_ID,
                    ChatData.KEY_PARTICIPANTS,
                    ChatData.KEY_SUBJECT
                },
                "(" + ChatData.KEY_REJOIN_ID + "='" + rejoinId + "')",
                null,
                ChatData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            String participants = cursor.getString(2);
            Map<String, ParticipantStatus> participantsInfo = parseEncodedParticipantInfos(participants);
            result = new GroupChatInfo(
                    cursor.getString(0),
                    cursor.getString(1),
                    cursor.getString(0),
                    null,
                    cursor.getString(3));
            result.setParticipantsWithStatus(participantsInfo);
        }
        cursor.close();
        logger.debug("getGroupChatInfoByRejoinId value :" + result);
        return result;
    }

    /**
     * Get chat id by rejoin id
     * @param rejoin
     *            id
     * @result chat id
     */
    public String getChatIdbyRejoinId(String rejoinId) {
        if (logger.isActivated()) {
            logger.debug("Get group chat id by: " + rejoinId);
        }
        String chatId = null;
        Cursor cursor = cr.query(chatDatabaseUri,
                new String[] {
                ChatData.KEY_CHAT_ID,
                },
                "(" + ChatData.KEY_REJOIN_ID + "='" + rejoinId + "')",
                null,
                ChatData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            chatId = cursor.getString(cursor.getColumnIndex(ChatData.KEY_CHAT_ID));
        }
        cursor.close();
        logger.debug("getChatIdbyRejoinId value :" + chatId);
        return chatId;
    }

    /**
     * Get the group member info
     *
     * @param chatId Chat ID
     * @result Group member info
     */
    public boolean isGroupMemberExist(String chatId, String contact) {
        if (logger.isActivated()) {
            logger.debug("Get group member exist  for " + chatId);
        }
        Cursor cursor = cr.query(grpMemberDatabaseUri,
                new String[] {
                    GroupMemberData.KEY_CHAT_ID
                },
                "(" + GroupMemberData.KEY_CHAT_ID + "='" + chatId + "')"
                        + " AND (" + GroupMemberData.KEY_CONTACT_NUMBER + "=" + contact + ")",
                null,
                null);
        if (cursor == null) {
            return false;
        }

        if ((cursor.moveToFirst())) {
            cursor.close();
            return true;
        }
        cursor.close();
        return false;
    }

    public int getGroupReasonCode(String chatId){
        Cursor cursor = getGroupChatData(ChatData.KEY_REASON_CODE, chatId);
        if (cursor == null) {
            return -1;
        }
        return getDataAsInteger(cursor);
    }

    public String getGroupRemoteContact(String chatId){
        Cursor cursor = getGroupChatData(ChatData.KEY_CONTACT, chatId);
        if (cursor == null) {
        return null;
    }
        return getDataAsString(cursor);
    }

    public long getGroupTimeStamp(String chatId){
        Cursor cursor = getGroupChatData(ChatData.KEY_TIMESTAMP, chatId);
        if (cursor == null) {
            return 0L;
        }
        return getDataAsLong(cursor);
    }

    /**
     * Get the chat Contact
     *
     * @param Message ID
     * @result chat contact
     */
    public String getMessageContact(String msgId) {
        Cursor cursor = getMessageChatData(MessageData.KEY_CONTACT, msgId);
            if (cursor == null) {
            return null;
        }
        return getDataAsString(cursor);
    }

    /**
     * Get the chatId
     *
     * @param Message ID
     * @result chatiD
     */
    public String getMessageChatId(String msgId) {
        Cursor cursor = getMessageChatData(MessageData.KEY_CHAT_ID, msgId);
            if (cursor == null) {
            return null;
        }
        return getDataAsString(cursor);
    }

    public Cursor getChatMessageData(String msgId){
        Uri contentUri = Uri.withAppendedPath(MessageData.CONTENT_URI, msgId);
        Cursor cursor = cr.query(contentUri, null, null, null, null);
        CursorUtil.assertCursorIsNotNull(cursor, contentUri);
        return cursor;
    }


    private Cursor getMessageChatData(String columnName, String msgId) {
        String[] projection = new String[] {
            columnName
        };
        Uri contentUri = Uri.withAppendedPath(MessageData.CONTENT_URI, msgId);
        Cursor cursor = cr.query(contentUri, projection, null, null, null);
        CursorUtil.assertCursorIsNotNull(cursor, contentUri);
        if (!cursor.moveToNext()) {
            CursorUtil.close(cursor);
            return null;
        }
        return cursor;
    }

    public ChatMessage getChatMessage(String msgId) {
        Cursor cursor = null;
        try {
            cursor = getChatMessageData(msgId);
            if (!cursor.moveToNext()) {
                throw new ServerApiPersistentStorageException("Data not found for message " + msgId);
            }

             ContactId mRemoteContact = null;

             String mContent = null;

             String mMimeType = null;

             String mChatId = null;

             Boolean mRead = false;

             int mDirection = 0;

             Long timeStamp = 0L;

             Long mTimeStampSent = 0L;

             Long mTimestampDelivered = 0L;

             Long mTimestampDisplayed = 0L;

             int status = 0;

             int reasonCode = 0;

             boolean isExpired = false;
            String contact = cursor
                    .getString(cursor.getColumnIndexOrThrow(MessageData.KEY_CONTACT));
            if (contact != null) {
                /* Do not check validity for trusted data */
                mRemoteContact = ContactUtil.createContactIdFromTrustedData(contact);
              //  mRemoteContact = contact;
            }
            mDirection = cursor.getInt(cursor
                    .getColumnIndexOrThrow(MessageData.KEY_DIRECTION));
            mContent = cursor.getString(cursor.getColumnIndexOrThrow(MessageData.KEY_CONTENT));
            mChatId = cursor.getString(cursor.getColumnIndexOrThrow(MessageData.KEY_CHAT_ID));
            mMimeType = cursor.getString(cursor.getColumnIndexOrThrow(MessageData.KEY_MIME_TYPE));
            if (!Boolean.TRUE.equals(mRead)) {
                mRead = ReadStatus.READ.toInt() == cursor.getInt(cursor
                        .getColumnIndexOrThrow(MessageData.KEY_READ_STATUS));
            }
            if (mTimestampDelivered == null || mTimestampDelivered == 0) {
                mTimestampDelivered = cursor.getLong(cursor
                        .getColumnIndexOrThrow(MessageData.KEY_TIMESTAMP_DELIVERED));
            }
            if (mTimestampDisplayed == null || mTimestampDisplayed == 0) {
                mTimestampDisplayed = cursor.getLong(cursor
                        .getColumnIndexOrThrow(MessageData.KEY_TIMESTAMP_DISPLAYED));
            }
        /*return new ChatMessage(msgId,
                mRemoteContact,
                mContent,
                mMimeType,
                "",
                mDirection,
                timeStamp,mTimeStampSent,
                mTimestampDelivered,mTimestampDisplayed,
                status,
                reasonCode,mChatId,
                mRead,isExpired);*/
            return null;
        }finally {
            CursorUtil.close(cursor);
        }
    }

    /**
     * Get the group chat info
     *
     * @param chatId Chat ID
     * @result Group chat info
     */
    public int getMessageStatus(String msgId) {
        if (logger.isActivated()) {
            logger.debug("getMessageStatus " + msgId);
        }
        int result = ChatLog.Message.Content.Status.FAILED.toInt();
        Cursor cursor = cr.query(msgDatabaseUri,
                new String[] {
                    MessageData.KEY_MESSAGE_ID,
                    MessageData.KEY_STATUS
                },
                "(" + MessageData.KEY_MESSAGE_ID + "='" + msgId + "')",
                null,
                ChatData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            int status = cursor.getInt(1);
            result = status;
        }
        cursor.close();
        logger.debug("getMessageStatus value :" + result);
        return result;
    }

    public String getMessageText(String msgId) {
        if (logger.isActivated()) {
            logger.debug("getMessageText " + msgId);
        }
        String result = null;
        Cursor cursor = cr.query(msgDatabaseUri,
                new String[] {
                    MessageData.KEY_MESSAGE_ID,
                    MessageData.KEY_CONTENT
                },
                "(" + MessageData.KEY_MESSAGE_ID + "='" + msgId + "')",
                null,
                MessageData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            byte[] blobText = null;
            blobText = cursor.getBlob(1);

            if (blobText != null) {
                String status = new String(blobText);
                result = status;
            }
        }

        cursor.close();
        logger.debug("getMessageText value :" + result);
        return result;
    }

    public int getMessageType(String msgId) {
        if (logger.isActivated()) {
            logger.debug("getMessageType " + msgId);
        }
        int result = 0;
        Cursor cursor = cr.query(msgDatabaseUri,
                new String[] {
                    MessageData.KEY_TYPE
                },
                "(" + MessageData.KEY_MESSAGE_ID + "='" + msgId + "')",
                null,
                MessageData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            result = cursor.getInt(0);
        }

        cursor.close();
        logger.debug("getMessageType value :" + result);
        return result;
    }

    public long getMessageTimeStamp(String msgId) {
        if (logger.isActivated()) {
            logger.debug("getMessageType " + msgId);
        }
        long result = 0;
        Cursor cursor = cr.query(msgDatabaseUri,
                new String[] {
                    MessageData.KEY_MESSAGE_ID,
                    MessageData.KEY_TIMESTAMP
                },
                "(" + MessageData.KEY_MESSAGE_ID + "='" + msgId + "')",
                null,
                MessageData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            result = cursor.getLong(1);
        }

        cursor.close();
        logger.debug("getMessageTimeStamp value :" + result);
        return result;
    }

    public long getLatestTimeStamp(String contact){
    	 if (logger.isActivated()) {
             logger.debug("getLatestTimeStamp " + contact);
         }
         long result = 0;
         Cursor cursor = cr.query(msgDatabaseUri,
                 new String[] {
                     MessageData.KEY_TIMESTAMP
                 },
                 "(" + MessageData.KEY_CHAT_ID + "='" + contact + "')",
                 null,
                 MessageData.KEY_TIMESTAMP + " DESC");

         if ((cursor != null)) {
             if (cursor.moveToFirst()) {
                 if (logger.isActivated()) {
                     logger.debug("getLatestTimeStamp " + cursor.getCount());
                 }
                 result = cursor.getLong(0);
             }
             cursor.close();
         }
         return result;
    }

    /**
     * IS GROUPCHAT EXISTS
     *
     * @param chatId Chat ID
     * @result boolean status
     */
    public boolean isGroupChatExists(String chatId) {

        if (logger.isActivated()) {
            logger.debug("isGroupChatExists for : " + chatId);
        }
        boolean result = false;
        //Cursor cursor = cr.query(chatDatabaseUri,
        //new String[] {
        //ChatData.KEY_CHAT_ID
        //},
        ///"(" + ChatData.KEY_CHAT_ID + "='" + chatId + "')",
        //null,
        //ChatData.KEY_TIMESTAMP + " DESC");

        Cursor cursor = cr.query(chatDatabaseUri,
                new String[] { ChatData.KEY_CHAT_ID },
                "( " + ChatData.KEY_CHAT_ID + " = '" + chatId + "')",
                null, null);

        if ((cursor!=null) && (cursor.moveToFirst())) {
            if (logger.isActivated()) {
                logger.info("isGroupChatExists for : returns true");
            }
            result = true;
            cursor.close();
        }
        logger.debug("isGroupChatExists value :" + result);
        return result;
    }

    /**
     * Get the group chat participants who have been connected to the chat
     *
     * @param chatId Chat ID
     * @result List of contacts
     */
    public List<String> getGroupChatConnectedParticipants(String chatId) {
        if (logger.isActivated()) {
            logger.debug("Get connected participants for " + chatId);
        }
        List<String> result = new ArrayList<String>();
        Cursor cursor =
                cr.query(
                        msgDatabaseUri,
                        new String[] {
                                MessageData.KEY_CONTACT
                        },
                        "(" + MessageData.KEY_CHAT_ID + "='" + chatId + "') AND ("
                            + MessageData.KEY_TYPE + "=" + ChatLog.Message.Type.SYSTEM + ")",
                        null,
                        MessageData.KEY_TIMESTAMP + " DESC");
        while (cursor.moveToNext()) {
            String participant = cursor.getString(0);
            if ((participant != null) && (!result.contains(participant))) {
                result.add(participant);
            }
        }
        cursor.close();
        return result;
    }

    /*--------------------- Chat messages methods -----------------------*/

    /**
     * Add a spam message
     *
     * @param msg Chat message
     */
    public void addSpamMessage(InstantMessage msg) {
        addChatMessage(msg, ChatLog.Message.Type.SPAM,Direction.INCOMING.toInt());
    }

    /**
     * Add a chat message
     *
     * @param msg Chat message
     * @param direction Direction
     */
    public void addChatMessage(InstantMessage msg, int direction) {
        addChatMessage(msg, ChatLog.Message.Type.CONTENT, direction);
    }

    /**
     * Add a chat message
     *
     * @param msg Chat message
     * @param direction Direction
     */
    public void addChatSystemMessage(InstantMessage msg, int direction) {
        addChatMessage(msg, ChatLog.Message.Type.SYSTEM, direction);
    }

    /**
     * Add a chat message
     *
     * @param msg Chat message
     * @param direction Direction
     */
    public void addExtendChatMessage(InstantMessage msg, int direction) {
        addChatMessage(msg, msg.getMessageType(), direction);
    }

    /**
     * Add a chat message
     *
     * @param msg Chat message
     * @param type Message type
     * @param direction Direction
     */
    private void addChatMessage(InstantMessage msg, int type, int direction) {
        String contact = null;
        if (logger.isActivated()) {
            logger.debug("Add chat message:" +
                    " contact["  + msg.getRemote() + "]" +
                    " msg[" + msg.getMessageId() + "]" +
                    " type[" + type + "]" + " ,Text: " + msg.getTextMessage());
        }

        contact = new String(msg.getRemote());
        String chatId = msg.getRemote();
        if (PhoneUtils.generateContactsList(msg.getRemote()).size() > 1) {
        } else
        if (type == ChatLog.Message.Type.PUBLIC) {
            contact = PhoneUtils.extractUuidFromUri(contact);
        } else {
            contact = PhoneUtils.extractNumberFromUri(contact);
            chatId = PhoneUtils.extractNumberFromUri(chatId);
        }

        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_CHAT_ID, chatId);
        values.put(MessageData.KEY_MESSAGE_ID, msg.getMessageId());
        values.put(MessageData.KEY_CONTACT, contact);
        values.put(MessageData.KEY_DIRECTION, direction);
        values.put(MessageData.KEY_TYPE, type);
        values.put(MessageData.KEY_MIME_TYPE, msg.getMimeType());
        values.put(MessageData.KEY_DISPLAY_NAME, msg.getDisplayName());
        values.put(MessageData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());

        byte[] blob = null;
        if (msg instanceof GeolocMessage) {
            values.put(MessageData.KEY_TYPE, ChatLog.Message.MimeType.GEOLOC_MESSAGE);
            GeolocPush geoloc = ((GeolocMessage) msg).getGeoloc();
            Geoloc geolocApi =
                    new Geoloc(geoloc.getLabel(), geoloc.getLatitude(), geoloc.getLongitude(),
                            geoloc.getExpiration(), geoloc.getAccuracy());
            blob = serializeGeoloc(geolocApi);
            if (blob != null) {
                values.put(MessageData.KEY_CONTENT, blob);
            }
        } else {
            if ((type == ChatLog.Message.Type.PUBLIC) && (direction == Direction.INCOMING.toInt()))
                values.put(MessageData.KEY_TYPE, "application/xml");
            blob = serializePlainText(msg.getTextMessage());

            values.put(MessageData.KEY_CONTENT, msg.getTextMessage());
        }


        if (direction == Direction.INCOMING.toInt()) {
            // Receive message
            values.put(MessageData.KEY_TIMESTAMP, msg.getDate().getTime());
            values.put(MessageData.KEY_TIMESTAMP_SENT, 0);
            values.put(MessageData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, 0);
            if (msg.isImdnDisplayedRequested()) {
                values.put(MessageData.KEY_STATUS, ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt());
            } else {
                values.put(MessageData.KEY_STATUS, ChatLog.Message.Content.Status.RECEIVED.toInt());
            }
        } else {
            // Send message
            values.put(MessageData.KEY_TIMESTAMP, msg.getDate().getTime());
            values.put(MessageData.KEY_TIMESTAMP_SENT, msg.getDate().getTime());
            values.put(MessageData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, 0);
            values.put(MessageData.KEY_STATUS, ChatLog.Message.Content.Status.SENDING.toInt());
        }
        cr.insert(msgDatabaseUri, values);
    }

    /**
     * Add a group chat message
     *
     * @param chatId Chat ID
     * @param msg Chat message
     * @param direction Direction
     */
    public void addGroupChatMessage(String chatId, InstantMessage msg, int direction) {
        if (logger.isActivated()) {
            logger.debug("Add group chat message: chatID=" + chatId
                    + ", msg=" + msg.getMessageId());
        }

        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_CHAT_ID, chatId);
        values.put(MessageData.KEY_MESSAGE_ID, msg.getMessageId());
        values.put(MessageData.KEY_CONTACT, PhoneUtils.extractNumberFromUri(msg.getRemote()));
        values.put(MessageData.KEY_DIRECTION, direction);
        values.put(MessageData.KEY_TYPE, msg.getMessageType());
        values.put(MessageData.KEY_DISPLAY_NAME, msg.getDisplayName());
        values.put(MessageData.KEY_MIME_TYPE, msg.getMimeType());
        values.put(MessageData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());

        byte[] blob = null;
        if (msg instanceof GeolocMessage) {
            values.put(MessageData.KEY_TYPE, ChatLog.Message.Type.CONTENT);
            GeolocPush geoloc = ((GeolocMessage) msg).getGeoloc();
            Geoloc geolocApi =
                    new Geoloc(geoloc.getLabel(), geoloc.getLatitude(), geoloc.getLongitude(),
                            geoloc.getExpiration(), geoloc.getAccuracy());
            blob = serializeGeoloc(geolocApi);
            if (blob != null) {
                values.put(MessageData.KEY_CONTENT, blob);
            }
        } else {
            values.put(MessageData.KEY_TYPE, ChatLog.Message.Type.CONTENT);
            blob = serializePlainText(msg.getTextMessage());
            values.put(MessageData.KEY_CONTENT, msg.getTextMessage());
        }

        if (direction == Direction.INCOMING.toInt()) {
            // Receive message
            Date date = new Date();
            values.put(MessageData.KEY_TIMESTAMP, msg.getDate().getTime());
            values.put(MessageData.KEY_TIMESTAMP_SENT, date.getTime());
            values.put(MessageData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, 0);
            if (msg.isImdnDisplayedRequested()) {
                values.put(MessageData.KEY_STATUS, ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt());
            } else {
                values.put(MessageData.KEY_STATUS, ChatLog.Message.Content.Status.RECEIVED.toInt());
            }
        } else {
            // Send message
            values.put(MessageData.KEY_TIMESTAMP, msg.getDate().getTime());
            values.put(MessageData.KEY_TIMESTAMP_SENT, msg.getDate().getTime());
            values.put(MessageData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, 0);
            values.put(MessageData.KEY_STATUS, ChatLog.Message.Content.Status.SENDING.toInt());
        }
        cr.insert(msgDatabaseUri, values);
    }

    /**
     * Add group chat system message
     *
     * @param chatId Chat ID
     * @param contact Contact
     * @param status Status
     */
    public void addGroupChatSystemMessage(String chatId, String contact, int status) {
        if (logger.isActivated()) {
            logger.debug("Add group chat system message: chatID=" + chatId
                    + ", contact=" + contact + ", status=" + status);
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_CHAT_ID, chatId);
        values.put(MessageData.KEY_CONTACT, contact);
        values.put(MessageData.KEY_TYPE, ChatLog.Message.Type.SYSTEM);
        values.put(MessageData.KEY_STATUS, status);
        values.put(MessageData.KEY_DIRECTION, Direction.IRRELEVANT.toInt());
        values.put(ChatData.KEY_TIMESTAMP, Calendar.getInstance().getTimeInMillis());
        cr.insert(msgDatabaseUri, values);
    }

    /**
     * Update chat message status
     *
     * @param msgId Message ID
     * @param status Message status
     */
    public void updateChatMessageStatus(String msgId, int status) {
        if (logger.isActivated()) {
            logger.debug("Update chat message: msgID=" + msgId + ", status=" + status);
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_STATUS, status);
        if (status == ChatLog.Message.Content.Status.RECEIVED.toInt()) {
            // Delivered
            values.put(MessageData.KEY_TIMESTAMP_DELIVERED,
                    Calendar.getInstance().getTimeInMillis());
        } else if (status == ChatLog.Message.Content.Status.DISPLAYED.toInt()) {
            // Displayed
            values.put(MessageData.KEY_TIMESTAMP_DISPLAYED,
                    Calendar.getInstance().getTimeInMillis());
        }
        cr.update(msgDatabaseUri, values, MessageData.KEY_MESSAGE_ID + " = '" + msgId + "'", null);
    }

    /**
     * Update chat message delivery status
     *
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void updateChatMessageDeliveryStatus(String msgId, String status) {
        if (logger.isActivated()) {
            logger.debug("Update chat delivery status: msgID=" + msgId + ", status=" + status);
        }
        if (status.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
            RichMessagingHistory.getInstance().updateChatMessageStatus(
                    msgId, ChatLog.Message.Content.Status.RECEIVED.toInt());
        } else if (status.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
            RichMessagingHistory.getInstance().updateChatMessageStatus(
                    msgId, ChatLog.Message.Content.Status.DISPLAYED.toInt());
        } else if (status.equals(ImdnDocument.DELIVERY_STATUS_ERROR) ||
                status.equals(ImdnDocument.DELIVERY_STATUS_FAILED)) {
            RichMessagingHistory.getInstance().updateChatMessageStatus(
                    msgId, ChatLog.Message.Content.Status.FAILED.toInt());
        }
    }

    /**
     * Check if it's a new message
     *
     * @param chatId chat ID
     * @param msgId message ID
     * @return true if new message
     */
    public boolean isNewMessage(String chatId, String msgId) {
        boolean result = true;
        Cursor cursor =cr.query(
                msgDatabaseUri,
                new String[] {
                    MessageData.KEY_MESSAGE_ID
                },
                "(" + MessageData.KEY_CHAT_ID + " = '" + chatId + "') AND ("
                    + MessageData.KEY_MESSAGE_ID + " = '" + msgId + "')",
                null,
                null);
        if (cursor.moveToFirst()) {
            result = false;
        } else {
            result = true;
        }
        cursor.close();
        logger.debug("isNewMessage value :" + result);
        return result;
    }

    public boolean isOne2OneMessageExists(String msgId) {
        if (logger.isActivated()) {
            logger.debug("isOne2OneMessageExists , msgid : " + msgId);
        }
        boolean result = false;
        Cursor cursor = cr.query(
                msgDatabaseUri,
                new String[] {
                        MessageData.KEY_CHAT_ID,
                        MessageData.KEY_CONTACT
                },
                "( " + MessageData.KEY_MESSAGE_ID + " = '" + msgId + "')",
                null, null);
        if (cursor.moveToFirst()) {
            if (logger.isActivated()) {
                logger.debug("cursor.moveToFirst() is true");
            }
            if (cursor.moveToFirst()) {
                String chat = cursor.getString(0);
                String contact = cursor.getString(1);
                if (logger.isActivated()) {
                    logger.debug("isOne2OneMessageExists chat: " + chat + ", contact: " + contact + ", result: " + result);
                }
                if(chat != null && contact != null && chat.equals(contact)){
                    result = true;
                }
            }
        } else {

            if (logger.isActivated()) {
                logger.debug("cursor.moveToFirst() is false");
            }
            result = false;
        }
        cursor.close();
        logger.debug("isOne2OneMessageExists value :" + result);
        return result;
    }

    public void clearMessageDeliveryExpiration(List<String> msgIds) {
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_DELIVERY_EXPIRATION, 0);
        values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
        List<String> parameters = new ArrayList<>();
        for (int i = 0; i < msgIds.size(); i++) {
            parameters.add("?");
        }
        String selection = MessageData.KEY_MESSAGE_ID + " IN (" + TextUtils.join(",", parameters)
                + ")";
        cr.update(MessageData.CONTENT_URI, values, selection,
                msgIds.toArray(new String[msgIds.size()]));
    }

    public boolean isGroupMessageExists(String msgId) {
        if (logger.isActivated()) {
            logger.debug("isGroupMessageExists , msgid : " + msgId);
        }
        boolean result = false;
        Cursor cursor = cr.query(
                msgDatabaseUri,
                new String[] {
                        MessageData.KEY_CHAT_ID,
                        MessageData.KEY_CONTACT
                },
                "( " + MessageData.KEY_MESSAGE_ID + " = '" + msgId + "')",
                null, null);
        if (cursor.moveToFirst()) {
            if (logger.isActivated()) {
                logger.debug("isGroupMessageExists message found");
            }
            if (cursor.moveToFirst()) {
                String chat = cursor.getString(0);
                String contact = cursor.getString(1);
                result = true;
                if (logger.isActivated()) {
                    logger.debug("isGroupMessageExists chat: " + chat + ", contact: " + contact + ",result: " + result);
                }
                if(chat.equals(contact)){
                    result = false;
                }
            }
        } else {

            if (logger.isActivated()) {
                logger.debug("isGroupMessageExists not found");
            }
            result = false;
        }
        cursor.close();
        logger.debug("isGroupMessageExists value :" + result);
        return result;
    }

    public boolean isMessageExists(String msgId) {
        return isOne2OneMessageExists(msgId);
    }

    /*--------------------- File transfer methods ----------------------*/

    /**
     * Add outgoing file transfer
     *
     * @param contact Contact
     * @param sessionId Session ID
     * @param direction Direction
     * @param content File content
     */
    public void addFileTransfer(String contact, String fileTransferId, int direction,
            MmContent content) {
        /*
         * if(contact.contains("")){//multimessage }else
         */
        contact = PhoneUtils.extractNumberFromUri(contact);
        if (logger.isActivated()) {
            logger.debug(
                    new StringBuilder("Add file transfer entry: fileTransferId=")
                            .append(fileTransferId)
                            .append(", contact=")
                            .append(contact)
                            .append(", filename=")
                            .append(content.getName())
                            .append(", size=")
                            .append(content.getSize()).append(", MIME=")
                            .append(content.getEncoding())
                            .toString());
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        values.put(FileTransferData.KEY_CHAT_ID, contact);
        values.put(FileTransferData.KEY_CONTACT, contact);
        values.put(FileTransferData.KEY_FILENAME, content.getUrl());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, direction);
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());
        values.put(FileTransferData.KEY_SESSION_TYPE, FileTransferLog.Type.CHAT);

        long date = Calendar.getInstance().getTimeInMillis();
        if (direction == Direction.INCOMING.toInt()) {
            // Receive file
            values.put(FileTransferData.KEY_TIMESTAMP, date);
            values.put(FileTransferData.KEY_TIMESTAMP_SENT, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
            //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
            values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        } else {
            // Send file
            values.put(FileTransferData.KEY_TIMESTAMP, date);
            values.put(FileTransferData.KEY_TIMESTAMP_SENT, date);
            values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
            //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
            values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        }

        cr.insert(ftDatabaseUri, values);
    }

    public void addFileTransfer(String fileTransferId, String contact,
            int direction, MmContent content, MmContent fileIcon, int state,
            int reasonCode, long timestamp, long timestampSent, long fileExpiration,
            long fileIconExpiration) {
        if (logger.isActivated()) {
            logger.debug("Add file transfer entry Id=" + fileTransferId + ", contact=" + contact
                    + ", filename=" + content.getName() + ", size=" + content.getSize() + ", MIME="
                    + content.getEncoding() + ", state=" + state + ", reasonCode=" + reasonCode
                    + ", timestamp=" + timestamp + ", timestampSent=" + timestampSent);
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        values.put(FileTransferData.KEY_CHAT_ID, contact.toString());
        values.put(FileTransferData.KEY_CONTACT, contact.toString());
        values.put(FileTransferData.KEY_FILE, content.getUrl().toString());
        values.put(FileTransferData.KEY_FILENAME, content.getName());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, direction);
        if (content.isPlayable()) {
            values.put(FileTransferData.KEY_DISPOSITION, FileTransfer.Disposition.RENDER.toInt());
        } else {
            values.put(FileTransferData.KEY_DISPOSITION, FileTransfer.Disposition.ATTACH.toInt());
        }
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());
        if (fileIcon != null) {
            values.put(FileTransferData.KEY_FILEICON, fileIcon.getUrl().toString());
            values.put(FileTransferData.KEY_FILEICON_MIME_TYPE, fileIcon.getEncoding());
            values.put(FileTransferData.KEY_FILEICON_EXPIRATION, fileIconExpiration);
        } else {
            values.put(FileTransferData.KEY_FILEICON_EXPIRATION,
                    FileTransferData.UNKNOWN_EXPIRATION);
        }
        values.put(FileTransferData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());
        values.put(FileTransferData.KEY_STATE, state);
        values.put(FileTransferData.KEY_REASON_CODE, reasonCode);
        values.put(FileTransferData.KEY_TIMESTAMP, timestamp);
        values.put(FileTransferData.KEY_TIMESTAMP_SENT, timestampSent);
        values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
        values.put(FileTransferData.KEY_DELIVERY_EXPIRATION, 0);
        values.put(FileTransferData.KEY_EXPIRED_DELIVERY, 0);
        values.put(FileTransferData.KEY_FILE_EXPIRATION, fileExpiration);
        cr.insert(FileTransferData.CONTENT_URI, values);
    }

    /**
     * Add outgoing file transfer
     *
     * @param contact Contact
     * @param sessionId Session ID
     * @param direction Direction
     * @param content File content
     */
    public void addBurnFileTransfer(String contact, String fileTransferId, int direction,
            MmContent content) {
        contact = PhoneUtils.extractNumberFromUri(contact);
        if (logger.isActivated()) {
            logger.debug(new StringBuilder("Add burn file transfer entry: fileTransferId=")
                    .append(fileTransferId)
                    .append(", contact=")
                    .append(contact)
                    .append(", filename=")
                    .append(content.getName())
                    .append(", size=")
                    .append(content.getSize())
                    .append(", MIME=")
                    .append(content.getEncoding())
                    .toString());
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        values.put(FileTransferData.KEY_CHAT_ID, contact);
        values.put(FileTransferData.KEY_CONTACT, contact);
        values.put(FileTransferData.KEY_FILENAME, content.getUrl());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, direction);
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());

        // add burn message
        values.put(FileTransferData.KEY_SESSION_TYPE, FileTransferLog.Type.BURN);

        long date = Calendar.getInstance().getTimeInMillis();
        if (direction == Direction.INCOMING.toInt()) {
            // Receive file
            values.put(FileTransferData.KEY_TIMESTAMP, date);
            values.put(FileTransferData.KEY_TIMESTAMP_SENT, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
            //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
            values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        } else {
            // Send file
            values.put(FileTransferData.KEY_TIMESTAMP, date);
            values.put(FileTransferData.KEY_TIMESTAMP_SENT, date);
            values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
            //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
            values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        }

        cr.insert(ftDatabaseUri, values);
    }

    /**
     * Add outgoing file transfer
     *
     * @param contact Contact
     * @param sessionId Session ID
     * @param direction Direction
     * @param content File content
     */
    public void addExtendedFileTransfer(
            String chatId,
            String contact,
            String fileTransferId,
            int direction,
            int type,
            MmContent content) {
        if (type != FileTransferLog.Type.PUBLIC &&
            !PhoneUtils.compareUuid(RcsSettings.getInstance().getSecondaryDeviceUserIdentity(), contact))
            contact = PhoneUtils.extractNumberFromUri(contact);

        if (logger.isActivated()) {
            logger.debug(
                    new StringBuilder("Add file transfer entry: fileTransferId=")
                            .append(fileTransferId)
                            .append(", contact=")
                            .append(contact)
                            .append(", filename=")
                            .append(content.getName())
                            .append(", size=")
                            .append(content.getSize()).append(", MIME=")
                            .append(content.getEncoding())
                            .toString());
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        if (chatId == null)
            values.put(FileTransferData.KEY_CHAT_ID, contact);
        else
            values.put(FileTransferData.KEY_CHAT_ID, chatId);
        values.put(FileTransferData.KEY_CONTACT, contact);
        values.put(FileTransferData.KEY_FILENAME, content.getUrl());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, direction);
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());
        values.put(FileTransferData.KEY_SESSION_TYPE, type);

        long date = Calendar.getInstance().getTimeInMillis();
        if (direction == Direction.INCOMING.toInt()) {
            // Receive file
            values.put(FileTransferData.KEY_TIMESTAMP, date);
            values.put(FileTransferData.KEY_TIMESTAMP_SENT, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
            //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
            values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        } else {
            // Send file
            values.put(FileTransferData.KEY_TIMESTAMP, date);
            values.put(FileTransferData.KEY_TIMESTAMP_SENT, date);
            values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
            values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
            //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
            values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        }

        cr.insert(ftDatabaseUri, values);
    }

    /*
     * (non-Javadoc)
     * @see
     * com.orangelabs.rcs.provider.messaging.IFileTransferLog#addOutgoingGroupFileTransfer
     * (java.lang.String, java.lang.String, com.orangelabs.rcs.core.content.MmContent,
     * com.orangelabs.rcs.core.content.MmContent)
     */

    public void addOutgoingGroupFileTransfer(String chatId, String fileTransferId,
            MmContent content, MmContent thumbnail, String remoteContact) {
        if (logger.isActivated()) {
            logger.debug("addOutgoingGroupFileTransfer: fileTransferId=" + fileTransferId
                    + ", chatId=" + chatId + " filename=" + content.getName() + ", size="
                    + content.getSize() + ", MIME=" + content.getEncoding());
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        values.put(FileTransferData.KEY_CHAT_ID, chatId);
        values.put(FileTransferData.KEY_CONTACT, remoteContact);
        values.put(FileTransferData.KEY_FILENAME, content.getUrl());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, RcsService.Direction.OUTGOING.toInt());
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());
        values.put(FileTransferData.KEY_SESSION_TYPE, FileTransferLog.Type.CHAT);

        long date = Calendar.getInstance().getTimeInMillis();
        // values.put(MessageData.KEY_READ_STATUS, ChatLog.Message.ReadStatus.UNREAD);
        // Send file
        values.put(FileTransferData.KEY_TIMESTAMP, date);
        values.put(FileTransferData.KEY_TIMESTAMP_SENT, date);
        values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
        //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
        values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        /*
         * if (thumbnail != null) { values.put(FileTransferData.KEY_FILEICON,
         * Uri.fromFile(new File(thumbnail.getUrl())).toString()); }
         */
        cr.insert(ftDatabaseUri, values);

    }

    public void addOutgoingGroupFileTransfer(String fileTransferId, String chatId,
            MmContent content, MmContent thumbnail, int state,
            int reasonCode, long timestamp, long timestampSent) {
        if (logger.isActivated()) {
            logger.debug("addOutgoingGroupFileTransfer: Id=" + fileTransferId + ", chatId="
                    + chatId + " filename=" + content.getName() + ", size=" + content.getSize()
                    + ", MIME=" + content.getEncoding());
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        values.put(FileTransferData.KEY_CHAT_ID, chatId);
        values.put(FileTransferData.KEY_FILE, content.getUrl().toString());
        values.put(FileTransferData.KEY_FILENAME, content.getName());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, RcsService.Direction.OUTGOING.toInt());
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());
        values.put(MessageData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());
        values.put(FileTransferData.KEY_TIMESTAMP, timestamp);
        values.put(FileTransferData.KEY_TIMESTAMP_SENT, timestampSent);
        values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
        values.put(FileTransferData.KEY_DELIVERY_EXPIRATION, 0);
        values.put(FileTransferData.KEY_EXPIRED_DELIVERY, 0);
        values.put(FileTransferData.KEY_STATE, state);
        values.put(FileTransferData.KEY_REASON_CODE, reasonCode);
        if (thumbnail != null) {
            values.put(FileTransferData.KEY_FILEICON, thumbnail.getUrl().toString());
            values.put(FileTransferData.KEY_FILEICON_MIME_TYPE, thumbnail.getEncoding());
        }
        values.put(FileTransferData.KEY_FILEICON_EXPIRATION, FileTransferData.UNKNOWN_EXPIRATION);
        values.put(FileTransferData.KEY_FILE_EXPIRATION, FileTransferData.UNKNOWN_EXPIRATION);
        if (content.isPlayable()) {
            values.put(FileTransferData.KEY_DISPOSITION, FileTransfer.Disposition.RENDER.toInt());
        } else {
            values.put(FileTransferData.KEY_DISPOSITION, FileTransfer.Disposition.ATTACH.toInt());
        }
        cr.insert(FileTransferData.CONTENT_URI, values);
    }

    /*
     * (non-Javadoc)
     * @see
     * com.orangelabs.rcs.provider.messaging.IFileTransferLog#addIncomingGroupFileTransfer
     * (java.lang.String, java.lang.String, java.lang.String,
     * com.orangelabs.rcs.core.content.MmContent,
     * com.orangelabs.rcs.core.content.MmContent)
     */

    public void addIncomingGroupFileTransfer(String chatId, String contact, String fileTransferId,
            MmContent content) {
        contact = PhoneUtils.extractNumberFromUri(contact);
        if (logger.isActivated()) {
            logger.debug(new StringBuilder("Add incoming file transfer entry: fileTransferId=")
                    .append(fileTransferId)
                    .append(", chatId=")
                    .append(chatId)
                    .append(", contact=")
                    .append(contact)
                    .append(", filename=")
                    .append(content.getName())
                    .append(", size=")
                    .append(content.getSize())
                    .append(", MIME=")
                    .append(content.getEncoding()).toString());
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        values.put(FileTransferData.KEY_CHAT_ID, chatId);
        values.put(FileTransferData.KEY_CONTACT, contact);
        values.put(FileTransferData.KEY_FILENAME, content.getUrl());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, RcsService.Direction.INCOMING.toInt());
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());
        // values.put(FileTransferData.KEY_READ_STATUS, FileTransfer.ReadStatus.UNREAD);

        long date = Calendar.getInstance().getTimeInMillis();
        values.put(FileTransferData.KEY_TIMESTAMP, date);
        values.put(FileTransferData.KEY_TIMESTAMP_SENT, 0);
        values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
        //values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED);
        values.put(FileTransferData.KEY_STATE, FileTransfer.State.INVITED.toInt());
        // if (thumbnail != null) {
        // values.put(FileTransferData.KEY_FILEICON, Uri.fromFile(new
        // File(thumbnail.getUrl())).toString());
        // }

        cr.insert(ftDatabaseUri, values);
    }

    public void addIncomingGroupFileTransfer(String fileTransferId, String chatId,
            String contact, MmContent content, MmContent fileIcon, int state,
            int reasonCode, long timestamp, long timestampSent, long fileExpiration,
            long fileIconExpiration) {
        if (logger.isActivated()) {
            logger.debug("Add incoming file transfer entry: fileTransferId=" + fileTransferId
                    + ", chatId=" + chatId + ", contact=" + contact + ", filename="
                    + content.getName() + ", size=" + content.getSize() + ", MIME="
                    + content.getEncoding() + ", state=" + state + ", reasonCode=" + reasonCode
                    + ", timestamp=" + timestamp + ", timestampSent=" + timestampSent
                    + ", expiration=" + fileExpiration);
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, fileTransferId);
        values.put(FileTransferData.KEY_CHAT_ID, chatId);
        values.put(FileTransferData.KEY_FILE, content.getUrl().toString());
        values.put(FileTransferData.KEY_CONTACT, contact.toString());
        values.put(FileTransferData.KEY_FILENAME, content.getName());
        values.put(FileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(FileTransferData.KEY_DIRECTION, RcsService.Direction.INCOMING.toInt());
        values.put(FileTransferData.KEY_TRANSFERRED, 0);
        values.put(FileTransferData.KEY_FILESIZE, content.getSize());
        values.put(FileTransferData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());
        values.put(FileTransferData.KEY_STATE, state);
        values.put(FileTransferData.KEY_REASON_CODE, reasonCode);
        values.put(FileTransferData.KEY_TIMESTAMP, timestamp);
        values.put(FileTransferData.KEY_TIMESTAMP_SENT, timestampSent);
        values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
        values.put(FileTransferData.KEY_DELIVERY_EXPIRATION, 0);
        values.put(FileTransferData.KEY_EXPIRED_DELIVERY, 0);
        if (fileIcon != null) {
            values.put(FileTransferData.KEY_FILEICON, fileIcon.getUrl().toString());
            values.put(FileTransferData.KEY_FILEICON_MIME_TYPE, fileIcon.getEncoding());
            values.put(FileTransferData.KEY_FILEICON_EXPIRATION, fileIconExpiration);
        } else {
            values.put(FileTransferData.KEY_FILEICON_EXPIRATION,
                    FileTransferData.UNKNOWN_EXPIRATION);
        }
        values.put(FileTransferData.KEY_FILE_EXPIRATION, fileExpiration);
        if (content.isPlayable()) {
            values.put(FileTransferData.KEY_DISPOSITION, FileTransfer.Disposition.RENDER.toInt());
        } else {
            values.put(FileTransferData.KEY_DISPOSITION, FileTransfer.Disposition.ATTACH.toInt());
        }
        cr.insert(FileTransferData.CONTENT_URI, values);
    }

    /**
     * Update file transfer Icon
     *
     * @param fileTransferId fileTransferId
     * @param Url new Url
     */

    public void updateFileIcon(String fileTransferId, String url) {
        if (logger.isActivated()) {
            logger.debug("updateFileIcon (File transfer ID" + fileTransferId + ")");
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FILEICON, url);

        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = "
                + fileTransferId, null);

    }

    /**
     * Update file transfer status
     *
     * @param sessionId Session ID
     * @param status New status
     */

    public void updateFileTransferStatus(String fileTransferId, int status) {
        if (logger.isActivated()) {
            logger.debug("updateFileTransferStatus (status=" + status + ") (fileTransferId="
                    + fileTransferId + ")");
        }
        if (!isMultiFT(fileTransferId)) {
            ContentValues values = new ContentValues();
            values.put(FileTransferData.KEY_STATE, status);
            //if (status == FileTransfer.State.DELIVERED) {
            if (status == FileTransfer.State.DELIVERED.toInt()) {
                // Delivered
                values.put(FileTransferData.KEY_TIMESTAMP_DELIVERED,
                        Calendar.getInstance().getTimeInMillis());
            //} else if (status == FileTransfer.State.DISPLAYED) {
            } else if (status == FileTransfer.State.DISPLAYED.toInt()) {
                // Displayed
                values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED,
                        Calendar.getInstance().getTimeInMillis());
                values.put(FileTransferData.KEY_READ_STATUS,1);
            }
            cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId,
                    null);
        } else {
            updateMultiFileTransferStatus(fileTransferId, status);
        }
    }

    /**
     * Update file transfer duration
     *
     * @param sessionId Session ID
     * @param status New status
     */

    public void updateFileTransferDuration(String fileTransferId, int duration) {
        if (logger.isActivated()) {
            logger.debug("updateFileTransferStatus (duration=" + duration + ") (fileTransferId="
                    + fileTransferId + ")");
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_DURATION, duration);
        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId, null);
    }

    /**
     * Update file transfer Id
     *
     * @param fileTransferId fileTransferId
     * @param snewFileTransferId
     */

    public void updateFileTransferId(String fileTransferId, String newFileTransferId) {
        if (logger.isActivated()) {
            logger.debug("updateFileTransferId (New=" + newFileTransferId + ") (Old="
                    + fileTransferId + ")");
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FT_ID, newFileTransferId);

        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId, null);
    }

    /**
     * Update file Id
     *
     * @param fileTransferId fileTransferId
     * @param snewFileTransferId
     */
    public void updateFileId(String fileTransferId, String file_id) {
        if (logger.isActivated()) {
            logger.debug("updateFileId (file_id=" + file_id + ") (fileTransferId ="
                    + fileTransferId + ")");
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FILE_ID, file_id);

        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = "
                + fileTransferId, null);
    }

    /**
     * Get the file transfer direction
     *
     * @param fileTransferId fileTransferId
     * @result Direction
     */
    public String getFtDirection(String fileTransferId) {
        if (logger.isActivated()) {
            logger.debug("getFtDirection " + fileTransferId);
        }
        String result = null;
        Cursor cursor = cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_DIRECTION
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + fileTransferId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            String direction = cursor.getString(0);
            result = direction;
        }
        cursor.close();
        logger.debug("getFtDirection value :" + result);
        return result;
    }

    /**
     * Get the file transfer Duration
     *
     * @param fileTransferId fileTransferId
     * @result Duration
     */
    public String getFtDuration(String fileTransferId) {
        if (logger.isActivated()) {
            logger.debug("getFtDuration " + fileTransferId);
        }
        String result = null;
        Cursor cursor = cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_DURATION
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + fileTransferId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            String direction = cursor.getString(0);
            result = direction;
        }
        cursor.close();
        logger.debug("getFtDuration value :" + result);
        return result;
    }

    /**
     * Update file transfer download progress
     *
     * @param sessionId Session ID
     * @param size Downloaded size
     * @param totalSize Total size to download
     */
    public void updateFileTransferProgressCode(
            String fileTransferId, long size, long totalSize,
            String path, String contact, String hash) {
        if (logger.isActivated()) {
            logger.debug("updateFileTransferProgressCode fileTransferId" + fileTransferId
                    + " size-" + size + " path-" + path + " contact-" + contact + " hash-" + hash);
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_TRANSFERRED, size);
        values.put(FileTransferData.KEY_FILESIZE, totalSize);
        values.put(FileTransferData.KEY_CONTACT, contact);
        values.put(FileTransferData.KEY_HASH, hash);
        values.put(FileTransferData.KEY_FILENAME, path);
        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = "
                + fileTransferId, null);
    }

    public PauseResumeFileObject getPauseInfo(String transferId) {
        if (logger.isActivated()) {
            logger.debug("Get pause file transfer info for " + transferId);
        }
        PauseResumeFileObject result = null;
        Cursor cursor = cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_TRANSFERRED,
                        FileTransferData.KEY_FILESIZE,
                        FileTransferData.KEY_CONTACT,
                        FileTransferData.KEY_HASH,
                        FileTransferData.KEY_FILENAME,
                        FileTransferData.KEY_FILE_ID,
                        FileTransferData.KEY_MIME_TYPE,
                        FileTransferData.KEY_SESSION_TYPE
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + transferId + "')",
                null, ChatData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            // String participants = cursor.getString(2);
            // List<String> list = RichMessagingHistory.getParticipants(participants);
            result = new PauseResumeFileObject();
            result.mFileTransferId = transferId;
            result.bytesTransferrred = cursor.getLong(0);
            result.mSize = cursor.getLong(1);
            result.mContact = cursor.getString(2);
            result.hashSelector = cursor.getString(3);
            result.mPath = cursor.getString(4);
            result.mOldFileTransferId = cursor.getString(5);
            result.mMimeType = cursor.getString(6);
            result.mTransferType = cursor.getInt(7);
            result.pausedStream = null;
        }
        cursor.close();
        return result;
    }

    /**
     * Update file transfer download progress
     *
     * @param sessionId Session ID
     * @param size Downloaded size
     * @param totalSize Total size to download
     */
    public void updateFileTransferProgress(String fileTransferId, long size, long totalSize) {
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_TRANSFERRED, size);
        values.put(FileTransferData.KEY_FILESIZE, totalSize);
        //values.put(FileTransferData.KEY_STATE, FileTransfer.State.STARTED);
        values.put(FileTransferData.KEY_STATE, FileTransfer.State.STARTED.toInt());
        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = "
                + fileTransferId, null);
    }

    /**
     * Update file transfer URL
     *
     * @param sessionId Session ID
     * @param url File URL
     */
    public void updateFileTransferUrl(String fileTransferId, String url) {
        if (logger.isActivated()) {
            logger.debug("updateFileTransferUrl (fileTransferId=" + fileTransferId + ") (url="
                    + url + ")");
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_FILENAME, url);
        //values.put(FileTransferData.KEY_STATE, FileTransfer.State.TRANSFERRED);
        values.put(FileTransferData.KEY_STATE, FileTransfer.State.TRANSFERRED.toInt());
        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = "
                + fileTransferId, null);
    }

    /**
     * Get file transfer ID from a received message
     *
     * @param msgId Message ID
     * @return Chat session ID of the file transfer
     */
    public String getFileTransferId(String msgId) {
        String result = null;
        Cursor cursor = cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_FT_ID
                },
                "(" + FileTransferData.KEY_MSG_ID + "='" + msgId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " ASC");
        if (cursor.moveToFirst()) {
            result = cursor.getString(0);
        }
        cursor.close();
        logger.debug("getFileTransferId value :" + result);
        return result;
    }

    /**
     * Get file transfer ID from a received message
     *
     * @param msgId Message ID
     * @return Chat session ID of the file transfer
     */
    public String getFileMessageId(String ftId) {
        String result = null;
        Cursor cursor = cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_MSG_ID
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + ftId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            result = cursor.getString(0);
        }
        cursor.close();
        logger.debug("getFileMessageId value :" + result);
        return result;
    }

    /**
     * Get file read status from a received message
     *
     * @param msgId Message ID
     * @return read status
     */
    public int getFileReadStatus(String ftId) {
        int result = -1;
        try {
            Cursor cursor = cr.query(
                    ftDatabaseUri,
                    new String[] {
                            FileTransferData.KEY_READ_STATUS
                    },
                    "(" + FileTransferData.KEY_FT_ID + "='" + ftId + "')",
                    null, FileTransferData.KEY_TIMESTAMP + " DESC");
            if (cursor.moveToFirst()) {
                result = cursor.getInt(0);
            }
            cursor.close();
            logger.debug("getFileMessageId value :" + result);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return result;
    }

    /**
     * Get file transfer time stamp
     *
     * @param transferId filetransferID
     * @return timestamp
     */
    public long getFileTimeStamp(String transferId) {
        long result = 0;
        Cursor cursor = cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_TIMESTAMP
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + transferId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            result = cursor.getLong(0);
        }
        cursor.close();
        logger.debug("getFileTimeStamp value :" + result);
        return result;
    }

    /**
     * Get file transfer contact
     *
     * @param ftID filetransferID
     * @return Contacts
     */
    public List<String> getFileTransferContacts(String ftId) {
        String result = null;

        Cursor cursor =cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_CONTACT
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + ftId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            result = cursor.getString(0);
        }
        if (logger.isActivated()) {
            logger.debug("getFileTransferContacts result  + fileTransferId " + result + "&" + ftId);
        }
        cursor.close();
        List<String> list = getMultiFileParticipants(result);
        return list;
    }

    /**
     * Get file transfer contact
     *
     * @param ftID filetransferID
     * @return Contacts
     */
    public String getFileTransferChatId(String ftId) {
        String result = null;

        Cursor cursor =cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_CHAT_ID
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + ftId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            result = cursor.getString(0);
        }
        if (logger.isActivated()) {
            logger.debug("getFileTransferChatId result  + fileTransferId " + result + "&" + ftId);
        }
        cursor.close();
        logger.debug("getFileTransferChatId value :" + result);
        return result;
    }

    /**
     * Get file transfer contact
     *
     * @param ftID filetransferID
     * @return Contacts
     */
    public String getFileTransferContactId(String ftId) {
        String result = null;

        Cursor cursor =cr.query(
                ftDatabaseUri,
                new String[] {
                        FileTransferData.KEY_CHAT_ID
                },
                "(" + FileTransferData.KEY_FT_ID + "='" + ftId + "')",
                null, FileTransferData.KEY_TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            result = cursor.getString(0);
        }
        if (logger.isActivated()) {
            logger.debug("getFileTransferContactId result  + fileTransferId " + result + "&" + ftId);
        }
        cursor.close();
        logger.debug("getFileTransferContactId value :" + result);
        return result;
    }

    /**
     * Update file transfer ChatId
     *
     * @param sessionId Session Id
     * @param chatId chat Id
     * @param msgId msgId of the corresponding chat
     */
    public void updateFileTransferMsgId(String fileTransferId, String msgId) {
        /*
         * TODO ContentValues values = new ContentValues();
         * values.put(RichMessagingData.KEY_CHAT_ID, chatId);
         * values.put(RichMessagingData.KEY_MESSAGE_ID , msgId); cr.update(databaseUri,
         * values, RichMessagingData.KEY_CHAT_SESSION_ID + " = " + sessionId, null);
         */
        if (logger.isActivated()) {
            logger.debug("updateFileTransferMsgId(fileTransferId="
                    + fileTransferId + ") MsgID = " + msgId);
        }
        if (msgId == null)
            return;
        ContentValues values = new ContentValues();
        if (msgId != null)
            values.put(FileTransferData.KEY_MSG_ID, msgId);
        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId, null);
    }

    /**
     * Update file transfer ChatId
     *
     * @param sessionId Session Id
     * @param chatId chat Id
     * @param msgId msgId of the corresponding chat
     */
    public void updateFileTransferChatId(String fileTransferId, String chatId, String msgId) {
        /*
         * TODO ContentValues values = new ContentValues();
         * values.put(RichMessagingData.KEY_CHAT_ID, chatId);
         * values.put(RichMessagingData.KEY_MESSAGE_ID , msgId); cr.update(databaseUri,
         * values, RichMessagingData.KEY_CHAT_SESSION_ID + " = " + sessionId, null);
         */
        if (logger.isActivated()) {
            logger.debug("updateFileTransferChatId (chatId=" + chatId + ") (fileTransferId="
                    + fileTransferId + ") MsgID = " + msgId);
        }
        if (chatId == null && msgId == null)
            return;
        ContentValues values = new ContentValues();
        if (chatId != null)
            values.put(FileTransferData.KEY_CHAT_ID, chatId);
        if (msgId != null)
            values.put(FileTransferData.KEY_MSG_ID, msgId);
        cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId, null);

    }

    /**
     * Serialize a geoloc to bytes array
     *
     * @param geoloc Geoloc info
     * @return Byte array
     */
    private byte[] serializeGeoloc(Geoloc geoloc) {
        byte[] blob = null;
        try {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            ObjectOutputStream os = new ObjectOutputStream(bos);
            os.writeObject(geoloc);
            blob = bos.toByteArray();
            bos.close();
            os.close();
        } catch (Exception e) {
            blob = null;
        }
        return blob;
    }

    /**
     * Serialize a text message to bytes array
     *
     * @param msg Message
     * @return Byte array
     */
    private byte[] serializePlainText(String msg) {
        if (msg != null) {
            return msg.getBytes();
        } else {
            return null;
        }
    }

    /**
     * M: add conversation ID column in Chat and Message database
     */

    public void addColumnConversationID() {

        if (isConversationIDColumnExists()) {
            if (logger.isActivated()) {
                //logger.debug("Conversation ID column already exists. so no need to add columns");
            }
        }

        try {
            // update in chat table
            cr.query(alterChatDatabaseUri, null, null, null, null);

            // update in msg table
            cr.query(alterMsgDatabaseUri, null, null, null, null);

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.debug("exception: " + e);
            }
        }
    }

    // type 1: o2o and tyep =2 grphcat
    public String getCoversationID(String chatID, int type) {
        try{
            String conversationID = "";

            if (logger.isActivated()) {
                logger.debug("getCoversationID for chatID: " + chatID);
            }
            if (type == 1) {
                // o2o chat
                String contact = new String(chatID);
                contact = PhoneUtils.extractNumberFromUri(contact);
                Cursor cursor = cr.query(
                        msgDatabaseUri,
                        new String[] {
                                MessageData.KEY_CONVERSATION_ID
                        },
                        "( " + MessageData.KEY_CONTACT + " = '" + contact + "'  AND "
                             + MessageData.KEY_TYPE + " = " + Type.SYSTEM + " )",
                        null, null);

                if ((cursor != null) && (cursor.moveToFirst())) {
                    conversationID = cursor.getString(0);

                    if (logger.isActivated()) {
                        logger.info("Conversation ID[" + conversationID + "]" +
                                "found for contact[" + contact + "]");
                    }
                    // cursor.close();
                }
                if (cursor != null) {
                    if (!cursor.isClosed()) {
                        cursor.close();
                    }
                }

            } else if (type == 2) {
                // grp chat

                Cursor cursor = cr.query(
                        chatDatabaseUri,
                        new String[] {
                                ChatData.KEY_CONVERSATION_ID
                        },
                        "( " + ChatData.KEY_CHAT_ID + " = '" + chatID + "')",
                        null, null);

                if ((cursor != null) && (cursor.moveToFirst())) {
                    conversationID = cursor.getString(0);

                    if (logger.isActivated()) {
                        logger.info("Conversation ID found : " + conversationID);
                    }
                    // cursor.close();
                }
                if (cursor != null) {
                    if (!cursor.isClosed()) {
                        cursor.close();
                    }
                }
            }

            return conversationID;
        } catch(Exception e){
            e.printStackTrace();
            return null;
        }
    }

    // update the conversation ID
    // type 1: o2o and type =2 grphcat
    public void UpdateCoversationID(String chatID, String conversationID, int type) {
        if (logger.isActivated()) {
            logger.debug("UpdateCoversationID" +
                    " Id[" + conversationID + "]" +
                    " chatId[" + chatID + "]" +
                    " type[" + type + "]");
        }
        if (type == 1) {
            // o2o chat
            String contact = new String(chatID);
            contact = PhoneUtils.extractNumberFromUri(contact);

            if (logger.isActivated()) {
                logger.info("UpdateConversationID for contact[" + contact + "]");
            }

            ContentValues values = new ContentValues();
            values.put(MessageData.KEY_CONVERSATION_ID, conversationID);
            int count = cr.update(msgDatabaseUri, values, MessageData.KEY_CONTACT + " = '" + contact + "' AND "
                    + MessageData.KEY_TYPE + " = " + Type.SYSTEM, null);
            if(count > 0){
                if (logger.isActivated()) {
                    logger.debug("UpdateCoversationID count: " + count);
                }
                return;
            }


            String msgId = IdGenerator.generateMessageID();
            InstantMessage msg = new InstantMessage(msgId, chatID, "dummy", true,"");
            addChatSystemMessage(msg, Direction.OUTGOING.toInt());
            cr.update(msgDatabaseUri, values, MessageData.KEY_CONTACT + " = '" + contact + "' AND "
                    + MessageData.KEY_TYPE + " = " + Type.SYSTEM, null);

        } else if (type == 2) {
            // grp chat
            ContentValues values = new ContentValues();
            values.put(ChatData.KEY_CONVERSATION_ID, conversationID);
            cr.update(chatDatabaseUri, values, ChatData.KEY_CHAT_ID + " = '" + chatID + "'", null);
        }

    }

    private boolean isConversationIDColumnExists() {

        boolean status = false;
        try {
            Cursor cursor = cr.query(
                chatDatabaseUri,
                new String[] {
                    ChatData.KEY_CONVERSATION_ID
                },
                null, null, null);
            if ((cursor != null)) {
                cursor.close();
            }
            status = true;
        } catch (Exception e) {
            status = false;
            if (logger.isActivated()) {
                logger.debug("exception no column as ChatData.KEY_CONVERSATION_ID present");
            }
        }
        //logger.debug("isConversationIDColumnExists - status: " + status);
        return status;
    }

    /**
     * Update group chat sybject
     *
     * @param chatId Chat ID
     * @param subject subject
     */
    public void updateGroupChatSubject(String chatId, String subject) {
        if (logger.isActivated()) {
            logger.debug("Update group chat subject to " + subject);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_SUBJECT, subject);
        cr.update(chatDatabaseUri, values, ChatData.KEY_CHAT_ID + " = '" + chatId + "'", null);
    }

    /**
     * Update group chat chairman
     *
     * @param chatId Chat ID
     * @param status Status
     */
    public void updateGroupChairman(String chatId, String chairman) {
        if (logger.isActivated()) {
            logger.debug("Update group chairman  to " + chairman);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_CHAIRMAN, chairman);
        cr.update(chatDatabaseUri, values, ChatData.KEY_CHAT_ID + " = '" + chatId + "'", null);
    }

    /**
     * Get Chairman of the group
     *
     * @param chatId chat ID of group
     * @return String chairman number
     */
    public String getGroupChairman(String chatId) {
        String chairman = null;
        Cursor cursor = cr.query(
                chatDatabaseUri,
                new String[] {
                        ChatData.KEY_CHAIRMAN
                },
                "(" + ChatData.KEY_CHAT_ID + "='" + chatId + "')",
                null, ChatData.KEY_TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            chairman = cursor.getString(0);
        }
        cursor.close();
        return chairman;
    }

    /**
     * Update group chat chairman nickname
     *
     * @param chatId Chat ID
     * @param status Status
     */
    public void updateGroupNickName(String chatId, String chairmanNickName) {
        if (logger.isActivated()) {
            logger.debug("Update group chairman nickname to " + chairmanNickName);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_NICKNAME, chairmanNickName);
        cr.update(chatDatabaseUri, values, ChatData.KEY_CHAT_ID + " = '" + chatId + "'", null);
    }

    /*---------------------Multi File transfer methods ----------------------*/

    public boolean isMultiFT(String fileTransferId) {
        boolean status = false;

        if (logger.isActivated()) {
            logger.debug("isMultiFile Transfer : " + fileTransferId);
        }
        List<String> result = new ArrayList<String>();
        try {
            Cursor cursor = cr.query(
                    multiFtDatabaseUri,
                    new String[] {
                            MultiFileTransferData.KEY_PARTICIPANTS_LIST
                    },
                    "(" + MultiFileTransferData.KEY_FT_ID + "=" + fileTransferId + ")",
                    null, MultiFileTransferData.KEY_TIMESTAMP + " DESC");

            while (cursor != null && cursor.moveToNext()) {
                status = true;
            }
            if (cursor != null) {
                cursor.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (logger.isActivated()) {
            logger.debug("isMultiFile Transfer : " + fileTransferId + " status: " + status);
        }
        return status;
    }

    /*
     * (non-Javadoc)
     * @see com.orangelabs.rcs.provider.messaging.IMultiFileTransferLog#
     * addOutgoingMultiFileTransfer(java.lang.String, java.lang.String,
     * com.orangelabs.rcs.core.content.MmContent,
     * com.orangelabs.rcs.core.content.MmContent)
     */

    public void addOutgoingMultiFileTransfer(String fileTransferId, MmContent content,
            MmContent thumbnail, List<String> participants) {
        if (logger.isActivated()) {
            logger.debug("addOutgoingMultiFileTransfer: "
                    + "fileTransferId=" + fileTransferId
                    + "filename=" + content.getName()
                    + ", size=" + content.getSize()
                    + ", MIME=" + content.getEncoding());
        }
        ContentValues values = new ContentValues();

        values.put(MultiFileTransferData.KEY_PARTICIPANTS_LIST,
                RichMessagingHistory.getParticipants(participants));

        values.put(MultiFileTransferData.KEY_FT_ID, fileTransferId);

        values.put(MultiFileTransferData.KEY_NAME, content.getUrl());
        values.put(MultiFileTransferData.KEY_MIME_TYPE, content.getEncoding());
        values.put(MultiFileTransferData.KEY_DIRECTION, Direction.OUTGOING.toInt());
        values.put(MultiFileTransferData.KEY_SIZE, 0);
        values.put(MultiFileTransferData.KEY_TOTAL_SIZE, content.getSize());
        long date = Calendar.getInstance().getTimeInMillis();
        // values.put(MessageData.KEY_READ_STATUS, ChatLog.Message.ReadStatus.UNREAD);
        // Send file
        values.put(MultiFileTransferData.KEY_TIMESTAMP, date);
        values.put(MultiFileTransferData.KEY_TIMESTAMP_SENT, date);
        values.put(MultiFileTransferData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(MultiFileTransferData.KEY_TIMESTAMP_DISPLAYED, 0);
         //values.put(MultiFileTransferData.KEY_STATUS, FileTransfer.State.INVITED);
        values.put(MultiFileTransferData.KEY_STATUS, FileTransfer.State.INVITED.toInt());
        /*
         * if (thumbnail != null) { values.put(FileTransferData.KEY_FILEICON,
         * Uri.fromFile(new File(thumbnail.getUrl())).toString()); }
         */
        cr.insert(multiFtDatabaseUri, values);
    }

    /**
     * Update Multi file transfer status
     *
     * @param sessionId Session ID
     * @param status New status
     */

    public void updateMultiFileTransferStatus(String fileTransferId, int status) {
        if (logger.isActivated()) {
            logger.debug("updateFileTransferStatus (status=" + status + ") (fileTransferId="
                    + fileTransferId + ")");
        }
        ContentValues values = new ContentValues();
        values.put(MultiFileTransferData.KEY_STATUS, status);
        //if (status == FileTransfer.State.DELIVERED) {
        if (status == FileTransfer.State.DELIVERED.toInt()) {
            // Delivered
            values.put(MultiFileTransferData.KEY_TIMESTAMP_DELIVERED, Calendar.getInstance()
                    .getTimeInMillis());
        //} else if (status == FileTransfer.State.DISPLAYED) {
        } else if (status == FileTransfer.State.DISPLAYED.toInt()) {
            // Displayed
            values.put(MultiFileTransferData.KEY_TIMESTAMP_DISPLAYED, Calendar.getInstance()
                    .getTimeInMillis());
        }
        cr.update(multiFtDatabaseUri, values, MultiFileTransferData.KEY_FT_ID + " = "
                + fileTransferId, null);
    }

    /**
     * Update multiple file transfer URL
     *
     * @param sessionId Session ID
     * @param url File URL
     */
    public void updateMultiFileTransferUrl(String fileTransferId, String url) {
        if (logger.isActivated()) {
            logger.debug("updateFileTransferUrl (fileTransferId=" + fileTransferId + ") (url="
                    + url + ")");
        }
        if (isMultiFT(fileTransferId)) {
            ContentValues values = new ContentValues();
            values.put(MultiFileTransferData.KEY_NAME, url);
            //values.put(MultiFileTransferData.KEY_STATUS, FileTransfer.State.TRANSFERRED);
            values.put(MultiFileTransferData.KEY_STATUS, FileTransfer.State.TRANSFERRED.toInt());
            cr.update(multiFtDatabaseUri, values, MultiFileTransferData.KEY_FT_ID + " = "
                    + fileTransferId, null);
        } else {
            updateMultiFileTransferUrl(fileTransferId, url);
        }
    }

    public List<String> getMessageParticipants(String msgId) {
        if (logger.isActivated()) {
            logger.debug("getMesageParticipants " + msgId);
        }
        List<String> result = null;
        Cursor cursor = cr.query(
                msgDatabaseUri,
                new String[] {
                        MessageData.KEY_MESSAGE_ID,
                        MessageData.KEY_CONTACT
                },
                "(" + MessageData.KEY_MESSAGE_ID + "='" + msgId + "')",
                null, MessageData.KEY_TIMESTAMP + " DESC");

        if (cursor.moveToFirst()) {
            String contacts = cursor.getString(1);
            if (!contacts.isEmpty()) {
                if (logger.isActivated()) {
                    logger.debug("getMesageChatParticipants: " + contacts);
                }

                result = PhoneUtils.generateContactsList(contacts);
            }
        }
        cursor.close();

        return result;
    }

    /*
     * END :MULTI PARTICIPANT MESSAGE DATABASE API
     */

    /**
     * Update group chat block status
     *
     * @param chatId Chat ID
     * @param flag block flag [0 : NOT_BLOCKED , 1: BLOCKED]
     */
    public void updateGroupBlockedStatus(String chatId, int flag) {
        if (logger.isActivated()) {
            logger.debug("Update group block status to " + flag);
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_ISBLOCKED, flag);
        cr.update(chatDatabaseUri, values, ChatData.KEY_CHAT_ID + " = '" + chatId + "'", null);
    }

    /**
     * Get Chairman of the group
     *
     * @param chatId chat ID of group
     * @return String chairman number
     */
    public int getGroupBlockedStatus(String chatId) {
        int isblocked = 0;
        Cursor cursor = cr.query(
                chatDatabaseUri,
                new String[] {
                        ChatData.KEY_ISBLOCKED
                },
                "(" + ChatData.KEY_CHAT_ID + "='" + chatId + "')",
                null, ChatData.KEY_TIMESTAMP + " DESC");
        if (cursor.moveToFirst()) {
            isblocked = cursor.getInt(0);
        }
        cursor.close();
        logger.debug("getGroupBlockedStatus value :" + isblocked);
        return isblocked;
    }

    public Cursor getFileTransferData(String fileTransferId) {
        Uri contentUri = Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId);
        Cursor cursor = cr.query(contentUri, null, null, null, null);
        CursorUtil.assertCursorIsNotNull(cursor, contentUri);
        return cursor;
    }

    /**
     * Set file transfer state and reason code. Note that this method should not be used for
     * State.DELIVERED and State.DISPLAYED. These states require timestamps and should be set
     * through setFileTransferDelivered and setFileTransferDisplayed respectively.
     *
     * @param fileTransferId File transfer ID
     * @param state File transfer state (see restriction above)
     * @param reasonCode File transfer state reason code
     */
    public boolean setFileTransferStateAndReasonCode(String fileTransferId, FileTransfer.State state,
            FileTransfer.ReasonCode reasonCode) {
        if (logger.isActivated()) {
            logger.debug("setFileTransferStateAndReasonCode: fileTransferId=" + fileTransferId
                    + ", state=" + state + ", reasonCode=" + reasonCode);
        }

        switch (state) {
            case DELIVERED:
            case DISPLAYED:
                throw new IllegalArgumentException("State that requires "
                        + "timestamp passed, use specific method taking timestamp"
                        + " to set state " + state.toString());
            default:
        }

        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_STATE, state.toInt());
        values.put(FileTransferData.KEY_REASON_CODE, reasonCode.toInt());
        boolean value =  cr.update(ftDatabaseUri, values, FileTransferData.KEY_FT_ID + " = " + fileTransferId,
                        null) > 0;
        if (logger.isActivated()) {
            logger.debug("setFileTransferStateAndReasonCode: value =" + value);
        }
        return true;//done for machine testing
    }

    public int markFileTransferAsRead(String fileTransferId, long timestampDisplayed) {
        if (logger.isActivated()) {
            logger.debug("Mark file transfer as read ID=" + fileTransferId);
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_READ_STATUS, ReadStatus.READ.toInt());
        values.put(FileTransferData.KEY_TIMESTAMP_DISPLAYED, timestampDisplayed);
        int value =  cr.update(
                Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId), values,
                SELECTION_BY_NOT_READ, null);
        if (logger.isActivated()) {
            logger.debug("markFileTransferAsRead: value =" + value);
        }
        return value;
    }

    public boolean setFileTransferProgress(String fileTransferId, long currentSize) {
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_TRANSFERRED, currentSize);
        boolean value =  cr.update(
                Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId), values, null,
                null) > 0;
        if (logger.isActivated()) {
            logger.debug("setFileTransferProgress: value =" + value);
        }
        return value;

    }

    public Long getFileTransferProgress(String fileTransferId) {
        Cursor cursor = getFileTransferData(FileTransferData.KEY_TRANSFERRED, fileTransferId);
        if (cursor == null) {
            return null;
        }
        return getDataAsLong(cursor);
    }

    public boolean setFileTransferred(String fileTransferId, MmContent content,
            long fileExpiration, long fileIconExpiration, long deliveryExpiration) {
        if (logger.isActivated()) {
            logger.debug("setFileTransferred (Id=" + fileTransferId + ") (uri=" + content.getUri()
                    + ")");
        }
        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_STATE, FileTransfer.State.TRANSFERRED.toInt());
        values.put(FileTransferData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
        values.put(FileTransferData.KEY_TRANSFERRED, content.getSize());
        values.put(FileTransferData.KEY_FILE_EXPIRATION, fileExpiration);
        values.put(FileTransferData.KEY_FILEICON_EXPIRATION, fileIconExpiration);
        values.put(FileTransferData.KEY_DELIVERY_EXPIRATION, deliveryExpiration);
        boolean value =  cr.update(
                Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId), values, null,
                null) > 0;
        if (logger.isActivated()) {
            logger.debug("setFileTransferred: value =" + value);
        }
        return value;
    }

    public boolean isFileTransfer(String fileTransferId) {
        Cursor cursor = null;
        try {
            Uri contentUri = Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId);
            cursor = cr.query(contentUri, PROJECTION_FILE_TRANSFER_ID, null,
                    null, null);
            CursorUtil.assertCursorIsNotNull(cursor, contentUri);
            return cursor.moveToNext();
        } finally {
            CursorUtil.close(cursor);
        }
    }

    private Cursor getFileTransferData(String columnName, String fileTransferId) {
        String[] projection = {
            columnName
        };
        Uri contentUri = Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId);
        Cursor cursor = cr.query(contentUri, projection, null, null, null);
        CursorUtil.assertCursorIsNotNull(cursor, contentUri);
        if (!cursor.moveToNext()) {
            CursorUtil.close(cursor);
            return null;
        }
        return cursor;
    }

    public Uri getFileTransferIcon(String fileTransferId) {
        Cursor cursor = getFileTransferData(FileTransferData.KEY_FILEICON, fileTransferId);
        if (cursor == null) {
            return null;
        }
        String uriString = getDataAsString(cursor);
        if (uriString == null) {
            return null;
        }
        return Uri.parse(uriString);
    }

    public FileTransfer.State getFileTransferState(String fileTransferId) {
        Cursor cursor = getFileTransferData(FileTransferData.KEY_STATE, fileTransferId);
        if (cursor == null) {
            return null;
        }
        return FileTransfer.State.valueOf(getDataAsInteger(cursor));
    }

    public FileTransfer.ReasonCode getFileTransferReasonCode(String fileTransferId) {
        Cursor cursor = getFileTransferData(FileTransferData.KEY_REASON_CODE, fileTransferId);
        if (cursor == null) {
            return null;
        }
        return FileTransfer.ReasonCode.valueOf(getDataAsInteger(cursor));
    }

    public Long getFileTransferTimestamp(String fileTransferId) {
        Cursor cursor = getFileTransferData(FileTransferData.KEY_TIMESTAMP, fileTransferId);
        if (cursor == null) {
            return null;
        }
        return getDataAsLong(cursor);
    }

    public Long getFileTransferSentTimestamp(String fileTransferId) {
        Cursor cursor = getFileTransferData(FileTransferData.KEY_TIMESTAMP_SENT, fileTransferId);
        if (cursor == null) {
            return null;
        }
        return getDataAsLong(cursor);
    }

    public Boolean isFileTransferExpiredDelivery(String fileTransferId) {
        Cursor cursor = getFileTransferData(FileTransferData.KEY_EXPIRED_DELIVERY, fileTransferId);
        if (cursor == null) {
            return null;
        }
        return getDataAsBoolean(cursor);
    }

    public boolean isGroupFileTransfer(String fileTransferId) {
        /*
         * Warning: return true if record does not exist.
         */
        if (logger.isActivated()) {
            logger.debug("isGroupFileTransfer fileTransferId: " + fileTransferId);
        }
        Cursor cursor = null;
        try {
            Uri contentUri = Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId);
            cursor = cr.query(contentUri, PROJECTION_FILE_TRANSFER_ID,
                    SELECTION_BY_EQUAL_CHAT_ID_AND_CONTACT, null, null);
            CursorUtil.assertCursorIsNotNull(cursor, contentUri);

             /* For a one-to-one file transfer, value of chatID is equal to the value of contact*/

            boolean value =  !cursor.moveToNext();
            if (logger.isActivated()) {
                logger.debug("isGroupFileTransfer fileTransferId: " + fileTransferId + ", value : " + value);
            }
            return value;

        } finally {
            CursorUtil.close(cursor);
        }
    }

    public boolean setFileTransferStateAndTimestamp(String fileTransferId, FileTransfer.State state,
            FileTransfer.ReasonCode reasonCode, long timestamp, long timestampSent) {
        if (logger.isActivated()) {
            logger.debug("setFileTransferStateAndTimestamp: fileTransferId=" + fileTransferId
                    + ", state=" + state + ", reasonCode=" + reasonCode + ", timestamp="
                    + timestamp + ", timestampSent=" + timestampSent);
        }

        ContentValues values = new ContentValues();
        values.put(FileTransferData.KEY_STATE, state.toInt());
        values.put(FileTransferData.KEY_REASON_CODE, reasonCode.toInt());
        values.put(FileTransferData.KEY_TIMESTAMP, timestamp);
        values.put(FileTransferData.KEY_TIMESTAMP_SENT, timestampSent);
        return cr.update(
                Uri.withAppendedPath(FileTransferData.CONTENT_URI, fileTransferId), values, null,
                null) > 0;
    }

    public Cursor getGroupChatData(String chatId) {
        Uri contentUri = Uri.withAppendedPath(ChatData.CONTENT_URI, chatId);
        Cursor cursor = cr.query(contentUri, null, null, null, null);
        CursorUtil.assertCursorIsNotNull(cursor, contentUri);
        return cursor;
    }

    public State getGroupChatState(String chatId) {
        Cursor cursor = getGroupChatData(ChatData.KEY_STATE, chatId);
        if (cursor == null) {
            return null;
        }
        return State.valueOf(getDataAsInteger(cursor));
    }

    private Cursor getGroupChatData(String columnName, String chatId) {
        String[] projection = new String[] {
            columnName
        };
        Uri contentUri = Uri.withAppendedPath(ChatData.CONTENT_URI, chatId);
        Cursor cursor = cr.query(contentUri, projection, null, null, null);
        CursorUtil.assertCursorIsNotNull(cursor, contentUri);
        if (!cursor.moveToNext()) {
            CursorUtil.close(cursor);
            return null;
        }
        return cursor;
    }

    public ReasonCode getGroupChatReasonCode(String chatId) {
        Cursor cursor = getGroupChatData(ChatData.KEY_REASON_CODE, chatId);
        if (cursor == null) {
            return null;
        }
        return ReasonCode.valueOf(getDataAsInteger(cursor));
    }

    public boolean setGroupChatRejoinId(String chatId, String rejoinId, boolean updateStateToStarted) {
        if (logger.isActivated()) {
            logger.debug("Update group chat rejoin ID to ".concat(rejoinId));
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_REJOIN_ID, rejoinId);
        if (updateStateToStarted) {
            values.put(ChatData.KEY_STATE, State.STARTED.toInt());
            values.put(ChatData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
        }
        return cr.update(
                Uri.withAppendedPath(ChatData.CONTENT_URI, chatId), values, null, null) > 0;
    }

    public void addIncomingGroupChatMessage(String chatId, com.orangelabs.rcs.core.ims.service.im.chat.ChatMessage msg,
            boolean imdnDisplayedRequested) {
        Status chatMessageStatus = imdnDisplayedRequested ? Status.DISPLAY_REPORT_REQUESTED
                : Status.RECEIVED;
        addGroupChatMessage(chatId, msg, Direction.INCOMING, null, chatMessageStatus,
                com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode.UNSPECIFIED);
    }

    /**
     * Add group chat message
     *
     * @param chatId Chat ID
     * @param msg Chat message
     * @param direction Direction
     * @param status Status
     * @param reasonCode Reason code
     */
    private void addGroupChatMessage(String chatId, com.orangelabs.rcs.core.ims.service.im.chat.ChatMessage msg, Direction direction,
            Set<ContactId> recipients, Status status, com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode reasonCode) {
        String msgId = msg.getMessageId();
        ContactId contact = msg.getRemoteContact();
        if (logger.isActivated()) {
            logger.debug("Add group chat message; chatId=" + chatId + ", msg=" + msgId + ", dir="
                    + direction + ", contact=" + contact + ".");
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_CHAT_ID, chatId);
        values.put(MessageData.KEY_MESSAGE_ID, msgId);
        if (contact != null) {
            values.put(MessageData.KEY_CONTACT, contact.toString());
        }
        values.put(MessageData.KEY_DIRECTION, direction.toInt());
        values.put(MessageData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());
        values.put(MessageData.KEY_STATUS, status.toInt());
        values.put(MessageData.KEY_REASON_CODE, reasonCode.toInt());
        values.put(MessageData.KEY_MIME_TYPE, msg.getMimeType());
        values.put(MessageData.KEY_CONTENT, msg.getContent());
        values.put(MessageData.KEY_TIMESTAMP, msg.getTimestamp());
        values.put(MessageData.KEY_TIMESTAMP_SENT, msg.getTimestampSent());
        values.put(MessageData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, 0);
        values.put(MessageData.KEY_DELIVERY_EXPIRATION, 0);
        values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
        cr.insert(MessageData.CONTENT_URI, values);
        if (Direction.OUTGOING == direction) {
            try {
                GroupDeliveryInfo.Status deliveryStatus = GroupDeliveryInfo.Status.NOT_DELIVERED;
              /*  if (RcsSettings.getInstance().isAlbatrosRelease()) {
                    deliveryStatus = GroupDeliveryInfo.Status.UNSUPPORTED;
                }*/
                for (ContactId recipient : recipients) {
                    /* Add entry with delivered and displayed timestamps set to 0. */
                    mGroupChatDeliveryInfoLog.addGroupChatDeliveryInfoEntry(chatId, recipient.toString(),
                            msgId, deliveryStatus, GroupDeliveryInfo.ReasonCode.UNSPECIFIED, 0, 0);
                }
            } catch (Exception e) {
                cr.delete(Uri.withAppendedPath(MessageData.CONTENT_URI, msgId),
                        null, null);
                cr.delete(
                        Uri.withAppendedPath(GroupDeliveryInfoData.CONTENT_URI, msgId), null, null);
                if (logger.isActivated()) {
                    logger.warn("Group chat message with msgId '" + msgId
                            + "' could not be added to database!");
                }
            }
        }
    }

    public boolean setRejectNextGroupChatNextInvitation(String chatId) {
        if (logger.isActivated()) {
            logger.debug("setRejectNextGroupChatNextInvitation (chatId=" + chatId + ")");
        }
        ContentValues values = new ContentValues();
        values.put(ChatData.KEY_USER_ABORTION, UserAbortion.SERVER_NOT_NOTIFIED.toInt());
        return cr.update(
                Uri.withAppendedPath(ChatData.CONTENT_URI, chatId), values, null, null) > 0;
    }

    public void addGroupChatFailedDeliveryMessage(String chatId, com.orangelabs.rcs.core.ims.service.im.chat.ChatMessage msg) {
        addGroupChatMessage(chatId, msg, Direction.INCOMING, null, Status.FAILED,
                com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode.FAILED_DELIVERY);
    }

    /**
     * Get list of participants from a string
     *
     * @param String participants (contacts are comma separated)
     * @return String[] contacts or null if
     */
    public Map<String, ParticipantStatus> getParticipants(String chatId) {

        Cursor cursor = getGroupChatData(ChatData.KEY_PARTICIPANTS, chatId);
        if (cursor == null) {
            return null;
        }
        return parseEncodedParticipantInfos(getDataAsString(cursor));
    }

    /**
     * Get list of participants into a string
     *
     * @param participants List of participants
     * @return String (contacts are comma separated)
     */
    private static String getParticipants(List<String> participants) {
        StringBuffer result = new StringBuffer();
        for (String contact : participants) {
            if (contact != null) {
                result.append(PhoneUtils.extractNumberFromUri(contact) + ";");
            }
        }
        return result.toString();
    }

    public boolean setChatMessageStatusDelivered(String msgId, long timestampDelivered) {
        if (logger.isActivated()) {
            logger.debug("setChatMessageStatusDelivered msgId=" + msgId + ", timestampDelivered="
                    + timestampDelivered);
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_STATUS, Status.DELIVERED.toInt());
        values.put(MessageData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
        values.put(MessageData.KEY_TIMESTAMP_DELIVERED, timestampDelivered);
        values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
        return cr.update(Uri.withAppendedPath(MessageData.CONTENT_URI, msgId),
                values, SELECTION_BY_NOT_DISPLAYED, null) > 0;
    }

    public boolean setChatMessageStatusDisplayed(String msgId, long timestampDisplayed) {
        if (logger.isActivated()) {
            logger.debug("setChatMessageStatusDisplayed msgId=" + msgId + ", timestampDisplayed="
                    + timestampDisplayed);
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_STATUS, Status.DISPLAYED.toInt());
        values.put(MessageData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
        values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, timestampDisplayed);
        values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
        return cr.update(Uri.withAppendedPath(MessageData.CONTENT_URI, msgId),
                values, null, null) > 0;
    }

    public String addGroupChatEvent(String chatId, ContactId contact, GroupChatEvent.Status status,
            long timestamp) {
        if (logger.isActivated()) {
            logger.debug("Add group chat system message: chatID=" + chatId + ", contact="
                    + contact + ", status=" + status);
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_CHAT_ID, chatId);
        if (contact != null) {
            values.put(MessageData.KEY_CONTACT, contact.toString());
        }
        String msgId = IdGenerator.generateMessageID();
        values.put(MessageData.KEY_MESSAGE_ID, msgId);
        values.put(MessageData.KEY_MIME_TYPE, MimeType.GROUPCHAT_EVENT);
        values.put(MessageData.KEY_STATUS, status.toInt());
        values.put(MessageData.KEY_REASON_CODE, ReasonCode.UNSPECIFIED.toInt());
        values.put(MessageData.KEY_DIRECTION, Direction.IRRELEVANT.toInt());
        values.put(MessageData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());
        values.put(MessageData.KEY_TIMESTAMP, timestamp);
        values.put(MessageData.KEY_TIMESTAMP_SENT, timestamp);
        values.put(MessageData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, 0);
        values.put(MessageData.KEY_DELIVERY_EXPIRATION, 0);
        values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
        cr.insert(MessageData.CONTENT_URI, values);
        return msgId;
    }


    public void addOutgoingGroupChatMessage(String chatId, com.orangelabs.rcs.core.ims.service.im.chat.ChatMessage msg,
            Set<ContactId> recipients, Status status,  com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode reasonCode) {
        addGroupChatMessage(chatId, msg, Direction.OUTGOING, recipients, status, reasonCode);
    }

    public void addOutgoingOneToOneChatMessage(com.orangelabs.rcs.core.ims.service.im.chat.ChatMessage msg, Status status,
            com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode reasonCode, long deliveryExpiration) {
        ContactId contact = msg.getRemoteContact();
        String msgId = msg.getMessageId();
        if (logger.isActivated()) {
            logger.debug("Add outgoing chat message: contact=" + contact + ", msg=" + msgId
                    + ", status=" + status + ", reasonCode=" + reasonCode + ".");
        }
        ContentValues values = new ContentValues();
        values.put(MessageData.KEY_CHAT_ID, contact.toString());
        values.put(MessageData.KEY_MESSAGE_ID, msgId);
        values.put(MessageData.KEY_CONTACT, contact.toString());
        values.put(MessageData.KEY_DIRECTION, Direction.OUTGOING.toInt());
        values.put(MessageData.KEY_READ_STATUS, ReadStatus.UNREAD.toInt());
        values.put(MessageData.KEY_MIME_TYPE, msg.getMimeType());
        values.put(MessageData.KEY_CONTENT, msg.getContent());
        values.put(MessageData.KEY_TIMESTAMP, msg.getTimestamp());
        values.put(MessageData.KEY_TIMESTAMP_SENT, msg.getTimestampSent());
        values.put(MessageData.KEY_TIMESTAMP_DELIVERED, 0);
        values.put(MessageData.KEY_TIMESTAMP_DISPLAYED, 0);
        values.put(MessageData.KEY_DELIVERY_EXPIRATION, deliveryExpiration);
        values.put(MessageData.KEY_EXPIRED_DELIVERY, 0);
        values.put(MessageData.KEY_STATUS, status.toInt());
        values.put(MessageData.KEY_REASON_CODE, reasonCode.toInt());
        cr.insert(MessageData.CONTENT_URI, values);
    }

}
