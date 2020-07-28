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

import javax2.sip.header.SubjectHeader;
import javax2.sip.header.WarningHeader;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;


import android.os.Handler;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsSessionBasedServiceError;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.geoloc.GeolocInfoDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.iscomposing.IsComposingInfo;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpInfoDocument;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.StringUtils;

/**
 * Abstract 1-1 chat session
 *
 * @author Jean-Marc AUFFRET
 */
public abstract class OneOneChatSession extends ChatSession {
    /**
     * Boundary tag
     */
    private final static String BOUNDARY_TAG = "boundary1";
    private int retryRegisterCount = 0;
    private int retryServiceCount = 0;
    private int retryMsrpCount = 0;
    private int retry502Count = 0;
    private int retry482Count = 0;
    private int retry480Count = 0;



    /**
     * Constructor
     *
     * @param parent IMS service
     * @param contact Remote contact
     */
    public OneOneChatSession(ImsService parent, String contact) {
        super(parent, contact, OneOneChatSession.generateOneOneParticipants(contact));

        if(!RcsSettings.getInstance().isCPMSupported()){
        // Set feature tags
        setFeatureTags(ChatUtils.getSupportedFeatureTagsForChat());
        }
        else{
            if (logger.isActivated()) {
                logger.info("CPMS OneOneChatSession ");
            }
            // Set feature tags
            setCpimFeatureTags(ChatUtils.getCpimSupportedFeatureTagsForChat());
        }

        // Set accept-types
        String acceptTypes = CpimMessage.MIME_TYPE + " " + IsComposingInfo.MIME_TYPE;
        setAcceptTypes(acceptTypes);

        // Set accept-wrapped-types
        String wrappedTypes = InstantMessage.MIME_TYPE + " " + ImdnDocument.MIME_TYPE;
        if (RcsSettings.getInstance().isGeoLocationPushSupported()) {
            wrappedTypes += " " + GeolocInfoDocument.MIME_TYPE;
        }
        if (RcsSettings.getInstance().isFileTransferHttpSupported()) {
            wrappedTypes += " " + FileTransferHttpInfoDocument.MIME_TYPE;
        }
        setWrappedTypes(wrappedTypes);
    }

    /**
     * Is group chat
     *
     * @return Boolean
     */
    public boolean isGroupChat() {
        return false;
    }

    /**
     * Generate the list of participants for a 1-1 chat
     *
     * @param contact Contact
     * @return List of participants
     */
    private static ListOfParticipant generateOneOneParticipants(String contact) {
        ListOfParticipant list = new ListOfParticipant();
        list.addParticipant(contact);
        return list;
    }

    /**
     * Returns the list of participants currently connected to the session
     *
     * @return List of participants
     */
    public ListOfParticipant getConnectedParticipants() {
        return getParticipants();
    }

    /**
     * Close media session
     */
    public void closeMediaSession() {
        // Stop the activity manager
        getActivityManager().stop();

        // Close MSRP session
        closeMsrpSession();
    }

