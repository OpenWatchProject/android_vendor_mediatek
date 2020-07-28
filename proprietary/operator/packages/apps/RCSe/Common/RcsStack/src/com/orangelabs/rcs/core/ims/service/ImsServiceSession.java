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

import java.util.Vector;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Collection;
import javax2.sip.header.ContactHeader;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
import android.os.SystemProperties;

import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.SipManager;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaAttribute;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSessionListener;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.protocol.sip.SipResponse;
import com.orangelabs.rcs.core.ims.protocol.sip.SipTransactionContext;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSessionListener;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.ContributionIdGenerator;
import com.orangelabs.rcs.core.ims.service.im.chat.OneOneChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.RejoinGroupChatSession;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.utils.NetworkSwitchInfo;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.HttpFileTransferSession;
import com.orangelabs.rcs.core.ims.service.capability.CapabilityUtils;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.network.NetworkException;
import com.orangelabs.rcs.core.ims.protocol.PayloadException;
import com.gsma.services.rcs.contact.ContactId;
import android.net.Uri;

import java.text.ParseException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Vector;

import javax2.sip.InvalidArgumentException;
import javax2.sip.header.ContactHeader;
import javax2.sip.message.Response;

/**
 * IMS service session
 *
 * @author jexa7410
 */
public abstract class ImsServiceSession extends Thread {
    /**
     * Local virtual contact id
     */
    private String mVirtualId = SystemProperties.get("persist.vendor.rcs.virtualid", null);

    /**
     * Wait user answer for session invitation
     */
    private boolean mSessionInterrupted = false;
	 /**
     * Remote contactId
     */
    private ContactId mContact;

    private final long mRingingPeriod = 0;

  /**
     * Session timestamp
     */
    private long mTimestamp;

  /**
     * Session invitation status
     */
    public enum InvitationStatus {

        INVITATION_NOT_ANSWERED, INVITATION_ACCEPTED, INVITATION_REJECTED, INVITATION_CANCELED,
        INVITATION_TIMEOUT, INVITATION_REJECTED_BY_SYSTEM, INVITATION_DELETED,
        INVITATION_REJECTED_DECLINE, INVITATION_REJECTED_BUSY_HERE, INVITATION_REJECTED_FORBIDDEN
    }

    /**
     * Session termination reason
     */
    public enum TerminationReason {
        TERMINATION_BY_SYSTEM, TERMINATION_BY_USER, TERMINATION_BY_TIMEOUT, TERMINATION_BY_INACTIVITY,
        TERMINATION_BY_CONNECTION_LOST, TERMINATION_BY_LOW_BATTERY, TERMINATION_BY_REMOTE
    }

    /**
     * Session invitation status
     */
    public final static int INVITATION_NOT_ANSWERED = 0;
    public final static int INVITATION_ACCEPTED = 1;
    public final static int INVITATION_REJECTED = 2;
    public final static int INVITATION_CANCELED = 3;
    public final static int INVITATION_TIMEOUT = 4 ;
    public final static int INVITATION_REJECTED_BY_SYSTEM = 5 ;
    public final static int INVITATION_DELETED = 6 ;
    public final static int INVITATION_REJECTED_DECLINE = 7 ;
    public final static int INVITATION_REJECTED_BUSY_HERE = 8;
    public final static int INVITATION_REJECTED_FORBIDDEN = 9;



    /**
     * Session termination reason
     */
    public final static int TERMINATION_BY_SYSTEM = 0;
    public final static int TERMINATION_BY_USER = 1;
    public final static int TERMINATION_BY_TIMEOUT = 2;
    public final static int TERMINATION_BY_MSRP_ERROR = 3;
    public final static int TERMINATION_BY_SIP_TIMEOUT = 4;
    public final static int TERMINATION_BY_INACTIVITY = 5;
    public final static int TERMINATION_BY_CONNECTION_LOST = 6;
    public final static int TERMINATION_BY_LOW_BATTERY = 7;
    public final static int TERMINATION_BY_REMOTE = 8;
    public final static int TERMINATION_BY_MSRP_TIMEOUT = 9;

    public final static String INTENT_RINGING = "com.orangelabs.rcs.180_RINGING";
    public final static String INTENT_TRYING = "com.orangelabs.rcs.100_TRYING";

    private Map<String, ImsServiceSession> mImsServiceSessionCache = new HashMap<>();
    private Map<String, ImsServiceSession> mImsServiceSessionWithoutDialogPathCache = new HashMap<>();


    /*
     * Set Digits virtual line id as originating contact.
     * It will be added in P-Associated-From header of SIP INVITE.
     */
    protected void setVirtualId(String id) {
        mVirtualId = id;
    }

    /*
     * Set Digits virtual line id as originating contact.
     * It will be added in P-Associated-From header of SIP INVITE.
     */
    protected String getVirtualId() {
        return mVirtualId;
    }

    /*
     * Remove Digits virtual line id in P-Associated-From header of SIP INVITE.
     */
    protected void removeVirtualId() {
        mVirtualId = null;
    }

    /*
     * This method is by choice not synchronized here since the class extending this base-class will
     * need to handle the synchronization over a larger scope when calling this method anyway and we
     * would like to avoid double locks.
     */
    protected void addImsServiceSession(ImsServiceSession session) {
        mImsServiceSessionCache.put(session.getDialogPath().getCallId(), session);
    }

    /*
     * This method is by choice not synchronized here since the class extending this base-class will
     * need to handle the synchronization over a larger scope when calling this method anyway and we
     * would like to avoid double locks.
     */
    protected void removeImsServiceSession(ImsServiceSession session) {
        mImsServiceSessionCache.remove(session.getDialogPath().getCallId());
    }


    /**
     * Verify if session is initiated by remote part
     *
     * @return true if session is initiated by remote part
     */
     public boolean isInitiatedByRemote(){

    	 return false;
     }

    /**
     * Handle 180 Ringing
     *
     * @param response the response
     */
    public void handle180Ringing(SipResponse response) {
    }

    /**
     * Session accepting flag
     */
    private boolean mSessionAccepted = false;
    /**
     * Is session accepted
     *
     * @return Boolean
     */
    public boolean isSessionAccepted() {
        return mSessionAccepted;
    }

    /**
     * Set session accepted
     */
    public void setSessionAccepted() {
        mSessionAccepted = true;
    }

    /**
     * Removes the session
     */
    public  void removeSession(){

    }

    /**
     * Session inactivity event
     *
     * @throws NetworkException
     * @throws PayloadException
     */
    public  void handleInactivityEvent(){

    }



    /**
     * Gets IMS session from callId
     *
     * @param callId call ID
     * @return ImsServiceSession
     */
    public ImsServiceSession getImsServiceSession(String callId) {
        synchronized (getImsServiceSessionOperationLock()) {
            return mImsServiceSessionCache.get(callId);
        }
    }

    protected Object getImsServiceSessionOperationLock() {
        return mImsServiceSessionCache;
    }


    /*
     * This method is by choice not synchronized here since the class extending this base-class will
     * need to handle the synchronization over a larger scope when calling this method anyway and we
     * would like to avoid double locks.
     */
    protected void addImsServiceSessionWithoutDialogPath(ImsServiceSession session) {
        mImsServiceSessionWithoutDialogPathCache.put(session.getSessionID(), session);
    }

    /*
     * This method is by choice not synchronized here since the class extending this base-class will
     * need to handle the synchronization over a larger scope when calling this method anyway and we
     * would like to avoid double locks.
     */
    protected void removeImsServiceSessionWithoutDialogPath(ImsServiceSession session) {
        mImsServiceSessionWithoutDialogPathCache.remove(session.getSessionID());
    }




