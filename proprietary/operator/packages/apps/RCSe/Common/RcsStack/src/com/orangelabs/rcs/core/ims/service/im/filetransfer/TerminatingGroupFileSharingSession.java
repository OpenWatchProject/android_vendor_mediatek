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

package com.orangelabs.rcs.core.ims.service.im.filetransfer;

import java.io.IOException;
import java.util.Vector;

import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.PauseResumeFileObject;
import com.orangelabs.rcs.utils.SettingUtils;
import com.orangelabs.rcs.core.content.ContentManager;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpConstants;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpEventListener;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpManager;
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
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;

import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;


import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import javax2.sip.header.ContactHeader;


import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Terminating file transfer session
 * 
 * @author jexa7410
 */
public class TerminatingGroupFileSharingSession extends ImsFileSharingSession implements MsrpEventListener {
    /**
     * MSRP manager
     */
    private MsrpManager msrpMgr = null;

    /**
     * Stream that writes the file
     */
    private BufferedOutputStream thumbStreamForFile = null;

    /**
     * File to be created
     */
    private File thumbFile;

    private GroupChatSession groupChatSession = null;

    private String chatSessionId = null;
    
    private String messageId;
    
    /**
     * Remote instance Id
     */
    private String remoteInstanceId = null;
    

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    /** M: ftAutAccept @{ */
    private boolean mAutoAccept = false;

    /** @} */