    /**
     * Send a text message
     *
     * @param id Message-ID
     * @param txt Text message
     */
    public void sendTextMessage(String msgId, String txt) {
        boolean useImdn = getImdnManager().isImdnActivated();
        String mime = CpimMessage.MIME_TYPE;
        String from = ChatUtils.ANOMYNOUS_URI;
        String to = ChatUtils.ANOMYNOUS_URI;

        String content;
        if (useImdn) {
            // Send message in CPIM + IMDN
            if(RcsSettings.getInstance().isImDisplayedNotificationActivated()){
                content = ChatUtils.buildCpimMessageWithImdn(from, to, msgId, StringUtils.encodeUTF8(txt), InstantMessage.MIME_TYPE);
            } else {
                content = ChatUtils.buildCpimMessageWithoutDisplay(from, to, msgId, StringUtils.encodeUTF8(txt), InstantMessage.MIME_TYPE);

            }
        } else {
            // Send message in CPIM
            content = ChatUtils.buildCpimMessage(from, to, StringUtils.encodeUTF8(txt), InstantMessage.MIME_TYPE);
        }

        // Update rich messaging history
        if (!RichMessagingHistory.getInstance().isOne2OneMessageExists(msgId)) {
            if (logger.isActivated()) {
                logger.info("CPMS sendTextMessage Add in DB Msgid:" + msgId );
            }
            InstantMessage msg = new InstantMessage(msgId, PhoneUtils.extractNumberFromUri(getRemoteContact()), txt, useImdn,null);
            RichMessagingHistory.getInstance().addChatMessage(msg, Direction.OUTGOING.toInt());
        }

        // Send content
        boolean result = sendDataChunks(msgId, content, mime);

        // Check if message has been sent with success or not
        if (!result) {
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateChatMessageStatus(msgId, ChatLog.Message.Content.Status.FAILED.toInt());

            // Notify listeners
            for(int i=0; i < getListeners().size(); i++) {
                ((ChatSessionListener)getListeners().get(i)).handleMessageDeliveryStatus(
                        msgId, ImdnDocument.DELIVERY_STATUS_SENDING_FAILED,this.getRemoteContact(),ImdnDocument.UNKNOWN,null);
            }
        }
    }

