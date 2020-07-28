package com.orangelabs.rcs.service.api;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

import javax2.sip.SipException;
import javax2.sip.header.ContentDispositionHeader;
import javax2.sip.header.ContentLengthHeader;
import javax2.sip.header.ContentTypeHeader;

import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.ExtendChat;
import com.gsma.services.rcs.chat.IChatMessage;
import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.chat.IExtendChat;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.chat.IExtendChatListener;
import com.gsma.services.rcs.chat.ConferenceEventData.ConferenceUser;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.Status;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.service.broadcaster.IOneToOneChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.IExtendChatEventBroadcaster;
import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.core.ims.ImsError;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipInterface;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsSessionBasedServiceError;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatError;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSessionListener;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.ContributionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocPush;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.StandaloneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.ChatMessagePersistedStorageAccessor;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.service.broadcaster.IOneToOneChatEventBroadcaster;
import com.orangelabs.rcs.utils.ContactIdUtils;
import com.orangelabs.rcs.utils.DateUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.utils.logger.Logger;

import android.content.Intent;
import android.os.AsyncTask;
import android.os.RemoteCallbackList;

/**
* Chat implementation
*
 * @author Jean-Marc AUFFRET
*/
public class ExtendChatImpl extends IExtendChat.Stub implements ChatSessionListener {
    
    private final static String BOUNDARY_TAG = "jXfWUFcrCxZEXdN";

    /**
     * Remote contact
     */
    private List<String> contacts;
    
    private List<Boolean> listBcc;
    
    private String contact;

    private ChatServiceImpl chatService;
    
    private RichMessagingHistory messagingLog = null;
    
    private String mChatId;

    /**
     * List of listeners
     */
    private RemoteCallbackList<IExtendChatListener> listeners = 
            new RemoteCallbackList<IExtendChatListener>();
    
    private CopyOnWriteArrayList<InstantMessage> pendingMsgList =
            new CopyOnWriteArrayList<InstantMessage>();
    
    private StandaloneChatSession session;
    
    private IExtendChatEventBroadcaster mBroadcaster = null;

    private boolean secondary = false;
    
    private InstantMessagingService mImsService = null;

    private Object lock = new Object();

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     *
     * @param contact Remote contact
     */
    public ExtendChatImpl(String contact, ChatServiceImpl chatService) {
        this.contacts = new ArrayList<String>();
        this.contacts.add(contact);
        this.contact = contact;

        this.chatService = chatService;
        
        this.secondary = PhoneUtils.compareUuid(
                RcsSettings.getInstance().getSecondaryDeviceUserIdentity(), contact);
        
        this.messagingLog = RichMessagingHistory.getInstance();
        this.mBroadcaster = chatService.getExtendChatEventBroadcaster();
        this.mImsService = Core.getInstance().getImService();
        
        // Set chat Id
        String callId = mImsService.getImsModule().getSipManager().getSipStack().generateCallId();
        this.mChatId = contact;
    }

    /**
     * Constructor
     *
     * @param contact Remote contact
     */
    public ExtendChatImpl(List<String> contacts, ChatServiceImpl chatService) {
        this.contacts = contacts;
        this.contact = PhoneUtils.generateContactsText(contacts);

        this.chatService = chatService;
        this.messagingLog = RichMessagingHistory.getInstance();
        this.mBroadcaster = chatService.getExtendChatEventBroadcaster();
        mImsService = Core.getInstance().getImService();
        
        // Set chat Id
        String callId = mImsService.getImsModule().getSipManager().getSipStack().generateCallId();
        this.mChatId = ContributionIdGenerator.getContributionId(callId);
        if (logger.isActivated()) {
            logger.info("ExtendChatImpl chatId: " + mChatId);
        }
    }

    public String getChatId(){
        return PhoneUtils.generateContactsText(contacts);
    }
    
    public String getExtendChatId(){
        return mChatId;
    }

    /**
     * Returns the remote contact
     *
     * @return Contact
     */
    public List<String> getRemoteContacts() {
        return contacts;
    }

    /**
     * Sends a message
     *
     * @param message message content
     * @param msgType specified message type, OP01 extension
     * @return Unique message ID or null in case of error
     */
    public IChatMessage sendMessage(String message, int msgType) {
        if (logger.isActivated()) {
            logger.debug("Send message:" + message + " with msgType " + msgType);
        }
        if(message == null){
            if (logger.isActivated()) {
                logger.debug("Send message message is null" );
            }
            return null;
        }
        if (logger.isActivated()) {
            logger.debug("Send message lemgth:" + message .length());
        }
        InstantMessagingService imService = Core.getInstance().getImService();
        

        final InstantMessage instantMessage = ChatUtils.createTextMessage(
                contact, message, imService.getImdnManager().isImdnActivated());
        //instantMessage.setMessageType(msgType);
        //instantMessage.setSecondary(secondary);

        String msgId = instantMessage.getMessageId();

        RichMessagingHistory.getInstance().addExtendChatMessage(
                instantMessage, Direction.OUTGOING.toInt());
        
        ChatMessagePersistedStorageAccessor persistedStorage = new ChatMessagePersistedStorageAccessor(
                messagingLog, instantMessage.getMessageId(), instantMessage.getRemote(), message,
                instantMessage.getMimeType(), contact, Direction.OUTGOING);
        IChatMessage chatMsg = new ChatMessageImpl(persistedStorage);
        
        Thread t = new Thread(){
            public void run(){
                sendChatMessage(instantMessage);
            }
        };
        t.start();
        
        return chatMsg;      
    }
    
