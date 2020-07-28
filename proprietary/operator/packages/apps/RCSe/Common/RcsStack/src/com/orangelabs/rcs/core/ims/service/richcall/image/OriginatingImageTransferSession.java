/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.service.richcall.image;

import static com.orangelabs.rcs.utils.StringUtils.UTF8;

import com.orangelabs.rcs.core.FileAccessException;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpEventListener;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpManager;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpSession;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityService;
import com.orangelabs.rcs.core.ims.service.richcall.ContentSharingError;
import com.orangelabs.rcs.core.ims.service.richcall.RichcallService;
import com.orangelabs.rcs.platform.AndroidFactory;
//import com.orangelabs.rcs.provider.contact.ContactManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.Base64;
import com.orangelabs.rcs.utils.CloseableUtils;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;
import com.orangelabs.rcs.provider.eab.ContactsManager;

import android.net.Uri;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.text.ParseException;

import javax2.sip.InvalidArgumentException;
import javax2.sip.header.ContentDispositionHeader;
import javax2.sip.header.ContentLengthHeader;
import javax2.sip.header.ContentTypeHeader;

/**
 * Originating content sharing session (transfer)
 * 
 * @author jexa7410
 */
public class OriginatingImageTransferSession extends ImageTransferSession implements
        MsrpEventListener {

    private final static String BOUNDARY_TAG = "boundary1";

    private MsrpManager msrpMgr;

    private final static Logger sLogger = Logger.getLogger(OriginatingImageTransferSession.class
            .getSimpleName());

    /**
     * Constructor
     * 
     * @param parent Richcall service
     * @param content Content to be shared
     * @param contact Remote contact Id
     * @param thumbnail Thumbnail content option
     * @param rcsSettings The RCS settings accessor
     * @param timestamp Local timestamp for the session
     * @param contactManager The contact manager accessor
     * @param capabilityService The capability service
     */
    public OriginatingImageTransferSession(RichcallService parent, MmContent content,
            ContactId contact, MmContent thumbnail, long timestamp,
            CapabilityService capabilityService) {
        super(parent, content, contact, thumbnail,  timestamp,
                capabilityService);
        createOriginatingDialogPath();
    }

    private byte[] getFileData(Uri file, int size) throws IOException {
        FileInputStream fileInputStream = null;
        try {
            fileInputStream = (FileInputStream) AndroidFactory.getApplicationContext()
                    .getContentResolver().openInputStream(file);
            byte[] data = new byte[size];
            if (size != fileInputStream.read(data, 0, size)) {
                throw new IOException("Unable to retrieve data from " + file);
            }
            return data;
        } finally {
            CloseableUtils.tryToClose(fileInputStream);
        }
    }

    @Override
    public void run() {
        try {
            if (sLogger.isActivated()) {
                sLogger.info("Initiate a new sharing session as originating");
            }
            // Set setup mode
            String localSetup = createMobileToMobileSetupOffer();
            if (sLogger.isActivated()) {
                sLogger.debug("Local setup attribute is " + localSetup);
            }
            // Set local port
            int localMsrpPort;
            if ("active".equals(localSetup)) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }
            // Create the MSRP manager
            String localIpAddress = getImsService().getImsModule().getCurrentNetworkInterface()
                    .getNetworkAccess().getIpAddress();
            msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            // Build SDP part
            String ipAddress = getDialogPath().getSipStack().getLocalIpAddress();
            String encoding = getContent().getEncoding();
            long maxSize = ImageTransferSession.getMaxImageSharingSize();
            // Set File-selector attribute
            String selector = getFileSelectorAttribute();
            StringBuilder sdp = new StringBuilder(SdpUtils.buildFileSDP(ipAddress, localMsrpPort,
                    msrpMgr.getLocalSocketProtocol(), encoding, getFileTransferId(), selector,
                    "render", localSetup, msrpMgr.getLocalMsrpPath(), SdpUtils.DIRECTION_SENDONLY,
                    maxSize));
            // Set File-location attribute
            Uri location = getFileLocationAttribute();
            if (location != null) {
                sdp.append("a=file-location:").append(location.toString()).append(SipUtils.CRLF);
            }
            MmContent fileIcon = getThumbnail();
            if (fileIcon == null) {
                /* Set the local SDP part in the dialog path */
                getDialogPath().setLocalContent(sdp.toString());

            } else {
                Capabilities remoteCapabilities = ContactsManager
                        .getInstance().getContactCapabilities(getRemoteContact());
                boolean fileIconSupported = remoteCapabilities != null
                        && remoteCapabilities.isFileTransferThumbnailSupported();
                if (fileIconSupported) {
                    sdp.append("a=file-icon:cid:image@joyn.com").append(SipUtils.CRLF);
                    // Encode the thumbnail file
                    String imageEncoded = Base64.encodeBase64ToString(getFileData(
                            fileIcon.getUri(), (int) fileIcon.getSize()));
                    String sdpContent = sdp.toString();
                    String multipart = Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF
                            + ContentTypeHeader.NAME + ": application/sdp" + SipUtils.CRLF
                            + ContentLengthHeader.NAME + ": " + sdpContent.getBytes(UTF8).length
                            + SipUtils.CRLF + SipUtils.CRLF + sdpContent + SipUtils.CRLF
                            + Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF
                            + ContentTypeHeader.NAME + ": " + getContent().getEncoding()
                            + SipUtils.CRLF + SipUtils.HEADER_CONTENT_TRANSFER_ENCODING
                            + ": base64" + SipUtils.CRLF + SipUtils.HEADER_CONTENT_ID
                            + ": <image@joyn.com>" + SipUtils.CRLF + ContentLengthHeader.NAME
                            + ": " + imageEncoded.length() + SipUtils.CRLF
                            + ContentDispositionHeader.NAME + ": icon" + SipUtils.CRLF
                            + SipUtils.CRLF + imageEncoded + SipUtils.CRLF
                            + Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG
                            + Multipart.BOUNDARY_DELIMITER;
                    // Set the local SDP part in the dialog path
                    getDialogPath().setLocalContent(multipart);

                } else {
                    // Set the local SDP part in the dialog path
                    getDialogPath().setLocalContent(sdp.toString());
                }
            }
            // Create an INVITE request
            if (sLogger.isActivated()) {
                sLogger.info("Send INVITE");
            }
            SipRequest invite = createInvite();
            // Set the Authorization header
            getAuthenticationAgent().setAuthorizationHeader(invite);
            // Set initial request in the dialog path
            getDialogPath().setInvite(invite);
            // Send INVITE request
            sendInvite(invite);

        } catch (Exception e) {
            sLogger.error("Failed to send invite!", e);
            handleError(new ContentSharingError(ContentSharingError.SESSION_INITIATION_FAILED, e));

        } 
        if (sLogger.isActivated()) {
            sLogger.debug("End of thread");
        }
    }

    @Override
    public void prepareMediaSession() {
        /* Get the remote SDP part */
        byte[] sdp = getDialogPath().getRemoteContent().getBytes(UTF8);
        MsrpSession session = msrpMgr.createMsrpSession(sdp, this);
        session.setFailureReportOption(true);
        session.setSuccessReportOption(false);
        /* Do not use right now the mapping to do not increase memory and cpu consumption */
       // session.setMapMsgIdFromTransationId(false);
    }

