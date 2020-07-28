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
 /*
 * Copyright (C) 2018 MediaTek Inc., this file is modified on 07/05/2018
 * by MediaTek Inc. based on Apache License, Version 2.0.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. See NOTICE for more details.
 */

package com.mediatek.presence.core.ims.service.capability;

import java.util.Vector;

import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.protocol.sip.SipException;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipTransactionContext;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.SessionAuthenticationAgent;
import com.mediatek.presence.core.ims.service.presence.PresenceError;
import com.mediatek.presence.core.ims.service.presence.PublishManagerEx;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;
import java.util.List;
import java.util.ArrayList;
import com.mediatek.presence.core.ims.service.im.chat.ChatUtils;
import com.mediatek.presence.service.api.PresenceServiceImpl;

/**
 * Anonymous fetch procedure which permits to request the capabilities
 * for a given contact thanks to a one shot subscribe.
 *
 * @author Jean-Marc AUFFRET
 */
public class AnonymousFetchRequestTask {
    /**
     * IMS module
     */
    private ImsModule imsModule;

    /**
     * Remote contact
     */
    private String contact;

    /**
     * Remote contact
     */
    private List<String> contacts;

    /**
     * Dialog path
     */
    private SipDialogPath dialogPath = null;

    /**
     * Authentication agent
     */
    private SessionAuthenticationAgent authenticationAgent;

    /**
     * The logger
     */
    private static Logger logger = null;

    private int mSlotId = 0;

    /**
     * RequestId which shall be sent to client when receiveing SIP response (AOSP procedure)
     */
    private int mRequestId = -1;

