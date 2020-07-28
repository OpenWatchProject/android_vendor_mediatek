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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipManager;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;

import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;

import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;


/**
 * Rejoin a group chat session
 * 
 * @author Jean-Marc AUFFRET
 */
public class RejoinGroupChatSession extends GroupChatSession {
	/**
     * Boundary tag
     */
    private final static String BOUNDARY_TAG = "boundary1";
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
/**
     * M: add for auto-rejoin group chat @{
     */
    /**
     * Max count to retry.
     */
    private static final int MAX_RETRY_COUNT = 10;
    /**
     * Current retry count.
     */
    private int mCurrentRetryCount = 0;
    
    /**
     * Max count to retry.
     */
    private static final int MAX_404RETRY_COUNT = 3;
    /**
     * Current retry count.
     */
    private int m404CurrentRetryCount = 0;

    /** @} */
    /**
     * Constructor
     *
     * @param parent IMS service
     * @param rejoinId Rejoin ID
     * @param chatId Chat ID or contribution ID
     * @param subject Subject
     * @param participants List of participants
     */
    public RejoinGroupChatSession(ImsService parent, String rejoinId, String chatId, String subject, Map<String, ParticipantStatus> participants) {
        super(parent, rejoinId, participants);

        // Set subject
        if ((subject != null) && (subject.length() > 0)) {
            setSubject(subject);
        }

        // Create dialog path
        createOriginatingDialogPath();

        // Set contribution ID
        setContributionID(chatId);

        if (RcsSettings.getInstance().isCPMSupported()) {
            if (logger.isActivated()) {
                logger.info("CPMS RejoinGroupChatSession old call id: "
                        + getDialogPath().getCallId());
            }
            RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
            String conversationId = rmHistory.getCoversationID(chatId, 2);
            if (conversationId.isEmpty()) {
                String callId = getImsService().getImsModule().getSipManager().getSipStack()
                        .generateCallId();
                if (logger.isActivated()) {
                    logger.info("CPMS RejoinGroupChatSession call id: " + callId);
                }
                // Set conversation ID
                conversationId = ContributionIdGenerator.getContributionId(callId);
                setConversationID(conversationId);
                rmHistory.UpdateCoversationID(chatId, conversationId, 1);
            } else {
                setConversationID(conversationId);
            }
        }
    }

    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("Rejoin an existing group chat session");
            }

            /**
             * Modified to resolve the 403 error issue.@{
             */
            SipRequest invite = createSipInvite();
            /**
             * @}
             */
            
            // Send INVITE request
            sendInvite(invite);            
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }

            // Unexpected error
            handleError(new ChatError(ChatError.UNEXPECTED_EXCEPTION,
                    e.getMessage()));
        }        
    }

    /**
 * Modified to resolve the 403 error issue.@{
     */
    /**
     * @return A sip invite request
     */
    protected SipRequest createSipInvite(String callId) {
        SipRequest invite = null;
        mCurrentRetryCount++;
        if (logger.isActivated()) {
            logger.debug("createSipInvite(), callId: " + callId + " mCurrentRetryCount: "
                    + mCurrentRetryCount);
        }
        if (mCurrentRetryCount <= MAX_RETRY_COUNT) {
            try {
                Thread.sleep(SipManager.TIMEOUT * 1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
                if (logger.isActivated()) {
                    logger.debug("createSipInvite() InterruptedException");
                }
            }
            createOriginatingDialogPath(callId);
            invite = createSipInvite();
        }
        return invite;
    }

    private SipRequest createSipInvite() {
        logger.debug("createSipInvite()");
            // Set setup mode
            String localSetup = createSetupOffer();
            if (logger.isActivated()){
                logger.debug("Local setup attribute is " + localSetup);
            }

            // Set local port
            int localMsrpPort;
            if ("active".equals(localSetup)) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = getMsrpMgr().getLocalMsrpPort();
            }

            // Build SDP part
            String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
            String ipAddress = ChatUtils.formatIPAddress(getDialogPath().getSipStack().getLocalIpAddress());
            logger.debug(" getDialogPath().getSipStack().getLocalIpAddress(); after format: " +  ipAddress);
            
            String sdp = null;
            if(isSecureProtocolMessage()){
                sdp =
                "v=0" + SipUtils.CRLF +
                "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "s=-" + SipUtils.CRLF +
                "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "t=0 0" + SipUtils.CRLF +            
                "m=message " + localMsrpPort + " " + getMsrpMgr().getLocalSocketProtocol() + " *" + SipUtils.CRLF +
                "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF +
                "a=fingerprint:" + KeyStoreManager.getFingerPrint() + SipUtils.CRLF +
                "a=setup:" + localSetup + SipUtils.CRLF +
                "a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
                "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
                "a=sendrecv" + SipUtils.CRLF;
            }
            else{
                sdp =
                "v=0" + SipUtils.CRLF +
                "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "s=-" + SipUtils.CRLF +
                "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "t=0 0" + SipUtils.CRLF +            
                "m=message " + localMsrpPort + " " + getMsrpMgr().getLocalSocketProtocol() + " *" + SipUtils.CRLF +
                "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF +
                "a=setup:" + localSetup + SipUtils.CRLF +
                "a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
                "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
                "a=sendrecv" + SipUtils.CRLF;
            }

            // Set the local SDP part in the dialog path
            getDialogPath().setLocalContent(sdp);

            // Create an INVITE request
            if (logger.isActivated()) {
                logger.info("Send INVITE");
            }
                SipRequest invite = null;
                try{
             invite = createInviteRequest(sdp);

            // Set the Authorization header
            getAuthenticationAgent().setAuthorizationHeader(invite);

            // Set initial request in the dialog path
            getDialogPath().setInvite(invite);
            
        } catch (SipException e) {
            e.printStackTrace();
        } catch (CoreException e) {
            e.printStackTrace();
        }        
        return invite;
    }
    
    private SipRequest createNewSipInvite() {
    	logger.debug("createNewSipInvite()");
        // Set setup mode
        String localSetup = createSetupOffer();
        if (logger.isActivated()){
            logger.debug("Local setup attribute is " + localSetup);
        }

        // Set local port
        int localMsrpPort;
        if ("active".equals(localSetup)) {
            localMsrpPort = 9; // See RFC4145, Page 4
        } else {
            localMsrpPort = getMsrpMgr().getLocalMsrpPort();
        }

        // Build SDP part
        String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
        String ipAddress = ChatUtils.formatIPAddress(getDialogPath().getSipStack().getLocalIpAddress());
        logger.debug(" createNewSipInvite; after format: " +  ipAddress);
        
        String sdp = null;
        if(isSecureProtocolMessage()){
            sdp =
            "v=0" + SipUtils.CRLF +
            "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "s=-" + SipUtils.CRLF +
            "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "t=0 0" + SipUtils.CRLF +            
            "m=message " + localMsrpPort + " " + getMsrpMgr().getLocalSocketProtocol() + " *" + SipUtils.CRLF +
            "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF +
            "a=fingerprint:" + KeyStoreManager.getFingerPrint() + SipUtils.CRLF +
            "a=setup:" + localSetup + SipUtils.CRLF +
            "a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
            "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
            "a=sendrecv" + SipUtils.CRLF;
        }
        else{
             sdp =
            "v=0" + SipUtils.CRLF +
            "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "s=-" + SipUtils.CRLF +
            "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "t=0 0" + SipUtils.CRLF +            
            "m=message " + localMsrpPort + " " + getMsrpMgr().getLocalSocketProtocol() + " *" + SipUtils.CRLF +
            "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF +
            "a=setup:" + localSetup + SipUtils.CRLF +
            "a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
            "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
            "a=sendrecv" + SipUtils.CRLF;
        }

        // Generate the resource list for given participants
        List<String> participantList = new ArrayList();
        Map<String, ParticipantStatus> participantsInfo = getGroupParticipants();
        for(String participant:participantsInfo.keySet()){
            participantList.add(participant);
        }
        String resourceList = ChatUtils.generateChatResourceList(participantList);
        
        // Build multipart
        String multipart =
            Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
            "Content-Type: application/sdp" + SipUtils.CRLF +
            "Content-Length: " + sdp.getBytes().length + SipUtils.CRLF +
            SipUtils.CRLF +
            sdp + SipUtils.CRLF +
            Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
            "Content-Type: application/resource-lists+xml" + SipUtils.CRLF +
            "Content-Length: " + resourceList.getBytes().length + SipUtils.CRLF +
            "Content-Disposition: recipient-list" + SipUtils.CRLF +
            SipUtils.CRLF +
            resourceList + SipUtils.CRLF +
            Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + Multipart.BOUNDARY_DELIMITER;

        // Set the local SDP part in the dialog path
        getDialogPath().setLocalContent(multipart);

        // Create an INVITE request
        if (logger.isActivated()) {
        	logger.info("createNewSipInvite INVITE");
        }
        try {
	        SipRequest invite = createMultiInviteRequest(multipart);
	
	        // Set the Authorization header
	        getAuthenticationAgent().setAuthorizationHeader(invite);
	
	        // Set initial request in the dialog path
	        getDialogPath().setInvite(invite);
	        
	        return invite;            
	    } catch (SipException e) {
	        e.printStackTrace();
	    } catch (CoreException e) {
	        e.printStackTrace();
	    }        
	    logger.error("createNewSipInvite failed, return null.");
	    return null;  
    }
    
    /**
     * @}
     */
    
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
                getFeatureTags(),
                content);
        }
        else{
            if (logger.isActivated()) {
                logger.info("RejoinGroupChatSession createInviteRequest0 CPMS");
            } 
             invite = SipMessageFactory.createCpmInvite(getDialogPath(),
                        getCpimFeatureTags(),
                        content,true);
        }

        if(RcsSettings.getInstance().supportOP01()) {
            List<String> list = Arrays.asList(getCmccCpimFeatureTags());  
            // Update Contact header
            StringBuffer acceptTags = new StringBuffer("*");
            for(int i=0; i < list.size(); i++) {
                acceptTags.append(";" + list.get(i));
            }  
            invite.addHeader(SipUtils.HEADER_ACCEPT_CONTACT, acceptTags.toString());
        }

        // Add a contribution ID header
        invite.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID());
        if(RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.info("RejoinGroupChatSession createInviteRequest 1 CPMS");
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
    private SipRequest createMultiInviteRequest(String content) throws SipException {
        SipRequest invite = null;
        if(!RcsSettings.getInstance().isCPMSupported()){
           invite = SipMessageFactory.createInvite(getDialogPath(),
                getFeatureTags(),
                content);
        }
        else{
            if (logger.isActivated()) {
                logger.info("RejoinGroupChatSession createMultiInviteRequest CPMS");
            } 
             invite = SipMessageFactory.createCpmMultipartInvite(getDialogPath(),
                         getCpimFeatureTags(),
                         content,BOUNDARY_TAG,true);
        }

        // Add a contribution ID header
        invite.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID());
        if(RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.info("RejoinGroupChatSession createMultiInviteRequest  CPMS");
            } 
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
        return createInviteRequest(getDialogPath().getLocalContent());
    }
    
    protected SipRequest createSipInviteWithNewId() {
        logger.debug("createSipInviteWithNewId()");
        createOriginatingDialogPath();
        // Set contribution ID
        String id = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
        setContributionID(id);
        return createNewSipInvite();
    }
    
    /**
     * Handle 404 Session Not Found
     *
     * @param resp 404 response
     */
    public void handle481TransactionDoesNotExist(SipResponse resp) {
        // Rejoin session has failed, we update the database with status terminated by remote
        RichMessagingHistory.getInstance().updateGroupChatStatus(getContributionID(), GroupChat.State.REJECTED.toInt());
        if (logger.isActivated()) {
            logger.info("handle481TransactionDoesNotExist");
        }
        SipRequest invite = createSipInviteWithNewId();
        if (invite != null) {
            try {
                sendInvite(invite);
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Session initiation has failed", e);
                }

                // Unexpected error
                handleError(new ChatError(ChatError.UNEXPECTED_EXCEPTION,
                        e.getMessage()));
            }  
    
        } else {
            handleError(new ChatError(ChatError.SESSION_NOT_FOUND, resp.getReasonPhrase()));
        }
    }
    
    public void handleMsrp403(final String msgId, String error) {
        if (logger.isActivated()) {
            logger.info("handleMsrp403 retryValue:" );
        }       
        String callId = dialogPath.getCallId();
        SipRequest invite = createSipInvite(callId);
        if (invite != null) {
            try {
                sendInvite(invite);
                String txt = RichMessagingHistory.getInstance().getMessageText(msgId);
                // Generate a message Id
                final String newMsgId = ChatUtils.generateMessageId();
                this.sendTextMessage(newMsgId, txt);
            } catch (SipException e) {
                if (logger.isActivated()) {
                    logger.debug("handleMsrp403 request failed.");
                }
                e.printStackTrace();
            }
    
        } else {
            if (logger.isActivated()) {
                logger.debug("handleMsrp403() invite is null");
            }
        }
        if (logger.isActivated()) {
            logger.debug("handleMsrp403() exit");
        }
    }

    /**
     * Handle 404 Session Not Found
     *
     * @param resp 404 response
     */
    public void handle404SessionNotFound(SipResponse resp) {
        // Rejoin session has failed, we update the database with status terminated by remote
        RichMessagingHistory.getInstance().updateGroupChatStatus(getContributionID(), GroupChat.State.REJECTED.toInt());
        if (logger.isActivated()) {
            logger.info("CPMS handle404SessionNotFound");
        }
        
        if(RcsSettings.getInstance().isGroupChat404HandlingSupported()){
            if(m404CurrentRetryCount < MAX_404RETRY_COUNT){
                m404CurrentRetryCount++;
                SipRequest invite = createSipInviteWithNewId();
                if (invite != null) {
                    try {
                        sendInvite(invite);
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Session initiation has failed", e);
                        }
    
                        // Unexpected error
                        handleError(new ChatError(ChatError.UNEXPECTED_EXCEPTION,
                                e.getMessage()));
                    }  
            
                } else {
                    if (logger.isActivated()) {
                        logger.debug("handle404SessionNotFound() invite is null");
                    }
                    handleError(new ChatError(ChatError.SESSION_NOT_FOUND, resp.getReasonPhrase()));
                }
            } else { 
                m404CurrentRetryCount = 0;
                handleError(new ChatError(ChatError.SESSION_NOT_FOUND, resp.getReasonPhrase()));
            }
        }
    }
    
    @Override
    public void startSession() {
        getImsService().getImsModule().getInstantMessagingService().addSession(this);
        start();
    }
    
    @Override
    public boolean isInitiatedByRemote() {
        return false;
    }
}
