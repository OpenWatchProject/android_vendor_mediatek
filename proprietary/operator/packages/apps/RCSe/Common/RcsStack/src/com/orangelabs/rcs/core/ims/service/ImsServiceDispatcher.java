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

package com.orangelabs.rcs.core.ims.service;

import java.util.Date;
import java.util.Enumeration;

import com.gsma.services.rcs.chat.ChatIntent;
import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.ChatMessage;

import javax2.sip.address.SipURI;
import javax2.sip.header.ContactHeader;
import javax2.sip.header.EventHeader;
import javax2.sip.message.Request;

import android.content.Intent;
import android.text.TextUtils;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.CoreListener;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.ImsNetworkInterface;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityService;
import com.orangelabs.rcs.core.ims.service.capability.OptionsManager;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimParser;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnManager;
import com.orangelabs.rcs.core.ims.service.im.chat.standfw.StoreAndForwardManager;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.FileTransferHttpInfoDocument;
import com.orangelabs.rcs.core.ims.service.terms.TermsConditionsService;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.FifoBuffer;
import com.orangelabs.rcs.utils.IdGenerator;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * IMS service dispatcher
 *
 * @author jexa7410
 */
public class ImsServiceDispatcher extends Thread {
    /**
     * IMS module
     */
    private ImsModule imsModule;

    /**
     * Buffer of messages
     */
    private FifoBuffer buffer = new FifoBuffer();


    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     *
     * @param imsModule IMS module
     */
    public ImsServiceDispatcher(ImsModule imsModule) {
        super("SipDispatcher");

        this.imsModule = imsModule;
    }

