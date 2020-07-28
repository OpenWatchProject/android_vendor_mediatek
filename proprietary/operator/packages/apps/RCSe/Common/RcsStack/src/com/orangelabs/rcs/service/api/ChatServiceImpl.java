package com.orangelabs.rcs.service.api;

import java.io.ByteArrayInputStream;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.TreeSet;
import java.util.Vector;
import java.net.URLDecoder;

import com.gsma.services.rcs.RcsService.ReadStatus;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.Status;
import com.gsma.services.rcs.Direction;
import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.RcsService.Build;
import com.gsma.services.rcs.IRcsServiceRegistrationListener;
import com.gsma.services.rcs.ICommonServiceConfiguration;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.IChatMessage;
import com.gsma.services.rcs.chat.ChatServiceConfiguration;
import com.gsma.services.rcs.chat.IChatServiceConfiguration;
import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.GroupChat.State;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.chat.IOneToOneChat;
import com.gsma.services.rcs.chat.IExtendChat;
import com.gsma.services.rcs.chat.IOneToOneChatListener;
import com.gsma.services.rcs.chat.IExtendChatListener;
import com.gsma.services.rcs.chat.IChatService;
import com.gsma.services.rcs.chat.IGroupChat;
import com.gsma.services.rcs.chat.IGroupChatListener;
import com.gsma.services.rcs.chat.IGroupChatSyncingListener;
import com.gsma.services.rcs.chat.INewChatListener;
import com.gsma.services.rcs.chat.IOneToOneChatListener;
import com.gsma.services.rcs.chat.IExtendChatListener;
import com.gsma.services.rcs.chat.IGroupChatListener;
import com.gsma.services.rcs.chat.ConferenceEventData;
import com.gsma.services.rcs.chat.ConferenceEventData.ConferenceUser;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import org.xml.sax.InputSource;

import android.text.TextUtils;
import android.content.Intent;
import android.database.Cursor;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

import com.orangelabs.rcs.provider.messaging.ChatMessagePersistedStorageAccessor;
import com.orangelabs.rcs.service.api.ServerApiIllegalArgumentException;
import com.orangelabs.rcs.service.api.ExceptionUtil;
import com.orangelabs.rcs.service.api.ServerApiBaseException;
import com.orangelabs.rcs.service.broadcaster.ExtendChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.GroupChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.IGroupChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.OneToOneChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.IOneToOneChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.IExtendChatEventBroadcaster;
import com.orangelabs.rcs.provider.CursorUtil;
import com.orangelabs.rcs.provider.messaging.MessageData;
import com.orangelabs.rcs.service.api.ServerApiPersistentStorageException;
import com.orangelabs.rcs.utils.ContactIdUtils;
import com.orangelabs.rcs.utils.ContactUtil;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.SubscribeRequest;
import com.orangelabs.rcs.core.ims.service.SubscriptionManager;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.ContributionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatInfo;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.OneOneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.StandaloneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.event.ConferenceInfoDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.event.ConferenceInfoParser;
import com.orangelabs.rcs.core.ims.service.im.chat.event.GroupListDocument;
import com.orangelabs.rcs.core.ims.service.im.chat.event.GroupListDocument.BasicGroupInfo;
import com.orangelabs.rcs.core.ims.service.im.chat.event.GroupListParser;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnManager;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.core.ims.network.registration.RegistrationInfo;
import com.orangelabs.rcs.core.ims.network.registration.RegistrationInfoParser;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Chat service implementation
 *
 * @author Jean-Marc AUFFRET
 */
public class ChatServiceImpl extends IChatService.Stub {
    /**
     * List of service event listeners
     */
    private RemoteCallbackList<IRcsServiceRegistrationListener> serviceListeners = new RemoteCallbackList<IRcsServiceRegistrationListener>();

    private final OneToOneChatEventBroadcaster mOneToOneChatEventBroadcaster = new OneToOneChatEventBroadcaster();

    private final ExtendChatEventBroadcaster mExtendChatEventBroadcaster = new ExtendChatEventBroadcaster();

    private final GroupChatEventBroadcaster mGroupChatEventBroadcaster = new GroupChatEventBroadcaster();

    /**
     * List of chat sessions
     */
    private static Hashtable<String, IOneToOneChat> chatSessions = new Hashtable<String, IOneToOneChat>();

    /**
     * List of chat sessions
     */
    private static Hashtable<String, IExtendChat> extendChatSessions = new Hashtable<String, IExtendChat>();

    /**
     * List of chat sessions
     */
    private static Hashtable<String, IOneToOneChat> storeForwardChatSessions = new Hashtable<String, IOneToOneChat>();

    /**
     * List of group chat sessions
     */
    private static Hashtable<String, IGroupChat> groupChatSessions = new Hashtable<String, IGroupChat>();

    /**
     * List of file chat invitation listeners
     */
    private RemoteCallbackList<INewChatListener> listeners = new RemoteCallbackList<INewChatListener>();

    /**
     * List of file chat invitation listeners
     */
    private RemoteCallbackList<IOneToOneChatListener> o2oListeners = new RemoteCallbackList<IOneToOneChatListener>();

    /**
     * List of file chat invitation listeners
     */
    private RemoteCallbackList<IGroupChatListener> groupListeners = new RemoteCallbackList<IGroupChatListener>();

    /**
     * List of file chat invitation listeners
     */
    private RemoteCallbackList<IExtendChatListener> extendListeners = new RemoteCallbackList<IExtendChatListener>();

    private final RcsSettings mRcsSettings = RcsSettings.getInstance();

    private InstantMessagingService imsService;

    private RichMessagingHistory messagingLog = RichMessagingHistory.getInstance();

    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger(ChatServiceImpl.class.getName());

    /**
     * Lock used for synchronization
     */
    private Object lock = new Object();

    /**
     * Constructor
     */
    public ChatServiceImpl() {
        imsService = Core.getInstance().getImService();
        if(messagingLog == null){
            RichMessagingHistory.createInstance(AndroidFactory.getApplicationContext());
            messagingLog = RichMessagingHistory.getInstance();
        }
    }

    /**
     * Close API
     */
    public void close() {
        // Clear list of sessions
        chatSessions.clear();
        extendChatSessions.clear();
        storeForwardChatSessions.clear();
        groupChatSessions.clear();

        if (logger.isActivated()) {
            logger.info("Chat service API is closed");
        }
    }

    /**
     * Returns true if the service is registered to the platform, else returns false
     *
     * @return Returns true if registered else returns false
     */
    public boolean isServiceRegistered() {
        return ServerApiUtils.isImsConnected();
    }

    public int getServiceRegistrationReasonCode(){
        return 0;
    }

