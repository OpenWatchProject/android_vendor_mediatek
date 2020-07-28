package com.orangelabs.rcs.provider.messaging;

import com.gsma.services.rcs.chat.ChatLog;

import android.net.Uri;

/**
 * Message data constants
 * 
 * @author Jean-Marc AUFFRET
 */
public class MessageData {
    /**
     * Database URIs
     */
    static public final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.chat/message");
    
    /**
     * History log member id
     */
    public static final int HISTORYLOG_MEMBER_ID = ChatLog.Message.HISTORYLOG_MEMBER_ID;
    
    /**
     * Unique history ID
     */
    /* package private */static final String KEY_BASECOLUMN_ID = ChatLog.Message.BASECOLUMN_ID;
    
    /**
     * Id of chat room
     */
    static final String KEY_CHAT_ID = ChatLog.Message.CHAT_ID;

    /**
     * ContactId formatted number of remote contact or null if the message is an outgoing group chat
     * message.
     */
    static final String KEY_CONTACT = ChatLog.Message.CONTACT;

    /**
     * Id of the message
     */
    static final String KEY_MESSAGE_ID = ChatLog.Message.MESSAGE_ID;

    /**
     * Content of the message (as defined by one of the mimetypes in ChatLog.Message.Mimetype)
     */
    static final String KEY_CONTENT = ChatLog.Message.CONTENT;
    
    /**
     * Multipurpose Internet Mail Extensions (MIME) type of message
     */
    static final String KEY_MIME_TYPE = ChatLog.Message.MIME_TYPE;
    
    /**
     * Status direction of message.
     * 
     * @see Direction
     */
    static final String KEY_DIRECTION = ChatLog.Message.DIRECTION;

    /**
     * @see Status
     */
    static final String KEY_STATUS = ChatLog.Message.STATUS;

    /**
     * Reason code associated with the message status.
     * 
     * @see ReasonCode
     */
    static final String KEY_REASON_CODE = ChatLog.Message.REASON_CODE;

    /**
     * This is set on the receiver side when the message has been displayed.
     * 
     * @see RcsService.ReadStatus for the list of status.
     */
    static final String KEY_READ_STATUS = ChatLog.Message.READ_STATUS;

    /**
     * Time when message inserted
     */
    static final String KEY_TIMESTAMP = ChatLog.Message.TIMESTAMP;
    
    /**
     * Time when message sent. If 0 means not sent.
     */
    static final String KEY_TIMESTAMP_SENT = ChatLog.Message.TIMESTAMP_SENT;
    
    /**
     * Time when message delivered. If 0 means not delivered
     */
    static final String KEY_TIMESTAMP_DELIVERED = ChatLog.Message.TIMESTAMP_DELIVERED;
    
    /**
     * Time when message displayed. If 0 means not displayed.
     */
    static final String KEY_TIMESTAMP_DISPLAYED = ChatLog.Message.TIMESTAMP_DISPLAYED;
    
    /**
     * If delivery has expired for this message. Values: 1 (true), 0 (false)
     */
    static final String KEY_EXPIRED_DELIVERY = ChatLog.Message.EXPIRED_DELIVERY;
    
    /**
     * Time when message delivery time-out will expire or 0 if this message is not eligible for
     * delivery expiration.
     */
    /* package private */static final String KEY_DELIVERY_EXPIRATION = "delivery_expiration";

    /**
     * Column name
     */
    static final String KEY_CONVERSATION_ID = ChatLog.Message.CONVERSATION_ID;

    /**
     * Column name
     */
    static final String KEY_TYPE = ChatLog.Message.MESSAGE_TYPE;

    /**
     * Display name 
     */
    static final String KEY_DISPLAY_NAME = ChatLog.Message.DISPLAY_NAME;
}
