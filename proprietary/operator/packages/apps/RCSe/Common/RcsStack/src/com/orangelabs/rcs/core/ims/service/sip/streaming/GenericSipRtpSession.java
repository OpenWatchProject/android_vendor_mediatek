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

package com.orangelabs.rcs.core.ims.service.sip.streaming;

import static com.orangelabs.rcs.utils.StringUtils.UTF8;

import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.protocol.rtp.MediaRtpReceiver;
import com.orangelabs.rcs.core.ims.protocol.rtp.MediaRtpSender;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.Format;
import com.orangelabs.rcs.core.ims.protocol.rtp.format.data.DataFormat;
import com.orangelabs.rcs.core.ims.protocol.rtp.stream.RtpStreamListener;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpUtils;
import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.orangelabs.rcs.core.ims.service.SessionNotEstablishedException;
import com.orangelabs.rcs.core.ims.service.sip.GenericSipSession;
import com.orangelabs.rcs.core.ims.service.sip.SipService;
import com.orangelabs.rcs.core.ims.service.sip.SipSessionError;
import com.orangelabs.rcs.core.ims.service.sip.SipSessionListener;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.NetworkRessourceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;
import com.orangelabs.rcs.core.CoreException;

/**
 * Generic SIP RTP session
 * 
 * @author jexa7410
 */
public abstract class GenericSipRtpSession extends GenericSipSession implements RtpStreamListener {
    /**
     * RTP payload format
     */
    protected DataFormat mFormat;

    private int mLocalRtpPort = -1;

    private DataSender mDataSender = new DataSender();

    private DataReceiver mDataReceiver = new DataReceiver(this);

    private MediaRtpReceiver mRtpReceiver;

    private MediaRtpSender mRtpSender;

    /**
     * Media Session started flag
     */
    private boolean mMediaSessionStarted;

    private final static Logger sLogger = Logger.getLogger(GenericSipRtpSession.class
            .getSimpleName());

    /**
     * Constructor
     * 
     * @param parent IMS service
     * @param contact Remote contact Id
     * @param featureTag Feature tag
     * @param rcsSettings RCS settings
     * @param timestamp Local timestamp for the session
     * @param encoding Encoding
     */
    public GenericSipRtpSession(SipService parent, ContactId contact, String featureTag, String encoding) throws CoreException{
        super(parent, contact, featureTag);

        /* Get local port */
        mLocalRtpPort = NetworkRessourceManager.generateLocalRtpPort();

        /* Create the RTP sender & receiver */
        mFormat = new DataFormat(encoding);
        mRtpReceiver = new MediaRtpReceiver(mLocalRtpPort);
        mRtpSender = new MediaRtpSender(mFormat, mLocalRtpPort);
    }

    /**
     * Get local port
     * 
     * @return RTP port
     */
    public int getLocalRtpPort() {
        return mLocalRtpPort;
    }

    /**
     * Returns the RTP receiver
     * 
     * @return RTP receiver
     */
    public MediaRtpReceiver getRtpReceiver() {
        return mRtpReceiver;
    }

    /**
     * Returns the RTP sender
     * 
     * @return RTP sender
     */
    public MediaRtpSender getRtpSender() {
        return mRtpSender;
    }

    /**
     * Returns the RTP format
     * 
     * @return RTP format
     */
    public Format getRtpFormat() {
        return mFormat;
    }

    /**
     * Generate SDP
     * 
     * @return SDP built
     */
    public String generateSdp() {
        String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
        String ipAddress = getDialogPath().getSipStack().getLocalIpAddress();
        return "v=0" + SipUtils.CRLF + "o=- " + ntpTime + " " + ntpTime + " "
                + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF + "s=-" + SipUtils.CRLF
                + "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF + "t=0 0"
                + SipUtils.CRLF + "m=application " + mLocalRtpPort + " RTP/AVP "
                + getRtpFormat().getPayload() + SipUtils.CRLF + "a=rtpmap:"
                + getRtpFormat().getPayload() + " " + getRtpFormat().getCodec()
                + SipUtils.CRLF +
                "a=sendrecv" + SipUtils.CRLF;
    }

