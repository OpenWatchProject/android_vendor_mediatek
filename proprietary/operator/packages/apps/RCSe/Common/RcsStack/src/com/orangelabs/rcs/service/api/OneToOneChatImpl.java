package com.orangelabs.rcs.service.api;

import java.nio.charset.Charset;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TimeZone;
import java.util.concurrent.CopyOnWriteArrayList;

import javax2.sip.header.ContentDispositionHeader;
import javax2.sip.header.ContentLengthHeader;
import javax2.sip.header.ContentTypeHeader;
import javax2.sip.header.ExtensionHeader;
import javax2.sip.header.Header;

import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.chat.OneToOneChat;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.Status;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;
import com.gsma.services.rcs.chat.IChatMessage;
import com.gsma.services.rcs.chat.ConferenceEventData.ConferenceUser;
import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.chat.IOneToOneChat;
import com.gsma.services.rcs.chat.IOneToOneChatListener;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMode;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;

import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteConstraintException;
import android.os.AsyncTask;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.text.TextUtils;
import android.util.Base64;

import com.orangelabs.rcs.provider.messaging.ChatMessagePersistedStorageAccessor;
import com.orangelabs.rcs.service.broadcaster.IOneToOneChatEventBroadcaster;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.service.api.ExceptionUtil;
import com.orangelabs.rcs.service.api.ServerApiBaseException;
import com.orangelabs.rcs.service.api.ServerApiGenericException;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.service.ImsService;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.SessionAuthenticationAgent;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatError;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSessionListener;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.ContributionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocPush;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.OneOneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.ContactIdUtils; 
import com.orangelabs.rcs.utils.ContactUtil;
import com.orangelabs.rcs.utils.DateUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.StringUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipInterface;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;

import gov2.nist.javax2.sip.header.SIPHeader;

/**
* Chat implementation
*
 * @author Jean-Marc AUFFRET
*/
public class OneToOneChatImpl extends IOneToOneChat.Stub implements ChatSessionListener {
    /**
     * Remote contact
     */
    private String contact;

    private InstantMessagingService imsService;
    
    private IOneToOneChatEventBroadcaster mBroadcaster = null;
    
    /**
     * Core session
     */
    private OneOneChatSession session;
    
    private RcsSettings mRcsSettings = RcsSettings.getInstance();
    
    private SipInterface sipStack;
    
    private ChatServiceImpl mChatService;
    
    private RichMessagingHistory mMessagingLog;

    private CopyOnWriteArrayList<InstantMessage> mPendingMessage =
            new CopyOnWriteArrayList<InstantMessage>();
    
    private CopyOnWriteArrayList<String> messageIds =
            new CopyOnWriteArrayList<String>();

    private final static String BOUNDARY_TAG = "jXfWUFcrCxZEXdN";
    
    /**
     * List of listeners
     */
    private RemoteCallbackList<IOneToOneChatListener> listeners = new RemoteCallbackList<IOneToOneChatListener>();

    /**
     * Lock used for synchronisation
     */
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
    public OneToOneChatImpl(String contact) {
        if (logger.isActivated()) {
            logger.debug("intialize OneToOneChatImpl1");
        }
        this.contact = contact;
        this.session = null;
        imsService = Core.getInstance().getImService();
        this.sipStack = Core.getInstance().getImsModule().getSipManager().getSipStack();
        if(mRcsSettings == null){
            RcsSettings.createInstance(AndroidFactory.getApplicationContext());
            mRcsSettings = RcsSettings.getInstance();
        }
        mMessagingLog = RichMessagingHistory.getInstance();
    }
    
    /**
     * Constructor
     *
     * @param contact Remote contact
     */
    public OneToOneChatImpl(String contact, ChatServiceImpl chatService, OneOneChatSession session) {
        if (logger.isActivated()) {
            logger.debug("intialize OneToOneChatImpl2");
        }
        this.contact = contact;
        this.session = null;
        imsService = Core.getInstance().getImService();
        this.sipStack = Core.getInstance().getImsModule().getSipManager().getSipStack();
        if(mRcsSettings == null){
            RcsSettings.createInstance(AndroidFactory.getApplicationContext());
            mRcsSettings = RcsSettings.getInstance();
        }
        this.mChatService = chatService;
        mMessagingLog = RichMessagingHistory.getInstance();
        mBroadcaster = mChatService.getOneToOnEventBroadcaster();
        this.session = session;
    }

    public String getChatId(){
        return session.getRemoteContact();
    }

    /**
     * Constructor
     *
     * @param contact Remote contact
     * @param session Session
     */
    public OneToOneChatImpl(String contact, OneOneChatSession session) {
        this.contact = contact;
        this.session = session;

        session.addListener(this);
    }

    /**
     * Set core session
     *
     * @param session Core session
     */
    public void setCoreSession(OneOneChatSession session) {
        if (logger.isActivated()) {
            logger.debug("setCoreSession session: " + session);
        }
        this.session = session;

        if(session != null){
            if (logger.isActivated()) {
                logger.debug("setCoreSession add listener");
            }
            session.addListener(this);
        }
    }

    /**
     * Reset core session
     */
    public void resetCoreSession() {
        this.session = null;
    }

    /**
     * Get core session
     *
     * @return Core session
     */
    public OneOneChatSession getCoreSession() {
        return session;
    }

