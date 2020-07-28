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

import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.RcsGenericException;
import com.gsma.services.rcs.RcsIllegalArgumentException;
import com.gsma.services.rcs.RcsPermissionDeniedException;
import com.gsma.services.rcs.RcsPersistentStorageException;
import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.RcsUnsupportedOperationException;
import com.gsma.services.rcs.contact.ContactId;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.ft.FileTransfer;
//import com.gsma.services.rcs.ft.FileTransferListener;
import com.gsma.services.rcs.ft.IFileTransfer;

import com.gsma.services.rcs.Logger;

import android.util.SparseArray;

/**
 * Group chat
 *
 * @author Jean-Marc AUFFRET
 */
public class GroupChat {

    public static final String TAG = "TAPI-GroupChat";    
    
    /**
     * Group chat state
     */
    public enum State {

        /**
         * Chat invitation received
         */
        INVITED(0),

        /**
         * Chat invitation sent
         */
        INITIATING(1),

        /**
         * Chat is started
         */
        STARTED(2),

        /**
         * Chat has been aborted
         */
        ABORTED(3),

        /**
         * Chat has failed
         */
        FAILED(4),

        /**
         * Chat has been accepted and is in the process of becoming started.
         */
        ACCEPTING(5),

        /**
         * Chat invitation was rejected.
         */
        REJECTED(6),
        
        /**
         * Chat has been closed by the user. A user which has closed a
         * conversation voluntary can't rejoin it afterward.
         */
        CLOSED_BY_USER(7);

        private final int mValue;

        private static SparseArray<State> mValueToEnum = new SparseArray<State>();
        static {
            for (State state : State.values()) {
                mValueToEnum.put(state.toInt(), state);
            }
        }

        State(int value) {
            mValue = value;
        }

        /**
         * Gets integer value associated to State instance
         * 
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * Returns a State instance for the specified integer value.
         * 
         * @param value the value associated to the state
         * @return instance
         */
        public static State valueOf(int value) {
            State state = mValueToEnum.get(value);
            if (state != null) {
                return state;
            }
            throw new IllegalArgumentException("No enum const class " + State.class.getName() + ""
                    + value + "!");
        }
    }

    /**
     * Direction of the group chat
     *//*
    public static class Direction {
        *//**
         * Incoming chat
         *//*
        public static final int INCOMING = 0;

        *//**
         * Outgoing chat
         *//*
        public static final int OUTGOING = 1;
    }

    *//**
     * State of the group chat conference Notify
     *//*
    public static class ConfState {

        public final static String FULL = "full";
        public final static String PARTIAL = "partial";
    }*/
    
    /**
     * Group chat participant status
     */
    public enum ParticipantStatus {
        /**
         * Invite can not be sent, instead it has been queued
         */
        INVITE_QUEUED(0),
        /**
         * Participant is about to be invited
         */
        INVITING(1),
        /**
         * Participant is invited
         */
        INVITED(2),
        /**
         * Participant is connected
         */
        CONNECTED(3),
        /**
         * Participant disconnected
         */
        DISCONNECTED(4),
        /**
         * Participant has departed
         */
        DEPARTED(5),
        /**
         * Participant status is failed
         */
        FAILED(6),
        /**
         * Participant declined invitation
         */
        DECLINED(7),
        /**
         * Participant invitation has timed-out
         */
        TIMEOUT(8);

        private final int mValue;

        private static SparseArray<ParticipantStatus> mValueToEnum = new SparseArray<ParticipantStatus>();
        static {
            for (ParticipantStatus status : ParticipantStatus.values()) {
                mValueToEnum.put(status.toInt(), status);
            }
        }

        ParticipantStatus(int value) {
            mValue = value;
        }

        public final int toInt() {
            return mValue;
        }

        public static ParticipantStatus valueOf(int value) {
            ParticipantStatus status = mValueToEnum.get(value);
            if (status != null) {
                return status;
            }
            throw new IllegalArgumentException("No enum const class "
                    + ParticipantStatus.class.getName() + "" + value + "!");
        }
    }

