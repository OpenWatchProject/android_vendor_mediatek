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
import java.text.ParseException;
import java.util.Vector;


import com.orangelabs.rcs.core.CoreException;
import javax.sip.header.ContentDispositionHeader;
import javax.sip.header.ContentLengthHeader;
import javax.sip.header.ContentTypeHeader;

import javax2.sip.header.ExtensionHeader;
import javax2.sip.header.Header;

import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
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
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsSessionBasedServiceError;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.ContributionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.FileTransferImpl;
import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.PauseResumeFileObject;
import com.orangelabs.rcs.utils.Base64;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;

import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;


/**
 * Originating file transfer session
 * 
 * @author jexa7410
 */
public class OriginatingLargeFileSharingSession extends ImsFileSharingSession implements MsrpEventListener {
    /**
     * Boundary tag
     */
    private final static String BOUNDARY_TAG = "boundary1";
    
    /**
     * MSRP manager
     */
    private MsrpManager msrpMgr = null;
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private boolean isResumed = false;
    
    private long totalFileSize = 0;
    
    long bytesTransferredtoSkip = 0;

    /**
     * Constructor
     * 
     * @param parent IMS service
     * @param content Content to be shared
     * @param contact Remote contact
     * @param thumbnail Thumbnail
     */
    public OriginatingLargeFileSharingSession(ImsService parent, MmContent content, String contact, byte[] thumbnail) {
        super(parent, content, contact, thumbnail);

        // Create dialog path
        createOriginatingDialogPath();

        // Set contribution ID
        String id = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
        setContributionID(id);    
        
        String newContact = PhoneUtils.extractNumberFromUri(contact);
        if (logger.isActivated()) {
            logger.info("CPMS OriginatingLargeFileSharingSession newContact: " + newContact);
        }
        
        if (RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.info("FTS OriginatingLargeFileSharingSession old call id: " + getDialogPath().getCallId());
            }
            if (RichMessagingHistory.getInstance().getCoversationID(newContact,1).equals("")) {
                // Set Call-Id
                String callId = getImsService().getImsModule().getSipManager().getSipStack().generateCallId();
                if (logger.isActivated()) {
                    logger.info("FTS OriginatingLargeFileSharingSession callId: " + callId);
                }
                String ConversationId = ContributionIdGenerator.getContributionId(callId);
                if (logger.isActivated()) {
                    logger.info("FTS OriginatingLargeFileSharingSession setConversationID: " + ConversationId);
                }
                setConversationID(ConversationId);
                RichMessagingHistory.getInstance().UpdateCoversationID(newContact,ConversationId,1);
            } else {
                setConversationID(RichMessagingHistory.getInstance().getCoversationID(newContact,1));
            }
        }

