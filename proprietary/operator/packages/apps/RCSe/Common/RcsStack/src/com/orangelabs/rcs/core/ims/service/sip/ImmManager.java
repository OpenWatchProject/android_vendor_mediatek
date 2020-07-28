/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications AB.
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
 * NOTE: This file has been modified by Sony Mobile Communications AB.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.orangelabs.rcs.core.ims.service.sip;


import com.gsma.services.rcs.contact.ContactId;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.network.sip.SipManager;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipInterface;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.SessionAuthenticationAgent;
import com.orangelabs.rcs.utils.FifoBuffer;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

import java.text.ParseException;

import javax2.sip.InvalidArgumentException;
import javax2.sip.message.Response;

/**
 * Instant Multimedia Message manager
 *
 * @author jexa7410
 */
public class ImmManager extends Thread {

    private final SipService mSipService;

    private FifoBuffer mBuffer = new FifoBuffer();


    private final static Logger sLogger = Logger.getLogger(ImmManager.class.getSimpleName());

    /**
     * Constructor
     *
     * @param sipService SIP service
     * @param rcsSettings RCS settings
     */
    public ImmManager(SipService sipService) {
        mSipService = sipService;
    }

    /**
     * Terminate manager
     */
    public void terminate() {
        if (sLogger.isActivated()) {
            sLogger.info("Terminate the IMM manager");
        }
        mBuffer.close();
    }

    /**
     * Background processing
     */
    public void run() {
        InstantMultimediaMessage msg;
        while ((msg = (InstantMultimediaMessage) mBuffer.getObject()) != null) {
            try {
                sendSipMessage(msg, null);// TODO: add sip.instance

            } catch (PayloadException | RuntimeException e) {
                sLogger.error("Failed to send instant multimedia message", e);

            } catch (NetworkException e) {
                if (sLogger.isActivated()) {
                    sLogger.debug(e.getMessage());
                }
            }
        }
    }

    /**
     * Send a multimedia message
     *
     * @param remote Remote contact
     * @param featureTag Feature tag
     * @param content Message content
     * @param contentType Message content type
     */
    public void sendMessage(ContactId remote, String featureTag, byte[] content, String contentType) {
        // Add request in the buffer for background processing
        InstantMultimediaMessage msg = new InstantMultimediaMessage(remote, featureTag, content,
                contentType);
        mBuffer.addObject(msg);
    }

    private void analyzeSipResponse(SipTransactionContext ctx,
            SessionAuthenticationAgent authenticationAgent, SipDialogPath dialogPath,
            InstantMultimediaMessage imm) throws NetworkException, PayloadException,
            InvalidArgumentException, ParseException {
        int statusCode = ctx.getStatusCode();
        switch (statusCode) {
            case Response.PROXY_AUTHENTICATION_REQUIRED:
                if (sLogger.isActivated()) {
                    sLogger.info("407 response received");
                }

                /* Set the Proxy-Authorization header */
                authenticationAgent.readProxyAuthenticateHeader(ctx.getSipResponse());

                /* Increment the Cseq number of the dialog path */
                dialogPath.incrementCseq();

                /* Create a second MESSAGE request with the right token */
                if (sLogger.isActivated()) {
                    sLogger.info("Send second MESSAGE");
                }
                SipRequest msg = null;
                try{
                msg = SipMessageFactory.createMessage(dialogPath, imm.getFeatureTag(),
                        imm.getContentType(), imm.getContent());
                }catch(SipException e){
                	//TODO TAPI 1.6
                }

                /* Set the Authorization header */
                try{
                authenticationAgent.setProxyAuthorizationHeader(msg);
                }catch(CoreException e){
                	
                }
                try{
                ctx = mSipService.getImsModule().getSipManager().sendSipMessageAndWait(msg);
                }catch(SipException e){
                	//TODO TAPI 1.6
                }
        
                analyzeSipResponse(ctx, authenticationAgent, dialogPath, imm);
               
                break;

            case Response.OK:
            case Response.ACCEPTED:
                if (sLogger.isActivated()) {
                    sLogger.info("20x OK response received");
                }
                break;
            default:
                throw new NetworkException("Instant multimedia message has failed: " + statusCode
                        + " response received");
        }
    }

    private void sendSipMessage(InstantMultimediaMessage imm, String remoteInstanceId)
            throws PayloadException, NetworkException {
        try {
            if (sLogger.isActivated()) {
                sLogger.debug("Send instant multimedia message");
            }
            ImsModule imsModule = mSipService.getImsModule();
            // Create authentication agent
            SessionAuthenticationAgent authenticationAgent = new SessionAuthenticationAgent(
                    imsModule);
            // @FIXME: This should be an URI instead of String
            String toUri = PhoneUtils.formatContactIdToUri(imm.getRemote()).toString();
            SipManager sipManager = imsModule.getSipManager();
            SipInterface sipInterface = sipManager.getSipStack();
            // Create a dialog path
            SipDialogPath dialogPath = new SipDialogPath(  
            		imsModule.getSipManager().getSipStack(),
                    sipInterface.generateCallId(), 
                    1, 
                    toUri, 
                    ImsModule.IMS_USER_PROFILE.getPublicUri(), 
                    toUri,
                    sipInterface.getServiceRoutePath());
            dialogPath.setRemoteSipInstance(remoteInstanceId);

            // Create MESSAGE request
            if (sLogger.isActivated()) {
                sLogger.info("Send first MESSAGE");
            }
            try{
            SipRequest msg = SipMessageFactory.createMessage(dialogPath, imm.getFeatureTag(),
                    imm.getContentType(), imm.getContent());
            SipTransactionContext ctx = sipManager.sendSipMessageAndWait(msg);
            analyzeSipResponse(ctx, authenticationAgent, dialogPath, imm);
            }catch(SipException e){
            	
            }

        } catch (InvalidArgumentException | ParseException e) {
            throw new PayloadException("Unable to set authorization header for remoteInstanceId: "
                    + remoteInstanceId, e);
        }
    }

    /**
     * Instant multimedia message
     */
    private static class InstantMultimediaMessage {
        private final ContactId mRemote;
        private final String mFeatureTag;
        private final byte[] mContent;
        private final String mContentType;

        public InstantMultimediaMessage(ContactId remote, String featureTag, byte[] content,
                String contentType) {
            mRemote = remote;
            mFeatureTag = featureTag;
            mContent = content;
            mContentType = contentType;
        }

        public ContactId getRemote() {
            return mRemote;
        }

        public String getFeatureTag() {
            return mFeatureTag;
        }

        public byte[] getContent() {
            return mContent;
        }

        public String getContentType() {
            return mContentType;
        }
    }
}