    /**
     * IMS service
     */
    protected ImsService imsService;

    public NetworkSwitchInfo netSwitchInfo = new NetworkSwitchInfo();

    /**
     * Session ID
     */
    private String sessionId =  SessionIdGenerator.getNewId();

    public void setSessionId(String sessionId) {
        this.sessionId = sessionId;
    }


    /**
     * Remote contact
     */
    private String contact;

    /**
     * Remote contact
     */
    private boolean isRingingRecieved = false;
    private boolean isTryingRecieved = false;

    public boolean isTryingRecieved() {
        return isTryingRecieved;
    }

    public void setTryingRecieved() {
        this.isTryingRecieved = true;
    }

    public boolean isRingingRecieved() {
        return isRingingRecieved;
    }

    public void setRingingRecieved() {
        this.isRingingRecieved = true;
    }

    private ApiReceiver mReceiver = null;
    private IntentFilter mIntentFilter = null;

    /**
     * Remote display name
     */
    private String remoteDisplayName = null;

    /**
     * Conversation ID
     */
    private static String mConversationId = "bcc642fe6a5347a64deb882d886c2c7a";

     /**
     * Remote display name
     */
    private String remoteGroupDisplayName = null;

    /**
     * Dialog path
     */
    protected SipDialogPath dialogPath = null;

    /**
     * Authentication agent
     */
    private SessionAuthenticationAgent authenticationAgent;

    /**
     * Session invitation status
     */
    protected int invitationStatus = INVITATION_NOT_ANSWERED;

    /**
     * Wait user answer for session invitation
     */
    protected Object waitUserAnswer = new Object();

    /**
     * Session listeners
     */
    private Vector<ImsSessionListener> listeners = new Vector<ImsSessionListener>();

    /**
     * Session timer manager
     */
    private SessionTimerManager sessionTimer = new SessionTimerManager(this);

    /**
     * Update session manager
     */
    protected UpdateSessionManager updateMgr;

    /**
     * Ringing period (in seconds)
     */
    private int ringingPeriod = RcsSettings.getInstance().getRingingPeriod();

    /**
     * Session interrupted flag
     */
    private boolean sessionInterrupted = false;

    /**
     * Session terminated by remote flag
     */
    private boolean sessionTerminatedByRemote = false;


    public long getmTimestamp() {
        return mTimestamp;
    }

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     *
     * @param parent IMS service
     * @param contact Remote contact
     */
    public ImsServiceSession(ImsService imsService, String contact) {
        this.imsService = imsService;
        this.contact = contact;
        this.authenticationAgent = new SessionAuthenticationAgent(imsService.getImsModule());
        this.updateMgr = new UpdateSessionManager(this);
        if(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()){
            mReceiver = new ApiReceiver();
            mIntentFilter = new IntentFilter();
            mIntentFilter.addAction(INTENT_RINGING);
            mIntentFilter.addAction(INTENT_TRYING);
            AndroidFactory.getApplicationContext().registerReceiver(mReceiver,mIntentFilter);
        }
        mTimestamp = System.currentTimeMillis();
    }



    /**
     * Constructor
     *
     * @param parent IMS service
     * @param contact Remote contact
     */
    public ImsServiceSession(ImsService imsService, ContactId contact) {
        this.imsService = imsService;
        mContact = contact;
        this.authenticationAgent = new SessionAuthenticationAgent(imsService.getImsModule());
        this.updateMgr = new UpdateSessionManager(this);
        if(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()){
            mReceiver = new ApiReceiver();
            mIntentFilter = new IntentFilter();
            mIntentFilter.addAction(INTENT_RINGING);
            mIntentFilter.addAction(INTENT_TRYING);
            AndroidFactory.getApplicationContext().registerReceiver(mReceiver,mIntentFilter);
        }
        mTimestamp = System.currentTimeMillis();
    }



    /**
     * Constructor
     *
     * @param parent IMS service
     * @param contact Remote contact
     */
    public ImsServiceSession(ImsService imsService, ContactId contact , long timestamp) {
        this.imsService = imsService;
        mContact = contact;
        mTimestamp = timestamp;
        this.authenticationAgent = new SessionAuthenticationAgent(imsService.getImsModule());
        this.updateMgr = new UpdateSessionManager(this);
        if(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()){
            mReceiver = new ApiReceiver();
            mIntentFilter = new IntentFilter();
            mIntentFilter.addAction(INTENT_RINGING);
            mIntentFilter.addAction(INTENT_TRYING);
            AndroidFactory.getApplicationContext().registerReceiver(mReceiver,mIntentFilter);
        }
    }

    /**
     * Returns the timestamp of the session
     *
     * @return timestamp
     */
    public long getTimestamp() {
        return mTimestamp;
    }

    /**
     * Create originating dialog path
     */
    public void createOriginatingDialogPath() {
        // Set Call-Id
        String callId = getImsService().getImsModule().getSipManager().getSipStack().generateCallId();

        // Set the route path
        Vector<String> route = getImsService().getImsModule().getSipManager().getSipStack().getServiceRoutePath();

        // Create a dialog path
        dialogPath = new SipDialogPath(
                getImsService().getImsModule().getSipManager().getSipStack(),
                callId,
                1,
                getRemoteContact(),
                ImsModule.IMS_USER_PROFILE.getPublicAddress(),
                getRemoteContact(),
                route);


        logger.debug("createOriginatingDialogPath() = " + ImsModule.IMS_USER_PROFILE.getPublicAddress());

        // Set the authentication agent in the dialog path
        dialogPath.setAuthenticationAgent(getAuthenticationAgent());
    }

    /**
     * M: Added to resolve the rich call 403 error.@{
     */
    /**
     * Create originating dialog path
     */
    public void createOriginatingDialogPath(String callId) {
        logger.debug("createOriginatingDialogPath(), callId = " + callId);
        // Set the route path
        Vector<String> route = getImsService().getImsModule().getSipManager().getSipStack()
                .getServiceRoutePath();

        // Create a dialog path
        dialogPath = new SipDialogPath(
                getImsService().getImsModule().getSipManager().getSipStack(), callId,
                1, getRemoteContact(),
                ImsModule.IMS_USER_PROFILE.getPublicUri(),
                getRemoteContact(), route);

        // Set the authentication agent in the dialog path
        dialogPath.setAuthenticationAgent(getAuthenticationAgent());
    }
    /**
     * @}
     */

    /**
     * Create terminating dialog path
     *
     * @param invite Incoming invite
     */
    public void createTerminatingDialogPath(SipRequest invite) {
        // Set the call-id
        String callId = invite.getCallId();

        // Set target
        String target = invite.getContactURI();

        // Set local party
        String localParty = invite.getTo();

        // Set remote party
        String remoteParty = invite.getFrom();

        // Get the CSeq value
        long cseq = invite.getCSeq();

        // Set the route path with the Record-Route
        Vector<String> route = SipUtils.routeProcessing(invite, false);

           // Create a dialog path
        dialogPath = new SipDialogPath(
                getImsService().getImsModule().getSipManager().getSipStack(),
                callId,
                cseq,
                target,
                localParty,
                remoteParty,
                route);

        // Set the INVITE request
        dialogPath.setInvite(invite);

        // Set the remote tag
        dialogPath.setRemoteTag(invite.getFromTag());

        // Set the remote content part
        dialogPath.setRemoteContent(invite.getContent());

        // Set the session timer expire
        dialogPath.setSessionExpireTime(invite.getSessionTimerExpire());
    }