    /**
     * Terminate the SIP dispatcher
     */
    public void terminate() {
        if (logger.isActivated()) {
            logger.info("Terminate the multi-session manager");
        }
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
        SipRequest request = null;
        while((request = (SipRequest)buffer.getObject()) != null) {
            try {
                // Dispatch the received SIP request
                dispatch(request);
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
     * M: Added to resolve the SIP OPTION timeout issue.@{
     *
     * @param needRequest Weather need request the capability
     * @param contact The contact to request the capability
     */
    private void requestCapability(boolean needRequest,String contact) {
        if (!needRequest) {
            if (logger.isActivated()) {
                logger.info("needRequest: " + needRequest);
            }
            return;
        }
        Core coreInstance = Core.getInstance();
        if (coreInstance == null && logger.isActivated()) {
            logger.info("coreInstance is null");
            return;
        }

        if (CoreListener.OFFLINE_CONTACTS.contains(contact)) {
            if (logger.isActivated()) {
                logger.info("CoreListener.OFFLINE_CONTACTS contains: " + contact);
            }
            CapabilityService capabilityService = coreInstance.getCapabilityService();
            if (capabilityService == null && logger.isActivated()) {
                logger.info("capabilityService is null");
                return;
            }

            OptionsManager optionsManager = capabilityService.getOptionsManager();
            if (optionsManager == null && logger.isActivated()) {
                logger.info("optionsManager is null");
                return;
            }

            optionsManager.requestCapabilities(contact);
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

        /*
         * Request URI received from OP01 commercial network might formed like Public URI
         * which means there is no IP address part in it, thus we can't filter out requests not
         * toward to us by comparing with local address.
         * Just workaround this.
         */
        if (RcsSettings.getInstance().supportOP01())
            isMatchingRegistered = true;
        else {
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
            if (!isMatchingRegistered) {
                logger.debug("localAddress:" + localIpAddress + ":" + requestURI.getHost());
            }

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
            // OPTIONS received
            if (imsModule.getCallManager().isCallConnected()) {
                // Rich call service
               // imsModule.getRichcallService().receiveCapabilityRequest(request); //since we didn't intialize RichCallService
            } else {
                // Capability discovery service
                imsModule.getCapabilityService().receiveCapabilityRequest(request);
            }
        } else
        if (request.getMethod().equals(Request.INVITE)) {
            // INVITE received
            if (session != null) {
                // Subsequent request received
                session.receiveReInvite(request);
                return;
            }

            // Send a 100 Trying response
            if(!RcsSettings.getInstance().singleRegistrationSupported()){
                send100Trying(request);
                if (logger.isActivated()) {
                    logger.debug("send100Trying ");
                }
            }

            // Extract the SDP part
            String sdp = request.getSdpContent();
            if (sdp == null) {
                // No SDP found: reject the invitation with a 606 Not Acceptable
                if (logger.isActivated()) {
                    logger.debug("No SDP found: automatically reject");
                }
                sendFinalResponse(request, 606);
                return;
            }
            sdp = sdp.toLowerCase();

            // New incoming session invitation
            if (isTagPresent(sdp, "msrp") &&
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_3GPP_VIDEO_SHARE) &&
                        (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_3GPP_IMAGE_SHARE) ||
                                SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_3GPP_IMAGE_SHARE_RCS2))) {
                // Image sharing
                if (RcsSettings.getInstance().isImageSharingSupported()) {
                    if (logger.isActivated()) {
                        logger.debug("Image content sharing transfer invitation");
                    }
                    //imsModule.getRichcallService().receiveImageSharingInvitation(request); //since we didn't intialize RichCallService
                } else {
                    // Service not supported: reject the invitation with a 603 Decline
                    if (logger.isActivated()) {
                        logger.debug("Image share service not supported: automatically reject");
                    }
                    sendFinalResponse(request, 603);
                }
            } else
            if (isTagPresent(sdp, "msrp") &&
                (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_OMA_IM) ||
                 SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CPM_SESSION) ||
                 SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CPM_FT) ||
                 SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CPM_LARGE_MSG)) &&
                isTagPresent(sdp, "file-selector")) {

                if (logger.isActivated()) {
                    logger.debug("File transfer invitation0");
                }
                // File transfer
                if (RcsSettings.getInstance().isFileTransferSupported()) {
                    if (logger.isActivated()) {
                        logger.debug("File transfer invitation");
                    }
                    if (ChatUtils.isGroupChatInvitation(request)) {
                        imsModule.getInstantMessagingService().receiveGroupFileTransferInvitation(request);
                    }
                    else{
                    imsModule.getInstantMessagingService().receiveFileTransferInvitation(request);
                    }
                } else {
                    // Service not supported: reject the invitation with a 603 Decline
                    if (logger.isActivated()) {
                        logger.debug("File transfer service not supported: automatically reject");
                    }
                    sendFinalResponse(request, 603);
                }
            } else
            if (isTagPresent(sdp, "msrp") &&
                (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_OMA_IM) ||
                 SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CPM_SESSION)) &&
                !SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CPM_LARGE_MSG)) {
                // IM service
                if (logger.isActivated()) {
                    logger.debug("CPM or IM supported");
                }
                if (!RcsSettings.getInstance().isImSessionSupported()) {
                    // Service not supported: reject the invitation with a 603 Decline
                    if (logger.isActivated()) {
                        logger.debug("IM service not supported: automatically reject");
                    }
                    sendFinalResponse(request, 603);
                    return;
                }

                if (ChatUtils.isFileTransferOverHttp(request)) {
                    FileTransferHttpInfoDocument ftHttpInfo = ChatUtils.getHttpFTInfo(request);
                    if (ftHttpInfo != null) {
                        // HTTP file transfer invitation
                        if (SipUtils.getReferredByHeader(request) != null) {
                            if (logger.isActivated()) {
                                logger.debug("Single S&F file transfer over HTTP invitation");
                            }
                            imsModule.getInstantMessagingService().receiveStoredAndForwardHttpFileTranferInvitation(request, ftHttpInfo);
                        } else {
                            if (logger.isActivated()) {
                                logger.debug("Single file transfer over HTTP invitation");
                            }
                            imsModule.getInstantMessagingService().receiveHttpFileTranferInvitation(request, ftHttpInfo);
                        }
                    } else {
                        // TODO : else return error to Originating side
                        // Malformed xml for FToHTTP: automatically reject with a 606 Not Acceptable
                        if (logger.isActivated()) {
                            logger.debug("Malformed xml for FToHTTP: automatically reject");
                        }
                        sendFinalResponse(request, 606);
                    }
                } else {
                    if (SipUtils.getAssertedIdentity(request).contains(StoreAndForwardManager.SERVICE_URI) &&
                        (!request.getContentType().contains("multipart"))) {
                        // Store & Forward push notifs session
                        if (logger.isActivated()) {
                            logger.debug("Store & Forward push notifications");
                        }
                        imsModule.getInstantMessagingService().receiveStoredAndForwardPushNotifications(request);
                    } else
                    if (ChatUtils.isGroupChatInvitation(request)) {
                        // Ad-hoc group chat session
                        if (logger.isActivated()) {
                            logger.debug("Ad-hoc group chat session invitation");
                        }
                        imsModule.getInstantMessagingService().receiveAdhocGroupChatSession(request);
                    } else
                    if (SipUtils.getReferredByHeader(request) != null) {
                        // Store & Forward push messages session
                        if (logger.isActivated()) {
                            logger.debug("Store & Forward push messages session");
                        }
                         imsModule.getInstantMessagingService().receiveStoredAndForwardPushMessages(request);
                    } else {
                        // 1-1 chat session
                        if (logger.isActivated()) {
                            logger.debug("1-1 large message invitation");
                        }
                        imsModule.getInstantMessagingService().receiveOne2OneChatSession(request);
                    }
                }
            } else if (isTagPresent(sdp, "msrp") &&
                (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CPM_LARGE_MSG))) {
                // IM service
                if (logger.isActivated()) {
                    logger.debug("Large CPM or IM supported");
                }
                if (!RcsSettings.getInstance().isImSessionSupported()) {
                    // Service not supported: reject the invitation with a 603 Decline
                    if (logger.isActivated()) {
                        logger.debug("Large IM service not supported: automatically reject");
                    }
                    sendFinalResponse(request, 603);
                    return;
                }
                if (ChatUtils.isGroupChatInvitation(request)) {
                    // Ad-hoc group chat session
                    if (logger.isActivated()) {
                        logger.debug("Large Ad-hoc group chat session invitation");
                    }
                    imsModule.getInstantMessagingService().receiveAdhocGroupChatSession(request);
                } else {
                    if (logger.isActivated()) {
                        logger.debug("Large O2O Standalone chat session invitation");
                    }
                    imsModule.getInstantMessagingService().receiveStandaloneChatSession(request);
                }
            } else if (isTagPresent(sdp, "msrp") &&
                    (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_CLOUD_FILE))) {
                    if (logger.isActivated()) {
                        logger.debug("Cloud file session invitation");
                    }
                    imsModule.getInstantMessagingService().receiveStandaloneChatSession(request);
            }
            else if (isTagPresent(sdp, "rtp") &&
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_3GPP_VIDEO_SHARE)) {
                // Video streaming
                if (RcsSettings.getInstance().isVideoSharingSupported()) {
                    if (logger.isActivated()) {
                        logger.debug("Video content sharing streaming invitation");
                    }
                   // imsModule.getRichcallService().receiveVideoSharingInvitation(request);
                } else {
                    // Service not supported: reject the invitation with a 603 Decline
                    if (logger.isActivated()) {
                        logger.debug("Video share service not supported: automatically reject");
                    }
                    sendFinalResponse(request, 603);
                }
            } else
            if (isTagPresent(sdp, "msrp") &&
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_3GPP_VIDEO_SHARE) &&
                        SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH )) {
                // Geoloc sharing
                if (RcsSettings.getInstance().isGeoLocationPushSupported()) {
                    if (logger.isActivated()) {
                        logger.debug("Geoloc content sharing transfer invitation");
                    }
                    Date date = new Date();
                    //imsModule.getRichcallService().onGeolocSharingInvitationReceived(request, date.getTime());
                } else {
                    // Service not supported: reject the invitation with a 603 Decline
                    if (logger.isActivated()) {
                        logger.debug("Geoloc share service not supported: automatically reject");
                    }
                    sendFinalResponse(request, 603);
                }
            } else
            if (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_RCSE_IP_VOICE_CALL) &&
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_3GPP_IP_VOICE_CALL))    {

                    // Service not supported: reject the invitation with a 603 Decline
                    if (logger.isActivated()) {
                        logger.debug("IP Voice call service not supported: automatically reject");
                    }
                    sendFinalResponse(request, 603);

            } else
            if (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_RCSE_IP_VOICE_CALL) &&
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_3GPP_IP_VOICE_CALL) &&
                        SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL))    {

                        // Service not supported: reject the invitation with a 603 Decline
                        if (logger.isActivated()) {
                            logger.debug("IP video call service not supported: automatically reject");
                        }
                        sendFinalResponse(request, 603);

                } else {
                    // Unknown service: reject the invitation with a 606 Not Acceptable
                    if (logger.isActivated()) {
                        logger.debug("Unknown IMS service: automatically reject");
                    }
                    sendFinalResponse(request, 606);

            }
        } else if (request.getMethod().equals(Request.MESSAGE)) {
            // MESSAGE received
            if (ChatUtils.isImdnService(request)) {
                // IMDN service
                imsModule.getInstantMessagingService().receiveMessageDeliveryStatus(request);
            }else if (TermsConditionsService.isTermsRequest(request)) {
                // Terms & conditions service
                imsModule.getTermsConditionsService().receiveMessage(request);
            }else if (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CPM_MSG) ||
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_PUBLIC_ACCOUNT) ||
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_EMOTICON) ||
                    SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_CLOUD_FILE)) {
                imsModule.getInstantMessagingService().receivePagerModeMessage(request);
             }else if (SipUtils.isFeatureTagPresent(request, FeatureTags.FEATURE_CMCC_PUBLIC_ACCOUNT)) {
            	//Public Message
//	                imsModule.getInstantMessagingService().receivePagerModePublicMessage(request);
            }else if (!TextUtils.isEmpty(request.getContentType())
                    && request.getContentType().contains("application/vnd.3gpp.sms")) {
                //Digits MT Call SIP MESSAGE over internet PDN
                //Parse MO side number by P-Asserted-Id
                String sipFrom = SipUtils.getAssertedIdentity(request);
                String sipTo = request.getTo();
                sipFrom = PhoneUtils.extractNumberFromUriWithoutPlus(sipFrom);
                sipTo = PhoneUtils.extractNumberFromUriWithoutPlus(sipTo);
                if (logger.isActivated()) {
                    logger.debug("Digits MT Call SIP MESSAGE from:" + sipFrom + ", to:" + sipTo);
                }

                //Notify Telephony Fwk
                // TODO: support Digits
                //RegCore.getInstance().sendIncomingDigitsLine(sipFrom, sipTo);

                //Resopnse 200 OK
                try {
                    SipResponse response = SipMessageFactory.createResponse(request, 200);
                    imsModule.getSipManager().sendSipResponse(response);
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("Can't send 200 OK response", e);
                    }
                }
            }else {
                    // Unknown service: reject the message with a 606 Not
                    // Acceptable
                    if (logger.isActivated()) {
                        logger.debug("Unknown IMS service: automatically reject");
                    }
                    sendFinalResponse(request, 606);

            }
        } else
        if (request.getMethod().equals(Request.NOTIFY)) {
            // NOTIFY received
            dispatchNotify(request);
        } else
        if (request.getMethod().equals(Request.BYE)) {
            // BYE received

            if(session instanceof GroupChatSession){
                final SipRequest newRequest = request;
                final ImsServiceSession newSession = session;
                Thread t = new Thread() {
                    public void run() {
                        // Route request to session
                        if (newSession != null) {
                            newSession.receiveBye(newRequest);
                        }
                    }
                };
                t.start();
            } else {
            // Route request to session
            if (session != null) {
                session.receiveBye(request);
            }
            }

            // Send a 200 OK response
            try {
                if (logger.isActivated()) {
                    logger.info("Send 200 OK");
                }
                SipResponse response = SipMessageFactory.createResponse(request, 200);
                imsModule.getSipManager().sendSipResponse(response);


                // Send message delivery status via a SIP MESSAGE
            } catch(Exception e) {
                   if (logger.isActivated()) {
                    logger.error("Can't send 200 OK response", e);
                }
            }
        } else
        if (request.getMethod().equals(Request.CANCEL)) {
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
        } else
        if (request.getMethod().equals(Request.UPDATE)) {
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

        // Dispatch the notification to the corresponding service
        if (eventHeader.getEventType().equalsIgnoreCase("presence.winfo")) {
            // Presence service
            if (RcsSettings.getInstance().isSocialPresenceSupported() && imsModule.getPresenceService().isServiceStarted()) {
                imsModule.getPresenceService().getWatcherInfoSubscriber().receiveNotification(notify);
            }
        } else
        if (eventHeader.getEventType().equalsIgnoreCase("presence")) {
            if (notify.getTo().indexOf("anonymous") != -1) {
                // Capability service
                imsModule.getCapabilityService().receiveNotification(notify);
            } else {
                // Presence service
                imsModule.getPresenceService().getPresenceSubscriber().receiveNotification(notify);
            }
        } else
        if (eventHeader.getEventType().equalsIgnoreCase("conference")) {
            // IM service
            imsModule.getInstantMessagingService().receiveConferenceNotification(notify);
        } else {
            if (!SubscriptionManager.getInstance().receiveNotification(notify)) {
                // Not supported service
                if (logger.isActivated()) {
                    logger.debug("Not supported notification event type");
                }
            }
        }
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
            for(Enumeration<ImsServiceSession> e = list[i].getSessions(); e.hasMoreElements();) {
                ImsServiceSession session = (ImsServiceSession)e.nextElement();
                if ((session != null) && session.getDialogPath().getCallId().equals(callId)) {
                    return session;
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
