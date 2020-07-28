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
package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.RcsPermissionDeniedException;
import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.GroupChat.State;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.contact.ContactUtil;

import java.io.ByteArrayInputStream;
import java.io.ObjectInputStream;
import java.util.HashMap;
import java.util.Map;

//import ChatLog.Message.Content.Status;
import android.content.Context;
import android.net.Uri;
import android.util.SparseArray;

import android.provider.BaseColumns;

/**
 * Content provider for chat history
 *
 * @author Jean-Marc AUFFRET
 */
public class ChatLog {
    /**
     * Group chat
     */
    public static class GroupChat {
        public static final Uri CONTENT_URI = Uri
                .parse("content://com.gsma.services.rcs.provider.chat/groupchat");

        /**
         * History log member id
         */
        public static final int HISTORYLOG_MEMBER_ID = 0;

        /**
         * The name of the column containing the unique id across provider tables.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String BASECOLUMN_ID = BaseColumns._ID;
        /**
         * The name of the column containing the unique ID for a row.
         * <P>Type: primary key</P>
         */
        public static final String ID = "_id";

        /**
         * The name of the column containing the unique ID of the group chat.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String CHAT_ID = "chat_id";

        /**
         * The name of the column containing the state of the group chat.
         * <P>
         * Type: INTEGER
         * </P>
         * 
         * @see State
         */
        public static final String STATE = "state";

        /**
         * The name of the column containing the reason code of the state of the group chat.
         * <P>
         * Type: INTEGER
         * </P>
         * 
         * @see ReasonCode
         */
        public static final String REASON_CODE = "reason_code";

        /**
         * The name of the column containing the subject of the group chat.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String SUBJECT = "subject";

        /**
         * The name of the column containing the direction of the group chat.
         * <P>
         * Type: INTEGER
         * </P>
         * 
         * @see Direction
         */
        public static final String DIRECTION = "direction";

        /**
         * The name of the column containing the time when group chat is created.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String TIMESTAMP = "timestamp";

        /**
         * The name of the column containing the list of participants and associated status.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String PARTICIPANTS = "participants";

        /**
         * ContactId formatted number of the inviter of the group chat or null if this is a group
         * chat initiated by the local user (ie outgoing group chat).
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String CONTACT = "contact";


        /*
         * M: CPM related changes
         */
        /**
         * The name of the column containing the conversation ID.(in case of CPM only its added)
         * <P>Type: TEXT</P>
         */
        public static final String CONVERSATION_ID = "conversation_id";

        /*
         *@: ENDS
         */
        /**
         * CMCC changes
         */

        /**
         * The name of the column containing the direction of the group chat.
         * <P>Type: INTEGER</P>
         * @see GroupChat.Direction
         */
        public static final String CHAIRMAN = "chairman";

        /**
         * The name of the column containing the direction of the group chat.
         * <P>Type: INTEGER</P>
         * @see GroupChat.Direction
         */
        public static final String NICKNAME = "nickname";

        public static final String ISBLOCKED = "isBlocked";
        
        /**
         * Utility method to get participants from its string representation in the ChatLog
         * provider.
         * 
         * @param ctx the context
         * @param participants Participants in string representation
         * @return Participants
         * @throws RcsPermissionDeniedException
         */
        public static Map<ContactId, ParticipantStatus> getParticipants(Context ctx,
                String participants) throws RcsPermissionDeniedException {
            ContactUtil contactUtils = ContactUtil.getInstance(ctx);
            String[] tokens = participants.split(",");
            Map<ContactId, ParticipantStatus> participantResult = new HashMap<ContactId, ParticipantStatus>();
            for (String participant : tokens) {
                String[] keyValue = participant.split("=");
                if (keyValue.length == 2) {
                    String contact = keyValue[0];
                    ParticipantStatus status = ParticipantStatus.valueOf(Integer
                            .parseInt(keyValue[1]));
                    participantResult.put(contactUtils.formatContact(contact), status);
                }
            }
            return participantResult;
        }
        