//    @Override
    public void openMediaSession() throws Exception {
        msrpMgr.openMsrpSession();
    }

    //@Override
    public void startMediaTransfer() throws NetworkException, FileAccessException {
        try {
            /* Start sending data chunks */
            InputStream stream = AndroidFactory.getApplicationContext().getContentResolver()
                    .openInputStream(getContent().getUri());
            msrpMgr.sendChunks(stream, getFileTransferId(), getContent().getEncoding(),
                    getContent().getSize());

        } catch (Exception e) {
            throw new FileAccessException("Failed to initiate media transfer!", e);
        }
    }

    @Override
    public void closeMediaSession() {
        if (msrpMgr != null) {
            msrpMgr.closeSession();
        }
        if (sLogger.isActivated()) {
            sLogger.debug("MSRP session has been closed");
        }
    }

    @Override
    public void msrpDataTransfered(String msgId) {
        try {
            if (sLogger.isActivated()) {
                sLogger.info("Data transferred");
            }
            setImageTransferred();
            closeMediaSession();
            closeSession(TerminationReason.TERMINATION_BY_USER);
            removeSession();
            ContactId contact = getRemoteContactId();
            Uri image = getContent().getUri();
            for (ImsSessionListener listener : getListeners()) {
                ((ImageTransferSessionListener) listener).onContentTransferred(contact, image);
            }
        } catch (Exception e) {
            if (sLogger.isActivated()) {
                sLogger.debug(e.getMessage());
            }
        }
    }

    @Override
    public void msrpDataReceived(String msgId, byte[] data, String mimeType) {
        // Not used in originating side
    }

    @Override
    public void msrpTransferProgress(long currentSize, long totalSize) {
        ContactId contact = getRemoteContactId();
        for (ImsSessionListener listener : getListeners()) {
            ((ImageTransferSessionListener) listener).onSharingProgress(contact, currentSize,
                    totalSize);
        }
    }

    @Override
    public boolean msrpTransferProgress(long currentSize, long totalSize, byte[] data) {
        // Not used in originating side
        return false;
    }

    @Override
    public void msrpTransferAborted() {
        if (sLogger.isActivated()) {
            sLogger.info("Data transfer aborted");
        }
    }

    @Override
    public void msrpTransferError(String msgId, String error) {
        try {
            if (isSessionInterrupted() || isInterrupted() || getDialogPath().isSessionTerminated()) {
                return;
            }
            if (sLogger.isActivated()) {
                sLogger.info("Data transfer error " + error);
            }
            closeSession(com.orangelabs.rcs.core.ims.service.ImsServiceSession.TerminationReason.TERMINATION_BY_SYSTEM);
            closeMediaSession();
            ContactId contact = getRemoteContactId();
            ContactsManager.getInstance().getContactCapabilities(getRemoteContact());
            removeSession();
            for (ImsSessionListener listener : getListeners()) {
                ((ImageTransferSessionListener) listener).onSharingError(contact,
                        new ContentSharingError(ContentSharingError.MEDIA_TRANSFER_FAILED));
            }
        } catch (Exception e) {
            sLogger.error("Failed to handle msrp error" + error + " for message " + msgId, e);
        }
    }

    @Override
    public boolean isInitiatedByRemote() {
        return false;
    }

    @Override
    public void handle180Ringing(SipResponse response) {
        if (sLogger.isActivated()) {
            sLogger.debug("handle180Ringing");
        }
        ContactId contact = getRemoteContactId();
        for (ImsSessionListener listener : getListeners()) {
            ((ImageTransferSessionListener) listener).onSessionRinging(contact);
        }
    }
}