    /**
     * Returns the state of the group chat message
     *
     * @return State of the message
     * @see GroupChat.MessageState
     */
    public int getState(String messageId) {
        //int messageStatus = RichMessagingHistory.getInstance().getMessageStatus(messageId);
        int messageStatus = 0;
        /*switch(messageStatus){
            case ChatLog.Message.Content.Status.SENDING.toInt():
                return Chat.MessageState.SENDING;

            case ChatLog.Message.Content.Status.SENT.toInt():
                return Chat.MessageState.SENT;

            case ChatLog.Message.Content.Status.UNREAD_REPORT.toInt():
            case ChatLog.Message.Content.Status.UNREAD.toInt():
            case ChatLog.Message.Content.Status.READ.toInt():
                return Chat.MessageState.DELIVERED;

            case ChatLog.Message.Content.Status.FAILED.toInt():
                return Chat.MessageState.FAILED;

            default:
                return Chat.MessageState.FAILED;
        }*/
        return 0;
    }

    /**
     * Returns the remote contact
     *
     * @return Contact
     */
    public ContactId getRemoteContact() {
        if (logger.isActivated()) {
            logger.debug("getRemoteContact");
        }
        String contactString =  PhoneUtils.extractNumberFromUri(contact);
        ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(contactString);
        return contactId;
    }
    
