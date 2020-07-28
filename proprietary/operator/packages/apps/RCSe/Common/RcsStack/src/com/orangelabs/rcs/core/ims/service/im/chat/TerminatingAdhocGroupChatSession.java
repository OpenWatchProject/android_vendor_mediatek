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

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;
import java.net.URLDecoder;

import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpSession;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaAttribute;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.SessionTimerManager;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityUtils;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;

import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;


/**
 * Terminating ad-hoc group chat session
 * 
 * @author jexa7410
 */
public class TerminatingAdhocGroupChatSession extends GroupChatSession {
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     * 
     * @param parent IMS service
     * @param invite Initial INVITE request
     */
    public TerminatingAdhocGroupChatSession(ImsService parent, SipRequest invite) {
        super(parent, ChatUtils.getReferredIdentity(invite), ChatUtils.getInvitedParticipants(invite));

        // Set subject
        String subject = ChatUtils.getSubject(invite);
        if (RcsSettings.getInstance().supportOP01()) {
            try {
                subject = URLDecoder.decode(subject, "UTF-8");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        if (logger.isActivated()) {
            logger.info("inviteMissingParticipants connected: " + RichMessagingHistory.getInstance().getGroupChatConnectedParticipants(getContributionID()));
            logger.info("inviteMissingParticipants invited: " + getParticipants().getList());
        }
        if (logger.isActivated()) {
            logger.info("TerminatingAdhocGroupChatSession  subject " + subject);
        } 
        setSubject(subject);
        
        //set the person as chairman who invited
        /*String chairman = ChatUtils.getReferredIdentity(invite);
        chairman = PhoneUtils.extractNumberFromUri(chairman);
        setChairman(chairman);*/
        
        // Create dialog path
        createTerminatingDialogPath(invite);
        
        // Set contribution ID
        String id = ChatUtils.getContributionId(invite); 
        setContributionID(id);    

        Capabilities capabilities = CapabilityUtils.extractCapabilities(invite);
        if(capabilities != null){
            ((GroupChatSession)this).setMsrpFtSupport(capabilities.isFileTransferSupported());
        }

            
                if(RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.info("TerminatingAdhocGroupChatSession  CPMS");
            } 
            setConversationID(ChatUtils.getCoversationId(invite));
            setInReplyID(ChatUtils.getInReplyId(invite));
        }
        if (logger.isActivated()) {
            String remoteContact = getRemoteContact();
            setGroupRemoteDisplayName(SipUtils.getDisplayNameFromUri(remoteContact));
            logger.info("TerminatingAdhocGroupChatSession getReferredBy" + remoteContact);
            logger.info("TerminatingAdhocGroupChatSession display name" + SipUtils.getDisplayNameFromUri(remoteContact));
        }
    }

    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("Initiate a new ad-hoc group chat session as terminating");
            }

            boolean autoAccept = RcsSettings.getInstance().isGroupChatAutoAccepted() ||
                ChatUtils.getHttpFTInfo(getDialogPath().getInvite()) != null;
            if (RcsSettings.getInstance().supportOP01()) {
                //always set false in op01, the app will do the choice.
                autoAccept = false;
            }
            if (autoAccept) {
                if (logger.isActivated()) {
                    logger.debug("Auto accept group chat invitation");
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("Accept manually group chat invitation");
                }
                
                // Send a 180 Ringing response
                send180Ringing(getDialogPath().getInvite(), getDialogPath().getLocalTag());
                
                // Wait invitation answer
                int answer = waitInvitationAnswer();
                if (answer == ImsServiceSession.INVITATION_REJECTED) {
                    if (logger.isActivated()) {
                        logger.debug("Session has been rejected by user");
                    }
                    
                    // Remove the current session
                    getImsService().removeSession(this);
    
                    // Notify listeners
                    for(int i=0; i < getListeners().size(); i++) {
                        getListeners().get(i).handleSessionAborted(ImsServiceSession.TERMINATION_BY_USER);
                    }
                    return;
                } else
                if (answer == ImsServiceSession.INVITATION_NOT_ANSWERED) {
                    if (logger.isActivated()) {
                        logger.debug("Session has been rejected on timeout");
                    }
    
                    // Ringing period timeout
                    send486Busy(getDialogPath().getInvite(), getDialogPath().getLocalTag());
                    
                    // Remove the current session
                    getImsService().removeSession(this);
    
                    // Notify listeners
                    for(int i=0; i < getListeners().size(); i++) {
                        getListeners().get(i).handleSessionAborted(ImsServiceSession.TERMINATION_BY_TIMEOUT);
                    }
                    return;
                } else
                if (answer == ImsServiceSession.INVITATION_CANCELED) {
                    if (logger.isActivated()) {
                        logger.debug("Session has been canceled");
                    }
                    return;
                }
            }

