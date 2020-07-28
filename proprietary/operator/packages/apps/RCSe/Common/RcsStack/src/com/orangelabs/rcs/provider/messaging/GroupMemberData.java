package com.orangelabs.rcs.provider.messaging;

import com.gsma.services.rcs.chat.ChatLog;

import android.net.Uri;

public class GroupMemberData {
    /**
     * Database URI
     */
    static public final Uri CONTENT_URI = Uri.parse("content://com.orangelabs.rcs.chat/groupmember");
    
    
    /**
     * Column name
     */
    static final String KEY_ID = ChatLog.GroupChatMember.ID;
    
    
    /**
     * Column name
     */
    static final String KEY_CHAT_ID = ChatLog.GroupChatMember.CHAT_ID;

    /**
     * Column name
     */
    static final String KEY_MEMBER_NAME = ChatLog.GroupChatMember.GROUP_MEMBER_NAME;

    /**
     * Column name
     */
    static final String KEY_PORTRAIT = ChatLog.GroupChatMember.GROUP_MEMBER_PORTRAIT;

    /**
     * Column name
     */
    static final String KEY_CONTACT_TYPE = ChatLog.GroupChatMember.GROUP_MEMBER_TYPE;

    
    /**
     * Display name 
     */
    static final String KEY_CONTACT_NUMBER = ChatLog.GroupChatMember.GROUP_MEMBER_NUMBER;
    
    
}