        private GroupChat() {
        }

    }

    /**
     * Chat message from a single chat or group chat
     */
    public static class Message {
        /**
         * Content provider URI for chat messages
         */
        public static final Uri CONTENT_URI = Uri
                .parse("content://com.gsma.services.rcs.provider.chat/chatmessage");

        /**
        * History log member id
        */
        public static final int HISTORYLOG_MEMBER_ID = 1;
        /**
         * Content provider URI for chat messages of a given conversation. In case of single chat
         * the conversation is identified by the contact phone number. In case of group chat the
         * the conversation is identified by the unique chat ID.
         */
        //public static final Uri CONTENT_CHAT_URI = Uri.parse("content://com.gsma.services.rcs.provider.chat/message/#");

        /**
         * The name of the column containing the unique id across provider tables.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String BASECOLUMN_ID = BaseColumns._ID;

        /**
         * The name of the column containing the unique ID for a row.
         * <P>Type: primary key</P>
         */
        public static final String ID = "_id";

        /**
         * The name of the column containing the chat ID.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String CHAT_ID = "chat_id";

        /**
         * The name of the column containing the message ID.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String MESSAGE_ID = "msg_id";

        /**
         * The name of the column containing the message status.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String STATUS = "status";
        
        /**
         * The name of the column containing the message status reason code.
         * <P>
         * Type: INTEGER
         * </P>
         * 
         * @see ReasonCode
         */
        public static final String REASON_CODE = "reason_code";

        /**
         * The name of the column containing the message read status.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String READ_STATUS = "read_status";

        /**
         * The name of the column containing the message direction.
         * <P>
         * Type: INTEGER
         * </P>
         * 
         * @see Direction
         */
        public static final String DIRECTION = "direction";

        /**
         * The name of the column containing the MSISDN of the remote contact.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String CONTACT = "contact";

        /**
         * The name of the column containing the type of message.
         * <P>Type: INTEGER</P>
         * @see ChatLog.Message.Type
         */
        public static final String MESSAGE_TYPE = "msg_type";

        /**
         * The name of the column containing the identity of the sender of the message.
         * <P>Type: TEXT</P>
         */
        public static final String CONTACT_NUMBER = "sender";


        /**
         * The name of the column containing the message content.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String CONTENT = "content";

        /**
         * The name of the column containing the time when message is created.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String TIMESTAMP = "timestamp";

        /**
         * The name of the column containing the time when message is sent. If 0 means not sent.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String TIMESTAMP_SENT = "timestamp_sent";

        /**
         * The name of the column containing the time when message is delivered. If 0 means not
         * delivered.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String TIMESTAMP_DELIVERED = "timestamp_delivered";

        /**
         * The name of the column containing the time when message is displayed. If 0 means not
         * displayed.
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String TIMESTAMP_DISPLAYED = "timestamp_displayed";
        
        /**
         * If delivery has expired for this message. Values: 1 (true), 0 (false)
         * <P>
         * Type: INTEGER
         * </P>
         */
        public static final String EXPIRED_DELIVERY = "expired_delivery";

        /**
         * The name of the column containing the MIME-TYPE of the message body.
         * <P>
         * Type: TEXT
         * </P>
         */
        public static final String MIME_TYPE = "mime_type";

        /**
         * Message MIME-types
         */
        public static class MimeType {

            /**
             * MIME-type of text messages
             */
            public static final String TEXT_MESSAGE = "text/plain";

            /**
             * MIME-type of geoloc messages
             */
            public static final String GEOLOC_MESSAGE = "application/geoloc";

            /**
             * MIME-type of group chat events
             */
            public static final String GROUPCHAT_EVENT = "rcs/groupchat-event";

            private MimeType() {
            }
        }


