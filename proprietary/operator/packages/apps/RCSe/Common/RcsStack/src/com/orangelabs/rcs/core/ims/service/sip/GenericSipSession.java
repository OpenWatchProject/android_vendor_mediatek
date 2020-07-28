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

package com.orangelabs.rcs.core.ims.service.sip;

import com.gsma.services.rcs.contact.ContactId;

import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsServiceError;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import gov2.nist.javax2.sip.header.ims.PPreferredServiceHeader;

import java.text.ParseException;
import java.util.Set;
import com.orangelabs.rcs.core.CoreException;

import javax2.sip.header.ExtensionHeader;

/**
 * Abstract generic SIP session
 * 
 * @author jexa7410
 */
public abstract class GenericSipSession extends ImsServiceSession {

    private final String mFeatureTag;

    private final SipService mSipService;

    private static final Logger sLogger = Logger.getLogger(GenericSipSession.class.getSimpleName());

    /**
     * Constructor
     * 
     * @param parent SIP service
     * @param contact Remote contactId
     * @param featureTag Feature tag
     * @param rcsSettings RCS settings accessor
     * @param timestamp Local timestamp for the session
     * @param contactManager Contact manager accessor
     */
    public GenericSipSession(SipService parent, ContactId contact, String featureTag)throws CoreException {
        
        super(parent, contact.toString());
        mSipService = parent;
        mFeatureTag = featureTag;
    }

    /**
     * Returns feature tag of the service
     * 
     * @return Feature tag
     */
    public String getFeatureTag() {
        return mFeatureTag;
    }

    /**
     * Returns the service ID
     * 
     * @return Service ID
     */
    public String getServiceId() {
        return CapabilityUtils.extractServiceId(mFeatureTag);
    }

    /**
     * Create an INVITE request
     * 
     * @return Request
     * @throws PayloadException
     */
    public SipRequest createInvite() throws SipException {
        SipRequest invite  = null;
        try {
        String ext = FeatureTags.FEATURE_3GPP + "=\"" + FeatureTags.FEATURE_3GPP_EXTENSION + "\"";
         invite = SipMessageFactory.createInvite(getDialogPath(), new String[] {
                getFeatureTag(), ext
        }, new String[] {
                getFeatureTag(), ext, SipUtils.EXPLICIT_REQUIRE
        }, getDialogPath().getLocalContent());

            ExtensionHeader header = (ExtensionHeader) SipUtils.HEADER_FACTORY.createHeader(
                    PPreferredServiceHeader.NAME, FeatureTags.FEATURE_3GPP_SERVICE_EXTENSION);
            invite.getStackMessage().addHeader(header);

        } catch (Exception e) {
            throw new SipException(
                    "Can't add SIP headertype ".concat(FeatureTags.FEATURE_3GPP_SERVICE_EXTENSION));
        }

        return invite;
    }

    /**
     * Create 200 OK response
     * 
     * @return Response
     * @throws PayloadException
     */
    public SipResponse create200OKResponse() throws PayloadException {
        String ext = FeatureTags.FEATURE_3GPP + "=\"" + FeatureTags.FEATURE_3GPP_EXTENSION + "\"";
        return SipMessageFactory.create200OkInviteResponse(getDialogPath(), new String[] {
                getFeatureTag(), ext
        }, new String[] {
                getFeatureTag(), ext, SipUtils.EXPLICIT_REQUIRE
        }, getDialogPath().getLocalContent());
    }

    /**
     * Prepare media session
     * 
     * @throws NetworkException
     */
    public abstract void prepareMediaSession() throws NetworkException;

    /**
     * Start media transfer
     */
    public abstract void startMediaTransfer();

    /**
     * Close media session
     */
    public abstract void closeMediaSession();
    
    @Override
    public void handleError(ImsServiceError error) {
        if (isSessionInterrupted()) {
            return;
        }
        sLogger.error("Session error: " + error.getErrorCode() + ", reason=" + error.getMessage());
        closeMediaSession();
        removeSession();
         ContactId contact = getRemoteContactId();
       /* for (ImsSessionListener listener : getListeners()) {
            ((SipSessionListener) listener).onSessionError(getRemoteContact(), new SipSessionError(error));
        }*/
    }
    
    @Override
    public void receiveBye(SipRequest bye) {
        super.receiveBye(bye);
    /*    for (ImsSessionListener listener : getListeners()) {
            listener.handleSessionAborted( TERMINATION_BY_REMOTE);
        }*/
        getImsService().getImsModule().getCapabilityService().requestContactCapabilities(getRemoteContact());
    }

    @Override
    public void receiveCancel(SipRequest cancel) {
        super.receiveCancel(cancel);
        // Request capabilities to the remote
        getImsService().getImsModule().getCapabilityService()
                .requestContactCapabilities(getRemoteContact());
    }

    /**
     * Gets the IARI feature tag from the set of feature tags
     * 
     * @param featureTags The set of feature tags
     * @return the IARI feature tag or null
     */
    public static String getIariFeatureTag(Set<String> featureTags) {
        for (String tag : featureTags) {
            if (tag.startsWith(FeatureTags.FEATURE_RCSE)) {
                return tag;
            }
        }
        return null;
    }

    /**
     * Gets SIP service
     * 
     * @return SIP service
     */
    public SipService getSipService() {
        return mSipService;
    }
    /**
     * Removes the session
     */
    public abstract void removeSession();

}
