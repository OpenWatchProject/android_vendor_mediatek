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

package com.orangelabs.rcs.core.ims.service.richcall.video;

import static com.orangelabs.rcs.utils.StringUtils.UTF8;

import com.orangelabs.rcs.core.content.ContentManager;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
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
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.sharing.video.IVideoPlayer;
import com.gsma.services.rcs.sharing.video.VideoCodec;

import android.os.RemoteException;

import java.util.Collection;
import java.util.Vector;

/**
 * Terminating live video content sharing session (streaming)
 * 
 * @author Jean-Marc AUFFRET
 */
public class TerminatingVideoStreamingSession extends VideoStreamingSession {

    private static final Logger sLogger = Logger.getLogger(TerminatingVideoStreamingSession.class
            .getName());

    /**
     * Constructor
     * 
     * @param parent Richcall service
     * @param invite Initial INVITE request
     * @param contact Contact Id
     * @param timestamp Local timestamp for the session
     * @param contactManager
     * @param capabilityService
     */
    public TerminatingVideoStreamingSession(RichcallService parent, SipRequest invite,
            ContactId contact, long timestamp
            , CapabilityService capabilityService) {
        super(parent, ContentManager.createLiveVideoContentFromSdp(invite.getContentBytes()),
                contact, timestamp, capabilityService);
        createTerminatingDialogPath(invite);
    }

    @Override
    public void run() {
        try {
            if (sLogger.isActivated()) {
                sLogger.info("Initiate a new live video sharing session as terminating");
            }
            SipDialogPath dialogPath = getDialogPath();
            // Send a 180 Ringing response
            send180Ringing(dialogPath.getInvite(), dialogPath.getLocalTag());

            // Parse the remote SDP part
            SdpParser parser = new SdpParser(dialogPath.getRemoteContent().getBytes(UTF8));
            MediaDescription mediaVideo = parser.getMediaDescription("video");
            String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription, mediaVideo);
            int remotePort = mediaVideo.port;

            // Extract video codecs from SDP
            Vector<MediaDescription> medias = parser.getMediaDescriptions("video");
            Vector<VideoCodec> proposedCodecs = VideoCodecManager.extractVideoCodecsFromSdp(medias);

            // Notify listener
            Collection<ImsSessionListener> listeners = getListeners();
            ContactId contact = getRemoteContactId();
            MmContent content = getContent();
            long timestamp = getTimestamp();
            for (ImsSessionListener listener : listeners) {
                ((VideoStreamingSessionListener) listener).onInvitationReceived(contact, content,
                        timestamp);
            }

            // Wait invitation answer
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

                    for (ImsSessionListener listener : listeners) {
                        //listener.onSessionAccepting(contact);
                    }
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

            IVideoPlayer player = getPlayer();
            // Check that a video player has been set
            if (player == null) {
                handleError(new ContentSharingError(
                        ContentSharingError.MEDIA_PLAYER_NOT_INITIALIZED));
                return;
            }

            // Codec negotiation
            VideoCodec selectedVideoCodec = VideoCodecManager.negociateVideoCodec(
                    player.getSupportedCodecs(), proposedCodecs);
            if (selectedVideoCodec == null) {
                if (sLogger.isActivated()) {
                    sLogger.debug("Proposed codecs are not supported");
                }

                // Send a 415 Unsupported media type response
                send415Error(dialogPath.getInvite());

                // Unsupported media type
                handleError(new ContentSharingError(ContentSharingError.UNSUPPORTED_MEDIA_TYPE));
                return;
            }

            // Set the video player orientation
            SdpOrientationExtension extensionHeader = SdpOrientationExtension.create(mediaVideo);
            if (extensionHeader != null) {
                // Update the orientation ID
                setOrientation(extensionHeader.getExtensionId());
            }

            // Build SDP part
            // Note ID_6_5 Extmap: it is recommended not to change the extmap's local
            // identifier in the SDP answer from the one in the SDP offer because there
            // are no reasons to do that since there should only be one extension in use.
            String ipAddress = dialogPath.getSipStack().getLocalIpAddress();
            String videoSdp = VideoSdpBuilder.buildSdpAnswer(selectedVideoCodec,
                    player.getLocalRtpPort(), mediaVideo);
            String sdp = SdpUtils.buildVideoSDP(ipAddress, videoSdp, SdpUtils.DIRECTION_RECVONLY);

            // Set the local SDP part in the dialog path
            dialogPath.setLocalContent(sdp);

            // Test if the session should be interrupted
            if (isInterrupted()) {
                if (sLogger.isActivated()) {
                    sLogger.debug("Session has been interrupted: end of processing");
                }
                return;
            }

            // Create a 200 OK response
            if (sLogger.isActivated()) {
                sLogger.info("Send 200 OK");
            }
            SipTransactionContext ctx = null;
            SipResponse resp = null;
            try{
             resp = SipMessageFactory.create200OkInviteResponse(dialogPath,
                    RichcallService.FEATURE_TAGS_VIDEO_SHARE, sdp);

            // The signalisation is established
            dialogPath.setSigEstablished();

            // Send response
             ctx = getImsService().getImsModule().getSipManager()
                    .sendSipMessageAndWait(resp);
            }catch(Exception e){
            	
            }
            // Analyze the received response
            if (ctx.isSipAck()) {
                // ACK received
                if (sLogger.isActivated()) {
                    sLogger.info("ACK request received");
                }

                // The session is established
                dialogPath.setSessionEstablished();

                // Start session timer
                SessionTimerManager sessionTimerManager = getSessionTimerManager();
                if (sessionTimerManager.isSessionTimerActivated(resp)) {
                    sessionTimerManager.start(SessionTimerManager.UAS_ROLE,
                            dialogPath.getSessionExpireTime());
                }

                // Set the video player remote info
                player.setRemoteInfo(selectedVideoCodec, remoteHost, remotePort, getOrientation());

                for (ImsSessionListener listener : listeners) {
                    //listener.onSessionStarted(contact);
                }
            } else {
                if (sLogger.isActivated()) {
                    sLogger.debug("No ACK received for INVITE");
                }

                // No response received: timeout
                handleError(new ContentSharingError(ContentSharingError.SEND_RESPONSE_FAILED));
            }

        } catch (Exception e) {
            sLogger.error("Failed to set remote info!", e);
            handleError(new ContentSharingError(ContentSharingError.SESSION_INITIATION_FAILED, e));

        }
    }

    /**
     * Handle error
     * 
     * @param error Error
     */
    public void handleError(ContentSharingError error) {
        if (isSessionInterrupted()) {
            return;
        }
        if (sLogger.isActivated()) {
            sLogger.info(new StringBuilder("Session error: ")
                    .append(String.valueOf(error.getErrorCode())).append(", reason=")
                    .append(error.getMessage()).toString());
        }
        closeMediaSession();
        removeSession();
        ContactId contact = getRemoteContactId();
        for (ImsSessionListener listener : getListeners()) {
            ((VideoStreamingSessionListener) listener).onSharingError(contact, error);
        }
    }

    @Override
    public void prepareMediaSession() {
        /* Nothing to do in case of external codec */
    }

   // @Override
    public void openMediaSession() {
        /* Nothing to do in case of external codec */
    }

  // @Override
    public void startMediaTransfer() {
        /* Nothing to do in case of external codec */
    }

    @Override
    public void closeMediaSession() {
        /* Nothing to do in case of external codec */
    }

    @Override
    public boolean isInitiatedByRemote() {
        return true;
    }
}
