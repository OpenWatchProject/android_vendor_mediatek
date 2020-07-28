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

import com.gsma.services.rcs.chat.ChatLog;

import android.net.Uri;

/**
 * Chat data constants
 * 
 * @author Jean-Marc AUFFRET
 */
public class ChatData {
    /**
     * Database URIs
     */
    public static final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.chat/chat");
    
    /**
     * History log member id
     */
    public static final int HISTORYLOG_MEMBER_ID = ChatLog.GroupChat.HISTORYLOG_MEMBER_ID;

    /**
     * Base column unique id
     */
    static final String KEY_BASECOLUMN_ID = ChatLog.GroupChat.BASECOLUMN_ID;
    
    /**
     * Id for chat room
     */
    static final String KEY_CHAT_ID = ChatLog.GroupChat.CHAT_ID;

    /**
     * State of chat room.
     * 
     * @see GroupChat.State
     */
    static final String KEY_STATE = ChatLog.GroupChat.STATE;

    /**
     * Reason code associated with the group chat state.
     * 
     * @see GroupChat.ReasonCode
     */
    static final String KEY_REASON_CODE = ChatLog.GroupChat.REASON_CODE;

    /**
     * Subject of the group chat room
     */
    static final String KEY_SUBJECT = ChatLog.GroupChat.SUBJECT;

    /**
     * List of participants and associated status stored as a String parseable with the
     * ChatLog.GroupChat.getParticipantInfos() method.
     */
    static final String KEY_PARTICIPANTS = ChatLog.GroupChat.PARTICIPANTS;

    /**
     * Status direction of group chat
     * 
     * @see RcsService.Direction
     */
    static final String KEY_DIRECTION = ChatLog.GroupChat.DIRECTION;    
    
    /**
     * Timestamp of the invitation
     */
    static final String KEY_TIMESTAMP = ChatLog.GroupChat.TIMESTAMP;

    /**
     * ContactId formatted number of the inviter of the group chat or null if this is a group chat
     * initiated by the local user (ie outgoing group chat).
     */
    static final String KEY_CONTACT = ChatLog.GroupChat.CONTACT;

    /**
     * Column name
     */
    static final String KEY_REJOIN_ID = "rejoin_id";

    /**
     * Column name : Departed by user
     */
    static final String KEY_USER_ABORTION = "user_abortion";
    
    /**
     * Column name
     */
    static final String KEY_CONVERSATION_ID = ChatLog.GroupChat.CONVERSATION_ID;
    
    
    /**
     * CMCC CHANGES
     */
    /**
     * Column name
     */
    static final String KEY_CHAIRMAN = ChatLog.GroupChat.CHAIRMAN;    
    
    /**
     * Column name
     */
    static final String KEY_NICKNAME = ChatLog.GroupChat.NICKNAME;    
    
    static final String KEY_ISBLOCKED = ChatLog.GroupChat.ISBLOCKED;
    
}