     /**
     * Direction of the group chat
     *//*
    public static class MessageState {
        *//**
         * Message being sent
         *//*
        public static final int SENDING = 0;

        *//**
         * Message sent
         *//*
        public static final int SENT = 1;

        *//**
         * Message delivered to remote
         *//*
        public static final int DELIVERED = 2;

        *//**
         * Message sending failed
         *//*
        public static final int FAILED = 3;
    }*/

     /**
     * Direction of the group chat
     *//*
    public static class ErrorCodes {
        *//**
         * Message being sent
         *//*
        public static final int TIMEOUT = 1;

        *//**
         * Message sent
         *//*
        public static final int UNKNOWN = 2;

        *//**
         * Message delivered to remote
         *//*
        public static final int INTERNAL_EROR = 3;

        *//**
         * Message sending failed
         *//*
        public static final int OUT_OF_SIZE = 4;
    }*/

    /**
     * Group chat state reason code
     */
    public enum ReasonCode {

        /**
         * No specific reason code specified.
         */
        UNSPECIFIED(0),

        /**
         * Group chat is aborted by local user.
         */
        ABORTED_BY_USER(1),

        /**
         * Group chat is aborted by remote user.
         */
        ABORTED_BY_REMOTE(2),

        /**
         * Group chat is aborted by inactivity.
         */
        ABORTED_BY_INACTIVITY(3),

        /**
         * Group chat is rejected because already taken by the secondary device.
         */
        REJECTED_BY_SECONDARY_DEVICE(4),

        /**
         * Group chat invitation was rejected as it was detected as spam.
         */
        REJECTED_SPAM(5),

        /**
         * Group chat invitation was rejected due to max number of chats open already.
         */
        REJECTED_MAX_CHATS(6),

        /**
         * Group chat invitation was rejected by remote.
         */
        REJECTED_BY_REMOTE(7),

        /**
         * Group chat invitation was rejected by timeout.
         */
        REJECTED_BY_TIMEOUT(8),

        /**
         * Group chat invitation was rejected by system.
         */
        REJECTED_BY_SYSTEM(9),

        /**
         * Group chat initiation failed.
         */
        FAILED_INITIATION(10),

        /**
         * Group chat initiation failed.
         */
        REJECTED_MAX_PARTICIPANTS(11),

        /**
         * Group chat initiation failed.
         */
        FALLABCK_MMS(12);

        private final int mValue;

        private static SparseArray<ReasonCode> mValueToEnum = new SparseArray<>();
        static {
            for (ReasonCode reasonCode : ReasonCode.values()) {
                mValueToEnum.put(reasonCode.toInt(), reasonCode);
            }
        }

        ReasonCode(int value) {
            mValue = value;
        }

        /**
         * Gets integer value associated to ReasonCode instance
         * 
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * Returns a ReasonCode instance for the specified integer value.
         * 
         * @param value the value associated to the reason code
         * @return instance
         */
        public static ReasonCode valueOf(int value) {
            ReasonCode reasonCode = mValueToEnum.get(value);
            if (reasonCode != null) {
                return reasonCode;
            }
            throw new IllegalArgumentException("No enum const class " + ReasonCode.class.getName()
                    + "" + value + "!");
        }
    }

    /**
     * Group chat error
     *//*
    public static class Error {
        *//**
         * Group chat has failed
         *//*
        public final static int CHAT_FAILED = 0;

        *//**
         * Group chat invitation has been declined by remote
         *//*
        public final static int INVITATION_DECLINED = 1;

        *//**
         * Chat conversation not found
         *//*
        public final static int CHAT_NOT_FOUND = 2;

    	*//**
    	 * Group chat invitation cancelled by server
    	 *//*
    	public final static int INVITATION_FORBIDDEN = 3;
        *//**
         * Group chat invitation cancelled by server because of too manu participants
         *//*
        public final static int INVITATION_DECLINED_TOO_MANY_PARTICIPANTS = 4;
        private Error() {
        }
    }*/