    /**
     * Returns true if it is possible to send messages in this one to one chat right now, else
     * return false.
     * 
     * @return boolean
     * @throws RemoteException
     */
    @Override
    public boolean isAllowedToSendMessage() throws RemoteException {
        try {
            if (!mRcsSettings.getMyCapabilities().isImSessionSupported()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot send message on one-to-one chat with contact '"
                            + contact + "' as IM capabilities are not supported for self.");
                }
                return false;
            }
            Capabilities remoteCapabilities = ContactsManager.getInstance().getContactCapabilities(contact);
            if (remoteCapabilities == null) {
                if (logger.isActivated()) {
                    logger.debug("Cannot send message on one-to-one chat with contact '"
                            +contact + "' as the contact's capabilities are not known.");
                }
                return false;
            }
            MessagingMode mode = mRcsSettings.getMessagingMode();
            switch (mode) {
                case INTEGRATED:
                case SEAMLESS:
                    if (!mRcsSettings.isImAlwaysOn()
                            && !imsService.isCapabilitiesValid(remoteCapabilities)) {
                        if (logger.isActivated()) {
                            logger.debug("Cannot send message on one-to-one chat with contact '"
                                    + contact
                                    + "' as the contact's cached capabilities are not valid anymore for one-to-one communication.");
                        }
                        return false;
                    }
                    break;
                default:
                    break;
            }
            if (!remoteCapabilities.isImSessionSupported()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot send message on one-to-one chat with contact '"
                            + contact + "' as IM capabilities are not supported for that contact.");
                }
                return false;
            }
            return true;

        } catch (ServerApiBaseException e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.debug("isAllowedToSendMessage exception1");
            }
            throw new ServerApiGenericException(e);

        } catch (Exception e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.debug("isAllowedToSendMessage exception2");
            }
            throw new ServerApiGenericException(e);
        }
    }

    /**
     * Sends a plain text message
     *
     * @param message Text message
     * @return Unique message ID or null in case of error
     */
    public IChatMessage sendMessage(String message) {
        if (logger.isActivated()) {
            logger.debug("Send text message:" + message);
        }

        boolean isImdnActivated = false;
        try {
            isImdnActivated = Core.getInstance().getImService().getImdnManager().isImdnActivated();
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.debug("IMDN manager is null");
            }
        }
        // Create a text message
        InstantMessage msg = ChatUtils.createTextMessage(contact, message,isImdnActivated);
        
        // Send message
        String msgId = sendChatMessage(msg);
        if (logger.isActivated()) {
            logger.debug("Send text message id:" + msgId + " oldid: " + msg.getMessageId());
        }

        ChatMessagePersistedStorageAccessor persistedStorage = new ChatMessagePersistedStorageAccessor(
                mMessagingLog, msgId, msg.getRemote(), message,
                msg.getMimeType(), contact, Direction.OUTGOING);
        IChatMessage chatMsg = new ChatMessageImpl(persistedStorage);
        return chatMsg;
    }
    
    /**
     * Sends a geoloc message
     *
     * @param geoloc Geoloc
     * @return Unique message ID or null in case of error
     */
    public IChatMessage sendMessage2(Geoloc geoloc) {
        if (logger.isActivated()) {
            logger.debug("Send geoloc message");
        }

        // Create a geoloc message
        GeolocPush geolocPush = new GeolocPush(geoloc.getLabel(),
                geoloc.getLatitude(), geoloc.getLongitude(),
                geoloc.getExpiration(), geoloc.getAccuracy());

        // Create a geoloc message
        GeolocMessage msg = ChatUtils.createGeolocMessage(contact, geolocPush,
                Core.getInstance().getImService().getImdnManager().isImdnActivated());
     
     // Send message
        String msgId = sendChatMessage(msg);
        if (logger.isActivated()) {
            logger.debug("Send text message id:" + msgId + " oldid: " + msg.getMessageId());
        }
        
        ChatMessagePersistedStorageAccessor persistedStorage = new ChatMessagePersistedStorageAccessor(
                mMessagingLog, msg.getMessageId(), msg.getRemote(), geoloc.toString(),
                msg.getMimeType(), contact, Direction.OUTGOING);
        IChatMessage chatMsg = new ChatMessageImpl(persistedStorage);
        return chatMsg;
    }
    
    public void addExpiredDeliveryIntent(IChatMessage msg){
        try {
        long timestampSent = msg.getTimestampSent();
        long deliveryExpiration = getDeliveryExpirationTime(timestampSent);
        String msgId = msg.getId();
        ContactId mContact = ContactIdUtils.createContactIdFromTrustedData(contact);
        if (deliveryExpiration != 0) {
            imsService.getDeliveryExpirationManager()
                    .scheduleOneToOneChatMessageDeliveryTimeoutAlarm(mContact, msgId,
                            deliveryExpiration);
        }
        }catch(Exception e){
            if (logger.isActivated()) {
                logger.debug("addExpiredDeliveryIntent exception");
            }
            e.printStackTrace();
        }
    }

   public void resendMessage(final String msgId) {
       if (logger.isActivated()) {
            logger.debug("resendMessage MsgId:" + msgId);
        }
        final String message = RichMessagingHistory.getInstance().getMessageText(msgId);
        int msgType = RichMessagingHistory.getInstance().getMessageType(msgId);
        if (logger.isActivated()) {
            logger.debug("resendMessage message:" + message);
        }

        /*// Create a text message
        InstantMessage msg = ChatUtils.createTextMessage(this.contact, message,
                Core.getInstance().getImService().getImdnManager().isImdnActivated());
        msg.setMessageId(msgId);
        msg.setMessageType(msgType);

        int result = -1;
        int size = ChatUtils.getEncodedMessageSize(message);
        if (size < RcsSettings.getInstance().getMaxPagerContentSize())
            result = sendPagerMessage(msg);

        if (result < 0) {
            if (logger.isActivated()) {
                logger.debug("resend message fail");
            }
            RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(
                    msgId, ImdnDocument.DELIVERY_STATUS_FAILED);
        }*/
        
        new Thread() {
            public void run() {
                session.sendTextMessage(msgId,message);
            }
        }.start();
        
        
   }
   
   private int sendPagerMessage(InstantMessage message) {
       int result = 0;

       String contentType = CpimMessage.MIME_TYPE;
       
       String content = composeRequestContent(message, contentType);

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
   
   private String composeRequestContent(InstantMessage message, String contentType) {
       String content;
       String contentPart;

       if (contentType.equals(CpimMessage.MIME_TYPE)) {
           contentPart = composeCpimContent(message);
       } else {
           // Raw content
           contentPart = message.getTextMessage();
       }
       content = contentPart;
       return content;
   }
   
   private String composeCpimContent(InstantMessage message) {
       String cpim;
       
       String from = ImsModule.IMS_USER_PROFILE.getPublicUri();
       String to = ChatUtils.ANOMYNOUS_URI;
       if (RcsSettings.getInstance().supportOP01()) {
           cpim = ChatUtils.buildCpimMessageWithDeliveredImdn(
                   from,
                   to,
                   message.getMessageId(),
                   StringUtils.encodeUTF8(message.getTextMessage()),
                   InstantMessage.MIME_TYPE);
       }
       else if (Core.getInstance().getImService().getImdnManager().isImdnActivated()) {
           cpim = ChatUtils.buildCpimMessageWithImdn(
                   from,
                   to,
                   message.getMessageId(),
                   StringUtils.encodeUTF8(message.getTextMessage()),
                   InstantMessage.MIME_TYPE);
       }
       else {
           cpim = ChatUtils.buildCpimMessage(
                   from,
                   to,
                   StringUtils.encodeUTF8(message.getTextMessage()),
                   InstantMessage.MIME_TYPE);
       }

       return cpim;
   }
   
   private SipRequest createSipMessageRequest(
           int msgType, String contentType, String content) {
       ArrayList<String> featureTagList = new ArrayList<String>();
       String preferService = null;

       featureTagList.add(FeatureTags.FEATURE_RCSE_PAGER_MSG);

       String remoteContact = PhoneUtils.formatNumberToSipUri(contact);

       SipDialogPath dialogPath = new SipDialogPath(
               sipStack,
               sipStack.generateCallId(),
               1,
               remoteContact,
               ImsModule.IMS_USER_PROFILE.getPublicUri(),
               remoteContact,
               sipStack.getServiceRoutePath());

       try {
           String[] featureTags = new String[featureTagList.size()];
           for (int i = 0; i < featureTags.length; i++) {
               featureTags[i] = featureTagList.get(i);
           }
           SipRequest request = SipMessageFactory.createMessageCPM(
                       dialogPath, featureTags, preferService, contentType, content);

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

       SipTransactionContext ctx = null;
       try {
           ctx = Core.getInstance().getImService().getImsModule().getSipManager().sendSipMessageAndWait(request);
           // Analyze received message
           if ((ctx.getStatusCode() == 200) || (ctx.getStatusCode() == 202)) {
               // 200 OK received
               if (logger.isActivated()) {
                   logger.info("20x OK response received");
               }
               handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_SENT, null, null);
           }  else {
               // Error responses
               if (logger.isActivated()) {
                   logger.info("failed: " + ctx.getStatusCode() + " response received");
               }
               handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_ERROR, null, null);
           }
       } catch (Exception e) {
           if (logger.isActivated()) {
               logger.info("exception while sending pager message");
           }
           handleMessageDeliveryStatus(msgId, ImdnDocument.DELIVERY_STATUS_ERROR, null, null);
       }
   }

    /**
     * Sends a chat message
     *
     * @param msg Message
     * @return Unique message ID or null in case of error
     */
    private String sendChatMessage(final InstantMessage msg) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.debug("sendChatMessage msgId: " + msg.getMessageId() + ", text: " + msg.getTextMessage() +
                        ", session: " + session);
            }

            // Check if a session should be initiated or not
            if ((session == null) ||
                    session.getDialogPath().isSessionTerminated() ||
                    !session.getDialogPath().isSessionEstablished()) {
                try {
                    if (logger.isActivated()) {
                        logger.debug("ABC Core session is not yet established:"
                                + " initiate a new session to send the message");
                    }

                    // Initiate a new session
                    session = (OneOneChatSession)Core.getInstance().getImService()
                            .initiateOne2OneChatSession(contact, msg);

                    // Update with new session
                    setCoreSession(session);

                    // Update rich messaging history
                    if(! RichMessagingHistory.getInstance().isOne2OneMessageExists(
                            msg.getMessageId())){

                        RichMessagingHistory.getInstance().addChatMessage(
                                msg, Direction.OUTGOING.toInt());
                    }
                    if(!RcsSettings.getInstance().isFirstMessageInInvite()){
                      mPendingMessage.add(msg);                     
                    }
                    // Start the session
                    Thread t = new Thread() {
                        public void run() {
                            session.startSession();
                        }
                    };
                    t.start();
                    return session.getFirstMessage().getMessageId();
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("ABC Can't send a new chat message", e);
                    }
                    return null;
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("ABC Core session is established:"
                            + " use existing one to send the message");
                }

                // Generate a message Id
                final String msgId = msg.getMessageId();

                // Send message
                Thread t = new Thread() {
                    public void run() {
                        if (msg instanceof GeolocMessage) {
                            session.sendGeolocMessage(msgId, ((GeolocMessage)msg).getGeoloc());
                        } else {
                            session.sendTextMessage(msg.getMessageId(), msg.getTextMessage());
                        }
                    }
                };
                t.start();
                return msgId;
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
                logger.info("LMM Set displayed delivery report for "
                        + msgId + "session: " + session);
            }
            // Send delivery status
            if ((session != null) &&
                    (session.getDialogPath() != null) &&
                    (session.getDialogPath().isSessionEstablished()) &&
                    session.getMsrpMgr() != null &&
                    session.getMsrpMgr().getMsrpSession() != null &&
                    session.getMsrpMgr().getMsrpSession().getConnection() != null &&
                    session.getMsrpMgr().getMsrpSession().getConnection().getSender() != null &&
                    !session.isLargeMessageMode()) {
                // Send via MSRP
                Thread t = new Thread() {
                    public void run() {
                        session.sendMsrpMessageDeliveryStatus(
                                session.getRemoteContact(),
                                msgId,
                                ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                    }
                };
                t.start();
            } else {
                
                if (logger.isActivated()) {
                    logger.info("LMM Set displayed delivery report for1 " + "contact: " + contact);
                }
                String subStr =  contact.substring(0,3);
                String newContact = null;
                if(!(subStr.equals("sip") || subStr.equals("tel"))){
                    newContact = "tel:" + contact ;
                }
                else{
                    newContact = contact;
                }
                newContact = PhoneUtils.formatNumberToSipUri(contact);
                if (logger.isActivated()) {
                    logger.info("LMM Set displayed delivery report for1 "
                            + "newContact: " + newContact + "subStr: " + subStr);
                }
                // Send via SIP MESSAGE
                Core.getInstance().getImService().getImdnManager().sendMessageDeliveryStatus(
                        newContact, msgId, ImdnDocument.DELIVERY_STATUS_DISPLAYED);
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("LMM Could not send MSRP delivery status",e);
            }
        }
    }

    /**
     * Sends an is-composing event. The status is set to true when
     * typing a message, else it is set to false.
     *
     * @param status Is-composing status
     */
    public void setComposingStatus(final boolean status) {
        boolean isComposing = RcsSettings.getInstance().isComposingSupported();
        if (logger.isActivated()) {
            logger.info("LMM sendIsComposingEvent status" + status + " iscomposing: " + isComposing);
        }
        if(session == null || session.getMsrpMgr() == null || session.getMsrpMgr().getMsrpSession() == null) {
            if (logger.isActivated()) {
                logger.info("LMM sendIsComposingEvent session parameters is null");
            }
            return;
        }
        if (logger.isActivated()) {
            logger.info("LMM sendIsComposingEvent mode" + session.isLargeMessageMode());
        }
        
        if (!session.isLargeMessageMode() && isComposing ) {
            Thread t = new Thread() {
                public void run() {
                    session.sendIsComposingStatus(status);
                }
            };
            t.start();
        }
    }
    
    /**
     * open the chat conversation. 
     * 
     * @throws RemoteException
     */
    public void openChat() throws RemoteException {
        Runnable run = new Runnable() {
            public void run() {
                if (logger.isActivated()) {
                    logger.info("openChat with  " + contact);
                }
                try {/*
                    ImSessionStartMode imSessionStartMode = mRcsSettings.getImSessionStartMode();
                    if (ImSessionStartMode.ON_OPENING == imSessionStartMode) {
                        mImService.acceptStoreAndForwardMessageSessionIfSuchExists(mContact);
                    }*/
                    if(session == null){
                        session = imsService.getOneToOneChatSession(contact);
                    }
                    if (session == null) {
                        /*
                         * If there is no session ongoing right now then we do not need to open
                         * anything right now so we just return here. A sending of a new message on
                         * this one-to-one chat will anyway result in creating a new session so we
                         * do not need to do anything more here for now.
                         */
                        return;
                    }
                    if (!session.getDialogPath().isSessionEstablished()) {
                        if (!session.isInitiatedByRemote()) {
                            /*
                             * This method needs to accept pending invitation if
                             * IM_SESSION_START_MODE is 0, which is not applicable if session is
                             * remote originated so we return here.
                             */
                            return;
                        }
                        if (logger.isActivated()) {
                            logger.debug("Accept one-to-one chat session with contact " + contact);
                        }
                        session.acceptSession();
                    }
                } catch (RuntimeException e) {
                    /*
                     * Normally we are not allowed to catch runtime exceptions as these are genuine
                     * bugs which should be handled/fixed within the code. However the cases when we
                     * are executing operations on a thread unhandling such exceptions will
                     * eventually lead to exit the system and thus can bring the whole system down,
                     * which is not intended.
                     */
                    if (logger.isActivated()) {
                        logger.debug("openChat failed with contact " + contact);
                    }
                }
            }
        };
        Thread th =  new Thread(run);
        th.start();
    }
    
    private long getDeliveryExpirationTime(long timestampSent) {
        if (mRcsSettings.isImAlwaysOn()) {
            return 0;
        }
        final long timeout = mRcsSettings.getMsgDeliveryTimeoutPeriod();
        if (timeout == 0L) {
            return 0;
        }
        return timestampSent + timeout;
    }

    /**
     * Adds a listener on chat events
     *
     * @param listener Chat event listener
     */
    public void addEventListener(IOneToOneChatListener listener) {
        if(listener == null) return;
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
    public void removeEventListener(IOneToOneChatListener listener) {
        if(listener == null) return;
        if (logger.isActivated()) {
            logger.info("LMM Remove an event listener");
        }

        synchronized(lock) {
            listeners.unregister(listener);
        }
    }

    /*------------------------------- SESSION EVENTS ----------------------------------*/

    /**
     * Session is started
     */
    public void handleSessionStarted() {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("LMM Session started");
            }

            if (logger.isActivated()) {
                logger.info("handleSessionStarted size: " + mPendingMessage.size());
            }
            synchronized(lock) {
                while(mPendingMessage.size() > 0) {
                    final InstantMessage msg = mPendingMessage.get(0);
                    
                    if (logger.isActivated()) {
                        logger.info("LMM Session started: send message" + msg.getTextMessage());
                    }

                    new Thread() {

                        public void run() {
                            session.sendTextMessage(msg.getMessageId(), msg.getTextMessage());
                        }

                    }.start();
                    mPendingMessage.remove(0);
                }
               
            }
            if (logger.isActivated()) {
                logger.info("LMM Session started: send display Size: " + messageIds.size());
            }
            Iterator<String> msgIdIterator = messageIds.iterator();
            while(msgIdIterator.hasNext()) {
                final String msgId = msgIdIterator.next();
                if (logger.isActivated()) {
                    logger.info("LMM Session started: Send display MSGID: " + msgId);
                }
               new Thread() {
                    public void run() {
                        if (logger.isActivated()) {
                            logger.info("LMM Session started: Send display MSGID1: " + msgId);
                        }
                        session.sendMsrpMessageDeliveryStatus(
                                session.getRemoteContact(),
                                msgId,
                                ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                    }
                }.start();
            }
            messageIds.clear();
        }
    }

    /**
     * Session has been aborted
     *
     * @param reason Termination reason
     */
    public void handleSessionAborted(int reason) {
        synchronized(lock) {
            String remoteContact = session.getRemoteContact();
            String number = PhoneUtils.extractNumberFromUri(remoteContact);
            if (logger.isActivated()) {
                logger.error("LMM handleSessionAborted: " + remoteContact + "Number: " + number);
            }

            // Update rich messaging history
            // Nothing done in database

            // Remove session from the list
            if(session.isStoreAndForward()){
                if (logger.isActivated()) {
                    logger.debug("LMM AddRemove remove storeChatSessions "
                            + session.getSessionID());
                }
                ChatServiceImpl.removeStoreChatSession(number);
            }
            else{
                if (logger.isActivated()) {
                    logger.debug("LMM AddRemove remove chatSessions " + session.getSessionID());
                }
                ChatServiceImpl.removeChatSession(number);
            }
        }
    }

    /**
     * Session has been terminated by remote
     */
    public void handleSessionTerminatedByRemote() {
        synchronized(lock) {

            String remoteContact = session.getRemoteContact();
            String number = PhoneUtils.extractNumberFromUri(remoteContact);
            if (logger.isActivated()) {
                logger.error("LMM handleSessionTerminatedByRemote: " + remoteContact + "Number: " + number);
            }

            // Update rich messaging history
            // Nothing done in database

            // Remove session from the list
            if(session.isStoreAndForward()){
                if (logger.isActivated()) {
                    logger.debug("LMM AddRemove remove storeChatSessions "
                            + session.getSessionID());
                }
                ChatServiceImpl.removeStoreChatSession(number);
            }
            else{
                if (logger.isActivated()) {
                    logger.debug("LMM AddRemove remove chatSessions " + session.getSessionID());
                }
                ChatServiceImpl.removeChatSession(number);
            }
        }
    }

    /**
     * New text message received
     *
     * @param text Text message
     */
    public void handleReceiveMessage(InstantMessage message) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("LMM handleReceiveMessage New IM received: "
                        + message.getTextMessage());
            }

            // Update rich messaging history
            try {
                RichMessagingHistory.getInstance().addChatMessage(
                        message, Direction.INCOMING.toInt());
            } catch (SQLException e) {
                if (e instanceof SQLiteConstraintException) {
                    logger.info("This message may be self-sent\nUpdate MessageId");
                    message.setMessageId("my_" + message.getMessageId());
                    RichMessagingHistory.getInstance().addChatMessage(
                            message, Direction.INCOMING.toInt());
                }
            }

            // Create a chat message
           /* ChatMessage msgApi = null;
            msgApi = new ChatMessage(
                        message.getMessageId(),
                        PhoneUtils.extractNumberFromUri(message.getRemote()),
                        message.getTextMessage(),
                        InstantMessage.MIME_TYPE,
                        message.getDisplayName(),
                        Direction.INCOMING.toInt(),
                        message.getServerDate().getTime(),0,
                        0,0,
                        ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt(),
                        0,getChatId(),
                        false,false
                        );*/
            mBroadcaster.broadcastMessageReceived(message.getMimeType(), message.getMessageId());
            // Broadcast intent related to the received invitation
            /*Intent intent = new Intent(ChatIntent.ACTION_NEW_CHAT);
            intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
            try{
                intent.putExtra(ChatIntent.EXTRA_CONTACT, msgApi.getRemoteContact().toString());
            } catch(Exception e){
                e.printStackTrace();
            }
            intent.putExtra(ChatIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
            intent.putExtra(ChatIntent.EXTRA_MESSAGE, msgApi);
            AndroidFactory.getApplicationContext().sendBroadcast(intent);

            // Notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("LMM handleReceiveMessage N:" + N);
            }
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onNewMessage(msgApi);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();*/
        }
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
            /*org.gsma.joyn.chat.GeolocMessage msgApi = new org.gsma.joyn.chat.GeolocMessage(
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
                                                                false,false,geolocApi);*/

            // Broadcast intent related to the received invitation
            mBroadcaster.broadcastMessageReceived(geoloc.getMimeType(), geoloc.getMessageId());
            /*Intent intent = new Intent(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE);
            intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
            try{
                intent.putExtra(ChatIntent.EXTRA_CONTACT, PhoneUtils.extractNumberFromUri(geoloc.getRemote()));
            } catch(Exception e){
                e.printStackTrace();
            }
            intent.putExtra(ChatIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
            intent.putExtra(ChatIntent.EXTRA_MESSAGE, msgApi);
            AndroidFactory.getApplicationContext().sendBroadcast(intent);

            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onNewGeoloc(msgApi);
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
        }*/
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
            if (mMessagingLog.setChatMessageStatusAndReasonCode(msgId, status, reasonCode)) {
                mBroadcaster.broadcastMessageStatusChanged(contactid, mimeType, msgId, mstatus,
                        mcode);
            }
        }
    }

    /**
     * IM session error
     *
     * @param error Error
     */
    public void handleImError(ChatError error,String msgId) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.error("LMM IM error " + error.getErrorCode() + "; status: " + error.getMessage() + ", msgId: " + msgId);
            }
            String remoteContact = session.getRemoteContact();
            String number = PhoneUtils.extractNumberFromUri(remoteContact);
            String status = error.getMessage();

            if (logger.isActivated()) {
                logger.info("LMM handleImError: " + remoteContact + "Number: " + number);
            }
            try {
                if(msgId == null){
                    InstantMessage firstMessage = session.getFirstMessage();
                    if(firstMessage == null)firstMessage = mPendingMessage.get(0);
                    if(firstMessage != null){
                        msgId = firstMessage.getMessageId();
                    }
                }
            } catch (Exception e1) {
                if (logger.isActivated()) {
                    logger.info("LMM handleImError exception: " + msgId);
                }
                e1.printStackTrace();
            }
            if (logger.isActivated()) {
                logger.info("LMM handleImError: " + msgId);
            }

            // Update rich messaging history
            switch(error.getErrorCode()){
                case ChatError.SESSION_INITIATION_FAILED:
                case ChatError.SESSION_INITIATION_CANCELLED:
                case ChatError.MEDIA_SESSION_FAILED:
                  /*  RichMessagingHistory.getInstance().updateChatMessageStatus(
                            session.getFirstMessage().getMessageId(),
                            ChatLog.Message.Content.Status.FAILED.toInt());*/
                    // Notify event listeners
                    try {
                        if(status.contains("timeout") && RcsSettings.getInstance().isFallbackToPagerModeSupported()){
                            //listeners.getBroadcastItem(i).onReportFailedMessage(session.getFirstMessage().getMessageId(),error.getErrorCode(),error.getMessage());
                            setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.TIMEOUT.toInt());
                            
                        } else {
                            //listeners.getBroadcastItem(i).onReportMessageFailed(session.getFirstMessage().getMessageId());
                            setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.UNSPECIFIED.toInt());
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't notify listener", e);
                        }
                    }
                    break;
                case ChatError.SESSION_INITIATION_FALLBACK:
                    setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.FALLABCK_PAGER.toInt());
                    break;
                case ChatError.SESSION_INITIATION_FALLBACK_MMS:
                    setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.FALLBACK.toInt());
                    break;
                default:
                    setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.UNSPECIFIED.toInt());
                    break;
            }

            if (logger.isActivated()) {
                logger.info("LMM handleImError: check if session can be removed");
            }
            int errorCode = error.getErrorCode();
            if((session.getDialogPath().isSessionTerminated() &&
                    !session.getDialogPath().isSessionEstablished()) || errorCode == ChatError.MEDIA_SESSION_FAILED){
                // Remove session from the list
                if(session.isStoreAndForward()){
                    if (logger.isActivated()) {
                        logger.debug("LMM AddRemove remove storeChatSessions "
                                + session.getSessionID());
                    }
                    ChatServiceImpl.removeStoreChatSession(number);
                }
                else{
                    if (logger.isActivated()) {
                        logger.debug("LMM AddRemove remove chatSessions " + session.getSessionID());
                    }
                    ChatServiceImpl.removeChatSession(number);
                }
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
            
            final int N = listeners.beginBroadcast();
            
            String msgId = null;
            try {
                InstantMessage firstMsg = session.getFirstMessage();
                if(firstMsg == null) {
                    firstMsg = mPendingMessage.get(0);
                }
                msgId = firstMsg.getMessageId();
            } catch(Exception e){
                e.printStackTrace();
            }
            
            // Update rich messaging history
            switch(error.getErrorCode()){
                case ChatError.SESSION_INITIATION_ERROR:
                    /*RichMessagingHistory.getInstance().updateChatMessageStatus(session.getFirstMessage().getMessageId(),
                            ChatLog.Message.Content.Status.FAILED.toInt());*/
                    // TODO: notify listener
                    // Notify event listeners
                    
                    if (logger.isActivated()) {
                        logger.info("LMM SESSION_INITIATION_ERROR N:" + N);
                    }
                    try {
                        //listeners.getBroadcastItem(i).onReportMessageInviteFailed(session.getFirstMessage().getMessageId());
                        setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.UNSPECIFIED.toInt());
                        if (logger.isActivated()) {
                            logger.info("Write new listener");
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't notify listener", e);
                        }
                    }
                    break;
                case ChatError.SESSION_INITIATION_DECLINED:
                   /* RichMessagingHistory.getInstance().updateChatMessageStatus(session.getFirstMessage().getMessageId(),
                            ChatLog.Message.Content.Status.FAILED.toInt());*/
                    // TODO: notify listener
                    // Notify event listeners
                    if (logger.isActivated()) {
                        logger.info("LMM SESSION_DECLINED_ERROR N:" + N);
                    }
                    try {
                        //listeners.getBroadcastItem(i).onReportMessageInviteForbidden(session.getFirstMessage().getMessageId(),"Decline");
                        setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),ReasonCode.DECLINED.toInt());
                        if (logger.isActivated()) {
                            logger.info("Write new listener");
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't notify listener", e);
                        }
                    }
                    break;
                case ChatError.SESSION_FORBIDDEN_ERROR:
                   /* RichMessagingHistory.getInstance().updateChatMessageStatus(session.getFirstMessage().getMessageId(),
                            ChatLog.Message.Content.Status.FAILED.toInt());*/
                    // TODO: notify listener
                    // Notify event listeners
                    if (logger.isActivated()) {
                        logger.info("LMM SESSION_FORBIDDEN_ERROR N:" + N);
                    }
                    try {
                        //listeners.getBroadcastItem(i).onReportMessageInviteForbidden(session.getFirstMessage().getMessageId(),error.getMessage());
                        ReasonCode reasonCode = ChatUtils.getReasonCode(error.getMessage());
                        setChatMessageStatusAndReasonCode(msgId,InstantMessage.MIME_TYPE,Status.FAILED.toInt(),reasonCode.toInt());
                        if (logger.isActivated()) {
                            logger.info("Write new listener");
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't notify listener", e);
                        }
                    }
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
            listeners.finishBroadcast();
        }
    }

    /**
     * Is composing event
     *
     * @param contact Contact
     * @param status Status
     */
    public void handleIsComposingEvent(String contact, boolean status) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleIsComposingEvent contact: " + contact + ", status: " + status);
            }
            
            

            // Notify event listeners
            try {
                //listeners.getBroadcastItem(i).onComposingEvent(status);
                contact = PhoneUtils.extractNumberFromUri(contact);
                if (logger.isActivated()) {
                    logger.info("handleIsComposingEvent contact: " + contact);
                }
                
                ContactId contactid = ContactIdUtils.createContactIdFromTrustedData(contact);
                mBroadcaster.broadcastComposingEvent(contactid, status);
                if (logger.isActivated()) {
                    logger.info("Write new listener");
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
    public void handleMessageDeliveryStatus(String msgId, String status, String contact, Date date) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleMessageDeliveryStatusO2O1 "
                        + msgId + ", status " + status + ", contact: " + contact);
            }
            if(contact == null){
                contact = this.contact;
            }
            try {
                contact = PhoneUtils.extractNumberFromUri(contact);
            } catch(Exception e){
                e.printStackTrace();
            }
            ContactId contactid = ContactIdUtils.createContactIdFromTrustedData(contact);

            logger.info("LMM handleMessageDeliveryStatus1 contact:" + contact);

            // Update rich messaging history
            /*RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(msgId, status);*/

            try {
                if (status.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
                    //listeners.getBroadcastItem(i).onReportMessageDelivered(msgId);
                    Status mstatus = Status.DELIVERED;
                    mChatService.onOneToOneMessageDeliveryStatusReceived(contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
                    //listeners.getBroadcastItem(i).onReportMessageDisplayed(msgId);
                    Status mstatus = Status.DISPLAYED;
                    mChatService.onOneToOneMessageDeliveryStatusReceived(contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_ERROR) || status.equals(ImdnDocument.DELIVERY_STATUS_FAILED)) {
                    //listeners.getBroadcastItem(i).onReportMessageFailed(msgId);
                    Status mstatus = Status.FAILED;
                    mChatService.onOneToOneMessageDeliveryStatusReceived(contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_SENDING_FAILED)) {
                    //listeners.getBroadcastItem(i).onReportMessageFailed(msgId);
                    Status mstatus = Status.FAILED;
                    mChatService.onOneToOneMessageDeliveryStatusReceived(contactid, mstatus, msgId, date, ReasonCode.FAILED_SEND);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_SENT)) {
                    //listeners.getBroadcastItem(i).onReportMessageSent(msgId);
                    Status mstatus = Status.SENT;
                    mChatService.onOneToOneMessageDeliveryStatusReceived(contactid, mstatus, msgId, date, ReasonCode.UNSPECIFIED);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
            int N =1;
            if(N == 0){
             // Broadcast intent related to the received invitation
                if (logger.isActivated()) {
                    logger.info("handleMessageDeliveryStatus0 Broadcast N: " + N);
                }
                Intent intent = new Intent(OneToOneChatIntent.ACTION_REINITIALIZE_LISTENER);
                intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
                intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, msgId);
                intent.putExtra(OneToOneChatIntent.EXTRA_STATUS, status);
                intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT, contact);
                AndroidFactory.getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
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
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.error("New message delivery status for message " + msgId + ", status " + status + ", Statuscode:" + statusCode + ", errorcode:" + errorCode + " ,Contact:" + contact);
            }
            int codeError = 0;
            try{
                contact = PhoneUtils.extractNumberFromUri(contact);
            } catch(Exception e){
                e.printStackTrace();
            }

            logger.info("LMM handleMessageDeliveryStatusO2O2 contact:" + contact);
            // Update rich messaging history
            /*RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(msgId, status);*/

            // Remove session from the list
            if(statusCode != null && statusCode.contains("403")){
                if(session.isStoreAndForward()){
                    if (logger.isActivated()) {
                        logger.debug("handleMessageDeliveryStatus Remove  storeChatSessions " + session.getSessionID());
                    }
                    ChatServiceImpl.removeStoreChatSession(contact);
                } else {
                    if (logger.isActivated()) {
                        logger.debug("handleMessageDeliveryStatus Remove  chatSessions " + session.getSessionID());
                    }
                    ChatServiceImpl.removeChatSession(contact);
                }
            }
            try {
                if (status.equals(ImdnDocument.DELIVERY_STATUS_ERROR) || status.equals(ImdnDocument.DELIVERY_STATUS_FAILED) || status.equals(ImdnDocument.DELIVERY_STATUS_SENDING_FAILED)) {
                    if (logger.isActivated()) {
                        logger.info("LMM handleMessageDeliveryStatus reportfailedmessage:");
                    }
                    //listeners.getBroadcastItem(i).onReportFailedMessage(msgId,codeError,statusCode);
                    ReasonCode reasonCode = ChatUtils.getReasonCode(statusCode);
                    ContactId contactid = ContactIdUtils.createContactIdFromTrustedData(contact);
                    if(status.equals(ImdnDocument.DELIVERY_STATUS_SENDING_FAILED)){
                        mChatService.onOneToOneMessageDeliveryStatusReceived(contactid, Status.FAILED, msgId, new Date(), ReasonCode.FAILED_SEND);
                    } else {
                        mChatService.onOneToOneMessageDeliveryStatusReceived(contactid, Status.FAILED, msgId, new Date(), reasonCode);
                    }
                    
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
            int N= 1;
            if (logger.isActivated()) {
                logger.info("handleReceiveMessage Bropadcast0 N1: " + N);
            }
            if(N == 0){
             // Broadcast intent related to the received invitation
                if (logger.isActivated()) {
                    logger.info("handleMessageDeliveryStatus1 Broadcast N: " + N);
                }
                Intent intent = new Intent(OneToOneChatIntent.ACTION_REINITIALIZE_LISTENER);
                intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
                intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, msgId);
                intent.putExtra(OneToOneChatIntent.EXTRA_STATUS, status);
                intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT, contact);
                //AndroidFactory.getApplicationContext().sendBroadcast(intent);
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