    /**
     * Constructor
     * 
     * @param parent IMS service
     * @param invite Initial INVITE request
     * @param thumbnail Thumbnail
     */
    public TerminatingGroupFileSharingSession(ImsService parent, GroupChatSession groupChatSession,SipRequest invite) {        
        super(parent, ContentManager.createMmContentFromSdp(invite), ChatUtils.getReferredIdentity(invite), ChatUtils.extractFileThumbnail(invite));


        try {
            messageId = null;
            if (isImdnSupport()) {
                logger.debug("Parse message id1 ");
                if (ChatUtils.isImdnDeliveredRequested(invite)) {
                    messageId = ChatUtils.getMessageId(invite);
                    this.setMessageId(messageId);
                    ContactHeader inviteContactHeader = (ContactHeader) invite
                            .getHeader(ContactHeader.NAME);
                    if (inviteContactHeader != null) {
                        this.remoteInstanceId = inviteContactHeader
                                .getParameter(SipUtils.SIP_INSTANCE_PARAM);
                    }
                    if (logger.isActivated()) {
                        logger.info("TerminatingGroupFileSharingSession: " + messageId
                                + ";Instance: " + remoteInstanceId);
                    }
                }
            }
        } catch (Exception e1) {
            if (logger.isActivated()) {
                logger.debug("Exception in parsing mesageId");
            }
            e1.printStackTrace();
        }
        
        // Create dialog path
        createTerminatingDialogPath(invite);
        this.groupChatSession = groupChatSession;
        if (groupChatSession != null) {
            this.chatSessionId = groupChatSession.getSessionID();
        }

        if (logger.isActivated()) {
            logger.info("Initiate a new file transfer session as terminating");
        }
        
        //setContributionID(groupChatSession.getContributionID());
        String thumbName ="thumb_" + IdGenerator.getIdentifier() + ".jpeg";
    // Init file
        thumbFile = new File(RcsSettings.getInstance().getFileRootDirectory(), thumbName);
        try{
            thumbStreamForFile = new BufferedOutputStream(new FileOutputStream(thumbFile));
        } catch (FileNotFoundException e) {
            if (logger.isActivated()) {
                logger.error("Could not create stream, file does not exists.");
            }
           }
        try{
            logger.debug("write thumnail ");
            thumbStreamForFile.write(getThumbnail(), 0, getThumbnail().length);
            thumbStreamForFile.flush();
            thumbStreamForFile.close();
        }
        catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Download file exception", e);
               }
        }
        setThumbUrl(RcsSettings.getInstance().getFileRootDirectory()+ thumbName);
        File file = new File(getThumbUrl());
        long length = file.length() ;
        if (logger.isActivated()) {
               logger.error("lemghth of thumnail file is" + length + "URL is" + getThumbUrl());
        }
        
                /** M: ftAutAccept @{ */
        boolean autoAccept = RcsSettings.getInstance().isFileTransferAutoAccepted();
        if (!autoAccept) {
            logger.debug("isFileTransferAutoAccepted: false! ");
            return;
        }
        boolean lessWarnSize = (getContent().getSize() < (RcsSettings
                .getInstance().getWarningMaxFileTransferSize() * 1024));        
        if (!lessWarnSize) {
            logger.debug("lessWarnSize: false! ");
            return;
        }
        boolean isRoaming = SettingUtils.isRoaming(AndroidFactory
                .getApplicationContext());
        // whether ftAutAccept is enabled if roaming.
        if (isRoaming) {
            mAutoAccept = RcsSettings.getInstance()
                    .isEnableFtAutoAcceptWhenRoaming();
        } else {
            mAutoAccept = RcsSettings.getInstance()
                    .isEnableFtAutoAcceptWhenNoRoaming();
        }
        if (logger.isActivated()) {
            logger.debug("autoAccept: " + autoAccept + " lessWarnSize: "
                    + lessWarnSize + " enable: " + isRoaming +"mAutoAccept" + mAutoAccept);
        }
        
        /** @}*/       

        
        // Set contribution ID
        String id = ChatUtils.getContributionId(invite);
        setContributionID(id);        
        if(RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.info("CPMS TerminatingGroupFileSharingSession");
            }
            setConversationID(ChatUtils.getCoversationId(invite));
            setInReplyID(ChatUtils.getInReplyId(invite));
        }
    }

    public GroupChatSession getGroupChatSession(){
        return groupChatSession ;
    }

    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("Initiate a new file transfer session as terminating");
            }
    
            // Parse the remote SDP part
            String remoteSdp = getDialogPath().getInvite().getSdpContent();
            SdpParser parser = new SdpParser(remoteSdp.getBytes());
            Vector<MediaDescription> media = parser.getMediaDescriptions();
            MediaDescription mediaDesc = media.elementAt(0);
            String protocol = mediaDesc.protocol;
            boolean isSecured = false;
            if (protocol != null) {
                isSecured = protocol.equalsIgnoreCase(MsrpConstants.SOCKET_MSRP_SECURED_PROTOCOL);
            }
            MediaAttribute attr1 = mediaDesc.getMediaAttribute("file-selector");
            String fileSelector = attr1.getName() + ":" + attr1.getValue();
            MediaAttribute attr2 = mediaDesc.getMediaAttribute("file-transfer-id");
            String fileTransferId = attr2.getName() + ":" + attr2.getValue();
            try {
                hashselector = attr1.getValue().substring(attr1.getValue().indexOf("hash:sha-1:"));
                } catch (Exception e1) {
                    // TODO Auto-generated catch block
                    e1.printStackTrace();
                }
                
                if(pauseInfo != null){
                    if (logger.isActivated()){
                        logger.debug("FTS receive Invite save hashselctor " + hashselector);
                    }
                    pauseInfo.hashSelector = hashselector;
                    pauseInfo.mOldFileTransferId = attr2.getValue();
                }
            
            /** M: ftAutAccept @{ */
            if (mAutoAccept) {
                if (logger.isActivated()) {
                    logger.debug("Auto accept file transfer invitation");
                }
                /** @}*/
            } else {
                if (logger.isActivated()) {
                    logger.debug("Accept manually file transfer invitation");
                }                

                // Send a 180 Ringing response
                send180Ringing(getDialogPath().getInvite(), getDialogPath().getLocalTag());
                
                try {
                        // Extract the "setup" parameter
                       String timeLen = null;
                       MediaAttribute attr5 = mediaDesc.getMediaAttribute("file-disposition");
                       if (attr5 != null) {
                           timeLen = attr5.getValue();
                       }
                       if (logger.isActivated()){
                               logger.debug("FTS timeLen attribute is " + timeLen);
                       }
                       int fileLen = 0;
                       if (timeLen != null) {
                           String[] timeStringArray = timeLen.split("=");
                           if (logger.isActivated()){
                               logger.debug("FTS timeStringArray attribute is " + timeStringArray[0] + " new" + timeStringArray[1]);
                       }
                           String timeString = timeStringArray[1];
                           fileLen = Integer.parseInt(timeString);
                           if (logger.isActivated()){
                                   logger.debug("FTS timeString attribute is " + timeString + "filelen is " + fileLen);
                           }
                       }
                           setTimeLen(fileLen);            
                   } catch(Exception e){
                           e.printStackTrace();
                   }
                
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
                    for(int j=0; j < getListeners().size(); j++) {
                        getListeners().get(j).handleSessionAborted(ImsServiceSession.TERMINATION_BY_TIMEOUT);
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
            
            // Reject if file is too big or size exceeds device storage capacity. This control should be done
            // on UI. It is done after end user accepts invitation to enable prior handling by the application.
            FileSharingError error = FileSharingSession.isFileCapacityAcceptable(getContent().getSize());
            if (error != null) {
                // Send a 603 Decline response
                sendErrorResponse(getDialogPath().getInvite(), getDialogPath().getLocalTag(), 603);

                // Close session
                handleError(error);
                return;
            }

            MediaAttribute attr3 = mediaDesc.getMediaAttribute("path");
            String remotePath = attr3.getValue();
            String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription, mediaDesc);
            int remotePort = mediaDesc.port;
            
            // Extract the "setup" parameter
            String remoteSetup = "passive";
            MediaAttribute attr4 = mediaDesc.getMediaAttribute("setup");
            if (attr4 != null) {
                remoteSetup = attr4.getValue();
            }
            if (logger.isActivated()){
                logger.debug("Remote setup attribute is " + remoteSetup);
            }
            
            // Set setup mode
            String localSetup = createSetupAnswer(remoteSetup);
            if (logger.isActivated()){
                logger.debug("Local setup attribute is " + localSetup);
            }

            // Set local port
            int localMsrpPort;
            if (localSetup.equals("active")) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }            
            if(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()){
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }
             if (logger.isActivated()){
                    logger.debug("Local setup localMsrpPort is " + localMsrpPort);
            }
            
            // Create the MSRP manager
            String localIpAddress  = getImsService().getImsModule().getCurrentNetworkInterface().getNetworkAccess().getIpAddress();
            localIpAddress = ChatUtils.formatIPAddress(localIpAddress);  
            localIpAddress = ChatUtils.formatIPAddress(localIpAddress);
            msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            msrpMgr.setSecured(isSecured);

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
                "m=message " + localMsrpPort + " " + msrpMgr.getLocalSocketProtocol() + " *" + SipUtils.CRLF +
                "a=" + fileSelector + SipUtils.CRLF +
                "a=" + fileTransferId + SipUtils.CRLF +
                "a=accept-types:" + getContent().getEncoding() + SipUtils.CRLF +
                "a=file-disposition:attachment" + SipUtils.CRLF + // @tct-stack-[IOT-5_5_8] added by fang.wu
                "a=setup:" + localSetup + SipUtils.CRLF +
                "a=path:" + msrpMgr.getLocalMsrpPath() + SipUtils.CRLF +
                "a=fingerprint:" + KeyStoreManager.getFingerPrint() + SipUtils.CRLF +
                "a=recvonly" + SipUtils.CRLF;
            }
            else{
                sdp =
                "v=0" + SipUtils.CRLF +
                "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "s=-" + SipUtils.CRLF +
                "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                "t=0 0" + SipUtils.CRLF +            
                "m=message " + localMsrpPort + " " + msrpMgr.getLocalSocketProtocol() + " *" + SipUtils.CRLF +
                "a=" + fileSelector + SipUtils.CRLF +
                "a=" + fileTransferId + SipUtils.CRLF +
                "a=accept-types:" + getContent().getEncoding() + SipUtils.CRLF +
                "a=file-disposition:attachment" + SipUtils.CRLF + // @tct-stack-[IOT-5_5_8] added by fang.wu
                "a=setup:" + localSetup + SipUtils.CRLF +
                "a=path:" + msrpMgr.getLocalMsrpPath() + SipUtils.CRLF +
                "a=recvonly" + SipUtils.CRLF;
            }
            int maxSize = ImsFileSharingSession.getMaxFileSharingSize();
            if (maxSize > 0) {
                sdp += "a=max-size:" + maxSize + SipUtils.CRLF;
            }
            if(msrpCemaParameter != null){
                sdp += msrpCemaParameter;
            }

            // Set the local SDP part in the dialog path
            getDialogPath().setLocalContent(sdp);

            // Test if the session should be interrupted
            if (isInterrupted()) {
                if (logger.isActivated()) {
                    logger.debug("Session has been interrupted: end of processing");
                }
                return;
            }

            // Create the MSRP server session
            if (localSetup.equals("passive")) {
                // Passive mode: client wait a connection
                msrpMgr.createMsrpServerSession(remotePath, this);
                
                // Open the connection
                Thread thread = new Thread(){
                    public void run(){
                        try {
                            // Open the MSRP session
                            msrpMgr.openMsrpSession(ImsFileSharingSession.DEFAULT_SO_TIMEOUT);
                            
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
                    InstantMessagingService.CHAT_FEATURE_TAGS, sdp);
            }
            else{
                  if (logger.isActivated()) {
                    logger.info("TerminatinggROUPFileSharingSession  CPMS");
                 }
                  resp = SipMessageFactory.createCpm200OkInviteResponse(getDialogPath(),
                    InstantMessagingService.CPM_CHAT_FEATURE_TAGS, sdp);
            }

            if(RcsSettings.getInstance().isCPMSupported()){
                 if (logger.isActivated()) {
                    logger.info("TerminatingFileSharingSession1  CPMS");
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

                // Notify listeners
                for(int j=0; j < getListeners().size(); j++) {
                    getListeners().get(j).handleSessionStarted();
                }

                // Create the MSRP client session
                if (localSetup.equals("active")) {
                    // Active mode: client should connect
                    msrpMgr.createMsrpClientSession(remoteHost, remotePort, remotePath, this);

                    // Open the MSRP session
                    msrpMgr.openMsrpSession(ImsFileSharingSession.DEFAULT_SO_TIMEOUT);
                    
                    // Send an empty packet
                    sendEmptyDataChunk();
                }

                // The session is established
                getDialogPath().sessionEstablished();

                // Start session timer
                if (getSessionTimerManager().isSessionTimerActivated(resp)) {            
                    getSessionTimerManager().start(SessionTimerManager.UAS_ROLE, getDialogPath().getSessionExpireTime());
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("No ACK received for INVITE");
                }

                // No response received: timeout
                handleError(new FileSharingError(FileSharingError.SESSION_INITIATION_FAILED));
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }

            // Unexpected error
            handleError(new FileSharingError(FileSharingError.UNEXPECTED_EXCEPTION,
                    e.getMessage()));
        }
        
        if (logger.isActivated()) {
            logger.debug("End of thread");
        }
    }

    /**
     * Send an empty data chunk
     */
    public void sendEmptyDataChunk() {
        try {
            msrpMgr.sendEmptyChunk();
        } catch(Exception e) {
               if (logger.isActivated()) {
                   logger.error("Problem while sending empty data chunk", e);
               }
        }
    }    

    /**
     * Data has been transfered
     * 
     * @param msgId Message ID
     */
    public void msrpDataTransfered(String msgId) {
        // Not used in terminating side
    }
    
    /**
     * Send delivery report
     * 
     * @param status Report status
     */
    private void sendDeliveryReport(String status) {
        if(messageId == null){
            messageId = RichMessagingHistory.getInstance().getFileMessageId(this.getSessionID());
        }
        if (logger.isActivated()){
            logger.debug("Send delivery report " + messageId);
        }
        if (messageId != null) {
            if (logger.isActivated()){
                logger.debug("Send delivery report " + status);
            }            
            // Send message delivery status via a SIP MESSAGE
            ((InstantMessagingService) getImsService()).getImdnManager().sendMessageDeliveryStatus(getRemoteContact(), messageId, status);
        }
    }
    
    /**
     * Data transfer has been received
     * 
     * @param msgId Message ID
     * @param data Received data
     * @param mimeType Data mime-type 
     */
    public void msrpDataReceived(String msgId, byte[] data, String mimeType) {
        if (logger.isActivated()) {
            logger.info("Data received");
        }
        
        // File has been transfered
        fileTransfered();
    
        try {
            // Close content with received data
            getContent().writeData2File(data);
            getContent().closeFile();
            
            try {
                if(isImdnSupport()) {
                    sendDeliveryReport(ImdnDocument.DELIVERY_STATUS_DELIVERED);
                }
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.info("exception in sending delivery report");
                }
                e.printStackTrace();
            }

            // Notify listeners
            for(int j=0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener)getListeners().get(j)).handleFileTransfered(getContent().getUrl(),getContent());
            }
           } catch(IOException e) {
               // Delete the temp file
            deleteFile();

               // Notify listeners
            for(int j=0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(FileSharingError.MEDIA_SAVING_FAILED));
            }
           } catch(Exception e) {
               // Delete the temp file
            deleteFile();

            // Notify listeners
            for(int j=0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(FileSharingError.MEDIA_TRANSFER_FAILED));
            }
                    if (logger.isActivated()) {
                   logger.error("Can't save received file", e);
               }
           }
    }
    
    /**
     * Data transfer in progress
     * 
     * @param currentSize Current transfered size in bytes
     * @param totalSize Total size in bytes
     */
    public void msrpTransferProgress(long currentSize, long totalSize) {
        // Not used
    }

    /**
     * Data transfer in progress
     *
     * @param currentSize Current transfered size in bytes
     * @param totalSize Total size in bytes
     * @param data received data chunk
     */
    public boolean msrpTransferProgress(long currentSize, long totalSize, byte[] data) {
        try {
            // Update content with received data
            getContent().writeData2File(data);
            if(pauseInfo != null){
                pauseInfo.bytesTransferrred = currentSize;
                pauseInfo.pausedStream = getContent().getOut();
            }
            if (logger.isActivated()) {
                logger.info("FTS msrpTransferProgress bytes transferred: " + currentSize + "Old TransferId: " + oldFileTransferId
                            + "hasselector: ");
            }
            // Notify listeners
            for(int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j)).handleTransferProgress(currentSize, totalSize);
            }
        } catch(Exception e) {
               // Delete the temp file
            deleteFile();

            // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j)).handleTransferError(new FileSharingError(
                        FileSharingError.MEDIA_SAVING_FAILED, e.getMessage()));
            }
        }
        return true;
    }    

    /**
     * Data transfer has been aborted
     */
    public void msrpTransferAborted() {
        if (logger.isActivated()) {
            logger.info("Data transfer aborted");
        }
        
        if (!isFileTransfered()) {
               // Delete the temp file
            deleteFile();
        }
    }    

    /**
     * Prepare media session
     * 
     * @throws Exception 
     */
    public void prepareMediaSession() throws Exception {
        // Nothing to do in terminating side
    }

    /**
     * Start media session
     * 
     * @throws Exception 
     */
    public void startMediaSession() throws Exception {
        // Nothing to do in terminating side
    }

    /**
     * Close media session
     */
    public void closeMediaSession() {
        // Close MSRP session
        if (msrpMgr != null) {
            msrpMgr.closeSession();
            if (logger.isActivated()) {
                logger.debug("MSRP session has been closed");
            }
        }
        if (!isFileTransfered()) {
               // Delete the temp file
            //deleteFile();
        }
    }

    /** M: ftAutAccept @{ */
    public boolean shouldAutoAccept() {
        return mAutoAccept;
    }

    /** @} */

    /**
     * Delete file
     */
    private void deleteFile() {
        if (logger.isActivated()) {
            logger.debug("Delete incomplete received file");
        }
        try {
            getContent().deleteFile();
        } catch (IOException e) {
            if (logger.isActivated()) {
                logger.error("Can't delete received file", e);
            }
        }
    }
}