        /** M:for adding alias or display name
         * The name of the column containing the display name user. in case of O2O only in SYSTEM_MSG this value will be set.
         * in case of GROUP CHAT, it will be set for all messages
         * <P>Type: TEXT</P>
         */
        public static final String DISPLAY_NAME = "display_name";
        /**@*/

        /*
         * M: CPM related changes
         */
        /**
         * The name of the column containing the conversation ID.(in case of CPM only its added)
         * <P>Type: TEXT</P>
         */
        public static final String CONVERSATION_ID = "conversation_id";

        /*
         *@: ENDS
         */


        /**
         * Type of the message
         */
        public static class Type {
            /**
             * Content message
             */
            public static final int CONTENT = 0;

            /**
             * System message
             */
            public static final int SYSTEM = 1;

            /**
             * OP01 specific spam message
             */
            public static final int SPAM = 2;

            /**
             * OP01 specific burned message
             */
            public static final int BURN = 3;

            /**
             * OP01 specific public account message
             */
            public static final int PUBLIC = 4;

            /**
             * OP01 specific cloud message
             */
            public static final int CLOUD = 5;

            /**
             * OP01 specific emoticon message
             */
            public static final int EMOTICON = 6;

            /**
             * OP01 specific prosecute message
             */
            public static final int PROSECUTE = 7;

            /**
             * OP01 specific card message
             */
            public static final int CARD = 8;
        }
        
        public static class Content {
            /**
             * Status of the message
             */
            public enum Status {

                /**
                 * The message has been rejected
                 */
                REJECTED(0),

                /**
                 * The message is queued to be sent by rcs service when possible
                 */
                QUEUED(1),

                /**
                 * The message is in progress of sending
                 */
                SENDING(2),

                /**
                 * The message has been sent
                 */
                SENT(3),

                /**
                 * The message sending has been failed
                 */
                FAILED(4),

                /**
                 * The message has been delivered to the remote.
                 */
                DELIVERED(5),

                /**
                 * The message has been received and a displayed delivery report is requested
                 */
                DISPLAY_REPORT_REQUESTED(6),

                /**
                 * The message is delivered and no display delivery report is requested.
                 */
                RECEIVED(7),

                /**
                 * The message has been displayed
                 */
                DISPLAYED(8),
                
                /**
                 * The message has been delivered and a displayed delivery report is
                 * requested, but we don't know if the message has been read by the remote
                 */
                UNREAD_REPORT(9);

                private final int mValue;

                private static SparseArray<Status> mValueToEnum = new SparseArray<Status>();
                static {
                    for (Status entry : Status.values()) {
                        mValueToEnum.put(entry.toInt(), entry);
                    }
                }

                Status(int value) {
                    mValue = value;
                }

                public final int toInt() {
                    return mValue;
                }

                public static Status valueOf(int value) {
                    Status entry = mValueToEnum.get(value);
                    if (entry != null) {
                        return entry;
                    }
                    throw new IllegalArgumentException("No enum const class "
                            + Status.class.getName() + "" + value + "!");
                }
            }

            /**
             * Reason code of the message status
             */
            public enum ReasonCode {

                /**
                 * No specific reason code specified.
                 */
                UNSPECIFIED(0),

                /**
                 * Sending of the message failed.
                 */
                FAILED_SEND(1),

                /**
                 * Delivering of the message failed.
                 */
                FAILED_DELIVERY(2),

                /**
                 * Displaying of the message failed.
                 */
                FAILED_DISPLAY(3),

                /**
                 * Incoming one-to-one message was detected as spam.
                 */
                REJECTED_SPAM(4),
                
                /**
                 * Fallback to SMS
                 */
                FALLBACK(5),
                
                /**
                 * ANONYMITY NOT ALLOWED.
                 */
                ANONYMITY_NOT_ALLOWED(7),

                /**
                 * FUNCTION NOT ALLOWED.
                 */
                FUNCTION_NOT_ALLOWED(8),