    public void sendChatMessage(InstantMessage instantMessage){
        int result;
        try {
            int size = instantMessage.getTextMessage().length();
            if (logger.isActivated()) {
                logger.debug("sendChatMessage length: " + size);
            }
            if (size < RcsSettings.getInstance().getMaxPagerContentSize())
                result = sendPagerMessage(instantMessage);
            else
                result = sendLargeMessage(instantMessage);
    
            if (result < 0) {
                if (logger.isActivated()) {
                    logger.debug("send message fail");
                }
                handleMessageDeliveryStatus(instantMessage.getMessageId(),ImdnDocument.DELIVERY_STATUS_SENDING_FAILED,instantMessage.getRemote(),0,"");
    
            }
        } catch(Exception e){
            e.printStackTrace();
            handleMessageDeliveryStatus(instantMessage.getMessageId(),ImdnDocument.DELIVERY_STATUS_SENDING_FAILED,instantMessage.getRemote(),0,"");
        }
    }

    /**
     * Re-send message
     *
     * @param msgId message identifier
     * @return Unique message ID or null in case of error
     */
    public void resendMessage(String msgId) {
        if (logger.isActivated()) {
             logger.debug("resendMessage msgId:" + msgId);
        }

        InstantMessagingService imService = Core.getInstance().getImService();
        String msgText = messagingLog.getMessageText(msgId);
        int msgType = messagingLog.getMessageType(msgId);

        if (msgText == null) {
            if (logger.isActivated()) {
                logger.debug("resend message fail->no message found");
            }
        }

        InstantMessage message = ChatUtils.createTextMessage(
                contact, msgText, imService.getImdnManager().isImdnActivated());
        message.setMessageType(msgType);
        message.setMessageId(msgId);

        int result;
        int size = ChatUtils.getEncodedMessageSize(msgText);
        if (size < RcsSettings.getInstance().getMaxPagerContentSize())
            result = sendPagerMessage(message);
        else
            result = sendLargeMessage(message);

        if (result < 0) {
            if (logger.isActivated()) {
                logger.debug("resend message fail");
            }
            handleMessageDeliveryStatus(message.getMessageId(),ImdnDocument.DELIVERY_STATUS_SENDING_FAILED,message.getRemote(),0,"");
        }
    }
    
    public String prosecuteMessage(String msgId) {
        if (logger.isActivated()) {
            logger.debug("resendMessage msgId:" + msgId);
        }

        InstantMessagingService imService = Core.getInstance().getImService();
        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance(); 
        String msgText = rmHistory.getMessageText(msgId);
        
        if (msgText.isEmpty())
            return null;

        InstantMessage message = ChatUtils.createTextMessage(
                contact, msgText, imService.getImdnManager().isImdnActivated());

        List<String> contacts = rmHistory.getMessageParticipants(msgId);
        long timeStamp = rmHistory.getMessageTimeStamp(msgId); 

        String extraContent = 
                "Spam-From: " + ChatUtils.formatCpimSipUri(contacts.get(0)) + SipUtils.CRLF +
                "Spam-To: " + ImsModule.IMS_USER_PROFILE.getPublicUri() + SipUtils.CRLF +
                "DateTime: " + DateUtils.encodeDate(timeStamp) + SipUtils.CRLF;
        message.setExtraContent(extraContent);

        int result = 0;
        int size = ChatUtils.getEncodedMessageSize(msgText);
        if (size < RcsSettings.getInstance().getMaxPagerContentSize())
            result = sendPagerMessage(message);
        else
            result = sendLargeMessage(message);
        
        if (result < 0)
            return null;

        return message.getMessageId();
    }
    
    /**
     * Sends a geoloc message
     *
     * @param geoloc Geoloc
     * @return Unique message ID or null in case of error
     */
    public String sendGeoloc(Geoloc geoloc) {
        if (logger.isActivated()) {
            logger.debug("Send geoloc message");
        }

        InstantMessagingService imService = Core.getInstance().getImService();
        GeolocPush geolocPush = new GeolocPush(geoloc.getLabel(),
                geoloc.getLatitude(), geoloc.getLongitude(),
                geoloc.getExpiration(), geoloc.getAccuracy());

        GeolocMessage msg = ChatUtils.createGeolocMessage(
                contact, geolocPush, imService.getImdnManager().isImdnActivated());

        if (sendLargeMessage(msg) >= 0)
            return msg.getMessageId();
        
        return null;
    }

    /**
     * Sends a Burn delivery report for a given message ID
     *
     * @param msgId Message ID
     */
    public void sendBurnedDeliveryReport(String msgId) {
        try {
            if (logger.isActivated()) {
                logger.info("Set displayed delivery report for " + msgId);
            }

            InstantMessagingService imService = Core.getInstance().getImService();
            RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
            
            List<String> contacts = rmHistory.getMessageParticipants(msgId);
            int msgType = rmHistory.getMessageType(msgId);

            if (contacts != null && msgType == ChatLog.Message.Type.BURN) {
                String contact = contacts.get(0);
                if (logger.isActivated()) {
                    logger.info("Set burned delivery report for contact: " + contact);
                }
                String subStr =  contact.substring(0,3);
                String newContact = null;
                if(!(subStr.equals("sip") || subStr.equals("tel"))){
                    newContact = "tel:" + contact ;
                }
                else{
                    newContact = contact;
                }
                if (logger.isActivated()) {
                    logger.info("Set burned delivery report for newContact: " + newContact);
                }

                imService.getImdnManager().sendMessageDeliveryStatus(
                        newContact, msgId, ImdnDocument.BURN_STATUS_BURNED);
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Could not send delivery status",e);
            }
        }
    }

