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

import com.orangelabs.rcs.core.CoreException;
import javax.sip.header.RequireHeader;
import javax.sip.header.SubjectHeader;
import javax2.sip.header.Header;
import javax2.sip.message.Request;

import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.GroupChat;

import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.utils.logger.Logger;


import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;

import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.net.URLEncoder;

/**
 * Originating ad-hoc group chat session
 * 
 * @author jexa7410
 */
public class OriginatingAdhocGroupChatSession extends GroupChatSession {
    /**
     * Boundary tag
     */
    private final static String BOUNDARY_TAG = "boundary1";

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    /**
     * Max count to retry.
     */
    private static final int MAX_404RETRY_COUNT = 3;
    
    /**
     * Current retry count.
     */
    private int m404CurrentRetryCount = 0;

    /**
     * Constructor
     * 
     * @param parent IMS service
     * @param conferenceId Conference ID
     * @param subject Subject associated to the session
     * @param participants List of invited participants
     */
    public OriginatingAdhocGroupChatSession(ImsService parent, String conferenceId, String subject, Map<String, ParticipantStatus> participants) {
        super(parent, conferenceId, participants);

        // Set subject
        if ((subject != null) && (subject.length() > 0)) {
            setSubject(subject);
        }

        // Create dialog path
        createOriginatingDialogPath();
        
        // Set contribution ID
        String id = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
        setContributionID(id);                

        if (RcsSettings.getInstance().isCPMSupported()) {
            if (logger.isActivated()) {
                logger.info("CPMS OriginatingAdhocGroupChatSession old call id: " + getDialogPath().getCallId());
            }
            RichMessagingHistory rmHistory= RichMessagingHistory.getInstance();
            if (rmHistory.getCoversationID(id,2).equals("")) {
                // Set Call-Id
                String callId = getImsService().getImsModule().getSipManager().getSipStack().generateCallId();
                if (logger.isActivated()) {
                    logger.info("CPMS OriginatingAdhocGroupChatSession callId: " + callId);
                }
                // Set conversation ID
                String ConversationId = ContributionIdGenerator.getContributionId(callId);
                setConversationID(ConversationId);
                rmHistory.UpdateCoversationID(id, ConversationId, 1);
            } else {
                setConversationID(rmHistory.getCoversationID(id, 1));
            }
        }
    }

    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("Initiate a new ad-hoc group chat session as originating chatid: " + getSessionID());
            }

            /**
              * M: Modified to resolve the 403 error issue @{
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
     * M: Modified to resolve the 403 error issue @{
     */
    /**
     * @return A sip invite request
     */
    protected SipRequest createSipInvite(String callId) {
        logger.debug("createSipInvite(), callId = " + callId);
        createOriginatingDialogPath(callId);
        return createSipInvite();
    }
    
    protected SipRequest createSipInviteWithNewId() {
        logger.debug("createSipInviteWithNewId()");
        createOriginatingDialogPath();
        // Set contribution ID
        String id = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
        setContributionID(id);
        return createSipInvite();
    }

    protected SipRequest createSipInvite(String callId, boolean isFocus) {
        logger.debug("createSipInvite(), callId = " + callId);
        createOriginatingDialogPath(callId);
        if(isFocus){
            //Increment the Cseq number of the dialog path
            getDialogPath().incrementCseq();       
            /*// Set Call-Id
            String newCallId = getImsService().getImsModule().getSipManager().getSipStack().generateCallId();
            if (logger.isActivated()) {
                logger.info("CPMS OriginatingAdHocGroup ChatSession callId: " + newCallId);
            }
            // Set conversation ID            
            String ConversationId = ContributionIdGenerator.getContributionId(newCallId);
            setConversationID(ConversationId);*/
        }
        return createSipInvite();
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
            String msrpCemaParameter = null;
            if(RcsSettings.getInstance().isSupportOP07()){
                msrpCemaParameter = "a=msrp-cema" + SipUtils.CRLF;
            }
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
            else {
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
            if(msrpCemaParameter != null){
                sdp += msrpCemaParameter;
            }

            // Generate the resource list for given participants
            String resourceList = ChatUtils.generateChatResourceList(getParticipants().getList());
            
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
            logger.info("Create INVITE");
            }
                try {
            SipRequest invite = createInviteRequest(multipart);

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
        logger.error("Create sip invite failed, return null.");
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
             invite = SipMessageFactory.createMultipartInvite(getDialogPath(), 
                getFeatureTags(), 
                content,
                BOUNDARY_TAG);
        }
        else{
            if (logger.isActivated()) {
                logger.info("CPMS OriginatingAdhocGroupChatSession createInviteRequest new");
            }
             invite = SipMessageFactory.createCpmMultipartInvite(getDialogPath(), 
                getCpimFeatureTags(), 
                content,
                BOUNDARY_TAG,true);
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

        // Test if there is a subject
        if (getSubject() != null) {
            // Add a subject header
            String subject = StringUtils.encodeUTF8(getSubject());
            if (RcsSettings.getInstance().supportOP01()) {
                try {
                    subject = URLEncoder.encode(subject, "UTF-8");
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            invite.addHeader(SubjectHeader.NAME, subject);
        }

        // Add a require header
        if(!(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()))
          invite.addHeader(RequireHeader.NAME, "recipient-list-invite");    
        
        // Add a contribution ID header
        invite.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID());
        if(RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.info("CPMS OriginatingAdhocGroupChatSession createInviteRequest 1");
            }
            if(getConversationID() != null){
                invite.addHeader(ChatUtils.HEADER_CONVERSATION_ID, getConversationID());
            }
        }
    
        return invite;
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
    /**
     * Create an INVITE request
     *
     * @return the INVITE request
     * @throws SipException 
     */
    public SipRequest createInvite() throws SipException {
        return createInviteRequest(getDialogPath().getLocalContent());
    }
    
    @Override
    public boolean isInitiatedByRemote() {
        return false;
    }

    @Override
    public void startSession() {
        getImsService().getImsModule().getInstantMessagingService().addSession(this);
        start();
    }
}
