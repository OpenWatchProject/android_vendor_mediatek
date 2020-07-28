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

package com.mediatek.presence.core.ims.service;

import java.util.Enumeration;

import com.gsma.services.rcs.chat.ChatIntent;
import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.ChatMessage;

import javax2.sip.address.SipURI;
import javax2.sip.header.ContactHeader;
import javax2.sip.header.EventHeader;
import javax2.sip.message.Request;
import java.lang.Runnable;

import android.content.Intent;

import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.CoreListener;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.ImsNetworkInterface;
import com.mediatek.presence.core.ims.network.sip.FeatureTags;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipException;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.service.capability.CapabilityService;
import com.mediatek.presence.core.ims.service.capability.OptionsManager;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.FifoBuffer;
import com.mediatek.presence.utils.IdGenerator;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;

/**
 * IMS service dispatcher
 *
 * @author jexa7410
 */
public class ImsServiceDispatcher implements Runnable {
    /**
     * IMS module
     */
    private ImsModule imsModule;

    /**
     * Buffer of messages
     */
    private FifoBuffer buffer = new FifoBuffer();

    /**
     * SIP intent manager
     */
    private SipIntentManager intentMgr = new SipIntentManager();

    private volatile boolean active = false;

    /**
     * The logger
     */
    private Logger logger = null;

    private int mSlotId = 0;

    /**
     * Constructor
     *
     * @param imsModule IMS module
     */
    public ImsServiceDispatcher(ImsModule imsModule) {
        this.imsModule = imsModule;
        mSlotId = imsModule.getSlotId();
        logger = Logger.getLogger(mSlotId, "ImsServiceDispatcher");
    }

    /**
     * Terminate the SIP dispatcher
     */
    public void terminate() {
        if (logger.isActivated()) {
            logger.info("Terminate the multi-session manager");
        }
        active = false;
        buffer.close();
        if (logger.isActivated()) {
            logger.info("Multi-session manager has been terminated");
        }
    }

    /**
     * Post a SIP request in the buffer
     *
     * @param request SIP request
     */
    public void postSipRequest(SipRequest request) {
        buffer.addObject(request);
    }

    /**
     * Background processing
     */
    public void run() {
        if (logger.isActivated()) {
            logger.info("Start background processing");
        }
        //add a flag break the loop at running state
        //original flow just for sleep state.
        active = true;
        SipRequest request = null;
        while(true) {
            try {
                if ((request = (SipRequest)buffer.getObject()) != null) {
                // Dispatch the received SIP request
                dispatch(request);
                } else if (!active) {
                    break;
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Unexpected exception", e);
                }
            }
        }
        if (logger.isActivated()) {
            logger.info("End of background processing");
        }
    }

    /**
     * @}
     */

    /**
     * Dispatch the received SIP request
     *
     * @param request SIP request
     */
    private void dispatch(SipRequest request) {
        if (logger.isActivated()) {
            logger.debug("Receive " + request.getMethod() + " request");
        }

        // Check the IP address of the request-URI
        String localIpAddress = imsModule.getCurrentNetworkInterface().getNetworkAccess().getIpAddress();
        ImsNetworkInterface imsNetIntf = imsModule.getCurrentNetworkInterface();
        boolean isMatchingRegistered = false;
        SipURI requestURI;
        try {
            requestURI = SipUtils.ADDR_FACTORY.createSipURI(request.getRequestURI());
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unable to parse request URI " + request.getRequestURI(), e);
            }
            sendFinalResponse(request, 400);
            return;
        }

        // First check if the request URI matches with the local interface address
        isMatchingRegistered = localIpAddress.equals(requestURI.getHost());

        // If no matching, perhaps we are behind a NAT
        if ((!isMatchingRegistered) && imsNetIntf.isBehindNat()) {
            // We are behind NAT: check if the request URI contains the previously
            // discovered public IP address and port number
            String natPublicIpAddress = imsNetIntf.getNatPublicAddress();
            int natPublicUdpPort = imsNetIntf.getNatPublicPort();
            if ((natPublicUdpPort != -1) && (natPublicIpAddress != null)) {
                isMatchingRegistered = natPublicIpAddress.equals(requestURI.getHost()) && (natPublicUdpPort == requestURI.getPort());
            } else {
                // NAT traversal and unknown public address/port
                isMatchingRegistered = false;
            }
        }