    /**
     * Sends a displayed delivery report for a given message ID
     *
     * @param msgId Message ID
     */
    public void sendDisplayedDeliveryReport(final String msgId) {
        try {
            if (logger.isActivated()) {
                logger.info("LMM Set displayed delivery report for " + msgId);
            }

            InstantMessagingService imService = Core.getInstance().getImService();
            RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
            
            List<String> contacts = rmHistory.getMessageParticipants(msgId);
            if (contacts != null) {
                String contact = contacts.get(0);
                if (logger.isActivated()) {
                    logger.info("LMM Set displayed delivery report for contact: " + contact);
                }
                String subStr =  contact.substring(0,3);
                String newContact = null;
                if(!(subStr.equals("sip") || subStr.equals("tel"))){
                    newContact = "tel:" + contact ;
                }
                else{
                    newContact = contact;
                }
                if (logger.isActivated()) {
                    logger.info("LMM Set displayed delivery report for newContact: " + newContact);
                }
                // Send via SIP MESSAGE
                imService.getImdnManager().sendMessageDeliveryStatus(
                        newContact, msgId, ImdnDocument.DELIVERY_STATUS_DISPLAYED);
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("LMM Could not send MSRP delivery status",e);
            }
        }
    }

    /**
     * Adds a listener on chat events
     *
     * @param listener Chat event listener
     */
    public void addEventListener(IExtendChatListener listener) {
        if (logger.isActivated()) {
            logger.info("LMM Add an event listener");
        }

        synchronized(lock) {
            listeners.register(listener);
        }
    }

    /**
     * Removes a listener on chat events
     *
     * @param listener Chat event listener
     */
    public void removeEventListener(IExtendChatListener listener) {
        if (logger.isActivated()) {
            logger.info("LMM Remove an event listener");
        }

        synchronized(lock) {
            listeners.unregister(listener);
        }
    }

    private int sendPagerMessage(InstantMessage message) {
        int result = 0;

        String content = composeRequestContent(message);
        
        String contentType = CpimMessage.MIME_TYPE;
        if (contacts.size() > 1 || message.getExtraContent() != null)
            contentType = "multipart/mixed;" + "boundary=" + BOUNDARY_TAG;

        final SipRequest request = createSipMessageRequest(
                message.getMessageType(), contentType, content);

        if (request == null)
            return -1;
        
        final String msgId = message.getMessageId();
        new Thread() {

            public void run() {
                sendSipRequest(request, msgId);
            }

        }.start();
        
        return result;
    }

    private int sendLargeMessage(InstantMessage message) {

        synchronized (lock) {
            pendingMsgList.add(message);
            return sendLargeMessageImmdiately(message);
        }
    }
    
    private int sendLargeMessageImmdiately(InstantMessage message) {
        InstantMessagingService imService = Core.getInstance().getImService();
        ChatSession session = null;
        try {
            session = imService.initiateStandaloneChatSession(contacts, listBcc,message);
        } catch (CoreException e) {
            e.printStackTrace();
            return -1;
        }
        this.session = (StandaloneChatSession)session;
        this.session.setExtraContent(message.getExtraContent());
        session.addListener(this);

        session.startSession();
        
        return 0;
    }

    private String composeRequestContent(InstantMessage message) {
        String content;
        String contentPart;
        String mimeType = InstantMessage.MIME_TYPE;

        switch (message.getMessageType()) {
        case ChatLog.Message.Type.CLOUD:
            mimeType = InstantMessage.CLOUD_MIME_TYPE;
            break;

        case ChatLog.Message.Type.EMOTICON:
            mimeType = InstantMessage.EMOTICONS_MIME_TYPE;
            break;

        case ChatLog.Message.Type.CARD:
            mimeType = InstantMessage.CARD_MIME_TYPE;
            break;
        }

        contentPart = composeCpimContent(message, mimeType);

        String extraContent = message.getExtraContent();
        if (contacts.size() > 1 ||
            extraContent != null) {
            content = Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF;
            
            if (extraContent != null) {
                content += ContentTypeHeader.NAME + ": text/plain;charset=UTF-8" + SipUtils.CRLF +
                           ContentLengthHeader.NAME + ": " + extraContent.getBytes().length + SipUtils.CRLF +
                           SipUtils.CRLF +
                           extraContent +
                           SipUtils.CRLF;
            }
            if (contacts.size() > 1) {
                String resourceList = null;
                boolean isBccOff = RcsSettings.getInstance().isGroupMessagingActivated();
                if (logger.isActivated()) {
                    logger.info("composeRequestContent isBcc: " + isBccOff);
                }
                if(isBccOff)
                    resourceList = ChatUtils.generateMultiChatResourceList(contacts);
                else
                    resourceList = ChatUtils.generateMultiChatBccResourceList(contacts);

                content += ContentTypeHeader.NAME + ": application/resource-lists+xml" + SipUtils.CRLF +
                           "Content-Length: " + resourceList.getBytes().length + SipUtils.CRLF +
                           ContentDispositionHeader.NAME + ": recipient-list" + SipUtils.CRLF +
                           "Content-Length: " + resourceList.getBytes().length + SipUtils.CRLF +
                           SipUtils.CRLF +
                           resourceList + SipUtils.CRLF +
                           SipUtils.CRLF;
            }
            content += Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + SipUtils.CRLF +
                    ContentTypeHeader.NAME + ": " + CpimMessage.MIME_TYPE + SipUtils.CRLF +
                    ContentLengthHeader.NAME + ": " + contentPart.getBytes().length + SipUtils.CRLF +
                    SipUtils.CRLF +
                    contentPart +
                    SipUtils.CRLF +
                    Multipart.BOUNDARY_DELIMITER + BOUNDARY_TAG + Multipart.BOUNDARY_DELIMITER;
        } else {
            content = contentPart;
        }
        
        return content;
    }
    
