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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Vector;

import com.orangelabs.rcs.core.CoreException;
import javax.sip.header.ContentDispositionHeader;
import javax.sip.header.ContentLengthHeader;
import javax.sip.header.ContentTypeHeader;

import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpEventListener;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpManager;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpSession;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaAttribute;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.ContributionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.PauseResumeFileObject;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.utils.Base64;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.ims.service.im.chat.ListOfParticipant;

import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;


/**
 * Originating file transfer session
 * 
 * @author jexa7410
 */
public class OriginatingGroupFileSharingSession extends ImsFileSharingSession implements MsrpEventListener {
    /**
     * Boundary tag
     */
    private final static String BOUNDARY_TAG = "boundary1";
    
    /**
     * MSRP manager
     */
    private MsrpManager msrpMgr = null;

    /**
     * Chat session ID
     */
    private String chatSessionId = null;
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private boolean isResumed = false;
    
    private long byteTransferredCount = -1;
    
    private long totalFileSize = -1;
    
    long bytesTransferredtoSkip = 0;

    /**
     * Constructor
     *
     * @param parent IMS service
     * @param content Content to be shared
     * @param contact Remote contact
     * @param thumbnail Thumbnail
     */
    public OriginatingGroupFileSharingSession(ImsService parent, MmContent content, String conferenceId, ListOfParticipant participants, byte[] thumbnail, String chatSessionID, String chatContributionId) {
        super(parent, content, conferenceId, thumbnail);

        // Create dialog path
        createOriginatingDialogPath();

        // Set contribution ID
        String id = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
        setContributionID(id);

        if (RcsSettings.getInstance().isCPMSupported()) {
            if (logger.isActivated()) {
                logger.info("CPMS OriginatingGroupFileSharingSession old call id: "
                        + getDialogPath().getCallId());
            }
            RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
            String conversationId = rmHistory.getCoversationID(chatContributionId, 2);
            if (conversationId.isEmpty()) {
                // Set Call-Id
                String callId = getImsService().getImsModule().getSipManager().getSipStack().generateCallId();
                if (logger.isActivated()) {
                    logger.info("CPMS OriginatingGroupFileSharingSession callId: " + callId);
                }
                // Set conversation ID
                conversationId = ContributionIdGenerator.getContributionId(callId);
                setConversationID(conversationId);
                RichMessagingHistory.getInstance().UpdateCoversationID(
                        chatContributionId, conversationId, 1);
            } else {
                setConversationID(conversationId);
            }
        }

        this.chatSessionId = chatSessionID;
        // Set participants involved in the transfer
        this.participants = participants;

        byteTransferredCount = -1;
        if (content != null) {
            totalFileSize = content.getSize();
        }
        if (logger.isActivated()) {
            logger.info("FTS OriginatingFileSharingSession content size: "
                    + totalFileSize);
        }
    }

    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("Initiate a group file transfer session as originating");
            }
            /**
             * M: Modified to resolve the 403 error issue.@{
             */
            SipRequest invite = createSipInvite();
            /**
             * @}
             */

            // Send INVITE request
            sendInvite(invite);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }
            
           // Unexpected error
            handleError(new FileSharingError(FileSharingError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }

        if (logger.isActivated()) {
            logger.debug("End of thread");
        }
    }
            
    /**
     * M: Modified to resolve the 403 error issue.@{
     */
    /**
     * @return A sip invite request
     */

    protected SipRequest createSipInvite(String callId) {
        logger.debug("createSipInvite(), callId = " + callId);
        createOriginatingDialogPath(callId);
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
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }

            // Create the MSRP manager
            String localIpAddress  = getImsService().getImsModule().getCurrentNetworkInterface().getNetworkAccess().getIpAddress();
            localIpAddress = ChatUtils.formatIPAddress(localIpAddress);  
            msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            if (getImsService().getImsModule().isConnectedToWifiAccess()) {
                msrpMgr.setSecured(RcsSettings.getInstance().isSecureMsrpOverWifi());
            }

            // Build SDP part
            String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
            String ipAddress = ChatUtils.formatIPAddress(getDialogPath().getSipStack().getLocalIpAddress());
            logger.debug(" getDialogPath().getSipStack().getLocalIpAddress(); after format: " +  ipAddress);
            
            String encoding = getContent().getEncoding();
            String sdp = null;
            String msrpCemaParameter = null;
            if(RcsSettings.getInstance().isSupportOP07()){
                msrpCemaParameter = "a=msrp-cema" + SipUtils.CRLF;
            }
            String sendMode = "a=sendonly";
            String filetransferID = getSessionID();// getFileTransferId();
            if (!isReceiveOnly) {
                if (logger.isActivated()) {
                    logger.debug("GFTS createSipInvite isReceiveOnly: "
                            + isReceiveOnly() + "sendMode: sendOnly");
                }
                sendMode = "a=sendonly";
            } else {
                if (logger.isActivated()) {
                    logger.debug("GFTS createSipInvite isReceiveOnly: "
                            + isReceiveOnly() + "sendMode: recvOnly"
                            + "oldFileTransferId:"
                            + pauseInfo.mOldFileTransferId
                            + "this.isFileTransferPaused() :"
                            + this.isFileTransferPaused());
                }
                sendMode = "a=recvonly";
                if (this.isFileTransferPaused() && pauseInfo != null)
                    filetransferID = pauseInfo.mOldFileTransferId;
    
            }
            int timeLen = getTimeLen();

        sdp = 
            "v=0" + SipUtils.CRLF +
            "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "s=-" + SipUtils.CRLF +
            "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "t=0 0" + SipUtils.CRLF +
            "m=message " + localMsrpPort + " " + msrpMgr.getLocalSocketProtocol() + " *" + SipUtils.CRLF +
            "a=path:" + msrpMgr.getLocalMsrpPath() + SipUtils.CRLF;
        if (isSecureProtocolMessage())
            sdp +=
            "a=fingerprint:" + KeyStoreManager.getFingerPrint() + SipUtils.CRLF;
        sdp +=
            "a=setup:" + localSetup + SipUtils.CRLF +
            "a=accept-types:" + encoding + SipUtils.CRLF;
        if (isGeoLocFile())
            sdp +=
            "a=accept-wrapped-types:" + encoding + SipUtils.CRLF;
        sdp +=
            "a=file-transfer-id:" + filetransferID + SipUtils.CRLF;
        if (timeLen > 0)
            sdp +=
            "a=file-disposition:timelen="+ timeLen + SipUtils.CRLF;
        sdp +=
            sendMode + SipUtils.CRLF;
        
        if(msrpCemaParameter != null){
            sdp += msrpCemaParameter;
        }

            int maxSize = FileSharingSession.getMaxFileSharingSize();
            if (maxSize > 0) {
                sdp += "a=max-size:" + maxSize + SipUtils.CRLF;
            }
            
            // Set File-selector attribute
            String selector = getFileSelectorAttribute();
            

            if(isReceiveOnly && pauseInfo!= null)
            {
                selector = "name:\"" + getContent().getName() + "\"" + 
                        " type:" + getContent().getEncoding() +
                        " size:" + (pauseInfo.mSize - pauseInfo.bytesTransferrred) + " ";
                totalFileSize = pauseInfo.mSize;
                if (logger.isActivated()){
                    logger.debug("FTS pauseResumeObject msize is  : " + totalFileSize );
                }
                getContent().setOut(pauseInfo.pausedStream);
            }
            if (this.isFileTransferPaused() && pauseInfo != null) {
                if (logger.isActivated()){
                    logger.debug("GFTS pauseResumeObject & hashselector : "
                            + pauseInfo + "selctor is:"
                            + pauseInfo.hashSelector);
                }
                hashselector = pauseInfo.hashSelector;
            } else {
                hashselector = getHashSelectorAttribute();
            if (pauseInfo != null) {
                pauseInfo.hashSelector = hashselector;
            }
            if (logger.isActivated()) {
                logger.debug("GFTS First INVITE pauseResumeObject & hashselector : "
                            + pauseInfo
                        + "selector is:" + hashselector
                            + "sessionID:" + getSessionID());
            }
            }
            if(this.isFileTransferPaused() && pauseInfo != null && pauseInfo.bytesTransferrred > 1 && !isReceiveOnly)
            {

                selector = "name:\"" + getContent().getName() + "\"" + 
                        " type:" + getContent().getEncoding() +
                        " size:" + (getContent().getSize() /*- pauseResumeObject.bytesTransferrred*/) + " ";
                if (logger.isActivated()){
                    logger.debug("GFTS pauseResumeObject pauseResumeObject.bytesTransferrred > 1 is  : " + pauseInfo.bytesTransferrred );
                }
             
            }
            if (selector != null) {
                sdp += "a=file-selector:" + selector + hashselector + SipUtils.CRLF;
            }
            long bytesTransferrred = 0;

            if(this.isFileTransferPaused() && pauseInfo!= null)
            {
                 if (logger.isActivated()) {
                    logger.info("File Resumed true while INVITE");
                }               
                bytesTransferrred = pauseInfo.bytesTransferrred;                
                bytesTransferredtoSkip = bytesTransferrred;
                 if (logger.isActivated()) {
                        logger.info("bytes transferred: " + bytesTransferrred + "Old TransferId: " + oldFileTransferId);
                   }
                String fileRange = "a=file-range:" + (bytesTransferrred + 1) + "-" + totalFileSize; 
                sdp += fileRange + SipUtils.CRLF;
            }

            // Set File-location attribute
            String location = getFileLocationAttribute();
            if (location != null) {
                sdp += "a=file-location:" + location + SipUtils.CRLF;
            }

            if (getThumbnail() != null) {
                // Encode the thumbnail file
                String imageEncoded = Base64.encodeBase64ToString(getThumbnail());
                
                if(imageEncoded.length() < 10 * 1024 && imageEncoded.length() > 0){ // 10k is the maximum size of thumbnail as mentioned in RCS specs.
                sdp += "a=file-icon:cid:image@joyn.com" + SipUtils.CRLF;

                // Build multipart
                String multipart = 
                        Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                        ContentTypeHeader.NAME + ": application/sdp" + SipUtils.CRLF +
                        ContentLengthHeader.NAME + ": " + sdp.getBytes().length + SipUtils.CRLF +
                        SipUtils.CRLF +
                        sdp + SipUtils.CRLF + 
                        Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                        ContentTypeHeader.NAME + ": " + "image/jpeg" + SipUtils.CRLF +
                        SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": base64" + SipUtils.CRLF +
                        SipUtils.HEADER_CONTENT_ID + ": <image@joyn.com>" + SipUtils.CRLF +
                        ContentLengthHeader.NAME + ": "+ imageEncoded.length() + SipUtils.CRLF +
                        ContentDispositionHeader.NAME + ": icon" + SipUtils.CRLF +
                        SipUtils.CRLF +
                        imageEncoded + SipUtils.CRLF +
                        Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + Multipart.BOUNDARY_DELIMITER;

                // Set the local SDP part in the dialog path
                getDialogPath().setLocalContent(multipart);       
                } else {
                    setThumbnail(null);
                    // Set the local SDP part in the dialog path
                    getDialogPath().setLocalContent(sdp);
                }
            } else {
                // Set the local SDP part in the dialog path
                getDialogPath().setLocalContent(sdp);
            }
            try{
            // Create an INVITE request
            if (logger.isActivated()) {
                logger.info("Send INVITE");
            }
            SipRequest invite = createInvite();
            
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
     * Prepare media session
     * 
     * @throws Exception 
     */
    public void prepareMediaSession() throws Exception {
        // Parse the remote SDP part
        SdpParser parser = new SdpParser(getDialogPath().getRemoteContent().getBytes());
        Vector<MediaDescription> media = parser.getMediaDescriptions();
        MediaDescription mediaDesc = media.elementAt(0);
        MediaAttribute attr = mediaDesc.getMediaAttribute("path");
        String remoteMsrpPath = attr.getValue();
        String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription, mediaDesc);
        int remotePort = mediaDesc.port;

        String remoteSetup = "passive";
        MediaAttribute attr4 = mediaDesc.getMediaAttribute("setup");
        if (attr4 != null) {
            remoteSetup = attr4.getValue();
        }
        if (logger.isActivated()) {
            logger.debug("FTS Remote setup attribute is " + remoteSetup);
        }
        
        if(this.isReceiveOnly() && this.isFileTransferPaused()){
            if (logger.isActivated()) {
                logger.error("FTS No need to prepareMediaSession ");
            }
            String localSetup = createSetupAnswer(remoteSetup);
            if (logger.isActivated()) {
                logger.debug("FTS Local setup attribute is " + localSetup);
            }

            // Set local port
            int localMsrpPort;
            if (localSetup.equals("active")) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }

            // Create the MSRP manager
            String localIpAddress = getImsService().getImsModule()
                    .getCurrentNetworkInterface().getNetworkAccess()
                    .getIpAddress();
            msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            // msrpMgr.setSecured(isSecured);
            // Create the MSRP server session
            if (localSetup.equals("passive")) {
                // Passive mode: client wait a connection
                msrpMgr.createMsrpServerSession(remoteMsrpPath, this);
                // Open the connection
                Thread thread = new Thread() {
                    public void run() {
                        try {
                            // Open the MSRP session
                            msrpMgr.openMsrpSession(ImsFileSharingSession.DEFAULT_SO_TIMEOUT);
                            // Send an empty packet
                            sendEmptyDataChunk();
                        } catch (IOException e) {
                            if (logger.isActivated()) {
                                logger.error(
                                        "FTS Can't create the MSRP server session",
                                        e);
                            }
                        }
                    }
                };
                thread.start();
            }

            // Create the MSRP client session
            if (localSetup.equals("active")) {
                // Active mode: client should connect
                msrpMgr.createMsrpClientSession(remoteHost, remotePort,
                        remoteMsrpPath, this);

                // Open the MSRP session
                msrpMgr.openMsrpSession(ImsFileSharingSession.DEFAULT_SO_TIMEOUT);

                // Send an empty packet
                sendEmptyDataChunk();
            }
             // The session is established
            getDialogPath().sessionEstablished();
            return;
        }

        // Create the MSRP client session
        MsrpSession session = msrpMgr.createMsrpClientSession(remoteHost, remotePort, remoteMsrpPath, this);
        session.setFailureReportOption(true);
        session.setSuccessReportOption(false);
    }

    /**
     * Send an empty data chunk
     */
    public void sendEmptyDataChunk() {
        try {
            msrpMgr.sendEmptyChunk();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Problem while sending empty data chunk", e);
            }
        }
    }

    /**
     * Start media session
     * 
     * @throws Exception 
     */
    public void startMediaSession() throws Exception {
        
        if (logger.isActivated()) {
            logger.info("GFTS startMediaSession isRecv: " + this.isReceiveOnly() + " isPaused: " + this.isFileTransferPaused());
        }
        if(this.isReceiveOnly() && this.isFileTransferPaused()){
            if (logger.isActivated()) {
                logger.error("No need to startMediaSession ");
            }
            return;
        }
        // Open the MSRP session
        msrpMgr.openMsrpSession();

        try {
            // Start sending data chunks
            byte[] data = getContent().getData();
            InputStream stream; 
            if (data == null) {
                // Load data from URL
                stream = FileFactory.getFactory().openFileInputStream(getContent().getUrl());
            } else {
                // Load data from memory
                stream = new ByteArrayInputStream(data);
            }
            if(isFileTransferPaused())
            {               
                //bytesTransferredtoSkip = FileTransferServiceImpl.getPauseInfo(getSessionID()).bytesTransferrred;
                if(bytesToSkip != 0){
                    bytesTransferredtoSkip = bytesToSkip -1;
                }
                if(bytesTransferredtoSkip != 0)
                stream.skip(bytesTransferredtoSkip);
                if (logger.isActivated()) {
                    logger.error("startMediaSession resumed interrupted file byteskipped :" + bytesTransferredtoSkip);
                }
            }        
            if (logger.isActivated()) {
                logger.info("startMediaSession sendtomsrp");
            }
            msrpMgr.sendChunks(stream, ChatUtils.generateMessageId(), getContent().getEncoding(), getContent().getSize());
        } catch(Exception e) {
            // Unexpected error
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }
            handleError(new ImsServiceError(ImsServiceError.UNEXPECTED_EXCEPTION,
                    e.getMessage()));
        }
    }

    /**
     * Data has been transfered
     * 
     * @param msgId Message ID
     */
    public void msrpDataTransfered(String msgId) {

        String contact = this.getRemoteContact();
        if (logger.isActivated()) {
            logger.info("Data transfered contact: " + contact);
        }
        
        // File has been transfered
        fileTransfered();
        
        // Close the media session
        closeMediaSession();
        
        // Terminate session
        terminateSession(ImsServiceSession.TERMINATION_BY_USER);
           
        // Remove the current session
        getImsService().removeSession(this);

        // Notify listeners
        for(int j=0; j < getListeners().size(); j++) {
            ((FileSharingSessionListener)getListeners().get(j)).handleFileTransfered(getContent().getUrl(),getContent());
        }

        // Notify delivery
        //((InstantMessagingService) getImsService()).receiveFileDeliveryStatus(getSessionID(), ImdnDocument.DELIVERY_STATUS_DELIVERED,contact);
        //((InstantMessagingService) getImsService()).receiveFileDeliveryStatus(getSessionID(), ImdnDocument.DELIVERY_STATUS_DISPLAYED,contact);
    }
    
    /**
     * Delete file
     */
    private void deleteFile() {
        if (logger.isActivated()) {
            logger.debug("FTS Delete incomplete received file");
        }
        try {
            getContent().deleteFile();
        } catch (IOException e) {
            if (logger.isActivated()) {
                logger.error("FTS Can't delete received file", e);
            }
        }
    }
    
    /**
     * Send delivery report
     * 
     * @param status Report status
     */
    private void sendDeliveryReport(String status) {
        String messageId = RichMessagingHistory.getInstance().getFileMessageId(this.getSessionID());
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
            logger.info("GFTS msrpDataReceived received");
        }
        
        // File has been transfered
        fileTransfered();
    
        try {
            // Close content with received data
            getContent().writeData2File(data);
            getContent().closeFile();
            
         // Send text message
            Thread t = new Thread() {
                public void run() {
                    if (logger.isActivated()) {
                        logger.info("FTS Data received1");
                    }
                    if(RcsSettings.getInstance().isSupportOP08()) {
                        sendDeliveryReport(ImdnDocument.DELIVERY_STATUS_DELIVERED);
                    }
                }
            };
            t.start();

            // Notify listeners
            for(int j=0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener)getListeners().get(j)).handleFileTransfered(getContent().getUrl(),getContent());
            }
        } catch(IOException e) {
            e.printStackTrace();
            // Delete the temp file
            deleteFile();

            // Notify listeners
            for(int j=0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(FileSharingError.MEDIA_SAVING_FAILED));
            }
        } catch(Exception e) {
            e.printStackTrace();
            // Delete the temp file
            deleteFile();

            // Notify listeners
            for(int j=0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(FileSharingError.MEDIA_TRANSFER_FAILED));
            }
                    if (logger.isActivated()) {
                logger.error("GFTS Can't save received file", e);
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
        // Notify listeners
        for(int j=0; j < getListeners().size(); j++) {
            ((FileSharingSessionListener)getListeners().get(j)).handleTransferProgress(currentSize, totalSize);
        }
        if(pauseInfo != null) {
            pauseInfo.bytesTransferrred = currentSize;
            pauseInfo.hashSelector = hashselector;
        }
         if (logger.isActivated()) {
                logger.info("GFTS msrpTransferProgress bytes transferred: " + currentSize + "pauseResumeObject:" + pauseInfo + "FT TransferId: " + getSessionID()
                        + "hasselector: " + hashselector);
         }
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
                if (logger.isActivated()) {
                    logger.info("FTS msrpTransferProgress pauseResumeObject : " + pauseInfo);
             }
                pauseInfo.bytesTransferrred = currentSize; 
                pauseInfo.pausedStream = getContent().getOut();
            }
             if (logger.isActivated()) {
                    logger.info("FTS msrpTransferProgress bytes transferred: " + currentSize + " TransferId: " + getSessionID()
                            + "hasselector: ");
             }
            
            // Notify listeners
            for(int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j)).handleTransferProgress(currentSize, totalSize);
            }
        } catch(Exception e) {
            // Delete the temp file
            //deleteFile();
            if (logger.isActivated()) {
                logger.info("FTS msrpTransferProgress exception " + e.getMessage());
         }
           /* // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j)).handleTransferError(new FileSharingError(
                        FileSharingError.MEDIA_SAVING_FAILED, e.getMessage()));
            }*/
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
    }

    /**
     * Close media session
     */
    public void closeMediaSession() {
        // Close MSRP session
        if (msrpMgr != null) {
            msrpMgr.closeSession();
        }
        if (logger.isActivated()) {
            logger.debug("MSRP session has been closed");
        }
    }
    
    /**
     * Handle error 
     * 
     * @param error Error
     */
    public void handleResponseTimeout(ImsServiceError error) {
        if (isSessionInterrupted()) {
            return;
        }

        // Error    
        if (logger.isActivated()) {
            logger.info("Session error: " + error.getErrorCode() + ", reason=" + error.getMessage());
        }

        // Close media session
        closeMediaSession();

        // Remove the current session
        getImsService().removeSession(this);

        // Notify listeners
        for(int j=0; j < getListeners().size(); j++) {
            ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(FileSharingError.MEDIA_FALLBACK_MMS, error.getMessage()));
        }
    }
}