    /**
     * Add a listener for receiving events
     *
     * @param listener Listener
     */
    public void addListener(ImsSessionListener listener) {
        listeners.add(listener);
    }

    /**
     * Remove a listener
     */
    public void removeListener(ImsSessionListener listener) {
        listeners.remove(listener);
    }

    /**
     * Remove all listeners
     */
    public void removeListeners() {
        listeners.removeAllElements();
    }

    /**
     * Returns the event listeners
     *
     * @return Listeners
     */
    public Vector<ImsSessionListener> getListeners() {
        return listeners;
    }

    /**
     * Get the session timer manager
     *
     * @return Session timer manager
     */
    public SessionTimerManager getSessionTimerManager() {
        return sessionTimer;
    }

    /**
     * Get the update session manager
     *
     * @return UpdateSessionManager
     */
    public UpdateSessionManager getUpdateSessionManager() {
        return updateMgr;
    }

    /**
     * Is behind a NAT
     *
     * @return Boolean
     */
    public boolean isBehindNat() {
        return getImsService().getImsModule().getCurrentNetworkInterface().isBehindNat();
    }

    public boolean isSecureProtocolMessage(){
        return getImsService().getImsModule().getCurrentNetworkInterface().getIsSecureProtocol()
                && !(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07());
        }

    /**
     * Start the session in background
     */
    public void startSession() {
        // Add the session in the session manager
        imsService.addSession(this);

        // Start the session
        start();
    }

    /**
     * Return the IMS service
     *
     * @return IMS service
     */
    public ImsService getImsService() {
        return imsService;
    }

    /**
     * Return the session ID
     *
     * @return Session ID
     */
    public String getSessionID() {
        return sessionId;
    }

    /**
     * Returns the remote contact
     *
     * @return String
     */
    public String getRemoteContact() {
        return contact;
    }

    public ContactId getRemoteContactId() {
        return mContact;
    }

    /**
     * Returns display name of the remote contact
     *
     * @return String
     */
    public String getRemoteDisplayName() {
        if (getDialogPath() == null) {
            return remoteDisplayName;
        } else {
            return SipUtils.getDisplayNameFromUri(getDialogPath().getInvite().getFrom());
        }
    }

    /**
     * Returns display name of the remote contact
     *
     * @return String
     */
    public String getGroupRemoteDisplayName() {
         return remoteGroupDisplayName;
    }
    /**
     * Set display name of the remote contact
     *
     * @param String
     */
    public void setGroupRemoteDisplayName(String remoteGroupDisplayName) {
        this.remoteGroupDisplayName = remoteGroupDisplayName;
    }

    /**
     * Set display name of the remote contact
     *
     * @param String
     */
    public void setRemoteDisplayName(String remoteDisplayName) {
        this.remoteDisplayName = remoteDisplayName;
    }

    /**
     * Get the dialog path of the session
     *
     * @return Dialog path object
     */
    public SipDialogPath getDialogPath() {
        return dialogPath;
    }

    /**
     * Set the dialog path of the session
     *
     * @param dialog Dialog path
     */
    public void setDialogPath(SipDialogPath dialog) {
        dialogPath = dialog;
    }

    /**
     * Returns the authentication agent
     *
     * @return Authentication agent
     */
    public SessionAuthenticationAgent getAuthenticationAgent() {
        return authenticationAgent;
    }

    /**
     * Reject the session invitation
     *
     * @param code Error code
     */
    public void rejectSession(int code) {
        if (logger.isActivated()) {
            logger.debug("Session invitation has been rejected");
        }
        invitationStatus = INVITATION_REJECTED;

        // Unblock semaphore
        synchronized(waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }

        // Decline the invitation
        sendErrorResponse(getDialogPath().getInvite(), getDialogPath().getLocalTag(), code);

        // Remove the session in the session manager
        imsService.removeSession(this);
    }

    /**
     * Session invitation status
     */
    protected InvitationStatus mInvitationStatus = InvitationStatus.INVITATION_NOT_ANSWERED;
    /**
     * Reject the session invitation
     *
     * @param status InvitationStatus
     */
    public void rejectSession(InvitationStatus status) {
        if (logger.isActivated()) {
        	logger.debug("Session invitation has been rejected");
        }
        mInvitationStatus = status;

        synchronized (waitUserAnswer) {
        }
    }




    /**
     * Accept the session invitation
     */
    public void acceptSession() {
        if (logger.isActivated()) {
            logger.debug("Session invitation has been accepted");
        }
        invitationStatus = INVITATION_ACCEPTED;

        // Unblock semaphore
        synchronized(waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }
    }

    /**
     * Wait session invitation answer
     *
     * @return Answer
     */
    public InvitationStatus waitInvitationAnswerExt() {
        return waitInvitationAnswer(mRingingPeriod);
    }

    /**
     * Wait session invitation answer
     *
     * @param timeout value
     * @return Answer
     */
    public InvitationStatus waitInvitationAnswer(long timeout) {
        if (logger.isActivated()) {
        	logger.debug("Wait session invitation answer delay=".concat(Long.toString(timeout)));
        }
        /* Wait until received response or received timeout */
        try {
            long timeToWait = (timeout > 0) ? timeout : mRingingPeriod;
            long startTime = System.currentTimeMillis();
            while (InvitationStatus.INVITATION_NOT_ANSWERED == mInvitationStatus) {
                synchronized (waitUserAnswer) {
                    waitUserAnswer.wait(timeToWait);
                    long waitedTime = System.currentTimeMillis() - startTime;
                    if (waitedTime < timeToWait) {
                        timeToWait -= waitedTime;
                    } else if (InvitationStatus.INVITATION_NOT_ANSWERED == mInvitationStatus) {
                        mInvitationStatus = InvitationStatus.INVITATION_TIMEOUT;
                    }
                }
            }
            return mInvitationStatus;

        } catch (InterruptedException e) {
            mSessionInterrupted = true;
            if (InvitationStatus.INVITATION_DELETED == mInvitationStatus) {
                return InvitationStatus.INVITATION_DELETED;
            }
            return InvitationStatus.INVITATION_REJECTED_BY_SYSTEM;
        }
    }

    /**
     * Wait session invitation answer
     *
     * @return Answer
     */
    public int waitInvitationAnswer() {
        if (invitationStatus != INVITATION_NOT_ANSWERED) {
            return invitationStatus;
        }

        if (logger.isActivated()) {
            logger.info("Wait session invitation answer");
        }

        // Wait until received response or received timeout
        try {
            synchronized(waitUserAnswer) {
                waitUserAnswer.wait(ringingPeriod * 1000);
            }
        } catch(InterruptedException e) {
            if (logger.isActivated()) {
                logger.info("ABC Wait session invitation answer1");
            }
            sessionInterrupted = true;
        }

        return invitationStatus;
    }

    /**
     * Interrupt session
     */
    public void interruptSession() {
        if (logger.isActivated()) {
            logger.info("ABC Interrupt the session");
        }

        try {
            // Unblock semaphore
            synchronized(waitUserAnswer) {
                waitUserAnswer.notifyAll();
            }

            if (!isSessionInterrupted()) {
                if (logger.isActivated()) {
                    logger.info("ABC Interrupt the session1");
                }
                // Interrupt thread
                sessionInterrupted = true;
                interrupt();
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't interrupt the session correctly", e);
            }
        }
        if (logger.isActivated()) {
            logger.debug("Session has been interrupted");
        }
    }

