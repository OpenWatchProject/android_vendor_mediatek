package com.orangelabs.rcs.core.ims.service.im.filetransfer;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;

import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.RcsService.Direction;

import javax2.sip.header.ContactHeader;
import javax2.sip.header.ContentDispositionHeader;
import javax2.sip.header.ContentLengthHeader;
import javax2.sip.header.ContentTypeHeader;
import javax2.sip.header.ExtensionHeader;
import javax2.sip.header.Header;
import javax2.sip.header.WarningHeader;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
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
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsSessionBasedServiceError;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.ContributionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.ListOfParticipant;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.file.FileDescription;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.PauseResumeFileObject;
import com.orangelabs.rcs.utils.Base64;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.utils.logger.Logger;

public class OriginatingExtendedFileSharingSession extends ImsFileSharingSession implements MsrpEventListener {
    
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    private final static String BOUNDARY_TAG = "boundary1";

    private String[] featureTags;
    private List<String> extraTags;
    private String extraContent;
    private String extraContentType;
    private String preferService;

    private MsrpManager msrpMgr;

    /**
     * Constructor
     * 
     * @param parent IMS service
     * @param content Content to be shared
     * @param contact Remote contact
     * @param thumbnail Thumbnail
     */
    public OriginatingExtendedFileSharingSession(
            ImsService parent,
            MmContent content,
            String contact,
            List<String> contacts,
            String chatId,
            byte[] thumbnail) {
        super(parent, content, contact, thumbnail);
        this.participants = new ListOfParticipant(contacts);

        // Create dialog path
        createOriginatingDialogPath();

        // Set contribution ID
        String id = ContributionIdGenerator.getContributionId(getDialogPath().getCallId());
        setContributionID(id);

        String newContact;
        newContact = PhoneUtils.generateContactsText(contacts);

        if (RcsSettings.getInstance().isCPMSupported()) {
            String conversationId;
            RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
            if (chatId != null) {
                conversationId = rmHistory.getCoversationID(chatId, 2);
            } else {
                conversationId = rmHistory.getCoversationID(newContact, 1);
            }
            if (!conversationId.isEmpty()) {
                setConversationID(conversationId);
            } else {
                String callId = getImsService().getImsModule().getSipManager().getSipStack().generateCallId();
                conversationId = ContributionIdGenerator.getContributionId(callId);
                setConversationID(conversationId);
                if (chatId != null)
                    rmHistory.UpdateCoversationID(chatId, conversationId, 1);
                else {
                    // Might be the 1st activity for the chat here, add system message
                    rmHistory.addChatSystemMessage(
                            ChatUtils.createTextMessage(newContact, "system", false),
                            Direction.OUTGOING.toInt());
                    rmHistory.UpdateCoversationID(newContact, conversationId, 1);
                }
            }
        }
    }
    
    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("FTS Initiate a file transfer session as originating");
            }
            /**
             * M: Modified to resolve the 403 error issue.@{
             */
            SipRequest invite = createSipInvite();
            
            if (featureTags != null) {
                ContactHeader contact = (ContactHeader)invite.getHeader(ContactHeader.NAME);

                for(Iterator<?> i = contact.getParameterNames(); i.hasNext();) {
                    // Extract parameter name & value 
                    String name = (String)i.next();
                    contact.removeParameter(name);
                }

                String acceptValue = "*";

                for (int i = 0; i < featureTags.length; i++) {
                    contact.setParameter(featureTags[i], null);
                    acceptValue += ";" + featureTags[i];
                }
                try {
                    Header acceptNew = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_ACCEPT_CONTACT, acceptValue);

                    invite.getStackMessage().setHeader(acceptNew);
                } catch (ParseException e) {
                    e.printStackTrace();
                }
            } else
            if (extraTags != null) {
                ContactHeader contact = (ContactHeader)invite.getHeader(ContactHeader.NAME);
                ExtensionHeader acceptHeader = (ExtensionHeader)invite.getHeader(SipUtils.HEADER_ACCEPT_CONTACT);

                String acceptValue = acceptHeader.getValue();

                for (String tag:extraTags) {
                    contact.setParameter(tag, null);
                    acceptValue += ";" + tag;
                }
                try {
                    Header acceptNew = SipUtils.HEADER_FACTORY.createHeader(
                        SipUtils.HEADER_ACCEPT_CONTACT, acceptValue);

                    invite.getStackMessage().setHeader(acceptNew);
                } catch (ParseException e) {
                    e.printStackTrace();
                }
            }

            sendInvite(invite);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Session initiation has failed", e);
            }

           // Unexpected error
            handleError(new FileSharingError(FileSharingError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }

        if (logger.isActivated()) {
            logger.debug("FTS End of thread");
        }
    }

    /**
     * Prepare media session
     * 
     * @throws Exception 
     */
    @Override
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

            if (msrpMgr == null) {
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
                localIpAddress = ChatUtils.formatIPAddress(localIpAddress);
                msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            }
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
     * Start media session
     * 
     * @throws Exception 
     */
    @Override
    public void startMediaSession() throws Exception {
        if (logger.isActivated()) {
            logger.info("FTS startMediaSession isRecv: " + this.isReceiveOnly() + " isPaused: " + this.isFileTransferPaused());
        }
        if (this.isReceiveOnly() && this.isFileTransferPaused()){
            if (logger.isActivated()) {
                logger.error("No need to startMediaSession ");
            }
            return;
        }
        // Open the MSRP session
        msrpMgr.openMsrpSession();
        if (RcsSettings.getInstance().supportOP01()) {
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
            String imdnString = null;
            String contentType = getContent().getEncoding();
            if(RcsSettings.getInstance().isSupportOP07()){
            	contentType = CpimMessage.MIME_TYPE;
                String imdnMsgId = ChatUtils.generateMessageId();
                String imdnFrom = ChatUtils.ANOMYNOUS_URI;
                String imdnTo = ChatUtils.ANOMYNOUS_URI;
                imdnString = ChatUtils.buildCpimMessageWithImdn(imdnFrom,imdnTo,imdnMsgId,getContent().getEncoding());
            }
            if (isFileTransferPaused() && bytesToSkip > 1) {
                stream.skip(bytesToSkip -1);
                if (logger.isActivated()) {
                    logger.error("startMediaSession resumed interrupted file byteskipped :" + bytesToSkip);
                }
                if(RcsSettings.getInstance().isSupportOP07()){
                    msrpMgr.setCPIM(imdnString);
                    //msrpMgr.setCpimType(true);
                }
                msrpMgr.sendChunks(stream, ChatUtils.generateMessageId(), getContent().getEncoding(), getContent().getSize()- bytesToSkip + 1);
            } else {
                if(RcsSettings.getInstance().isSupportOP07()){
                    msrpMgr.setCPIM(imdnString);
                   // msrpMgr.setCpimType(true);
                }
                msrpMgr.sendChunks(stream, ChatUtils.generateMessageId(), getContent().getEncoding(), getContent().getSize());
            }
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
     * Close media session
     */
    @Override
    public void closeMediaSession() {
        if (msrpMgr != null) {
            msrpMgr.closeSession();
        }
        if (logger.isActivated()) {
            logger.debug("FTS MSRP session has been closed");
        }
    }
    
    public void handleMsrpConnectionException(Exception e) {
        if (logger.isActivated()) {
            logger.error("handleMsrpConnectionException");
        }
        if (logger.isActivated()) {
            logger.error("handleMsrpConnectionException");
        }
        handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK_MULTI_MMS, "connectionrefused"));
    }

    /**
     * Handle error 
     * 
     * @param error Error
     */
    public void handleError(ImsServiceError error) {
        if (isSessionInterrupted()) {
            return;
        }

        // Error    
        if (logger.isActivated()) {
            logger.info("Session error: " + error.getErrorCode() + ", reason=" + error.getMessage());
        }
        
        terminateSession(TERMINATION_BY_MSRP_TIMEOUT);

        // Close media session
        closeMediaSession();

        // Remove the current session
        getImsService().removeSession(this);

        // Notify listeners
        for(int j=0; j < getListeners().size(); j++) {
            ((FileSharingSessionListener)getListeners().get(j)).handleTransferError(new FileSharingError(error));
        }
    }

    /**
     * Data transfer in progress
     * 
     * @param currentSize Current transfered size in bytes
     * @param totalSize Total size in bytes
     */
    @Override
    public void msrpTransferProgress(long currentSize, long totalSize) {
        for (int j=0; j < getListeners().size(); j++) {
            ((FileSharingSessionListener)getListeners().get(j)).handleTransferProgress(currentSize, totalSize);
        }

        if(pauseInfo != null) {
            pauseInfo.bytesTransferrred = currentSize;
            pauseInfo.hashSelector = hashselector;
        }
        if (logger.isActivated()) {
            logger.info("FTS msrpTransferProgress" +
                        " bytes transferred: " + currentSize +
                        " pauseResumeObject:" + pauseInfo +
                        " FT TransferId: " + getSessionID() +
                        " hasselector: " + hashselector);
        }
    }

    /**
     * Data has been transfered
     * 
     * @param msgId Message ID
     */
    @Override
    public void msrpDataTransfered(String msgId) {
        String contact = this.getRemoteContact();
        if (logger.isActivated()) {
            logger.info("FTS Data transfered contact: " + contact + " msgId: " + msgId);
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
    }

    /**
     * Data transfer in progress
     *
     * @param currentSize Current transfered size in bytes
     * @param totalSize Total size in bytes
     * @param data received data chunk
     */
    @Override
    public boolean msrpTransferProgress(long currentSize, long totalSize, byte[] data) {
        try {
            // Update content with received data
            getContent().writeData2File(data);

            if (pauseInfo != null) {
                if (logger.isActivated()) {
                    logger.info("FTS msrpTransferProgress pauseInfo : " + pauseInfo);
                }
                pauseInfo.bytesTransferrred += data.length; 
                pauseInfo.pausedStream = getContent().getOut();
            }
            if (logger.isActivated()) {
                logger.info("FTS msrpTransferProgress bytes transferred: " + currentSize);
            }

            // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
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
     * Data transfer has been received
     * 
     * @param msgId Message ID
     * @param data Received data
     * @param mimeType Data mime-type 
     */
    @Override
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
                if(RcsSettings.getInstance().supportOP01()) {
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
     * Data transfer has been aborted
     */
    @Override
    public void msrpTransferAborted() {
        if (logger.isActivated()) {
            logger.info("FTS Data transfer aborted");
        }
    }
    
    @Override
    public void handle403Forbidden(SipResponse resp) {
        if (logger.isActivated()) {
           logger.error("handle403 Forbidden() entry");
        }
        
        WarningHeader warn = (WarningHeader)resp.getHeader(WarningHeader.NAME);
        if (warn == null || !warn.getText().contains("File is received")) {
            super.handle403Forbidden(resp);
        } else {
            /*
             * Walk-around method to deal with the gap between local & server
             * in which case, file actually sent successfully
             */
            // File has been transfered
            fileTransfered();

            // Remove the current session
            getImsService().removeSession(this);

            // Notify listeners
            for (int i=0; i < getListeners().size(); i++) {
                FileSharingSessionListener listener = (FileSharingSessionListener)getListeners().get(i); 

                listener.handleFileTransfered(getContent().getUrl(),getContent());
            }
        }
   }

    public void setFeatureTags(String[] featureTags) {
        this.featureTags = featureTags;
    }

    public void setExtraTags(List<String> extraTags) {
        this.extraTags = extraTags;
    }

    public void setPreferService(String preferService) {
        this.preferService = preferService;
    }

    public void setExtraContent(String content, String contentType) {
        this.extraContent = content;
        this.extraContentType = contentType;
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
        String ipAddress = getDialogPath().getSipStack().getLocalIpAddress();
        ipAddress = ChatUtils.formatIPAddress(ipAddress);
        String encoding = getContent().getEncoding();
        String wrappedType = null;
        if(this.isUseLargeMode()){
            wrappedType = "*";
        }
        String sdp = null;
        String sendMode = "a=sendonly";
        String msrpCemaParameter = null;
        if(RcsSettings.getInstance().isSupportOP07()){
            msrpCemaParameter = "a=msrp-cema" + SipUtils.CRLF;
        }
        String filetransferID = getSessionID();//getFileTransferId();

        if (isReceiveOnly()) {
            sendMode = "a=recvonly";
            filetransferID = pauseInfo.mOldFileTransferId;
        } else {
            sendMode = "a=sendonly";
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
            "a=accept-types:*"  + SipUtils.CRLF;
        if (isGeoLocFile())
            sdp +=
            "a=accept-wrapped-types:" +encoding + SipUtils.CRLF;
        if(isUseLargeMode())
            sdp +=
            "a=accept-wrapped-types:*" + SipUtils.CRLF;
        if(!isUseLargeMode()){
            sdp +=
                "a=file-transfer-id:" + filetransferID + SipUtils.CRLF;
        }
        if (timeLen > 0)
            sdp +=
            "a=file-disposition:timelen=" + timeLen + SipUtils.CRLF;
        
        sdp +=
            sendMode + SipUtils.CRLF;
        
        if(msrpCemaParameter != null){
            sdp += msrpCemaParameter;
        }
 
        int maxSize = FileSharingSession.getMaxFileSharingSize();
        if (maxSize > 0) {
            sdp += "a=max-size:" + maxSize + SipUtils.CRLF;
        }

        long totalFileSize = 0;
        if (isReceiveOnly()) {
            // Receive only happens only for download resume case
            totalFileSize = pauseInfo.mSize;
            getContent().setOut(pauseInfo.pausedStream);
        } else {
            totalFileSize = getContent().getSize();
        }

        // Set File-selector attribute
        if (isFileTransferPaused()) {
            hashselector = pauseInfo.hashSelector;
        } else {
            hashselector = getHashSelectorAttribute();
            pauseInfo.hashSelector = hashselector;
        }

        if (logger.isActivated()) {
            logger.debug("FTS hashselector : " + hashselector);
        }

        String selector = "name:\"" + getContent().getName() + "\"" + 
                         " type:" + getContent().getEncoding() +
                         " size:" + totalFileSize +
                         " " + hashselector;
        if(isUseLargeMode()){
            selector = " size:" + totalFileSize;
        }

        sdp += "a=file-selector:" + selector + SipUtils.CRLF;

        if (isFileTransferPaused()) {
            if (logger.isActivated()) {
                logger.info("File Resumed true while INVITE");
            }               
            if (logger.isActivated()) {
                logger.info("bytes transferred: " + pauseInfo.bytesTransferrred + "Old TransferId: " + oldFileTransferId);
            }
            if(RcsSettings.getInstance().isSupportOP07()) {
                pauseInfo.bytesTransferrred = 0;
            }
            String fileRange = "a=file-range:" + (pauseInfo.bytesTransferrred + 1) + "-" + totalFileSize; 
            if (isReceiveOnly && pauseInfo.pausedStream == null)
                fileRange = "a=file-range:" + 1 + "-" + totalFileSize; 
            sdp += fileRange + SipUtils.CRLF;
        }

        // Set File-location attribute
        String location = getFileLocationAttribute();
        if (location != null) {
            sdp += "a=file-location:" + location + SipUtils.CRLF;
        }

        boolean iconPart = false;
        boolean imdnPart = false;
        boolean rlistPart = false;

        String encodedImage = null;/*
        if (getThumbnail() != null) {
            encodedImage = null;//Base64.encodeBase64ToString(getThumbnail());
            if (encodedImage.length() > 0) {
                iconPart = false;//true;
                //sdp += "a=file-icon:cid:image@joyn.com" + SipUtils.CRLF;                
            }
        }*/
        if (isImdnSupport() && !isUseLargeMode()) {
            imdnPart = true;
        }
        if (participants.getList() != null && participants.getList().size() > 1) {
            rlistPart = true;
        }

        boolean multipart = iconPart || imdnPart || rlistPart || (extraContent != null);

        if (multipart) {
            String localContent = "";
            if (extraContent != null) {
                localContent += 
                        Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                        ContentTypeHeader.NAME + ": " + extraContentType + SipUtils.CRLF +
                        ContentLengthHeader.NAME + ": " + extraContent.getBytes().length + SipUtils.CRLF +
                        SipUtils.CRLF +
                        extraContent + SipUtils.CRLF;
            }

            localContent +=
                    Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                    ContentTypeHeader.NAME + ": application/sdp" + SipUtils.CRLF +
                    ContentLengthHeader.NAME + ": " + sdp.getBytes().length + SipUtils.CRLF +
                    SipUtils.CRLF +
                    sdp + SipUtils.CRLF;

            if (iconPart) {
                localContent +=
                        Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                        ContentTypeHeader.NAME + ": " + "image/jpeg" + SipUtils.CRLF +
                        SipUtils.HEADER_CONTENT_TRANSFER_ENCODING + ": base64" + SipUtils.CRLF +
                        SipUtils.HEADER_CONTENT_ID + ": <image@joyn.com>" + SipUtils.CRLF +
                        ContentLengthHeader.NAME + ": "+ encodedImage.length() + SipUtils.CRLF +
                        ContentDispositionHeader.NAME + ": icon" + SipUtils.CRLF +
                        SipUtils.CRLF +
                        encodedImage + SipUtils.CRLF;
            }
            if (logger.isActivated()) {
                logger.debug("Originatingextendfilesharingsession: " + imdnPart);
            }
            if (imdnPart) {
                String from = ImsModule.IMS_USER_PROFILE.getPublicAddress();
                String to = getRemoteContact();
                String msgId = ChatUtils.generateMessageId();
                if(isFileTransferPaused()){
                    msgId = RichMessagingHistory.getInstance().getFileMessageId(filetransferID);
                }
                final String updatedMsgId = msgId;
                if (logger.isActivated()) {
                    logger.debug("Originatingextendfilesharingsession msgId: " + msgId);
                }
                final boolean isPausedFile = this.fileTransferPaused;
                Thread t = new Thread() {
                    public void run() {
                        if(!isPausedFile){
                            RichMessagingHistory.getInstance().updateFileTransferMsgId(getSessionID(), updatedMsgId);
                        }
                    }
                };
                t.start();
                String imdnData = buildCpimMessageWithImdn(from, to, msgId);
                if (logger.isActivated()) {
                    logger.info("FTS createSipInvite IMDN data: " + imdnData);
                }
                imdnData  = imdnData + ContentTypeHeader.NAME + ": text/plain" + SipUtils.CRLF +
                        ContentLengthHeader.NAME + ": " + "0" + SipUtils.CRLF;
                int length = imdnData.length();
                localContent +=
                        Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                        ContentTypeHeader.NAME + ": message/cpim" + SipUtils.CRLF +
                        ContentLengthHeader.NAME + ": " + length + SipUtils.CRLF +
                        SipUtils.CRLF +
                        imdnData +
                        SipUtils.CRLF;
            }
            if (rlistPart) {
                String resourceList = null;
                boolean isBccOff = RcsSettings.getInstance().isGroupMessagingActivated();
                if (logger.isActivated()) {
                    logger.error("isBccOff value : " + isBccOff);
                 }
                if(isBccOff){
                    resourceList = ChatUtils.generateMultiChatResourceList(
                            getParticipants().getList());
                } else {
                    resourceList = ChatUtils.generateMultiChatBccResourceList(
                            getParticipants().getList());
                }

                localContent +=
                        Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                        "Content-Type: application/resource-lists+xml" + SipUtils.CRLF +
                        "Content-Length: " + resourceList.getBytes().length + SipUtils.CRLF +
                        "Content-Disposition: recipient-list" + SipUtils.CRLF +
                        SipUtils.CRLF +
                        resourceList + SipUtils.CRLF;
            }

            localContent += 
                    Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + Multipart.BOUNDARY_DELIMITER;
            getDialogPath().setLocalContent(localContent);

        } else {
            getDialogPath().setLocalContent(sdp);
        }

        try {
            SipRequest invite = createInvite(multipart);
            getAuthenticationAgent().setAuthorizationHeader(invite);
            getDialogPath().setInvite(invite);

            return invite;
        } catch (SipException e) {
            e.printStackTrace();
        } catch (CoreException e) {
            e.printStackTrace();
        }

        return null;
    }

    /**
     * Create an INVITE request
     *
     * @return the INVITE request
     * @throws SipException 
     */
    public SipRequest createInvite(boolean multiPart) throws SipException {
        SipRequest invite;

        if (RcsSettings.getInstance().isCPMSupported()){
            if (logger.isActivated()) {
                logger.debug("create Invite islarge " + isUseLargeMode());
            }
            if(isUseLargeMode()){
                invite = SipMessageFactory.createFileCpmInvite(
                        getDialogPath(),
                        InstantMessagingService.CPM_LARGE_MESSAGE_FEATURE_TAGS,
                        preferService,
                        getDialogPath().getLocalContent(),
                        multiPart,
                        BOUNDARY_TAG);
            } else {
                invite = SipMessageFactory.createFileCpmInvite(
                        getDialogPath(),
                        InstantMessagingService.CPM_FT_FEATURE_TAGS,
                        preferService,
                        getDialogPath().getLocalContent(),
                        multiPart,
                        BOUNDARY_TAG);
            }
            
        }
        else{
            if (multiPart) {
                invite = SipMessageFactory.createMultipartInvite(
                    getDialogPath(),
                    InstantMessagingService.FT_FEATURE_TAGS,
                    getDialogPath().getLocalContent(),
                    BOUNDARY_TAG);
            } else {
                invite = SipMessageFactory.createInvite(
                    getDialogPath(),
                    InstantMessagingService.FT_FEATURE_TAGS,
                    getDialogPath().getLocalContent());
            }
        }
        
        // Add a contribution ID header
        invite.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID());
        if (RcsSettings.getInstance().isCPMSupported()) {
            if (getConversationID() != null) {
                invite.addHeader(ChatUtils.HEADER_CONVERSATION_ID, getConversationID());
            }
        }

        return invite;
    }


    /**
     * Send an empty data chunk
     */
    private void sendEmptyDataChunk() {
        try {
            msrpMgr.sendEmptyChunk();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Problem while sending empty data chunk", e);
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
}
