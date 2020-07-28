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

package com.orangelabs.rcs.core.ims.service.im.chat;

import java.util.List;

import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.RcsService.Direction;

import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.ImsSessionListener;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.geoloc.GeolocInfoDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.iscomposing.IsComposingInfo;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.StringUtils;

/**
 * Abstract 1-1 chat session
 * 
 * @author Jean-Marc AUFFRET
 */
public abstract class StandaloneChatSession extends ChatSession {
    /**
     * Boundary tag
     */
    protected final static String BOUNDARY_TAG = "boundary1";

    protected String chatId;
    
    protected List<String> featureTags;

    protected String extraContent;
    
    protected String contentType;
    
    protected String preferService;
    
    protected int msgType = ChatLog.Message.Type.CONTENT;

    protected int direction = Direction.IRRELEVANT.toInt();

    protected boolean secondary = false;    

    protected String destination = null;

    /**
     * Constructor
     * 
     * @param parent IMS service
     * @param contact Remote contact
     */
    public StandaloneChatSession(
            ImsService parent, String target, List<String> contacts, List<String> featureTags) {
        super(parent, target, new ListOfParticipant(contacts));

        if (logger.isActivated()) {
            logger.info("CPMS StandaloneChatSession");
        }

        chatId = PhoneUtils.generateContactsText(contacts);

        if (featureTags == null) {
            if (!RcsSettings.getInstance().isCPMSupported()) {
                setFeatureTags(ChatUtils.getSupportedFeatureTagsForChat());
            } else {
                setCpimFeatureTags(ChatUtils.getCpimSupportedFeatureTagsForChat());
            }
        } else {
            setFeatureTags(featureTags);
        }

        // Set accept-types
        String acceptTypes = CpimMessage.MIME_TYPE + " " + IsComposingInfo.MIME_TYPE;
        setAcceptTypes(acceptTypes);
                
        // Set accept-wrapped-types
        String wrappedTypes = InstantMessage.MIME_TYPE + " " + ImdnDocument.MIME_TYPE;
        if (RcsSettings.getInstance().isGeoLocationPushSupported()) {
            wrappedTypes += " " + GeolocInfoDocument.MIME_TYPE;
        }
        setWrappedTypes(wrappedTypes);
    }

    /**
     * Is group chat
     * 
     * @return Boolean
     */
    public boolean isGroupChat() {
        return false;
    }
    
    /**
     * Returns the list of participants currently connected to the session
     * 
     * @return List of participants
     */
    public ListOfParticipant getConnectedParticipants() {
        return getParticipants();
    }

    /**
     * Close media session
     */
    public void closeMediaSession() {
        // Stop the activity manager
        getActivityManager().stop();

        // Close MSRP session
        closeMsrpSession();
    }
    
    /**
     * Send a text message
     * 
     * @param id Message-ID
     * @param txt Text message
     */
    public void sendTextMessage(String msgId, String txt) {
        sendMessage(msgId, txt, ChatLog.Message.Type.CONTENT);
    }
    
    public void sendMessage(String msgId, String text, int msgType) {
        boolean useImdn = getImdnManager().isImdnActivated();
        String contentType;

        switch (msgType) {
        case ChatLog.Message.Type.CLOUD:
            contentType = InstantMessage.CLOUD_MIME_TYPE;
            break;

        case ChatLog.Message.Type.EMOTICON:
            contentType = InstantMessage.EMOTICONS_MIME_TYPE;
            break;

        case ChatLog.Message.Type.CARD:
            contentType = InstantMessage.CARD_MIME_TYPE;
            break;

        default:
            contentType = InstantMessage.MIME_TYPE;
        }

        String from = ChatUtils.ANOMYNOUS_URI;
        String to = ChatUtils.ANOMYNOUS_URI;

        String content;
        if (RcsSettings.getInstance().supportOP01()) {
            content = ChatUtils.buildCpimMessageWithDeliveredImdn(
                    from, to, msgId, StringUtils.encodeUTF8(text), contentType);
        }
        else if (useImdn) {
            // Send message in CPIM + IMDN
            if(RcsSettings.getInstance().isImDisplayedNotificationActivated()){
                content = ChatUtils.buildCpimMessageWithImdn(
                        from, to, msgId, StringUtils.encodeUTF8(text), contentType);
            } else {
                content = ChatUtils.buildCpimMessageWithoutDisplay(
                        from, to, msgId, StringUtils.encodeUTF8(text), contentType);
            }
        } else {
            // Send message in CPIM
            content = ChatUtils.buildCpimMessage(
                    from, to, StringUtils.encodeUTF8(text), contentType);
        }

        // Send content
        boolean result = sendDataChunks(msgId, content, CpimMessage.MIME_TYPE);

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
        // Update rich messaging history
        if (!rmHistory.isMessageExists(msgId)) {
            if (logger.isActivated()) {
                logger.info("CPMS sendTextMessage Add in DB Msgid:" + msgId );
            }
            InstantMessage msg = new InstantMessage(msgId, chatId, text, useImdn, null);
            msg.setMessageType(msgType);
            rmHistory.addExtendChatMessage(msg, Direction.OUTGOING.toInt());
        }

        // Check if message has been sent with success or not
        if (!result) {
            // Notify listeners
            for(ImsSessionListener listener: getListeners()) {
                ((ChatSessionListener)listener).handleMessageDeliveryStatus(
                        msgId, ImdnDocument.DELIVERY_STATUS_FAILED, chatId, ImdnDocument.UNKNOWN, null);
            }
        }
    }