    private String composeCpimContent(InstantMessage message, String mimeType) {
        String cpim;
        InstantMessagingService imService = Core.getInstance().getImService();
        
        String from = ImsModule.IMS_USER_PROFILE.getPublicUri();
        String to = ChatUtils.formatCpimSipUri(message.getRemote());
        if(RcsSettings.getInstance().isSupportOP07()){
            from = ChatUtils.ANOMYNOUS_URI;
            to = ChatUtils.ANOMYNOUS_URI;
        }
        if (RcsSettings.getInstance().supportOP01()) {
            cpim = ChatUtils.buildCpimMessageWithDeliveredImdn(
                    from,
                    to,
                    message.getMessageId(),
                    StringUtils.encodeUTF8(message.getTextMessage()),
                    mimeType);
        }
        else if (imService.getImdnManager().isImdnActivated()) {
            if(RcsSettings.getInstance().isImDisplayedNotificationActivated()){
                cpim = ChatUtils.buildCpimMessageWithImdn(
                        from,
                        to,
                        message.getMessageId(),
                        StringUtils.encodeUTF8(message.getTextMessage()),
                        mimeType);
            } else {
                cpim = ChatUtils.buildCpimMessageWithoutDisplay(
                        from,
                        to,
                        message.getMessageId(),
                        StringUtils.encodeUTF8(message.getTextMessage()),
                        mimeType);
            }
        }
        else {
            cpim = ChatUtils.buildCpimMessage(
                    from,
                    to,
                    StringUtils.encodeUTF8(message.getTextMessage()),
                    mimeType);
        }

        return cpim;
    }

