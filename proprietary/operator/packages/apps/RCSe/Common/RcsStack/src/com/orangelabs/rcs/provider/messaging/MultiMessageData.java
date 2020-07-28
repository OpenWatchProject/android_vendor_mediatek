package com.orangelabs.rcs.provider.messaging;

import com.gsma.services.rcs.chat.ChatLog;

import android.net.Uri;

/**
 * Message data constants
 * 
 * @author Jean-Marc AUFFRET
 */
public class MultiMessageData {
    /**
     * Database URI
     */
    static public final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.chat/multimessage");
    
    /**
     * Column name
     */
    static final String KEY_ID = ChatLog.MultiMessage.ID;
    
    /**
     * Column name
     */
    static final String KEY_CHAT_ID = ChatLog.MultiMessage.CHAT_ID;

    static final String KEY_MESSAGE_ID = ChatLog.MultiMessage.MESSAGE_ID;

    /**
     * Column name
     */
    static final String KEY_STATUS = ChatLog.MultiMessage.STATE;

    /**
     * Column name
     */
    static final String KEY_SUBJECT = ChatLog.MultiMessage.SUBJECT;

    /**
     * Column name
     */
    static final String KEY_PARTICIPANTS = "participants";

    /**
     * Column name
     */
    static final String KEY_DIRECTION = ChatLog.MultiMessage.DIRECTION;    
    
    /**
     * Column name
     */
    static final String KEY_TIMESTAMP = ChatLog.MultiMessage.TIMESTAMP;
    
}
