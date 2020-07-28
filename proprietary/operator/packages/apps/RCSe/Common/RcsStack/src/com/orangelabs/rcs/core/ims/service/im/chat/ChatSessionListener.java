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

package com.orangelabs.rcs.core.ims.service.im.chat;

import java.util.Date;
import java.util.List;
import java.util.Map;

import com.gsma.services.rcs.chat.ConferenceEventData.ConferenceUser;

import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;

/**
 * Chat session listener
 * 
 * @author Jean-Marc AUFFRET
 */
public interface ChatSessionListener extends ImsSessionListener {
    /**
     * New message received
     * 
     * @param message Message
     */
    public void handleReceiveMessage(InstantMessage message);
    
    /**
     * IM error
     * 
     * @param error Error
     */
    public void handleImError(ChatError error,String msgId);
    
    /**
     * Invite error
     * 
     * @param error Error
     */
    public void handleInviteError(ChatError error);
    
    /**
     * Is composing event
     * 
     * @param contact Contact
     * @param status Status
     */
    public void handleIsComposingEvent(String contact, boolean status);

    /**
     * Modify Subject Successful
     * 
     * @param subject subject
     */
    public void handleModifySubjectSuccessful(String subject);
    
    /**
     * Modify Subject BY remote
     * 
     * @param subject subject
     */
    public void handleModifySubjectByRemote(String subject);
    
    /**
     * Transfer chairman by other participant
     * 
     * @param newChairman new chairman
     */
    public void handleTransferChairmanByRemote(String newChairman);
    
    /**
     * Modify Subject Failed
     * 
     * @param statusCode statusCode
     */
    public void handleModifySubjectFailed(int statusCode);
    
    /**
     * Modify Nickname Successful
     * 
     * @param contact contact
     */
    public void handleModifyNicknameSuccessful(String contact, String newNickName);
    
    /**
     * Modify Subject Failed
     * 
     * @param statusCode statusCode
     */
    public void handleModifyNicknameFailed(String contact, int statusCode);
    
    /**
     * Modify Nickname BY remote
     * 
     * @param contact contact
     */
    public void handleModifyNicknameByRemote(String contact, String newNickname);
    /**
     * Transfer chairman Successful
     * 
     * @param subject subject
     */
    public void handleTransferChairmanSuccessful(String newChairman);
    
    /**
     * Abort Conversation successful
     * 
     * @param code reason code
     */
    public void handleAbortConversationResult(int reason, int code);
    
    /**
     * Transfer chairman Failed
     * 
     * @param statusCode statusCode
     */
    public void handleTransferChairmanFailed(int statusCode);

    /**
     * Remove Participant Successful
     * 
     * @param subject subject
     */
    public void handleRemoveParticipantSuccessful(String removedParticipant);
    
    /**
     * Remove Participant Failed
     * 
     * @param statusCode statusCode
     */
    public void handleRemoveParticipantFailed(int statusCode);

    /**
     * New conference event
     * 
     * @param contact Contact
     * @param contactDisplayname Contact display name
     * @param state State associated to the contact
     */
    public void handleConferenceEvent(String contact, String contactDisplayname, String state, String method, String userStateParameter, String conferenceState);

    
    /**
     * New conference event for op01
     * 
     * @param confState conference state
     */
    public void handleConferenceNotify(String confState, List<ConferenceUser> users);
    
    /**
     * Update participants 
     * 
     * 
     */
    public void handleConferenceNotify(Map<String, ParticipantStatus> participants);
    
    /**
     * New message delivery status
     * 
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String msgId, String status, String contact, int errorCode, String statusCode);

    /**
     * New message delivery status
     * 
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String msgId, String status, String contact, Date date);
    
    /**
     * Request to add participant is successful
     */
    public void handleAddParticipantSuccessful();
    
    /**
     * Request to add participant has failed
     * 
     * @param reason Error reason
     */
    public void handleAddParticipantFailed(String reason);
    
    /**
     * New geoloc message received
     * 
     * @param geoloc Geoloc message
     */
    public void handleReceiveGeoloc(GeolocMessage geoloc);
    
    /**
     * Group SIP bye received for group session
     * 
     * @param cause cause in 'cause' parameter
     */
    public void handleSessionTerminatedByRemote(String cause, String text);
    
    /**
     * handle Quit Conversation by User(not chairman)
     * 
     * @param code status Code
     */
    public void handleQuitConversationResult(int code);
}
