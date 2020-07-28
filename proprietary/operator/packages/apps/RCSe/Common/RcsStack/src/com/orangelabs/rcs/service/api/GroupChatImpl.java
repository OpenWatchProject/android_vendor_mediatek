package com.orangelabs.rcs.service.api;

import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.ChatLog.GroupChatEvent;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.Status;
import com.gsma.services.rcs.chat.IChatMessage;
import com.gsma.services.rcs.chat.ConferenceEventData.ConferenceUser;
import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.ChatLog.Message.Content;
//import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.GroupChat.ReasonCode;
import com.gsma.services.rcs.chat.IGroupChat;
import com.gsma.services.rcs.chat.IGroupChatListener;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo;

import android.content.Intent;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.widget.Toast;

import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession.TerminationReason;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatError;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSessionListener;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.GeolocPush;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatInfo;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.InstantMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.OriginatingAdhocGroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.RejoinGroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.RestartGroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.messaging.ChatMessagePersistedStorageAccessor;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.utils.ContactIdUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.service.broadcaster.GroupChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.IGroupChatEventBroadcaster;
import com.orangelabs.rcs.service.broadcaster.IGroupFileTransferBroadcaster;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Group chat implementation
 * 
 * @author Jean-Marc AUFFRET
 */
public class GroupChatImpl extends IGroupChat.Stub implements ChatSessionListener {
    
    public enum State{
        UNKNOW,
        ACTIVE,
        REJOINING,
        MANUAL_REJOIN,
        RESTARTING,
        MANUAL_RESTART;
    }
    
    /**
     * Core session
     */
    private GroupChatSession session;
    
    private IGroupChatEventBroadcaster mBroadcaster = null;
    
    /**
     * Group Chat Id
     */
    private String mChatId;
    
    /**
     * Chat Service
     */
    private ChatServiceImpl mChatService;
    
    /**
     * IM Service
     */
    private InstantMessagingService mImService;
    
    /**
    * RcsSettings
    */
    private RcsSettings mRcsSettings;
    
    /**
     * RichMessagingHistory
     */
    private RichMessagingHistory mMessagingLog;
    
    /**
     * State
     */
    private State mState = State.UNKNOW;
    
    /**
     * List of listeners
     */
    private RemoteCallbackList<IGroupChatListener> listeners = new RemoteCallbackList<IGroupChatListener>();

    /**
     * Lock used for synchronisation
     */
    private Object lock = new Object();
    
