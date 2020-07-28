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
import com.gsma.services.rcs.ft.FileTransfer;

import com.orangelabs.rcs.service.api.FileTransferServiceImpl;
import com.orangelabs.rcs.service.api.PauseResumeFileObject;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.utils.PhoneUtils;
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
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimParser;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
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
public class TerminatingFileSharingSession extends ImsFileSharingSession implements
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

    private volatile String messgeId;

    /**
     * Remote instance Id
     */
    private String remoteInstanceId = null;
    
    private boolean isDisplayRequested = false;

    public boolean isDisplayRequested() {
        return isDisplayRequested;
    }

    public void setDisplayRequested(boolean isDisplayRequested) {
        this.isDisplayRequested = isDisplayRequested;
    }

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
    public TerminatingFileSharingSession(ImsService parent, SipRequest invite) {
        super(parent, ContentManager.createLargeMmContentFromSdp(invite), SipUtils
                .getAssertedIdentity(invite), ChatUtils.extractFileThumbnail(invite));

        messgeId = null;
        logger.debug("Parse message id ");
        
        try {
            if (isImdnSupport()) {
                logger.debug("Parse message id1 ");
                if (ChatUtils.isImdnDeliveredRequested(invite)) {
                    messgeId = ChatUtils.getMessageId(invite);
                    boolean isDisplayRequested = ChatUtils.isImdnDisplayedRequested(invite);
                    setDisplayRequested(isDisplayRequested);
                    this.setMessageId(messgeId);
                    ContactHeader inviteContactHeader = (ContactHeader) invite
                            .getHeader(ContactHeader.NAME);
                    if (inviteContactHeader != null) {
                        this.remoteInstanceId = inviteContactHeader
                                .getParameter(SipUtils.SIP_INSTANCE_PARAM);
                    }
                    if (logger.isActivated()) {
                        logger.info("TerminatingFileSharingSession1: " + messgeId
                                + ";Instance:" + remoteInstanceId + ",isDisplayRequested: " + isDisplayRequested);
                    }
                }
                boolean isLargeMode = SipUtils.isFeatureTagPresent(invite, FeatureTags.FEATURE_CPM_LARGE_MSG);
                if (logger.isActivated()) {
                    logger.debug("isLargeMode status: " + isLargeMode);
                }
                setUseLargeMode(isLargeMode);
            }
        } catch (Exception e1) {
            if (logger.isActivated()) {
                logger.debug("Exception in parsing mesageId");
            }
            if (logger.isActivated()) {
                logger.info("TerminatingFileSharingSession1: " + messgeId
                        + ";Instance:" + remoteInstanceId + ",isDisplayRequested: " + isDisplayRequested);
            }
            e1.printStackTrace();
        }

        // Create dialog path
        createTerminatingDialogPath(invite);
        String name = "thumb_" + IdGenerator.getIdentifier() + ".jpeg";
        String mimeType = MediaFile.getMimeTypeForFile(name);
        if (logger.isActivated()) {
            logger.debug("FTS Thumb Extraction, mimetype and name are : " + mimeType
                    + ":" + name);
        }
        if (mimeType != null && mimeType.contains("video")) {
            if (mimeType.contains("3gpp"))
                name = name.replace(".3gp", ".jpg");
            else if (mimeType.contains("mpeg-4") || mimeType.contains("mp4"))
                name = name.replace(".mp4", ".jpg");
            if (logger.isActivated()) {
                logger.debug("FTS Video thumb change mimetype name : " + name);
            }
            if (name.contains(".mp4")) {
                name = name.replace(".mp4", ".jpg");
            }
            if (logger.isActivated()) {
                logger.debug("FTS Video thumb1 change mimetype name : " + name);
            }
        }

        // Init file
        thumbFile = new File(RcsSettings.getInstance().getFileRootDirectory(), name);
        try {
            thumbStreamForFile = new BufferedOutputStream(new FileOutputStream(thumbFile));
        } catch (FileNotFoundException e) {
            if (logger.isActivated()) {
                logger.error("FTS Could not create stream, file does not exists.");
            }
        }
        try {
            if (thumbStreamForFile != null) {
                logger.debug("write thumnail ");
                if (logger.isActivated()) {
                    logger.info("TerminatingFileSharingSession123:" + getThumbnail());
                    logger.info("TerminatingFileSharingSession123:"
                            + getThumbnail().length);
                }
                thumbStreamForFile.write(getThumbnail(), 0, getThumbnail().length);
                thumbStreamForFile.flush();
                thumbStreamForFile.close();
            } else {
                logger.debug("FTS Thumbnail file is null ");
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Download file exception", e);
            }
        }
        setThumbUrl(RcsSettings.getInstance().getFileRootDirectory() + name);
        File file = new File(getThumbUrl());
        long length = 0;
        if (file != null) {
            length = file.length();
        }
        if (logger.isActivated()) {
            logger.error("FTS lemghth of thumnail file is" + length + "URL is"
                    + getThumbUrl());
        }

        /*
         * BURN MESSAGE
         */
        boolean isBurnMsg = false;
        isBurnMsg = SipUtils.isFeatureTagPresent(invite,
                FeatureTags.FEATURE_CPM_BURNED_MSG);
        setBurnMessage(isBurnMsg);

        /** M: ftAutAccept @{ */
        boolean autoAccept = RcsSettings.getInstance().isFileTransferAutoAccepted();
        if (!autoAccept) {
            logger.debug("FTS isFileTransferAutoAccepted: false! ");
            return;
        }
        boolean lessWarnSize = (getContent().getSize() < (RcsSettings.getInstance()
                .getWarningMaxFileTransferSize() * 1024));
        if (!lessWarnSize) {
            logger.debug("FTS lessWarnSize: false! ");
            return;
        }
        boolean isRoaming = SettingUtils.isRoaming(AndroidFactory.getApplicationContext());
        // whether ftAutAccept is enabled if roaming.
        if (isRoaming) {
            mAutoAccept = RcsSettings.getInstance().isEnableFtAutoAcceptWhenRoaming();
        } else {
            mAutoAccept = RcsSettings.getInstance().isEnableFtAutoAcceptWhenNoRoaming();
        }

        if (logger.isActivated()) {
            logger.debug("FTS autoAccept: " + autoAccept + " lessWarnSize: "
                    + lessWarnSize + " enable: " + isRoaming + "mAutoAccept"
                    + mAutoAccept + " , isburnmessage : " + isBurnMsg);
        }

        /** @}*/

        // Set contribution ID
        String id = ChatUtils.getContributionId(invite);
        setContributionID(id);
        if (RcsSettings.getInstance().isCPMSupported()) {
            if (logger.isActivated()) {
                logger.info("FTS TerminatingFileSharingSession  CPMS");
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
                logger.info("FTS Initiate a new file transfer session as terminating");
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
            String fileTransferId = null;
            try {
                fileTransferId = attr2.getName() + ":" + attr2.getValue();
            } catch (Exception e2) {
                if (logger.isActivated()) {
                    logger.debug("Transferid is null");
                }
                e2.printStackTrace();
            }
            if (logger.isActivated()) {
                logger.debug("fileTransferId: " + fileTransferId);
            }
            try {
                hashselector = attr1.getValue().substring(
                        attr1.getValue().indexOf("hash:sha-1:"));
            } catch (Exception e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }

            try {
                if (pauseInfo != null) {
                    if (logger.isActivated()) {
                        logger.debug("FTS receive Invite save hashselctor " + hashselector);
                    }
                    pauseInfo.hashSelector = hashselector;
                    pauseInfo.mOldFileTransferId = attr2.getValue();
                }
            } catch (Exception e1) {
                if (logger.isActivated()) {
                    logger.debug("pauseInfo is not null" );
                }
                e1.printStackTrace();
            }

            /** M: ftAutAccept @{ */
            if (mAutoAccept || isSendOnly) {
                if (logger.isActivated()) {
                    logger.debug("FTS Auto accept file transfer invitation");
                }
                /** @}*/
            } else {
                if (logger.isActivated()) {
                    logger.debug("FTS Accept manually file transfer invitation");
                }

                // Send a 180 Ringing response
                send180Ringing(getDialogPath().getInvite(), getDialogPath().getLocalTag());

                try {
                    // Extract the "setup" parameter
                    String timeLen = null;
                    MediaAttribute attr5 = mediaDesc
                            .getMediaAttribute("file-disposition");
                    if (attr5 != null) {
                        timeLen = attr5.getValue();
                    }
                    if (logger.isActivated()) {
                        logger.debug("FTS timeLen attribute is " + timeLen);
                    }
                    int fileLen = 0;
                    if (timeLen != null) {
                        String[] timeStringArray = timeLen.split("=");
                        if (logger.isActivated()) {
                            logger.debug("FTS timeStringArray attribute is "
                                    + timeStringArray[0] + " new" + timeStringArray[1]);
                        }
                        String timeString = timeStringArray[1];
                        fileLen = Integer.parseInt(timeString);
                        if (logger.isActivated()) {
                            logger.debug("FTS timeString attribute is " + timeString
                                    + "filelen is " + fileLen);
                        }
                    }
                    setTimeLen(fileLen);
                } catch (Exception e) {
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
                    for (int i = 0; i < getListeners().size(); i++) {
                        getListeners().get(i).handleSessionAborted(
                                ImsServiceSession.TERMINATION_BY_USER);
                    }
                    return;
                } else if (answer == ImsServiceSession.INVITATION_NOT_ANSWERED) {
                    if (logger.isActivated()) {
                        logger.debug("FTS Session has been rejected on timeout");
                    }

                    // Ringing period timeout
                    send486Busy(getDialogPath().getInvite(), getDialogPath()
                            .getLocalTag());

                    // Remove the current session
                    getImsService().removeSession(this);

                    // Notify listeners
                    for (int j = 0; j < getListeners().size(); j++) {
                        getListeners().get(j).handleSessionAborted(
                                ImsServiceSession.TERMINATION_BY_TIMEOUT);
                    }
                    return;
                } else if (answer == ImsServiceSession.INVITATION_CANCELED) {
                    if (logger.isActivated()) {
                        logger.debug("FTS Session has been canceled");
                    }
                    return;
                }
            }

            // Reject if file is too big or size exceeds device storage
            // capacity. This control should be done
            // on UI. It is done after end user accepts invitation to enable
            // prior handling by the application.
            FileSharingError error = FileSharingSession
                    .isFileCapacityAcceptable(getContent().getSize());
            if (error != null) {
                // Send a 603 Decline response
                sendErrorResponse(getDialogPath().getInvite(), getDialogPath()
                        .getLocalTag(), 603);

                // Close session
                handleError(error);
                return;
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
                logger.debug("FTS Remote setup attribute is " + remoteSetup);
            }

            // Set setup mode
            String localSetup = createSetupAnswer(remoteSetup);
            if (logger.isActivated()) {
                logger.debug("FTS Local setup attribute is " + localSetup);
            }

            // Set local port
            int localMsrpPort;
            if (localSetup.equals("active") && !(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07())) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }
            if (logger.isActivated()){
                logger.debug("FTS Local setup localMsrpPort is " + localMsrpPort);
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
            String msrpCemaParameter = null;
            if(RcsSettings.getInstance().isSupportOP07()){
                msrpCemaParameter = "a=msrp-cema" + SipUtils.CRLF;
            }
            String sendMode = "a=recvonly";
            // Query Db whether FT is incoming/outgoing //Deepak
            if (logger.isActivated()) {
                logger.debug("FTS terminating file session isSendonly: " + isSendOnly()
                        + " isPaused" + isFileTransferPaused());
            }
            if (logger.isActivated()) {
                logger.debug("FTS terminating file session isSecureProtocolMessage: "
                        + isSecureProtocolMessage());
            }
            if (isFileTransferPaused() && isSendOnly()) {
                sendMode = "a=sendonly";
            } else {
                sendMode = "a=recvonly";
            }
            
            String fileTransferIdSdp =  null;
            if(fileTransferId != null) {
                fileTransferIdSdp = "a=" + fileTransferId + SipUtils.CRLF + "a=accept-types:"
                        + getContent().getEncoding()
                        + SipUtils.CRLF
                        + "a=file-disposition:attachment"
                        + SipUtils.CRLF;
            } else {
                fileTransferIdSdp = "a=accept-types:message/cpim"
                        + SipUtils.CRLF
                        + "a=accept-wrapped-types:*" 
                        + SipUtils.CRLF;
            }
            
            if (isSecureProtocolMessage()) {
                sdp = "v=0" + SipUtils.CRLF + "o=- " + ntpTime + " " + ntpTime + " "
                        + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF + "s=-"
                        + SipUtils.CRLF + "c=" + SdpUtils.formatAddressType(ipAddress)
                        + SipUtils.CRLF + "t=0 0" + SipUtils.CRLF + "m=message "
                        + localMsrpPort + " " + msrpMgr.getLocalSocketProtocol() + " *"
                        + SipUtils.CRLF + "a=" + fileSelector + SipUtils.CRLF
                        + fileTransferIdSdp
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
                        + SipUtils.CRLF + "a=" + fileSelector + SipUtils.CRLF 
                        + fileTransferIdSdp
                        + // @tct-stack-[IOT-5_5_8] added by fang.wu
                        "a=setup:" + localSetup + SipUtils.CRLF + "a=path:"
                        + msrpMgr.getLocalMsrpPath() + SipUtils.CRLF + sendMode
                        + SipUtils.CRLF;
            }
            int maxSize = ImsFileSharingSession.getMaxFileSharingSize();
            if (maxSize > 0) {
                sdp += "a=max-size:" + maxSize + SipUtils.CRLF;
            }
            if(msrpCemaParameter != null){
                sdp += msrpCemaParameter;
            }

            if (pauseInfo != null) {
                if (logger.isActivated()) {
                    logger.debug("FTS terminating file session pause object is null");
                }
                long bytesTransferrred = pauseInfo.bytesTransferrred;
                if (logger.isActivated()) {
                    logger.info("bytes transferred: " + bytesTransferrred
                            + "Old TransferId: " + oldFileTransferId);
                }
            }

            // Set the local SDP part in the dialog path
            getDialogPath().setLocalContent(sdp);

            // Test if the session should be interrupted
            if (isInterrupted()) {
                if (logger.isActivated()) {
                    logger.debug("FTS Session has been interrupted: end of processing");
                }
                return;
            }

            // Create the MSRP server session
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
            }

            // Create a 200 OK response
            if (logger.isActivated()) {
                logger.info("FTS Send 200 OK");
            }
            SipResponse resp = null;
            if (!RcsSettings.getInstance().isCPMSupported()) {
                resp = SipMessageFactory.create200OkInviteResponse(getDialogPath(),
                        InstantMessagingService.CHAT_FEATURE_TAGS, sdp);
            } else {
                if (logger.isActivated()) {
                    logger.info("TerminatingFileSharingSession0  FTS");
                }
                if(isUseLargeMode()){
                    resp = SipMessageFactory.createCpm200OkInviteResponse(getDialogPath(),
                            InstantMessagingService.CPM_LARGE_MESSAGE_FEATURE_TAGS, sdp);
                } else {
                    resp = SipMessageFactory.createCpm200OkInviteResponse(getDialogPath(),
                            InstantMessagingService.CPM_CHAT_FEATURE_TAGS, sdp);
                }
            }

            if (RcsSettings.getInstance().isCPMSupported()) {
                if (logger.isActivated()) {
                    logger.info("TerminatingFileSharingSession1  FTS");
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
                    logger.info("FTS ACK request received");
                }

                // Notify listeners
                for (int j = 0; j < getListeners().size(); j++) {
                    getListeners().get(j).handleSessionStarted();
                }

                // Create the MSRP client session
                if (localSetup.equals("active")) {
                    // Active mode: client should connect
                    msrpMgr.createMsrpClientSession(remoteHost, remotePort, remotePath,
                            this);

                    // Open the MSRP session
                    msrpMgr.openMsrpSession(ImsFileSharingSession.DEFAULT_SO_TIMEOUT);

                    // Send an empty packet
                    sendEmptyDataChunk();
                }

                // The session is established
                getDialogPath().sessionEstablished();

                // Start session timer
                if (getSessionTimerManager().isSessionTimerActivated(resp)) {
                    getSessionTimerManager().start(SessionTimerManager.UAS_ROLE,
                            getDialogPath().getSessionExpireTime());
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("FTS No ACK received for INVITE");
                }

                // No response received: timeout
                handleError(new FileSharingError(
                        FileSharingError.SESSION_INITIATION_FAILED));
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Session initiation has failed", e);
            }

            // Unexpected error
            handleError(new FileSharingError(FileSharingError.UNEXPECTED_EXCEPTION,
                    e.getMessage()));
        }

        if (logger.isActivated()) {
            logger.debug("FTS End of thread");
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
        if (logger.isActivated()) {
            logger.debug("Send delivery report " + messgeId);
        }
        if (messgeId != null) {
            String contact = getRemoteContact();
            contact = PhoneUtils.extractNumberFromUri(contact);
            if (logger.isActivated()) {
                logger.debug("Send delivery report " + status + ", contact: " + contact);
            }
            contact = PhoneUtils.formatNumberToSipUri(contact);
            if (logger.isActivated()) {
                logger.debug("Send delivery contact: " + contact);
            }
            // Send via SIP MESSAGE
            ((InstantMessagingService) getImsService()).getImdnManager().sendMessageDeliveryStatus(
                    contact, messgeId, ImdnDocument.DELIVERY_STATUS_DELIVERED);
        }
    }

    /**
     * Data has been transfered
     *
     * @param msgId Message ID
     */
    public void msrpDataTransfered(String msgId) {
        // Not used in terminating side
        if (logger.isActivated()) {
            logger.info("msrpDataTransfered");
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
            logger.info("FTS Data received Mime: " + mimeType);
        }
        
        try {
            if (ChatUtils.isMessageCpimType(mimeType)){
                CpimParser cpimParser = new CpimParser(data);
                CpimMessage cpimMsg = cpimParser.getCpimMessage();
                if (logger.isActivated()) {
                    logger.info("msrpDataReceived: " + cpimMsg);
                }
                if (cpimMsg != null) {
                    String cpimContent = cpimMsg.getMessageContent();
                    String cpimMsgId = cpimMsg.getHeader(ImdnUtils.HEADER_IMDN_MSG_ID);
                    if (logger.isActivated()) {
                        logger.info("msrpDataReceived msgid: " + cpimMsgId);
                    }
                    if (cpimMsgId != null) {
                        messgeId = cpimMsgId;
                    }
                    String dispositionNotification = cpimMsg.getHeader(ImdnUtils.HEADER_IMDN_DISPO_NOTIF);
                    if (logger.isActivated()) {
                        logger.info("msrpDataReceived dispositionNotification: " + dispositionNotification);
                    }
                    RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
                    rmHistory.updateFileTransferMsgId(getSessionID(), cpimMsgId);
                    if (dispositionNotification.contains(ImdnDocument.DISPLAY)) {
                        if (logger.isActivated()) {
                            logger.info("msrpDataReceived display is true: ");
                        }
                        this.setDisplayRequested(true);
                    }
                    if(!this.isDisplayRequested()){
                        rmHistory.updateFileTransferStatus(getSessionID(), FileTransfer.State.DISPLAYED.toInt());
                    }
                }
            }
        } catch (Exception e1) {
            e1.printStackTrace();
        }

        // File has been transfered
        fileTransfered();

        try {
            // Close content with received data
            getContent().writeData2File(data);
            getContent().closeFile();

            // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j))
                        .handleFileTransfered(getContent().getUrl(),getContent());
            }
            
            // Send text message
            Thread t = new Thread() {
                public void run() {
                    if (logger.isActivated()) {
                        logger.info("FTS Data received1");
                    }
                    if (isImdnSupport()) {
                        sendDeliveryReport(ImdnDocument.DELIVERY_STATUS_DELIVERED);
                    }
                }
            };
            t.start();
            
        } catch (IOException e) {
            // Delete the temp file
            deleteFile();

            // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j))
                        .handleTransferError(new FileSharingError(
                                FileSharingError.MEDIA_SAVING_FAILED));
            }
        } catch (Exception e) {
            // Delete the temp file
            deleteFile();

            // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j))
                        .handleTransferError(new FileSharingError(
                                FileSharingError.MEDIA_TRANSFER_FAILED));
            }
            if (logger.isActivated()) {
                logger.error("FTS Can't save received file", e);
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

            if (pauseInfo != null) {
                pauseInfo.bytesTransferrred = currentSize;
                pauseInfo.pausedStream = getContent().getOut();
            }
            if (logger.isActivated()) {
                logger.info("FTS msrpTransferProgress bytes transferred: " + currentSize
                        + "Old TransferId: " + oldFileTransferId + "hasselector: ");
            }

            // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j))
                        .handleTransferProgress(currentSize, totalSize);
            }
        } catch (Exception e) {
            // Delete the temp file
            deleteFile();
            if (logger.isActivated()) {
                logger.info("FTS msrpTransferProgress delete file exception");
            }
            // Notify listeners
            for (int j = 0; j < getListeners().size(); j++) {
                ((FileSharingSessionListener) getListeners().get(j))
                        .handleTransferError(new FileSharingError(
                                FileSharingError.MEDIA_SAVING_FAILED, e.getMessage()));
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
        
        if (logger.isActivated()) {
            logger.info("Data transfer error " + error);
        }
        String statusCode = null;
        int errorCode = FileSharingError.MEDIA_DOWNLOAD_FAILED;
        try{
            statusCode = error.substring(12);
        } catch(Exception e){
            logger.error("msrpTransferError ", e);
        }

        try {
            if(error.contains("timeout")){
                errorCode = FileSharingError.MEDIA_TRANSFER_FAILED;
            }
            // Close the media session
            closeMediaSession();
            
            terminateSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
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
            logger.info("FTS Data transfer aborted");
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
        if (logger.isActivated()) {
            logger.info("FTS prepareMediaSession isSendOnly: " + this.isSendOnly()
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
            logger.info("FTS startMediaSession isSendOnly: " + this.isSendOnly()
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
            logger.info("FTS prepareResumeMediaSession ");
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
            logger.info("FTS startResumeMediaSession ");
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
                bytesTransferredtoSkip = pauseInfo.bytesTransferrred;
                stream.skip(bytesTransferredtoSkip);
                if (logger.isActivated()) {
                    logger.error("startMediaSession resumed interrupted file byteskipped :"
                            + bytesTransferredtoSkip);
                }
            }
            msrpMgr.sendChunks(stream, ChatUtils.generateMessageId(), getContent()
                    .getEncoding(), getContent().getSize() - bytesTransferredtoSkip);
        } catch (Exception e) {
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
    public void closeMediaSession() {
        // Close MSRP session
        if (msrpMgr != null) {
            msrpMgr.closeSession();
            if (logger.isActivated()) {
                logger.debug("FTS MSRP session has been closed");
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
            logger.debug("FTS Delete incomplete received file Paused = "
                    + fileTransferPaused);
        }
        try {
            if (!fileTransferPaused)
                getContent().deleteFile();
        } catch (IOException e) {
            if (logger.isActivated()) {
                logger.error("FTS Can't delete received file", e);
            }
        }
    }
}