    private SipRequest createSipMessageRequest(
            int msgType, String contentType, String content) {
        ArrayList<String> featureTags = new ArrayList<String>();
       
        String preferService = null;

        featureTags.add(FeatureTags.FEATURE_RCSE_PAGER_MSG);

        switch (msgType) {
        case ChatLog.Message.Type.BURN:
            featureTags.add(FeatureTags.FEATURE_CPM_BURNED_MSG);
            break;

        case ChatLog.Message.Type.PUBLIC:
            featureTags.add(FeatureTags.FEATURE_CMCC_IARI_PUBLIC_ACCOUNT);
            preferService = FeatureTags.FEATURE_CMCC_URN_PUBLIC_ACCOUNT;
            break;

        case ChatLog.Message.Type.CLOUD:
            featureTags.set(0, FeatureTags.FEATURE_CMCC_IARI_CLOUD_FILE);
            break;

        case ChatLog.Message.Type.EMOTICON:
            featureTags.set(0, FeatureTags.FEATURE_CMCC_IARI_EMOTICON);
            break;

        case ChatLog.Message.Type.CARD:
            featureTags.set(0, FeatureTags.FEATURE_CMCC_IARI_CARD_MSG);
            break;
        }

        
        String remoteContact = contact;
        if (logger.isActivated()) {
            logger.debug("sendSipRequest send Message remoteContact before : " + remoteContact);
        }
        //Handling for short code
        boolean shortCode = false;
        if(remoteContact.length() == 6){
            shortCode = true;
        }
        if (contacts.size() > 1)
            remoteContact = ImsModule.IMS_USER_PROFILE.getMultiImConferenceUri();
        else if (msgType == ChatLog.Message.Type.PUBLIC || secondary)
            remoteContact = PhoneUtils.formatUuidToSipUri(contact);
        else
            remoteContact = PhoneUtils.formatNumberToSipUri(contact);
        if(shortCode){
            String newRemote = remoteContact;
            int index = newRemote.indexOf("+1");
            remoteContact = newRemote.substring(0,index) + newRemote.substring(index+2);
        }
        
        if (logger.isActivated()) {
            logger.debug("sendSipRequest send Message remoteContact after : " + remoteContact);
        }

        SipInterface sipStack = Core.getInstance().getImsModule().getSipManager().getSipStack();
        SipDialogPath dialogPath = new SipDialogPath(
                sipStack,
                sipStack.generateCallId(),
                1,
                remoteContact,
                ImsModule.IMS_USER_PROFILE.getPublicUri(),
                remoteContact,
                sipStack.getServiceRoutePath());

        try {
            SipRequest request = SipMessageFactory.createMessageCPM(dialogPath,
                    featureTags.toArray(new String[0]), preferService, contentType, content);

            SipUtils.buildAllowHeader(request.getStackMessage());

            String contribId = ContributionIdGenerator.getContributionId(dialogPath.getCallId());
            request.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, contribId);

            String convsId = RichMessagingHistory.getInstance().getCoversationID(contact, 1);
            if (convsId.isEmpty()) {
                String callId = sipStack.generateCallId();
                convsId = ContributionIdGenerator.getContributionId(callId);

                InstantMessage conversationMsg = ChatUtils.createTextMessage(remoteContact, "system", false);
                RichMessagingHistory.getInstance().addChatSystemMessage(
                        conversationMsg, Direction.OUTGOING.toInt());
                RichMessagingHistory.getInstance().UpdateCoversationID(contact, convsId, 1);
            }
            request.addHeader(ChatUtils.HEADER_CONVERSATION_ID, convsId);
            
            return request;
        } catch (SipException e) {
            e.printStackTrace();
            return null;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    private void sendSipRequest(SipRequest request, String msgId) {
        InstantMessagingService imService = Core.getInstance().getImService();
        SipTransactionContext ctx = null;
        try {
            ctx = imService.getImsModule().getSipManager().sendSipMessageAndWait(request);
            
            if (ctx.isSipResponse()) {
                if (logger.isActivated()) {
                    logger.error("sendSipRequest send Message: " + ctx.getStatusCode());
                }
                // Analyze received message
                if ((ctx.getStatusCode() == 200) || (ctx.getStatusCode() == 202)) {
                    // 200 OK received
                    if (logger.isActivated()) {
                        logger.info("20x OK response received");
                    }
                    handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_SENT, null, null);
                }  else if(ctx.getStatusCode() == 404 || ctx.getStatusCode() == 480 || ctx.getStatusCode() == 503){
                    // Error responses
                    if (logger.isActivated()) {
                        logger.info("message failed: " + ctx.getStatusCode() + " response received for msgid " + msgId);
                    }
                    handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_FALLBACK, null, null);
                } else {
                    // Error responses
                    if (logger.isActivated()) {
                        logger.info("failed: " + ctx.getStatusCode() + " response received");
                    }
                    handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_ERROR, null, null);
                }
            } else {
                if (logger.isActivated()) {
                    logger.info("No response for message: " + msgId);
                }
                handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_FALLBACK, null, null);
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("exception while sending pager message");
            }
            handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_ERROR, null, null);
        }
    }

    /*------------------------------- SESSION EVENTS ----------------------------------*/

    /**
     * Session is started
     */
    public void handleSessionStarted() {

        if (logger.isActivated()) {
            logger.info("LMM handleSessionStarted pendinglist size: " + pendingMsgList.size());
        }

        synchronized(lock) {
            if (pendingMsgList.size() > 0) {
                final InstantMessage msg = pendingMsgList.get(0);
                if (logger.isActivated()) {
                    logger.info("handleSessionStarted send message: " + msg.getMessageId() + ", text: " + msg.getTextMessage());
                }
                new Thread() {

                    public void run() {
                        session.sendMessage(
                                msg.getMessageId(), msg.getTextMessage(), msg.getMessageType());
                        session.abortSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
                    }

                }.start();
            }
        }
    }

    /**
     * Session has been aborted
     *
     * @param reason Termination reason
     */
    public void handleSessionAborted(int reason) {

        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("LMM handleSessionAborted size: " + pendingMsgList.size());
            }
            ChatServiceImpl.removeExtendChatSession(contact);
            
            if(pendingMsgList.size() != 0){
                pendingMsgList.remove(0);
            } else {
                return;
            }
            
            if (pendingMsgList.size() > 0){
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        InstantMessage imMessage = pendingMsgList.get(0);
                        if (logger.isActivated()) {
                            logger.info("LMM handleSessionAborted " +
                                    "text: " + imMessage.getTextMessage() +
                                    "MsgId: " + imMessage.getMessageId());
                        }
                        sendLargeMessageImmdiately(imMessage);
                    }
                });
            }
        }
    }

    /**
     * Session has been terminated by remote
     */
    public void handleSessionTerminatedByRemote() {
        if (session == null)
            return;

        String msgId = session.getFirstMessage().getMessageId();
        handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_FAILED, contact, new Date());
        
        synchronized(lock) {
            ChatServiceImpl.removeExtendChatSession(contact);
            if(pendingMsgList.size() != 0){
                pendingMsgList.remove(0);
            } else {
                return;
            }
            
            if (pendingMsgList.size() > 0) {
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        InstantMessage imMessage = pendingMsgList.get(0);
                        if (logger.isActivated()) {
                            logger.info("LMM handleSessionAborted " +
                                    "text: " + imMessage.getTextMessage() +
                                    "MsgId: " + imMessage.getMessageId());
                        }
                        sendLargeMessageImmdiately(imMessage);
                    }
                });
            }
        }
    }

    /**
     * New text message received
     *
     * @param text Text message
     */
    public void handleReceiveMessage(InstantMessage message) {
        if (logger.isActivated()) {
            logger.info("LMM handleReceiveMessage New IM received: "
                    + message.getTextMessage());
        }

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();

        rmHistory.addExtendChatMessage(message, message.getDirection());

        String number;
        if (message.getMessageType() == ChatLog.Message.Type.PUBLIC || 
            message.toSecondary())
            number = PhoneUtils.extractUuidFromUri(message.getRemote());
        else
            number = PhoneUtils.extractNumberFromUri(message.getRemote());
        /*ExtendMessage msgApi = new ExtendMessage(
                                    message.getMessageId(),
                                    number,
                                    message.getTextMessage(),
                                    InstantMessage.MIME_TYPE,
                                    message.getDisplayName(),
                                    Direction.INCOMING.toInt(),
                                    message.getServerDate().getTime(),0,
                                    0,0,
                                    ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt(),
                                    0,getChatId(),
                                    false,false,
                                    message.getMessageType());
        msgApi.setSecondary(message.toSecondary());*/

        // Broadcast intent related to the received invitation
        synchronized(lock) {
            mBroadcaster.broadcastMessageReceived(message.getMimeType(), message.getMessageId());
        }
        /*Intent intent = new Intent(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE);
        intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        try{
            intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT,number);
        } catch(Exception e){
            e.printStackTrace();
        }
        intent.putExtra(OneToOneChatIntent.EXTRA_DISPLAY_NAME, message.getDisplayName());
        intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, message.getMessageId());

        AndroidFactory.getApplicationContext().sendBroadcast(intent);*/
    }

    /**
     * New geoloc message received
     *
     * @param geoloc Geoloc message
     */
    public void handleReceiveGeoloc(GeolocMessage geoloc) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("New geoloc received");
            }

            // Update rich messaging history
            RichMessagingHistory.getInstance().addChatMessage(
                        geoloc, Direction.INCOMING.toInt());

            // Create a geoloc message
            Geoloc geolocApi = new Geoloc(geoloc.getGeoloc().getLabel(),
                    geoloc.getGeoloc().getLatitude(), geoloc.getGeoloc().getLongitude(),
                    geoloc.getGeoloc().getExpiration());
            /*com.gsma.services.rcs.chat.GeolocMessage msgApi = new com.gsma.services.rcs.chat.GeolocMessage(
                        geoloc.getMessageId(),
                        PhoneUtils.extractNumberFromUri(geoloc.getRemote()),
                        geoloc.getTextMessage(),
                        InstantMessage.MIME_TYPE,
                        geoloc.getDisplayName(),
                        Direction.INCOMING.toInt(),
                        geoloc.getServerDate().getTime(),0,
                        0,0,
                        ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt(),
                        0,getChatId(),
                        false,false,geolocApi);
            com.gsma.services.rcs.chat.GeolocMessage msgApi = new com.gsma.services.rcs.chat.GeolocMessage(
                        geoloc.getMessageId(),
                        PhoneUtils.extractNumberFromUri(geoloc.getRemote()),
                        geolocApi, new Date(), true);*/

            chatService.receiveStandaloneChatMessage(geoloc);
        }
    }
    
    /**
     * Set chat message status
     * 
     * @param msgId message ID
     * @param mimeType mime type
     * @param status status of message
     * @param reasonCode Reason code
     */
    private void setChatMessageStatusAndReasonCode(String msgId, String mimeType, int status,
            int reasonCode) {
        ContactId contactid = ContactIdUtils.createContactIdFromTrustedData(contact);
        Status mstatus = Status.valueOf(status);
        ReasonCode mcode = ReasonCode.valueOf(reasonCode);
        synchronized (lock) {
            if (messagingLog.setChatMessageStatusAndReasonCode(msgId, status, reasonCode)) {
                mBroadcaster.broadcastMessageStatusChanged(mChatId,contactid, mimeType, msgId, mstatus,
                        mcode);
            }
        }
    }

    /**
     * IM session error
     *
     * @param error Error
     */
    public void handleImError(ChatError error,String messagegId) {
        if (logger.isActivated()) {
            logger.info("handleImError " + error.getErrorCode() );
        }

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
            try {
                if(messagegId == null){
                    InstantMessage firstMessage = session.getFirstMessage();
                    if(firstMessage == null){
                        if (logger.isActivated()) {
                            logger.info("handleImError first message is null");
                        }
                        firstMessage = pendingMsgList.get(0);
                    }
                    if(firstMessage != null){
                        messagegId = firstMessage.getMessageId();
                        if (logger.isActivated()) {
                            logger.info("handleImError messagegId" + messagegId);
                        }
                    }
                }
            } catch (Exception e1) {
                if (logger.isActivated()) {
                    logger.info("LMM handleImError exception: " + messagegId);
                }
                e1.printStackTrace();
            }
    
            switch(error.getErrorCode()){
            case ChatError.SESSION_INITIATION_FAILED:
            case ChatError.SESSION_INITIATION_CANCELLED:
                /*rmHistory.updateChatMessageStatus(msgId, ChatLog.Message.Content.Status.FAILED.toInt());*/
                synchronized (lock) {
                    try {
                        //listeners.getBroadcastItem(i).onReportMessageFailed(
                                //msgId, ExtendChat.ErrorCodes.INTERNAL, ""); 
                        setChatMessageStatusAndReasonCode(messagegId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),
                                ReasonCode.UNSPECIFIED.toInt());
                        if (logger.isActivated()) {
                            logger.info("Write new listener");
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't notify listener", e);
                        }
                    }
                }
                break;
            case ChatError.SESSION_INITIATION_FALLBACK:
            case ChatError.SESSION_INITIATION_FALLBACK_MMS:
                setChatMessageStatusAndReasonCode(messagegId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.FALLBACK.toInt());
                break;
            default:
                break;
            }

        synchronized(lock) {
            pendingMsgList.remove(0);
            if (pendingMsgList.size() > 0) {
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        InstantMessage imMessage = pendingMsgList.get(0);
                        if (logger.isActivated()) {
                            logger.info("LMM handleIMError " +
                                "text: " + imMessage.getTextMessage() +
                                "MsgId: " + imMessage.getMessageId());
                        }
                        sendLargeMessageImmdiately(imMessage);
                    }
                });
            }
        }
    }

    /**
     * IM session error
     * 
     * @param error Error
     */
    public void handleInviteError(ChatError error) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("LMM Invite error " + error.getErrorCode() );
            }
            String remoteContact = session.getRemoteContact();
            String number = PhoneUtils.extractNumberFromUri(remoteContact);

            if (logger.isActivated()) {
                logger.info("LMM handleInviteError: " + remoteContact + "Number: " + number);
            }
            String messagegId = null;
            try {
                InstantMessage firstMessage = session.getFirstMessage();
                if(firstMessage == null){
                    if (logger.isActivated()) {
                        logger.info("handleInviteError firstMessage is null");
                    }
                    firstMessage = pendingMsgList.get(0);
                }
                if(firstMessage != null){
                    messagegId = firstMessage.getMessageId();
                    if (logger.isActivated()) {
                        logger.info("handleInviteError messagegId" + messagegId);
                    }
                }
            } catch (Exception e1) {
                if (logger.isActivated()) {
                    logger.info("LMM handleImError exception: " + messagegId);
                }
                e1.printStackTrace();
            }
            
            final int N = listeners.beginBroadcast();
            
            // Update rich messaging history
            switch(error.getErrorCode()){
                case ChatError.SESSION_INITIATION_ERROR:
                    RichMessagingHistory.getInstance().updateChatMessageStatus(messagegId,
                            ChatLog.Message.Content.Status.FAILED.toInt());
                    // TODO: notify listener
                    // Notify event listeners
                    
                    if (logger.isActivated()) {
                        logger.info("LMM SESSION_INITIATION_ERROR N:" + N);
                    }
                    for (int i=0; i < N; i++) {
                        try {
                            //listeners.getBroadcastItem(i).onReportMessageInviteError(session.getFirstMessage().getMessageId(),null,false);
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
                        } catch(Exception e) {
                            if (logger.isActivated()) {
                                logger.error("Can't notify listener", e);
                            }
                        }
                    }
                    break;
                case ChatError.SESSION_FORBIDDEN_ERROR:
                    RichMessagingHistory.getInstance().updateChatMessageStatus(messagegId,
                            ChatLog.Message.Content.Status.FAILED.toInt());
                    // TODO: notify listener
                    // Notify event listeners
                    if (logger.isActivated()) {
                        logger.info("LMM SESSION_FORBIDDEN_ERROR N:" + N);
                    }
                    for (int i=0; i < N; i++) {
                        try {
                            //listeners.getBroadcastItem(i).onReportMessageInviteError(session.getFirstMessage().getMessageId(),error.getMessage(),true);
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
                        } catch(Exception e) {
                            if (logger.isActivated()) {
                                logger.error("Can't notify listener", e);
                            }
                        }
                    }
                case ChatError.SESSION_INITIATION_FALLBACK:
                case ChatError.SESSION_INITIATION_FALLBACK_MMS:
                    setChatMessageStatusAndReasonCode(messagegId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.FALLBACK.toInt());
                    break;
                default:
                    break;
            }
            
            // Remove session from the list
            if(session.isStoreAndForward()){
                if (logger.isActivated()) {
                    logger.debug("LMM AddRemove remove storeChatSessions " + session.getSessionID());
                }
                ChatServiceImpl.removeStoreChatSession(number);
            }
            else {
                if (logger.isActivated()) {
                    logger.debug("LMM AddRemove remove chatSessions " + session.getSessionID());
                }
                ChatServiceImpl.removeChatSession(number);
            }
        }
        listeners.finishBroadcast();
    }

    /**
     * Is composing event
     *
     * @param contact Contact
     * @param status Status
     */
    public void handleIsComposingEvent(String contact, boolean status) {
        
    }

     /**
     * New message delivery status
     *
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String msgId, String status, String contact, Date date) {
        if (logger.isActivated()) {
            logger.info("handleMessageDeliveryStatus1 " + msgId + ", status " + status + "; date: " + date);
        }
        List<String> contacts = new ArrayList();

        if(this.contacts.size() == 1){
            contacts.add(this.contact);
        } else {
            contacts = this.contacts;
        }
        if(date == null){
            date = new Date();
        }
        if (logger.isActivated()) {
            logger.info("handleMessageDeliveryStatus1 contact: " + contact + ", date: " + date);
        }
        
        /*// Update rich messaging history
        RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(msgId, status);*/

        for(String mContact: contacts){
            if (logger.isActivated()) {
                logger.info("handleMessageDeliveryStatus1 mContact: " + mContact );
            }
            ContactId contactid = ContactIdUtils.createContactIdFromTrustedData(mContact);
            try {
                if (status.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
                    //listeners.getBroadcastItem(i).onReportMessageDelivered(msgId);
                    Status mstatus = Status.DELIVERED;
                    chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
                    //listeners.getBroadcastItem(i).onReportMessageDisplayed(msgId);
                    Status mstatus = Status.DISPLAYED;
                    chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_ERROR)) {
                    //listeners.getBroadcastItem(i).onReportMessageFailed(msgId);
                    Status mstatus = Status.FAILED;
                    chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                    if (status.equals(ImdnDocument.DELIVERY_STATUS_SENDING_FAILED)) {
                    //listeners.getBroadcastItem(i).onReportMessageFailed(msgId);
                    Status mstatus = Status.FAILED;
                    chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, date, ReasonCode.FAILED_SEND);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                }  else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_SENT)) {
                    //listeners.getBroadcastItem(i).onReportMessageSent(msgId);
                    Status mstatus = Status.SENT;
                    chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                    if (status.equals(ImdnDocument.DELIVERY_STATUS_FALLBACK)) {
                    //listeners.getBroadcastItem(i).onReportMessageSent(msgId);
                    Status mstatus = Status.FAILED;
                    chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, date, ReasonCode.FALLBACK);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        
    }

    /**
     * New message delivery status
     *
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(
            String msgId, String status, String contact, int errorCode ,String statusCode) {
        if (logger.isActivated()) {
            logger.info(" New message delivery status for message " + msgId + ", status " + status);
        }
        int codeError = 0;

        /*// Update rich messaging history
        RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(msgId, status);*/
        ContactId contactid = ContactIdUtils.createContactIdFromTrustedData(contact);
        
        ReasonCode reasonCode = ChatUtils.getReasonCode(statusCode);
        try {
            if (status.equals(ImdnDocument.DELIVERY_STATUS_ERROR) || status.equals(ImdnDocument.DELIVERY_STATUS_FAILED)) {
                //listeners.getBroadcastItem(i).onReportMessageFailed(
                       // msgId, codeError, statusCode);
                Status mstatus = Status.FAILED;
                chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, new Date(), reasonCode);
                if (logger.isActivated()) {
                    logger.info("Write new listener");
                }
            } else if(status.equals(ImdnDocument.DELIVERY_STATUS_SENDING_FAILED)){
                Status mstatus = Status.FAILED;
                chatService.onExtendMessageDeliveryStatusReceived(mChatId,contactid, mstatus, msgId, new Date(), ReasonCode.FAILED_SEND);
                if (logger.isActivated()) {
                    logger.info("Write new listener");
                }
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't notify listener", e);
            }
        }
    }

    /**
     * Conference event
     *
     * @param contact Contact
     * @param contactDisplayname Contact display name
     * @param state State associated to the contact
     */
    public void handleConferenceEvent(
            String contact, String contactDisplayname,
            String state, String method,
            String userStateParameter, String conferenceState) {
        // Not used here
    }

    /**
     * Request to add participant is successful
    */
    public void handleAddParticipantSuccessful() {
        // Not used in single chat
    }

    /**
     * Request to add participant has failed
     *
     * @param reason Error reason
     */
    public void handleAddParticipantFailed(String reason) {
        // Not used in single chat
    }

    @Override
    public void handleModifySubjectSuccessful(String subject) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleModifySubjectFailed(int statusCode) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleTransferChairmanSuccessful(String newChairman) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleTransferChairmanFailed(int statusCode) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleRemoveParticipantSuccessful(String removedParticipant){
        // TODO Auto-generated method stub
    }

    @Override
    public void handleRemoveParticipantFailed(int statusCode){
        // TODO Auto-generated method stub
    }

    @Override
    public void handleAbortConversationResult(int reason, int code) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleSessionTerminatedByRemote(String cause, String text) {
        // TODO Auto-generated method stub
        if (logger.isActivated()) {
            logger.info("LMM handleSessionTerminatedByRemote cause" + cause);
        }
        InstantMessage msg = null;
        if (pendingMsgList.size() > 0) {
             msg = pendingMsgList.get(0);
             pendingMsgList.remove(0);
        }
        String msgId = null;
        if(msg != null)
            msgId = msg.getMessageId();
        
        if (logger.isActivated()) {
            logger.info("LMM handleMessageDeliveryStatus0 msgid; " + msgId);
        }
        
        if(msgId != null){
            synchronized (lock) {
                final int N = listeners.beginBroadcast();
                for (int i=0; i < N; i++) {
                    try {
                        //listeners.getBroadcastItem(i).onReportMessageFailed(
                           //     msgId, ExtendChat.ErrorCodes.UNKNOWN, "fallback");
                        if (logger.isActivated()) {
                            logger.info("Write new listener");
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't notify listener", e);
                        }
                    }
                }
                listeners.finishBroadcast();
            }
        }
    }

    @Override
    public void handleQuitConversationResult(int code) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleModifySubjectByRemote(String subject) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleModifyNicknameSuccessful(String contact,
            String newNickName) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleModifyNicknameFailed(String contact, int statusCode) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleModifyNicknameByRemote(String contact, String newNickname) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleTransferChairmanByRemote(String newChairman) {
        // TODO Auto-generated method stub
    }

    @Override
    public void handleConferenceNotify(String confState, List<ConferenceUser> users) {
        // TODO Auto-generated method stub
    }
    
    public void handleConferenceNotify(Map<String, ParticipantStatus> participants) {
        
    }
}