    public void handleMsrp400(final String msgId, String error) {
        if (logger.isActivated()) {
            logger.info("handleMsrp400 msgId: " + msgId + "; retryMsrpCount:" + retryMsrpCount);
        }
        if(retryMsrpCount < 1){
            retryMsrpCount++;
            // Send text message
            Thread t = new Thread() {
                public void run() {
                    String text = RichMessagingHistory.getInstance().getMessageText(msgId);
                    if (logger.isActivated()) {
                        logger.info("handleMsrp400 text: " + text);
                    }
                    sendTextMessage(msgId, text);
                }
            };
            t.start();
        } else {
            retryMsrpCount = 0;
            // Notify listeners
            for(int i=0; i < getListeners().size(); i++) {
                ((ChatSessionListener)getListeners().get(i)).handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_FAILED,this.getRemoteContact(),ImdnDocument.UNKNOWN,error);
            }
        }
    }

    public void handleMsrp413(final String msgId, String error) {
        if (logger.isActivated()) {
            logger.info("handleMsrp413 msgId: " + msgId + "; retryMsrpCount:" + retryMsrpCount);
        }
        final String newMsgid = ChatUtils.generateMessageId();
        // Send text message
        Thread t = new Thread() {
            public void run() {
                String text = RichMessagingHistory.getInstance().getMessageText(msgId);
                if (logger.isActivated()) {
                    logger.info("handleMsrp413 text: " + text);
                }
                sendTextMessage(newMsgid, text);
            }
        };
        t.start();

    }

    public void handleMsrp403(final String msgId, String error) {
        if (logger.isActivated()) {
            logger.info("handleMsrp403 msgId: " + msgId);
        }
        terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
        // Notify listeners
        for(int i=0; i < getListeners().size(); i++) {
            ((ChatSessionListener)getListeners().get(i)).handleMessageDeliveryStatus(this.getRemoteContact(),msgId, ImdnDocument.DELIVERY_STATUS_FAILED,ImdnDocument.UNKNOWN,error);
        }
    }

    public void handleResponseTimeout(ImsServiceError error) {
        if (logger.isActivated()) {
            logger.info("handleResponseTimeout");
        }

        if (isSessionInterrupted()) {
            return;
        }

        // Error
        if (logger.isActivated()) {
            logger.error("handleResponseTimeout Session error: " + error.getErrorCode() + ", reason=" + error.getMessage());
        }

        if(!RcsSettings.getInstance().isSupportOP07()){
            if(RcsSettings.getInstance().isSupportOP08() && !(this.isRingingRecieved()) && !(this.isTryingRecieved()))
                terminateSipSession(ImsServiceSession.TERMINATION_BY_SIP_TIMEOUT);
            else
                terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
        }
        // Close media session
        closeMediaSession();

        // Remove the current session
        getImsService().removeSession(this);

        // Notify listeners
        for (int i = 0; i < getListeners().size(); i++) {
            ((ChatSessionListener) getListeners().get(i)).handleImError(new ChatError(error),null);
        }
    }

    /**
     * Send a geoloc message
     *
     * @param msgId Message ID
     * @param geoloc Geoloc info
     */
    public void sendGeolocMessage(String msgId, GeolocPush geoloc) {
        boolean useImdn = getImdnManager().isImdnActivated();
        String mime = CpimMessage.MIME_TYPE;
        String from = ChatUtils.ANOMYNOUS_URI;
        String to = ChatUtils.ANOMYNOUS_URI;
        //TODO
        String geoDoc = ChatUtils.buildGeolocDocument(null, ImsModule.IMS_USER_PROFILE.getPublicUri(), msgId);

        String content;
        if (useImdn) {
            // Send message in CPIM + IMDN
            content = ChatUtils.buildCpimMessageWithImdn(from, to, msgId, geoDoc, GeolocInfoDocument.MIME_TYPE);
        } else {
            // Send message in CPIM
            content = ChatUtils.buildCpimMessage(from, to, geoDoc, GeolocInfoDocument.MIME_TYPE);
        }

        // Send content
        boolean result = sendDataChunks(msgId, content, mime);

        // Update rich messaging history
        GeolocMessage geolocMsg = new GeolocMessage(msgId, getRemoteContact(), geoloc, useImdn);
        RichMessagingHistory.getInstance().addChatMessage(geolocMsg, Direction.OUTGOING.toInt());

        // Check if message has been sent with success or not
        if (!result) {
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateChatMessageStatus(msgId, ChatLog.Message.Content.Status.FAILED.toInt());

            // Notify listeners
            for(int i=0; i < getListeners().size(); i++) {
                ((ChatSessionListener)getListeners().get(i)).handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_FAILED, null, null);
            }
        }
    }

    /**
     * Send is composing status
     *
     * @param status Status
     */
    public void sendIsComposingStatus(boolean status) {
        if (logger.isActivated()) {
            logger.info("CPMS OneOneChatSession sendIsComposingStatus ");
        }
        String content = IsComposingInfo.buildIsComposingInfo(status);
        String msgId = ChatUtils.generateMessageId();
        sendDataChunks(msgId, content, IsComposingInfo.MIME_TYPE,true);
    }

    /**
     * Reject the session invitation
     */
    public void rejectSession() {
        rejectSession(486);
    }

    /**
     * Add a participant to the session
     *
     * @param participant Participant
     */
    public void addParticipant(String participant) {
        ArrayList<String> participants = new ArrayList<String>();
        participants.add(participant);
        addParticipants(participants);
    }

    /**
     * Add a list of participants to the session
     *
     * @param participants List of participants
     */
    public void addParticipants(List<String> participants) {
        // Build the list of participants
        String existingParticipant = getParticipants().getList().get(0);
        participants.add(existingParticipant);

        if (logger.isActivated()) {
            logger.info("CPMS OneOneChatSession addParticipants ");
        }

        Map<String, ParticipantStatus> mParticipants = ChatUtils.changeParticipantsListToMap(new ListOfParticipant(participants),ParticipantStatus.INVITING);

        // Create a new session
        ExtendOneOneChatSession session = new ExtendOneOneChatSession(
            getImsService(),
            ImsModule.IMS_USER_PROFILE.getImConferenceUri(),
            this,
            mParticipants);

        // Start the session
        session.startSession();
    }

    /**
     * Create INVITE request
     *
     * @param content Content part
     * @return Request
     * @throws SipException
     */
    private SipRequest createMultipartInviteRequest(String content) throws SipException {
        SipRequest invite = null;
        if(!RcsSettings.getInstance().isCPMSupported()){
             invite = SipMessageFactory.createMultipartInvite(getDialogPath(),
                    getFeatureTags(),
                    content,
                    BOUNDARY_TAG);
        }
        else{
            if (logger.isActivated()) {
                logger.info("CPMS OneOneChatSession createMultipartInviteRequest0 ");
            }
            invite = SipMessageFactory.createCpmMultipartInvite(getDialogPath(),
                    InstantMessagingService.CPM_CHAT_FEATURE_TAGS,
                    content,
                    BOUNDARY_TAG,false);
        }

        // Test if there is a text message
        if ((getFirstMessage() != null) && (getFirstMessage().getTextMessage() != null)) {
            // Add a subject header
            //invite.addHeader(SubjectHeader.NAME, StringUtils.encodeUTF8(getFirstMessage().getTextMessage()));
        }

        // Add a contribution ID header
        invite.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID());
        if(RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.info("CPMS OneOneChatSession createMultipartInviteRequest 1");
            }
            if(getConversationID() != null){
                invite.addHeader(ChatUtils.HEADER_CONVERSATION_ID, getConversationID());
            }
        }

        return invite;
    }

    /**
     * Create INVITE request
     *
     * @param content Content part
     * @return Request
     * @throws SipException
     */
    private SipRequest createInviteRequest(String content) throws SipException {

        SipRequest invite = null;
        if(!RcsSettings.getInstance().isCPMSupported()){
            invite = SipMessageFactory.createInvite(getDialogPath(),
                    InstantMessagingService.CHAT_FEATURE_TAGS,
                    content);
        }
        else{
            if (logger.isActivated()) {
                logger.info("CPMS OneOneChatSession createInviteRequest 0");
            }
            invite = SipMessageFactory.createCpmInvite(getDialogPath(),
                    InstantMessagingService.CPM_CHAT_FEATURE_TAGS,
                    content,false);
        }

        // Add a contribution ID header
        invite.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID());
        if(RcsSettings.getInstance().isCPMSupported()){
            if(getConversationID() != null){
                invite.addHeader(ChatUtils.HEADER_CONVERSATION_ID, getConversationID());
            }
        }
        return invite;
    }

    /**
     * Create an INVITE request
     *
     * @return the INVITE request
     * @throws SipException
     */
    public SipRequest createInvite() throws SipException {
        // If there is a first message then builds a multipart content else builds a SDP content
        SipRequest invite;
        if (getFirstMessage() != null && RcsSettings.getInstance().isFirstMessageInInvite()) {
            invite = createMultipartInviteRequest(getDialogPath().getLocalContent());
        } else {
            invite = createInviteRequest(getDialogPath().getLocalContent());
        }
        return invite;
    }

    /**
     * Handle 200 0K response
     *
     * @param resp 200 OK response
     */
    public void handle200OK(SipResponse resp) {
        super.handle200OK(resp);

        // Start the activity manager
        getActivityManager().start();
    }

    /**
     * Handle 403 Forbidden
     *
     * @param resp 403 response
     */
    public void handle403Forbidden(SipResponse resp) {
       WarningHeader warn = (WarningHeader)resp.getHeader(WarningHeader.NAME);
       String warningText = "";
       if((warn != null && warn.getText() != null)) {
         warningText = warn.getText();
       }
       if (logger.isActivated()) {
           logger.error("handle403Forbidden warning text " + warningText + ", registercount:" + retryRegisterCount + ", warn:" + warn);
       }
       if (isSessionInterrupted()) {
          return;
       }

       if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
           if ((warn != null)){
               if (logger.isActivated()) {
                   logger.info("ABC handle403Forbidden warning text3 " + warningText);
               }

                ChatError error = new ChatError(ChatError.SESSION_FORBIDDEN_ERROR,warningText);
                if (logger.isActivated()) {
                    logger.info("ABC Invite error: " + error.getErrorCode() + ", reason=" + error.getMessage());
                }

                // Close media session
                closeMediaSession();

                // Remove the current session
                getImsService().removeSession(this);

                // Notify listeners
                for(int i=0; i < getListeners().size(); i++) {
                    ((ChatSessionListener)getListeners().get(i)).handleInviteError(error);
                }
           } else {
               if (logger.isActivated()) {
               logger.error("re-register retryCount: " + retryRegisterCount);
           }
               if(retryRegisterCount < 4){
                   retryRegisterCount++;
                   boolean isRegistered = false;
                    isRegistered = imsService.getImsModule().getCurrentNetworkInterface().getRegistrationManager().registration();
                    if (logger.isActivated()) {
                        logger.debug("re-register isRegistered: " + isRegistered);
                    }
                    if (isRegistered) {
                        String callId = dialogPath.getCallId();
                        SipRequest invite = createSipInvite(callId);
                        if (invite != null) {
                            try {
                                sendInvite(invite);
                            } catch (SipException e) {
                                if (logger.isActivated()) {
                                    logger.debug("re send sip request failed.");
                                }
                                e.printStackTrace();
                            }
                        } else {
                            if (logger.isActivated()) {
                                logger.debug("handle403Forbidden() invite is null");
                            }
                        }
                    }
                    if (logger.isActivated()) {
                        logger.debug("handle403Forbidden() exit");
                    }
                } else {
                    handleInviteDeclined(resp);
                }
           }
       } else {
           super.handle403Forbidden(resp);
       }
    }

    public void handleRetry(SipResponse resp){
        if (logger.isActivated()) {
            logger.info("handleRetry retryValue:" );
        }
        String callId = dialogPath.getCallId();
        SipRequest invite = createSipInvite(callId);
        if (invite != null) {
            try {
                sendInvite(invite);
            } catch (SipException e) {
                if (logger.isActivated()) {
                    logger.debug("resendsip request failed.");
                }
                e.printStackTrace();
            }

        } else {
            if (logger.isActivated()) {
                logger.debug("handleRetry() invite is null");
            }
        }
        if (logger.isActivated()) {
            logger.debug("handleRetry() exit");
        }
    }

    public void handle500Retry(SipResponse resp){
        if (logger.isActivated()) {
            logger.info("handleRetry retryValue:" );
        }
        String callId = dialogPath.getCallId();
        SipRequest invite = createSipInvite(callId);
        if (invite != null) {
            try {
                sendInvite(invite);
            } catch (SipException e) {
                if (logger.isActivated()) {
                    logger.debug("resendsip request failed.");
                }
                e.printStackTrace();
            }

        } else {
            if (logger.isActivated()) {
                logger.debug("handleRetry() invite is null");
            }
        }
        if (logger.isActivated()) {
            logger.debug("handleRetry() exit");
        }
    }

    /**
     * Handle 480 Temporarily Unavailable
     *
     * @param resp 480 response
     */
    public void handle480Unavailable(SipResponse resp) {
        int retryValue = SipUtils.getRetryAfterValue(resp);
        if (logger.isActivated()) {
            logger.error("480 response received retryValue:" + retryValue);
        }
        if (isSessionInterrupted()) {
            return;
        }
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported() && retryValue == 5) {
            handleRetry(resp);
        } else if(RcsSettings.getInstance().isFallbackToPagerModeSupported() && retryValue == 6){
            handleOtherNetworkErrors(resp);
        } else if(RcsSettings.getInstance().isFallbackToPagerModeSupported() && retryValue == -1 ) {
            if(retry480Count == 0 && RcsSettings.getInstance().isSupportOP08()){
                retry480Count++;
                handleRetry(resp);
            } else {
                ImsServiceError error = new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_ERROR,
                    resp.getStatusCode() + " " + resp.getReasonPhrase());

                if (logger.isActivated()) {
                   logger.info("error: " + error.getErrorCode() + ", reason=" + error.getMessage());
                }
                closeMediaSession();
                getImsService().removeSession(this);
                // Notify listeners
                for(int i=0; i < getListeners().size(); i++) {
                    ((ChatSessionListener)getListeners().get(i)).handleInviteError(new ChatError(error));
                }
            }
        } else {
            if(RcsSettings.getInstance().isSupportOP07()){
                handleError(new ChatError(ChatError.SESSION_INITIATION_FALLBACK, resp.getReasonPhrase()));
            } else {
                handleError(new ChatError(ChatError.SESSION_INITIATION_FAILED, resp.getReasonPhrase()));
            }
        }
    }

    /**
     * Handle 600 Busy EveryWhere
     *
     * @param resp 600 response
     */
    public void handle600BusyEveryWhere(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("500 handle600BusyEveryWhere");
        }
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
           String retyrHeader = SipUtils.getRetryAfterHeader(resp);
           if(retyrHeader.contains("5")){
        	   handleRetry(resp);
           }else if(retyrHeader.contains("6")){
        	   handleOtherNetworkErrors(resp);
           }else{
        	   handleDefaultError(resp);
           }
        } else {
        	handleDefaultError(resp);
        }
    }

    /**
     * Handle 481 transaction does not exist
     *
     * @param resp 481 response
     */
    public void handle481TransactionDoesNotExist(SipResponse resp) {
        String warningText = "";
        WarningHeader warn = (WarningHeader)resp.getHeader(WarningHeader.NAME);
        if (logger.isActivated()) {
            logger.error("481 response received warn:" + warn + ", retryservicount:" + retryServiceCount);
        }
        if (isSessionInterrupted()) {
            return;
      }
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported() && warn == null) {
            if(retryServiceCount < 1){
                retryServiceCount++;
                handleRetry(resp);
            } else {
            ImsServiceError error = new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_ERROR,
                  resp.getStatusCode() + " " + resp.getReasonPhrase());

              // Error
                 if (logger.isActivated()) {
                     logger.info("error: " + error.getErrorCode() + ", reason=" + error.getMessage());
                 }

              // Close media session
              closeMediaSession();

              // Remove the current session
              getImsService().removeSession(this);

              // Notify listeners
              for(int i=0; i < getListeners().size(); i++) {
                  ((ChatSessionListener)getListeners().get(i)).handleInviteError(new ChatError(error));
              }
            }
        } else {
            handleDefaultError(resp);
        }
    }

    /**
     * Handle 404 Session Not Found
     *
     * @param resp 404 response
     */
    public void handle404SessionNotFound(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("404 handle404SessionNotFound");
        }
        try{
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
           String retyrHeader = SipUtils.getRetryAfterHeader(resp);
               if(retyrHeader != null && retyrHeader.contains("5")){
                   handleRetry(resp);
               }else if(retyrHeader != null && retyrHeader.contains("6")){
        	   handleOtherNetworkErrors(resp);
           }else{
        	   handleDefaultError(resp);
           }
        } else if(RcsSettings.getInstance().isSupportOP07()){
            handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK,
                    resp.getStatusCode() + " " + resp.getReasonPhrase()));
        } else {
            handleDefaultError(resp);
        }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("404 handle404SessionNotFound exception");
            }
            e.printStackTrace();
            handleDefaultError(resp);
        }
    }

    /**
     * Handle 663 Session Expiry Found
     *
     * @param resp 663 response
     */
    public void handle663TimerExpiry(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("handle663TimerExpiry");
        }
        try{
            if(RcsSettings.getInstance().isSupportOP07()) {
                handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK,"timeout"));
            } else if(RcsSettings.getInstance().isFallbackToPagerModeSupported()){
                handleResponseTimeout(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED, "timeout"));
            } else {
                handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED, "timeout"));
            }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("handle663TimerExpiry exception");
            }
            e.printStackTrace();
            handleDefaultError(resp);
        }
    }

    /**
     * Handle 503 service unavailable
     *
     * @param resp 503 response
     */
    public void handle503ServiceUnavailable(SipResponse resp) {
        int retryValue = SipUtils.getRetryAfterValue(resp);
        if (logger.isActivated()) {
            logger.error("503 response received retryValue:" + retryValue);
        }
        if (isSessionInterrupted()) {
            return;
        }
        try{
    if(RcsSettings.getInstance().isFallbackToPagerModeSupported() && retryValue == 5) {
           handleRetry(resp);
        } else if(RcsSettings.getInstance().isFallbackToPagerModeSupported() && retryValue == 6){
            handleOtherNetworkErrors(resp);
        }else if(RcsSettings.getInstance().isFallbackToPagerModeSupported() && retryValue == -1 ) {
            ImsServiceError error = new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_ERROR,
                resp.getStatusCode() + " " + resp.getReasonPhrase());

            if (logger.isActivated()) {
               logger.info("error: " + error.getErrorCode() + ", reason=" + error.getMessage());
            }
                closeMediaSession();
                getImsService().removeSession(this);
                // Notify listeners
                for(int i=0; i < getListeners().size(); i++) {
                    ((ChatSessionListener)getListeners().get(i)).handleInviteError(new ChatError(error));
                }
        } else {
            handleDefaultError(resp);
        }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("503 handle503ServiceUnavailable exception");
            }
            e.printStackTrace();
            handleDefaultError(resp);
        }
    }

    /**
     * Handle 500 Internal Server Error
     *
     * @param resp 500 response
     */
    public void handle500InternalServerError(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("500 handleInternalServerError");
        }
        try{
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
           String retyrHeader = SipUtils.getRetryAfterHeader(resp);
               if(retyrHeader != null && retyrHeader.contains("5")){
                   handleRetry(resp);
               }else if(retyrHeader != null && retyrHeader.contains("6")){
        	   handleOtherNetworkErrors(resp);
           }else{
        	   handleDefaultError(resp);
           }
        } else {
        	handleDefaultError(resp);
        }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("500 handle500InternalServerError exception");
            }
            e.printStackTrace();
            handleDefaultError(resp);
        }
    }

    /**
     * Handle 600 Busy Everywhere
     *
     * @param resp 600 response
     */
    public void handle600BusyEverywhere(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("600 handle600BusyEverywhere");
        }
        try{
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
           String retyrHeader = SipUtils.getRetryAfterHeader(resp);
               if(retyrHeader != null && retyrHeader.contains("5")){
               handleRetry(resp);
               }else if(retyrHeader != null && retyrHeader.contains("6")){
               handleOtherNetworkErrors(resp);
           }else{
               handleDefaultError(resp);
           }
        } else {
            handleDefaultError(resp);
        }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("600 handle600BusyEverywhere exception");
            }
            e.printStackTrace();
            handleDefaultError(resp);
        }
    }

    public void handleMsrpConnectionException(Exception e) {
        if (logger.isActivated()) {
            logger.error("handleMsrpConnectionException");
        }
        handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK, "connectionrefused"));
    }

    /**
     * Handle 502 Bad Gateway
     *
     * @param resp 502 response
     */
    public void handle502BadGateway(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("502 handle502BadGateway");
        }
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
            if(retry502Count < 1){
                retry502Count++;
                handleRetry(resp);
            } else {
                handleOtherNetworkErrors(resp);
            }
        } else {
            handleDefaultError(resp);
        }
    }

    /**
     * Handle 482 Loop Detected
     *
     * @param resp 482 response
     */
    public void handle482LoopDetected(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("482 handle482LoopDetected");
        }
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
            if(retry482Count < 1){
                retry482Count++;
                handleRetry(resp);
            } else {
                handleOtherNetworkErrors(resp);
            }
        } else {
            handleDefaultError(resp);
        }
    }

    /**
     * Handle 504  Server Timeout
     *
     * @param resp 504 response
     */
    public void handle504ServerTimeout(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("504 handle504ServerTimeout");
        }
        if(RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
            handleOtherNetworkErrors(resp);
        } else {
            handleDefaultError(resp);
        }
    }

    /**
     * Data transfer error
     *
     * @param msgId Message ID
     * @param error Error code
     */
    public void msrpTransferError(String msgId, String error) {
        super.msrpTransferError(msgId, error);

        // Request capabilities
        getImsService().getImsModule().getCapabilityService().requestContactCapabilities(getDialogPath().getRemoteParty());
    }

    public void removeSession() {
        Core.getInstance().getImService().removeSession(this);
    }
}
