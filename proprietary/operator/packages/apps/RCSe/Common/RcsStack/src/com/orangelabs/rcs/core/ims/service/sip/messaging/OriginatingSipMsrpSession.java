/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2015 Sony Mobile Communications Inc.
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

package com.orangelabs.rcs.core.ims.service.sip.messaging;

import com.gsma.services.rcs.contact.ContactId;


import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.FileAccessException;
import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.orangelabs.rcs.core.ims.service.sip.SipService;
import com.orangelabs.rcs.core.ims.service.sip.SipSessionError;
import com.orangelabs.rcs.core.ims.service.sip.SipSessionListener;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.ims.service.sip.messaging.GenericSipMsrpSession;
import com.orangelabs.rcs.core.CoreException;

import java.text.ParseException;

import javax2.sip.InvalidArgumentException;

/**
 * Originating SIP MSRP session
 * 
 * @author jexa7410
 */
public class OriginatingSipMsrpSession extends GenericSipMsrpSession {

    private static final Logger sLogger = Logger.getLogger(OriginatingSipMsrpSession.class
            .getSimpleName());

    private final String[] mAcceptTypes;

    private final String[] mAcceptWrappedTypes;

    /**
     * Constructor
     * 
     * @param parent SIP service
     * @param contact Remote contact Id
     * @param featureTag Feature tag
     * @param timestamp Local timestamp for the session
     * @param acceptTypes Accept-types related to exchanged messages
     * @param acceptWrappedTypes Accept-wrapped-types related to exchanged messages
     */
    public OriginatingSipMsrpSession(SipService parent, ContactId contact, String featureTag,
            String[] acceptTypes, String[] acceptWrappedTypes)  throws CoreException {
        super(parent, contact, featureTag);

        mAcceptTypes = acceptTypes;
        mAcceptWrappedTypes = acceptWrappedTypes;

        createOriginatingDialogPath();
    }
    
    @Override
    public void startMediaSession() throws Exception {

    }
    
    @Override
    public void run() {
        try {
            if (sLogger.isActivated()) {
                sLogger.info("Initiate a new MSRP session as originating");
            }

            /* Set setup mode */
            String localSetup = createMobileToMobileSetupOffer();
            if (sLogger.isActivated()) {
                sLogger.debug("Local setup attribute is ".concat(localSetup));
            }

            /* Build SDP offer */
            String sdp = generateSdp(localSetup, mAcceptTypes, mAcceptWrappedTypes);

            /* Set the local SDP part in the dialog path */
            getDialogPath().setLocalContent(sdp);

            if (sLogger.isActivated()) {
                sLogger.info("Send INVITE");
            }
            SipRequest invite = createInvite();
            
            getAuthenticationAgent().setAuthorizationHeader(invite);

            /* Set initial request in the dialog path */
            getDialogPath().setInvite(invite);

            sendInvite(invite);


        } catch (Exception e) {
            handleError(new SipSessionError(SipSessionError.SESSION_INITIATION_FAILED, e));

        }
    }

    public boolean isInitiatedByRemote() {
        return false;
}

    public void handle180Ringing(SipResponse response) {
        if (sLogger.isActivated()) {
            sLogger.debug("handle180Ringing");
        }
        ContactId contact = getRemoteContactId();
        for (ImsSessionListener listener : getListeners()) {
            ((SipSessionListener) listener).onSessionRinging(contact);
        }
    }
}