            // Parse the remote SDP part
            String remoteSdp = getDialogPath().getInvite().getSdpContent();
            SdpParser parser = new SdpParser(remoteSdp.getBytes());
            Vector<MediaDescription> media = parser.getMediaDescriptions();
            MediaDescription mediaDesc = media.elementAt(0);
            MediaAttribute attr1 = mediaDesc.getMediaAttribute("path");
            String remotePath = attr1.getValue();
            String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription, mediaDesc);
            int remotePort = mediaDesc.port;
            
            // Extract the "setup" parameter
            String remoteSetup = "passive";
            MediaAttribute attr2 = mediaDesc.getMediaAttribute("setup");
            if (attr2 != null) {
                remoteSetup = attr2.getValue();
            }
            if (logger.isActivated()){
                logger.info("Remote setup attribute is " + remoteSetup);
            }
            
            // Set setup mode
            String localSetup = createSetupAnswer(remoteSetup);
            if (logger.isActivated()){
                logger.info("Local setup attribute is " + localSetup);
            }

            // Set local port
            int localMsrpPort;
            if (localSetup.equals("active") && !(RcsSettings.getInstance().isSupportOP07())) {
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
                "a=setup:" + localSetup + SipUtils.CRLF +
                "a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
                "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
                "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF +
                "a=fingerprint:" + KeyStoreManager.getFingerPrint() + SipUtils.CRLF;
            }
            else{
                sdp =
                "v=0" + SipUtils.CRLF +
                "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "s=-" + SipUtils.CRLF +
                "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "t=0 0" + SipUtils.CRLF +            
                "m=message " + localMsrpPort + " " + getMsrpMgr().getLocalSocketProtocol() + " *" + SipUtils.CRLF +
                "a=setup:" + localSetup + SipUtils.CRLF +
                "a=accept-types:" + getAcceptTypes() + SipUtils.CRLF +
                "a=accept-wrapped-types:" + getWrappedTypes() + SipUtils.CRLF +
                "a=path:" + getMsrpMgr().getLocalMsrpPath() + SipUtils.CRLF;
            }
            if(RcsSettings.getInstance().isSupportOP08() && this.isClosedGroup()){
                sdp += "a=chatroom:closedgroupchat" + SipUtils.CRLF;
            }
            if(msrpCemaParameter != null){
                sdp += msrpCemaParameter;
            }
            
            sdp +=  "a=sendrecv" + SipUtils.CRLF;

            // Set the local SDP part in the dialog path
            getDialogPath().setLocalContent(sdp);

            // Test if the session should be interrupted
            if (isInterrupted()) {
                if (logger.isActivated()) {
                    logger.info("Session has been interrupted: end of processing");
                }
                return;
            }

            // Create the MSRP server session
            if (localSetup.equals("passive")) {
                // Passive mode: client wait a connection
                MsrpSession session = getMsrpMgr().createMsrpServerSession(remotePath, this);
                session.setFailureReportOption(true);
                session.setSuccessReportOption(false);
                
                // Open the connection
                Thread thread = new Thread(){
                    public void run(){
                        try {
                            // Open the MSRP session
                            getMsrpMgr().openMsrpSession();
                            
                            // Send an empty packet
                            sendEmptyDataChunk();                            
                        } catch (IOException e) {
                            if (logger.isActivated()) {
                                logger.error("Can't create the MSRP server session", e);
                            }
                        }        
                    }
                };
                thread.start();
            }
            
            // Create a 200 OK response
            if (logger.isActivated()) {
                logger.info("Send 200 OK");
            }
            SipResponse resp = null;
            if(!RcsSettings.getInstance().isCPMSupported()){
                 resp = SipMessageFactory.create200OkInviteResponse(getDialogPath(),
                    getFeatureTags(), sdp);
            }
            else{
                  if (logger.isActivated()) {
                    logger.info("TerminatingAdhocGroupChatSession0  CPMS");
                 }
                  if(RcsSettings.getInstance().supportOP01()){
                       resp = SipMessageFactory.createCpm200OkInviteResponse(getDialogPath(),
                               getCmccCpimFeatureTags(), sdp);
                    } else{
                  resp = SipMessageFactory.createCpm200OkInviteResponse(getDialogPath(),
                    getCpimFeatureTags(), sdp);
            }
            }
            
            if(RcsSettings.getInstance().isCPMSupported()){
                 if (logger.isActivated()) {
                    logger.info("TerminatingAdhocGroupChatSession1  CPMS");
                 }
                if(getContributionID() != null){
                    resp.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID()); 
                } 
                if(getConversationID() != null){
                    resp.addHeader(ChatUtils.HEADER_CONVERSATION_ID, getConversationID());
                }
                if(getInReplyID() != null){
                    resp.addHeader(ChatUtils.HEADER_INREPLY_TO_CONTRIBUTION_ID, getInReplyID());
                }
            }

            // The signalisation is established
            getDialogPath().sigEstablished();

            // Send response
            SipTransactionContext ctx = getImsService().getImsModule().getSipManager().sendSipMessageAndWait(resp);

            // Analyze the received response 
            if (ctx.isSipAck()) {
                // ACK received
                if (logger.isActivated()) {
                    logger.info("ACK request received");
                }

                // The session is established
                getDialogPath().sessionEstablished();

                // Create the MSRP client session
                if (localSetup.equals("active")) {
                    // Active mode: client should connect
                    MsrpSession session = getMsrpMgr().createMsrpClientSession(remoteHost, remotePort, remotePath, this);
                    session.setFailureReportOption(true);
                    session.setSuccessReportOption(false);
                    
                    // Open the MSRP session
                    getMsrpMgr().openMsrpSession();
                    
                    // Send an empty packet
                    sendEmptyDataChunk();
                }

                // Notify listeners
                
                for(int i=0; i < getListeners().size(); i++) {
                    getListeners().get(i).handleSessionStarted();
                }

                // Subscribe to event package
                getConferenceEventSubscriber().subscribe();

                if(!RcsSettings.getInstance().supportOP01()) {
                // Invite missing participant
                inviteMissingParticipants();
                }
                
                // Start session timer
                if (getSessionTimerManager().isSessionTimerActivated(resp)) {            
                    getSessionTimerManager().start(SessionTimerManager.UAS_ROLE, getDialogPath().getSessionExpireTime());
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("No ACK received for INVITE");
                }

                // No response received: timeout
                handleError(new ChatError(ChatError.SESSION_INITIATION_FAILED));
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }

            // Unexpected error
            handleError(new ChatError(ChatError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }        
    }
    
    /***
     * Invite missing participants in background
     */
    private void inviteMissingParticipants() {
        Thread t = new Thread() {
            public void run() {
                try {
                    if (logger.isActivated()) {
                        logger.info("Check if participants are missing in the conference");
                    }
                    List<String> connected = RichMessagingHistory.getInstance().getGroupChatConnectedParticipants(getContributionID());
                    List<String> invited = getParticipants().getList();
                    if (logger.isActivated()) {
                        logger.info("inviteMissingParticipants connected: " + connected);
                        logger.info("inviteMissingParticipants invited: " + invited);
                    }
                    List<String> missing = new ArrayList<String>();
                    for(int i= 0; i < connected.size(); i++) {
                        String contact = connected.get(i);
                        if (!invited.contains(contact)) {
                            missing.add(contact);
                        }
                    }
                    if (missing.size() > 0) {
                        if (logger.isActivated()) {
                            logger.info("Add " + missing.size() + " missing participants in the conference");
                        }
                        addParticipants(missing);
                    }
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("Session initiation has failed", e);
                    }
                }
            }

        };
        t.start();
    }
    
    @Override
    public boolean isInitiatedByRemote() {
        return true;
    }
    
    @Override
    public void startSession() {
        
        getImsService().getImsModule().getInstantMessagingService().addSession(this);
        start();
    }
}