    /**
     * Group chat interface
     */
    private IGroupChat mGroupChatInf;

    /**
     * Constructor
     *
     * @param chatIntf Group chat interface
     */
    GroupChat(IGroupChat chatIntf) {
        this.mGroupChatInf = chatIntf;
    }
    
    /**
     * @throws RcsGenericException Returns the chat ID
     * @return chat Id
     */
    public String getChatId() throws RcsGenericException {
        try {
            return mGroupChatInf.getChatId();

        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the direction of the group chat
     * 
     * @return Direction
     * @see Direction
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public Direction getDirection() throws RcsPersistentStorageException, RcsGenericException {
        try {
            return Direction.valueOf(mGroupChatInf.getDirection());

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the state of the group chat
     * 
     * @return State
     * @see State
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public State getState() throws RcsPersistentStorageException, RcsGenericException {
        try {
            return State.valueOf(mGroupChatInf.getState());

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the reason code of the state of the group chat
     * 
     * @return ReasonCode
     * @see ReasonCode
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public ReasonCode getReasonCode() throws RcsPersistentStorageException, RcsGenericException {
        try {
            return ReasonCode.valueOf(mGroupChatInf.getReasonCode());

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the remote contact
     * 
     * @return ContactId
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public ContactId getRemoteContact() throws RcsPersistentStorageException, RcsGenericException {
        try {
            return mGroupChatInf.getRemoteContact();

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the subject of the group chat
     * 
     * @return String
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public String getSubject() throws RcsPersistentStorageException, RcsGenericException {
        try {
            return mGroupChatInf.getSubject();

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the map of participants and associated status .
     * 
     * @return Map&lt;ContactId, ParticipantStatus&gt;
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    /*
     * Unchecked cast must be suppressed since AIDL provides a raw Map type that must be cast.
     */
    @SuppressWarnings("unchecked")
    public Map<ContactId, ParticipantStatus> getParticipants()
            throws RcsPersistentStorageException, RcsGenericException {
        try {
            Map<ContactId, Integer> apiParticipants = mGroupChatInf.getParticipants();
            Map<ContactId, ParticipantStatus> participants = new HashMap<>();

            for (Map.Entry<ContactId, Integer> apiParticipant : apiParticipants.entrySet()) {
                participants.put(apiParticipant.getKey(),
                        ParticipantStatus.valueOf(apiParticipant.getValue()));
            }

            return participants;

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the local timestamp of when the group chat invitation was initiated for outgoing
     * group chats or the local timestamp of when the group chat invitation was received for
     * incoming group chat invitations.
     * 
     * @return long
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public long getTimestamp() throws RcsPersistentStorageException, RcsGenericException {
        try {
            return mGroupChatInf.getTimestamp();

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it is possible to send messages in the group chat right now, else returns
     * false.
     * 
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public boolean isAllowedToSendMessage() throws RcsPersistentStorageException,
            RcsGenericException {
        try {
            return mGroupChatInf.isAllowedToSendMessage();

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Sends a text message to the group
     * 
     * @param text Message
     * @return ChatMessage
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public ChatMessage sendMessage(String text) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsGenericException {
        try {
            return new ChatMessage(mGroupChatInf.sendMessage(text));
            //IChatMessage chatMessage = null;
            //return chatMessage;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Sends a geoloc message
     * 
     * @param geoloc Geoloc info
     * @return ChatMessage
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public ChatMessage sendMessage(Geoloc geoloc) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsGenericException {
        try {
            return new ChatMessage(mGroupChatInf.sendMessage2(geoloc));
            //IChatMessage chatMessage = null;
            //return chatMessage;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }
    
    /**
     * Sends a text message to the group
     * 
     * @param text Message
     * @param msgType Type
     * @return ChatMessage
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public ChatMessage sendMessage(String text, int msgType) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsGenericException {
        try {
            return new ChatMessage(mGroupChatInf.sendMessageEx(text,msgType));
            //IChatMessage chatMessage = null;
            //return chatMessage;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Sends a text message to the group
     * 
     * @param msgId MessageId
     * @return ChatMessage
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void resendMessage(String msgId) throws RcsPermissionDeniedException,
            RcsPersistentStorageException, RcsGenericException {
        try {
            mGroupChatInf.resendMessage(msgId);
            //IChatMessage chatMessage = null;
            //return chatMessage;

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * This method should be called to notify the stack of if there is ongoing composing or not in
     * this GroupChat. If there is an ongoing chat session established with the remote side
     * corresponding to this GroupChat this means that a call to this method will send the
     * 'is-composing' event or the 'is-not-composing' event to the remote side. However since this
     * method can be called at any time even when there is no chat session established with the
     * remote side or when the stack is not even connected to the IMS server then the stack
     * implementation needs to hold the last given information (i.e. composing or not composing) in
     * memory and then send it later when there is an established session available to relay this
     * information on. Note: if this GroupChat corresponds to an incoming pending chat session and
     * the parameter IM SESSION START is 1 then the session is accepted before sending the
     * 'is-composing' event.
     * 
     * @param ongoing True is client application is composing
     * @throws RcsGenericException
     */
    public void setComposingStatus(boolean ongoing) throws RcsGenericException {
        try {
            mGroupChatInf.setComposingStatus(ongoing);
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it is possible to invite additional participants to the group chat right now,
     * else returns false.
     * 
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public boolean isAllowedToInviteParticipants() throws RcsPersistentStorageException,
            RcsGenericException {
        try {
            return mGroupChatInf.isAllowedToInviteParticipants();

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it is possible to invite the specified participant to the group chat right
     * now, else returns false.
     * 
     * @param participant the contact ID
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public boolean isAllowedToInviteParticipant(ContactId participant)
            throws RcsPersistentStorageException, RcsGenericException {
        try {
            return mGroupChatInf.isAllowedToInviteParticipant(participant);

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Invite additional participants to this group chat.
     * 
     * @param participants List of participants
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void inviteParticipants(Set<ContactId> participants)
            throws RcsPermissionDeniedException, RcsPersistentStorageException, RcsGenericException {
        try {
            mGroupChatInf.inviteParticipants(new ArrayList<>(participants));
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsUnsupportedOperationException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Remove participants from group chat.
     * 
     * @param participants List of participants
     * @throws RcsPermissionDeniedException
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void removeParticipants(Set<ContactId> participants)
            throws RcsPermissionDeniedException, RcsPersistentStorageException, RcsGenericException {
        try {
            mGroupChatInf.removeParticipants(new ArrayList<>(participants));
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsUnsupportedOperationException.assertException(e);
            RcsPermissionDeniedException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns the max number of participants in the group chat. This limit is read during the
     * conference event subscription and overrides the provisioning parameter.
     * 
     * @return int
     * @throws RcsGenericException
     */
    public int getMaxParticipants() throws RcsGenericException {
        try {
            return mGroupChatInf.getMaxParticipants();

        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it is possible to leave this group chat.
     * 
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public boolean isAllowedToLeave() throws RcsPersistentStorageException, RcsGenericException {
        try {
            return mGroupChatInf.isAllowedToLeave();

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Leaves a group chat willingly and permanently. The group chat will continue between other
     * participants if there are enough participants.
     * 
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void leave() throws RcsPersistentStorageException, RcsGenericException {
        try {
            mGroupChatInf.leave();
        } catch (Exception e) {
            RcsUnsupportedOperationException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * open the chat conversation.<br>
     * Note: if it is an incoming pending chat session and the parameter IM SESSION START is 0 then
     * the session is accepted now.
     * 
     * @throws RcsGenericException
     */
    public void openChat() throws RcsGenericException {
        try {
            mGroupChatInf.openChat();
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }
/*
    *//**
     * Returns the chat ID
     *
     * @return Session ID
     * @throws JoynServiceException
     *//*
    public String getChatSessionId() throws JoynServiceException {
        Logger.i(TAG, "getChatSessionId entry " + chatInf);
        try {
            return chatInf.getChatSessionId();
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }*/

    /**
     * Returns the state of the group chat
     *
     * @param messageId Message Id
     * @return State
     * @see GroupChat.State
     * @throws JoynServiceException
     *//*
    public int getMessageState(String messageId) throws JoynServiceException {
        Logger.i(TAG, "getState() entry " + chatInf + "Message Id = " + messageId);
        try {
            //return chatInf.getState();
            return chatInf.getMessageState(messageId);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * Accepts chat invitation
     *
     * @throws JoynServiceException
     *//*
    public void acceptInvitation() throws JoynServiceException {
        Logger.i(TAG, "acceptInvitation() entry " + chatInf);
        try {
            chatInf.acceptInvitation();
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * Rejects chat invitation
     *
     * @throws JoynServiceException
     *//*
    public void rejectInvitation() throws JoynServiceException {
        Logger.i(TAG, "rejectInvitation() entry " + chatInf);
        try {
            chatInf.rejectInvitation();
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * Sends a displayed delivery report for a given message ID
     *
     * @param msgId Message ID
     * @return State of the message
     * @throws JoynServiceException
     *//*
    public void sendDisplayedDeliveryReport(String msgId) throws JoynServiceException {
        Logger.i(TAG, "sendDisplayedDeliveryReport() entry " + msgId);
        try {
            chatInf.sendDisplayedDeliveryReport(msgId);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * Adds a listener on chat events
     *
     * @param listener Group chat event listener
     * @throws JoynServiceException
     *//*
    public void addEventListener(GroupChatListener listener) throws JoynServiceException {
        Logger.i(TAG, "addEventListener() entry " + listener);

        try {
            chatInf.addEventListener(listener);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * Removes a listener on chat events
     *
     * @param listener Group chat event listener
     * @throws JoynServiceException
     *//*
    public void removeEventListener(GroupChatListener listener) throws JoynServiceException {
        Logger.i(TAG, "removeEventListener() entry " + listener);
        try {
            chatInf.removeEventListener(listener);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }*/

    /**
     * Set the new chairman(chairman privilege).
     *
     * @param newChairman new chairman of the group, should be a group member
     * @throws JoynServiceException
     *//*
    public void transferChairman(String newChairman) throws JoynServiceException {
        Logger.i(TAG, "transferChairman() entry " + newChairman);
        try {
            chatInf.transferChairman(newChairman);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * modify subject of group(chairman privilege).
     *
     * @param newSubject new subject string
     * @throws JoynServiceException
     *//*
    public void modifySubject(String newSubject) throws JoynServiceException {
        Logger.i(TAG, "modifySubject() entry " + newSubject);
        try {
            chatInf.modifySubject(newSubject);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * modify nickname of participant.
     *
     * @param newNickname new nick name of participant
     * @throws JoynServiceException
     *//*
    public void modifyMyNickName(String newNickname) throws JoynServiceException {
        Logger.i(TAG, "modifyMyNickName() entry " + newNickname);
        try {
            chatInf.modifyMyNickName(newNickname);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }*/

    /**
     * chairman abort(leave) the group, Group session will abort
     *
     * @throws JoynServiceException
     *//*
    public void abortConversation() throws JoynServiceException {
        Logger.i(TAG, "abortConversation() entry ");
        try {
            chatInf.abortConversation();
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * If myself chairman of the group
     *
     * @return true/false
     * @throws JoynServiceException
     *//*
    public boolean isMeChairman() throws JoynServiceException {
        Logger.i(TAG, "isMeChairman() entry ");
        boolean flag = false;
        try {
            flag = chatInf.isMeChairman();
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
        return flag;
    }*/
}
