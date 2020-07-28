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
import com.orangelabs.rcs.core.content.ContentManager;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpConstants;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpEventListener;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpManager;
import com.orangelabs.rcs.core.ims.protocol.msrp.MsrpSession;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaAttribute;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.orangelabs.rcs.core.ims.service.SessionTimerManager;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityService;
import com.orangelabs.rcs.core.ims.service.richcall.ContentSharingError;
import com.orangelabs.rcs.core.ims.service.richcall.RichcallService;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;

import android.net.Uri;

import java.io.IOException;
import java.util.Collection;
import java.util.Vector;

/**
 * Terminating content sharing session (transfer)
 * 
 * @author jexa7410
 */
public class TerminatingImageTransferSession extends ImageTransferSession implements
        MsrpEventListener {

    private MsrpManager msrpMgr;

    private static final Logger sLogger = Logger.getLogger(TerminatingImageTransferSession.class
            .getName());

    /**
     * Constructor
     * 
     * @param parent Richcall service
     * @param invite Initial INVITE request
     * @param contact Contact ID
     * @param rcsSettings The RCS settings accessor
     * @param timestamp Local timestamp for the session
     * @param contactManager The contact manager accessor
     * @param capabilityService The capability service
     * @throws PayloadException
     * @throws FileAccessException
     */
    public TerminatingImageTransferSession(RichcallService parent, SipRequest invite,
            ContactId contact, long timestamp,
             CapabilityService capabilityService)
            throws PayloadException, FileAccessException {
        super(parent, 
                null, contact,null,timestamp,
                 capabilityService);
        createTerminatingDialogPath(invite);
    }

    @Override
    public void run() {
        try {
            if (sLogger.isActivated()) {
                sLogger.info("Initiate a new sharing session as terminating");
            }
            SipDialogPath dialogPath = getDialogPath();
            send180Ringing(dialogPath.getInvite(), dialogPath.getLocalTag());

            if (getContent() == null) {
                if (sLogger.isActivated()) {
                    sLogger.debug("MIME type is not supported");
                }
                send415Error(dialogPath.getInvite());
                handleError(new ContentSharingError(ContentSharingError.UNSUPPORTED_MEDIA_TYPE));
                return;
            }

            Collection<ImsSessionListener> listeners = getListeners();
            ContactId contact = getRemoteContactId();
            MmContent content = getContent();
            long timestamp = getTimestamp();
            for (ImsSessionListener listener : listeners) {
                ((ImageTransferSessionListener) listener).onInvitationReceived(contact, content,
                        timestamp);
            }

            int answer = waitInvitationAnswer();
            switch (answer) {
                case INVITATION_REJECTED_DECLINE:
                    /* Intentional fall through */
                case INVITATION_REJECTED_BUSY_HERE:
                    if (sLogger.isActivated()) {
                        sLogger.debug("Session has been rejected by user");
                    }
                    sendErrorResponse(dialogPath.getInvite(), dialogPath.getLocalTag(), answer);
                    removeSession();

                    for (ImsSessionListener listener : listeners) {
                        listener.handleSessionTerminatedByRemote();
                    }
                    return;

                case INVITATION_TIMEOUT:
                    if (sLogger.isActivated()) {
                        sLogger.debug("Session has been rejected on timeout");
                    }

                    // Ringing period timeout
                    send486Busy(dialogPath.getInvite(), dialogPath.getLocalTag());

                    removeSession();

                    for (ImsSessionListener listener : listeners) {
                        listener.handleSessionTerminatedByRemote();
                    }
                    return;

                case INVITATION_REJECTED_BY_SYSTEM:
                    if (sLogger.isActivated()) {
                        sLogger.debug("Session has been aborted by system");
                    }
                    removeSession();
                    return;

                case INVITATION_CANCELED:
                    if (sLogger.isActivated()) {
                        sLogger.debug("Session has been rejected by remote");
                    }

                    removeSession();

                    for (ImsSessionListener listener : listeners) {
                        listener.handleSessionTerminatedByRemote();
                    }
                    return;

                case INVITATION_ACCEPTED:
                    setSessionAccepted();

                  /*  for (ImsSessionListener listener : listeners) {
                        listener.onSessionAccepting(contact);
                    }*/
                    break;

                case INVITATION_DELETED:
                    if (sLogger.isActivated()) {
                        sLogger.debug("Session has been deleted");
                    }
                    removeSession();
                    return;

                default:
                    throw new IllegalArgumentException(new StringBuilder(
                            "Unknown invitation answer in run; answer=").append(answer).toString());
            }
            final SipRequest invite = dialogPath.getInvite();
            String remoteSdp = invite.getSdpContent();
           // SipUtils.assertContentIsNotNull(remoteSdp, invite);
            SdpParser parser = new SdpParser(remoteSdp.getBytes(UTF8));
            Vector<MediaDescription> media = parser.getMediaDescriptions();
            MediaDescription mediaDesc = media.elementAt(0);
            String protocol = mediaDesc.protocol;
            boolean isSecured = false;
            if (protocol != null) {
                isSecured = protocol.equalsIgnoreCase(MsrpConstants.SOCKET_MSRP_SECURED_PROTOCOL);
            }
            // Changed by Deutsche Telekom
            String fileSelector = mediaDesc.getMediaAttribute("file-selector").getValue();
            // Changed by Deutsche Telekom
            String fileTransferId = mediaDesc.getMediaAttribute("file-transfer-id").getValue();
            MediaAttribute attr3 = mediaDesc.getMediaAttribute("path");
            String remotePath = attr3.getValue();
            String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription, mediaDesc);
            int remotePort = mediaDesc.port;

            // Changed by Deutsche Telekom
            String fingerprint = SdpUtils.extractFingerprint(parser, mediaDesc);
            String remoteSetup = "passive";
            MediaAttribute attr4 = mediaDesc.getMediaAttribute("setup");
            if (attr4 != null) {
                remoteSetup = attr4.getValue();
            }
            if (sLogger.isActivated()) {
                sLogger.debug("Remote setup attribute is ".concat(remoteSetup));
            }
            String localSetup = createSetupAnswer(remoteSetup);
            if (sLogger.isActivated()) {
                sLogger.debug("Local setup attribute is ".concat(localSetup));
            }
            int localMsrpPort;
            if (localSetup.equals("active")) {
                localMsrpPort = 9; // See RFC4145, Page 4
            } else {
                localMsrpPort = NetworkRessourceManager.generateLocalMsrpPort();
            }
            String localIpAddress = getImsService().getImsModule().getCurrentNetworkInterface()
                    .getNetworkAccess().getIpAddress();
            msrpMgr = new MsrpManager(localIpAddress, localMsrpPort);
            msrpMgr.setSecured(isSecured);
            String ipAddress = dialogPath.getSipStack().getLocalIpAddress();
            long maxSize = ImageTransferSession.getMaxImageSharingSize();
            String sdp = SdpUtils.buildFileSDP(ipAddress, localMsrpPort,
                    msrpMgr.getLocalSocketProtocol(), getContent().getEncoding(), fileTransferId,
                    fileSelector, null, localSetup, msrpMgr.getLocalMsrpPath(),
                    SdpUtils.DIRECTION_RECVONLY, maxSize);
            dialogPath.setLocalContent(sdp);
            if (isInterrupted()) {
                if (sLogger.isActivated()) {
                    sLogger.debug("Session has been interrupted: end of processing");
                }
                return;
            }
            if (sLogger.isActivated()) {
                sLogger.info("Send 200 OK");
            }
            SipResponse resp = SipMessageFactory.create200OkInviteResponse(dialogPath,
                    RichcallService.FEATURE_TAGS_IMAGE_SHARE, sdp);
            dialogPath.setSigEstablished();
            SipTransactionContext ctx = null;/*getImsService().getImsModule().getSipManager()
                    .sendSipMessage(resp);*/
            
            if (localSetup.equals("passive")) {
                // Passive mode: client wait a connection
                // Changed by Deutsche Telekom
                MsrpSession session = msrpMgr.createMsrpServerSession(remotePath, this);
                // Do not use right now the mapping to do not increase memory and cpu consumption
               // session.setMapMsgIdFromTransationId(false);

                msrpMgr.openMsrpSession((int)ImageTransferSession.DEFAULT_SO_TIMEOUT);
                sendEmptyDataChunk();
            }
            getImsService().getImsModule().getSipManager().waitResponse(ctx);
            if (isInterrupted()) {
                if (sLogger.isActivated()) {
                    sLogger.debug("Session has been interrupted: end of processing");
                }
                return;
            }
            if (ctx.isSipAck()) {
                if (sLogger.isActivated()) {
                    sLogger.info("ACK request received");
                }
                if (localSetup.equals("active")) {
                    // Active mode: client should connect
                    // Changed by Deutsche Telekom
                    MsrpSession session = msrpMgr.createMsrpClientSession(remoteHost, remotePort,
                            remotePath, this);
                  //  session.setMapMsgIdFromTransationId(false);
                    msrpMgr.openMsrpSession((int)ImageTransferSession.DEFAULT_SO_TIMEOUT);
                    sendEmptyDataChunk();
                }
                dialogPath.setSessionEstablished();
                for (ImsSessionListener listener : listeners) {
                    listener.handleSessionStarted();
                }
                SessionTimerManager sessionTimerManager = getSessionTimerManager();
                if (sessionTimerManager.isSessionTimerActivated(resp)) {
                    sessionTimerManager.start(SessionTimerManager.UAS_ROLE,
                            dialogPath.getSessionExpireTime());
                }
            } else {
                if (sLogger.isActivated()) {
                    sLogger.debug("No ACK received for INVITE");
                }
                handleError(new ContentSharingError(ContentSharingError.SEND_RESPONSE_FAILED));
            }

        } catch (Exception e) {
            sLogger.error("Failed to send 200OK response!", e);
            handleError(new ContentSharingError(ContentSharingError.SEND_RESPONSE_FAILED, e));

        } 

        if (sLogger.isActivated()) {
            sLogger.debug("End of thread");
        }
    }

	/**
	 * Send an empty data chunk
	 * 
	 * @throws NetworkException
	 */
	public void sendEmptyDataChunk() throws NetworkException {

		try {
			msrpMgr.sendEmptyChunk();
		} catch (Exception e) {

		}
	}

    @Override
    public void msrpDataTransfered(String msgId) {
        // Not used in terminating side
    }


    //@Override
    public void receiveMsrpData(String msgId, byte[] data, String mimeType) {
        if (sLogger.isActivated()) {
            sLogger.info("Data received");
        }
        setImageTransferred();
        ContactId contact = getRemoteContactId();
        MmContent content = getContent();
        try {
            content.writeData2File(data);
            content.closeFile();

            Uri image = getContent().getUri();
            for (ImsSessionListener listener : getListeners()) {
                ((ImageTransferSessionListener) listener).onContentTransferred(contact, image);
            }

        } catch (Exception e) {
            deleteFile();
            for (ImsSessionListener listener : getListeners()) {
                ((ImageTransferSessionListener) listener).onSharingError(contact,
                        new ContentSharingError(ContentSharingError.MEDIA_SAVING_FAILED));
            }
        }
    }

    @Override
    public void msrpTransferProgress(long currentSize, long totalSize) {
        // Not used
    }

    @Override
    public boolean msrpTransferProgress(long currentSize, long totalSize, byte[] data) {
        ContactId contact = getRemoteContactId();
        try {
            getContent().writeData2File(data);
            for (ImsSessionListener listener : getListeners()) {
                ((ImageTransferSessionListener) listener).onSharingProgress(contact, currentSize,
                        totalSize);
            }

        } catch (Exception e) {
            deleteFile();
            for (ImsSessionListener listener : getListeners()) {
                ((ImageTransferSessionListener) listener).onSharingError(contact,
                        new ContentSharingError(ContentSharingError.MEDIA_TRANSFER_FAILED));
            }
        }
        return true;
    }

    @Override
    public void msrpTransferAborted() {
        if (sLogger.isActivated()) {
            sLogger.info("Data transfer aborted");
        }
        if (!isImageTransferred()) {
            deleteFile();
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

            closeSession(TerminationReason.TERMINATION_BY_SYSTEM);
            closeMediaSession();

            ContactId contact = getRemoteContactId();
            //mCapabilityService.requestContactCapabilities(contact);
            removeSession();

            if (isImageTransferred()) {
                return;
            }
            for (ImsSessionListener listener : getListeners()) {
                ((ImageTransferSessionListener) listener).onSharingError(contact,
                        new ContentSharingError(ContentSharingError.MEDIA_TRANSFER_FAILED));
            }

        } catch (Exception e) {
            sLogger.error(
                    new StringBuilder("Failed to handle msrp error").append(error)
                            .append(" for message ").append(msgId).toString(), e);

        }
    }

    @Override
    public void prepareMediaSession() {
        /* Nothing to do in terminating side */
    }

    //@Override
    public void openMediaSession() {
        /* Nothing to do in terminating side */
    }

    //@Override
    public void startMediaTransfer() {
        /* Nothing to do in terminating side */
    }

    @Override
    public void closeMediaSession() {
        if (msrpMgr != null) {
            msrpMgr.closeSession();
        }
        if (sLogger.isActivated()) {
            sLogger.debug("MSRP session has been closed");
        }
        if (!isImageTransferred()) {
            deleteFile();
        }
    }

    /**
     * Delete file
     */
    private void deleteFile() {
        if (sLogger.isActivated()) {
            sLogger.debug("Delete incomplete received image");
        }
        try {
            getContent().deleteFile();
        } catch (IOException e) {
            if (sLogger.isActivated()) {
                sLogger.error("Can't delete received image", e);
            }
        }
    }

    @Override
    public boolean isInitiatedByRemote() {
        return true;
    }
    
    @Override
    public void msrpDataReceived(String s , byte[] d, String df) {
    	
    }
    
}
