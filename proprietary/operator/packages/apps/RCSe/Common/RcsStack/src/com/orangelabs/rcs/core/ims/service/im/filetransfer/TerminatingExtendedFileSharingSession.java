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

import android.media.MediaFile;
import android.webkit.MimeTypeMap;

import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.PauseResumeFileObject;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.utils.SettingUtils;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.content.ContentManager;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpConstants;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpEventListener;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpManager;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpSession;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaAttribute;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.SessionTimerManager;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.content.MmContent;

import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManager;
import com.orangelabs.rcs.core.ims.security.cert.KeyStoreManagerException;
import java.security.KeyStoreException;


import java.io.BufferedOutputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.InputStream;

import javax2.sip.header.ContactHeader;


import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Terminating file transfer session
 *
 * @author jexa7410
 */
public class TerminatingExtendedFileSharingSession extends ImsFileSharingSession implements
        MsrpEventListener {
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

    private String messgeId;

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
    public TerminatingExtendedFileSharingSession(ImsService parent, MmContent content, SipRequest invite) {
        super(parent, content, SipUtils
                .getAssertedIdentity(invite), ChatUtils.extractFileThumbnail(invite));

        messgeId = null;
        if (RcsSettings.getInstance().supportOP01() || RcsSettings.getInstance().isSupportOP08()) {
            if (ChatUtils.isImdnDeliveredRequested(invite)) {
                messgeId = ChatUtils.getMessageId(invite);
                this.setMessageId(messgeId);
                ContactHeader inviteContactHeader = (ContactHeader) invite
                        .getHeader(ContactHeader.NAME);
                if (inviteContactHeader != null) {
                    this.remoteInstanceId = inviteContactHeader
                            .getParameter(SipUtils.SIP_INSTANCE_PARAM);
                }
                if (logger.isActivated()) {
                    logger.info("TerminatingExtendedFileSharingSession" + messgeId
                            + ";Instance:" + remoteInstanceId);
                }
            }
        }

        // Create dialog path
        createTerminatingDialogPath(invite);          
        // Set contribution ID
        String id = ChatUtils.getContributionId(invite);
        setContributionID(id);
        if (RcsSettings.getInstance().isCPMSupported()) {
            if (logger.isActivated()) {
                logger.info("EFTS terminatingExtendedFileSharingSession  CPMS");
            }
            setConversationID(ChatUtils.getCoversationId(invite));
            setInReplyID(ChatUtils.getInReplyId(invite));
        }
    }

    /**
     * Background processing
     */
    public void run() {
        try {
            if (logger.isActivated()) {
                logger.info("EFTS Initiate a new extended file transfer session, isSendOnly" + isSendOnly);
            }

            String remoteSdp = getDialogPath().getInvite().getSdpContent();
            SdpParser parser = new SdpParser(remoteSdp.getBytes());
            Vector<MediaDescription> media = parser.getMediaDescriptions();
            MediaDescription mediaDesc = media.elementAt(0);
            String protocol = mediaDesc.protocol;
            boolean isSecured = false;
            if (protocol != null) {
                isSecured = protocol
                        .equalsIgnoreCase(MsrpConstants.SOCKET_MSRP_SECURED_PROTOCOL);
            }
            MediaAttribute attr1 = mediaDesc.getMediaAttribute("file-selector");
            String fileSelector = attr1.getName() + ":" + attr1.getValue();
            MediaAttribute attr2 = mediaDesc.getMediaAttribute("file-transfer-id");
            String fileTransferId = attr2.getName() + ":" + attr2.getValue();
            try {
                hashselector = attr1.getValue().substring(
                        attr1.getValue().indexOf("hash:sha-1:"));
            } catch (Exception e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }

            if (pauseInfo != null) {
                if (logger.isActivated()) {
                    logger.debug("FTS receive Invite save hashselctor " + hashselector);
                }
                pauseInfo.hashSelector = hashselector;
                pauseInfo.mOldFileTransferId = attr2.getValue();
            }
            
            if(this instanceof FileSharingSession){
                ((FileSharingSession)this).extractFileRange(remoteSdp);
            }

            /** M: isSendOnly Resume file As A Sender @{ */
            if (isSendOnly) {
                if (logger.isActivated()) {
                    logger.debug("EFTS isSendOnly Resume file As A Sender");
                }
                /** @}*/
            } 
            // Parse the remote SDP part
            MediaAttribute attr3 = mediaDesc.getMediaAttribute("path");
            String remotePath = attr3.getValue();
            String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription,
                    mediaDesc);
            int remotePort = mediaDesc.port;

            // Extract the "setup" parameter
            String remoteSetup = "passive";
            MediaAttribute attr4 = mediaDesc.getMediaAttribute("setup");
            if (attr4 != null) {
                remoteSetup = attr4.getValue();
            }
            if (logger.isActivated()) {
                logger.debug("EFTS Remote setup attribute is " + remoteSetup);
            }

            // Set setup mode
            String localSetup = createSetupAnswer(remoteSetup);
            if (logger.isActivated()) {
                logger.debug("EFTS Local setup attribute is " + localSetup);
            }

            // Set local port
            int localMsrpPort;
            if (localSetup.equals("active") && !(RcsSettings.getInstance().isSupportOP08()  || RcsSettings.getInstance().isSupportOP07())) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }
               if (logger.isActivated()){
                    logger.debug("EFTS Local setup localMsrpPort is " + localMsrpPort);
                }

            // Create the MSRP manager
            String localIpAddress  = getImsService().getImsModule().getCurrentNetworkInterface().getNetworkAccess().getIpAddress();
            localIpAddress = ChatUtils.formatIPAddress(localIpAddress);  
            msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            msrpMgr.setSecured(isSecured);

            // Build SDP part
            String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
            String ipAddress = ChatUtils.formatIPAddress(getDialogPath().getSipStack().getLocalIpAddress());
            logger.debug(" getDialogPath().getSipStack().getLocalIpAddress(); after format: " +  ipAddress);
            
            String sdp = null;
            String sendMode = "a=recvonly";
            // Query Db whether FT is incoming/outgoing //Deepak
            if (logger.isActivated()) {
                logger.debug("EFTS terminating file session isSendonly: " + isSendOnly()
                        + " isPaused" + isFileTransferPaused());
            }
            if (logger.isActivated()) {
                logger.debug("EFTS terminating file session isSecureProtocolMessage: "
                        + isSecureProtocolMessage());
            }
            if (isFileTransferPaused() && isSendOnly()) {
                sendMode = "a=sendonly";
            } else {
                sendMode = "a=recvonly";
            }
            if (isSecureProtocolMessage()) {
                sdp = "v=0" + SipUtils.CRLF + "o=- " + ntpTime + " " + ntpTime + " "
                        + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF + "s=-"
                        + SipUtils.CRLF + "c=" + SdpUtils.formatAddressType(ipAddress)
                        + SipUtils.CRLF + "t=0 0" + SipUtils.CRLF + "m=message "
                        + localMsrpPort + " " + msrpMgr.getLocalSocketProtocol() + " *"
                        + SipUtils.CRLF + "a=" + fileSelector + SipUtils.CRLF + "a="
                        + fileTransferId + SipUtils.CRLF + "a=accept-types:"
                        + getContent().getEncoding()
                        + SipUtils.CRLF
                        + "a=file-disposition:attachment"
                        + SipUtils.CRLF
                        + // @tct-stack-[IOT-5_5_8] added by fang.wu
                        "a=setup:" + localSetup + SipUtils.CRLF + "a=path:"
                        + msrpMgr.getLocalMsrpPath() + SipUtils.CRLF + "a=fingerprint:"
                        + KeyStoreManager.getFingerPrint() + SipUtils.CRLF + sendMode
                        + SipUtils.CRLF;
            } else {
                sdp = "v=0" + SipUtils.CRLF + "o=- " + ntpTime + " " + ntpTime + " "
                        + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF + "s=-"
                        + SipUtils.CRLF + "c=" + SdpUtils.formatAddressType(ipAddress)
                        + SipUtils.CRLF + "t=0 0" + SipUtils.CRLF + "m=message "
                        + localMsrpPort + " " + msrpMgr.getLocalSocketProtocol() + " *"
                        + SipUtils.CRLF + "a=" + fileSelector + SipUtils.CRLF + "a="
                        + fileTransferId + SipUtils.CRLF + "a=accept-types:"
                        + getContent().getEncoding() + SipUtils.CRLF
                        + "a=file-disposition:attachment"
                        + SipUtils.CRLF
                        + // @tct-stack-[IOT-5_5_8] added by fang.wu
                        "a=setup:" + localSetup + SipUtils.CRLF + "a=path:"
                        + msrpMgr.getLocalMsrpPath() + SipUtils.CRLF + sendMode
                        + SipUtils.CRLF;
            }
            int maxSize = ImsFileSharingSession.getMaxFileSharingSize();
            if (maxSize > 0) {
                sdp += "a=max-size:" + maxSize + SipUtils.CRLF;
            }

            if (pauseInfo != null) {
                if (logger.isActivated()) {
                    logger.debug("EFTS terminating extended file session pause object is null");
                }
                long bytesTransferrred = pauseInfo.bytesTransferrred;
                if (logger.isActivated()) {
                    logger.info("EFTS terminating extended file bytes transferred: " + bytesTransferrred
                            + "Old TransferId: " + oldFileTransferId);
                }
            }
            String fileRange = "a=file-range:" + (bytesToSkip) + "-" + pauseInfo.mSize;         
            sdp += fileRange + SipUtils.CRLF;

            // Set the local SDP part in the dialog path
            getDialogPath().setLocalContent(sdp);

            // Test if the session should be interrupted
            if (isInterrupted()) {
                if (logger.isActivated()) {
                    logger.debug("FTS Session has been interrupted: end of processing");
                }
                return;
            }

        /*    // Create the MSRP server session
            if (localSetup.equals("passive")) {
                // Passive mode: client wait a connection
                msrpMgr.createMsrpServerSession(remotePath, this);

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
                                logger.error("FTS Can't create the MSRP server session",
                                        e);
                            }
                        }
                    }
                };
                thread.start();
            }*/

            // Create a 200 OK response
            if (logger.isActivated()) {
                logger.info("EFTS Send 200 OK");
            }
            SipResponse resp = null;
            if (!RcsSettings.getInstance().isCPMSupported()) {
                resp = SipMessageFactory.create200OkInviteResponse(getDialogPath(),
                        InstantMessagingService.CHAT_FEATURE_TAGS, sdp);
            } else {
                if (logger.isActivated()) {
                    logger.info("terminatingExtendedFileSharingSession0  EFTS");
                }
                resp = SipMessageFactory.createCpm200OkInviteResponse(getDialogPath(),
                        InstantMessagingService.CPM_CHAT_FEATURE_TAGS, sdp);
            }

            if (RcsSettings.getInstance().isCPMSupported()) {
                if (logger.isActivated()) {
                    logger.info("terminatingExtendedFileSharingSession1  EFTS");
                }
                if (getContributionID() != null) {
                    resp.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID());
                }
                if (getConversationID() != null) {
                    resp.addHeader(ChatUtils.HEADER_CONVERSATION_ID, getConversationID());
                }
                if (getInReplyID() != null) {
                    resp.addHeader(ChatUtils.HEADER_INREPLY_TO_CONTRIBUTION_ID,
                            getInReplyID());
                }
            }

            // The signalisation is established
            getDialogPath().sigEstablished();

            // Send response
            SipTransactionContext ctx = getImsService().getImsModule().getSipManager()
                    .sendSipMessageAndWait(resp);

            // Analyze the received response
            if (ctx.isSipAck()) {
                // ACK received
                if (logger.isActivated()) {
                    logger.info("EFTS Extended ACK request received");
                }

                // Notify listeners
                for (int j = 0; j < getListeners().size(); j++) {
                    //getListeners().get(j).handleSessionStarted();
                }

                /*// Create the MSRP client session
                if (localSetup.equals("active")) {
                    // Active mode: client should connect
                    msrpMgr.createMsrpClientSession(remoteHost, remotePort, remotePath,
                            this);

                    // Open the MSRP session
                    msrpMgr.openMsrpSession(ImsFileSharingSession.DEFAULT_SO_TIMEOUT);

                    // Send an empty packet
                    sendEmptyDataChunk();
                }*/

                // The session is established
                getDialogPath().sessionEstablished();
                
             // Prepare Media Session
                prepareMediaSession();                           

                // Start Media Session
                startMediaSession();

                // Notify listeners
                for(int i=0; i < getListeners().size(); i++) {
                    getListeners().get(i).handleSessionStarted();
                }

                if (logger.isActivated()) {
                    logger.info("After sesion start");
                }               

                // Start session timer
                if (getSessionTimerManager().isSessionTimerActivated(resp)) {
                    getSessionTimerManager().start(SessionTimerManager.UAS_ROLE,
                            getDialogPath().getSessionExpireTime());
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("EFTS No ACK received for INVITE");
                }

                closeMediaSession();
                // Remove the current session
                getImsService().removeSession(this);
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("EFTS Session initiation has failed", e);
            }

            closeMediaSession();
            // Remove the current session
            getImsService().removeSession(this);
        }

        if (logger.isActivated()) {
            logger.debug("EFTS End of thread");
        }
    }

    /**
     * Send an empty data chunk
     */
    public void sendEmptyDataChunk() {
        try {
            msrpMgr.sendEmptyChunk();
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("EFTS Problem while sending empty data chunk", e);
            }
        }
    }

    /**
     * Send delivery report
     *
     * @param status Report status
     */
    private void sendDeliveryReport(String status) {
        //Not used
    }

    /**
     * Data has been transfered
     *
     * @param msgId Message ID
     */
    public void msrpDataTransfered(String msgId) {
        String contact = this.getRemoteContact();
        if (logger.isActivated()) {
            logger.info("EFTS Data transfered contact: " + contact + " msgId: " + msgId);
        }

        // File has been transfered
        fileTransfered();

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
            logger.info("EFTS Data received");
        }
    }

    /**
     * Data transfer in progress
     *
     * @param currentSize Current transfered size in bytes
     * @param totalSize Total size in bytes
     */
    public void msrpTransferProgress(long currentSize, long totalSize) {
        if(pauseInfo != null) {
            pauseInfo.bytesTransferrred = currentSize;
            pauseInfo.hashSelector = hashselector;
        }
        if (logger.isActivated()) {
            logger.info("EFTS msrpTransferProgress" +
                        " bytes transferred: " + currentSize +
                        " pauseResumeObject:" + pauseInfo +
                        " FT TransferId: " + getSessionID() +
                        " hasselector: " + hashselector);
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
        if (logger.isActivated()) {
            logger.info("EFTS Extended msrpTransferProgress as receiver not needed");
        }
        return true;
    }

    /**
     * Data transfer has been aborted
     */
    public void msrpTransferAborted() {
        if (logger.isActivated()) {
            logger.info("EFTS Data transfer aborted");
        }       
    }

    /**
     * Prepare media session
     *
     * @throws Exception
     */
    public void prepareMediaSession() throws Exception {
        // Nothing to do in terminating side
        if (logger.isActivated()) {
            logger.info("EFTS Extended prepareMediaSession isSendOnly: " + this.isSendOnly()
                    + " isPaused: " + this.isFileTransferPaused());
        }
        if (this.isSendOnly() && this.isFileTransferPaused()) {
            if (logger.isActivated()) {
                logger.error(" prepareresumeMediaSession ");
            }
            prepareResumeMediaSession();
        }
    }

    /**
     * Start media session
     *
     * @throws Exception
     */
    public void startMediaSession() throws Exception {
        // Nothing to do in terminating side
        if (logger.isActivated()) {
            logger.info("EFTS Extended startMediaSession isSendOnly: " + this.isSendOnly()
                    + " isPaused: " + this.isFileTransferPaused());
        }
        if (this.isSendOnly() && this.isFileTransferPaused()) {
            if (logger.isActivated()) {
                logger.error(" startresumeMediaSession ");
            }
            startResumeMediaSession();
        }
    }

    /**
     * Prepare media session
     *
     * @throws Exception
     */
    public void prepareResumeMediaSession() throws Exception {
        // Parse the remote SDP part
        if (logger.isActivated()) {
            logger.info("EFTS prepareResumeMediaSession ");
        }        
        SdpParser parser = new SdpParser(getDialogPath().getRemoteContent().getBytes());
        Vector<MediaDescription> media = parser.getMediaDescriptions();
        MediaDescription mediaDesc = media.elementAt(0);
        MediaAttribute attr = mediaDesc.getMediaAttribute("path");
        String remoteMsrpPath = attr.getValue();
        String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription,
                mediaDesc);
        int remotePort = mediaDesc.port;

        // Create the MSRP client session
        MsrpSession session = msrpMgr.createMsrpClientSession(remoteHost, remotePort,
                remoteMsrpPath, this);
        session.setFailureReportOption(true);
        session.setSuccessReportOption(false);
    }

    /**
     * Start media session
     *
     * @throws Exception
     */
    public void startResumeMediaSession() throws Exception {
        // Open the MSRP session
        if (logger.isActivated()) {
            logger.info("EFTS TerminatingExtended startResumeMediaSession ");
        }
        msrpMgr.openMsrpSession();

        try {
            // Start sending data chunks
            byte[] data = getContent().getData();
            InputStream stream;
            if (data == null) {
                // Load data from URL
                stream = FileFactory.getFactory().openFileInputStream(
                        getContent().getUrl());
            } else {
                // Load data from memory
                stream = new ByteArrayInputStream(data);
            }
            long bytesTransferredtoSkip = 0;
            if (isFileTransferPaused()) {
                bytesTransferredtoSkip = bytesToSkip;
                if(bytesToSkip > 1)
                    stream.skip(bytesToSkip -1);
                if (logger.isActivated()) {
                    logger.error("startMediaSession resumed interrupted file byteskipped :" + bytesToSkip);
                }                
                if (logger.isActivated()) {
                    logger.error("startMediaSession resumed interrupted file byteskipped :"
                            + bytesTransferredtoSkip);
                }
            }
            msrpMgr.sendChunks(stream, ChatUtils.generateMessageId(), getContent()
                    .getEncoding(), getContent().getSize() - bytesTransferredtoSkip + 1);
        } catch (Exception e) {
            // Unexpected error
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }
            closeMediaSession();
            // Remove the current session
            getImsService().removeSession(this);
        }
    }

    /**
     * Close media session
     */
    public void closeMediaSession() {
        // Close MSRP session
        if (msrpMgr != null) {
            msrpMgr.closeSession();
            if (logger.isActivated()) {
                logger.debug("EFTS MSRP session has been closed");
            }
        }
        if (!isFileTransfered()) {
            // Delete the temp file
            // deleteFile();
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
            logger.debug("EFTS Delete incomplete received file Paused = "
                    + fileTransferPaused);
        }
        try {
            if (!fileTransferPaused)
                getContent().deleteFile();
        } catch (IOException e) {
            if (logger.isActivated()) {
                logger.error("EFTS Can't delete received file", e);
            }
        }
    }
}