    /**
     * M: do not send SIP BYE when timeout to distinguish BOOTED from DEPARTED @{
     */
    /**
     * Abort the session
     */
    public void abortSessionWithoutBye() {
        if (logger.isActivated()) {
            logger.info("abortSessionWithoutBye() entry");
        }

        // Interrupt the session
        interruptSession();

        // Terminate session
        terminateSessionWithoutBy();

        // Close media session
        closeMediaSession();

        // Remove the current session
        getImsService().removeSession(this);
        removeSession();

        // Notify listeners
        int size = getListeners().size();
        for (int i = 0; i < size; i++) {
            getListeners().get(i).handleSessionAborted(0);
        }
    }

    /** @} */

    /**
     * Abort the session
     *
     * @param reason Termination reason
     */
    public void abortSession(int reason) {
        if (logger.isActivated()) {
            logger.info("ABC Abort the session " + reason);
            if((netSwitchInfo.get_ims_off_by_network())){
                logger.info("ABC Abort the session Network gone");
            }
        }

        // Interrupt the session
        interruptSession();
        if(!(netSwitchInfo.get_ims_off_by_network()) || !(this instanceof HttpFileTransferSession)){

            if (logger.isActivated()) {
                logger.info("Close media session before closing sip session");
            }
            // Close media session
            closeMediaSession();

        // Terminate session
        terminateSession(reason);

        if (logger.isActivated()) {
            logger.info("ABC Abort the session " + reason);
        }

        // Remove the current session
        getImsService().removeSession(this);
        removeSession();

        //notify user about quit conversation is successful if triggered by user
        //if session is group session
        if(reason == ImsServiceSession.TERMINATION_BY_USER){
            if (this instanceof GroupChatSession){
                for(int i=0; i < getListeners().size(); i++) {
                    ((ChatSessionListener)getListeners().get(i)).handleQuitConversationResult(200);
                }
            }
        }

        // Notify listeners
        for(int i=0; i < getListeners().size(); i++) {
            getListeners().get(i).handleSessionAborted(reason);
        }

        if(netSwitchInfo.get_ims_off_by_network()){
            netSwitchInfo.reset_ims_off_by_network();
        }

      }
       if((this instanceof HttpFileTransferSession)){
            ((HttpFileTransferSession)this).pauseFileTransfer();
        }
    }

    /**
     * M: do not send SIP BYE when timeout to distinguish BOOTED from DEPARTED @{
     */
    /**
     * Terminate session
     */
    public void terminateSessionWithoutBy() {
        if (dialogPath.isSessionTerminated()) {
            // Already terminated
            return;
        }

        // Stop session timer
        getSessionTimerManager().stop();

        // Update dialog path
        dialogPath.sessionTerminated();

        // Unblock semaphore (used for terminating side only)
        synchronized (waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }
    }

    public void terminateSipSession(int reason) {
        if (logger.isActivated()) {
            logger.debug("terminateSipSession (reason " + reason + ")");
        }

        if ((dialogPath == null) || dialogPath.isSessionTerminated()) {
            // Already terminated
            return;
        }

        // Stop session timer
        getSessionTimerManager().stop();
        dialogPath.sessionTerminated();

        // Unblock semaphore (used for terminating side only)
        synchronized (waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }
    }

    /**
     * This function is used when session needs to terminated in both invitation pending and started
     * state. If an error has occurred and the session needs to closed, please call
     * closeSession(TerminationReason).
     *
     * @param reason Termination reason
     * @throws NetworkException
     * @throws PayloadException
     */
    public void terminateSession(TerminationReason reason) throws Exception {
        if (logger.isActivated()) {
            logger.info("Terminate the session ".concat(reason.toString()));
        }
        boolean wasEstablished;
        if (isInitiatedByRemote()) {
            wasEstablished = dialogPath != null && isSessionAccepted();
        } else {
            wasEstablished = dialogPath != null && dialogPath.isSigEstablished();
        }

        interruptSession();

        closeSession(reason);

        closeMediaSession();

        removeSession();

        Collection<ImsSessionListener> listeners = getListeners();
        if (wasEstablished) {
          /*  for (ImsSessionListener listener : listeners) {
                listener.onSessionAborted(mContact, reason);
            }*/
            return;
        }
       /* for (ImsSessionListener listener : listeners) {
            listener.onSessionRejected(mContact, reason);
        }*/
    }

/**
     * Force terminate and remove the session
     *
     * @throws NetworkException
     * @throws PayloadException
     */
    public void deleteSession() throws PayloadException, NetworkException {
        invitationStatus = INVITATION_DELETED;
        interruptSession();
        try{
            closeSession(TerminationReason.TERMINATION_BY_USER);
           }catch(Exception e){
              throw new PayloadException("error closing session");
           }
        closeMediaSession();
        removeSession();
    }



    public void closeSession(TerminationReason reason) throws Exception {
      /*  if (dialogPath.isActivated()) {
            dialogPath.debug("Close the session (reason " + reason + ")");
        }*/
       /* if ((dialogPath == null) || dialogPath.isSessionTerminated()) {
            return;
        }*/
        sessionTimer.stop();
      /*  if (TerminationReason.TERMINATION_BY_USER == reason) {
            dialogPath.setSessionTerminated(Response.OK, "Call completed");
        } else {
            dialogPath.setSessionTerminated();
        }*/
        /* Unblock semaphore (used for terminating side only) */
        synchronized (waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }
        if (TerminationReason.TERMINATION_BY_CONNECTION_LOST == reason) {
            /*
             * Since connection is lost , we should not even try to send a BYE or CANCEL as we will
             * anyway fail , Hence return from this point.
             */
            return;
        }
        /* Close the session */
        if (dialogPath.isSigEstablished()) {
            getDialogPath().incrementCseq();

            // @FIXME:Inject SipManager instead
            /* Send BYE without waiting a response */
            getImsService().getImsModule().getSipManager().sendSipBye(getDialogPath());
        } else {
            /* Send CANCEL without waiting a response */
            getImsService().getImsModule().getSipManager().sendSipCancel(getDialogPath());
        }
        /*if (dialogPath.isActivated()) {
            dialogPath.debug("SIP session has been closed");
        }*/
    }