    /**
     * Constructor
     *
     * @param parent IMS module
     * @param contact Remote contact
     */
    public AnonymousFetchRequestTask(ImsModule parent, String contact) {
        this.imsModule = parent;
        this.contact = contact;

        this.contacts = new ArrayList<String>();
        contacts.add(contact);

        this.authenticationAgent = new SessionAuthenticationAgent(imsModule);
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "AnonymousFetchRequestTask");
    }

    /**
     * Constructor (AOSP procedure)
     *
     * @param parent IMS module
     * @param contact Remote contact
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public AnonymousFetchRequestTask(ImsModule parent, String contact, int requestId) {
        this.imsModule = parent;
        this.contact = contact;
        this.mRequestId = requestId;
        this.contacts = new ArrayList<String>();
        contacts.add(contact);
        this.authenticationAgent = new SessionAuthenticationAgent(imsModule);
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "AnonymousFetchRequestTask");
    }

    /**
     * Constructor
     *
     * @param parent IMS module
     * @param contact Remote contact
     */
    public AnonymousFetchRequestTask(ImsModule parent, List<String> contacts) {
        this.imsModule = parent;
        this.contacts = contacts;
        this.authenticationAgent = new SessionAuthenticationAgent(imsModule);
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "AnonymousFetchRequestTask");
    }

    /**
     * Constructor (AOSP procedure)
     *
     * @param parent IMS module
     * @param contacts Remote contacts
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public AnonymousFetchRequestTask(ImsModule parent, List<String> contacts, int requestId) {
        this.imsModule = parent;
        this.contact = contact;
        this.mRequestId = requestId;
        this.authenticationAgent = new SessionAuthenticationAgent(imsModule);
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "AnonymousFetchRequestTask");
    }

    /**
     * Start task
     */
    public void start() {
        if (contacts != null) {
            if (SimUtils.isTmoSimCard(mSlotId)) {
                sendTmoSubscribe();
            } else {
                sendSubscribe();
            }
        }
    }

    /**
     * Send a SUBSCRIBE request
     */
    private void sendTmoSubscribe() {
        if (logger.isActivated()) {
            logger.info("sendTmoSubscribe");
            logger.info("Send SUBSCRIBE request to " + contacts);
        }

        try {
            // Set target
            String contactUri = ImsModule.IMS_USER_PROFILE.getPublicUri();

            // Set Call-Id
            String callId = imsModule.getSipManager().getSipStack().generateCallId();

            // Set target
            String target = contactUri;

            // Set local party
            String localParty = "sip:anonymous@anonymous.invalid";

            // Set remote party
            String remoteParty = contactUri;

            // Set the route path
            Vector<String> route = imsModule.getSipManager().getSipStack().getServiceRoutePath();

            // Create a dialog path
            dialogPath = new SipDialogPath(
                    imsModule.getSipManager().getSipStack(),
                    callId,
                    1,
                    target,
                    localParty,
                    remoteParty,
                    route);

            // Create a SUBSCRIBE request
            SipRequest subscribe = createSubscribe();

            // Send SUBSCRIBE request
            sendSubscribe(subscribe);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Subscribe has failed", e);
            }
            handleError(new PresenceError(PresenceError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }
    }

    /**
     * Send a SUBSCRIBE request
     */
    private void sendSubscribe() {
        if (logger.isActivated()) {
            logger.info("Send SUBSCRIBE request to " + contact);
            logger.info("Send SUBSCRIBE request to " + contacts);
        }

        if (SimUtils.isAttSimCard(mSlotId)) {
            if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .is489BadEventState())
                return;
        }

        try {
            // Create a dialog path
            String contactUri = null;
            if (contacts != null && contacts.size() > 1) {
                contactUri = ImsModule.IMS_USER_PROFILE.getPublicUri();
            } else if (contacts != null && contacts.size() == 1) {
                if (SimUtils.isAttSimCard(mSlotId)) {
                    contactUri = PhoneUtils.formatNumberToTelUri(contacts.get(0));
                } else {
                    contactUri = PhoneUtils.formatNumberToSipUri(contacts.get(0));
                }
            } else if (contact != null) {
                if (SimUtils.isAttSimCard(mSlotId)) {
                    contactUri = PhoneUtils.formatNumberToTelUri(contact);
                } else {
                    contactUri = PhoneUtils.formatNumberToSipUri(contact);
                }
            }
            logger.info("Send SUBSCRIBE contactUri=" + contactUri);

            // Set Call-Id
            String callId = imsModule.getSipManager().getSipStack().generateCallId();

            // Set target
            String target = contactUri;

            String localParty;
            if (SimUtils.isAttSimCard(mSlotId)) {
                localParty = ImsModule.IMS_USER_PROFILE.getPublicUri();
            } else {
            // Set local party
                localParty = "sip:anonymous@" + ImsModule.IMS_USER_PROFILE.getHomeDomain();
            }

            // Set remote party
            String remoteParty = contactUri;

            // Set the route path
            Vector<String> route = imsModule.getSipManager().getSipStack().getServiceRoutePath();

            // Create a dialog path
            dialogPath = new SipDialogPath(
                    imsModule.getSipManager().getSipStack(),
                    callId,
                    1,
                    target,
                    localParty,
                    remoteParty,
                    route);

            // Create a SUBSCRIBE request
            SipRequest subscribe = createSubscribe();

            // Send SUBSCRIBE request
            sendSubscribe(subscribe);
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Subscribe has failed", e);
            }
            handleError(new PresenceError(PresenceError.UNEXPECTED_EXCEPTION, e.getMessage()));
        }
    }

    /**
     * Create a SUBSCRIBE request
     *
     * @return SIP request
     * @throws SipException
     * @throws CoreException
     */
    private SipRequest createSubscribe() throws SipException, CoreException {
        if (logger.isActivated()) {
            logger.info("createSubscribe, contacts=" + contacts +", contact=" + contact);
        }
        SipRequest subscribe = null;
        if (contacts != null && contacts.size() > 1) {
            logger.info("createSubscribe - contacts != null && contacts.size() > 1");
            String xml = ChatUtils.generateSubscribeResourceList(contacts);
            subscribe = SipMessageFactory.createPresenceSubscribe(mSlotId, dialogPath, 3600, xml);
        } else if (contacts != null && contacts.size() == 1) {
            logger.info("createSubscribe - contacts != null && contacts.size() == 1");
            subscribe = SipMessageFactory.createIndividualPresenceSubscribe(mSlotId, dialogPath, 3600, contacts.get(0));
        } else if (contact != null) {
            logger.info("createSubscribe - contact != null");
            subscribe = SipMessageFactory.createIndividualPresenceSubscribe(mSlotId, dialogPath, 3600, contact);
        }


        return subscribe;
    }

    /**
     * Send SUBSCRIBE message
     *
     * @param subscribe SIP SUBSCRIBE
     * @throws Exception
     */
    private void sendSubscribe(SipRequest subscribe) throws Exception {
        if (logger.isActivated()) {
            logger.info("Send SUBSCRIBE, expire=" + subscribe.getExpires());
        }

        if(authenticationAgent != null){
            authenticationAgent.setAuthorizationHeader(subscribe);
        }

        // Send SUBSCRIBE request
        SipTransactionContext ctx = imsModule.getSipManager().sendSipMessageAndWait(subscribe);
        if (mRequestId != -1) {
            logger.info("Notify the SipResponse with requestId = " + String.valueOf(mRequestId));
            PresenceServiceImpl.receiveSipResponse(mRequestId, ctx.getSipResponse());
        }

        // Analyze the received response
        if (ctx.isSipResponse()) {
            // A response has been received
            if ((ctx.getStatusCode() >= 200) && (ctx.getStatusCode() < 300)) {
                // 200 OK
                handle200OK(ctx);
            } else
            if (ctx.getStatusCode() == 407) {
                // 407 Proxy Authentication Required
                handle407Authentication(ctx);
            } else
            if (ctx.getStatusCode() == 404) {
                // User not found
                handleUserNotFound(ctx);
            } else
            if (ctx.getStatusCode() == 489) {
                handle489BadEvent(ctx);
            } else {
                // Other error response
                handleError(new PresenceError(PresenceError.SUBSCRIBE_FAILED,
                        ctx.getStatusCode() + " " + ctx.getReasonPhrase()));
            }
        } else {
            if (logger.isActivated()) {
                logger.debug("No response received for SUBSCRIBE");
            }

            // No response received: timeout
            handleError(new PresenceError(PresenceError.SUBSCRIBE_FAILED));
        }
    }

    /**
     * Handle 200 0K response
     *
     * @param ctx SIP transaction context
     */
    private void handle200OK(SipTransactionContext ctx) {
        // 200 OK response received
        if (logger.isActivated()) {
            logger.info("200 OK response received");
        }
    }

    /**
     * Handle 407 response
     *
     * @param ctx SIP transaction context
     * @throws Exception
     */
    private void handle407Authentication(SipTransactionContext ctx) throws Exception {
        // 407 response received
        if (logger.isActivated()) {
            logger.info("407 response received");
        }

        SipResponse resp = ctx.getSipResponse();

        // Set the Proxy-Authorization header
        authenticationAgent.readProxyAuthenticateHeader(resp);

        // Increment the Cseq number of the dialog path
        dialogPath.incrementCseq();

        // Create a second SUBSCRIBE request with the right token
        if (logger.isActivated()) {
            logger.info("Send second SUBSCRIBE");
        }
        SipRequest subscribe = createSubscribe();

        // Set the Authorization header
        authenticationAgent.setProxyAuthorizationHeader(subscribe);

        // Send SUBSCRIBE request
        sendSubscribe(subscribe);
    }

    protected void handle489BadEvent(SipTransactionContext ctx) {
        //
        if (!RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .is489BadEventState()) {
            Intent intent = new Intent(PublishManagerEx.SIP_489_BAD_EVENT_STATE);
            intent.putExtra("slotId", mSlotId);
            LocalBroadcastManager.getInstance(AndroidFactory.getApplicationContext())
                    .sendBroadcast(intent);
        }
        handleError(new PresenceError(PresenceError.SUBSCRIBE_FAILED,
                ctx.getStatusCode() + " " + ctx.getReasonPhrase()));
        return;
    }

    /**
     * Handle error response
     *
     * @param error Error
     */
    private void handleError(PresenceError error) {
        // On error don't modify the existing capabilities
        if (logger.isActivated()) {
            logger.info("Subscribe has failed: " + error.getErrorCode() + ", reason=" + error.getMessage());
        }

        //commcnted temp
        // We update the database capabilities timestamp
        //ContactsManager.getInstance().setContactCapabilitiesTimestamp(contact, System.currentTimeMillis());
    }

    /**
     * Handle user not found
     *
     * @param ctx SIP transaction context
     */
    private void handleUserNotFound(SipTransactionContext ctx) {
        if (logger.isActivated()) {
            logger.info("User not found (" + ctx.getStatusCode() + " error)");
        }

        // We update the database with empty capabilities
        Capabilities capabilities = new Capabilities();
        ContactsManager.getInstance().setContactCapabilities(contact, capabilities, ContactInfo.NOT_RCS, ContactInfo.REGISTRATION_STATUS_UNKNOWN);
    }
}