                /**
                 * SIZE EXCEEDED.
                 */
                SIZE_EXCEEDED(9),

                /**
                 * TEST_ERROR.
                 */
                TEST_ERROR(10),

                /**
                 * NO DESTINATIONS.
                 */
                NO_DESTINATIONS(11),

                /**
                 * VERSION UNSUPPORTED.
                 */
                VERSION_UNSUPPORTED(12),

                /**
                 * SERVICE UNAUTHORIZED.
                 */
                SERVICE_UNAUTHORIZED(13),

                /**
                 * DECLINED.
                 */
                DECLINED(14),

                /**
                 * Fallabck to pager mode.
                 */
                FALLABCK_PAGER(15),

                /**
                 * Invitation Timeout.
                 */
                TIMEOUT(6),
                
                BAD_REQUEST(400),
                
                FORBIDDEN(403),

                REQUEST_TIMEDOUT(408),
                
                UNSUPPORTED_MEDIA(415),
                
                INTERVAL_SMALL(423),

                INTERNAL_ERROR(501);

                private final int mValue;

                private static SparseArray<ReasonCode> mValueToEnum = new SparseArray<ReasonCode>();
                static {
                    for (ReasonCode entry : ReasonCode.values()) {
                        mValueToEnum.put(entry.toInt(), entry);
                    }
                }

                ReasonCode(int value) {
                    mValue = value;
                }

                public final int toInt() {
                    return mValue;
                }

                public static ReasonCode valueOf(int value) {
                    ReasonCode entry = mValueToEnum.get(value);
                    if (entry != null) {
                        return entry;
                    }
                    throw new IllegalArgumentException("No enum const class "
                            + ReasonCode.class.getName() + "" + value + "!");
                }
            }

            private Content() {
            }
        }       

        /**
         * Direction of the message
         *//*
        public static class Direction {
            *//**
             * Incoming message
             *//*
            public static final int INCOMING = 0;

            *//**
             * Outgoing message
             *//*
            public static final int OUTGOING = 1;

            *//**
             * Irrelevant or not applicable (e.g. for a system message)
             *//*
            public static final int IRRELEVANT = 2;
        }*/

        /**
         * Status of the message
         *//*
        public static class Status {
            *//**
             * Status of a content message
             *//*
            public static class Content {
                *//**
                 * The message has been delivered, but we don't know if the message
                 * has been read by the remote
                 *//*
                public static final int UNREAD = 0;

                *//**
                 * The message has been delivered and a displayed delivery report is
                 * requested, but we don't know if the message has been read by the remote
                 *//*
                public static final int UNREAD_REPORT = 1;

                *//**
                 * The message has been read by the remote (i.e. displayed)
                 *//*
                public static final int READ = 2;

                *//**
                 * The message is in progress of sending
                 *//*
                public static final int SENDING = 3;

                *//**
                 * The message has been sent
                 *//*
                public static final int SENT = 4;

                *//**
                 * The message is failed to be sent
                 *//*
                public static final int FAILED = 5;

                *//**
                 * The message is queued to be sent by joyn service when possible
                 *//*
                public static final int TO_SEND = 6;

                *//**
                 * The message is a spam message
                 *//*
                public static final int BLOCKED = 7;
            }

            *//**
             * Status of the system message
             *//*
            public static class System {
                *//**
                 * Invitation of a participant is pending
                 *//*
                public static final int PENDING = 0;

                *//**
                 * Invitation accepted by a participant
                 *//*
                public static final int ACCEPTED = 1;

                *//**
                 * Invitation declined by a participant
                 *//*
                public static final int DECLINED = 2;

                *//**
                 * Invitation of a participant has failed
                 *//*
                public static final int FAILED = 3;

                *//**
                 * Participant has joined the group chat
                 *//*
                public static final int JOINED = 4;

                *//**
                 * Participant has left the group chat (i.e. departed)
                 *//*
                public static final int GONE = 5;

                *//**
                 * Participant has been disconnected from the group chat (i.e. booted)
                 *//*
                public static final int DISCONNECTED = 6;

                *//**
                 * Participant is busy
                 *//*
                public static final int BUSY = 7;
            }
        }*/
    }
    