    /**
     * Terminate session
     *
     * @param reason Reason
     */
    public void terminateSession(int reason) {
        if (logger.isActivated()) {
            logger.debug("Terminate the session (reason " + reason + ")");
        }

        if ((dialogPath == null) || dialogPath.isSessionTerminated()) {
            // Already terminated
            return;
        }

        // Stop session timer
        getSessionTimerManager().stop();

        // Update dialog path
        if (reason == ImsServiceSession.TERMINATION_BY_USER) {
            dialogPath.sessionTerminated(200, "Call completed");
        } else
        if (reason == ImsServiceSession.TERMINATION_BY_SYSTEM) {
            if (RcsSettings.getInstance().supportOP01())
                dialogPath.sessionTerminated(480, "Temporarily Unavailable");
            else
                dialogPath.sessionTerminated();
        } else
        if (reason == ImsServiceSession.TERMINATION_BY_MSRP_ERROR) {
            dialogPath.sessionTerminated(402, "Loop Detected");
        } else if(reason == ImsServiceSession.TERMINATION_BY_MSRP_TIMEOUT) {
            dialogPath.sessionTerminated(480, "Temporarily Unavailable");
        } else {
            dialogPath.sessionTerminated();
        }

        // Unblock semaphore (used for terminating side only)
        synchronized (waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }

        try {
            // Terminate the session
            if (dialogPath.isSigEstablished()) {
                // Increment the Cseq number of the dialog path
                getDialogPath().incrementCseq();

                // Send BYE without waiting a response
                getImsService().getImsModule().getSipManager().sendSipBye(getDialogPath());
            } else {
                // Send CANCEL without waiting a response

                try {
                    Thread t = new Thread() {
                        public void run() {
                            handleError(new ImsSessionBasedServiceError(
                                    ImsSessionBasedServiceError.SESSION_INITIATION_FAILED,
                                    "Cancelled by sender"));
                        }
                    };
                    t.start();
                } catch (Exception e) {
                    e.printStackTrace();
                }

                getImsService().getImsModule().getSipManager().sendSipCancel(getDialogPath());
            }

            if (logger.isActivated()) {
                logger.debug("SIP session has been terminated");
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Session termination has failed", e);
            }
        }

        if (this.getDialogPath().isSigEstablished() &&
                reason != ImsServiceSession.TERMINATION_BY_USER) {
            for (int j = 0; j < getListeners().size(); j++) {
                final ImsSessionListener listener = getListeners().get(j);
                if (listener instanceof FileSharingSessionListener) {
                    AsyncTask.execute(new Runnable() {
                        @Override
                        public void run() {
                            try {
                                Thread.sleep(3 * 1000);
                            } catch (InterruptedException e) {
                                // Nothing to do
                            }
                            ((FileSharingSessionListener) listener).handleTransferError(
                                    new FileSharingError(FileSharingError.MEDIA_TRANSFER_FAILED));
                        }
                    });
                }
            }
        }
    }

    /**
     * Terminate Group session due to chairman leave the group
     *
     * @param reason Reason
     * @return
     */
    public int terminateGroupSession(int reason) {
        if (logger.isActivated()) {
            logger.debug("Terminate the session (reason " + reason + ")");
        }

        if ((dialogPath == null) || dialogPath.isSessionTerminated()) {
            // Already terminated
            return 0;
        }
        // Unblock semaphore (used for terminating side only)
        synchronized(waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }

        try {
            // Terminate the session
            if (dialogPath.isSigEstablished()) {
                // Increment the Cseq number of the dialog path
                getDialogPath().incrementCseq();
                // Create the SIP request

                SipRequest bye = SipMessageFactory.createGroupBye(getDialogPath());
                //getImsService().getImsModule().getSipManager().sendSipGroupBye(getDialogPath());
                //SipTransactionContext ctx = getImsService().getImsModule().getSipManager().sendSipMessageAndWait(bye, getResponseTimeout());
                SipTransactionContext ctx = getImsService().getImsModule().getSipManager().sendSubsequentRequest(getDialogPath(), bye);
                // Analyze the received response
                if (ctx.isSipResponse()) {
                    // A response has been received
                    if (logger.isActivated()) {
                        logger.debug("Response received with status code  " + ctx.getStatusCode() );
                    }
                    if ((ctx.getStatusCode() >= 200) && (ctx.getStatusCode() < 300)) {
                        //200 OK received
                     // Stop session timer
                        getSessionTimerManager().stop();

                        // Update dialog path
                        if (reason == ImsServiceSession.TERMINATION_BY_USER) {
                            dialogPath.sessionTerminated(200, "Call completed");
                        } else {
                            dialogPath.sessionTerminated();
                        }
                    }
                    return ctx.getStatusCode();
                }

                return 0;
                // Send BYE without waiting a response
                //getImsService().getImsModule().getSipManager().sendSipGroupBye(getDialogPath());
            }

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Session termination has failed", e);
            }
            return 0;
        }
        return 0;