    private Object rejoinLock = null;

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());
    
    private CopyOnWriteArrayList<InstantMessage> mPendingMessage =
            new CopyOnWriteArrayList<InstantMessage>();

    /**
     * Constructor
     * 
     * @param session Session
     */
    public GroupChatImpl(GroupChatSession session) {
        this.session = session;
        
        session.addListener(this);
        mRcsSettings = RcsSettings.getInstance();
        mImService = Core.getInstance().getImService();
        mMessagingLog = RichMessagingHistory.getInstance();
        this.mChatId =  session.getContributionID();
    }
    
    /**
     * Constructor
     * 
     * @param session Session
     */
    public GroupChatImpl(GroupChatSession session, ChatServiceImpl chatService) {
        this.session = session;
        this.mChatService = chatService;
        session.addListener(this);
        mImService = Core.getInstance().getImService();
        mRcsSettings = RcsSettings.getInstance();
        mBroadcaster = mChatService.getGroupEventBroadcaster();
        mMessagingLog = RichMessagingHistory.getInstance();
        this.mChatId =  session.getContributionID();
    }
    
    /**
     * Constructor
     * 
     * @param session Session
     */
    public GroupChatImpl(String chatId, ChatServiceImpl chatService) {
        if (logger.isActivated()) {
            logger.info("GroupChatImpl chatId: " + chatId );
        }
        this.mChatId = chatId;
        this.mChatService = chatService;
        mImService = Core.getInstance().getImService();
        mRcsSettings = RcsSettings.getInstance();
        mBroadcaster = mChatService.getGroupEventBroadcaster();
        mMessagingLog = RichMessagingHistory.getInstance();
        this.mState = State.MANUAL_REJOIN;
    }
    
    /**
     * Get chat ID
     * 
     * @return Chat ID
     */
    public String getChatId() {
        return mChatId;
    }
    
    public void setGroupBroadcaster(IGroupChatEventBroadcaster broadcaster){
        mBroadcaster = broadcaster;
    }
    
    /**
     * Get chat ID
     * 
     * @return Chat ID
     */
    public String getChatSessionId() {
        return session.getSessionID();
    }
    
    /**
     * Get remote contact
     * 
     * @return Contact
     */
    public ContactId getRemoteContact() {
        ContactId id = null;
        String contact = null;
        if (logger.isActivated()) {
            logger.info("getRemoteContact entry" );
        }
        try {
        if(session == null){
            session = mImService.getGroupChatSession(mChatId);
        }
        if(session != null) {
            if(!session.isInitiatedByRemote()) 
                return null;
            else 
                contact = session.getRemoteContact();
            
        }
        if(session == null){
            contact = RichMessagingHistory.getInstance().getGroupRemoteContact(mChatId);
            if (logger.isActivated()) {
                logger.info("getRemoteContact  chatId:" + mChatId + "; remotecontact: " + contact);
            }
        }
        contact = PhoneUtils.extractNumberFromUri(contact);
        id = ContactIdUtils.createContactIdFromTrustedData(contact);
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("getRemoteContact  exception" );
            }
            e.printStackTrace();
        }
        return id;
    }
    
    /**
     * Get remote contact
     * 
     * @return Contact
     */
    public String getRemoteContact(int dummy) {
        return PhoneUtils.extractNumberFromUri(session.getRemoteContact());
    }
    
    /**
     * Returns the direction of the group chat (incoming or outgoing)
     * 
     * @return Direction
     */
    public int getDirection() {
        try {
            if (logger.isActivated()) {
                logger.info("getDirection session :" + session);
            }
            if(session == null){
                session = mImService.getGroupChatSession(mChatId);
            }
            if(session == null){
                //return from db
                GroupChatInfo groupInfo = RichMessagingHistory.getInstance().getGroupChatInfo(mChatId);
                int direction = groupInfo.getDirection();
                return direction;
            }
            if (session.isInitiatedByRemote()) {
                return Direction.INCOMING.toInt();
            } else {
                return Direction.OUTGOING.toInt();
            }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("getDirection exception");
            }
            e.printStackTrace();
            return Direction.IRRELEVANT.toInt();
        }
    }
    
    public int getReasonCode(){
        try {
            if (logger.isActivated()) {
                logger.info("getReasonCode exception");
            }
            int reasonCode = RichMessagingHistory.getInstance().getGroupReasonCode(mChatId);
            if (logger.isActivated()) {
                logger.info("getReasonCode : " + reasonCode);
            }
            return reasonCode;
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("getReasonCode exception");
            }
            e.printStackTrace();
            return 0;
        }
    }
    
    /**
     * Returns the state of the group chat
     * 
     * @return State 
     */
    public int getState() {
        int result = GroupChat.State.INITIATING.toInt();
        try {
            if(session == null){
                session = mImService.getGroupChatSession(mChatId);
            }
            if (logger.isActivated()) {
                logger.info("getState session: " + session);
            }
            if(session == null || mState != State.ACTIVE){
                //return from db
                GroupChat.State groupState = RichMessagingHistory.getInstance().getGroupChatState(mChatId);
                if (logger.isActivated()) {
                    logger.info("getState groupState: " + groupState);
                }
                return groupState.toInt();
            }
            SipDialogPath dialogPath = session.getDialogPath();
            if (dialogPath != null) {
                if (dialogPath.isSessionCancelled()) {
                    // Session canceled
                    result = GroupChat.State.ABORTED.toInt();
                } else
                if (dialogPath.isSessionEstablished()) {
                    // Session started
                    result = GroupChat.State.STARTED.toInt();
                } else
                if (dialogPath.isSessionTerminated()) {
                    // Session terminated
                    result = GroupChat.State.REJECTED.toInt();
                } else if (session.isInitiatedByRemote()) {
                    if (session.isSessionAccepted()) {
                        return GroupChat.State.ACCEPTING.toInt();
                    }
                    return GroupChat.State.INVITED.toInt();
                }
    
            }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("getState exception");
            }
            e.printStackTrace();
        }
        if (logger.isActivated()) {
            logger.info("getState:" + result);
        }
        return result;            
    }        
    
    /**
     * Is Store & Forward
     * 
     * @return Boolean
     */
    public boolean isStoreAndForward() {
        return session.isStoreAndForward();
    }
    
    /**
     * Get subject associated to the session
     * 
     * @return String
     */
    public String getSubject() {
        try {
            if (logger.isActivated()) {
                logger.info("getSubject session :" + session);
            }
            if(session == null){
                session = mImService.getGroupChatSession(mChatId);
            }
            if(session == null){
                //return from db
                GroupChatInfo groupInfo = RichMessagingHistory.getInstance().getGroupChatInfo(mChatId);
                String subject = groupInfo.getSubject();
                return subject;
            }
            return session.getSubject();
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("getSubject exception");
            }
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Accepts chat invitation
     */
    public void acceptInvitation() {
        if (logger.isActivated()) {
            logger.info("GCM Accept session invitation");
        }
                
        // Accept invitation
        Thread t = new Thread() {
            public void run() {
                session.acceptSession();
            }
        };
        t.start();
    }
    
    /**
     * Rejects chat invitation
     */ 
    public void rejectInvitation() {
        if (logger.isActivated()) {
            logger.error("GCM Reject session invitation");
        }

        // Update rich messaging history
        RichMessagingHistory.getInstance().updateGroupChatStatus(getChatId(), GroupChat.State.ABORTED.toInt());
        
        // Reject invitation
        Thread t = new Thread() {
            public void run() {
                session.rejectSession(603);
            }
        };
        t.start();
    }

    /**
     * Quits a group chat conversation. The conversation will continue between
     * other participants if there are enough participants.
     */
    public void quitConversation() {
        if (logger.isActivated()) {
            logger.error("quitConversation ");
        }
        
        // Abort the session
        Thread t = new Thread() {
            public void run() {
                session.abortSession(ImsServiceSession.TERMINATION_BY_USER);
            }
        };
        t.start();
    }
    
    /**
     * Returns the list of connected participants. A participant is identified
     * by its MSISDN in national or international format, SIP address, SIP-URI or Tel-URI.
     * 
     * @return List of participants
     */
    public Map<ContactId,Integer> getParticipants() throws RemoteException{
        try {
            if (logger.isActivated()) {
                logger.info("getParticipants entry");
            }
            Map<ContactId, Integer> apiParticipants = new HashMap<>();
            Map<String, ParticipantStatus> participants;
            
            if (session == null) {
                session = mImService.getGroupChatSession(mChatId);
            }
            if(session != null){
                participants = session.getGroupParticipants();
            } else {
                GroupChatInfo groupInfo = RichMessagingHistory.getInstance().getGroupChatInfo(mChatId);
                participants = groupInfo.getParticipantsWithStatus();
            }

            for (Map.Entry<String, ParticipantStatus> participant : participants.entrySet()) {
                if (logger.isActivated()) {
                    logger.info("getParticipants1 : " + participant.getKey());
                }
                ContactId id = ContactIdUtils.createContactIdFromTrustedData(participant.getKey());
                apiParticipants.put(id, participant.getValue().toInt());
            }
            if (logger.isActivated()) {
                logger.info("getParticipants size: " + apiParticipants.size());
            }
            return apiParticipants;

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("ParticipantStatus exception");
            }
            e.printStackTrace();
            throw e;

        }
    }
    
    public long getTimestamp(){
        if (logger.isActivated()) {
            logger.info("getTimestamp entry");
        }
        long timeStamp = 0L;
        try {
            if (session == null) {
                session = mImService.getGroupChatSession(mChatId);
            }
            if(session != null){
                return session.getmTimestamp();
            }
            timeStamp = RichMessagingHistory.getInstance().getGroupTimeStamp(mChatId);
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("getTimestamp exception");
            }
            e.printStackTrace();
        }
        return timeStamp;
    }

    /**
     * Returns the list of connected participants. A participant is identified
     * by its MSISDN in national or international format, SIP address, SIP-URI or Tel-URI.
     * 
     * @return List of participants
     */
    public List<String> getAllParticipants() {
        if (logger.isActivated()) {
            logger.info("GCM Get list of connected participants in the session");
        }
        return session.getParticipants().getList();
    }
    
    public void leave(){
        if (logger.isActivated()) {
            logger.info("leave entry");
        }
        if (isGroupChatAbandoned()) {
            return;
        }
        Runnable run = new Runnable() {
            public void run() {
                try {
                    final GroupChatSession session = mImService.getGroupChatSession(mChatId);
                    if (session == null || !ServerApiUtils.isImsConnected()) {
                        /*
                         * Quitting group chat that is inactive/ not available due to network drop
                         * should reject the next group chat invitation that is received
                         */
                        mMessagingLog.setGroupChatStateAndReasonCode(mChatId,GroupChat.State.ABORTED.toInt(),
                                ReasonCode.ABORTED_BY_USER.toInt());
                        /*mPersistedStorage.setRejectNextGroupChatNextInvitation();
                        mImService
                                .tryToMarkQueuedGroupChatMessagesAndGroupFileTransfersAsFailed(mChatId);*/
                        
                        return;
                    }
                    if (logger.isActivated()) {
                        logger.info("Cancel session");
                    }
                    /* Terminate the session */
                    session.abortSession(ImsServiceSession.TERMINATION_BY_USER);

                } catch (Exception e) {
                    if (logger.isActivated()) {
                        logger.info("leave group exception");
                    }
                }
            }
        };
        Thread t = new Thread(run);
        t.start();
    }
    
    public void openChat(){
        if (logger.isActivated()) {
            logger.info("openChat entry");
        }
        Runnable run = new Runnable() {
            public void run() {
                if (logger.isActivated()) {
                    logger.info("Open a group chat session with chatId " + mChatId);
                }
                try {
                    final GroupChatSession session = mImService.getGroupChatSession(mChatId);
                    if (session == null) {
                        /*
                         * If there is no session ongoing right now then we do not need to open
                         * anything right now so we just return here. A sending of a new message on
                         * this group chat will anyway result in a rejoin attempt if this group chat
                         * has not been left by choice so we do not need to do anything more here
                         * for now.
                         */
                        return;
                    }
                    if (session.getDialogPath().isSessionEstablished()) {
                        return;
                    }
                    int imSessionStartMode = mRcsSettings.getImSessionStartMode();
                    if (!session.isInitiatedByRemote()) {
                        /*
                         * This method needs to accept pending invitation if IM_SESSION_START_MODE
                         * is 0, which is not applicable if session is remote originated so we
                         * return here.
                         */
                        return;
                    }
                    //if (0 == imSessionStartMode) {
                        if (logger.isActivated()) {
                            logger.debug("openChat imSessionStartMode: " + imSessionStartMode);
                        }
                        session.acceptSession();
                    //}
                } catch (Exception e) {
                    if (logger.isActivated()) {
                        logger.info("openChat exception");
                    }
                }
            }
        };
        Thread t = new Thread(run);
        t.start();
        
    }
    
    public boolean isGroupChatAbandoned() {
        if (logger.isActivated()) {
            logger.info("isGroupChatAbandoned entry");
        }
        if (session == null) {
            session = mImService.getGroupChatSession(mChatId);
        }
        if (session != null) {
            /* Group chat is not abandoned if there exists a session */
            if (logger.isActivated()) {
                logger.info("isGroupChatAbandoned false");
            }
            return false;
        }
        int reasonCode = getReasonCode();
        ReasonCode code = ReasonCode.valueOf(reasonCode);
        switch (code) {
            case ABORTED_BY_USER:
            case ABORTED_BY_REMOTE:
            case FAILED_INITIATION:
            case REJECTED_BY_REMOTE:
            case REJECTED_MAX_CHATS:
            case REJECTED_SPAM:
            case REJECTED_BY_TIMEOUT:
            case REJECTED_BY_SYSTEM:
                if (logger.isActivated()) {
                    logger.debug("Group chat with chatId '" + mChatId + "' is " + code);
                }
                return true;
            default:
                break;
        }
        if (logger.isActivated()) {
            logger.info("isGroupChatAbandoned exit false");
        }
        return false;
    }
    
    private boolean isParticipantEligibleToBeInvited(String contactid) {
        if (logger.isActivated()) {
            logger.info("isParticipantEligibleToBeInvited entry contact: " + contactid);
        }
        GroupChatInfo groupInfo = mMessagingLog.getGroupChatInfo(mChatId);
        String participant = contactid.toString();
        Map<String,ParticipantStatus> currentParticipants = groupInfo.getParticipantsWithStatus();
        for (Map.Entry<String, ParticipantStatus> currentParticipant : currentParticipants
                .entrySet()) {
            if (currentParticipant.getKey().equals(participant)) {
                ParticipantStatus status = currentParticipant.getValue();
                switch (status) {
                    case INVITE_QUEUED:
                    case INVITED:
                    case INVITING:
                    case CONNECTED:
                    case DISCONNECTED:
                        if (logger.isActivated()) {
                            logger.debug("Cannot invite participant to group chat with group chat Id '"
                                    + mChatId
                                    + "' as the participant '"
                                    + participant
                                    + "' is ."
                                    + status);
                        }
                        return false;
                    default:
                        break;
                }
            }
        }
        return true;
    }

    private boolean isParticipantCapableToBeInvited(String contactId) {
        if (logger.isActivated()) {
            logger.info("isParticipantCapableToBeInvited entry contact: " + contactId);
        }
        String participant = contactId.toString();
        boolean inviteOnlyFullSF = mRcsSettings.isGroupChatInviteIfFullStoreForwardSupported();
        Capabilities remoteCapabilities = ContactsManager.getInstance().getContactCapabilities(participant);
        if (remoteCapabilities == null) {
            if (logger.isActivated()) {
                logger.debug("Cannot invite participant to group chat with group chat Id '"
                        + mChatId + "' as the capabilities of participant '" + participant
                        + "' are not known.");
            }
            return false;
        }
        if (!remoteCapabilities.isImSessionSupported()) {
            if (logger.isActivated()) {
                logger.debug("Cannot invite participant to group chat with group chat Id '"
                        + mChatId + "' as the participant '" + participant
                        + "' does not have IM capabilities.");
            }
            return false;
        }
        if (inviteOnlyFullSF && !remoteCapabilities.isGroupChatStoreForwardSupported()) {
            if (logger.isActivated()) {
                logger.debug("Cannot invite participant to group chat with group chat Id '"
                        + mChatId + "' as full store and forward is required and the participant '"
                        + participant + "' does not have that feature supported.");
            }
            return false;
        }
        return true;
    }
    
    private boolean isGroupChatCapableOfReceivingParticipantInvitations() {
        if (logger.isActivated()) {
            logger.info("isGroupChatCapableOfReceivingParticipantInvitations entry");
        }
        if (!mRcsSettings.isGroupChatActivated()) {
            if (logger.isActivated()) {
                logger.debug("Cannot add participants to on group chat with group chat Id '"
                        + mChatId + "' as group chat feature has been disabled by the operator.");
            }
            return false;
        }
        if (isGroupChatAbandoned()) {
            if (logger.isActivated()) {
                logger.debug("Cannot invite participants to group chat with group chat Id '"
                        + mChatId + "'");
            }
            return false;
        }
        return true;
    }
    
    private boolean isGroupChatRejoinable() {
        if (logger.isActivated()) {
            logger.info("isGroupChatRejoinable entry");
        }
        GroupChatInfo groupChat = mMessagingLog.getGroupChatInfo(mChatId);
        if (groupChat == null) {
            if (logger.isActivated()) {
                logger.debug("Group chat with group chat Id '" + mChatId
                        + "' is not rejoinable as the group chat does not exist in DB.");
            }
            return false;
        }
        if (groupChat.getRejoinId() == null) {
            if (logger.isActivated()) {
                logger.debug("Group chat with group chat Id '" + mChatId
                        + "' is not rejoinable as there is no ongoing session with "
                        + "corresponding chatId and there exists no rejoinId to "
                        + "rejoin the group chat.");
            }
            return false;
        }
        return true;
    }
    
    /**
     * Returns true if it is possible to send messages in the group chat right now, else returns
     * false.
     * 
     * @return boolean
     * @throws RemoteException
     */
    @Override
    public boolean isAllowedToSendMessage() throws RemoteException {
        try {
            if (logger.isActivated()) {
                logger.info("isAllowedToSendMessage entry");
            }
            if (!mRcsSettings.isGroupChatActivated()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot send message on group chat with group chat Id '"
                            + mChatId + "' as group chat feature is not supported.");
                }
                return false;
            }
            if (isGroupChatAbandoned()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot send message on group chat with group chat Id '"
                            + mChatId + "'");
                }
                return false;
            }
            if (!mRcsSettings.getMyCapabilities().isImSessionSupported()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot send message on group chat with group chat Id '"
                            + mChatId + "' as IM capabilities are not supported for self.");
                }
                return false;
            }
            GroupChatSession session = mImService.getGroupChatSession(mChatId);
            if (session == null) {
                if (!isGroupChatRejoinable()) {
                    return false;
                }
            }
            if (logger.isActivated()) {
                logger.info("isAllowedToSendMessage exit");
            }
            return true;

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("isAllowedToSendMessage exception");
            }
            e.printStackTrace();
            throw e;

        }
    }
    
    private boolean isAllowedToInviteAdditionalParticipants(int additionalParticipants)
            throws RemoteException {
        if (logger.isActivated()) {
            logger.info("isAllowedToInviteAdditionalParticipants size: " + additionalParticipants);
        }
        int nrOfParticipants = getParticipants().size() + additionalParticipants;
        int maxNrOfAllowedParticipants = mRcsSettings.getMaxChatParticipants();
        return nrOfParticipants < maxNrOfAllowedParticipants;
    }
    
    public boolean isGroupActive(){
        if (logger.isActivated()) {
            logger.info("isGroupActive mState: " + mState);
        }
        return mState == State.ACTIVE;
    }
    
    /**
     * Returns true if it is possible to invite additional participants to the group chat right now,
     * else returns false.
     * 
     * @return boolean
     * @throws RemoteException
     */
    @Override
    public boolean isAllowedToInviteParticipants() throws RemoteException {
        try {
            if (logger.isActivated()) {
                logger.info("isAllowedToInviteParticipants ");
            }
            if (!isGroupChatCapableOfReceivingParticipantInvitations()) {
                return false;
            }
            if (!isAllowedToInviteAdditionalParticipants(1)) {
                if (logger.isActivated()) {
                    logger.debug("Cannot invite participants to group chat with group chat Id '"
                            + mChatId + "' as max number of participants has been reached already.");
                }
                return false;
            }
            return true;

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("isAllowedToInviteParticipants exception");
            }
            e.printStackTrace();
            throw e;

        }
    }

    /**
     * Returns true if it is possible to invite the specified participants to the group chat right
     * now, else returns false.
     * 
     * @param participant ContactId
     * @return boolean
     * @throws RemoteException
     */
    @Override
    public boolean isAllowedToInviteParticipant(ContactId participant) throws RemoteException {
        if (logger.isActivated()) {
            logger.info("isAllowedToInviteParticipant participant: " + participant);
        }
        if (participant == null) {
            return false;
        }
        if (!isAllowedToInviteParticipants()) {
            return false;
        }
        try {
            return isParticipantEligibleToBeInvited(participant.toString())
                    && isParticipantCapableToBeInvited(participant.toString());

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("isAllowedToInviteParticipant exception");
            }
            e.printStackTrace();
            throw e;

        }
    }
    
    public boolean isAllowedToLeave(){
        try {
            if (logger.isActivated()) {
                logger.info("isAllowedToLeave ");
            }
            if (isGroupChatAbandoned()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot leave group chat with group chat Id '" +  mChatId);
                }
                return false;
            }
            return true;

        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("isAllowedToLeave exception");
            }
            e.printStackTrace();
            throw e;

        }
    }

    /**
     * Returns the max number of participants for a group chat from the group
     * chat info subscription (this value overrides the provisioning parameter)
     * 
     * @return Number
     */
    public int getMaxParticipants() {
        if (logger.isActivated()) {
            logger.info("GCM Get max number of participants in the session");
        }
        if(session == null){
            mRcsSettings.getMaxChatParticipants();
        }
        return session.getMaxParticipants();
    }

    /**
     * Adds participants to a group chat
     * 
     * @param participants List of participants
     */
    public void addParticipants(final List<String> participants) {
        if (logger.isActivated()) {
            logger.info("GCM Add " + participants.size() + " participants to the session");
        }

        int max = session.getMaxParticipants()-1;
        int connected = session.getConnectedParticipants().getList().size(); 
        if (connected < max) {
            // Add a list of participants to the session
            Thread t = new Thread() {
                public void run() {
                    session.addParticipants(participants);
                }
            };
            t.start();
        } else {
            // Max participants achieved
            //Toast.makeText(AndroidFactory.getApplicationContext(), "Too many participants", Toast.LENGTH_LONG).show();
            //handleAddParticipantFailed("Maximum number of participants reached");
            Thread t = new Thread() {
                public void run() {
                    for(String participant : participants){
                        onAddParticipantFailed(participant,"Maximum number of participants reached");
                    }
                    
                }
            };
            t.start();
        }
    }
    
    /**
     * Invite additional participants to this group chat.
     * 
     * @param participants Set of participants
     * @throws RemoteException
     */
    @Override
    public void inviteParticipants(final List<ContactId> contacts) throws RemoteException {
        if (logger.isActivated()) {
            logger.info("inviteParticipants contacts: " + contacts.size());
        }
        if (contacts == null || contacts.isEmpty()) {
            return;
            //throw new Exception("participants list must not be null or empty!");
        }
        final List<String> participants = PhoneUtils.generateContactsStringList(contacts);
        if (!isGroupChatCapableOfReceivingParticipantInvitations()) {
            return;
            //throw new Exception("Not capable of receiving participant invitations!");
        }
        try {
            for (String participant : participants) {
                if (!isParticipantEligibleToBeInvited(participant)) {
                    throw new ServerApiPermissionDeniedException(
                            "Participant not eligible to be invited!");
                }
            }
            Runnable run = new Runnable() {
                public void run() {
                    GroupChatSession session = mImService.getGroupChatSession(mChatId);
                    try {
                        boolean mediaEstablished = (session != null && session.isMediaEstablished());
                        if (mediaEstablished) {
                            addParticipants(participants);
                            return;
                        }
                        Map<String,ParticipantStatus> participantsToStore = mMessagingLog.getParticipants(mChatId);
                        for (String contact : participants) {
                            participantsToStore.put(contact, ParticipantStatus.INVITE_QUEUED);
                        }
                        if (session != null) {
                            session.updateParticipants(participantsToStore);
                        } else {
                            mMessagingLog.setGroupChatParticipants(mChatId, participantsToStore);
                        }
                        if (session == null) {
                            if (isGroupChatRejoinable() && ServerApiUtils.isImsConnected()) {
                                rejoinGroup();
                            }
                        }
                    } catch (Exception e) {
                        if (session != null) {
                            session.handleError(new ChatError(ChatError.SEND_RESPONSE_FAILED));
                        } else {
                            logger.info("inviteParticipants exception0");
                        }
                    }
                }
            };
           Thread t = new Thread(run);
           t.start();
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("inviteParticipants exception");
            }
            e.printStackTrace();
        }
    }
    
    /**
     * Sends a text message to the group
     * 
     * @param text Message
     * @return Message ID
     */
    public IChatMessage sendMessage(final String text) {
        if (logger.isActivated()) {
            logger.info("GCM sendMessage:" + text);
        }
        // Generate a message Id
        final String msgId = ChatUtils.generateMessageId();
        boolean isImdnActivated = false;
        try {
            isImdnActivated = Core.getInstance().getImService().getImdnManager().isImdnActivated();
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.debug("IMDN manager is null");
            }
        }
        String displayName = RcsSettings.getInstance().getJoynUserAlias();
        InstantMessage msgToSend = new InstantMessage(msgId, "Default_Group", text, isImdnActivated ,displayName);
        
        if (RcsSettings.getInstance().supportOP01() || RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()) {
            // Update rich messaging history
            if (!RichMessagingHistory.getInstance().isOne2OneMessageExists(msgId)) {
                if (logger.isActivated()) {
                    logger.info("CPMS Group sendMessage Add in DB Msgid:" + msgId );
                }
                
                RichMessagingHistory.getInstance().addGroupChatMessage(mChatId, msgToSend,
                        Direction.OUTGOING.toInt());
            }
        }
        
        if(session == null){
            if (logger.isActivated()) {
                logger.info("sendMessage session is  null");
            }
            session = mImService.getGroupChatSession(mChatId);
            if (logger.isActivated()) {
                logger.info("session is: " + session);
            }
            sendAfterRejoin(msgToSend);
        } else {
            // Send text message
            Thread t = new Thread() {
                public void run() {
                    if(session != null){
                        session.sendTextMessage(msgId, text);
                    }else {
                        if (logger.isActivated()) {
                            logger.info("sendMessage cant send Message session is  null");
                        }
                    }
                    
                }
            };
            t.start();
        }        
        
        if (logger.isActivated()) {
            logger.info("sendMessage exit");
        }
        ChatMessagePersistedStorageAccessor persistedStorage = new ChatMessagePersistedStorageAccessor(
                mMessagingLog, msgId, msgToSend.getRemote(), text,
                msgToSend.getMimeType(), mChatId, Direction.OUTGOING);
        IChatMessage chatMsg = new ChatMessageImpl(persistedStorage);
        return chatMsg;
    }
    
    /**
     * Sends a text message to the group
     * 
     * @param text Message
     * @return Message ID
     */
    public void sendInstantMessage(final InstantMessage msg) {
        if (logger.isActivated()) {
            logger.info("sendInstantMessage:" + msg.getTextMessage() + " id: " + msg.getMessageId());
        }
        //Update msg status in DB to sending
        
        if(session == null){
            session = mImService.getGroupChatSession(mChatId);
            if(session == null){
                sendAfterRejoin(msg);
            }
        }

        // Send text message
        Thread t = new Thread() {
            public void run() {
                session.sendTextMessage(msg.getMessageId(), msg.getTextMessage());
            }
        };
        t.start();
    }
    
    public void rejoinGroup(){
        try{
            if (logger.isActivated()) {
                logger.info("rejoinGroup rejoinId: " + mChatId);
            }
            mState = State.REJOINING;
            GroupChatSession chatSession = (GroupChatSession)mImService.rejoinGroupChatSession(mChatId);
            this.session = chatSession;
            chatSession.addListener(this);
            mChatService.addGroupChatSession(this);
            
            // Start the session
            if (logger.isActivated()) {
                logger.info("rejoinGroup Start session " + mChatId);
            }
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();
            
        } catch (Exception e){
            e.printStackTrace();
        }
    }
    
    public void rejoinGroup(Object lock){
        try {
            if (logger.isActivated()) {
                logger.info("rejoinGroup rejoinId: " + mChatId);
            }
            mState = State.REJOINING;
            rejoinLock = lock;
            GroupChatSession chatSession = (GroupChatSession)mImService.rejoinGroupChatSession(mChatId);
            this.session = chatSession;
            chatSession.addListener(this);
            mChatService.addGroupChatSession(this);
            
            // Start the session
            if (logger.isActivated()) {
                logger.info("rejoinGroup Start session " + mChatId);
            }
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();
            
        } catch (Exception e){
            e.printStackTrace();
        }
    }
    
    public void restartGroup(){
        try {
            if (logger.isActivated()) {
                logger.info("restartGroup rejoinId: " + mChatId);
            }
            mState = State.RESTARTING;
            GroupChatSession chatSession = (GroupChatSession)mImService.restartGroupChatSession(mChatId);
            this.session = chatSession;
            chatSession.addListener(this);
            mChatService.addGroupChatSession(this);
            
         // Start the session
            if (logger.isActivated()) {
                logger.info("restartGroup Start session " + mChatId);
            }
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();
            
        } catch (Exception e){
            e.printStackTrace();
        }
    }
    
    public void sendAfterRejoin(InstantMessage msg){
        try {
            if (logger.isActivated()) {
                logger.info("sendAfterRejoin:" + msg.getMessageId() + ", chatId: " + mChatId);
            }
            GroupChatInfo groupInfo = RichMessagingHistory.getInstance().getGroupChatInfo(mChatId);
            if(groupInfo == null) return;
            int status = groupInfo.getStatus();
                enqueMessages(msg);
            if(mState == State.MANUAL_REJOIN){
                rejoinGroup();
            } else if(mState == State.REJOINING || mState == State.RESTARTING){
                //Do nothing
            } else if(mState == State.MANUAL_RESTART){
                restartGroup();
            }
        } catch(Exception e){
            if (logger.isActivated()) {
                logger.info("sendAfterRejoin exception:" + msg.getMessageId());
            }
            e.printStackTrace();
        }
    }
    
    public void enqueMessages(InstantMessage msg){
        if (logger.isActivated()) {
            logger.info("enqueMessages:" + msg.getMessageId() + ", chatId: " + mChatId);
        }
        mPendingMessage.add(msg);
    }
    
    /**
     * Sends a text message to the group
     *
     * @param text Message
     * @return Message ID
     */
    public IChatMessage sendMessageEx(final String text, final int msgType) {
        if (logger.isActivated()) {
            logger.info("GCM sendMessage:" + text);
        }
        // Generate a message Id
        final String msgId = ChatUtils.generateMessageId();

        if (RcsSettings.getInstance().supportOP01()) {
            // Update rich messaging history
            if (!RichMessagingHistory.getInstance().isOne2OneMessageExists(msgId)) {
                if (logger.isActivated()) {
                    logger.info("CPMS Group sendMessage Add in DB Msgid:" + msgId );
                }
                InstantMessage msg = new InstantMessage(msgId, getRemoteContact(0), text, session.getImdnManager().isImdnActivated() ,null);
                msg.setMessageType(msgType);
                RichMessagingHistory.getInstance().addGroupChatMessage(
                        session.getContributionID(), msg, Direction.OUTGOING.toInt());
            }
        }

        // Send text message
        Thread t = new Thread() {
            public void run() {
                session.sendMessageEx(msgId, text, msgType);
            }
        };
        t.start();

        IChatMessage msg = null;
        return msg;
    }
    
    /**
     * @param msgId message Id of message
     * @throws JoynServiceException
     */
    public  void resendMessage(final String msgId)
    {
        final String message = RichMessagingHistory.getInstance().getMessageText(msgId);
        if (logger.isActivated()) {
            logger.info("GCM resendMessage:" + message + " msgId:" + msgId);
        }
        
        if(RcsSettings.getInstance().supportOP01()) {
            // Update rich messaging history
            InstantMessage msg = new InstantMessage(msgId, getRemoteContact(0), message, session.getImdnManager().isImdnActivated() ,null);
            //RichMessagingHistory.getInstance().addGroupChatMessage(session.getContributionID(), msg,
                //    Direction.OUTGOING.toInt());
        }
        
        // Send text message
        Thread t = new Thread() {
            public void run() {
                session.sendTextMessage(msgId, message);
            }
        };
        t.start();
        
        //return getMessageState(msgId);
    }

     /**    
     * Returns the state of the group chat message
     * 
     * @return State of the message
     * @see GroupChat.MessageState
     */
    public int getMessageState(String messageId) {
        int messageStatus = RichMessagingHistory.getInstance().getMessageStatus(messageId);
         if (logger.isActivated()) {
            logger.info("GCM getMessageState: msgId" + messageId + " StackStatus:" + messageStatus);
         }
        /*switch(messageStatus){
            case ChatLog.Message.Content.Status.SENDING.toInt():
                return GroupChat.MessageState.SENDING;
                
            case ChatLog.Message.Content.Status.SENT.toInt():
                return GroupChat.MessageState.SENT;
                
            case ChatLog.Message.Content.Status.UNREAD_REPORT.toInt():
            case ChatLog.Message.Content.Status.UNREAD.toInt():
            case ChatLog.Message.Content.Status.DISPLAYED.toInt():
                return GroupChat.MessageState.DELIVERED;
                
            case ChatLog.Message.Content.Status.FAILED.toInt():
                return GroupChat.MessageState.FAILED;
                
            default:
                return GroupChat.MessageState.FAILED;*/
        //}
         return 0;
    }
    
    
    /**
     * Sends a geoloc message
     * 
     * @param geoloc Geoloc
     * @return Unique message ID or null in case of error
     */
    public IChatMessage sendMessage2(Geoloc geoloc) {
        // Generate a message Id
        final String msgId = ChatUtils.generateMessageId();

        // Send geoloc message
        final GeolocPush geolocPush = new GeolocPush(geoloc.getLabel(),
                geoloc.getLatitude(), geoloc.getLongitude(),
                geoloc.getExpiration(), geoloc.getAccuracy());
        Thread t = new Thread() {
            public void run() {
                session.sendGeolocMessage(msgId, geolocPush);
            }
        };
        t.start();
        
        IChatMessage msg = null;
        return msg;
    }    

    /**
     * Set the new chairman(chairman privilege).
     * 
     * @param newChairman new chairman of the group, should be a group member 
     * @throws JoynServiceException
     */
    public void transferChairman(final String newChairman) {
        Thread t = new Thread() {
            public void run() {
                session.transferChairman(newChairman);
            }
        };
        t.start();
    }
    
    /**
     * modify subject of group(chairman privilege).
     * 
     * @param newSubject new subject string 
     * @throws JoynServiceException
     */
    public void modifySubject(final String newSubject) {
        Thread t = new Thread() {
            public void run() {
                session.modifySubject(newSubject);
            }
        };
        t.start();
    }
    
    /**
     * modify nickname of participant.
     * 
     * @param contact contact of the participant
     * @param newNickname new nick name of participant 
     * @throws JoynServiceException
     */
    public void modifyMyNickName(final String newNickname) {

        final String contact = ImsModule.IMS_USER_PROFILE.getUsername();
        Thread t = new Thread() {
            public void run() {
                session.modifyMyNickName(contact, newNickname);
            }
        };
        t.start();
    }
    
    public void removeContacts(final List<String> contacts){
        Thread t = new Thread() {
        public void run() {
            for(String participant : contacts) {
                session.removeParticipants(participant);
            }
        }
        };
        t.start();
    }
    
    /**
     * remove set of participants(chairman privilege).
     * 
     * @param participants list of participants to be removed 
     * @throws JoynServiceException
     */
    public void removeParticipants(final List<ContactId> contacts) {
        if (logger.isActivated()) {
            logger.info("removeParticipants:" + contacts.size());
        }
        if (contacts == null || contacts.isEmpty()) {
            return;
            //throw new Exception("participants list must not be null or empty!");
        }
        final List<String> participants = PhoneUtils.generateContactsStringList(contacts);
        try {
            Runnable run = new Runnable() {
                public void run() {
                    GroupChatSession session = mImService.getGroupChatSession(mChatId);
                    try {
                        boolean mediaEstablished = (session != null && session.isMediaEstablished());
                        if (mediaEstablished) {
                            removeContacts(participants);
                            return;
                        }
                        Map<String,ParticipantStatus> participantsToStore = mMessagingLog.getParticipants(mChatId);
                        for (String contact : participants) {
                            participantsToStore.put(contact, ParticipantStatus.DISCONNECTED);
                        }
                        if (session != null) {
                            session.updateParticipants(participantsToStore);
                        } else {
                            mMessagingLog.setGroupChatParticipants(mChatId, participantsToStore);
                        }
                        if (session == null) {
                            if (isGroupChatRejoinable() && ServerApiUtils.isImsConnected()) {
                                rejoinGroup();
                                removeContacts(participants);
                                return;
                            }
                        }
                    } catch (Exception e) {
                        if (session != null) {
                            session.handleError(new ChatError(ChatError.SEND_RESPONSE_FAILED));
                        } else {
                            logger.info("removeParticipants exception0");
                        }
                    }
                }
            };
           Thread t = new Thread(run);
           t.start();
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.info("removeParticipants exception");
            }
            e.printStackTrace();
        }
    }
    
    /**
     * chairman abort(leave) the group, Group session will abort
     *
     * @throws JoynServiceException
     */
    public void abortConversation() {
         if (logger.isActivated()) {
         logger.error("abortConversation");
        }
        Thread t = new Thread() {
            public void run() {
                session.abortGroupSession(ImsServiceSession.TERMINATION_BY_USER);
            }
        };
        t.start();
    }
    
    
    /**
     * Block messages in group, stack will not notify application about
     * any received message in this group
     *
     * @param flag true means block the message, false means unblock it
     * @throws JoynServiceException
     */
    public void blockMessages(final boolean flag) {
         if (logger.isActivated()) {
                logger.info("GCM blockMessages flag:" + flag);
        }
        Thread t = new Thread() {
            public void run() {
                session.blockMessages(flag);
            }
        };
        t.start();
    }
    
    /**
     * If myself chairman of the group
     * 
     * @return true/false
     * @throws JoynServiceException
     */
    public boolean isMeChairman() {
        if (logger.isActivated()) {
            logger.info("GCM isMeChairman entry");
        }
        boolean flag = false;
        String chairman = RichMessagingHistory.getInstance().getGroupChairman(getChatId());
        if (logger.isActivated()) {
            logger.info("GCM Current chairman of group "+ chairman);
        }
        String me = ImsModule.IMS_USER_PROFILE.getUsername();
        if (logger.isActivated()) {
            logger.info("GCM me is "+ me);
        }
        //TODO change 'contains' to 'equals' later after verify
        if(me.contains(chairman)){
            return true;
        } else {
            return false;
        }
    }

    /**
     * Sends a is-composing event. The status is set to true when typing
     * a message, else it is set to false.
     * 
     * @param status Is-composing status
     */
    public void setComposingStatus(final boolean status) {
        boolean isComposing = RcsSettings.getInstance().isComposingSupported();
        if (logger.isActivated()) {
            logger.debug("sendIsComposingEvent isComposingSupported " + isComposing);
        }
        if(isComposing){
            Thread t = new Thread() {
                public void run() {
                    if(session != null){
                        session.sendIsComposingStatus(status);
                    }
                }
            };
            t.start();
        }
    }
    
    /**
     * Sends a displayed delivery report for a given message ID
     * 
     * @param msgId Message ID
     */
    public void sendDisplayedDeliveryReport(final String msgId) {
        try {
            final String remote = mMessagingLog.getMessageContact(msgId);
            if (logger.isActivated()) {
                logger.debug("GCM Set displayed delivery report for " + msgId + ", remote: " + remote);
            }
            // Send MSRP delivery status
            Thread t = new Thread() {
                public void run() {
                    session.sendMsrpMessageDeliveryStatus(remote, msgId, ImdnDocument.DELIVERY_STATUS_DISPLAYED);
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("GCM Could not send MSRP delivery status",e);
            }
        }
    }    
    
    /**
     * Adds a listener on chat events
     * 
     * @param listener Group chat event listener 
     */
    public void addEventListener(IGroupChatListener listener) {
        if (logger.isActivated()) {
            logger.info("GCM Add an event listener");
        }

        synchronized(lock) {
            listeners.register(listener);
        }
    }
    
    /**
     * Removes a listener on chat events
     * 
     * @param listener Group chat event listener 
     */
    public void removeEventListener(IGroupChatListener listener) {
        if (logger.isActivated()) {
            logger.info("GCM Remove an event listener");
        }

        synchronized(lock) {
            listeners.unregister(listener);
        }
    }
    
    /*------------------------------- SESSION EVENTS ----------------------------------*/

    private void setStateAndReasonCode(GroupChat.State state, ReasonCode reasonCode) {
        if (logger.isActivated()) {
            logger.info("setStateAndReasonCode state : " + state.toInt() + ", reason: " + reasonCode.toInt());
        }
        mMessagingLog.setGroupChatStateAndReasonCode(getChatId(), state.toInt(),reasonCode.toInt());
        mBroadcaster.broadcastStateChanged(mChatId, state, reasonCode);
    }
    
    /**
     * Session is started
     */
    public void handleSessionStarted() {
        synchronized (lock) {
            if (logger.isActivated()) {
                logger.info("GCM Session started" +" ,mstate: " + mState + ", rejoinlock: " + rejoinLock);
            }

            // Update rich messaging history
            if (RichMessagingHistory.getInstance().getGroupChatInfo(getChatId()) != null) {
                RichMessagingHistory.getInstance().updateGroupChatStatus(
                    getChatId(), GroupChat.State.STARTED.toInt());
                RichMessagingHistory.getInstance().updateGroupChatRejoinId(
                    getChatId(), session.getImSessionIdentity());
            }
            mState = State.ACTIVE;

            if (!(RichMessagingHistory.getInstance().isGroupMemberExist(
                    getChatId(), ImsModule.IMS_USER_PROFILE.getUsername()))) {
                RichMessagingHistory.getInstance().addGroupMember(
                        session.getContributionID(),
                        RcsSettings.getInstance().getJoynUserAlias(),
                        ImsModule.IMS_USER_PROFILE.getUsername(),
                        null);
            }
            
            session = mImService.getGroupChatSession(mChatId);
            mChatService.broadcastGroupChatStateChange(mChatId, GroupChat.State.STARTED,
                    ReasonCode.UNSPECIFIED);
            
            try {
                if(rejoinLock != null) {
                    synchronized (rejoinLock) {
                        if (logger.isActivated()) {
                            logger.info("handleSessionStarted notify for FT start ");
                        }
                        rejoinLock.notify();
                        rejoinLock = null;
                    }
                }
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.info("handleSessionStarted notify exception");
                }
                e.printStackTrace();
            }
            
            if (logger.isActivated()) {
                logger.info("handleSessionStarted size: " + mPendingMessage.size());
            }
            while(mPendingMessage.size() > 0) {
                final InstantMessage msg = mPendingMessage.get(0);
             // Send text message
                Thread t = new Thread() {
                    public void run() {
                        sendInstantMessage(msg);
                    }
                };
                t.start();
                mPendingMessage.remove(0);
            }

            // send pending ft
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
                logger.info("handleSessionAborted reason: " + reason +" ,mstate: " + mState);
            }
    
            // Update rich messaging history
            /*if (reason == ImsServiceSession.TERMINATION_BY_USER) {
                RichMessagingHistory.getInstance().updateGroupChatStatus(getChatId(), GroupChat.State.CLOSED_BY_USER.toInt());
            } else {
                if (session.getDialogPath().isSessionCancelled()) {
                    RichMessagingHistory.getInstance().updateGroupChatStatus(getChatId(), GroupChat.State.ABORTED.toInt());
                } else {
                    RichMessagingHistory.getInstance().updateGroupChatStatus(getChatId(), GroupChat.State.REJECTED.toInt());
                }
            }*/
            // Remove session from the list
            ChatServiceImpl.removeGroupChatSession(getChatId());
            
            switch (reason) {
            case ImsServiceSession.TERMINATION_BY_CONNECTION_LOST:
            case ImsServiceSession.TERMINATION_BY_SYSTEM:
                /*
                 * This error is caused because of a network drop so the group chat is not set
                 * to ABORTED state in this case as it will try to be auto-rejoined when IMS
                 * connection is regained
                 */
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.REJECTED_BY_SYSTEM);
                break;
            case ImsServiceSession.TERMINATION_BY_USER:
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.ABORTED_BY_USER);
               // mImService
                //        .tryToMarkQueuedGroupChatMessagesAndGroupFileTransfersAsFailed(mChatId);
                break;
            case ImsServiceSession.TERMINATION_BY_REMOTE:
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.ABORTED_BY_REMOTE);
               // mImService
                 //       .tryToMarkQueuedGroupChatMessagesAndGroupFileTransfersAsFailed(mChatId);
                break;
            case ImsServiceSession.TERMINATION_BY_TIMEOUT:
            case ImsServiceSession.TERMINATION_BY_INACTIVITY:
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.ABORTED_BY_INACTIVITY);
                break;
            default:
                if (logger.isActivated()) {
                    logger.error("handleSessionAborted default");
                }
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.ABORTED_BY_INACTIVITY);
            }
            if(mState == State.ACTIVE){
                mState = State.MANUAL_REJOIN;
            } else if(mState == State.MANUAL_REJOIN || mState == State.REJOINING){
                mState = State.MANUAL_RESTART;
            }
            this.session = null;
        }
    }
    
    /**
     * Group SIP bye received for group session
     * 
     * @param cause cause in 'cause' parameter
     */
    public void handleSessionTerminatedByRemote(String cause, String text){
        
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("GCM handleSessionTerminatedByGroupRemote Session terminated cause=" + cause + ",text ="+ text + " ,mstate: " + mState);
            }
    
            // Update rich messaging history
            if (session.getDialogPath().isSessionCancelled()) {
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.ABORTED_BY_REMOTE);
            } else if(cause.contains("480")) {
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.REJECTED_BY_SECONDARY_DEVICE);
            } else {
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.REJECTED_BY_REMOTE);
            }
            int causeParameter = 0;
            if(cause !=null){
                causeParameter = Integer.valueOf(cause);
            }
            /*// Notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("GCM handleSessionTerminatedByGroupRemote N: " + N);
            }
            if(causeParameter == 200 && text.equalsIgnoreCase("booted")){
                //text 'booted' means participant is kicked out
                //text 'Call completed' means session is aborted by chairman
                //cause is 200 in both case
                
                causeParameter = 201;
                if (logger.isActivated()) {
                    logger.info("GCM participant is kicked out " + cause);
                }
            }
            for (int i=0; i < N; i++) {
                try {
                    switch(causeParameter){
                        case 200:
                            //session aborted by chairman
                            //listeners.getBroadcastItem(i).onSessionAbortedbyChairman();
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
                            break;
                        case 201:
                           //listeners.getBroadcastItem(i).onReportMeKickedOut(session.getChairman());
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
                           break;
                        case 410:
                            //cause 410 means that group chat is dissolved
                           //listeners.getBroadcastItem(i).onGroupChatDissolved();
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
                           break;
                        default:
                            //listeners.getBroadcastItem(i).onSessionAborted();
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
                    }
                    
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("GCM Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();*/
            
            // Remove session from the list
            ChatServiceImpl.removeGroupChatSession(getChatId());
            if(mState == State.ACTIVE){
                mState = State.MANUAL_REJOIN;
            } else if(mState == State.MANUAL_REJOIN || mState == State.REJOINING){
                mState = State.MANUAL_RESTART;
            }
            this.session = null;
        }
        
    }
    
    
    /**
     * handle Quit Conversation by User(not chairman)
     * 
     * @param code status Code
     */
    public void handleQuitConversationResult(int code) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("GCM handleQuitConversationResult code =" + code);
            }
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onQuitConversationResult(ReasonCode.SUCCESSFUL, 200);
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
    /**
     * Abort Conversation successful, only received by chairman after leave group
     * (chairman privilege)
     * @param code reason code
     */
    public void handleAbortConversationResult(int reason, int code) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Session aborted group reason=" + reason + ",code =" + code);
            }
            if(code == 200){
            // Update rich messaging history
            if (reason == ImsServiceSession.TERMINATION_BY_USER) {
                RichMessagingHistory.getInstance().updateGroupChatStatus(getChatId(), GroupChat.State.CLOSED_BY_USER.toInt());
            } else {
                if (session.getDialogPath().isSessionCancelled()) {
                    RichMessagingHistory.getInstance().updateGroupChatStatus(getChatId(), GroupChat.State.ABORTED.toInt());
                } else {
                    RichMessagingHistory.getInstance().updateGroupChatStatus(getChatId(), GroupChat.State.REJECTED.toInt());
                }
            }
            
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("GCM handleAbortConversationResult N: " + N);
            }
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onAbortConversationResult(ReasonCode.SUCCESSFUL, code);
                     //   listeners.getBroadcastItem(i).onSessionAborted();
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
            
            // Remove session from the list
            ChatServiceImpl.removeGroupChatSession(getChatId());
            } else {
                // Notify event listeners
                final int N = listeners.beginBroadcast();
                for (int i=0; i < N; i++) {
                    try {
                       // listeners.getBroadcastItem(i).onAbortConversationResult(ReasonCode.INTERNAL_ERROR, code);
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
    
    /**
     * Transfer chairman Successful
     * 
     * @param subject subject
     */
    public void handleTransferChairmanSuccessful(String newChairman) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleTransferChairmanSuccessful " + newChairman);
            }
            
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onChairmanChanged(newChairman);
                   // listeners.getBroadcastItem(i).onSetChairmanResult(GroupChat.ReasonCode.SUCCESSFUL, 200);
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
    
    /**
     * Transfer chairman Failed
     * 
     * @param statusCode statusCode
     */
    public void handleTransferChairmanFailed(int statusCode) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleTransferChairmanFailed " + statusCode);
            }
              
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //need switch case of statusCode
                    //listeners.getBroadcastItem(i).onSetChairmanResult(GroupChat.ReasonCode.INTERNAL_ERROR, statusCode);
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
    
    
    /**
     * Transfer chairman by other participant
     * 
     * @param newChairman new chairman
     */
    public void handleTransferChairmanByRemote(String newChairman){
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleTransferChairmanByRemote " + newChairman);
            }
            session.setChairman(newChairman);
            // Update chairman in DB
            RichMessagingHistory.getInstance().updateGroupChairman(getChatId(), newChairman);

            
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    
                    //listeners.getBroadcastItem(i).onChairmanChanged(newChairman);
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
    
    /**
     * Remove Participant Successful
     * 
     * @param subject subject
     */
    public void handleRemoveParticipantSuccessful(String removedParticipant) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.error("handleRemoveParticipantSuccessful " + removedParticipant);
            }

            //remove participant from session
            session.removeParticipantFromSession(removedParticipant);
            //remove participant from database
            RichMessagingHistory.getInstance().removeGroupMember(getChatId(),removedParticipant);
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onRemoveParticipantResult(GroupChat.ReasonCode.SUCCESSFUL, 200, removedParticipant);
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
    
    /**
     * Remove Participant Failed
     * 
     * @param statusCode statusCode
     */
    public void handleRemoveParticipantFailed(int statusCode) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleRemoveParticipantFailed " + statusCode);
            }
              
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //need switch case of statusCode
                    //listeners.getBroadcastItem(i).onRemoveParticipantResult(GroupChat.ReasonCode.INTERNAL_ERROR, statusCode, null);
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
    
    /**
     * Modify Subject Successful
     * 
     * @param subject subject
     */
    public void handleModifySubjectSuccessful(String subject) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleModifySubjectSuccessful " + subject);
            }
            session.setSubject(subject);
            // Update subject in DB
            RichMessagingHistory.getInstance().updateGroupChatSubject(getChatId(), subject);

            
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onSubjectChanged(subject);
                    //listeners.getBroadcastItem(i).onModifySubjectResult(GroupChat.ReasonCode.SUCCESSFUL, 200);
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
    
    /**
     * Modify Subject Failed
     * 
     * @param statusCode statusCode
     */
    public void handleModifySubjectFailed(int statusCode) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleModifySubjectFailed " + statusCode);
            }
              
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //need switch case of statusCode
                    //listeners.getBroadcastItem(i).onModifySubjectResult(GroupChat.ReasonCode.INTERNAL_ERROR, statusCode);
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
    
    /**
     * Modify Subject BY remote
     * 
     * @param subject subject
     */
    public void handleModifySubjectByRemote(String subject) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleModifySubjectByRemote " + subject);
            }
            session.setSubject(subject);
            // Update subject in DB
            RichMessagingHistory.getInstance().updateGroupChatSubject(getChatId(), subject);

            
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onSubjectChanged(subject);
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
    
    /**
     * Modify Subject Successful
     * 
     * @param subject subject
     */
    public void handleModifyNicknameSuccessful(String contact, String newNickName) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleModifyNicknameSuccessful " + contact + "nickname:"+ newNickName);
            }
            //session.setSubject(subject);
            // Update subject in DB
            //RichMessagingHistory.getInstance().updateGroupChatSubject(getChatId(), subject);
            RichMessagingHistory.getInstance().updateGroupMemberName(session.getContributionID(),contact ,newNickName);
            
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onSubjectChanged(subject);
                    //listeners.getBroadcastItem(i).onModifyNickNameResult(GroupChat.ReasonCode.SUCCESSFUL, 200);
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
    
    /**
     * Modify Subject Failed
     * 
     * @param statusCode statusCode
     */
    public void handleModifyNicknameFailed(String contact, int statusCode) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleModifyNicknameFailed " + statusCode + ",contact:"+ contact);
            }
              
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //need switch case of statusCode
                    //listeners.getBroadcastItem(i).onModifyNickNameResult(GroupChat.ReasonCode.INTERNAL_ERROR, statusCode);
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
    
    /**
     * Modify Nickname BY remote
     * 
     * @param contact contact
     */
    public void handleModifyNicknameByRemote(String contact, String newNickname) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("handleModifyNicknameByRemote " + contact + ", newNickname:"+ newNickname);
            }

            RichMessagingHistory.getInstance().updateGroupMemberName(session.getContributionID(),contact ,newNickname);
            
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onNickNameChanged(contact, newNickname);
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
    
    /**
     * Session has been terminated by remote
     */
    public void handleSessionTerminatedByRemote() {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.error("GCM Session terminated by remote");
            }
    
            // Update rich messaging history
            if (session.getDialogPath().isSessionCancelled()) {
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.ABORTED_BY_REMOTE);
            } else {
                setStateAndReasonCode(GroupChat.State.ABORTED, ReasonCode.REJECTED_BY_REMOTE);
            }
            
            /*  // Notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("handleSessionTerminatedByRemote N: " + N);
            }
            try {
                //listeners.getBroadcastItem(i).onSessionAborted();
                if (logger.isActivated()) {
                    logger.info("Write new listener");
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("GCM Can't notify listener", e);
                }
            }
            listeners.finishBroadcast();*/
            
            // Remove session from the list
            ChatServiceImpl.removeGroupChatSession(getChatId());
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
                logger.info("ABCG New IM received: alias: " + message.getDisplayName() + "Text: " + message.getTextMessage());
            }
            
            // Update rich messaging history
            RichMessagingHistory.getInstance().addGroupChatMessage(session.getContributionID(),
                    message, Direction.INCOMING.toInt());
            
              // Notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("handleReceiveMessage n: " + N);
            }
            int status = -1;
            if(message.isImdnDisplayedRequested()){
                status = Status.DISPLAY_REPORT_REQUESTED.toInt();
            } else {
                status = Status.RECEIVED.toInt();
            }
           /* ExtendMessage msgApi = new ExtendMessage(
                    message.getMessageId(),
                    PhoneUtils.extractNumberFromUri(message.getRemote()),
                    message.getTextMessage(),message.getMimeType(),
                    message.getDisplayName(),
                    Direction.INCOMING.toInt(),
                    message.getServerDate().getTime(),message.getServerDate().getTime(),
                    message.getServerDate().getTime(),message.getServerDate().getTime(),
                    status,
                    0,getChatId(),
                    false,false,
                    message.getMessageType());*/
            ((GroupChatEventBroadcaster)mBroadcaster).broadcastMessageReceived(message.getMimeType(), message.getMessageId(),message.getDisplayName());
            /*if(N == 0){
             // Broadcast intent related to the received invitation
                if (logger.isActivated()) {
                    logger.info("handleReceiveMessage Broadcast N: " + N);
                }
                Intent intent = new Intent(GroupChatIntent.ACTION_REINVITATION);
                intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
                intent.putExtra(GroupChatIntent.EXTRA_CHAT_ID, getChatId());
                intent.putExtra(GroupChatIntent.EXTRA_SUBJECT, getSubject());
                //intent.putExtra("autoAccept", autoAccept);
               // intent.putExtra(ChatIntent.EXTRA_CONTACT, msgApi.getContact());
                //intent.putExtra(ChatIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
                intent.putExtra(GroupChatIntent.EXTRA_MESSAGE_ID, message.getMessageId());
                AndroidFactory.getApplicationContext().sendBroadcast(intent);
            }*/
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
                logger.error("handleImError IM error message:" + error.getMessage() + ", code:" + error.getErrorCode());
            }
            if (error.getErrorCode() == ChatError.SESSION_INITIATION_CANCELLED) {
                if (logger.isActivated()) {
                    logger.info(" handleImError IM error1 ");
                }
                // Do nothing here, this is an aborted event
                // Remove session from the list
                ChatServiceImpl.removeGroupChatSession(getChatId());
                return;
            }
            
            if (logger.isActivated()) {
                logger.info("IM error " + error.getErrorCode());
            }
            
            /*// Update rich messaging history
            switch(error.getErrorCode()){
                case ChatError.SESSION_NOT_FOUND:
                case ChatError.SESSION_RESTART_FAILED:
                    // These errors are not logged
                    break;
                default:
                    RichMessagingHistory.getInstance().updateGroupChatStatus(session.getContributionID(), GroupChat.State.FAILED.toInt());
                    break;
            }*/
            
            try {
                GroupChat.ReasonCode code;
                switch(error.getErrorCode()) {
                    case ChatError.SESSION_INITIATION_DECLINED:
                        code = GroupChat.ReasonCode.REJECTED_BY_REMOTE;
                        if(error.getMessage().contains("Too Many Participants"))
                            code = GroupChat.ReasonCode.REJECTED_MAX_PARTICIPANTS;
                        setStateAndReasonCode(GroupChat.State.REJECTED, code);
                        break;
                    case ChatError.SESSION_NOT_FOUND:
                        restartGroup();
                        break;
                    default:
                        if (logger.isActivated()) {
                            logger.info("handleImError default");
                        }
                        code = GroupChat.ReasonCode.FAILED_INITIATION;
                        setStateAndReasonCode(GroupChat.State.FAILED, code);
                }
                //listeners.getBroadcastItem(i).onSessionError(code);
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
            
            // Remove session from the list
            ChatServiceImpl.removeGroupChatSession(getChatId());
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
                logger.error("GCM Invite error " + error.getErrorCode());
            }
            
            // Update rich messaging history
            switch(error.getErrorCode()){
                case ChatError.SESSION_NOT_FOUND:
                case ChatError.SESSION_RESTART_FAILED:
                    // These errors are not logged
                    break;
                default:
                    RichMessagingHistory.getInstance().updateGroupChatStatus(session.getContributionID(), GroupChat.State.FAILED.toInt());
                    break;
            }
            
              // Notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("GCM handleInviteError N: " + N);
            }
            for (int i=0; i < N; i++) {
                try {
                    int code;
                    switch(error.getErrorCode()) {
                        case ChatError.SESSION_INITIATION_ERROR:
                            code = GroupChat.ReasonCode.FAILED_INITIATION.toInt();
                            break;
                        case ChatError.SESSION_FORBIDDEN_ERROR:
                            // TODO: notify listener
                            // Notify event listeners
                            if (logger.isActivated()) {
                                logger.info("GroupChat SESSION_FORBIDDEN_ERROR N:" + N);
                            }
                            code = GroupChat.ReasonCode.REJECTED_BY_REMOTE.toInt();                          
                            break;
                        default:
                            code = GroupChat.ReasonCode.REJECTED_BY_SYSTEM.toInt();
                    }
                    //listeners.getBroadcastItem(i).onSessionError(code);
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
            
            // Remove session from the list
            ChatServiceImpl.removeGroupChatSession(getChatId());
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
            contact = PhoneUtils.extractNumberFromUri(contact);

            if (logger.isActivated()) {
                logger.info("GCM " + contact + " is composing status set to " + status);
            }
    
              // Notify event listeners
            final int N = listeners.beginBroadcast();
            /*for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onComposingEvent(contact, status);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("GCM Can't notify listener", e);
                    }
                }
            }*/
            // Notify event listeners
            ContactId id = ContactIdUtils.createContactIdFromTrustedData(contact);
            mBroadcaster.broadcastComposingEvent(mChatId, id, status);
            listeners.finishBroadcast();
        }
    }
    
    public void onConferenceEvent(String contact, GroupChat.ParticipantStatus status){
        if (logger.isActivated()) {
            logger.error("onConferenceEvent contact" + contact + ", status=" + status.toInt());
        }
        ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(contact);
        if (ParticipantStatus.CONNECTED.equals(status)) {
            RichMessagingHistory.getInstance().addGroupChatEvent(mChatId, contactId, GroupChatEvent.Status.JOINED,System.currentTimeMillis());
        } else {
            RichMessagingHistory.getInstance().addGroupChatEvent(mChatId, contactId, GroupChatEvent.Status.DEPARTED,System.currentTimeMillis());
        }
        
        mBroadcaster.broadcastParticipantStatusChanged(mChatId, contactId, status);
    }
    
    /**
     * Conference event
     * 
     * @param contact Contact
     * @param contactDisplayname Contact display name
     * @param state State associated to the contact
     */
    public void handleConferenceEvent(String contact, String contactDisplayname, String state, String method, String userStateParameter, String conferenceState) {
        synchronized(lock) {
            contact = PhoneUtils.extractNumberFromUri(contact);

            if (logger.isActivated()) {
                logger.error("GCM New conference event " + state + "method=" + method + " for " + contact + ",Displayname: " + contactDisplayname + ",user State parameter is :" + userStateParameter + ",conf state:"+ conferenceState);
            }
            
            GroupChat.ParticipantStatus status = User.getParticipantStatus(state);
            
            // Update history and notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("GCM handleConferenceEvent N: " + N);
            }
            
            for (int i=0; i < N; i++) {
                try {
                    if (state.equals(User.STATE_DISCONNECTED) && method.equals(User.STATE_BOOTED) && userStateParameter.equals(User.STATE_DELETED)) {
                        // Update rich messaging history
                        //RichMessagingHistory.getInstance().addGroupChatSystemMessage(session.getContributionID(), contact, GroupChat.ParticipantStatus.DISCONNECTED.toInt());
                        RichMessagingHistory.getInstance().removeGroupMember(session.getContributionID(),contact);
                        // Notify event listener
                        //listeners.getBroadcastItem(i).onReportParticipantKickedOut(contact);
                        onConferenceEvent(contact,status);
                        if (logger.isActivated()) {
                            logger.info("Write new listener");
                        }
                    } else
                    if (state.equals(User.STATE_CONNECTED)) {
                        // Update rich messaging history
                        //RichMessagingHistory.getInstance().addGroupChatSystemMessage(session.getContributionID(), contact, GroupChat.ParticipantStatus.DISCONNECTED.toInt());
                        if(!(RichMessagingHistory.getInstance().isGroupMemberExist(getChatId(),contact))){//TODO why not get from IMS profile instead of db
                        RichMessagingHistory.getInstance().addGroupMember(session.getContributionID(),contactDisplayname,contact,null);
                            if(conferenceState.equals("partial") || !RcsSettings.getInstance().supportOP01()){
                             // Notify event listener with state partial, partial means something changed in group
                                // full conference state means , full list of members
                                //listeners.getBroadcastItem(i).onParticipantJoined(contact, contactDisplayname);
                                onConferenceEvent(contact,status);
                                if (logger.isActivated()) {
                                    logger.info("Write new listener");
                                }
                            }
                        }else if(RcsSettings.getInstance().supportOP01()){
                            //listeners.getBroadcastItem(i).onParticipantJoined(contact, contactDisplayname);
                            onConferenceEvent(contact,status);
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
                        }
                          
                    } else
                    if (state.equals(User.STATE_DISCONNECTED)) {
                        // Update rich messaging history
                        //RichMessagingHistory.getInstance().addGroupChatSystemMessage(session.getContributionID(), contact, GroupChat.ParticipantStatus.DISCONNECTED.toInt());
                        RichMessagingHistory.getInstance().removeGroupMember(session.getContributionID(),contact);
                          // Notify event listener
                        if(method.equals(User.STATE_BOOTED)){
						    //listeners.getBroadcastItem(i).onSessionAborted();
                            onConferenceEvent(contact,status);
                            if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
						} else {
						    //listeners.getBroadcastItem(i).onParticipantDisconnected(contact);
						    onConferenceEvent(contact,status);
						    if (logger.isActivated()) {
                                logger.info("Write new listener");
                            }
						}
                    } else
                    if (state.equals(User.STATE_DEPARTED)) {
                        // Update rich messaging history
                        //RichMessagingHistory.getInstance().addGroupChatSystemMessage(session.getContributionID(), contact, GroupChat.ParticipantStatus.DISCONNECTED.toInt());
                        RichMessagingHistory.getInstance().removeGroupMember(session.getContributionID(),contact);
                          // Notify event listener
                        //listeners.getBroadcastItem(i).onParticipantLeft(contact);
                        onConferenceEvent(contact,status);
                        if (logger.isActivated()) {
                            logger.info("Write new listener");
                        }
                    }
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("GCM Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();
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
                logger.info("handleMessageDeliveryStatus1 " + msgId + ", status " + status + "Contact" + contact + ",date: " + date);
            }
    
            try{
                contact = PhoneUtils.extractNumberFromUri(contact);
            } catch(Exception e){
                e.printStackTrace();
            }

            logger.info("handleGroupMessageDeliveryStatus1 contact:" + contact);
            
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(msgId, status);
            
            // Notify event listeners
            try {
                if(date == null){
                    date = new Date();
                }
                Content.ReasonCode error = Content.ReasonCode.FAILED_SEND;
                if (status.equals(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
                    onMessageDeliveryStatusDelivered(contact,msgId,date.getTime());
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
                    onMessageDeliveryStatusDisplayed(contact,msgId,date.getTime());
                } else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_ERROR)) {
                    onMessageDeliveryStatusFailed(contact,msgId,error);
                }  else
                if (status.equals(ImdnDocument.DELIVERY_STATUS_SENT)) {
                    onMessageSent(msgId);
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("GCM Can't notify listener", e);
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
    public void handleMessageDeliveryStatus(String msgId, String status, String contact, int errorCode, String statusCode) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.error("handleGroupMessageDeliveryStatus2 " + msgId + ", status " + status + "Contact" + contact + ", errorcode:" + errorCode);
            }

            try{
                contact = PhoneUtils.extractNumberFromUri(contact);
            } catch(Exception e){
                e.printStackTrace();
            }

            logger.info("handleGroupMessageDeliveryStatus2 contact:" + contact);
            
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateChatMessageDeliveryStatus(msgId, status);
            Content.ReasonCode error = null;
            
            try {                    
                if(status.contains("fallback")){
                    error = Content.ReasonCode.FALLBACK;
                } else {
                    int code = 0;
                    try {
                        code = Integer.parseInt(statusCode);
                    } catch (Exception e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }
                    switch(code){
                        case 501: error = Content.ReasonCode.INTERNAL_ERROR; break;
                        case 415: error = Content.ReasonCode.UNSUPPORTED_MEDIA; break;
                        case 423: error = Content.ReasonCode.INTERVAL_SMALL; break;
                        default : error = Content.ReasonCode.FAILED_SEND; break;
                    }
                }
                if (status.equals(ImdnDocument.DELIVERY_STATUS_ERROR)) {
                    onMessageDeliveryStatusFailed(contact,msgId,error);
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
     * Request to add participant is successful
     */
    public void handleAddParticipantSuccessful() {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("GCM Add participant request is successful");
            }
    
            // TODO: nothing send over API?
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("GCM handleAddParticipantSuccessful N: " + N);
            }
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onInviteParticipantsResult(GroupChat.ParticipantStatus.SUCCESS,"");
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
    
    /**
     * Request to add participant has failed
     * 
     * @param reason Error reason
     */
    public void handleAddParticipantFailed(String reason) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("GCM Add participant request has failed " + reason);
            }
    
            // TODO: nothing send over API?
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onInviteParticipantsResult(GroupChat.ParticipantStatus.FAIL,reason);
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

    /**
     * Request to add participant has failed
     * 
     * @param reason Error reason
     */
    public void handleNickNameModified(String contact , String displayname) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("GCM handleNickNameModified contact" + contact +  " displayname: " + displayname);
            }
            RichMessagingHistory.getInstance().updateGroupMemberName(session.getContributionID(),contact,displayname);
    
            // TODO: nothing send over API?
            // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onInviteParticipantsResult(GroupChat.ParticipantStatus.FAIL,reason);
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
            RichMessagingHistory.getInstance().addGroupChatMessage(session.getContributionID(),
                    geoloc, Direction.INCOMING.toInt());
            
              // Notify event listeners
            final int N = listeners.beginBroadcast();
            for (int i=0; i < N; i++) {
                try {
                    //Geoloc geolocApi = new Geoloc(geoloc.getGeoloc().getLabel(),
                     //       geoloc.getGeoloc().getLatitude(), geoloc.getGeoloc().getLongitude(),
                     //       geoloc.getGeoloc().getExpiration());
                    //com.gsma.services.rcs.chat.GeolocMessage msgApi = new com.gsma.services.rcs.chat.GeolocMessage(geoloc.getMessageId(),
                     //       PhoneUtils.extractNumberFromUri(geoloc.getRemote()),
                      //      geolocApi, geoloc.getDate(), geoloc.isImdnDisplayedRequested());
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
        }
    }

    @Override
    public void handleConferenceNotify(String confState, List<ConferenceUser> users) {
        synchronized(lock) {

            if (logger.isActivated()) {
                logger.error("GCM New conference event op01: " + confState + "users=" + users );
            }

            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("GCM handleConferenceEvent N: " + N);
            }
            for (int i=0; i < N; i++) {
                try {

                   //listeners.getBroadcastItem(i).onConferenceNotify(confState, users);
                    if (logger.isActivated()) {
                        logger.info("Write new listener");
                    }
                   
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("GCM Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();
        }
        
    }
    
    public void handleConferenceNotify(Map<String, ParticipantStatus> participants) {
        synchronized(lock) {

            if (logger.isActivated()) {
                logger.error("GCM New conference event notify size: " + participants.size());
            }
            try {
                for(Map.Entry<String, ParticipantStatus> participant : participants.entrySet()){
                    onConferenceEvent(participant.getKey(), participant.getValue());
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("GCM Can't notify listener", e);
                }
            }
        }
        
    }
    
    public void onMessageSent(String msgId) {
        if (logger.isActivated()) {
            logger.info("onMessageSent; msgId=" + msgId);
        }
        String mimeType = mMessagingLog.getMessageMimeType(msgId);
        synchronized (lock) {
            if (mMessagingLog.setChatMessageStatusAndReasonCode(msgId, Status.SENT.toInt(),
                    Content.ReasonCode.UNSPECIFIED.toInt())) {
                mBroadcaster.broadcastMessageStatusChanged(getChatId(), mimeType, msgId,
                        Status.SENT, Content.ReasonCode.UNSPECIFIED);
            }
        }
    }
    
    private void onMessageDeliveryStatusDelivered(String contact, String msgId,
            long timestampDelivered) {
        if (logger.isActivated()) {
            logger.info("onMessageDeliveryStatusDelivered contact:" + contact + ", msgId: " + msgId);
        }
        String mimeType = mMessagingLog.getMessageMimeType(msgId);
        synchronized (lock) {
            if (mMessagingLog.setGroupChatDeliveryInfoDelivered(mChatId, contact, msgId,
                    timestampDelivered)) {
                ContactId id = ContactIdUtils.createContactIdFromTrustedData(contact);
                mBroadcaster.broadcastMessageGroupDeliveryInfoChanged(mChatId, id, mimeType,
                        msgId, GroupDeliveryInfo.Status.DELIVERED,
                        GroupDeliveryInfo.ReasonCode.UNSPECIFIED);
                if (mMessagingLog.isDeliveredToAllRecipients(msgId)) {
                    if (mMessagingLog.setMessageStatusDelivered(msgId, timestampDelivered)) {
                        mBroadcaster.broadcastMessageStatusChanged(mChatId, mimeType, msgId,
                                Status.DELIVERED, Content.ReasonCode.UNSPECIFIED);
                    }
                }
            }
        }
    }

    private void onMessageDeliveryStatusDisplayed(String contact, String msgId,
            long timestampDisplayed) {
        if (logger.isActivated()) {
            logger.info("onMessageDeliveryStatusDisplayed contact:" + contact + ", msgId: " + msgId);
        }
        String mimeType = mMessagingLog.getMessageMimeType(msgId);
        synchronized (lock) {
            if (mMessagingLog.setDeliveryInfoDisplayed(mChatId, contact, msgId,
                    timestampDisplayed)) {
                ContactId id = ContactIdUtils.createContactIdFromTrustedData(contact);
                mBroadcaster.broadcastMessageGroupDeliveryInfoChanged(mChatId, id, mimeType,
                        msgId, GroupDeliveryInfo.Status.DISPLAYED,
                        GroupDeliveryInfo.ReasonCode.UNSPECIFIED);
                if (mMessagingLog.isDisplayedByAllRecipients(msgId)) {
                    if (mMessagingLog.setMessageStatusDisplayed(msgId, timestampDisplayed)) {
                        mBroadcaster.broadcastMessageStatusChanged(mChatId, mimeType, msgId,
                                Status.DISPLAYED, Content.ReasonCode.UNSPECIFIED);
                    }
                }
            }
        }
    }

    private void onMessageDeliveryStatusFailed(String contact, String msgId,
            Content.ReasonCode reasonCode) {
        if (logger.isActivated()) {
            logger.info("onMessageDeliveryStatusFailed contact:" + contact + ", msgId: " + msgId + ", reasonCode: " + reasonCode.toInt() );
        }
        String mimeType = mMessagingLog.getMessageMimeType(msgId);
        synchronized (lock) {
            if (Content.ReasonCode.FAILED_DELIVERY == reasonCode) {
                if (!mMessagingLog.setGroupChatDeliveryInfoStatusAndReasonCode(mChatId, contact,
                        msgId, GroupDeliveryInfo.Status.FAILED.toInt(),
                        GroupDeliveryInfo.ReasonCode.FAILED_DELIVERY.toInt())) {
                    /* Add entry with delivered and displayed timestamps set to 0. */
                    mMessagingLog.addGroupChatDeliveryInfoEntry(mChatId, contact, msgId,
                            GroupDeliveryInfo.Status.FAILED.toInt(),
                            GroupDeliveryInfo.ReasonCode.FAILED_DELIVERY.toInt(), 0, 0);
                }
                ContactId id = ContactIdUtils.createContactIdFromTrustedData(contact);
                mBroadcaster.broadcastMessageGroupDeliveryInfoChanged(mChatId, id, mimeType,
                        msgId, GroupDeliveryInfo.Status.FAILED,
                        GroupDeliveryInfo.ReasonCode.FAILED_DELIVERY);
            } else {
                if (!mMessagingLog.setGroupChatDeliveryInfoStatusAndReasonCode(mChatId, contact.toString(),
                        msgId, GroupDeliveryInfo.Status.FAILED.toInt(),
                        GroupDeliveryInfo.ReasonCode.FAILED_DISPLAY.toInt())) {
                    /* Add entry with delivered and displayed timestamps set to 0. */
                    mMessagingLog.addGroupChatDeliveryInfoEntry(mChatId, contact, msgId,
                            GroupDeliveryInfo.Status.FAILED.toInt(),
                            GroupDeliveryInfo.ReasonCode.FAILED_DISPLAY.toInt(), 0, 0);
                }
                ContactId id = ContactIdUtils.createContactIdFromTrustedData(contact);
                mBroadcaster.broadcastMessageGroupDeliveryInfoChanged(mChatId, id, mimeType,
                        msgId, GroupDeliveryInfo.Status.FAILED,
                        GroupDeliveryInfo.ReasonCode.FAILED_DISPLAY);
            }
        }
    }
    
    public void onSessionAccepting(String contact) {
        if (logger.isActivated()) {
            logger.info("Accepting group chat session");
        }
        synchronized (lock) {
            setStateAndReasonCode(GroupChat.State.ACCEPTING, ReasonCode.UNSPECIFIED);
        }
    }

    public void onSessionRejected(String contact, TerminationReason reason) {
        if (logger.isActivated()) {
            logger.info("onSessionRejected contact:" + contact + ", reason: " + reason);
        }
        switch (reason) {
            case TERMINATION_BY_SYSTEM:
                /* Intentional fall through */
            case TERMINATION_BY_CONNECTION_LOST:
                //handleSessionRejected(ReasonCode.REJECTED_BY_SYSTEM);
                break;
            case TERMINATION_BY_TIMEOUT:
                //handleSessionRejected(ReasonCode.REJECTED_BY_TIMEOUT);
                break;
            case TERMINATION_BY_REMOTE:
                //handleSessionRejected(ReasonCode.REJECTED_BY_REMOTE);
                //mImService.tryToMarkQueuedGroupChatMessagesAndGroupFileTransfersAsFailed(mChatId);
                break;
            default:
                //handleSessionRejected(ReasonCode.REJECTED_BY_SYSTEM);
        }
    }
    
    /**
     * Request to add participant has failed
     * 
     * @param contact Contact ID
     * @param reason Error reason
     */
    public void onAddParticipantFailed(String contact, String reason) {
        if (logger.isActivated()) {
            logger.info("Add participant request has failed " + reason);
        }
        ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(contact);
        synchronized (lock) {
            mBroadcaster.broadcastParticipantStatusChanged(mChatId, contactId,
                    ParticipantStatus.FAILED);
        }
    }
}