        if (!isMatchingRegistered) {
            // Send a 404 error
            if (logger.isActivated()) {
                logger.debug("Request-URI address and port do not match with registered contact: reject the request");
            }
            sendFinalResponse(request, 404);
            return;
        }

        // Check SIP instance ID: RCS client supporting the multidevice procedure shall respond to the
        // invite with a 486 BUSY HERE if the identifier value of the "+sip.instance" tag included
        // in the Accept-Contact header of that incoming SIP request does not match theirs
        String instanceId = SipUtils.getInstanceID(request);
        if (logger.isActivated()) {
            logger.debug("SIP instanceid : " + instanceId);

            if(imsModule.getSipManager().getSipStack() != null){
                logger.debug("SIP instance3 : " + imsModule.getSipManager().getSipStack().getInstanceId());
            }
        }

        if ((instanceId != null) && imsModule.getSipManager().getSipStack().getInstanceId() != null
                && !instanceId.contains(imsModule.getSipManager().getSipStack().getInstanceId())) {
            // Send 486 Busy Here
            if (logger.isActivated()) {
                logger.debug("SIP instance ID doesn't match: reject the request");
            }
            sendFinalResponse(request, 486);
            return;
        }

        // Check public GRUU : RCS client supporting the multidevice procedure shall respond to the
        // invite with a 486 BUSY HERE if the identifier value of the "pub-gruu" tag included
        // in the Accept-Contact header of that incoming SIP request does not match theirs
        String publicGruu = SipUtils.getPublicGruu(request);
        if ((publicGruu != null) && imsModule.getSipManager().getSipStack().getPublicGruu() != null
                && !publicGruu.contains(imsModule.getSipManager().getSipStack().getPublicGruu())) {
            // Send 486 Busy Here
            if (logger.isActivated()) {
                logger.debug("SIP public-gruu doesn't match: reject the request");
            }
            sendFinalResponse(request, 486);
            return;
        }

        // Update remote SIP instance ID in the dialog path of the session
        ImsServiceSession session = searchSession(request.getCallId());
        if (session != null) {
            ContactHeader contactHeader = (ContactHeader)request.getHeader(ContactHeader.NAME);
            if (contactHeader != null) {
                String remoteInstanceId = contactHeader.getParameter(SipUtils.SIP_INSTANCE_PARAM);
                session.getDialogPath().setRemoteSipInstance(remoteInstanceId);
            }
        }