        /*if (this.getDialogPath().isSigEstablished() && reason != ImsServiceSession.TERMINATION_BY_USER) {
            for (int j = 0; j < getListeners().size(); j++) {
                final ImsSessionListener listener = getListeners().get(j);
                if (listener instanceof FileSharingSessionListener) {
                     AsyncTask.execute(new Runnable() {
                @Override
                    public void run() {
                           try {
                            Thread.sleep(3 * 1000);
                          } catch (InterruptedException e) {
                            // Nothing to do
                          }
                    ((FileSharingSessionListener) listener).handleTransferError(new FileSharingError(FileSharingError.MEDIA_TRANSFER_FAILED));
                }
                });
                    }
            }
        }*/
    }

    /**
     * Receive BYE request
     *
     * @param bye BYE request
     */
    public void receiveBye(SipRequest bye) {
        if (logger.isActivated()) {
            logger.info("ABC Receive a BYE message from the remote");
        }
        String cause = "";
        cause = SipUtils.getReasonCause(bye);
        String reasonText = SipUtils.getReasonText(bye);
        // Close media session
        closeMediaSession();

        // Update the dialog path status
        getDialogPath().sessionTerminated();
        sessionTerminatedByRemote = true;

        // Remove the current session
        getImsService().removeSession(this);
        removeSession();

        // Stop session timer
        getSessionTimerManager().stop();

        try {
            // Notify listeners
            if (this instanceof GroupChatSession){

                for(int i=0; i < getListeners().size(); i++) {
                    ((ChatSessionListener)getListeners().get(i)).handleSessionTerminatedByRemote(cause, reasonText);
                }

            } else if (cause != null && cause.contains("480") && RcsSettings.getInstance().isSupportOP07()){
                for(int i=0; i < getListeners().size(); i++) {
                    ((ChatSessionListener)getListeners().get(i)).handleSessionTerminatedByRemote(cause, reasonText);
                }
            } else {
                for(int i=0; i < getListeners().size(); i++) {
                    getListeners().get(i).handleSessionTerminatedByRemote();
                }
            }
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.info("ABC Bye mesage not handled properly");
            }
            e.printStackTrace();
        }
    }

    /**
     * Receive CANCEL request
     *
     * @param cancel CANCEL request
     */
    public void receiveCancel(SipRequest cancel) {
        if (logger.isActivated()) {
            logger.info("ABC Receive a CANCEL message from the remote");
        }

        if (getDialogPath().isSigEstablished()) {
            if (logger.isActivated()) {
                logger.info("Ignore the received CANCEL message from the remote (session already established)");
            }
            return;
        }

        // Close media session
        closeMediaSession();

        // Update dialog path
        getDialogPath().sessionCancelled();

        // Send a 487 Request terminated
        try {
            if (logger.isActivated()) {
                logger.info("Send 487 Request terminated");
            }
            SipResponse terminatedResp = SipMessageFactory.createResponse(getDialogPath().getInvite(),
                    getDialogPath().getLocalTag(), 487);
            getImsService().getImsModule().getSipManager().sendSipResponse(terminatedResp);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send 487 error response", e);
            }
        }

        // Remove the current session
        getImsService().removeSession(this);
        removeSession();

        // Set invitation status
        invitationStatus = ImsServiceSession.INVITATION_CANCELED;

        // Unblock semaphore
        synchronized(waitUserAnswer) {
            waitUserAnswer.notifyAll();
        }

        // Notify listeners
        for(int i=0; i < getListeners().size(); i++) {
            getListeners().get(i).handleSessionTerminatedByRemote();
        }

        // Request capabilities to the remote
        getImsService().getImsModule().getCapabilityService().requestContactCapabilities(getDialogPath().getRemoteParty());
    }

    /**
     * Receive re-INVITE request
     *
     * @param reInvite re-INVITE request
     */
    public void receiveReInvite(SipRequest reInvite) {
        // Session refresh management
        sessionTimer.receiveReInvite(reInvite);
    }

    /**
     * Receive UPDATE request
     *
     * @param update UPDATE request
     */
    public void receiveUpdate(SipRequest update) {
        sessionTimer.receiveUpdate(update);
    }

    /**
     * Prepare media session
     *
     * @throws Exception
     */
    public abstract void prepareMediaSession() throws Exception;

    //public abstract void setMsrpFtSupport(boolean ftSupport);

    /**
     * Start media session
     *
     * @throws Exception
     */
    public  void startMediaSession() throws Exception{

    }

    /**
     * Close media session
     */
    public abstract void closeMediaSession();

    /**
     * Send a 180 Ringing response to the remote party
     *
     * @param request SIP request
     * @param localTag Local tag
     */
    public void send180Ringing(SipRequest request, String localTag) {
        try {
            SipResponse progress = null;
            if(RcsSettings.getInstance().supportOP01()) {
                progress = SipMessageFactory.createCMCCResponse(getDialogPath(),request, localTag, 180);
            } else if(RcsSettings.getInstance().isSupportOP08()){
                progress = SipMessageFactory.create180Response(getDialogPath(),request, localTag, 180);
                modify180Ringing(progress);
            } else {
                progress = SipMessageFactory.createResponse(request, localTag, 180);
            }
            getImsService().getImsModule().getSipManager().sendSipResponse(progress);
        } catch(Exception e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.error("Can't send a 180 Ringing response");
            }
        }
    }

    public SipResponse modify180Ringing(SipResponse progress){
        try {
            String contributionId = null;
            String conversationId = null;

            if(this instanceof ChatSession){
                 contributionId = ((ChatSession)this).getContributionID();
                 conversationId = ((ChatSession)this).getConversationID();
            } else if(this instanceof FileSharingSession){
                 contributionId = ((FileSharingSession)this).getContributionID();
                 conversationId = ((FileSharingSession)this).getConversationID();
            }

            if(contributionId != null){
                progress.addHeader(ChatUtils.HEADER_CONTRIBUTION_ID, contributionId);
            }
            if(conversationId != null){
                progress.addHeader(ChatUtils.HEADER_CONVERSATION_ID,conversationId);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return progress;
    }


    /**
     * Send an error response to the remote party
     *
     * @param request SIP request
     * @param localTag Local tag
     * @param code Response code
     */
    public void sendErrorResponse(SipRequest request, String localTag, int code) {
        try {
            // Send  error
            if (logger.isActivated()) {
                logger.info("Send " + code + " error response");
            }
            SipResponse resp = SipMessageFactory.createResponse(request, localTag, code);
            getImsService().getImsModule().getSipManager().sendSipResponse(resp);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send error response", e);
            }
        }
    }

    /**
     * Send a 603 "Decline" to the remote party
     *
     * @param request SIP request
     * @param localTag Local tag
     */
    public void send603Decline(SipRequest request, String localTag) {
        try {
            // Send a 603 Decline error
            if (logger.isActivated()) {
                logger.info("Send 603 Decline");
            }
            SipResponse resp = SipMessageFactory.createResponse(request, localTag, 603);
            getImsService().getImsModule().getSipManager().sendSipResponse(resp);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send 603 Decline response", e);
            }
        }
    }

    /**
     * Send a 486 "Busy" to the remote party
     *
     * @param request SIP request
     * @param localTag Local tag
     */
    public void send486Busy(SipRequest request, String localTag) {
        try {
            // Send a 486 Busy error
            if (logger.isActivated()) {
                logger.info("Send 486 Busy");
            }
            SipResponse resp = SipMessageFactory.createResponse(request, localTag, 486);
            getImsService().getImsModule().getSipManager().sendSipResponse(resp);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send 486 Busy response", e);
            }
        }
    }

    /**
     * Send a 415 "Unsupported Media Type" to the remote party
     *
     * @param request SIP request
     */
    public void send415Error(SipRequest request) {
        try {
            if (logger.isActivated()) {
                logger.info("Send 415 Unsupported Media Type");
            }
            SipResponse resp = SipMessageFactory.createResponse(request, 415);
            // TODO: set Accept-Encoding header
            getImsService().getImsModule().getSipManager().sendSipResponse(resp);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send 415 error response", e);
            }
        }
    }

    /**
     * Create SDP setup offer (see RFC6135, RFC4145)
     *
     * @return Setup offer
     */
    public String createSetupOffer() {
        if (isBehindNat()) {
            // Active mode by default if there is a NAT
            return "active";
        } else {
            // Active/passive mode is exchanged in order to be compatible
            // with UE not supporting COMEDIA
            return "actpass";
        }
    }

    /**
     * Create SDP setup offer for mobile to mobile (see RFC6135, RFC4145)
     *
     * @return Setup offer
     */
    public String createMobileToMobileSetupOffer() {
        // Always active mode proposed here
        return "active";
    }

    /**
     * Create SDP setup answer (see RFC6135, RFC4145)
     *
     * @param offer setup offer
     * @return Setup answer ("active" or "passive")
     */
    public String createSetupAnswer(String offer) {
        if (offer.equals("actpass")) {
            // Active mode by default if there is a NAT or AS IM
            return "active";
        } else
        if (offer.equals("active")) {
            // Passive mode
            return "passive";
        } else
        if (offer.equals("passive")) {
            // Active mode
            return "active";
        } else {
            // Passive mode by default
            return "passive";
        }
    }

    /**
     * Returns the response timeout (in seconds)
     *
     * @return Timeout
     */
    public int getResponseTimeout() {
        return ringingPeriod + SipManager.TIMEOUT;
    }

    /**
     * Is session interrupted
     *
     * @return Boolean
     */
    public boolean isSessionInterrupted() {
        if(sessionInterrupted){
             if (logger.isActivated()) {
                logger.info("ABC isSessionInterrupted1");
            }
        }
           if(isInterrupted()){
             if (logger.isActivated()) {
                logger.info("ABC isSessionInterrupted2");
            }
        }
            if(getDialogPath() != null && getDialogPath().isSessionTerminated()){
             if (logger.isActivated()) {
                logger.info("ABC isSessionInterrupted3");
            }
        }
        return sessionInterrupted || isInterrupted() ||
                (getDialogPath() != null && getDialogPath().isSessionTerminated());
    }

    /**
     * Is session terminated by remote
     *
     * @return Boolean
     */
    public boolean isSessionTerminatedByRemote() {
        return sessionTerminatedByRemote;
    }

    /**
     * Create an INVITE request
     *
     * @return the INVITE request
     * @throws SipException
     */
    public abstract SipRequest createInvite() throws SipException;

    /**
     * Send INVITE message
     *
     * @param invite SIP INVITE
     * @throws SipException
     */
    public void sendInvite(SipRequest invite) throws SipException {
        // add P-Associated-From
        if (mVirtualId != null && !mVirtualId.isEmpty() && RcsSettings.getInstance().isSupportOP08()) {
            invite.addHeader(SipUtils.HEADER_P_ASSOCIATED_FROM, "sip:" + mVirtualId + "@msg.pc.t-mobile.com;user=phone");
        }

        // Send INVITE request
        SipTransactionContext ctx = getImsService().getImsModule().getSipManager().sendSipMessageAndWait(invite, getResponseTimeout());

        if (logger.isActivated()) {
            logger.info("sendInvite response received timeout:" + getResponseTimeout());
        }

        // Analyze the received response
        if (ctx.isSipResponse()) {
            if (logger.isActivated()) {
                logger.error("sendInvite statuscode:" + ctx.getStatusCode());
            }
            // A response has been received
            if ((ctx.getStatusCode() == 200) || (ctx.getStatusCode() == 202)) {
                // 200 OK
                handle200OK(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 404 || ctx.getStatusCode() == 410) {
                // 404 session not found
                handle404SessionNotFound(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 407) {
                // 407 Proxy Authentication Required
                handle407Authentication(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 422) {
                // 422 Session Interval Too Small
                handle422SessionTooSmall(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 480) {
                // 480 Temporarily Unavailable
                handle480Unavailable(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 481) {
                // 480 Temporarily Unavailable
                handle481TransactionDoesNotExist(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 482) {
                // 480 Temporarily Unavailable
                handle482LoopDetected(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 486) {
                // 486 busy
                handle486Busy(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 487) {
                // 487 Invitation cancelled
                handle487Cancel(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 500) {
                // 480 Temporarily Unavailable
                handle500InternalServerError(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 502) {
                // 480 Temporarily Unavailable
                handle502BadGateway(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 504) {
                // 480 Temporarily Unavailable
                handle504ServerTimeout(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 603) {
                // 603 Invitation declined
                handle603Declined(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 403) {
                // 603 Invitation declined
                handle403Forbidden(ctx.getSipResponse());
            } else
            if (ctx.getStatusCode() == 503) {
                // 503 Invitation declined
                handle503ServiceUnavailable(ctx.getSipResponse());
            }else if (ctx.getStatusCode() == 600) {
                // 600 Invitation declined
                handle600BusyEverywhere(ctx.getSipResponse());
            } else if (ctx.getStatusCode() == 663) {
                // 663 Timer expiry
                handle663TimerExpiry(ctx.getSipResponse());
            } else
            if ((ctx.getStatusCode() == 400 || ctx.getStatusCode() == 408 || ctx.getStatusCode() == 413 ||
                    ctx.getStatusCode() == 421 || ctx.getStatusCode() == 487 || ctx.getStatusCode() == 604 ||
                    ctx.getStatusCode() == 606 || ctx.getStatusCode() == 504) &&
                    RcsSettings.getInstance().isFallbackToPagerModeSupported()) {
                // 603 Invitation declined
                handleOtherNetworkErrors(ctx.getSipResponse());
            } else
                // Other error response
                handleDefaultError(ctx.getSipResponse());
        } else {
            // No response received: timeout
            if(RcsSettings.getInstance().isSupportOP07()){
                handleResponseTimeout(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FALLBACK, "timeout"));
            } else if(RcsSettings.getInstance().isFallbackToPagerModeSupported()){
                handleResponseTimeout(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED, "timeout"));
            } else {
                handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED, "timeout"));
            }
        }
    }

    /**
     * Handle 200 0K response
     *
     * @param resp 200 OK response
     */
    public void handle200OK(SipResponse resp) {
        try {
            // 200 OK received
            if (logger.isActivated()) {
                logger.info("200 OK response received");
            }

            // The signaling is established
            getDialogPath().sigEstablished();

            // Set the remote tag
            getDialogPath().setRemoteTag(resp.getToTag());

            // Set the target
            getDialogPath().setTarget(resp.getContactURI());

            // Set the route path with the Record-Route header
            Vector<String> newRoute = SipUtils.routeProcessing(resp, true);
            getDialogPath().setRoute(newRoute);

            // Set the remote SDP part
            getDialogPath().setRemoteContent(resp.getContent());

            Capabilities capabilities = CapabilityUtils.extractCapabilities(resp);
            if(capabilities != null &&  this instanceof GroupChatSession){
                ((GroupChatSession)this).setMsrpFtSupport(capabilities.isFileTransferSupported());
            }

            // Set the remote SIP instance ID
            ContactHeader remoteContactHeader = (ContactHeader)resp.getHeader(ContactHeader.NAME);
            if (remoteContactHeader != null) {
                getDialogPath().setRemoteSipInstance(remoteContactHeader.getParameter(SipUtils.SIP_INSTANCE_PARAM));
            }

         // Send ACK request
            if (logger.isActivated()) {
                logger.info("Send ACK");
            }

            // Send ack
            Thread t = new Thread() {
                public void run() {
                    // Send ACK request
                    try{
                        getImsService().getImsModule().getSipManager().sendSipAck(getDialogPath());
                    }catch (Exception e){
                        e.printStackTrace();
                    }
                }
            };
            t.start();

            // The session is established
            getDialogPath().sessionEstablished();

            // Prepare Media Session
            prepareMediaSession();

            if(this instanceof FileSharingSession){
                String remoteSdp = resp.getSdpContent();
                ((FileSharingSession)this).extractFileRange(remoteSdp);
            }

            // Start Media Session
            startMediaSession();

            // Notify listeners
            for(int i=0; i < getListeners().size(); i++) {
                getListeners().get(i).handleSessionStarted();
            }

            if (logger.isActivated()) {
                logger.info("After sesion start");
            }

            // Start session timer
            if (getSessionTimerManager().isSessionTimerActivated(resp)) {
                getSessionTimerManager().start(resp.getSessionTimerRefresher(), resp.getSessionTimerExpire());
            }
        } catch(Exception e) {
            // Unexpected error
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }
            if(RcsSettings.getInstance().isFallbackToPagerModeSupported()){
                try {
                    Thread.sleep(10000);
                } catch (InterruptedException e1) {
                    // TODO Auto-generated catch block
                    e1.printStackTrace();
                }
                terminateSession(TERMINATION_BY_SIP_TIMEOUT);
                if (logger.isActivated()) {
                    logger.error("Session initiation has failed after sleep");
                }
                handleError(new ImsServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED, "timeout"));
            } else if(RcsSettings.getInstance().isSupportOP07()){
                handleMsrpConnectionException(e);
            } else {
                handleError(new ImsServiceError(ImsServiceError.UNEXPECTED_EXCEPTION,
                        e.getMessage()));
            }
        }
    }

    /**
     * Handle default error
     *
     * @param resp Error response
     */
    public void handleDefaultError(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("handleDefaultError() entry");
        }
        // Default handle
        handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED,
                resp.getStatusCode() + " " + resp.getReasonPhrase()));
    }

    public void handleMsrpConnectionException(Exception e){
        // Default handle
        handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED, "timeout"));
    }

    /**
     * Handle default error
     *
     * @param resp Error response
     */
    public void handleResponseTimeout(ImsServiceError error) {
        if (logger.isActivated()) {
            logger.error("handleDefaultError() entry");
        }
        handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_FAILED, "timeout"));
    }

    /**
     * Handle default error
     *
     * @param resp Error response
     */
    public void handleOtherNetworkErrors(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("handleOtherNetworkErrors() entry");
        }
    }

    /**
     * M: Added to resolve the rich call 403 error.@{
     */
    /**
     * Handle 403 error. First do re-register then send request again
     *
     * @param request The request was responded with 403
     */
    public void handle403Forbidden(SipResponse resp) {
         if (logger.isActivated()) {
            logger.error("handle403 Forbidden() entry");
        }
        boolean isRegistered = imsService.getImsModule().getCurrentNetworkInterface()
                .getRegistrationManager().registration();
        if (logger.isActivated()) {
            logger.debug("re-register isRegistered: " + isRegistered);
        }
        if (isRegistered) {
            String callId = dialogPath.getCallId();
            SipRequest invite = createSipInvite(callId);
            if (invite != null) {
                try {
                    sendInvite(invite);
                } catch (SipException e) {
                    if (logger.isActivated()) {
                        logger.debug("re send sip request failed.");
                    }
                    e.printStackTrace();
                }

            } else {
                if (logger.isActivated()) {
                    logger.debug("handle403Forbidden() invite is null");
                }
            }
        }
        if (logger.isActivated()) {
            logger.debug("handle403Forbidden() exit");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 404 Session Not Found
     *
     * @param resp 404 response
     */
    public void handle404SessionNotFound(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("404 handle404 SessionNotFound");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 500 Internal Server Error
     *
     * @param resp 500 response
     */
    public void handle500InternalServerError(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("500 handle Internal Server Error");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 482 Loop Detected
     *
     * @param resp 482 response
     */
    public void handle482LoopDetected(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("482 Loop Detected");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 502 Bas Gateway
     *
     * @param resp 502 response
     */
    public void handle502BadGateway(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("502 handle Bad gateway");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 504 Internal Server Error
     *
     * @param resp 500 response
     */
    public void handle504ServerTimeout(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("504 handle  Server Timeout");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 407 Proxy Authentication Required
     *
     * @param resp 407 response
     */
    public void handle407Authentication(SipResponse resp) {
        try {
            if (logger.isActivated()) {
                logger.info("407 response received");
            }

            // Set the remote tag
            getDialogPath().setRemoteTag(resp.getToTag());

            // Update the authentication agent
            getAuthenticationAgent().readProxyAuthenticateHeader(resp);

            // Increment the Cseq number of the dialog path
            getDialogPath().incrementCseq();

            // Create the invite request
            SipRequest invite = createInvite();

            // Reset initial request in the dialog path
            getDialogPath().setInvite(invite);

            // Set the Proxy-Authorization header
            getAuthenticationAgent().setProxyAuthorizationHeader(invite);

            // Send INVITE request
            sendInvite(invite);

        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }

            // Unexpected error
            handleError(new ImsServiceError(ImsServiceError.UNEXPECTED_EXCEPTION,
                    e.getMessage()));
        }
    }

    /**
     * Handle 422 response
     *
     * @param resp 422 response
     */
    public void handle422SessionTooSmall(SipResponse resp) {
        try {
            // 422 response received
            if (logger.isActivated()) {
                logger.error("422 response received");
            }

            // Extract the Min-SE value
            int minExpire = SipUtils.getMinSessionExpirePeriod(resp);
            if (minExpire == -1) {
                if (logger.isActivated()) {
                    logger.error("Can't read the Min-SE value");
                }
                handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.UNEXPECTED_EXCEPTION, "No Min-SE value found"));
                return;
            }

            // Set the min expire value
            getDialogPath().setMinSessionExpireTime(minExpire);

            // Set the expire value
            getDialogPath().setSessionExpireTime(minExpire);

            // Increment the Cseq number of the dialog path
            getDialogPath().incrementCseq();

            // Create a new INVITE with the right expire period
            if (logger.isActivated()) {
                logger.info("Send new INVITE");
            }
            SipRequest invite = createInvite();

            // Set the Authorization header
            getAuthenticationAgent().setAuthorizationHeader(invite);

            // Reset initial request in the dialog path
            getDialogPath().setInvite(invite);

            // Send INVITE request
            sendInvite(invite);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Session initiation has failed", e);
            }

            // Unexpected error
            handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.UNEXPECTED_EXCEPTION,
                    e.getMessage()));
        }
    }

protected SipRequest createSipInvite(String callId) {
        logger.debug("ImsServiceSession::createSipInvite(), do nothing in the parent class");
        return null;
    }

    protected SipRequest createSipInvite(String callId, boolean isFocus) {
        logger.debug("ImsServiceSession::createSipInvite(), do nothing in the parent class");
        return null;
    }

    /**
     * Handle 480 Temporarily Unavailable
     *
     * @param resp 480 response
     */
    public void handle480Unavailable(SipResponse resp) {
        handleDefaultError(resp);
    }

    /**
     * Handle 481 transaction does not exist
     *
     * @param resp 481 response
     */
    public void handle481TransactionDoesNotExist(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("481 transactiond does not exist response received");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 481 transaction does not exist
     *
     * @param resp 481 response
     */
    public void handle503ServiceUnavailable(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("503 Service unavailable response received");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 481 transaction does not exist
     *
     * @param resp 481 response
     */
    public void handle600BusyEverywhere(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("600 busy every where");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 66 Timer Expiry
     *
     * @param resp 663 response
     */
    public void handle663TimerExpiry(SipResponse resp) {
        if (logger.isActivated()) {
            logger.error("663 timer expiry");
        }
        handleDefaultError(resp);
    }

    /**
     * Handle 486 Busy
     *
     * @param resp 486 response
     */
    public void handle486Busy(SipResponse resp) {
        handleDefaultError(resp);
    }

    /**
     * Handle 487 Cancel
     *
     * @param resp 487 response
     */
    public void handle487Cancel(SipResponse resp) {
        handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_CANCELLED,
                resp.getStatusCode() + " " + resp.getReasonPhrase()));
    }

    /**
     * Handle 603 Decline
     *
     * @param resp 603 response
     */
    public void handle603Declined(SipResponse resp) {
        handleError(new ImsSessionBasedServiceError(ImsSessionBasedServiceError.SESSION_INITIATION_DECLINED,
                resp.getStatusCode() + " " + resp.getReasonPhrase()));
    }

    /**
     * Handle Error
     *
     * @param error ImsServiceError
     */
    public abstract void handleError(ImsServiceError error);

    /**
     * Handle Error
     *
     * @param error ImsServiceError
     */
    //public abstract void handleInviteError(ImsServiceError error);

    /**
     * Handle ReInvite Sip Response
     *
     * @param response Sip response to reInvite
     * @param int code response code
     * @param reInvite reInvite SIP request
     */
    public void handleReInviteResponse(int  code, SipResponse response, int requestType) {
    }

    /**
     * Handle User Answer in Response to Session Update notification
     *
     * @param int code response code
     * @param reInvite reInvite SIP request
     */
    public void handleReInviteUserAnswer(int  code, int requestType) {
    }

    /**
     * Handle ACK sent in Response to 200Ok ReInvite
     *
     * @param int code response code
     * @param reInvite reInvite SIP request
     */
    public void handleReInviteAck(int  code, int requestType) {
    }

    /**
     * Handle 407 Proxy Authent error ReInvite Response
     *
     * @param response reInvite SIP response
     * @param int requestType  service context of reInvite
     */
    public void handleReInvite407ProxyAuthent(SipResponse response, int serviceContext){
    }

    public String buildReInviteSdpResponse(SipRequest ReInvite, int serviceContext){
        return null;
    }

    /**
     * This class used to receive broadcasts from RCS Core Service.
    */
    private class ApiReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(final Context context, final Intent intent) {
            new AsyncTask<Void, Void, Void>() {
                @Override
                protected Void doInBackground(Void... params) {
                    logger.debug("Receive intent");
                    String action = intent.getAction();
                    if (action == null) {
                        logger.debug("Receive intent action is null");
                        return null;
                    }

                    if (INTENT_RINGING.equalsIgnoreCase(action)) {
                        logger.debug("Receive intent setRingingRecieved");
                        setRingingRecieved();
                    }
                    if (INTENT_TRYING.equalsIgnoreCase(action)) {
                        logger.debug("Receive intent setTryingRecieved");
                        setTryingRecieved();
                    }
                    return null;
                }
            } .execute();
        }
    }

}