        if (content != null && !isReceiveOnly) {
            totalFileSize = content.getSize();
        }
        if (logger.isActivated()) {
            logger.info("FTS OriginatingLargeFileSharingSession content size: "
                    + totalFileSize);
        }
    }

    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("OriginatingLargeFileSharingSession Initiate a file transfer session as originating");
            }
            SipRequest invite = createSipInvite();
            
            // Send INVITE request
            sendInvite(invite);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("OriginatingLargeFileSharingSession Session initiation has failed", e);
            }

           // Unexpected error
            handleError(new FileSharingError(FileSharingError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }

        if (logger.isActivated()) {
            logger.debug("FTS End of thread");
        }
    }

    /**
     * @return A sip invite request
     */
    protected SipRequest createSipInvite(String callId) {
        logger.debug("FTS createSipInvite(), callId = " + callId);
        createOriginatingDialogPath(callId);
        return createSipInvite();
    }

    private SipRequest createSipInvite() {
        logger.debug("FTS createSipInvite()");
            // Set setup mode
            String localSetup = createSetupOffer();
            if (logger.isActivated()){
                logger.debug("FTS Local setup attribute is " + localSetup);
            }

            // Set local port
            int localMsrpPort;
            if ("active".equals(localSetup)) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }

            // Create the MSRP manager
            String localIpAddress = getImsService().getImsModule().getCurrentNetworkInterface().getNetworkAccess().getIpAddress();
            localIpAddress = ChatUtils.formatIPAddress(localIpAddress);
            msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            if (getImsService().getImsModule().isConnectedToWifiAccess()) {
                msrpMgr.setSecured(RcsSettings.getInstance().isSecureMsrpOverWifi());
            }

            // Build SDP part
            String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
            String ipAddress = ChatUtils.formatIPAddress(getDialogPath().getSipStack().getLocalIpAddress());
            logger.debug(" getDialogPath().getSipStack().getLocalIpAddress(); after format1: " +  ipAddress);
            
            String encoding = getContent().getEncoding();
            String sdp = null;
            String sendMode = "a=sendonly";
            String msrpCemaParameter = null;
            if(RcsSettings.getInstance().isSupportOP07()){
                msrpCemaParameter = "a=msrp-cema" + SipUtils.CRLF;
            }
            String filetransferID = getSessionID();//getFileTransferId();
            if(!isReceiveOnly){
                if (logger.isActivated()){
                    logger.debug("FTS createSipInvite isReceiveOnly: " + isReceiveOnly() + "sendMode: sendOnly");
                }
                sendMode = "a=sendonly";
            } else{
                if (logger.isActivated()){
                    logger.debug("FTS createSipInvite isReceiveOnly: " + isReceiveOnly() + "sendMode: recvOnly" +"oldFileTransferId:" + pauseInfo.mOldFileTransferId
                            + "this.isFileTransferPaused() :" + this.isFileTransferPaused());
                }
                sendMode = "a=recvonly";
                if(this.isFileTransferPaused() && pauseInfo != null)
                filetransferID = pauseInfo.mOldFileTransferId;
                
            }
            int timeLen = getTimeLen();
        if (logger.isActivated()) {
            logger.debug("FTS createSipInvite"
                    + " timeLen: " + timeLen
                    + " IsSecure: " + isSecureProtocolMessage()
                    + " IsPaused: " + this.isFileTransferPaused());
        }

        sdp =
            "v=0" + SipUtils.CRLF +
            "o=- " + ntpTime + " " + ntpTime + " "
                + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "s=-" + SipUtils.CRLF +
            "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
            "t=0 0" + SipUtils.CRLF +
            "m=message " + localMsrpPort + " " + msrpMgr.getLocalSocketProtocol() +
                " *" + SipUtils.CRLF +
            "a=path:" + msrpMgr.getLocalMsrpPath() + SipUtils.CRLF;
        if (isSecureProtocolMessage())
            sdp +=
            "a=fingerprint:" + KeyStoreManager.getFingerPrint() + SipUtils.CRLF;
        sdp +=
            "a=setup:" + localSetup + SipUtils.CRLF +
            "a=accept-types:*" + SipUtils.CRLF;
        sdp +=
        "a=accept-wrapped-types:*" + SipUtils.CRLF;
        sdp +=
            sendMode + SipUtils.CRLF;
        
        if(msrpCemaParameter != null){
            sdp += msrpCemaParameter;
        }
        // Set File-selector attribute
        String selector = " size:" + totalFileSize;
        if (selector != null) {
            sdp += "a=file-selector:" + selector + SipUtils.CRLF;
        }
        
        // Set the local SDP part in the dialog path
        getDialogPath().setLocalContent(sdp);
        
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
     * Prepare media session
     * 
     * @throws Exception 
     */
    public void prepareMediaSession() throws Exception {
        if (logger.isActivated()) {
            logger.info("FTS prepareMediaSession isRecv: "
                    + this.isReceiveOnly() + " isPaused: "
                    + this.isFileTransferPaused());
        }
        SdpParser parser = new SdpParser(getDialogPath().getRemoteContent().getBytes());
        Vector<MediaDescription> media = parser.getMediaDescriptions();
        MediaDescription mediaDesc = media.elementAt(0);
        MediaAttribute attr = mediaDesc.getMediaAttribute("path");
        String remoteMsrpPath = attr.getValue();
        String remoteHost = SdpUtils.extractRemoteHost(
                parser.sessionDescription, mediaDesc);
        int remotePort = mediaDesc.port;
        // Extract the "setup" parameter
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
                                logger.error("FTS Can't create the MSRP server session", e);
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
        // Parse the remote SDP part

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
            logger.info("FTS startMediaSession isRecv: " + this.isReceiveOnly() + " isPaused: " + this.isFileTransferPaused());
        }
        if(this.isReceiveOnly() && this.isFileTransferPaused()){
            if (logger.isActivated()) {
                logger.error("No need to startMediaSession ");
            }
            return;
        }
        // Open the MSRP session
        msrpMgr.openMsrpSession();
        if(RcsSettings.getInstance().supportOP01()) {
            sendEmptyDataChunk();
        }

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
            String imdnString = null;
            String contentType = CpimMessage.MIME_TYPE;
            String imdnMsgId = ChatUtils.generateMessageId();
            String imdnFrom = ChatUtils.ANOMYNOUS_URI;
            String imdnTo = ChatUtils.ANOMYNOUS_URI;
            imdnString = ChatUtils.buildCpimMessageWithImdn(imdnFrom,imdnTo,imdnMsgId,getContent().getEncoding());
            //msrpMgr.setCpimType(true);
            msrpMgr.setCPIM(imdnString);
            msrpMgr.sendChunks(stream, ChatUtils.generateMessageId(), contentType, getContent().getSize()-bytesTransferredtoSkip);
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
            logger.info("FTS Data transfered contact: " + contact + " msgId: " + msgId);
        }

        // File has been transfered
        fileTransfered();
        
        Thread t = new Thread(){
            public void run(){
             // Notify listeners
                if (logger.isActivated()) {
                    logger.info("msrpDataTransfered inform to Impl");
                }
                for(int j=0; j < getListeners().size(); j++) {
                    ((FileSharingSessionListener)getListeners().get(j)).handleFileTransfered(getContent().getUrl(),getContent());
                }
            }
        };
        t.start();

        // Close the media session
        closeMediaSession();

        // Terminate session
        terminateSession(ImsServiceSession.TERMINATION_BY_USER);

        // Remove the current session
        getImsService().removeSession(this);
        
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
            logger.info("FTS msrpDataReceived received");
        }
        
        // File has been transfered
        fileTransfered();
    
        // Send text message
        Thread t = new Thread() {
            public void run() {
                if (logger.isActivated()) {
                    logger.info("FTS Data received1");
                }
                if(RcsSettings.getInstance().supportOP01() || RcsSettings.getInstance().isSupportOP08()) {
                    sendDeliveryReport(ImdnDocument.DELIVERY_STATUS_DELIVERED);
                }
            }
        };
        t.start();
    
        try {
            // Close content with received data
            getContent().writeData2File(data);
            getContent().closeFile();

            // Notify listeners
            for(int j=0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener)getListeners().get(j)).handleFileTransfered(getContent().getUrl(),getContent());
            }
           } catch(IOException e) {
           if (logger.isActivated()) {
                logger.error("Error delete incomplete file" + e);
            }
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
                   logger.error("FTS Can't save received file", e);
               }
           }
    
    }
    
     /**
     * Delete file
     */
    private void deleteFile() {
        if (logger.isActivated()) {
            logger.error("FTS Delete incomplete received file");
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
        hashselector = hashselector.trim();
        if (pauseInfo != null) {
            pauseInfo.bytesTransferrred = currentSize;
            pauseInfo.hashSelector = hashselector;
        }
         if (logger.isActivated()) {
                 logger.info("FTS msrpTransferProgress bytes transferred: " + currentSize + "pauseResumeObject:" + pauseInfo + "FT TransferId: " + getSessionID()
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

            if (pauseInfo != null){
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
            if (logger.isActivated()) {
                logger.info("FTS msrpTransferProgress exception " + e.getMessage());
            }
        }

        return true;

    }

    /**
     * Data transfer error
     *
     * @param msgId Message ID
     * @param error Error code
     */
    public void msrpTransferError(String msgId, String error) {
        if (isSessionInterrupted()) {
            return;
        }
        
        if (logger.isActivated()) {
            logger.info("Data transfer error " + error);
        }
        String statusCode = null;
        int errorCode = FileSharingError.MEDIA_TRANSFER_FAILED;
        try{
            statusCode = error.substring(12);
        } catch(Exception e){
            logger.error("msrpTransferError ", e);
        }

        try {
            // Terminate session
            if(statusCode != null && statusCode.contains("481")){
                terminateSession(ImsServiceSession.TERMINATION_BY_MSRP_ERROR);
                if(RcsSettings.getInstance().isSupportOP08()){
                    errorCode = FileSharingError.MEDIA_RESUME_FAILED;
                }
            } else if(statusCode != null && statusCode.contains("403")){
                terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
                errorCode = FileSharingError.MEDIA_TRANSFER_FAILED;
            } else if(statusCode != null && statusCode.contains("413")){
                terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
                errorCode = FileSharingError.MEDIA_RESUME_FAILED;
            } else if(statusCode != null && statusCode.contains(MsrpSession.ACK_NOT_RECEIVED)){
                terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
                if(this.isResend()){
                    errorCode = FileSharingError.MEDIA_TRANSFER_FAILED;
                } else {
                    errorCode = FileSharingError.FT_AUTO_RESEND;
                }
            } else {
                terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
                errorCode = FileSharingError.MEDIA_TRANSFER_FAILED;
                if(RcsSettings.getInstance().isSupportOP08()){
                    errorCode = FileSharingError.MEDIA_TRANSFER_FAILED; 
                }
            }

            // Close the media session
            closeMediaSession();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't close correctly the file transfer session", e);
            }
        }

        // Request capabilities
        getImsService().getImsModule().getCapabilityService().requestContactCapabilities(getDialogPath().getRemoteParty());

        // Remove the current session
        getImsService().removeSession(this);

        // Notify listeners
        for(int j=0; j < getListeners().size(); j++) {
            ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(errorCode, error));
        }
    }

    /**
     * Data transfer has been aborted
     */
    public void msrpTransferAborted() {
        if (logger.isActivated()) {
            logger.error("FTS Data transfer aborted");
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
        int code = FileSharingError.MEDIA_FALLBACK_MMS;
        
        if(RcsSettings.getInstance().isSupportOP07() && !this.isUseLargeMode()){
            code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK;
        }

        // Notify listeners
        for(int j=0; j < getListeners().size(); j++) {
            ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(code, error.getMessage()));
        }
    }
    
    /**
     * Handle 480 
     * 
     * @param resp SipResponse
     */
    public void handle480Unavailable(SipResponse resp) {
        if (isSessionInterrupted()) {
            return;
        }
        if (logger.isActivated()) {
            logger.debug("handle480Unavailable");
        }
        if(RcsSettings.getInstance().isSupportOP07()){
            int code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK;
            if(this.isUseLargeMode()){
                code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK_MMS;
            }
            handleError(new ImsSessionBasedServiceError(code,
                    resp.getStatusCode() + " " + resp.getReasonPhrase()));
        } else {
            handleDefaultError(resp);
        }
    }
    
    /**
     * Handle 480 
     * 
     * @param resp SipResponse
     */
    public void handle404SessionNotFound(SipResponse resp) {
        if (isSessionInterrupted()) {
            return;
        }
        if (logger.isActivated()) {
            logger.debug("handle404SessionNotFound");
        }
        if(RcsSettings.getInstance().isSupportOP07()){
            int code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK;
            if(this.isUseLargeMode()){
                code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK_MMS;
            }
            handleError(new ImsSessionBasedServiceError(code,
                    resp.getStatusCode() + " " + resp.getReasonPhrase()));
        } else {
            handleDefaultError(resp);
        }
    }
    
    /**
     * Handle 663 
     * 
     * @param resp SipResponse
     */
    public void handle663TimerExpiry(SipResponse resp) {
        if (isSessionInterrupted()) {
            return;
        }
        if (logger.isActivated()) {
            logger.debug("handle663TimerExpiry");
        }
        if(RcsSettings.getInstance().isSupportOP07()){
            int code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK;
            if(this.isUseLargeMode()){
                code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK_MMS;
            }
            handleError(new ImsSessionBasedServiceError(code,
                    resp.getStatusCode() + " " + resp.getReasonPhrase()));
        } else {
            handleDefaultError(resp);
        }
    }
    
    public void handleMsrpConnectionException(Exception e) {
        if (logger.isActivated()) {
            logger.error("handleMsrpConnectionException");
        }
        if (logger.isActivated()) {
            logger.error("handleMsrpConnectionException");
        }
        int code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK;
        if(this.isUseLargeMode()){
            code = ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK_MMS;
        }
        handleError(new ImsSessionBasedServiceError(code, "connectionrefused"));
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
            logger.debug("FTS MSRP session has been closed");
        }
    }
}