        if (request.getMethod().equals(Request.OPTIONS)) {
            imsModule.getCapabilityService().receiveCapabilityRequest(request);
        } else if (request.getMethod().equals(Request.INVITE)) {
            logger.info("INVITE received");
        } else if (request.getMethod().equals(Request.MESSAGE)) {

             if (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_RCSE_PAGER_MSG) ||
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_PUBLIC_ACCOUNT) ||
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_EMOTICON) ||
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_CLOUD_FILE)) {
                //Pager Message
                //imsModule.getInstantMessagingService().receivePagerModeMessage(request);
            } else {
                Intent intent = intentMgr.isSipRequestResolved(request);
                if (intent != null) {
                    // Generic SIP instant message
                    if (logger.isActivated()) {
                        logger.debug("Generic instant message");
                    }
                    //imsModule.getSipService().receiveInstantMessage(intent, request);
                } else {
                    // Unknown service: reject the message with a 606 Not
                    // Acceptable
                    if (logger.isActivated()) {
                        logger.debug("Unknown IMS service: automatically reject");
                    }
                    sendFinalResponse(request, 606);
                }
            }
        } else if (request.getMethod().equals(Request.NOTIFY)) {
            // NOTIFY received
            dispatchNotify(request);
        } else if (request.getMethod().equals(Request.BYE)) {
            logger.info("BYE received");
        } else if (request.getMethod().equals(Request.CANCEL)) {
            // CANCEL received
            if (logger.isActivated()) {
                logger.info("Receive Cancel from server");
            }
            final SipRequest newRequest = request;
            final ImsServiceSession newSession = session;

            Thread t = new Thread() {
                public void run() {
                    if (logger.isActivated()) {
                        logger.info("Receive Cancel1 from server");
                    }
                    // Route request to session
                    if (newSession != null) {
                        newSession.receiveCancel(newRequest);
            }

            // Send a 200 OK
            try {
                if (logger.isActivated()) {
                    logger.info("Send 200 OK");
                }
                SipResponse cancelResp = SipMessageFactory.createResponse(newRequest, 200);
                imsModule.getSipManager().sendSipResponse(cancelResp);
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't send 200 OK response", e);
                }
            }
                }
            };
            t.start();
        } else if (request.getMethod().equals(Request.UPDATE)) {
            // UPDATE received
            if (session != null) {
                session.receiveUpdate(request);
            }
        } else {
            // Unknown request received
            if (logger.isActivated()) {
                logger.debug("Unknown request " + request.getMethod());
            }
        }
    }

    /**
     * Dispatch the received SIP NOTIFY
     *
     * @param notify SIP request
     */
    private void dispatchNotify(SipRequest notify) {
        try {
            // Create 200 OK response
            SipResponse resp = SipMessageFactory.createResponse(notify, 200);

            // Send 200 OK response
            imsModule.getSipManager().sendSipResponse(resp);
        } catch(SipException e) {
            if (logger.isActivated()) {
                logger.error("Can't send 200 OK for NOTIFY", e);
            }
        }

        // Get the event type
        EventHeader eventHeader = (EventHeader)notify.getHeader(EventHeader.NAME);
        if (eventHeader == null) {
            if (logger.isActivated()) {
                logger.debug("Unknown notification event type");
            }
            return;
        }

        int slotId = imsModule.getSlotId();
        // Dispatch the notification to the corresponding service
        if (eventHeader.getEventType().equalsIgnoreCase("presence.winfo")) {
            // Presence service
            if (RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .isSocialPresenceSupported() && imsModule.getPresenceService().isServiceStarted()) {
                imsModule.getPresenceService().getWatcherInfoSubscriber().receiveNotification(notify);
            }
        } else if (eventHeader.getEventType().equalsIgnoreCase("presence")) {
            // Fix open source issue
            // Send to AnonymousFetchManager to handle NOTIFY even "to" field is not send to anonymous
            if (notify.getTo().indexOf("anonymous") != -1 || SimUtils.isAttSimCard(slotId) ||
                !imsModule.getPresenceService().isPresenceSubscribed()) {
                // Capability service
                imsModule.getCapabilityService().receiveNotification(notify);
            } else {
                // Presence service
                imsModule.getPresenceService().getPresenceSubscriber().receiveNotification(notify);
            }
        } else
            if (!SubscriptionManager.getInstance().receiveNotification(notify)) {
                // Not supported service
                if (logger.isActivated()) {
                    logger.debug("Not supported notification event type");
                }
            }
        //}
    }

    /**
     * Test a tag is present or not in SIP message
     *
     * @param message Message or message part
     * @param tag Tag to be searched
     * @return Boolean
     */
    private boolean isTagPresent(String message, String tag) {
        if ((message != null) && (tag != null) && (message.toLowerCase().indexOf(tag) != -1)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Search the IMS session that corresponds to a given call-ID
     *
     * @param callId Call-ID
     * @return IMS session
     */
    private ImsServiceSession searchSession(String callId) {
        if (callId == null) {
            return null;
        }
        ImsService[] list = imsModule.getImsServices();
        for(int i=0; i< list.length; i++) {
            if (list[i] != null) {
                for(Enumeration<ImsServiceSession> e = list[i].getSessions() ; e.hasMoreElements();) {
                    ImsServiceSession session = (ImsServiceSession)e.nextElement();

                    if ((session != null) && session.getDialogPath().getCallId().equals(callId)) {
                        return session;
                    }
                }
            }
        }
        return null;
    }


    /**
     * Send a 100 Trying response to the remote party
     *
     * @param request SIP request
     */
    private void send100Trying(SipRequest request) {
        try {
            // Send a 100 Trying response
            SipResponse trying = SipMessageFactory.createResponse(request, null, 100);
            imsModule.getCurrentNetworkInterface().getSipManager().sendSipResponse(trying);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send a 100 Trying response");
            }
        }
    }

    /**
     * Send a final response
     *
     * @param request SIP request
     * @param code Response code
     */
    private void sendFinalResponse(SipRequest request, int code) {
        try {
            SipResponse resp = SipMessageFactory.createResponse(request, IdGenerator.getIdentifier(), code);
            imsModule.getCurrentNetworkInterface().getSipManager().sendSipResponse(resp);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send a " + code + " response");
            }
        }
    }
}