    /**
     * Registers a listener on service registration events
     *
     * @param listener Service registration listener
     */
    public void addEventListener(IRcsServiceRegistrationListener listener) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Add a service listener");
            }

            serviceListeners.register(listener);
        }
    }

    /**
     * Unregisters a listener on service registration events
     *
     * @param listener Service registration listener
     */
    public void removeEventListener(IRcsServiceRegistrationListener listener) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Remove a service listener");
            }

            serviceListeners.unregister(listener);
        }
    }

    /**
     * Receive registration event
     *
     * @param state Registration state
     */
    public void notifyRegistrationEvent(boolean state) {
        // Notify listeners
        synchronized(lock) {
            final int N = serviceListeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    if (state) {
                        //serviceListeners.getBroadcastItem(i).onServiceRegistered();
                    } else {
                        //serviceListeners.getBroadcastItem(i).onServiceUnregistered();
                    }
                    if (logger.isActivated()) {
                        logger.info("write calback");
                    }
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("Can't notify listener", e);
                    }
                }
            }
            serviceListeners.finishBroadcast();
        }
    }

    /**
     * New pager message received
     *
     * @param message Message received
     */
    public void receivePagerMessage(InstantMessage message, String conversationId) {
        if (logger.isActivated()) {
            logger.info("receivePagerMessage " + message.getRemote() + ", msgId: " + message.getMessageId());
        }

        String contact = message.getRemote();
        String number;
        if (message.getDirection() == Direction.OUTGOING.toInt()) {
            number = PhoneUtils.extractNumberFromUri(contact);
        } else
        if (message.getMessageType() == ChatLog.Message.Type.PUBLIC ||
            message.toSecondary()) {
            number = PhoneUtils.extractUuidFromUri(contact);
        } else {
            number = PhoneUtils.extractNumberFromUri(contact);
        }

        if (logger.isActivated()) {
            logger.info("receivePagerMessage " + number);
        }

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();

        if (RcsSettings.getInstance().isCPMSupported()) {
            String saved = rmHistory.getCoversationID(number, 1);

            if (saved.isEmpty()) {
                // This is the 1st message for the conversation, save conversation ID here
                InstantMessage conversationMsg = ChatUtils.createTextMessage(contact, "system", false);
                rmHistory.addChatSystemMessage(conversationMsg, Direction.INCOMING.toInt());
                rmHistory.UpdateCoversationID(number, conversationId, 1);
            } else
            if (!saved.equals(conversationId)) {
                if (conversationId != null) {
                    if (logger.isActivated()) {
                       logger.info("Receive stand alone chat invitation OldId: " + saved + "NewId: " + conversationId);
                    }
                    rmHistory.UpdateCoversationID(number, conversationId, 1);
                } else {
                    if (logger.isActivated()) {
                        logger.info("Receive stand alone chat invitation Conversation Id is null");
                     }
                }
            }
        }

        synchronized(lock) {
            Object[] sessions = extendChatSessions.values().toArray();
            int i;
            for (i = 0; i < sessions.length; i++) {
                List<String> contacts = ((ExtendChatImpl)sessions[i]).getRemoteContacts();
                if (logger.isActivated()) {
                    logger.info("receivePagerMessage contact: " + contact);
                }
                if (contacts.contains(number)) {
                    if (logger.isActivated()) {
                        logger.info("LMM extendChat is not null");
                    }
                    ((ExtendChatImpl)sessions[i]).handleReceiveMessage(message);
                    break;
                }
            }
            if (i >= sessions.length) {
                if (logger.isActivated()) {
                    logger.info("receivePagerMessage Extendchat is null");
                }
                // Add session in the list
                ExtendChatImpl extendChat = new ExtendChatImpl(number, this);
                ChatServiceImpl.addExtendChatSession(number, extendChat);

                rmHistory.addExtendChatMessage(message, message.getDirection());

                Intent intent = new Intent(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE);
                intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
                try{
                    intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT, message.getRemote());
                } catch(Exception e){
                    e.printStackTrace();
                }
                intent.putExtra(OneToOneChatIntent.EXTRA_DISPLAY_NAME, message.getDisplayName());
                intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, message.getMessageId());

                mExtendChatEventBroadcaster.broadcastExtendMessageReceived(message.getMimeType(), message.getMessageId(), message.getDisplayName());

                //AndroidFactory.getApplicationContext().sendBroadcast(intent);
            }
      }
    }

    /**
     * Receive a new chat invitation
     *
     * @param session Chat session
     */
    public void receiveStandaloneChatInvitation(StandaloneChatSession session) {
        if (logger.isActivated()) {
            logger.info("Receive stand alone chat invitation from " + session.getRemoteContact() +
                    "  Display name: " + session.getRemoteDisplayName());
        }

        if (logger.isActivated()) {
            logger.info("Receive stand alone chat invitation Conversation: "
                    + session.getConversationID());
        }

        String contact = session.getRemoteContact();
        String number;
        if (session.getDirection() == Direction.OUTGOING.toInt()) {
            number = PhoneUtils.extractNumberFromUri(contact);
        } else
        if (session.getMessageType() == ChatLog.Message.Type.PUBLIC ||
            session.toSecondary())
            number =  PhoneUtils.extractUuidFromUri(contact);
        else
            number =  PhoneUtils.extractNumberFromUri(contact);

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();

        if (RcsSettings.getInstance().isCPMSupported() &&
            session.getDirection() != Direction.OUTGOING.toInt()) {
            String newConvId = session.getConversationID();
            String oldConvId = rmHistory.getCoversationID(number, 1);

            if (oldConvId.isEmpty()) {
                // This is the 1st message for the conversation, save conversation ID here
                InstantMessage conversationMsg = ChatUtils.createTextMessage(contact, "system", false);
                rmHistory.addChatSystemMessage(conversationMsg, Direction.INCOMING.toInt());
                rmHistory.UpdateCoversationID(number, newConvId, 1);
            } else
            if (!oldConvId.equals(newConvId)) {
                if (newConvId != null) {
                    if (logger.isActivated()) {
                       logger.info("Receive stand alone chat invitation OldId: " + oldConvId + "NewId: " + newConvId);
                    }
                    rmHistory.UpdateCoversationID(number, newConvId, 1);
                } else {
                    if (logger.isActivated()) {
                        logger.info("Receive stand alone chat invitation Conversation Id is null");
                     }
                }
            }
        }

        synchronized(lock) {
            Object[] sessions = extendChatSessions.values().toArray();
            int i;
            for (i = 0; i < sessions.length; i++) {
                ExtendChatImpl extendChat = (ExtendChatImpl)sessions[i];
                List<String> contacts = extendChat.getRemoteContacts();
                if (contacts.contains(number)) {
                    if (logger.isActivated()) {
                        logger.info("LMM extendChat is not null");
                    }
                    if (!session.getListeners().contains(extendChat)) {
                        if (logger.isActivated()) {
                            logger.info("Add extendChat to session as callback~");
                        }
                        session.addListener(extendChat);
                    }
                    break;
                }
            }
            if (i >= sessions.length) {
                // Add session in the list
                ExtendChatImpl extendChat = new ExtendChatImpl(number, this);
                ChatServiceImpl.addExtendChatSession(number, extendChat);
                session.addListener(extendChat);

                // Notify chat invitation listeners
                synchronized(lock) {
                    final int N = listeners.beginBroadcast();
                    if (logger.isActivated()) {
                        logger.info("receiveStandaloneChatMessage N: " + N);
                    }
                    for (int j=0; j < N; j++) {
                        try {
                            //listeners.getBroadcastItem(i).onNewSingleChat(number, null);
                            if (logger.isActivated()) {
                                logger.info("write calback");
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
    }

    public void receiveStandaloneChatMessage(InstantMessage msgApi) {

        // Broadcast intent related to the received invitation
        Intent intent = new Intent(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_STANDALONE_CHAT_MESSAGE);
        intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        try{
            intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT, msgApi.getRemote());
        } catch(Exception e){
            e.printStackTrace();
        }
        intent.putExtra(OneToOneChatIntent.EXTRA_DISPLAY_NAME, msgApi.getDisplayName());
        try {
        intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, msgApi.getMessageId());
        }catch(Exception e){
            if (logger.isActivated()) {
                logger.info("Receive chat invitation from ");
            }
            e.printStackTrace();
        }

        AndroidFactory.getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");

    }

    /**
     * Receive a new chat invitation
     *
     * @param session Chat session
     */
    public void receiveOneOneChatInvitation(OneOneChatSession session) {
        if (logger.isActivated()) {
            logger.info("Receive chat invitation from " + session.getRemoteContact() + "  Display name: " + session.getRemoteDisplayName());
        }

        // Extract number from contact
        String number = PhoneUtils.extractNumberFromUri(session.getRemoteContact());

        // Update rich messaging history
        // Nothing done in database
        // Update rich messaging history with o2o chat message

        InstantMessage msg = session.getFirstMessage();
        if(msg != null){
            msg.setDisplayName(session.getRemoteDisplayName());
            if (logger.isActivated()) {
                logger.info("Receive chat invitation from first message display: " + msg.getDisplayName());
            }
            if (!RichMessagingHistory.getInstance().isOne2OneMessageExists(msg.getMessageId())) {
                if (logger.isActivated()) {
                    logger.info("Receive chat invitation from first message Id: " + msg.getMessageId());
                }
                RichMessagingHistory.getInstance().addChatSystemMessage(msg, Direction.INCOMING.toInt());
            }
        }
         if(RcsSettings.getInstance().isCPMSupported()) {
                if(!(RichMessagingHistory.getInstance().getCoversationID(session.getRemoteContact(), 1).equals(session.getConversationID()))){
                    if(session.getConversationID() != null) {
                        if (logger.isActivated()) {
                           logger.info("Receive O2O chat invitation OldId: "  + RichMessagingHistory.getInstance().getCoversationID(session.getRemoteContact(),1) + " NewId: " + session.getConversationID());
                        }
                        RichMessagingHistory.getInstance().UpdateCoversationID(session.getRemoteContact(),session.getConversationID(), 1);
                    } else {
                        if (logger.isActivated()) {
                            logger.info("Receive O2O chat invitation Conversation Id is null");
                         }
                    }
                } else {
                    // Create a text message
                    InstantMessage conversationMsg = ChatUtils.createTextMessage(session.getRemoteContact(), "system",
                            Core.getInstance().getImService().getImdnManager().isImdnActivated());
                     if(conversationMsg != null){
                         conversationMsg.setDisplayName(session.getRemoteDisplayName());
                         if (logger.isActivated()) {
                            logger.info("Receive1 chat invitation from first message display: " + conversationMsg.getDisplayName());
                         }
                     }
                     RichMessagingHistory.getInstance().addChatSystemMessage(conversationMsg, Direction.INCOMING.toInt());
                     RichMessagingHistory.getInstance().UpdateCoversationID(session.getRemoteContact(),session.getConversationID(),1);
                }
            }

        // Add session in the list
        OneToOneChatImpl sessionApi = new OneToOneChatImpl(number, this,session);
        sessionApi.setCoreSession(session);
        ChatServiceImpl.addChatSession(number, sessionApi);

        //ChatMessage msgApi = null;
        if(msg != null){
            /*if (msg instanceof GeolocMessage) {
                GeolocMessage geoloc = (GeolocMessage)msg;
                Geoloc geolocApi = new Geoloc(geoloc.getGeoloc().getLabel(),
                        geoloc.getGeoloc().getLatitude(), geoloc.getGeoloc().getLongitude(),
                        geoloc.getGeoloc().getExpiration());
                msgApi = new org.gsma.joyn.chat.GeolocMessage(msg.getMessageId(),
                                                            PhoneUtils.extractNumberFromUri(msg.getRemote()),
                                                            msg.getTextMessage(),
                                                            InstantMessage.MIME_TYPE,
                                                            msg.getDisplayName(),
                                                            Direction.INCOMING.toInt(),
                                                            msg.getServerDate().getTime(),0,
                                                            0,0,
                                                            ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt(),
                                                            0,sessionApi.getChatId(),
                                                            false,false,geolocApi);
            } else {
                msgApi = new ChatMessage(msg.getMessageId(),
                                PhoneUtils.extractNumberFromUri(msg.getRemote()),
                                msg.getTextMessage(),
                                InstantMessage.MIME_TYPE,
                                msg.getDisplayName(),
                                Direction.INCOMING.toInt(),
                                msg.getServerDate().getTime(),0,
                                0,0,
                                ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt(),
                                0,sessionApi.getChatId(),
                                false,false);
            }*/
            synchronized(lock) {
                mOneToOneChatEventBroadcaster.broadcastMessageReceived(msg.getMimeType(),msg.getMessageId());
            }
        } /*else {
            // Broadcast intent related to the received invitation
            Intent intent = new Intent(OneToOneChatIntent.ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE);
            intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
            intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT, number);
            intent.putExtra(OneToOneChatIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
            intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, msg.getMessageId());
            AndroidFactory.getApplicationContext().sendBroadcast(intent);
        }      */
    }

    /**
     * Open a single chat with a given contact and returns a Chat instance.
     * The parameter contact supports the following formats: MSISDN in national
     * or international format, SIP address, SIP-URI or Tel-URI.
     *
     * @param contact Contact
     * @param listener Chat event listener
     * @return Chat
     * @throws ServerApiException
     */
    public IOneToOneChat openSingleChat(String contact, IOneToOneChatListener listener) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Open a 1-1 chat session with " + contact);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Extract number from contact
            String number = PhoneUtils.extractNumberFromUri(contact);

            // Check if there is an existing chat or not
            OneToOneChatImpl sessionApi = (OneToOneChatImpl)ChatServiceImpl.getChatSession(number);
            //if(sessionApi != null){
            //    OneOneChatSession tempSession = sessionApi.getCoreSession();
            //}

            if (sessionApi != null && sessionApi.getCoreSession() != null && !sessionApi.getCoreSession().isStoreAndForward()) {
            //if (sessionApi != null ){
                if (logger.isActivated()) {
                    logger.debug("Chat session already exist for " + number);
                }

                // Add session listener
                sessionApi.addEventListener(listener);

                // Check core session state
                final OneOneChatSession coreSession = sessionApi.getCoreSession();
                if (coreSession != null) {
                    if (logger.isActivated()) {
                        logger.debug("Core chat session already exist: " + coreSession.getSessionID());
                    }

                    if (coreSession.getDialogPath().isSessionTerminated() ||
                            coreSession.getDialogPath().isSessionCancelled()) {
                        if (logger.isActivated()) {
                            logger.debug("Core chat session is terminated: reset it");
                        }

                        // Session has expired, remove it
                        sessionApi.resetCoreSession();
                    } else
                    if (!coreSession.getDialogPath().isSessionEstablished()) {
                        if (logger.isActivated()) {
                            logger.debug("Core chat session is pending: auto accept it");
                        }

                        // Auto accept the pending session
                        Thread t = new Thread() {
                            public void run() {
                                coreSession.acceptSession();
                            }
                        };
                        t.start();
                    } else {
                        if (logger.isActivated()) {
                            logger.debug("Core chat session is already established");
                        }
                    }
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("Create a new chat session with " + number);
                }

                if(sessionApi == null || (sessionApi != null && sessionApi.getCoreSession() != null)) {
                    try {
                        int size = chatSessions.size();
                        if (logger.isActivated()) {
                            logger.debug("chatSessions size:" +size + ", session Api is:" + sessionApi);
                        }
                        if(size >= RcsSettings.getInstance().getMaxChatSessions()) {
                            abortLastActiveSession();
                        }
                     }catch(Exception e) {
                        if (logger.isActivated()) {
                           logger.debug("exception in creating new chat ");
                        }
                        e.printStackTrace();
                    }
                 }

                // Add session listener
                sessionApi = new OneToOneChatImpl(number,this,null);
                sessionApi.addEventListener(listener);

                // Add session in the list
                ChatServiceImpl.addChatSession(number, sessionApi);
            }

            return sessionApi;

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    public IOneToOneChat getOrCreateOneToOneChat(String contact) {
        if (logger.isActivated()) {
            logger.info("getOrCreateOneToOneChat " + contact);
        }
        OneToOneChatImpl sessionApi = (OneToOneChatImpl)ChatServiceImpl.getChatSession(contact);
        if(sessionApi == null){
            if (logger.isActivated()) {
                logger.info("getOrCreateOneToOneChat session API is null");
            }
            sessionApi = new OneToOneChatImpl(contact,this,null);
            // Add session in the list
            ChatServiceImpl.addChatSession(contact, sessionApi);
        }
        return sessionApi;
    }

    public IExtendChat getOrCreateExtendChat(String contact) {
        if (logger.isActivated()) {
            logger.info("getOrCreateExtendChat " + contact);
        }
        ExtendChatImpl sessionApi = (ExtendChatImpl)ChatServiceImpl.getExtendChatSession(contact);
        if(sessionApi == null){
            if (logger.isActivated()) {
                logger.info("getOrCreateExtendChat session API is null");
            }
            // Add session in the list
            sessionApi = new ExtendChatImpl(contact,this);
            ChatServiceImpl.addExtendChatSession(contact, sessionApi);
        }
        return sessionApi;
    }

    public IExtendChat createExtendMultiChat(List<String> contacts) {
        if (logger.isActivated()) {
            logger.info("Create multi chat session with " + contacts);
        }
        ExtendChatImpl sessionApi = new ExtendChatImpl(contacts,this);
        // Add session in the list
        ChatServiceImpl.addExtendChatSession(sessionApi.getChatId(), sessionApi);
        return sessionApi;
    }

    public void abortLastActiveSession() {
        if (logger.isActivated()) {
           logger.info("abortLastActiveSession");
        }
        Set<String> keys = chatSessions.keySet();
        TreeSet<Long> setTimeStamp = new TreeSet<Long>();
        Hashtable<Long, String> keyTable = new Hashtable<Long, String>();
        RichMessagingHistory instance = RichMessagingHistory.getInstance();
        for(String key: keys){
            long timeStamp = instance.getLatestTimeStamp(key);
             if (logger.isActivated()) {
                 logger.info("abortLastActiveSession timestamp:" + timeStamp);
             }
            setTimeStamp.add(timeStamp);
            keyTable.put(timeStamp, key);
        }
        final String lastUsedContact = keyTable.get(setTimeStamp.first());
        if (logger.isActivated()) {
            logger.info("abortLastActiveSession Contact :" + lastUsedContact);
        }
        final IOneToOneChat mChat = chatSessions.get(lastUsedContact);
        if(mChat != null) {
             // Send text message
            if(mChat instanceof OneToOneChatImpl){
                if (logger.isActivated()) {
                    logger.info("abortLastActiveSession Contact :" + lastUsedContact);
                }
                OneToOneChatImpl mChatImpl = ((OneToOneChatImpl)mChat);
                mChatImpl.getCoreSession().abortSession(ImsServiceSession.TERMINATION_BY_USER);
            }
            try {
                if (logger.isActivated()) {
                    logger.info("abortLastActiveSession  entry");
                }
                Thread.sleep(1000);
            } catch(Exception e){
                if (logger.isActivated()) {
                    logger.info("abortLastActiveSession sleep exception");
                }
            }
            if (logger.isActivated()) {
                logger.info("abortLastActiveSession  exit");
            }

        }
   }

    /**
     * Open a single chat with a given contact and returns a Chat instance.
     * The parameter contact supports the following formats: MSISDN in national
     * or international format, SIP address, SIP-URI or Tel-URI.
     *
     * @param contact Contact
     * @param listener Chat event listener
     * @return Chat
     * @throws ServerApiException
     */
    public IExtendChat openSingleChatEx(String contact, IExtendChatListener listener) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Open a 1-1 chat session with " + contact);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            /* To compatible with public account */
            contact = PhoneUtils.extractUuidFromUri(contact);

            // Check if there is an existing chat or not
            ExtendChatImpl sessionApi = (ExtendChatImpl)getExtendChatSession(contact);

            if (sessionApi != null) {
                if (logger.isActivated()) {
                    logger.debug("Chat session already exist for " + contact);
                }

                // Add session listener
                sessionApi.addEventListener(listener);
            } else {
                if (logger.isActivated()) {
                    logger.debug("Create a new chat session with " + contact);
                }

                // Add session listener
                sessionApi = new ExtendChatImpl(contact, this);
                sessionApi.addEventListener(listener);

                // Add session in the list
                addExtendChatSession(contact, sessionApi);
            }

            return sessionApi;

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Open a single chat with a given contact and returns a Chat instance.
     * The parameter contact supports the following formats: MSISDN in national
     * or international format, SIP address, SIP-URI or Tel-URI.
     *
     * @param contact Contact
     * @param listener Chat event listener
     * @return Chat
     * @throws ServerApiException
     */
    public IExtendChat openMultipleChat(List<String> contacts, IExtendChatListener listener) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Open a multi chat session with " + contacts);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            String number = PhoneUtils.generateContactsText(contacts);

            // Check if there is an existing chat or not
            ExtendChatImpl sessionApi = (ExtendChatImpl)getExtendChatSession(number);

            if (sessionApi != null) {
                if (logger.isActivated()) {
                    logger.debug("Chat session already exist for " + number);
                }

                // Add session listener
                sessionApi.addEventListener(listener);
            } else {
                if (logger.isActivated()) {
                    logger.debug("Create a new chat session with " + number);
                }

                // Add session listener
                sessionApi = new ExtendChatImpl(contacts, this);
                sessionApi.addEventListener(listener);

                // Add session in the list
                addExtendChatSession(number, sessionApi);
            }

            return sessionApi;

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Open a single chat with a given contact and returns a Chat instance.
     * The parameter contact supports the following formats: MSISDN in national
     * or international format, SIP address, SIP-URI or Tel-URI.
     *
     * @param contact Contact
     * @param listener Chat event listener
     * @return Chat
     * @throws ServerApiException
     */
    public IExtendChat openSecondaryDeviceChat(IExtendChatListener listener) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Open a secondary device 1-1 chat session");
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            String contact = RcsSettings.getInstance().getSecondaryDeviceUserIdentity();

            /* To compatible with public account */
            contact = PhoneUtils.extractUuidFromUri(contact);

            // Check if there is an existing chat or not
            ExtendChatImpl sessionApi = (ExtendChatImpl)getExtendChatSession(contact);

            if (sessionApi != null) {
                if (logger.isActivated()) {
                    logger.debug("Chat session already exist for " + contact);
                }

                // Add session listener
                sessionApi.addEventListener(listener);
            } else {
                if (logger.isActivated()) {
                    logger.debug("Create a new chat session with " + contact);
                }

                // Add session listener
                sessionApi = new ExtendChatImpl(contact, this);
                sessionApi.addEventListener(listener);

                // Add session in the list
                addExtendChatSession(contact, sessionApi);
            }

            return sessionApi;

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Receive message delivery status
     *
     * @param contact Contact
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void receiveMessageDeliveryStatus(String contact, String msgId, String status, Date date) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("LMM Receive message delivery status for message " + msgId + ", status " + status);
            }

            String number = PhoneUtils.extractNumberFromUri(contact);

            if (logger.isActivated()) {
                logger.info("LMM receiveMessageDeliveryStatus " + contact + ", number " + number);
            }

            // Notify message delivery listeners
            OneToOneChatImpl chat = (OneToOneChatImpl)ChatServiceImpl.getChatSession(number);
            if (chat != null) {
                if (logger.isActivated()) {
                    logger.info("LMM chat is not null");
                }
                chat.handleMessageDeliveryStatus(msgId, status, null, null);
            } else {
                Object[] sessions = extendChatSessions.values().toArray();
                int i;
                for (i = 0; i < sessions.length; i++) {
                    List<String> contacts = ((ExtendChatImpl)sessions[i]).getRemoteContacts();
                    if (contacts.contains(number)) {
                        if (logger.isActivated()) {
                            logger.info("LMM extendChat is not null");
                        }
                        ((ExtendChatImpl)sessions[i]).handleMessageDeliveryStatus(msgId, status, number,date);
                        break;
                    }
                }
                if (i >= sessions.length) {
                    if (logger.isActivated()) {
                        logger.info("LMM chat is null");
                    }
                    // Update rich messaging history
                    RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(msgId, status);

                    Intent intent = new Intent(OneToOneChatIntent.ACTION_DELIVERY_STATUS);
                    intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
                    intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT, number);
                    intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, msgId);
                    intent.putExtra(OneToOneChatIntent.EXTRA_STATUS, status);
                    intent.putExtra("date", date.getTime());
                    AndroidFactory.getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
                }
            }
        }
    }

    /**
     * Receive message delivery status
     *
     * @param contact Contact ID
     * @param imdn Imdn document
     */
    public void onOneToOneMessageDeliveryStatusReceived(ContactId contact, Status status, String msgId, Date date, ReasonCode reasonCode) {
        if(date == null){
            date = new Date();
        }
        Long timestamp = date.getTime();
        if (logger.isActivated()) {
            logger.info("onOneToOneMessageDeliveryStatusReceived contact: " + contact + " status: " + status + " msgid: " + msgId + " reason: " + reasonCode);
        }
        String mimeType = messagingLog.getMessageMimeType(msgId);
        if (status == Status.FAILED) {
            synchronized (lock) {
                if (messagingLog.setChatMessageStatusAndReasonCode(msgId, Status.FAILED.toInt(),
                        reasonCode.toInt())) {
                    mOneToOneChatEventBroadcaster.broadcastMessageStatusChanged(contact, mimeType,
                            msgId, Status.FAILED, reasonCode);
                }
            }
        } else if (status == Status.DELIVERED) {
            imsService.getDeliveryExpirationManager().cancelDeliveryTimeoutAlarm(msgId);
            synchronized (lock) {
                if (messagingLog.setMessageStatusDelivered(msgId, timestamp)) {
                    mOneToOneChatEventBroadcaster.broadcastMessageStatusChanged(contact, mimeType,
                            msgId, Status.DELIVERED, ReasonCode.UNSPECIFIED);
                }
            }
        } else if (status == Status.DISPLAYED) {
            imsService.getDeliveryExpirationManager().cancelDeliveryTimeoutAlarm(msgId);
            synchronized (lock) {
                if (messagingLog.setMessageStatusDisplayed(msgId, timestamp)) {
                    mOneToOneChatEventBroadcaster.broadcastMessageStatusChanged(contact, mimeType,
                            msgId, Status.DISPLAYED, ReasonCode.UNSPECIFIED);
                }
            }
        } else if (status == Status.SENT) {
            imsService.getDeliveryExpirationManager().cancelDeliveryTimeoutAlarm(msgId);
            synchronized (lock) {
                if (messagingLog.setMessageStatusSent(msgId, timestamp)) {
                    mOneToOneChatEventBroadcaster.broadcastMessageStatusChanged(contact, mimeType,
                            msgId, Status.SENT, ReasonCode.UNSPECIFIED);
                }
            }
        }
    }

    /**
     * Receive message delivery status
     *
     * @param contact Contact ID
     * @param imdn Imdn document
     */
    public void onExtendMessageDeliveryStatusReceived(String mChatid,ContactId contact, Status status, String msgId, Date date, ReasonCode reasonCode) {
        Long timestamp = date.getTime();
        if (logger.isActivated()) {
            logger.info("onExtendMessageDeliveryStatusReceived contact: " + contact + " status: " + status + " msgid: " + msgId + " reason: " + reasonCode);
        }
        String mimeType = messagingLog.getMessageMimeType(msgId);
        if (status == Status.FAILED) {
            synchronized (lock) {
                if (messagingLog.setChatMessageStatusAndReasonCode(msgId, Status.FAILED.toInt(),
                        reasonCode.toInt())) {
                    mExtendChatEventBroadcaster.broadcastMessageStatusChanged(mChatid,contact, mimeType,
                            msgId, Status.FAILED, reasonCode);
                }
            }
        } else if (status == Status.DELIVERED) {
            imsService.getDeliveryExpirationManager().cancelDeliveryTimeoutAlarm(msgId);
            synchronized (lock) {
                if (messagingLog.setMessageStatusDelivered(msgId, timestamp)) {
                    mExtendChatEventBroadcaster.broadcastMessageStatusChanged(mChatid,contact, mimeType,
                            msgId, Status.DELIVERED, ReasonCode.UNSPECIFIED);
                }
            }
        } else if (status == Status.DISPLAYED) {
            imsService.getDeliveryExpirationManager().cancelDeliveryTimeoutAlarm(msgId);
            synchronized (lock) {
                if (messagingLog.setMessageStatusDisplayed(msgId, timestamp)) {
                    mExtendChatEventBroadcaster.broadcastMessageStatusChanged(mChatid,contact, mimeType,
                            msgId, Status.DISPLAYED, ReasonCode.UNSPECIFIED);
                }
            }
        } else if (status == Status.SENT) {
            imsService.getDeliveryExpirationManager().cancelDeliveryTimeoutAlarm(msgId);
            synchronized (lock) {
                if (messagingLog.setMessageStatusSent(msgId, timestamp)) {
                    mExtendChatEventBroadcaster.broadcastMessageStatusChanged(mChatid,contact, mimeType,
                            msgId, Status.SENT, ReasonCode.UNSPECIFIED);
                }
            }
        }
    }

    /**
     * Add a chat session in the list
     *
     * @param contact Contact
     * @param session Chat session
     */
    public static void addChatSession(String contact, OneToOneChatImpl session) {
        if (logger.isActivated()) {
            logger.debug("LMM a chat session in the list (size=" + chatSessions.size() + ") for " + contact);
        }

        if((session.getCoreSession() != null) && session.getCoreSession().isStoreAndForward()){
            if (logger.isActivated()) {
                logger.debug("LMM ChatSession s&f");
            }
            if (logger.isActivated()) {
                logger.debug("AddRemove add storeForwardChatSessions " + session.getCoreSession().getSessionID());
            }
            storeForwardChatSessions.put(contact, session);
        }
        else{
            if (logger.isActivated()) {
                //logger.debug("LMM AddRemove add chatSessions " + session.getCoreSession().getSessionID());
            }
            chatSessions.put(contact, session);
        }
    }

    /**
     * Add a chat session in the list
     *
     * @param contact Contact
     * @param session Chat session
     */
    public static void addExtendChatSession(String contact, ExtendChatImpl session) {
        if (logger.isActivated()) {
            logger.debug("LMM a chat session in the list (size=" + extendChatSessions.size() + ") for " + contact);
        }

        extendChatSessions.put(contact, session);
    }

    /**
     * Get a chat session from the list for a given contact
     *
     * @param contact Contact
     * @param GroupChat session
     */
    protected static IOneToOneChat getStoreChatSession(String contact) {
        if (logger.isActivated()) {
            logger.debug("LMM Get ChatSession s&f " + contact);
        }

        return storeForwardChatSessions.get(contact);
    }

    /**
     * Remove a chat session from the list
     *
     * @param contact Contact
     */
    protected static void removeStoreChatSession(String contact) {
        if (logger.isActivated()) {
            logger.debug("LMM Remove removeStoreChatSession (size=" + storeForwardChatSessions.size() + ") for " + contact);
        }

        if ((storeForwardChatSessions != null) && (contact != null)) {
            storeForwardChatSessions.remove(contact);
        }
    }

    /**
     * Get a chat session from the list for a given contact
     *
     * @param contact Contact
     * @param GroupChat session
     */
    protected static IOneToOneChat getChatSession(String contact) {
        if (logger.isActivated()) {
            logger.debug("LMM Get a chat session for " + contact);
        }

        return chatSessions.get(contact);
    }

    /**
     * Get a chat session from the list for a given contact
     *
     * @param contact Contact
     * @param GroupChat session
     */
    protected static IExtendChat getExtendChatSession(String contact) {
        if (logger.isActivated()) {
            logger.debug("LMM Get a chat session for " + contact);
        }

        return extendChatSessions.get(contact);
    }

    /**
     * Remove a chat session from the list
     *
     * @param contact Contact
     */
    protected static void removeChatSession(String contact) {
        if (logger.isActivated()) {
            logger.debug("LMM Remove a chat session from the list (size=" + chatSessions.size() + ") for " + contact);
        }

        try {
            if ((chatSessions != null) && (contact != null)) {
                chatSessions.remove(contact);
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * Remove a chat session from the list
     *
     * @param contact Contact
     */
    protected static void removeExtendChatSession(String contact) {
        if (logger.isActivated()) {
            logger.debug("LMM Remove a chat session from the list (size=" + extendChatSessions.size() + ") for " + contact);
        }

        try {
            if ((extendChatSessions != null) && (contact != null)) {
                extendChatSessions.remove(contact);
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * Returns the list of single chats in progress
     *
     * @return List of chats
     * @throws ServerApiException
     */
    public List<IBinder> getChats() throws ServerApiException {
        int size = chatSessions.size() + storeForwardChatSessions.size();
        if (logger.isActivated()) {
            logger.info("Get chat sessions sze: " + size);
        }

        try {
            ArrayList<IBinder> result = new ArrayList<IBinder>(size);
            for (Enumeration<IOneToOneChat> e = chatSessions.elements() ; e.hasMoreElements() ;) {
                IOneToOneChat sessionApi = (IOneToOneChat)e.nextElement() ;
                result.add(sessionApi.asBinder());
            }
            //ArrayList<IBinder> result = new ArrayList<IBinder>(storeForwardChatSessions.size());
            for (Enumeration<IOneToOneChat> e = storeForwardChatSessions.elements() ; e.hasMoreElements() ;) {
                IOneToOneChat sessionApi = (IOneToOneChat)e.nextElement() ;
                result.add(sessionApi.asBinder());
            }
            return result;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Returns the list of single chats in progress
     *
     * @return List of chats
     * @throws ServerApiException
     */
    public List<IBinder> getExtendChats() throws ServerApiException {
        int size = extendChatSessions.size();
        if (logger.isActivated()) {
            logger.info("Get chat sessions sze: " + size);
        }

        try {
            ArrayList<IBinder> result = new ArrayList<IBinder>(size);
            for (Enumeration<IExtendChat> e = extendChatSessions.elements() ; e.hasMoreElements() ;) {
                IExtendChat sessionApi = (IExtendChat)e.nextElement() ;
                result.add(sessionApi.asBinder());
            }
            return result;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Returns a chat in progress from its unique ID
     *
     * @param contact Contact
     * @return Chat or null if not found
     * @throws ServerApiException
     */
    public IOneToOneChat getChat(String contact) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get chat session with " + contact);
        }

        // Return a session instance
        return chatSessions.get(contact);
    }

    /**
     * Returns a chat in progress from its unique ID
     *
     * @param contact Contact
     * @return Chat or null if not found
     * @throws ServerApiException
     */
    public IOneToOneChat getOneToOneChat(ContactId contact) throws ServerApiException {
        String contactString = contact.toString();
        if (logger.isActivated()) {
            logger.info("Get chat session with " + contactString);
        }
        return getOrCreateOneToOneChat(contactString);
    }

    public static Hashtable<String, IOneToOneChat> getO2OSessions(){
        return chatSessions;
    }

    /**
     * Returns a chat in progress from its unique ID
     *
     * @param contact Contact
     * @return Chat or null if not found
     * @throws ServerApiException
     */
    public IExtendChat getExtendChat(ContactId contact) throws ServerApiException {
        String contactString = contact.toString();
        if (logger.isActivated()) {
            logger.info("Get extend chat session with " + contactString);
        }

        // Return a session instance
        //return extendChatSessions.get(contact);
        return getOrCreateExtendChat(contactString);
    }

    /**
     * Returns a chat in progress from its unique ID
     *
     * @param contact Contact
     * @return Chat or null if not found
     * @throws ServerApiException
     */
    public IExtendChat getExtendMultiChat(List<String> contacts) throws ServerApiException {
        List<String> contactsList = contacts;//PhoneUtils.generateContactsStringList(contacts);
        if (logger.isActivated()) {
            logger.info("getExtendMultiChat " + contactsList + ", size:" + contacts.size());
        }
        String contact = PhoneUtils.generateContactsText(contactsList);
        IExtendChat extendMultiChat = extendChatSessions.get(contact);
        if(extendMultiChat != null) return extendMultiChat;

        return createExtendMultiChat(contactsList);
    }

    /**
     * Returns a chat in progress from its unique ID
     *
     * @param contact Contact
     * @return Chat or null if not found
     * @throws ServerApiException
     */
    public IExtendChat getChatForSecondaryDevice() throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get chat session for secondary device ");
        }

        String contact = RcsSettings.getInstance().getSecondaryDeviceUserIdentity();
        contact = PhoneUtils.extractUuidFromUri(contact);
        // Return a session instance
        return extendChatSessions.get(contact);
    }

    /**
     * Receive a new group chat invitation
     *
     * @param session Chat session
     */
    public void receiveGroupChatInvitation(GroupChatSession session) {
        if (logger.isActivated()) {
            logger.info("Receive group chat invitation from " +
                    session.getRemoteContact() +
                    " Display name: " + session.getGroupRemoteDisplayName() +
                    " new name: " + SipUtils.getDisplayNameFromUri(session.getRemoteContact()));
        }

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();

        // Extract number from contact
        String number = PhoneUtils.extractNumberFromUri(session.getRemoteContact());

        Boolean isChatExist = false;
        GroupChatInfo info = null;

        isChatExist = rmHistory.isGroupChatExists(session.getContributionID());

        // Contribution ID might be different for re-Invite case, try conference ID once more
        if (isChatExist) {
            info = rmHistory.getGroupChatInfo(session.getContributionID());
        } else {
            info = rmHistory.getGroupChatInfoByRejoinId(session.getImSessionIdentity());
            if (info != null) {
                session.setContributionID(info.getContributionId());
                isChatExist = true;
            }
        }
        boolean offline = false;
        if (RcsSettings.getInstance().supportOP01()) {
            // This is group chat offline message, just auto accept it despite the setting
            if (PhoneUtils.extractUuidFromUri(session.getRemoteContact()).equals(
                PhoneUtils.extractUuidFromUri(session.getImSessionIdentity())))
                offline = true;
        }

        // Update rich messaging history if not present , to stop same chatId to come , TODO add update function and call in else
        if (!isChatExist) {
         // Update rich messaging history
            messagingLog.addGroupChat(
                    session.getContributionID(),
                    session.getRemoteContact(),
                    session.getSubject(),
                    session.getParticipantsWithStatus(),
                    GroupChat.State.INVITED.toInt(),
                    ReasonCode.UNSPECIFIED.toInt(),
                    Direction.INCOMING.toInt(),
                    Calendar.getInstance().getTimeInMillis());

            if (RcsSettings.getInstance().supportOP01()) {
                rmHistory.updateGroupChatRejoinId(
                        session.getContributionID(),
                        session.getImSessionIdentity());
            }
            info = rmHistory.getGroupChatInfo(session.getContributionID());
        }

        int blocked = rmHistory.getGroupBlockedStatus(info.getSessionId());
        if(blocked == 1){
           session.setMessageBlocked(true);
        } else if(blocked == 0){
           session.setMessageBlocked(false);
        }
        //session.setChairman(session.getRemoteContact());//chairman already set in session
        //RichMessagingHistory.getInstance().updateGroupChairman(session.getContributionID(), session.getChairman());
        if (RcsSettings.getInstance().isCPMSupported()) {
            if (logger.isActivated()) {
                logger.info("receiveGroupChatInvitation" +
                        " conversationId: " + session.getConversationID() +
                        " contributionId: " + session.getContributionID());
            }
            rmHistory.UpdateCoversationID(info.getSessionId(), session.getConversationID(), 2);
        }

        // Add session in the list
        GroupChatImpl sessionApi = new GroupChatImpl(session, this);
        ChatServiceImpl.addGroupChatSession(sessionApi);

        if (logger.isActivated()) {
            logger.info("receiveGroupChatInvitation ischatexists: " + isChatExist);
        }

        // Broadcast intent related to the received invitation
        //Intent intent = new Intent(GroupChatIntent.ACTION_NEW_INVITATION);
        /*intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        intent.putExtra(GroupChatIntent.EXTRA_CONTACT, number);
        intent.putExtra(GroupChatIntent.EXTRA_DISPLAY_NAME, session.getGroupRemoteDisplayName());
        intent.putExtra(GroupChatIntent.EXTRA_CHAT_ID, sessionApi.getChatId());
        intent.putExtra(GroupChatIntent.EXTRA_SUBJECT, sessionApi.getSubject());
        intent.putExtra("autoAccept", autoAccept);
        intent.putExtra("isGroupChatExist", isChatExist);
        intent.putExtra("isClosedGroupChat", session.isClosedGroup());
        if (RcsSettings.getInstance().supportOP01()) {
            intent.putExtra("offlineInvite", offline);
        }
        intent.putExtra(GroupChatIntent.EXTRA_SESSION_IDENTITY, session.getImSessionIdentity());
         *//**
         * M: managing extra local chat participants that are
         * not present in the invitation for sending them invite request.@{
         *//*
        String participants = "";
        List<String> ListParticipant = session.getParticipants().getList();
        for(String currentPartc : ListParticipant){
         participants += currentPartc + ";";
        }
        *//**
         * @}
         *//*
        intent.putExtra("participantList", participants);

        AndroidFactory.getApplicationContext().sendBroadcast(intent);*/
        synchronized (lock) {
            mGroupChatEventBroadcaster.broadcastInvitation(sessionApi.getChatId(), session.isClosedGroup(), offline, isChatExist, session);
        }

        // Notify chat invitation listeners
        /*synchronized(lock) {
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("receiveGroupChatInvitation N: " + N);
            }
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onNewGroupChat(sessionApi.getChatId());
                    if (logger.isActivated()) {
                        logger.info("write calback");
                    }
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();
        }        */
    }

    /**
     * Extend a 1-1 chat session
     *
     * @param groupSession Group chat session
     * @param oneoneSession 1-1 chat session
     */
    public void extendOneOneChatSession(GroupChatSession groupSession, OneOneChatSession oneoneSession) {
        if (logger.isActivated()) {
            logger.info("extendOneOneChatSession ReplaceId: " + groupSession.getSessionID());
        }

        // Add session in the list
        GroupChatImpl sessionApi = new GroupChatImpl(groupSession,this);
        ChatServiceImpl.addGroupChatSession(sessionApi);

        if (logger.isActivated()) {
            logger.info("extendOneOneChatSession ExtraChatId: " + sessionApi.getChatId());
        }

        // Broadcast intent related to the received invitation
        Intent intent = new Intent(GroupChatIntent.ACTION_SESSION_REPLACED);
        intent.putExtra("sessionId", groupSession.getSessionID());
        intent.putExtra(GroupChatIntent.EXTRA_CHAT_ID, sessionApi.getChatId());
        AndroidFactory.getApplicationContext().sendBroadcast(intent,
                                                             "com.gsma.services.permission.RCS");
    }

    /**
     * Add a group chat session in the list
     *
     * @param session Chat session
     */
    protected static void addGroupChatSession(GroupChatImpl session) {
        if (logger.isActivated()) {
            logger.debug("Add a group chat session in the list (size=" + groupChatSessions.size() + ")");
        }
        groupChatSessions.put(session.getChatId(), session);
    }

    /**
     * Remove a group chat session from the list
     *
     * @param chatId Chat ID
     */
    protected static void removeGroupChatSession(String chatId) {
        if (logger.isActivated()) {
            logger.debug("Remove a group chat session from the list (size=" + groupChatSessions.size() + ")");
        }

        groupChatSessions.remove(chatId);
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
            case ChatLog.Message.Content.Status.SENDING:
                return Chat.MessageState.SENDING;

            case ChatLog.Message.Content.Status.SENT:
                return Chat.MessageState.SENT;

            case ChatLog.Message.Content.Status.UNREAD_REPORT:
            case ChatLog.Message.Content.Status.UNREAD:
            case ChatLog.Message.Content.Status.READ:
                return Chat.MessageState.DELIVERED;

            case ChatLog.Message.Content.Status.FAILED:
                return Chat.MessageState.FAILED;

            default:
                return Chat.MessageState.FAILED;
        }*/
        return 0;
    }

    /**
     * Initiates a group chat with a group of contact and returns a GroupChat
     * instance. The subject is optional and may be null.
     *
     * @param contact List of contacts
     * @param subject Subject
     * @param listener Chat event listener
     * @throws ServerApiException
     */
    public IGroupChat initiateGroupChat(List<String> contacts, String subject, IGroupChatListener listener) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Initiate an ad-hoc group chat session Subject:" + subject);
        }

        if (logger.isActivated()) {
            logger.info("initiateGroupChat contacts:" + contacts);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            final ChatSession session = Core.getInstance().getImService().initiateAdhocGroupChatSession(contacts, subject);

            // Add session listener
            GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session);
            sessionApi.addEventListener(listener);

            // Update rich messaging history
            RichMessagingHistory.getInstance().addGroupChat(
                    session.getContributionID(),
                    session.getSubject(),
                    session.getParticipants().getList(),
                    GroupChat.State.INITIATING.toInt(),
                    Direction.OUTGOING.toInt());

            if (RcsSettings.getInstance().isCPMSupported()) {
                if (logger.isActivated()) {
                    logger.info("initiateGroupChat conversationId: "
                            + session.getConversationID()
                            + " contributionId: "
                            + session.getContributionID());
                }
                RichMessagingHistory.getInstance().UpdateCoversationID(
                        session.getContributionID(),
                        session.getConversationID(),
                        2);
            }
            session.setChairman(ImsModule.IMS_USER_PROFILE.getUsername());
            RichMessagingHistory.getInstance().updateGroupChairman(session.getContributionID(), session.getChairman());

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            ChatServiceImpl.addGroupChatSession(sessionApi);
            return sessionApi;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Initiates a group chat with a group of contact and returns a GroupChat
     * instance. The subject is optional and may be null.
     *
     * @param contact List of contacts
     * @param subject Subject
     * @throws ServerApiException
     */
    public IGroupChat initiateGroupChat(List<ContactId> contactIds, String subject) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("initiateGroupChat:" + subject);
        }
        List<String> contacts = PhoneUtils.generateContactsStringList(contactIds);

        if (logger.isActivated()) {
            logger.info("initiateGroupChat contacts:" + contacts);
        }

        if (contacts == null || contacts.isEmpty()) {
            throw new ServerApiIllegalArgumentException(
                    "GroupChat participants list must not be null or empty!");
        }
        if (contacts.size() > mRcsSettings.getMaxChatParticipants() - 1) {
            throw new ServerApiIllegalArgumentException(
                    "Number of contacts exeeds maximum number that can be added to a group chat!");
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            final ChatSession session = Core.getInstance().getImService().initiateAdhocGroupChatSession(contacts, subject);
            GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session,this);
            //sessionApi.addEventListener(listener);

            // Update rich messaging history
            RichMessagingHistory.getInstance().addGroupChat(
                    session.getContributionID(),
                    "",
                    session.getSubject(),
                    session.getParticipantsWithStatus(),
                    GroupChat.State.INITIATING.toInt(),
                    ReasonCode.UNSPECIFIED.toInt(),
                    Direction.OUTGOING.toInt(),
                    Calendar.getInstance().getTimeInMillis());

            if (RcsSettings.getInstance().isCPMSupported()) {
                if (logger.isActivated()) {
                    logger.info("initiateGroupChat conversationId: "
                            + session.getConversationID()
                            + " contributionId: "
                            + session.getContributionID());
                }
                RichMessagingHistory.getInstance().UpdateCoversationID(
                        session.getContributionID(),
                        session.getConversationID(),
                        2);
            }
            session.setChairman(ImsModule.IMS_USER_PROFILE.getUsername());
            RichMessagingHistory.getInstance().updateGroupChairman(session.getContributionID(), session.getChairman());

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            ChatServiceImpl.addGroupChatSession(sessionApi);
            return sessionApi;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Initiates a group chat with a group of contact and returns a GroupChat
     * instance. The subject is optional and may be null.
     *
     * @param contact List of contacts
     * @param subject Subject
     * @param listener Chat event listener
     * @throws ServerApiException
     */
    public IGroupChat initiateClosedGroupChat(List<ContactId> contactIds, String subject) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("initiateClosedGroupChat:" + subject);
        }
        List<String> contacts = PhoneUtils.generateContactsStringList(contactIds);

        if (logger.isActivated()) {
            logger.info("initiateClosedGroupChat contacts:" + contacts);
        }

        if (contacts == null || contacts.isEmpty()) {
            throw new ServerApiIllegalArgumentException(
                    "GroupChat participants list must not be null or empty!");
        }
        if (contacts.size() > mRcsSettings.getMaxChatParticipants() - 1) {
            throw new ServerApiIllegalArgumentException(
                    "Number of contacts exeeds maximum number that can be added to a group chat!");
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            final ChatSession session = Core.getInstance().getImService().initiateClosedGroupChatSession(contacts, subject);
            GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session,this);
            //sessionApi.addEventListener(listener);

            // Update rich messaging history
            RichMessagingHistory.getInstance().addGroupChat(
                    session.getContributionID(),
                    "",
                    session.getSubject(),
                    session.getParticipantsWithStatus(),
                    GroupChat.State.INITIATING.toInt(),
                    ReasonCode.UNSPECIFIED.toInt(),
                    Direction.OUTGOING.toInt(),
                    Calendar.getInstance().getTimeInMillis());

            if (RcsSettings.getInstance().isCPMSupported()) {
                if (logger.isActivated()) {
                    logger.info("initiateGroupChat conversationId: "
                            + session.getConversationID()
                            + " contributionId: "
                            + session.getContributionID());
                }
                RichMessagingHistory.getInstance().UpdateCoversationID(
                        session.getContributionID(),
                        session.getConversationID(),
                        2);
            }
            session.setChairman(ImsModule.IMS_USER_PROFILE.getUsername());
            RichMessagingHistory.getInstance().updateGroupChairman(session.getContributionID(), session.getChairman());

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            ChatServiceImpl.addGroupChatSession(sessionApi);
            return sessionApi;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Initiates a group chat with a group of contact and returns a GroupChat
     * instance. The subject is optional and may be null.
     *
     * @param contact List of contacts
     * @param subject Subject
     * @param listener Chat event listener
     * @throws ServerApiException
     */
    public IGroupChat initiateClosedGroupChat(List<String> contacts, String subject, IGroupChatListener listener) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Initiate an closed group chat session Subject:" + subject);
        }

        if (logger.isActivated()) {
            logger.info("initiateClosedGroupChat contacts:" + contacts);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            final ChatSession session = Core.getInstance().getImService().initiateClosedGroupChatSession(contacts, subject);

            // Add session listener
            GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session);
            sessionApi.addEventListener(listener);

            // Update rich messaging history
            RichMessagingHistory.getInstance().addGroupChat(session.getContributionID(),
                    session.getSubject(), session.getParticipants().getList(),
                    GroupChat.State.INITIATING.toInt(), Direction.OUTGOING.toInt());
            session.setChairman(ImsModule.IMS_USER_PROFILE.getUsername());
            RichMessagingHistory.getInstance().updateGroupChairman(session.getContributionID(), session.getChairman());

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            ChatServiceImpl.addGroupChatSession(sessionApi);
            return sessionApi;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Rejoins an existing group chat from its unique chat ID
     *
     * @param chatId Chat ID
     * @return Group chat
     * @throws ServerApiException
     */
    public IGroupChat rejoinGroupChat(String chatId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Rejoin group chat session related to the conversation " + chatId);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            final ChatSession session = Core.getInstance().getImService().rejoinGroupChatSession(chatId);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session,this);
            ChatServiceImpl.addGroupChatSession(sessionApi);
            return sessionApi;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Rejoins an existing group chat from its unique chat ID
     *
     * @param chatId Chat ID
     * @return Group chat
     * @throws ServerApiException
     */
    public IGroupChat rejoinGroupChatId(String chatId, String rejoinId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Rejoin group chat session related to the conversation " + chatId + "; rejoinId: " + rejoinId);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            final ChatSession session = Core.getInstance().getImService().rejoinGroupChatSession(chatId,rejoinId);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session,this);
            ChatServiceImpl.addGroupChatSession(sessionApi);
            return sessionApi;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Restarts a previous group chat from its unique chat ID
     *
     * @param chatId Chat ID
     * @return Group chat
     * @throws ServerApiException
     */
    public IGroupChat restartGroupChat(String chatId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Restart group chat session related to the conversation " + chatId);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            final ChatSession session = Core.getInstance().getImService().restartGroupChatSession(chatId);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            GroupChatImpl sessionApi = new GroupChatImpl((GroupChatSession)session,this);
            ChatServiceImpl.addGroupChatSession(sessionApi);
            return sessionApi;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    public void syncAllGroupChats(IGroupChatSyncingListener listener) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Sync all group chats from server");
        }
        ServerApiUtils.testIms();
        new GrouplistSubscriber(listener).start();
    }

    public void syncGroupChat(String chatId, IGroupChatSyncingListener listener)
            throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Sync one group chat from server");
        }
        ServerApiUtils.testIms();

        InstantMessagingService imService = Core.getInstance().getImsModule().getInstantMessagingService();

        if (chatId == null) {
            if (logger.isActivated()) {
                logger.error("Sync group chat info with null chatId");
            }
            throw new ServerApiException("Invalid Argument");
        }

        /* No need to sync group chat offline when session is active */
        Vector<ChatSession> sessions = imService.getImSessions();
        for (int i=0; i < sessions.size(); i++) {
            ChatSession session = sessions.get(i);
            if (session instanceof GroupChatSession) {
                if (chatId.equals(session.getContributionID()))
                    return;
            }
        }

        new GroupInfoSubscriber(chatId, listener).start();
    }

    /**
     * Returns the list of group chats in progress
     *
     * @return List of group chat
     * @throws ServerApiException
     */
    public List<IBinder> getGroupChats() throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get group chat sessions");
        }

        try {
            ArrayList<IBinder> result = new ArrayList<IBinder>(groupChatSessions.size());
            for (Enumeration<IGroupChat> e = groupChatSessions.elements() ; e.hasMoreElements() ;) {
                IGroupChat sessionApi = (IGroupChat)e.nextElement() ;
                result.add(sessionApi.asBinder());
            }
            return result;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Returns a group chat in progress from its unique ID
     *
     * @param chatId Chat ID
     * @return Group chat or null if not found
     * @throws ServerApiException
     */
    public IGroupChat getGroupChat(String chatId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Get group chat session " + chatId);
        }

        // Return a session instance
        IGroupChat groupChat = groupChatSessions.get(chatId);
        if(groupChat == null) {
            groupChat = new GroupChatImpl(chatId, this);
            ChatServiceImpl.addGroupChatSession((GroupChatImpl)groupChat);
        }

        return groupChat;
    }

    public boolean isGroupChatAbandoned(String chatId){
        if (logger.isActivated()) {
            logger.info("isGroupChatAbandoned " + chatId);
        }
        GroupChatImpl groupChat =  null;
        try{
            groupChat = (GroupChatImpl)getGroupChat(chatId);
            return groupChat.isGroupChatAbandoned();
        } catch(Exception e){
            e.printStackTrace();
            return false;
        }

    }

    public void markMessageAsRead(String msgId){
        if (logger.isActivated()) {
           logger.info("markMessageAsRead msgid:" + msgId);
        }
        if(msgId == null) return;
        try {
            if (mRcsSettings.isImReportsActivated()
                    && mRcsSettings.isRespondToDisplayReports()) {
                if (logger.isActivated()) {
                    logger.debug("tryToDispatchAllPendingDisplayNotifications for msgID "
                            .concat(msgId));
                }
                int status = messagingLog.getMessageStatus(msgId);
                if (logger.isActivated()) {
                    logger.info("markMessageAsRead contact status: " + status);
                }
                if(status != ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt()){
                    if (logger.isActivated()) {
                        logger.info("markMessageAsRead status is not readreport");
                    }
                    return;
                }
                ImdnManager imdnManager = imsService.getImdnManager();
                /*if (imdnManager.isSendOneToOneDeliveryDisplayedReportsEnabled()
                        || imdnManager.isSendGroupDeliveryDisplayedReportsEnabled()) {
                    mImService.tryToDispatchAllPendingDisplayNotifications();
                }*/
                String chatid = messagingLog.getMessageChatId(msgId);
                if (logger.isActivated()) {
                    logger.info("LMM Set displayed delivery report chatid: " + chatid);
                }
                IGroupChat groupChat = groupChatSessions.get(chatid);
                if(groupChat != null){
                    markGroupMessageAsRead(msgId,groupChat);
                    RichMessagingHistory.getInstance().updateChatMessageStatus(msgId, ChatLog.Message.Content.Status.DISPLAYED.toInt());
                    return;
                }

                chatid = messagingLog.getMessageContact(msgId);
                if (logger.isActivated()) {
                    logger.info("LMM Set displayed delivery report contact: " + chatid);
                }
                IOneToOneChat o2oChat = chatSessions.get(chatid);
                if(o2oChat != null){
                    markO2OMessageAsRead(msgId,o2oChat);
                    RichMessagingHistory.getInstance().updateChatMessageStatus(msgId, ChatLog.Message.Content.Status.DISPLAYED.toInt());
                    return;
                }
                IExtendChat extendChat = extendChatSessions.get(chatid);
                if(extendChat != null){
                    markExtendMessageAsRead(msgId,extendChat);
                    RichMessagingHistory.getInstance().updateChatMessageStatus(msgId, ChatLog.Message.Content.Status.DISPLAYED.toInt());
                    return;
                }
            }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("markMessageAsRead exception msgid:" + msgId);
             }
            e.printStackTrace();
        }
    }

    public void markO2OMessageAsRead(final String msgId, final IOneToOneChat chat){
        if (logger.isActivated()) {
            logger.info("markO2OMessageAsRead contact msgid: " + msgId);
        }
        int status = messagingLog.getMessageStatus(msgId);
        if (logger.isActivated()) {
            logger.info("markO2OMessageAsRead contact status: " + status);
        }
        if(status != ChatLog.Message.Content.Status.DISPLAY_REPORT_REQUESTED.toInt()){
            if (logger.isActivated()) {
                logger.info("markO2OMessageAsRead status is not readreport");
            }
            return;
        }
        Thread t = new Thread() {
            public void run() {
                ((OneToOneChatImpl)chat).sendDisplayedDeliveryReport(msgId);
            }
        };
        t.start();
    }

    public void markExtendMessageAsRead(final String msgId, final IExtendChat chat){
        if (logger.isActivated()) {
            logger.info("markExtendMessageAsRead contact msgid: " + msgId);
        }
        Thread t = new Thread() {
            public void run() {
                ((ExtendChatImpl)chat).sendDisplayedDeliveryReport(msgId);
            }
        };
        t.start();
    }

    public void markGroupMessageAsRead(final String msgId, final IGroupChat chat){
        if (logger.isActivated()) {
            logger.info("markGroupMessageAsRead contact msgid: " + msgId);
        }
        Thread t = new Thread() {
            public void run() {
                ((GroupChatImpl)chat).sendDisplayedDeliveryReport(msgId);
            }
        };
        t.start();
    }

    /**
     * Block messages in group, stack will not notify application about
     * any received message in this group
     *
     * @param chatId chatId of the group
     * @param flag true means block the message, false means unblock it
     * @throws JoynServiceException
     */
    public void blockGroupMessages(String chatId, boolean flag) {
         if (logger.isActivated()) {
                logger.info("GCM blockGroupMessages flag:" + flag);
        }
        GroupChatImpl groupImpl = (GroupChatImpl)groupChatSessions.get(chatId);
        if(groupImpl != null){
            groupImpl.blockMessages(flag);
        } else{
            // Only Update flag in DB, can't update in session
            if(flag == true) {
                RichMessagingHistory.getInstance().updateGroupBlockedStatus(chatId, 1);
            } else {
                RichMessagingHistory.getInstance().updateGroupBlockedStatus(chatId, 0);
            }
        }
    }

    /**
     * Adds a listener on new chat invitation events
     *
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void addEventListener2(IOneToOneChatListener listener) throws ServerApiException {

        try {
            synchronized (lock) {
                mOneToOneChatEventBroadcaster.addOneToOneChatEventListener(listener);
            }
        } catch (ServerApiBaseException e) {
            if (logger.isActivated()) {
                logger.info("addEventListener2 exception");
                e.printStackTrace();
            }
            throw e;
        }
    }

    /**
     * Removes a listener on new chat invitation events
     *
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void removeEventListener2(IOneToOneChatListener listener) throws ServerApiException {

        try {
            synchronized (lock) {
                mOneToOneChatEventBroadcaster.removeOneToOneChatEventListener(listener);
            }
        } catch (ServerApiBaseException e) {
            if (logger.isActivated()) {
                logger.info("removeEventListener2 exception");
            }
            throw e;
        }
    }

    /**
     * Adds a listener on new chat invitation events
     *
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void addEventListener3(IGroupChatListener listener) throws ServerApiException {

        try {
            synchronized (lock) {
                mGroupChatEventBroadcaster.addGroupChatEventListener(listener);
            }
        } catch (ServerApiBaseException e) {
            if (logger.isActivated()) {
                logger.info("addEventListener3 exception");
                e.printStackTrace();
            }
            throw e;
        }
    }

    /**
     * Removes a listener on new chat invitation events
     *
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void removeEventListener3(IGroupChatListener listener) throws ServerApiException {

        try {
            synchronized (lock) {
                mGroupChatEventBroadcaster.removeGroupChatEventListener(listener);
            }
        } catch (ServerApiBaseException e) {
            if (logger.isActivated()) {
                logger.info("removeEventListener3 exception");
            }
            throw e;
        }
    }

    /**
     * Adds a listener on new chat invitation events
     *
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void addEventListenerExtend2(IExtendChatListener listener) throws ServerApiException {

        try {
            synchronized (lock) {
                mExtendChatEventBroadcaster.addOneToOneChatEventListener(listener);
            }
        } catch (ServerApiBaseException e) {
            if (logger.isActivated()) {
                logger.info("addEventListenerExtend2 exception");
                e.printStackTrace();
            }
            throw e;
        }
    }

    /**
     * Removes a listener on new chat invitation events
     *
     * @param listener Chat invitation listener
     * @throws ServerApiException
     */
    public void removeEventListenerExtend2(IExtendChatListener listener) throws ServerApiException {

        try {
            synchronized (lock) {
                mExtendChatEventBroadcaster.removeOneToOneChatEventListener(listener);
            }
        } catch (ServerApiBaseException e) {
            if (logger.isActivated()) {
                logger.info("removeEventListenerExtend2 exception");
            }
            throw e;
        }
    }

    /**
     * Returns the configuration of the chat service
     *
     * @return Configuration
     */
    @Override
    public IChatServiceConfiguration getConfiguration() {
        return new ChatServiceConfigurationImpl(mRcsSettings);
    }

    /**
     * Returns the common service configuration
     *
     * @return the common service configuration
     */
    @Override
    public ICommonServiceConfiguration getCommonConfiguration() {
        return new CommonServiceConfigurationImpl(mRcsSettings);
    }

    /**
     * Registers a new chat invitation listener
     *
     * @param listener New file transfer listener
     * @throws ServerApiException
     */
    public void addNewChatListener(INewChatListener listener) throws ServerApiException {
        listeners.register(listener);
    }

    /**
     * Unregisters a chat invitation listener
     *
     * @param listener New file transfer listener
     * @throws ServerApiException
     */
    public void removeNewChatListener(INewChatListener listener) throws ServerApiException {
        listeners.unregister(listener);
    }

    /**
     * Returns IM always on or not
     *
     * @return boolean value
     * @throws ServerApiException
     */
    public boolean isImCapAlwaysOn(){
        return RcsSettings.getInstance().isImAlwaysOn();
    }

    /**
     * Returns service version
     *
     * @return Version
     * @see Build.VERSION_CODES
     * @throws ServerApiException
     */
    public int getServiceVersion() throws ServerApiException {
        return RcsService.Build.API_VERSION;
    }

    public IChatMessage getChatMessage(String msgId) throws ServerApiException {
        //return Build.API_VERSION;
        try {
            ChatMessagePersistedStorageAccessor persistedStorage = new ChatMessagePersistedStorageAccessor(
                    messagingLog, msgId);
            return new ChatMessageImpl(persistedStorage);

        } catch (ServerApiBaseException e) {
            e.printStackTrace();
            throw e;
        }

    }

    /**
     * Returns true if it is possible to initiate a group chat now, else returns false.
     *
     * @return boolean
     * @throws RemoteException
     */
    @Override
    public boolean isAllowedToInitiateGroupChat() throws RemoteException {
        try {
            if (!mRcsSettings.isGroupChatActivated()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot initiate group chat as group chat feature is not supported.");
                }
                return false;
            }
            if (!mRcsSettings.getMyCapabilities().isImSessionSupported()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot initiate group chat as IM capabilities are not supported for self.");
                }
                return false;
            }
            if (!ServerApiUtils.isImsConnected()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot initiate group chat as IMS is not connected.");
                }
                return false;
            }
            return true;

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    /**
     * Returns true if it's possible to initiate a new group chat with the specified contactId right
     * now, else returns false.
     *
     * @param contact Remote contact
     * @return true if it's possible to initiate a new group chat
     * @throws RemoteException
     */
    @Override
    public boolean isAllowedToInitiateGroupChat2(ContactId contactId) throws RemoteException {
        if (contactId == null) {
            throw new ServerApiIllegalArgumentException("contact must not be null!");
        }
        String contact = contactId.toString();
        try {
            if (!isAllowedToInitiateGroupChat()) {
                return false;
            }
            Capabilities contactCapabilities = ContactsManager.getInstance().getContactCapabilities(contact);
            if (contactCapabilities == null) {
                if (logger.isActivated()) {
                    logger.debug("Cannot initiate group chat as the capabilities of the participant '"
                            + contact + "' are not known.");
                }
                return false;
            }
            if (!contactCapabilities.isImSessionSupported()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot initiate group chat as the participant '" + contact
                            + "' does not have IM capabilities.");
                }
                return false;
            }
            /*if (mRcsSettings.isGroupChatInviteIfFullStoreForwardSupported()
                    && !contactCapabilities.isGroupChatStoreForwardSupported()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot initiate group chat as the participant '" + contact
                            + "' does not have store and forward feature supported.");
                }
                return false;
            }*/
            return true;

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.debug("Cannot initiate group chat due to exceptionz");
            }
            e.printStackTrace();
            throw e;

        }
    }

    private void deleteExtendChats(){
        if (logger.isActivated()) {
            logger.debug("deleteExtendChats size: " + extendChatSessions.size());
        }
        Enumeration<String> extendChatIterator = extendChatSessions.keys();
        while(extendChatIterator.hasMoreElements()) {
            String key = extendChatIterator.nextElement();
            IExtendChat extendChat = extendChatSessions.get(key);
            extendChatSessions.remove(key);
            // remove from DB

        }
    }

    public void deleteOneToOneChats(){
        if (logger.isActivated()) {
            logger.debug("deleteOneToOneChats size: " + chatSessions.size());
        }
        Enumeration<String> o2oChatIterator = chatSessions.keys();
        while(o2oChatIterator.hasMoreElements()) {
            String key = o2oChatIterator.nextElement();
            IOneToOneChat o2oChat = chatSessions.get(key);
            chatSessions.remove(key);
            ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(key);
            deleteOneToOneChat(contactId);
        }
    }

    public void deleteGroupChats() {
        if (logger.isActivated()) {
            logger.debug("deleteGroupChats size: " + groupChatSessions.size());
        }
        Enumeration<String> groupChatIterator = groupChatSessions.keys();
        final Set<String> chatIds = new TreeSet<String>();
        while(groupChatIterator.hasMoreElements()) {
            String key = groupChatIterator.nextElement();
            IGroupChat groupChat = groupChatSessions.get(key);
            groupChatSessions.remove(key);
            chatIds.add(key);
        }
        Thread t = new Thread(){
            public void run(){
              deleGroupChatsFromDb(chatIds);
           }
        };
        t.start();
    }

    public void deleGroupChatsFromDb(Set<String> chatIds){
        for(String chatId : chatIds) {
            messagingLog.deleteGroupChat(chatId);
            broadcastGroupChatsDeleted(chatIds);
        }
    }

    public void deleteOneToOneChat(ContactId contactId){
        if (logger.isActivated()) {
            logger.debug("deleteOneToOneChat: " + contactId);
        }
        if(contactId == null) return;
        final String contact = contactId.toString();
        IOneToOneChat o2oChat = chatSessions.get(contact);
        if(o2oChat != null){
            chatSessions.remove(contact);
            Thread t = new Thread(){
                public void run(){
                  messagingLog.deleteO2OChat(contact);
               }
           };
           t.start();

        } else {
            IExtendChat extendChat = extendChatSessions.get(contact);
            extendChatSessions.remove(contact);
            Thread t = new Thread(){
                public void run(){
                    messagingLog.deleteO2OChat(contact);
                 }
             };
             t.start();
        }
    }

    public void deleteGroupChat(final String chatId){
        if (logger.isActivated()) {
            logger.debug("deleteGroupChat : " + chatId);
        }
        IGroupChat groupChat = groupChatSessions.get(chatId);
        groupChatSessions.remove(chatId);
        Thread t = new Thread(){
             public void run(){
               //delete chat from DB
                 Set<String> chatIds = new HashSet<String>();
                 chatIds.add(chatId);
                 messagingLog.deleteGroupChat(chatId);
                 broadcastGroupChatsDeleted(chatIds);
            }
        };
        t.start();
    }

    public void deleteMessage(final String msgId){
        if (logger.isActivated()) {
            logger.debug("deleteMessage : " + msgId);
        }
        //Delete from DB
        /*if(messagingLog.isOne2OneMessageExists(msgId)){
            Thread t = new Thread(){
                public void run(){
                    deleteO2OMessage(msgId);
               }
           };
           t.start();
        } else if(messagingLog.isGroupMessageExists(msgId)){
            Thread t = new Thread(){
                public void run(){
                    deleteGroupMessage(msgId);
               }
           };
           t.start();
        }*/
        try{
            //messagingLog.isOne2OneMessageExists(msgId);
        } catch(Exception e){
            e.printStackTrace();
        }

        try{
            //messagingLog.isGroupMessageExists(msgId);
        } catch(Exception e){
            e.printStackTrace();
        }
        messagingLog.deleteMessage(msgId);
    }

    public void deleteO2OMessage(final String msgId){
        messagingLog.deleteMessage(msgId);
    }

    public void deleteGroupMessage(final String msgId){
        messagingLog.deleteMessage(msgId);
    }

    public void broadcastGroupChatMessagesDeleted(String chatId, Set<String> msgIds) {
        mGroupChatEventBroadcaster.broadcastMessagesDeleted(chatId, msgIds);
    }

    public void broadcastGroupChatsDeleted(Set<String> chatIds) {
        mGroupChatEventBroadcaster.broadcastGroupChatsDeleted(chatIds);
    }

    public void broadcastOneToOneMessagesDeleted(String contact, Set<String> msgIds) {
        ContactId contactid = ContactIdUtils.createContactIdFromTrustedData(contact);
        mOneToOneChatEventBroadcaster.broadcastMessagesDeleted(contactid, msgIds);
    }

    public void clearMessageDeliveryExpiration(final List<String> msgIds) throws RemoteException {
        if (logger.isActivated()) {
            logger.debug("deleteOneToOneChats size: " + chatSessions.size());
        }
        if (msgIds == null || msgIds.isEmpty()) {
            throw new ServerApiIllegalArgumentException(
                    "Undelivered chat messageId list must not be null and empty!");
        }
        Runnable run = new Runnable() {
            @Override
            public void run() {
                try {
                    for (final String msgId : msgIds) {
                        imsService.getDeliveryExpirationManager().cancelDeliveryTimeoutAlarm(msgId);
                    }
                    messagingLog.clearMessageDeliveryExpiration(msgIds);
                } catch (RuntimeException e) {
                    /*
                     * Normally we are not allowed to catch runtime exceptions as these are genuine
                     * bugs which should be handled/fixed within the code. However the cases when we
                     * are executing operations on a thread unhandling such exceptions will
                     * eventually lead to exit the system and thus can bring the whole system down,
                     * which is not intended.
                     */
                    logger.error("Failed to mark message as read!", e);
                }
            }
        };
        Thread t= new Thread(run);
        t.start();
    }

    /**
     * Broadcasts Group Chat state change
     *
     * @param chatId Chat id
     * @param state State
     * @param reasonCode Reason code
     */
    public void broadcastGroupChatStateChange(String chatId, State state,
            GroupChat.ReasonCode reasonCode) {
        mGroupChatEventBroadcaster.broadcastStateChanged(chatId, state, reasonCode);
    }

    public IGroupChatEventBroadcaster getGroupEventBroadcaster(){
        return mGroupChatEventBroadcaster;
    }

    public IOneToOneChatEventBroadcaster getOneToOnEventBroadcaster(){
        return mOneToOneChatEventBroadcaster;
    }

    public IExtendChatEventBroadcaster getExtendChatEventBroadcaster(){
        return mExtendChatEventBroadcaster;
    }

    private static class GrouplistSubscriber extends Thread {
        public GrouplistSubscriber(IGroupChatSyncingListener listener) {
            this.listener = listener;
        }

        public void terminate() {
            beenCanceled = true;
        }

        @Override
        public void run() {
            boolean missing = false;
            synchronized(listLock) {
                subscribeGrouplist();
                try {
                    listLock.wait();
                    if (logger.isActivated()) {
                        logger.info("sync group list done!");
                    }
                    listener.onSyncStart(basicGroupInfos.size());
                    if (basicGroupInfos.size() == 0) {
                        listener.onSyncDone(0);
                    } else {
                        while (basicGroupInfos.size() > 0) {
                            synchronized (infoLock) {
                                String rejoinId = basicGroupInfos.get(0).getUri();
                                String convId = basicGroupInfos.get(0).getConversationid();
                                subscribeGroupInfo(rejoinId, convId);
                                try {
                                    infoLock.wait();
                                    if (eventData == null) {
                                        missing = true;
                                    }
                                    else {
                                        logger.info("callback to group info handler");
                                        String chatId = RichMessagingHistory.getInstance()
                                                .getChatIdbyRejoinId(rejoinId);
                                        listener.onSyncInfo(chatId, eventData);
                                    }
                                } catch (InterruptedException e) {
                                    missing = true;
                                } finally {
                                    basicGroupInfos.remove(0);
                                }
                            }
                        }
                        if (missing)
                            listener.onSyncDone(-1);
                        else
                            listener.onSyncDone(0);
                    }
                } catch (InterruptedException e) {
                    try {
                        listener.onSyncDone(-1);
                    } catch (Exception e2) {

                    }
                } catch (Exception e1) {

                }
            }
        }

        private void subscribeGrouplist() {
            SubscriptionManager manager = SubscriptionManager.getInstance();

            SubscriptionManager.EventCallback eventCallback = new SubscriptionManager.EventCallback() {
                @Override
                protected void handleEventNotify(byte[] content) {
                    if (logger.isActivated()) {
                        logger.info("subscribe group list receive notify");
                    }
                    if (content == null || content.length == 0)
                        return;
                    try {
                        if (logger.isActivated()) {
                            logger.info("group list not null");
                        }
                        InputSource input = new InputSource(new ByteArrayInputStream(content));
                        GroupListParser parser = new GroupListParser(input);
                        GroupListDocument info = parser.getGroupList();

                        basicGroupInfos = info.getGroups();
                        if (logger.isActivated()) {
                            logger.info("group list size: " + basicGroupInfos.size());
                        }
                    } catch (Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't parse XML notification", e);
                        }
                    } finally {
                        synchronized (listLock) {
                            listLock.notify();
                        }
                    }
                }

                @Override
                protected void onActive(String identity) {
                }

                @Override
                protected void onPending(String identity) {
                }

                @Override
                protected void onTerminated(String reason, int retryAfter) {
                }
            };

            String contentType = "application/grouplist-ver+xml";
            String content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
                             "<grouplist-ver version=\"0\">" + SipUtils.CRLF +
                             "</grouplist-ver>";

            SubscribeRequest request = new SubscribeRequest.Builder()
                .setRequestUri(RcsSettings.getInstance().getImConferenceUri())
                .setSubscibeEvent("grouplist")
                .setAcceptContent("application/conference-info+xml")
                .setContent(contentType, content.getBytes())
                .build();
            manager.pollStatus(request, eventCallback);
        }

        private void subscribeGroupInfo(final String groupId, final String convId) {
            SubscriptionManager manager = SubscriptionManager.getInstance();

            if (logger.isActivated()) {
                logger.info("subscribeGroupInfo->groupId[" + groupId + "] convId[" + convId + "]");
            }

            SubscriptionManager.EventCallback callback = new SubscriptionManager.EventCallback() {
                @Override
                protected void handleEventNotify(byte[] content) {
                    logger.info("handle group info notify");
                    if (content == null || content.length == 0)
                        return;
                    try {
                        InputSource input = new InputSource(new ByteArrayInputStream(content));
                        ConferenceInfoParser parser = new ConferenceInfoParser(input);
                        ConferenceInfoDocument info = parser.getConferenceInfo();

                        if (info == null)
                            return;

                        String chatId = updateGroupChat(groupId, convId, info);
                        ArrayList<ConferenceUser> confUsers =
                                new ArrayList<ConferenceUser>();

                        Vector<User> users = info.getUsers();
                        for (User user:users) {
                            ConferenceUser confUser =
                                    new ConferenceUser(
                                            user.getEntity(),
                                            user.getUserState(),
                                            user.getState(),
                                            user.getDisconnectionMethod(),
                                            user.getRole(),
                                            user.getEtype(),
                                            user.getDisplayName());
                            confUsers.add(confUser);
                        }
                        eventData = new ConferenceEventData(
                                "full",
                                URLDecoder.decode(info.getSubject(), "UTF-8"),
                                info.getChairman(),
                                confUsers);
                    } catch (Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't parse XML notification", e);
                        }
                    } finally {
                        synchronized (infoLock) {
                            infoLock.notify();
                        }
                    }
                }

                @Override
                protected void onActive(String identity) {
                }

                @Override
                protected void onPending(String identity) {
                }

                @Override
                protected void onTerminated(String reason, int retryAfter) {
                }
            };

            eventData = null;
            SubscribeRequest request = new SubscribeRequest.Builder()
                .setRequestUri(groupId)
                .setSubscibeEvent("groupinfo")
                .setAcceptContent("application/conference-info+xml")
                .build();
            manager.pollStatus(request, callback);
        }

        private String updateGroupChat(String rejoinId, String convId, ConferenceInfoDocument info)
            throws Exception{
            RichMessagingHistory msgHistory = RichMessagingHistory.getInstance();
            GroupChatInfo chatInfo = msgHistory.getGroupChatInfoByRejoinId(rejoinId);
            String chatId = null;

            if (chatInfo == null) {
                String callId = Core.getInstance().getImsModule().getSipManager().getSipStack().generateCallId();
                chatId = ContributionIdGenerator.getContributionId(callId);

                Vector<User> users = info.getUsers();
                Map<String, ParticipantStatus> participants = new HashMap<>();

                for (User user:users) {
                    ParticipantStatus status = user.getParticipantStatus(user.getState());
                    participants.put(user.getEntity(),status);
                }

                msgHistory.addGroupChat(
                            chatId,
                            "",
                            URLDecoder.decode(info.getSubject(), "UTF-8"),
                            participants,
                            GroupChat.State.REJECTED.toInt(),
                            0,
                            Direction.INCOMING.toInt(),
                            Calendar.getInstance().getTimeInMillis());

                msgHistory.updateGroupChatRejoinId(chatId, rejoinId);
                msgHistory.updateGroupChairman(chatId, info.getChairman());
                msgHistory.UpdateCoversationID(chatId, convId, 2);
            } else {
                chatId = chatInfo.getContributionId();
                msgHistory.updateGroupChatSubject(chatId,
                    URLDecoder.decode(info.getSubject(), "UTF-8"));
                msgHistory.updateGroupChairman(chatId, info.getChairman());
            }
            return chatId;
        }

        private IGroupChatSyncingListener listener;
        private List<BasicGroupInfo> basicGroupInfos;
        ConferenceEventData eventData;

        Object listLock = new Object();
        Object infoLock = new Object();
        boolean beenCanceled = false;
    }

    private static class GroupInfoSubscriber extends Thread {
        private String chatId;
        private IGroupChatSyncingListener listener;

        public GroupInfoSubscriber(String chatId, IGroupChatSyncingListener listener) {
            this.chatId = chatId;
            this.listener = listener;
        }

        @Override
        public void run() {
            GroupChatInfo chatInfo = RichMessagingHistory.getInstance().getGroupChatInfo(chatId);
            subscribeGroupInfo(chatInfo.getRejoinId());
        }

        private void subscribeGroupInfo(final String groupId) {
            SubscriptionManager manager = SubscriptionManager.getInstance();

            SubscriptionManager.EventCallback callback = new SubscriptionManager.EventCallback() {
                @Override
                protected void handleEventNotify(byte[] content) {
                    if (content == null || content.length == 0)
                        return;
                    try {
                        InputSource input = new InputSource(new ByteArrayInputStream(content));
                        ConferenceInfoParser parser = new ConferenceInfoParser(input);
                        ConferenceInfoDocument info = parser.getConferenceInfo();

                        if (info == null)
                            return;

                        String chatId = updateGroupChat(groupId, info);
                        ArrayList<ConferenceUser> confUsers =
                                new ArrayList<ConferenceUser>();

                        Vector<User> users = info.getUsers();
                        for (User user:users) {
                            ConferenceUser confUser =
                                    new ConferenceUser(
                                            user.getEntity(),
                                            user.getUserState(),
                                            user.getState(),
                                            user.getDisconnectionMethod(),
                                            user.getRole(),
                                            user.getEtype(),
                                            user.getDisplayName());
                            confUsers.add(confUser);
                        }
                        listener.onSyncInfo(chatId, new ConferenceEventData(
                                "full",
                                URLDecoder.decode(info.getSubject(), "UTF-8"),
                                info.getChairman(),
                                confUsers));
                    } catch (Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't parse XML notification", e);
                        }
                    } finally {
                    }
                }

                @Override
                protected void onActive(String identity) {
                }

                @Override
                protected void onPending(String identity) {
                }

                @Override
                protected void onTerminated(String reason, int retryAfter) {
                }
            };

            SubscribeRequest request = new SubscribeRequest.Builder()
                .setRequestUri(groupId)
                .setSubscibeEvent("groupinfo")
                .setAcceptContent("application/conference-info+xml")
                .build();
            manager.pollStatus(request, callback);
        }

        private String updateGroupChat(String rejoinId, ConferenceInfoDocument info) throws Exception {
            RichMessagingHistory msgHistory = RichMessagingHistory.getInstance();
            GroupChatInfo chatInfo = msgHistory.getGroupChatInfoByRejoinId(rejoinId);
            String chatId = null;

            if (chatInfo == null) {
                String callId = Core.getInstance().getImsModule().getSipManager().getSipStack().generateCallId();
                chatId = ContributionIdGenerator.getContributionId(callId);

                Vector<User> users = info.getUsers();
                Map<String, ParticipantStatus> participants = new HashMap<>();

                for (User user:users) {
                    ParticipantStatus status = user.getParticipantStatus(user.getState());
                    participants.put(user.getEntity(),status);
                }

                msgHistory.addGroupChat(
                            chatId,
                            null,
                            URLDecoder.decode(info.getSubject(), "UTF-8"),
                            participants,
                            GroupChat.State.REJECTED.toInt(),
                            0,
                            Direction.INCOMING.toInt(),
                            Calendar.getInstance().getTimeInMillis());

                msgHistory.updateGroupChatRejoinId(chatId, rejoinId);
                msgHistory.updateGroupChairman(chatId, info.getChairman());
            } else {
                chatId = chatInfo.getContributionId();
                msgHistory.updateGroupChatSubject(chatId,
                    URLDecoder.decode(info.getSubject(), "UTF-8"));
                msgHistory.updateGroupChairman(chatId, info.getChairman());
            }
            return chatId;
        }
    }
}