    /**
     * Send a geoloc message
     * 
     * @param msgId Message ID
     * @param geoloc Geoloc info
     */
    public void sendGeolocMessage(String msgId, GeolocPush geoloc) {
        boolean useImdn = getImdnManager().isImdnActivated();
        String mime = CpimMessage.MIME_TYPE;
        String from = ChatUtils.ANOMYNOUS_URI;
        String to = ChatUtils.ANOMYNOUS_URI;
        String geoDoc = null;//ChatUtils.buildGeolocDocument(geoloc, ImsModule.IMS_USER_PROFILE.getPublicUri(), msgId);

        String content;
        if (useImdn) {
            // Send message in CPIM + IMDN
            content = ChatUtils.buildCpimMessageWithImdn(from, to, msgId, geoDoc, GeolocInfoDocument.MIME_TYPE);
        } else {
            // Send message in CPIM
            content = ChatUtils.buildCpimMessage(from, to, geoDoc, GeolocInfoDocument.MIME_TYPE);
        }

        // Send content
        boolean result = sendDataChunks(msgId, content, mime);

        // Update rich messaging history
        GeolocMessage geolocMsg = new GeolocMessage(msgId, getRemoteContact(), geoloc, useImdn);
        RichMessagingHistory.getInstance().addChatMessage(geolocMsg, Direction.OUTGOING.toInt());

        // Check if message has been sent with success or not
        if (!result) {
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateChatMessageStatus(msgId, ChatLog.Message.Content.Status.FAILED.toInt());
            
            // Notify listeners
            for(int i=0; i < getListeners().size(); i++) {
                ((ChatSessionListener)getListeners().get(i)).handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_FAILED, null, null);
            }
        }
    }
    
    /**
     * Send is composing status
     * 
     * @param status Status
     */
    public void sendIsComposingStatus(boolean status) {
        if (logger.isActivated()) {
            logger.info("CPMS OneOneChatSession sendIsComposingStatus ");
        }
        String content = IsComposingInfo.buildIsComposingInfo(status);
        String msgId = ChatUtils.generateMessageId();
        sendDataChunks(msgId, content, IsComposingInfo.MIME_TYPE,true);
    }
    
    /**
     * Reject the session invitation
     */
    public void rejectSession() {
        rejectSession(486);
    }

    /**
     * Create an INVITE request
     *
     * @return the INVITE request
     * @throws SipException 
     */
    public SipRequest createInvite() throws SipException {
        SipRequest invite = null;
        boolean multipart = getFirstMessage() != null ||
                            getParticipants().getList().size() > 1;
        boolean cpmSupported = true;
                            
        if (RcsSettings.getInstance().supportOP01())
            multipart = true;

        if (RcsSettings.getInstance().isCPMSupported()) {
            invite = SipMessageFactory.createLargeCpmInvite(
                    getDialogPath(),
                    getFeatureTags(),
                    preferService,
                    getDialogPath().getLocalContent(),
                    multipart,
                    BOUNDARY_TAG);
        } else {
            cpmSupported = false;
            if (multipart)
                invite = SipMessageFactory.createMultipartInvite(
                            getDialogPath(),
                            getFeatureTags(),
                            getDialogPath().getLocalContent(),
                            BOUNDARY_TAG);
            else
                invite = SipMessageFactory.createInvite(
                            getDialogPath(),
                            getFeatureTags(),
                            getDialogPath().getLocalContent());
        }

        invite.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, getContributionID()); 
        if (cpmSupported) {
            if(getConversationID() != null){
                invite.addHeader(ChatUtils.HEADER_CONVERSATION_ID, getConversationID());
            }
        }

        return invite;
    }

    /**
     * Handle 200 0K response 
     *
     * @param resp 200 OK response
     */
    public void handle200OK(SipResponse resp) {
        super.handle200OK(resp);

        // Start the activity manager
        getActivityManager().start();
    }

    /**
     * Data transfer error
     *
     * @param msgId Message ID
     * @param error Error code
     */
    public void msrpTransferError(String msgId, String error) {
        super.msrpTransferError(msgId, error);

        // Request capabilities
        getImsService().getImsModule().getCapabilityService().requestContactCapabilities(getDialogPath().getRemoteParty());
    }
    
    public void setExtraContent(String extraContent) {
        this.extraContent = extraContent;
    }
    
    public String getChatId() {
        return chatId;
    }
    
    public void setDirection(int direction) {
        this.direction = direction;
    }
    
    public int getDirection() {
        return direction;
    }

    public void setDestination(String destination) {
        this.destination = destination;
    }
    
    public String getDestination() {
        return destination;
    }

    public void setMessageType(int msgType) {
        this.msgType = msgType;
    }
    
    public int getMessageType() {
        return msgType;
    }

    public void setSecondary(boolean secondary) {
        this.secondary = secondary;
    }
    
    public boolean toSecondary() {
        return secondary;
    }
}