    @Override
    public void prepareMediaSession() throws NetworkException {
        SdpParser parser = new SdpParser(getDialogPath().getRemoteContent().getBytes(UTF8));
        MediaDescription mediaApp = parser.getMediaDescription("application");

        // Extract session description
        String remoteHost = SdpUtils.extractRemoteHost(parser.sessionDescription, mediaApp);
        int remotePort = mediaApp.port;

        // Extract encoding name
        String rtpmap = mediaApp.getMediaAttribute("rtpmap").getValue();
        String encoding = rtpmap.substring(
                rtpmap.indexOf(mediaApp.payload) + mediaApp.payload.length() + 1).trim();

        mFormat = new DataFormat(encoding);
        try{
        mRtpReceiver.prepareSession(remoteHost, remotePort, mDataReceiver, mFormat, this);
        mRtpSender.prepareSession(mDataSender, remoteHost, remotePort,
                mRtpReceiver.getInputStream(), this);
        }catch(Exception e){
        	
        }
    }

    /**
     * Gets the encoding payload
     *
     * @param content SDP content
     * @return encoding
     */
    public static String getEncoding(String content) {
        SdpParser parser = new SdpParser(content.getBytes(UTF8));
        MediaDescription mediaApp = parser.getMediaDescription("application");
        String rtpmap = mediaApp.getMediaAttribute("rtpmap").getValue();
        String encoding = rtpmap.substring(
                rtpmap.indexOf(mediaApp.payload) + mediaApp.payload.length() + 1).trim();
        return encoding;
    }



    @Override
    public void startMediaTransfer() {
        synchronized (this) {
            mRtpReceiver.startSession();
            mRtpSender.startSession();
            mMediaSessionStarted = true;
        }
    }

    @Override
    public void closeMediaSession() {
        synchronized (this) {
            mMediaSessionStarted = false;
            mRtpSender.stopSession();
            mRtpReceiver.stopSession();
        }
    }

    /**
     * Sends a payload in real time
     * 
     * @param content Payload content
     * @throws SessionNotEstablishedException
     */
    public void sendPlayload(byte[] content) throws SessionNotEstablishedException {
        if (!mMediaSessionStarted) {
            throw new SessionNotEstablishedException("Unable to send payload!");
        }
        mDataSender.addFrame(content, System.currentTimeMillis());
    }

    @Override
    public void rtpStreamAborted() {
        try {
            if (isSessionInterrupted()) {
                return;
            }
            if (sLogger.isActivated()) {
                sLogger.error("Media has failed: network failure");
            }
            closeMediaSession();
            closeSession(TerminationReason.TERMINATION_BY_SYSTEM);
            removeSession();
            for (ImsSessionListener listener : getListeners()) {
                ((SipSessionListener) listener).onSessionError(getRemoteContactId(), new SipSessionError(
                        SipSessionError.MEDIA_FAILED));
            }

        } catch (Exception e) {
            /*
             * Normally we are not allowed to catch runtime exceptions as these are genuine bugs
             * which should be handled/fixed within the code. However the cases when we are
             * executing operations on a thread unhandling such exceptions will eventually lead to
             * exit the system and thus can bring the whole system down, which is not intended.
             */
            sLogger.error("Failed to abort rtp stream!", e);
        }
    }

    /**
     * Receive media data
     *
     * @param data Data
     * @param mimeType MIME-type
     */
    public void receiveData(byte[] data, String mimeType) {
        for (ImsSessionListener listener : getListeners()) {
            ((SipSessionListener) listener).onDataReceived(getRemoteContactId(), data, mimeType);
        }
    }

    @Override
    public void startSession() {
        getSipService().addSession(this);
        start();
    }

    @Override
    public void removeSession() {
        getSipService().removeSession(this);
    }
}