    public static class GroupChatEvent {
        /**
         * Status of group chat event message
         */
        public enum Status {

            /**
             * JOINED.
             */
            JOINED(0),

            /**
             * DEPARTED.
             */
            DEPARTED(1);

            private final int mValue;

            private static SparseArray<Status> mValueToEnum = new SparseArray<Status>();
            static {
                for (Status entry : Status.values()) {
                    mValueToEnum.put(entry.toInt(), entry);
                }
            }

            Status(int value) {
                mValue = value;
            }

            public final int toInt() {
                return mValue;
            }

            public static Status valueOf(int value) {
                Status entry = mValueToEnum.get(value);
                if (entry != null) {
                    return entry;
                }
                throw new IllegalArgumentException("No enum const class "
                        + Status.class.getName() + "" + value + "!");
            }
        }

        private GroupChatEvent() {
        }
    }

    /**
     * Chat message from a single chat or group chat
     */
    public static class MultiMessage {
        /**
         * Content provider URI for chat messages
         */
        public static final Uri CONTENT_URI = Uri.parse("content://com.gsma.services.rcs.provider.chat/multimessage");

        /**
         * Content provider URI for chat messages of a given conversation. In case of single chat
         * the conversation is identified by the contact phone number. In case of group chat the
         * the conversation is identified by the unique chat ID.
         */
        public static final Uri CONTENT_CHAT_URI = Uri.parse("content://com.gsma.services.rcs.provider.chat/multimessage/#");


        /**
         * The name of the column containing the unique ID for a row.
         * <P>Type: primary key</P>
         */
        public static final String ID = "_id";

        /**
         * The name of the column containing the unique ID of the group chat.
         * <P>Type: TEXT</P>
         */
        public static final String CHAT_ID = "chat_id";

        public static final String MESSAGE_ID = "msg_id";
        /**
         * The name of the column containing the state of the group chat.
         * <P>Type: INTEGER</P>
         * @see GroupChat.State
         */
        public static final String STATE = "state";

        /**
         * The name of the column containing the subject of the group chat.
         * <P>Type: TEXT</P>
         */
        public static final String SUBJECT = "subject";


        /**
         * The name of the column containing the subject of the group chat.
         * <P>Type: TEXT</P>
         */
        public static final String PARTICIPANTS_LIST = "participants";

        /**
         * The name of the column containing the direction of the group chat.
         * <P>Type: INTEGER</P>
         * @see GroupChat.Direction
         */
        public static final String DIRECTION = "direction";

        /**
         * The name of the column containing the time when group chat is created.
         * <P>Type: LONG</P>
         */
        public static final String TIMESTAMP = "timestamp";

        /*
         *@: ENDS
         */


        /**
         * Type of the message
         */
        public static class Type {
            /**
             * Content message
             */
            public static final int CONTENT = 0;

            /**
             * System message
             */
            public static final int SYSTEM = 1;

            /**
             * Spam message
             */
            public static final int SPAM = 2;
        }

        /**
         * Direction of the message
         */
        public static class Direction {
            /**
             * Incoming message
             */
            public static final int INCOMING = 0;

            /**
             * Outgoing message
             */
            public static final int OUTGOING = 1;

            /**
             * Irrelevant or not applicable (e.g. for a system message)
             */
            public static final int IRRELEVANT = 2;
        }

        /**
         * Status of the message
         */
        public static class Status {
            /**
             * Status of a content message
             */
            public static class Content {
                /**
                 * The message has been delivered, but we don't know if the message
                 * has been read by the remote
                 */
                public static final int UNREAD = 0;

                /**
                 * The message has been delivered and a displayed delivery report is
                 * requested, but we don't know if the message has been read by the remote
                 */
                public static final int UNREAD_REPORT = 1;

                /**
                 * The message has been read by the remote (i.e. displayed)
                 */
                public static final int READ = 2;

                /**
                 * The message is in progress of sending
                 */
                public static final int SENDING = 3;

                /**
                 * The message has been sent
                 */
                public static final int SENT = 4;

                /**
                 * The message is failed to be sent
                 */
                public static final int FAILED = 5;

                /**
                 * The message is queued to be sent by joyn service when possible
                 */
                public static final int TO_SEND = 6;

                /**
                 * The message is a spam message
                 */
                public static final int BLOCKED = 7;
            }

            /**
             * Status of the system message
             */
            public static class System {
                /**
                 * Invitation of a participant is pending
                 */
                public static final int PENDING = 0;

                /**
                 * Invitation accepted by a participant
                 */
                public static final int ACCEPTED = 1;

                /**
                 * Invitation declined by a participant
                 */
                public static final int DECLINED = 2;

                /**
                 * Invitation of a participant has failed
                 */
                public static final int FAILED = 3;

                /**
                 * Participant has joined the group chat
                 */
                public static final int JOINED = 4;

                /**
                 * Participant has left the group chat (i.e. departed)
                 */
                public static final int GONE = 5;

                /**
                 * Participant has been disconnected from the group chat (i.e. booted)
                 */
                public static final int DISCONNECTED = 6;

                /**
                 * Participant is busy
                 */
                public static final int BUSY = 7;
            }
        }
    }


    public static class GroupChatMember {
        /**
         * Content provider URI for chat conversations
         */
        public static final Uri CONTENT_URI = Uri.parse("content://com.gsma.services.rcs.provider.chat/groupmember");


        /**
         *
         * <P>Type: TEXT</P>
         */
        public static final String ID = "_id";


        /**
         *
         * <P>Type: TEXT</P>
         */
        public static final String CHAT_ID = "CHAT_ID";


        /**
         *
         * <P>Type: TEXT</P>
         */
        public static final String GROUP_MEMBER_NAME = "MEMBER_NAME";

        /**
         * .
         * <P>Type: INTEGER</P>
         * @see GroupChat.Direction
         */
        public static final String GROUP_MEMBER_PORTRAIT = "PORTRAIT";

        /**
         * The name of the column containing the direction of the group chat.
         * <P>Type: INTEGER</P>
         * @see GroupChat.Direction
         */
        public static final String GROUP_MEMBER_NUMBER = "CONTACT_NUMBER";


        /**
         * The name of the column containing the direction of the group chat.
         * <P>Type: INTEGER</P>
         * @see GroupChat.Direction
         */
        public static final String GROUP_MEMBER_TYPE = "CONTACT_ETYPE";



        /*
         * M: CPM related changes
         */
        /**
         * The name of the column containing the conversation ID.(in case of CPM only its added)
         * <P>Type: TEXT</P>
         */
        public static final String CONVERSATION_ID = "conversation_id";

        /*
         *@: ENDS
         */
    }

    /**
     * Get plain text message from a BLOB
     *
     * @param content BLOB content
     * @return Text message or null in case of error
     */
    public static String getTextFromBlob(byte[] content) {
        try {
            return new String(content);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Get geoloc object from a BLOB
     *
     * @param content BLOB content
     * @return Geoloc object or null in case of error
     * @see Geoloc
     */
    public static Geoloc getGeolocFromBlob(byte[] content) {
        try {
            ByteArrayInputStream bis = new ByteArrayInputStream(content);
            ObjectInputStream is = new ObjectInputStream(bis);
            Geoloc geoloc = (Geoloc) is.readObject();
            is.close();
            return geoloc;
        } catch (Exception e) {
            return null;
        }
    }
}
