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
package com.orangelabs.rcs.service.api;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Vector;

import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMode;
import com.gsma.services.rcs.ICommonServiceConfiguration;
import com.gsma.services.rcs.IRcsServiceRegistrationListener;
import com.gsma.services.rcs.RcsService;
//import com.gsma.services.rcs.RcsService.Build.VERSION_CODES;
import com.gsma.services.rcs.Build;
import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.RcsServiceRegistration;
import com.gsma.services.rcs.chat.GroupChat;
import com.gsma.services.rcs.chat.OneToOneChatIntent;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.ft.FileTransfer.Disposition;
import com.gsma.services.rcs.ft.FileTransfer.ReasonCode;
import com.gsma.services.rcs.ft.FileTransfer.State;
import com.gsma.services.rcs.ft.IFileTransfer;
import com.gsma.services.rcs.ft.IFileTransferService;
import com.gsma.services.rcs.ft.IFileTransferServiceConfiguration;
import com.gsma.services.rcs.ft.IGroupFileTransferListener;
import com.gsma.services.rcs.ft.IOneToOneFileTransferListener;
import com.gsma.services.rcs.ft.INewFileTransferListener;
import com.gsma.services.rcs.ft.FileTransferLog;
import com.gsma.services.rcs.ft.FileTransferIntent;
import com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo;
import com.gsma.services.rcs.RcsService.Direction;

import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.content.Context;
import android.text.TextUtils;

import com.orangelabs.rcs.service.broadcaster.OneToOneFileTransferBroadcaster;
import com.orangelabs.rcs.service.broadcaster.GroupFileTransferBroadcaster;
import com.orangelabs.rcs.provider.messaging.FileTransferData;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.content.ContentManager;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaAttribute;
import com.orangelabs.rcs.core.ims.protocol.sdp.MediaDescription;
import com.orangelabs.rcs.core.ims.protocol.sdp.SdpParser;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.ChatUtils;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatInfo;
import com.orangelabs.rcs.core.ims.service.im.chat.ListOfParticipant;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.service.api.ChatServiceImpl;
import com.orangelabs.rcs.core.ims.service.im.chat.cpim.CpimMessage;
import com.orangelabs.rcs.core.ims.service.im.chat.imdn.ImdnDocument;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.OriginatingExtendedFileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.TerminatingFileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.HttpFileTransferSession;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.platform.file.FileDescription;
import com.orangelabs.rcs.platform.file.FileFactory;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.messaging.FileTransferData;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.DateUtils;
import com.orangelabs.rcs.utils.FileUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.TerminatingGroupFileSharingSession;
import com.orangelabs.rcs.provider.settings.RcsSettingsData.FileTransferProtocol;
import com.orangelabs.rcs.provider.messaging.FileTransferPersistedStorageAccessor;
import com.orangelabs.rcs.service.api.ServerApiBaseException;
import com.orangelabs.rcs.service.api.ServerApiGenericException;
import com.orangelabs.rcs.service.api.ServerApiIllegalArgumentException;

/**
 * File transfer service implementation
 *
 * @author Jean-Marc AUFFRET
 */
public class FileTransferServiceImpl extends IFileTransferService.Stub {
    /**
     * List of service event listeners
     */
    private RemoteCallbackList<IRcsServiceRegistrationListener> serviceListeners =
            new RemoteCallbackList<IRcsServiceRegistrationListener>();


    private static Map<String, FileTransferImpl> ftSessions = new HashMap<>();

    private static Map<String, GroupFileTransferImpl> groupFtSessions = new HashMap<>();

    private final OneToOneFileTransferBroadcaster mOneToOneFileTransferBroadcaster = new OneToOneFileTransferBroadcaster();

    private final GroupFileTransferBroadcaster mGroupFileTransferBroadcaster = new GroupFileTransferBroadcaster();

    private final RcsSettings mRcsSettings = RcsSettings.getInstance();

    private  ChatServiceImpl mChatService = null;

    private InstantMessagingService mImsService;

    private RichMessagingHistory mMessagingLog = RichMessagingHistory.getInstance();

    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger(FileTransferServiceImpl.class.getName());

    /**
     * Lock used for synchronization
     */
    private Object lock = new Object();

    public static ArrayList<PauseResumeFileObject> mFileTransferPauseResumeData =
            new ArrayList<PauseResumeFileObject>();

    /**
     * Constructor
     */
    public FileTransferServiceImpl() {
        mImsService = Core.getInstance().getImService();
        mMessagingLog = RichMessagingHistory.getInstance();
    }

    /**
     * Constructor
     */
    public FileTransferServiceImpl(ChatServiceImpl chatService) {
        if (logger.isActivated()) {
            logger.info("File transfer service API is loaded");
        }
        this.mChatService = chatService;
        mImsService = Core.getInstance().getImService();
        mMessagingLog = RichMessagingHistory.getInstance();
    }

    /**
     * Close API
     */
    public void close() {
        // Clear list of sessions
        ftSessions.clear();
        groupFtSessions.clear();

        if (logger.isActivated()) {
            logger.info("File transfer service API is closed");
        }
    }

    /**
     * Add a file transfer session in the list
     *
     * @param session File transfer session
     */
    protected static void addFileTransferSession(FileTransferImpl session) {
        if (logger.isActivated()) {
            logger.debug("FTS Add a file transfer session in the list (size="
                    + ftSessions.size() + ")" + " Id: " + session.getTransferId());
        }

        ftSessions.put(session.getTransferId(), session);
    }

    /**
     * Add a file transfer session in the list
     *
     * @param session File transfer session
     */
    protected static void addFileTransferSession(GroupFileTransferImpl session) {
        if (logger.isActivated()) {
            logger.debug("FTS Add group file transfer session in the list (size="
                    + groupFtSessions.size() + ")" + " Id: " + session.getTransferId());
        }

        groupFtSessions.put(session.getTransferId(), session);
    }

    /**
     * Remove a file transfer session from the list
     *
     * @param sessionId Session ID
     */
    protected static void removeFileTransferSession(String sessionId) {
        if (logger.isActivated()) {
            logger.debug("FTS Remove a file transfer session from the list (size="
                    + ftSessions.size() + ")" + " Id: " + sessionId);
        }

        ftSessions.remove(sessionId);
    }

    /**
     * Remove a file transfer session from the list
     *
     * @param sessionId Session ID
     */
    protected static void removeGroupFileTransferSession(String sessionId) {
        if (logger.isActivated()) {
            logger.debug("FTS Remove group file transfer session from the list (size="
                    + groupFtSessions.size() + ")" + " Id: " + sessionId);
        }

        groupFtSessions.remove(sessionId);
    }

    /**
     * Returns true if the service is registered to the platform, else returns false
     *
     * @return Returns true if registered else returns false
     */
    public boolean isServiceRegistered() {
        return ServerApiUtils.isImsConnected();
    }

    /**
     *
     * @param listener Service registration listener
     */
    public void addEventListener(IRcsServiceRegistrationListener listener) {
        synchronized (lock) {
            if (logger.isActivated()) {
                logger.info("FTS Add a service listener");
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
        synchronized (lock) {
            if (logger.isActivated()) {
                logger.info("FTS Remove a service listener");
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
    }

    /**
     * Receive a new file transfer invitation
     *
     * @param session File transfer session
     * @param isGroup is group file transfer
     */
    public void receiveHttpFileTransferInvitation(String remote, FileSharingSession session,
            boolean isGroup) {
        if (logger.isActivated()) {
            logger.info("FTS Receive httpfile transfer invitation from " + remote + " isGroup "
                    + isGroup);
        }
        String chatId = null;
        int timeLen = session.getTimeLen();
        if (session instanceof HttpFileTransferSession) {
            chatId = ((HttpFileTransferSession) session).getContributionID();
            if (logger.isActivated()) {
                logger.info("FTS Receive HTTP file transfer invitation chatid: " + chatId);
            }
        } else if (isGroup) {
            chatId = ((TerminatingGroupFileSharingSession) session).getGroupChatSession()
                    .getContributionID();
            if (logger.isActivated()) {
                logger.info("FTS Receive group MSRP file transfer invitation chatid: " + chatId);
            }
        }

        // Extract number from contact
        String number = remote;

        // Add session in the list
        FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                session.getSessionID(), mMessagingLog);

        FileTransferImpl sessionApi = null;
        GroupFileTransferImpl groupSessionApi = null;
        if(!isGroup){
            sessionApi = new FileTransferImpl(session,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            FileTransferServiceImpl.addFileTransferSession(sessionApi);
            sessionApi.onSessionInvited();
        } else {
            groupSessionApi = new GroupFileTransferImpl(session,persistedStorage,mGroupFileTransferBroadcaster,chatId,this);
            FileTransferServiceImpl.addFileTransferSession(groupSessionApi);
            groupSessionApi.onSessionInvited();
        }


        /*if (isGroup) {
            RichMessagingHistory.getInstance().addIncomingGroupFileTransfer(chatId, number,
                    session.getSessionID(), session.getContent());
        } else {
            // Update rich messaging history
            RichMessagingHistory.getInstance().addFileTransfer(
                    number, session.getSessionID(),
                    Direction.INCOMING.toInt(), session.getContent());
        }*/

        RichMessagingHistory.getInstance().updateFileIcon(
                session.getSessionID(), session.getThumbUrl());


        if (RcsSettings.getInstance().isCPMSupported()) {
            if (!(RichMessagingHistory.getInstance().getCoversationID(
                    session.getRemoteContact(), 1).equals(session.getConversationID()))) {
                if (session.getConversationID() != null) {
                    if (logger.isActivated()) {
                        logger.info("FTS receiveHttpFileTransferInvitation  OldId: "
                                + RichMessagingHistory.getInstance().getCoversationID(
                                        session.getRemoteContact(), 1) + " NewId: "
                                + session.getConversationID());
                    }
                    RichMessagingHistory.getInstance().UpdateCoversationID(
                            session.getRemoteContact(), session.getConversationID(), 1);
                } else {
                    if (logger.isActivated()) {
                        logger.info("FTS receiveHttpFileTransferInvitation"
                                + " conversation id is null");
                    }
                }
            }
        }

        /*// Broadcast intent related to the received invitation
        Intent intent = new Intent(FileTransferIntent.ACTION_NEW_INVITATION);
        intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        intent.putExtra(FileTransferIntent.EXTRA_CONTACT, number);
        intent.putExtra(FileTransferIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
        intent.putExtra(FileTransferIntent.EXTRA_TRANSFER_ID, session.getSessionID());
        intent.putExtra(FileTransferIntent.EXTRA_FILENAME, session.getContent().getName());
        intent.putExtra(FileTransferIntent.EXTRA_FILESIZE, session.getContent().getSize());
        intent.putExtra(FileTransferIntent.EXTRA_FILETYPE, session.getContent().getEncoding());
        intent.putExtra(FileTransferIntent.TIME_LEN, session.getTimeLen());
        intent.putExtra(FileTransferIntent.GEOLOC_FILE, session.isGeoLocFile());
        *//** M: ftAutAccept @{ *//*
        intent.putExtra("autoAccept", session.shouldAutoAccept());
        *//** @} *//*
        String chatSessionId = null;
        if (session instanceof HttpFileTransferSession) {
            intent.putExtra("chatSessionId", ((HttpFileTransferSession)session).getChatSessionID());
            if (isGroup) {
                intent.putExtra("chatId", chatId);
            }
            intent.putExtra("isGroupTransfer", isGroup);
        } else if (isGroup) {

            chatSessionId = ((TerminatingGroupFileSharingSession) session).getGroupChatSession()
                    .getSessionID();
            if (logger.isActivated()) {
                logger.info("FTS Receive file transfer invitation: " + "Chatsessionid: "
                        + chatSessionId);
            }
            intent.putExtra("chatSessionId", chatSessionId);
            intent.putExtra("chatId", chatId);
            intent.putExtra("isGroupTransfer", isGroup);
        }
        AndroidFactory.getApplicationContext().sendBroadcast(intent);*/

        /*// Notify file transfer invitation listeners
        synchronized (lock) {
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("FTS Receive HTTP file transfer invitation N: " + N);
            }
            for (int i = 0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onNewFileTransfer(session.getSessionID());
                    listeners.getBroadcastItem(i).onNewFileTransferReceived(
                            session.getSessionID(),
                            session.shouldAutoAccept(),
                            isGroup, chatSessionId, chatId, timeLen);
                } catch (Exception e) {
                    if (logger.isActivated()) {
                        logger.error("FTS Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();
        }*/
    }

    /**
     * Receive a new file transfer invitation
     *
     * @param session File transfer session
     * @param isGroup is group file transfer
     */
    public void receiveFileTransferInvitation(FileSharingSession session, boolean isGroup) {
        if (logger.isActivated()) {
            logger.info("Receive file transfer invitation from " + session.getRemoteContact()
                    + " Timelen " + session.getTimeLen());
        }
        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
        String chatId = null;

        boolean isBurn = false;
        isBurn = session.isBurnMessage();

        boolean isPublicAccountFile = session.isPublicChatFile();

        int timeLen = session.getTimeLen();
        if (session instanceof HttpFileTransferSession) {
            chatId = ((HttpFileTransferSession) session).getContributionID();
            if (logger.isActivated()) {
                logger.info("Receive HTTP file transfer invitation chatid: " + chatId);
            }
        } else if (isGroup) {
            GroupChatSession parentSession =
                ((TerminatingGroupFileSharingSession) session).getGroupChatSession();
            if (parentSession != null) {
                chatId = parentSession.getContributionID();
            } else {
                //String rejoinId = SipUtils.getAssertedIdentity(session.getDialogPath().getInvite());
                //the string format of p-a-i is <...> not a uri use from header:
                //and for the group ft offline the target does not equals from.
                String rejoinId = session.getDialogPath().getInvite().getFromUri();
                chatId = rmHistory.getChatIdbyRejoinId(rejoinId);
            }
            if (logger.isActivated()) {
                logger.info("Receive group MSRP file transfer invitation chatid: " + chatId);
            }
		} else {
			chatId = session.getRemoteContact();
			if (logger.isActivated()) {
				logger.info("Receive O2O MSRP file transfer invitation chatid: " + chatId);
			}
        }
        String transferId = session.getSessionID();

        // Add session in the list
        FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                transferId, mMessagingLog);

        FileTransferImpl sessionApi = null;
        GroupFileTransferImpl groupSessionApi = null;
        if(!isGroup){
            sessionApi = new FileTransferImpl(session,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            FileTransferServiceImpl.addFileTransferSession(sessionApi);
            sessionApi.onSessionInvited();
        } else {
            groupSessionApi = new GroupFileTransferImpl(session,persistedStorage,mGroupFileTransferBroadcaster,chatId,this);
            FileTransferServiceImpl.addFileTransferSession(groupSessionApi);
            groupSessionApi.onSessionInvited();
        }

        String number = session.getRemoteContact();
        int sessionType = FileTransferLog.Type.CHAT;
        if (session.isPublicChatFile()) {
            sessionType = FileTransferLog.Type.PUBLIC;
        } else if (isBurn) {
            sessionType = FileTransferLog.Type.BURN;
        }

        /*rmHistory.addExtendedFileTransfer(
                chatId,
                number,
                transferId,
                Direction.INCOMING.toInt(),
                sessionType,
                session.getContent());*/
        rmHistory.updateFileTransferMsgId(transferId, session.getMessageId());
        if(session instanceof TerminatingFileSharingSession && !((TerminatingFileSharingSession)session).isDisplayRequested()){
            rmHistory.updateFileTransferStatus(transferId, FileTransfer.State.DISPLAYED.toInt());
        }
        if (session.getTimeLen() > 0) {
            rmHistory.updateFileTransferDuration(transferId, session.getTimeLen());
        }
        rmHistory.updateFileIcon(transferId, session.getThumbUrl());

        if (RcsSettings.getInstance().supportOP01()) {
            String remoteSdp = session.getDialogPath().getInvite().getSdpContent();
            SdpParser parser = new SdpParser(remoteSdp.getBytes());
            Vector<MediaDescription> media = parser.getMediaDescriptions();
            MediaDescription mediaDescription = media.elementAt(0);
            MediaAttribute attrftid = mediaDescription.getMediaAttribute("file-transfer-id");
            if (attrftid != null) {
                RichMessagingHistory.getInstance().updateFileId(session.getSessionID(),
                        attrftid.getValue());
            }
            if (logger.isActivated()) {
                logger.debug("file-transfer-id: " + attrftid.getValue());
            }
        }

        try {
            if (RcsSettings.getInstance().isCPMSupported() && isGroup) {
                if (!(rmHistory.getCoversationID(session.getRemoteContact(), 1).equals(
                        session.getConversationID()))) {
                    if (session.getConversationID() != null) {
                        if (logger.isActivated()) {
                            logger.info("FTS receiveFileTransferInvitation " +
                                    " OldId: " + rmHistory.getCoversationID(session.getRemoteContact(), 1) +
                                    " NewId: " + session.getConversationID());
                        }
                        rmHistory.UpdateCoversationID(session.getRemoteContact(), session.getConversationID(), 1);
                    } else {
                        if (logger.isActivated()) {
                            logger.info("FTS receiveFileTransferInvitation  Conversation Id is null");
                        }
                    }
                }
            }
        } catch(Exception e){
            e.printStackTrace();
        }

        // addFileTransferPauseResumeData(
        // session.getSessionID(),session.getContent().getUrl(),session.getContent().getSize(),0,
        // session.getRemoteContact(),session.getHashselector());

        // Broadcast intent related to the received invitation
        Intent intent = new Intent(FileTransferIntent.ACTION_NEW_INVITATION);
        /*intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        intent.putExtra(FileTransferIntent.EXTRA_CONTACT, number);
        intent.putExtra(FileTransferIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
        intent.putExtra(FileTransferIntent.EXTRA_TRANSFER_ID, session.getSessionID());
        intent.putExtra(FileTransferIntent.EXTRA_FILENAME, session.getContent().getName());
        intent.putExtra(FileTransferIntent.EXTRA_FILESIZE, session.getContent().getSize());
        intent.putExtra(FileTransferIntent.EXTRA_FILETYPE, session.getContent().getEncoding());
        intent.putExtra(FileTransferIntent.TIME_LEN, session.getTimeLen());
        intent.putExtra(FileTransferIntent.GEOLOC_FILE, session.isGeoLocFile());
        intent.putExtra(FileTransferIntent.EXTRA_MSG_ID, session.getMessageId());
        if (isBurn)
            intent.putExtra(FileTransferIntent.EXTRA_TRANSFERTYPE, FileTransfer.Type.BURNED);
        else
        if (isPublicAccountFile)
            intent.putExtra(FileTransferIntent.EXTRA_TRANSFERTYPE, FileTransfer.Type.PUBACCOUNT);
        else
            intent.putExtra(FileTransferIntent.EXTRA_TRANSFERTYPE, FileTransfer.Type.NORMAL);

        String chatSessionId = null;
        *//** M: ftAutAccept @{ *//*
        intent.putExtra("autoAccept", session.shouldAutoAccept());
        *//** @} *//*
        if (session instanceof HttpFileTransferSession) {
            chatSessionId = ((HttpFileTransferSession) session).getChatSessionID();
            intent.putExtra("chatSessionId", chatSessionId);
            if (isGroup) {
                intent.putExtra("chatId", chatId);
            }
            intent.putExtra("isGroupTransfer", isGroup);
        } else if (isGroup) {
            //not happened now.
            GroupChatSession parentSession =
                ((TerminatingGroupFileSharingSession) session).getGroupChatSession();
            if (parentSession != null) {
                chatSessionId = parentSession.getContributionID();
            } else {
                String rejoinId = session.getDialogPath().getTarget();
                chatSessionId = RichMessagingHistory.getInstance().getChatIdbyRejoinId(rejoinId);
            }
            if (logger.isActivated()) {
                logger.info("FTS Receive file transfer invitation: " + "Chatsessionid: "
                        + chatSessionId);
            }
            intent.putExtra("chatSessionId", chatSessionId);
            intent.putExtra("chatId", chatId);
            intent.putExtra("isGroupTransfer", isGroup);
        } else {
            if (logger.isActivated()) {
                logger.info("FTS Receive file transfer invitation: It is not group");
            }
        }*/
        //AndroidFactory.getApplicationContext().sendBroadcast(intent);

       /* // Notify file transfer invitation listeners
        synchronized (lock) {
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("FTS receiveFileTransferInvitation N: " + N);
            }
            for (int i = 0; i < N; i++) {
                try {
                    if (isPublicAccountFile) {
                        listeners.getBroadcastItem(i).onNewPublicAccountChatFile(
                                session.getSessionID(), session.shouldAutoAccept(), isGroup,
                                chatSessionId, chatId);
                    } else {
                        if (isBurn) {

                            listeners.getBroadcastItem(i).onNewBurnFileTransfer(
                                    session.getSessionID(), isGroup, chatSessionId, chatId);
                        } else {
                            listeners.getBroadcastItem(i).onNewFileTransfer(session.getSessionID());
                            listeners.getBroadcastItem(i).onNewFileTransferReceived(
                                    session.getSessionID(),
                                    session.shouldAutoAccept(),
                                    isGroup, chatSessionId, chatId, timeLen);
                        }
                    }

                } catch (Exception e) {
                    if (logger.isActivated()) {
                        logger.error("FTS Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();
        }*/
    }

    /**
     * Receive a new file transfer invitation
     *
     * @param session File transfer session
     * @param isGroup is group file transfer
     */
    public void receiveResumeFileTransferInvitation(
            FileSharingSession session, boolean isGroup, String fileTransferId) {
        if (logger.isActivated()) {
            logger.info("Resume file transfer invitation from " + session.getRemoteContact());
        }
        String chatId = null;
        if (session instanceof HttpFileTransferSession) {
            chatId = ((HttpFileTransferSession) session).getContributionID();
            if (logger.isActivated()) {
                logger.info("Receive Resume HTTP file transfer invitation chatid: " + chatId);
            }
        } else if (isGroup) {
            chatId = ((TerminatingGroupFileSharingSession) session).getGroupChatSession()
                    .getContributionID();
            if (logger.isActivated()) {
                logger.info("Receive Resume group MSRP file transfer invitation chatid: "
                        + chatId);
            }
        }

        // Extract number from contact
        String number = PhoneUtils.extractNumberFromUri(session.getRemoteContact());

        // Update rich messaging history -- Deepak
        // RichMessagingHistory.getInstance().addFileTransfer(number,
        // session.getSessionID(), FileTransfer.Direction.INCOMING, session.getContent());

        // Add session in the list
        FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                session.getSessionID(), mMessagingLog);

        FileTransferImpl sessionApi = null;
        GroupFileTransferImpl groupSessionApi = null;
        if(!isGroup){
            sessionApi = new FileTransferImpl(session,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            FileTransferServiceImpl.addFileTransferSession(sessionApi);
            sessionApi.onSessionInvited();
        } else {
            groupSessionApi = new GroupFileTransferImpl(session,persistedStorage,mGroupFileTransferBroadcaster,chatId,this);
            FileTransferServiceImpl.addFileTransferSession(groupSessionApi);
            groupSessionApi.onSessionInvited();
        }

        // Broadcast intent related to the received invitation
        Intent intent = new Intent(FileTransferIntent.ACTION_RESUME_FILE);
        intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        intent.putExtra(FileTransferIntent.EXTRA_CONTACT, number);
        intent.putExtra(FileTransferIntent.EXTRA_DISPLAY_NAME, session.getRemoteDisplayName());
        intent.putExtra(FileTransferIntent.EXTRA_TRANSFER_ID, session.getSessionID());
        intent.putExtra(FileTransferIntent.RESUMED_TRANSFER_ID, fileTransferId);
        intent.putExtra(FileTransferIntent.EXTRA_FILENAME, session.getContent().getName());
        intent.putExtra(FileTransferIntent.EXTRA_FILESIZE, session.getContent().getSize());
        intent.putExtra(FileTransferIntent.EXTRA_FILETYPE, session.getContent().getEncoding());

        /** @} */
        if (session instanceof HttpFileTransferSession) {
            intent.putExtra(
                    "chatSessionId", ((HttpFileTransferSession) session).getChatSessionID());
            if (isGroup) {
                intent.putExtra("chatId", chatId);
            }
            intent.putExtra("isGroupTransfer", isGroup);
        } else if (isGroup) {

            String chatSessionId = ((TerminatingGroupFileSharingSession) session)
                    .getGroupChatSession().getSessionID();
            if (logger.isActivated()) {
                logger.info("Resume file transfer invitation: " + "Chatsessionid: "
                        + chatSessionId);
            }
            intent.putExtra("chatSessionId", chatSessionId);
            intent.putExtra("chatId", chatId);
            intent.putExtra("isGroupTransfer", isGroup);
        }
        AndroidFactory.getApplicationContext().sendBroadcast(intent,
                                                             "com.gsma.services.permission.RCS");
        // Notify file transfer invitation listeners about resume File
        /*
         * synchronized(lock) { final int N = listeners.beginBroadcast(); for (int i=0; i
         * < N; i++) { try {
         * listeners.getBroadcastItem(i).onNewFileTransfer(session.getSessionID()); }
         * catch(Exception e) { if (logger.isActivated()) {
         * logger.error("Can't notify listener", e); } } } listeners.finishBroadcast(); }
         */
    }

    /**
     * Receive a new HTTP file transfer invitation outside of an existing chat session
     *
     * @param session File transfer session
     */
    public void receiveFileTransferInvitation(
            FileSharingSession session, ChatSession chatSession) {

        if (logger.isActivated()) {
            logger.info("FTS Receive file transfer invitation from1 "
                    + session.getRemoteContact());
        }
        // Update rich messaging history
        if (chatSession.isGroupChat()) {
            // RichMessagingHistory.getInstance().updateFileTransferChatId(
            // chatSession.getFirstMessage().getMessageId(),
            // chatSession.getContributionID());
        }
        // Add session in the list

        //FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
        //        session.getSessionID(), mMessagingLog);

        //FileTransferImpl sessionApi = new FileTransferImpl(session,persistedStorage);
        //addFileTransferSession(sessionApi);

        // Display invitation
        receiveFileTransferInvitation(session, chatSession.isGroupChat());
        // Display invitation
        /*
         * TODO receiveFileTransferInvitation(session, chatSession.isGroupChat());
         *
         * // Update rich messaging history
         * RichMessaging.getInstance().addIncomingChatSessionByFtHttp(chatSession);
         *
         * // Add session in the list ImSession sessionApi = new ImSession(chatSession);
         * MessagingApiService.addChatSession(sessionApi);
         */
    }

    public PauseResumeFileObject getPauseInfo(String transferId) {
        PauseResumeFileObject object = null;
        for (int j = 0; j < mFileTransferPauseResumeData.size(); j++) {
            object = FileTransferServiceImpl.mFileTransferPauseResumeData.get(j);
            if (object.mFileTransferId.equals(transferId)) {
                if (logger.isActivated()) {
                    logger.info("getPauseInfo, FileTransfer Id & currentBytesTransferred"
                            + object.mFileTransferId);
                }
                return object;
            }
        }
        if (logger.isActivated()) {
            logger.info("getPauseInfo , Check databse");
        }
        object = RichMessagingHistory.getInstance().getPauseInfo(transferId);
        if (object != null && object.mFileTransferId != null) {
            mFileTransferPauseResumeData.add(object);
        }
        if (logger.isActivated()) {
            logger.info("getPauseInfo , After check databse" + object);
        }
        return object;
    }

    public static PauseResumeFileObject getHashPauseInfo(String hashSelector) {
        try {
            if (logger.isActivated()) {
                logger.info("getHashPauseInfo, entry hashSelector " + hashSelector + "&size =" + mFileTransferPauseResumeData.size());
            }
            for (int j = 0; j < mFileTransferPauseResumeData.size(); j++) {
                PauseResumeFileObject object = FileTransferServiceImpl.mFileTransferPauseResumeData
                        .get(j);
                if (logger.isActivated()) {
                    logger.info("getPauseInfo, hashSelector Id & object.hashSelector"
                            + object.hashSelector);
                }
                if (object.hashSelector.equals(hashSelector)) {
                    if (logger.isActivated()) {
                        logger.info("getPauseInfo, hashSelector Id & currentBytesTransferred"
                                + object.hashSelector);
                    }
                    return object;
                }
            }
            if (logger.isActivated()) {
                logger.info("getPauseInfo , Return null");
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return null;
    }

    @Override
    public IFileTransferServiceConfiguration getConfiguration() {
        return new FileTransferServiceConfigurationImpl(mRcsSettings);
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

    public int getServiceRegistrationReasonCode(){
        return 0;
    }

    public boolean isAllowedToTransferFile(ContactId contact){
        if (contact == null) {
            return false;
        }
        try {
            Capabilities remoteCapabilities = ContactsManager.getInstance().getContactCapabilities(contact.toString());
            if (remoteCapabilities == null) {
                if (logger.isActivated()) {
                    logger.debug("Cannot transfer file as the capabilities of contact " + contact
                            + " are not known.");
                }
                return false;
            }
            FileTransferProtocol protocol = getFileTransferProtocolForOneToOneFileTransfer(contact);
            if (protocol == null) {
                if (logger.isActivated()) {
                    logger.debug("Cannot transfer file as no valid file transfer protocol could be determined.");
                }
                return false;
            }
            /*MessagingMode mode = mRcsSettings.getMessagingMode();
            switch (mode) {
                case INTEGRATED:
                case SEAMLESS:
                    if ((FileTransferProtocol.MSRP == protocol && mRcsSettings.isFtAlwaysOn())
                            || (FileTransferProtocol.HTTP == protocol && mRcsSettings
                                    .isFtHttpCapAlwaysOn())) {
                        break;
                    }
                    if (!mImService.isCapabilitiesValid(remoteCapabilities)) {
                        if (sLogger.isActivated()) {
                            sLogger.debug("Cannot transfer file as the cached capabilities of contact "
                                    + contact
                                    + " are not valid anymore for one-to-one communication.");
                        }
                        return false;
                    }
                    break;
                default:
                    break;
            }*/
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.debug("isAllowedToTransferFile exception");
            }
            return false;
        }
    }

    public boolean isAllowedToTransferFileToGroupChat(String chatId){
        if (TextUtils.isEmpty(chatId)) {
            return false;
        }
        try {
            if (!mRcsSettings.isGroupChatActivated()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot transfer file to group chat with group chat Id '"
                            + chatId + "' as group chat feature is not supported.");
                }
                return false;
            }
            if (!mRcsSettings.getMyCapabilities().isFileTransferHttpSupported()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot transfer file to group chat with group chat Id '"
                            + chatId + "' as FT over HTTP capabilities are not supported for self.");
                }
                return false;
            }
            if (mChatService.isGroupChatAbandoned(chatId)) {
                if (logger.isActivated()) {
                    logger.debug("Cannot transfer file to group chat with group chat Id '"
                            + chatId
                            + "' as the group chat is abandoned and can no more be used to send or receive messages.");
                }
                return false;
            }
            GroupChatSession session = mImsService.getGroupChatSession(chatId);
            if (session == null) {
                GroupChatInfo groupChat = mMessagingLog.getGroupChatInfo(chatId);
                if (groupChat == null) {
                    if (logger.isActivated()) {
                        logger.debug("Cannot transfer file to group chat with group chat Id '"
                                + chatId + "' as the group chat does not exist in DB.");
                    }
                    return false;
                }
                if (groupChat.getRejoinId() == null) {
                    if (logger.isActivated()) {
                        logger.debug("Cannot transfer file to group chat with group chat Id '"
                                + chatId
                                + "' as there is no ongoing session with corresponding chatId and there exists no rejoinId to rejoin the group chat.");
                    }
                    return false;
                }
            }
            return true;

        } catch (Exception e) {
            if(logger.isActivated()){
                logger.debug("exception in isAllowedToTransferFileToGroupChat");
            }
            return false;
        }
    }

    public void markFileTransferAsRead(String transferId) throws RemoteException {
        if (TextUtils.isEmpty(transferId)) {
            throw new ServerApiIllegalArgumentException("transferId must not be null or empty!");
        }
        try {
            /* No notification type corresponds currently to mark as read */
            if(logger.isActivated()){
                logger.debug("markFileTransferAsRead ftid: " + transferId);
            }
            mMessagingLog.markFileTransferAsRead(transferId, System.currentTimeMillis());
            String msgId = mMessagingLog.getFileMessageId(transferId);
            String contact = mMessagingLog.getFileTransferContactId(transferId);
            int status = mMessagingLog.getFileReadStatus(transferId);
            if(logger.isActivated()){
                logger.debug("markFileTransferAsRead status: " + status + ", transferId: " + transferId);
            }
            if(status == FileTransfer.State.DISPLAYED.toInt()){
                if(logger.isActivated()){
                    logger.debug("markFileTransferAsRead return msgId: " + msgId);
                }
                return;
            }
            contact = PhoneUtils.formatNumberToSipUri(contact);
            if(logger.isActivated()){
                logger.debug("markFileTransferAsRead contact: " + contact + ", msgId: " + msgId);
            }
            if(msgId == null) {
                if(logger.isActivated()){
                    logger.debug("Can not markFileTransferAsRead transferId: " + transferId + ", msgId: " + msgId);
                }
                return;
            }

            // Send via SIP MESSAGE
            Core.getInstance().getImService().getImdnManager().sendMessageDeliveryStatus(
                    contact, msgId, ImdnDocument.DELIVERY_STATUS_DISPLAYED);

        } catch (Exception e) {
            e.printStackTrace();
            throw new ServerApiGenericException(e);
        }
    }

    public void deleteOneToOneFileTransfers(){
        //return false;
    }

    public void deleteGroupFileTransfers(){
        //return false;
    }

    public void deleteOneToOneFileTransfers2(ContactId contact){
        //return false;
    }

    public void deleteGroupFileTransfers2(String chatId){
        //return false;
    }

    public void deleteFileTransfer(String transferId){
        //return false;
    }

    public static PauseResumeFileObject addFileTransferPauseResumeData(
                    String fileTransferId,
                    String path,
                    long fileSize,
                    String contact,
                    String mimeType,
                    int transferType) {
        if (logger.isActivated()) {
            logger.info("addFileTransferPauseResumeData" +
                    " fileTransferId " + fileTransferId +
                    " path : " + path +
                    " fileSize : " + fileSize +
                    " contact :" + contact);
        }

        PauseResumeFileObject filePauseDdata = new PauseResumeFileObject();
        filePauseDdata.bytesTransferrred = 0;
        filePauseDdata.mFileTransferId = fileTransferId;
        filePauseDdata.mPath = path;
        filePauseDdata.mSize = fileSize;
        filePauseDdata.mContact = contact;
        filePauseDdata.mMimeType = mimeType;
        filePauseDdata.mTransferType = transferType;
        mFileTransferPauseResumeData.add(filePauseDdata);

        return filePauseDdata;
    }

    public IFileTransfer resumeGroupFileTransfer(String chatId, String fileTranferId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("resumeGroupFile file " + fileTranferId);
        }

        PauseResumeFileObject pauseResumeObject = getPauseInfo(fileTranferId);
        if (pauseResumeObject == null) {
            if (logger.isActivated()) {
                logger.info("resumeGroupFile return null ");
            }
            return null;
        }

        String filename = pauseResumeObject.mPath;

        if (logger.isActivated()) {
            logger.info("resumeGroupFile file contact:" + "filename:" + filename);
        }

        // Test IMS connection
        ServerApiUtils.testIms();
        // Query Db whether FT is incoming/outgoing //Deepak

        try {
            // Initiate the session
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());
            FileSharingSession session = null;
            List fileParticipants = RichMessagingHistory.getInstance().getFileTransferContacts(
                    fileTranferId);
            if (logger.isActivated() && fileParticipants != null) {
                logger.info("resumeGroupFile file participants:" + fileParticipants + " size:"
                        + fileParticipants.size());
            }
            session = Core.getInstance().getImService().initiateGroupFileTransferSession(
                    fileParticipants, content, null, chatId, null);
            final FileSharingSession resumedSession = session;
            resumedSession.fileTransferPaused();
            resumedSession.setOldFileTransferId(fileTranferId);
            resumedSession.setSessionId(fileTranferId);

            String direction = RichMessagingHistory.getInstance().getFtDirection(fileTranferId);
            if (logger.isActivated()) {
                logger.info("resumeGroupFile file direction:" + direction);
            }
            if (direction.equals("1")) {
                resumedSession.setReceiveOnly(false);
            } else {
                resumedSession.setReceiveOnly(true);
            }
            // Add session listener
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    resumedSession.getSessionID(), mMessagingLog);

            GroupFileTransferImpl sessionApi = new GroupFileTransferImpl(resumedSession,persistedStorage,mGroupFileTransferBroadcaster,
                    chatId,this);
            //sessionApi.addEventListener(listener);

            if (pauseResumeObject != null && direction.equals("1")) {
                if (logger.isActivated()) {
                    logger.info("resumeGroupFile file pauseResumeObject:"
                            + pauseResumeObject + " & new Id is"
                            + sessionApi.getTransferId());
                }
                pauseResumeObject.mFileTransferId = sessionApi.getTransferId();

            }
            // Update Filetransfer Id rich messaging history
            if (direction.equals("1"))
                RichMessagingHistory.getInstance().updateFileTransferId(fileTranferId,
                        sessionApi.getTransferId());
            // Start the session
            Thread t = new Thread() {
                public void run() {
                    resumedSession.startSession();
                }
            };
            t.start();

            // Add session in the list
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Transfers a file to a contact. The parameter file contains the complete filename
     * including the path to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     *
     * @param contact Contact
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listenet File transfer event listener
     * @return File transfer
     * @throws ServerApiException
     */
    public IFileTransfer resumeFileTransfer(
            String fileTranferId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("resumeFile file " + fileTranferId);
        }

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
        InstantMessagingService imService = Core.getInstance().getImService();

        PauseResumeFileObject pauseResumeObject = getPauseInfo(fileTranferId);
        if (pauseResumeObject == null) {
            if (logger.isActivated()) {
                logger.info("resumeFile return null ");
            }
            return null;
        }

        String contact = pauseResumeObject.mContact;
        String filename = pauseResumeObject.mPath;
        String chatId = rmHistory.getFileTransferChatId(fileTranferId);

        if (logger.isActivated()) {
            logger.info("resumeFile file contact:" + contact + " filename:" + filename);
        }

        // Test IMS connection
        ServerApiUtils.testIms();
        // Query Db whether FT is incoming/outgoing //Deepak

        try {
            // Initiate the session
            final FileSharingSession resumedSession;
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());

            if (contact != null &&
                (PhoneUtils.compareNumbers(contact, chatId) ||
                 PhoneUtils.compareUuid(contact, chatId))) {
                // not file transfer in group chat
                List<String> contacts = PhoneUtils.generateContactsList(contact);
                resumedSession = imService.initiateExtendedTransferSession(
                        null, contacts, content, null, 0, pauseResumeObject.mTransferType);
            } else {
                resumedSession = imService.initiateExtendedTransferSession(
                        chatId, null, content, null, 0, pauseResumeObject.mTransferType);
            }
            resumedSession.fileTransferPaused();
            resumedSession.setOldFileTransferId(fileTranferId);
            resumedSession.setSessionId(fileTranferId);

            String direction = rmHistory.getFtDirection(fileTranferId);
            if (logger.isActivated()) {
                logger.info("resumeFile file direction:" + direction + "contact:" + contact);
            }
            if (direction.equals("1")) {
                resumedSession.setReceiveOnly(false);
                resumedSession.setDirection(Direction.OUTGOING.toInt());
            } else {
                resumedSession.setReceiveOnly(true);
                resumedSession.setDirection(Direction.INCOMING.toInt());
            }
            ((OriginatingExtendedFileSharingSession)resumedSession).setPauseInfo(pauseResumeObject);

            ((OriginatingExtendedFileSharingSession)resumedSession).setPreferService(FeatureTags.FEATURE_CPM_URN_SESSION);

            // Add session listener
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    resumedSession.getSessionID(), mMessagingLog);

            FileTransferImpl sessionApi = new FileTransferImpl(resumedSession,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            //sessionApi.addEventListener(listener);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    resumedSession.startSession();
                }
            };
            t.start();

            // Add session in the list
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Transfers a file to a contact. The parameter file contains the complete filename
     * including the path to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     *
     * @param contact Contact
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listenet File transfer event listener
     * @return File transfer
     * @throws ServerApiException
     */
    void resendFileTransfer(ContactId contactId, MmContent content,
            MmContent fileIcon, String fileTransferId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("resendFileTransfer " + fileTransferId);
        }

        String contact = contactId.toString();
        if (logger.isActivated()) {
            logger.info("resendFileTransfer file contact:" + contact + " filename:" + content.getName());
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            String chatSessionid = null;
            String chatContibutionId = null;
            String fileIconUrl = null;
            final FileSharingSession session = Core.getInstance().getImService()
                    .initiateFileTransferSession(contact, content, fileIconUrl, chatSessionid, chatContibutionId); // TODO

            session.setSessionId(fileTransferId);
            FileTransferImpl o2oFileTransfer = ftSessions.get(fileTransferId);
            o2oFileTransfer.setFileSession(session);

            int reasonCode = o2oFileTransfer.getReasonCode();
            if(reasonCode == FileTransfer.ReasonCode.AUTO_RESEND.toInt()){
                session.setResend(true);
            }

            if (content.getEncoding().contains("vnd.gsma.rcspushlocation"))
                session.setGeoLocFile();

            final long timestamp = System.currentTimeMillis();
            /* For outgoing file transfer, timestampSent = timestamp */
            final long timestampSent = timestamp;

            //sessionApi.addEventListener(listener);

            // Update rich messaging history
            /*RichMessagingHistory.getInstance().addFileTransfer(contact, session.getSessionID(),
                    Direction.OUTGOING.toInt(), session.getContent());*/

            if (session.getTimeLen() > 0) {
                mMessagingLog.updateFileTransferDuration(
                        session.getSessionID(), session.getTimeLen());
            }
            PauseResumeFileObject info = addFileTransferPauseResumeData(
                                            session.getSessionID(),
                                            content.getName(),
                                            session.getContent().getSize(),
                                            contact,
                                            content.getEncoding(),
                                            FileTransferLog.Type.CHAT);
            session.setPauseInfo(info);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Transfer a file to a group of contacts of existing group chat
     *
     * @param transferId File Transfer Id
     * @param contacts List of contact
     * @param file File to be transfered
     * @param thumbnail Thumbnail option
     * @return File transfer session
     * @throws ServerApiException
     */
    public IFileTransfer resendFileToGroup(String transferId,
                String chatId, List<String> contacts, Uri file,
                boolean attachFileicon) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("resendFileToGroup: " + chatId);
        }

        Context ctx = AndroidFactory.getApplicationContext();
        String filename = FileUtils.getFileName(ctx, file);
        logger.info("resendFileToGroup " + filename + " to " + contacts + ", transferId: " + transferId);

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());
            MmContent fileIconContent = null;
            String fileicon = null;
            try {
                GroupChatImpl groupObject = (GroupChatImpl)(mChatService.getGroupChat(chatId));
                if(!groupObject.isGroupActive()){
                    Object lockObject = new Object();
                    groupObject.rejoinGroup(groupObject);
                    synchronized (lockObject) {
                        lockObject.wait(2000);
                    }
                }
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.info("transferFileToGroup exception in rejoin");
                }
                e.printStackTrace();
            }

            final FileSharingSession session = Core.getInstance().getImService()
                    .initiateGroupFileTransferSession(contacts, content, fileicon, chatId, null);
           /* if (timeLen > 0) {
                session.setTimeLen(timeLen);
            }*/
            if (content.getEncoding().contains("vnd.gsma.rcspushlocation"))
                session.setGeoLocFile();

            session.setSessionId(transferId);

            final long timestamp = System.currentTimeMillis();
            /* For outgoing file transfer, timestampSent = timestamp */
            final long timestampSent = timestamp;

            /*if (Disposition.RENDER == Disposition.valueOf(disposition)) {
                content.setPlayable(true);
            }*/
            // Add session in the list
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    transferId, mMessagingLog);

            GroupFileTransferImpl sessionApi = new GroupFileTransferImpl(session,persistedStorage,mGroupFileTransferBroadcaster,
                    chatId,this);

            logger.info("ABC Transfer file ContribitionID: " + session.getContributionID());

            /*addOutgoingGroupFileTransfer(transferId,chatId,content,fileIconContent,State.INITIATING,
                            timestamp,timestampSent);*/

            /*RichMessagingHistory.getInstance().updateFileTransferDuration(
                    session.getSessionID(), timeLen);*/

            PauseResumeFileObject info = addFileTransferPauseResumeData(
                                            session.getSessionID(),
                                            filename,
                                            session.getContent().getSize(),
                                            "",
                                            content.getEncoding(),
                                            FileTransferLog.Type.CHAT);
            session.setPauseInfo(info);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    public IFileTransfer transferFile2(ContactId contact,Uri file,int disposition,boolean attachFileicon) throws ServerApiException {
        Context ctx = AndroidFactory.getApplicationContext();
        if (logger.isActivated()) {
            logger.info("transferFile2 uri: " + file.toString() + ", contact: " + contact);
        }
        String fileName = FileUtils.getFileName(ctx, file);
        return transferFile(contact.toString(),fileName,disposition,attachFileicon);
    }

    public IFileTransfer transferFile(ContactId contact,Uri file,boolean attachFileicon){
        IFileTransfer file1 = null;
        return file1;
    }

    /**
     * Transfers a file to a contact. The parameter file contains the complete filename
     * including the path to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     *
     * @param contact Contact
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listenet File transfer event listener
     * @return File transfer
     * @throws ServerApiException
     */
    public IFileTransfer transferFile(
            String contact,
            String filename,
            int disposition,
            boolean fileicon ) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("FTS Transfer file " + filename + " to " + contact + ", disposition: " + disposition + ", fileicon " + fileicon);
        }
        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            if (logger.isActivated()) {
                logger.info("FTS Transfer file " + filename + " size " + desc.getSize());
            }
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());
            MmContent iconContent = null;

            String chatSessionid = null;
            String chatContibutionId = null;
            String fileIconUrl = null;
            final FileSharingSession session = Core.getInstance().getImService()
                    .initiateFileTransferSession(contact, content, fileIconUrl, chatSessionid, chatContibutionId); // TODO

            if (content.getEncoding().contains("vnd.gsma.rcspushlocation"))
                session.setGeoLocFile();

            final long timestamp = System.currentTimeMillis();
            /* For outgoing file transfer, timestampSent = timestamp */
            final long timestampSent = timestamp;

            if (Disposition.RENDER == Disposition.valueOf(disposition)) {
                content.setPlayable(true);
            }

            // Add session listener
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    session.getSessionID(), mMessagingLog);

            FileTransferImpl sessionApi = new FileTransferImpl(session,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            //sessionApi.addEventListener(listener);

            // Update rich messaging history
            /*RichMessagingHistory.getInstance().addFileTransfer(contact, session.getSessionID(),
                    Direction.OUTGOING.toInt(), session.getContent());*/

            addOutgoingOneToOneFileTransfer(sessionApi.getTransferId(),contact,content,iconContent,State.INITIATING,
                    timestamp,timestampSent);

            if (session.getTimeLen() > 0) {
                mMessagingLog.updateFileTransferDuration(
                        session.getSessionID(), session.getTimeLen());
            }
            PauseResumeFileObject info = addFileTransferPauseResumeData(
                                            session.getSessionID(),
                                            filename,
                                            session.getContent().getSize(),
                                            contact,
                                            content.getEncoding(),
                                            FileTransferLog.Type.CHAT);
            session.setPauseInfo(info);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    public IFileTransfer transferFileLargeMode(ContactId contact,Uri file,int disposition,boolean attachFileicon) throws ServerApiException{
        Context ctx = AndroidFactory.getApplicationContext();
        String fileName = FileUtils.getFileName(ctx, file);
        return transferFileLargeMode(contact.toString(),fileName,disposition,attachFileicon);
    }

    /**
     * Transfers a file to a contact by large mode. The parameter file contains the complete filename
     * including the path to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     *
     * @param contact Contact
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param listenet File transfer event listener
     * @return File transfer
     * @throws ServerApiException
     */
    public IFileTransfer transferFileLargeMode(
            String contact,
            String filename,
            int disposition,
            boolean fileicon) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("FTS Transfer large mode file " + filename + " to " + contact + ", disposition: " + disposition + ", fileicon " + fileicon);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            if (logger.isActivated()) {
                logger.info("FTS Transfer file " + filename + " size " + desc.getSize());
            }
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());
            MmContent iconContent = null;

            String chatSessionid = null;
            String chatContibutionId = null;
            String fileIconUrl = null;
            FileSharingSession session = null;
            if(RcsSettings.getInstance().isSupportOP07()){
                session = Core.getInstance().getImService().
                        initiateLargeFileTransferSession(contact, content, fileIconUrl, chatSessionid, chatContibutionId);
            } else {
                session = Core.getInstance().getImService().
                        initiateFileTransferSession(contact, content, fileIconUrl, chatSessionid, chatContibutionId);
            }
            session.setUseLargeMode(true);

            if (content.getEncoding().contains("vnd.gsma.rcspushlocation"))
                session.setGeoLocFile();

            final long timestamp = System.currentTimeMillis();
            /* For outgoing file transfer, timestampSent = timestamp */
            final long timestampSent = timestamp;

            if (Disposition.RENDER == Disposition.valueOf(disposition)) {
                content.setPlayable(true);
            }

            // Add session listener
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    session.getSessionID(), mMessagingLog);

            FileTransferImpl sessionApi = new FileTransferImpl(session,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            //sessionApi.addEventListener(listener);

            // Update rich messaging history
            /*RichMessagingHistory.getInstance().addFileTransfer(contact, session.getSessionID(),
                    Direction.OUTGOING.toInt(), session.getContent());*/
            addOutgoingOneToOneFileTransfer(sessionApi.getTransferId(),contact,content,iconContent,State.INITIATING,
                    timestamp,timestampSent);

            if (session.getTimeLen() > 0) {
                RichMessagingHistory.getInstance().updateFileTransferDuration(
                        session.getSessionID(), session.getTimeLen());
            }
            PauseResumeFileObject info = addFileTransferPauseResumeData(
                                            session.getSessionID(),
                                            filename,
                                            session.getContent().getSize(),
                                            contact,
                                            content.getEncoding(),
                                            FileTransferLog.Type.CHAT);
            session.setPauseInfo(info);

            final FileSharingSession finalSession = session;
            // Start the session
            Thread t = new Thread() {
                public void run() {
                    finalSession.startSession();
                }
            };
            t.start();

            // Add session in the list
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    public IFileTransfer transferFileToGroupChat2(String chatId, Uri file, int disposition,boolean attachFileicon) throws ServerApiException {
        Context ctx = AndroidFactory.getApplicationContext();
        if (logger.isActivated()) {
            logger.info("transferFileToGroupChat2 uri: " + file.toString() + ", chatId: " + chatId);
        }
        String fileName = FileUtils.getFileName(ctx, file);
        GroupChatInfo groupInfo = RichMessagingHistory.getInstance().getGroupChatInfo(chatId);
        List<String> participantList = groupInfo.getParticipants();
        IFileTransfer file1 = transferFileToGroup(chatId,participantList,fileName,disposition,attachFileicon,0);
        return file1;
    }

    public void clearFileTransferDeliveryExpiration(List<String> transferIds){

    }

    /**
     * Transfer a file to a group of contacts outside of an existing group chat
     *
     * @param contacts List of contact
     * @param file File to be transfered
     * @param thumbnail Thumbnail option
     * @return File transfer session
     * @throws ServerApiException
     */
    public IFileTransfer transferFileToGroup(
                String chatId, List<String> contacts, String filename,
                int disposition,boolean attachFileicon,int timeLen)
                        throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("transferFileToGroup " + filename + " to " + contacts);
            logger.info(" transferFileToGroup: " + chatId + ", disposition: " + disposition + ", attachfileicon: " + attachFileicon);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        try {
            // Initiate the session
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());
            MmContent fileIconContent = null;
            String fileicon = null;
            try {
                GroupChatImpl groupObject = (GroupChatImpl)(mChatService.getGroupChat(chatId));
                if(!groupObject.isGroupActive()){
                    Object lockObject = new Object();
                    groupObject.rejoinGroup(lockObject);
                    synchronized (lockObject) {
                        lockObject.wait(5000);
                    }
                }
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.info("transferFileToGroup exception in rejoin");
                }
                e.printStackTrace();
            }

            final FileSharingSession session = Core.getInstance().getImService()
                    .initiateGroupFileTransferSession(contacts, content, fileicon, chatId, null);
            if (timeLen > 0) {
                session.setTimeLen(timeLen);
            }
            if (content.getEncoding().contains("vnd.gsma.rcspushlocation"))
                session.setGeoLocFile();

            final long timestamp = System.currentTimeMillis();
            /* For outgoing file transfer, timestampSent = timestamp */
            final long timestampSent = timestamp;

            if (Disposition.RENDER == Disposition.valueOf(disposition)) {
                content.setPlayable(true);
            }

            String transferId = session.getSessionID();
            // Add session in the list
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    transferId, mMessagingLog);

            GroupFileTransferImpl sessionApi = new GroupFileTransferImpl(session,persistedStorage,mGroupFileTransferBroadcaster,
                    chatId,this);
            //sessionApi.addEventListener(listener);

            logger.info("ABC Transfer file ContribitionID: " + session.getContributionID());

            /*// Update rich messaging history
            RichMessagingHistory.getInstance().addOutgoingGroupFileTransfer(
                    chatId,
                    session.getSessionID(),
                    session.getContent(),
                    null,
                    session.getRemoteContact());*/

            addOutgoingGroupFileTransfer(transferId,chatId,content,fileIconContent,State.INITIATING,
                            timestamp,timestampSent);

            RichMessagingHistory.getInstance().updateFileTransferDuration(
                    session.getSessionID(), timeLen);

            PauseResumeFileObject info = addFileTransferPauseResumeData(
                                            session.getSessionID(),
                                            filename,
                                            session.getContent().getSize(),
                                            "",
                                            content.getEncoding(),
                                            FileTransferLog.Type.CHAT);
            session.setPauseInfo(info);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Transfers a file to a contact. The parameter file contains the complete filename
     * including the path to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     *
     * @param contact Contact
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param duration Video media file duration
     * @param type Specified transfer file
     * @param listener File transfer event listener
     * @return File transfer
     * @throws ServerApiException
     */
    public IFileTransfer transferFileEx(
            String contact,
            String filename,
            int disposition,
            boolean fileicon,
            int duration,
            String type) throws ServerApiException {

        List<String> contacts = PhoneUtils.generateContactsList(contact);
        return transferFileEx(null, contacts, filename, disposition,fileicon,duration, type);
    }

    public IFileTransfer transferFileToMultiple(List<String> contacts, Uri file,int disposition,boolean attachFileicon) throws ServerApiException{
        Context ctx = AndroidFactory.getApplicationContext();
        String fileName = FileUtils.getFileName(ctx, file);
        return transferMultiFileEx(null,contacts, fileName, disposition,attachFileicon, 0, FileTransfer.Type.NORMAL);
    }

    /**
     * Transfers a file to group chat. The parameter file contains the complete filename
     * including the path to be transferred. The parameter contact supports the following
     * formats: MSISDN in national or international format, SIP address, SIP-URI or
     * Tel-URI. If the format of the contact is not supported an exception is thrown.
     *
     * @param chatId Identifier of group chat
     * @param contacts Contact
     * @param filename Filename to transfer
     * @param fileicon Filename of the file icon associated to the file to be transfered
     * @param duration Video media file duration
     * @param type Specified transfer file
     * @param listener File transfer event listener
     * @return File transfer
     * @throws ServerApiException
     */
    public IFileTransfer transferFileToGroupEx(
                String chatId,
                String filename,
                int disposition,
                boolean fileicon,
                int duration,
                String type) throws ServerApiException {
        List<String> contacts = null;
        return transferFileEx(chatId, contacts, filename, disposition, fileicon, duration, type);
    }

    public IFileTransfer transferFileToSecondaryDevice(
            String chatId,
            String filename,
            int disposition,
            boolean fileicon,
            int duration,
            String type) throws ServerApiException {

        String contact = RcsSettings.getInstance().getSecondaryDeviceUserIdentity();
        List<String> contacts = PhoneUtils.generateContactsList(contact);

        return transferFileEx(chatId, contacts, filename, disposition, fileicon, duration, type);
    }

    /**
     * Prosecute file to specific service number.
     *
     * @param contacts Contact
     * @param transferId file identifier
     * @param listener File transfer event listener
     * @return File transfer
     * @throws ServerApiException
     */
    public IFileTransfer prosecuteFile(
            String contact,
            String transferId,
            IOneToOneFileTransferListener listener) throws ServerApiException {

        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();
        PauseResumeFileObject info = rmHistory.getPauseInfo(transferId);

        String extraContent =
                "Spam-From: " + ChatUtils.formatCpimSipUri(info.mContact) + SipUtils.CRLF +
                "Spam-To: " + ImsModule.IMS_USER_PROFILE.getPublicUri() + SipUtils.CRLF +
                "DateTime: " + DateUtils.encodeDate(rmHistory.getFileTimeStamp(transferId));

        List<String> contacts = PhoneUtils.generateContactsList(contact);
        /*return transferFileEx(
                null, contacts, info.mPath, extraContent, 0, FileTransfer.Type.PROSECUTE, listener);*/
        return null;
    }

    private IFileTransfer transferMultiFileEx(
            String chatId,
            List<String> contacts,
            String filename,
            int disposition,
            boolean fileicon,
            int duration,
            String type) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("FTS Transfer file " + filename + " to " + contacts + "with Timelen "
                    + duration);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        InstantMessagingService imService = Core.getInstance().getImService();
        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();

        int sessionType = FileTransferLog.Type.CHAT;
        if (type.equals(FileTransfer.Type.PUBACCOUNT)) {
            sessionType = FileTransferLog.Type.PUBLIC;
        } else
        if (type.equals(FileTransfer.Type.BURNED)) {
            sessionType = FileTransferLog.Type.BURN;
        }
        String extraContent = null;
        if (type.equals(FileTransfer.Type.PROSECUTE)) {
            //fileicon = null;
        }

        try {
            // Initiate the session
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());
            //content.setEncoding(CpimMessage.MIME_TYPE);
            MmContent iconContent = null;
            final FileSharingSession session = imService.initiateExtendedTransferSession(
                    chatId, contacts, content, null, duration, sessionType);

            String transferId = session.getSessionID();
            session.setUseLargeMode(true);
            String fileIconUrl = null;


            // Add session listener
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    transferId, mMessagingLog);

            FileTransferImpl sessionApi = new FileTransferImpl(session,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            //sessionApi.addEventListener(listener);

            if (content.getEncoding().contains("vnd.gsma.rcspushlocation"))
                session.setGeoLocFile();

            session.setDirection(Direction.OUTGOING.toInt());

            String contact = PhoneUtils.generateContactsText(contacts);

            if (type.equals(FileTransfer.Type.NORMAL)) {

            } else
            if (type.equals(FileTransfer.Type.BURNED)) {
                session.setBurnMessage(true);
                OriginatingExtendedFileSharingSession exSession =
                        (OriginatingExtendedFileSharingSession)session;
                List<String> extraTags = new ArrayList<String>();
                extraTags.add(FeatureTags.FEATURE_CPM_BURNED_MSG);
                exSession.setExtraTags(extraTags);
            } else
            if (type.equals(FileTransfer.Type.PUBACCOUNT)) {
                session.setPublicChatFile(true);
                OriginatingExtendedFileSharingSession exSession =
                        (OriginatingExtendedFileSharingSession)session;
                List<String> extraTags = new ArrayList<String>();
                extraTags.add(FeatureTags.FEATURE_CMCC_IARI_PUBLIC_ACCOUNT);
                exSession.setExtraTags(extraTags);
                exSession.setPreferService(FeatureTags.FEATURE_CMCC_URN_PUBLIC_ACCOUNT);
            } else
            if (type.equals(FileTransfer.Type.PROSECUTE)) {
                OriginatingExtendedFileSharingSession exSession =
                        (OriginatingExtendedFileSharingSession)session;
                exSession.setExtraContent(extraContent, "text/plain;charset=UTF-8");
            }

            final long timestamp = System.currentTimeMillis();
            /* For outgoing file transfer, timestampSent = timestamp */
            final long timestampSent = timestamp;

            if (Disposition.RENDER == Disposition.valueOf(disposition)) {
                content.setPlayable(true);
            }

            /*rmHistory.addExtendedFileTransfer(
                    chatId,
                    contact,
                    transferId,
                    Direction.OUTGOING.toInt(),
                    sessionType,
                    content);*/
            addOutgoingOneToOneFileTransfer(transferId,contact,content,iconContent,State.INITIATING,
                    timestamp,timestampSent);

            // Update rich messaging history
            if (duration > 0) {
                rmHistory.updateFileTransferDuration(transferId, duration);
            }
            PauseResumeFileObject info = addFileTransferPauseResumeData(
                                            transferId,
                                            filename,
                                            content.getSize(),
                                            contact,
                                            content.getEncoding(),
                                            sessionType);
            session.setPauseInfo(info);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    private IFileTransfer transferFileEx(
            String chatId,
            List<String> contacts,
            String filename,
            int disposition,
            boolean fileicon,
            int duration,
            String type) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("FTS Transfer file " + filename + " to " + contacts + "with Timelen "
                    + duration);
        }

        // Test IMS connection
        ServerApiUtils.testIms();

        InstantMessagingService imService = Core.getInstance().getImService();
        RichMessagingHistory rmHistory = RichMessagingHistory.getInstance();

        int sessionType = FileTransferLog.Type.CHAT;
        if (type.equals(FileTransfer.Type.PUBACCOUNT)) {
            sessionType = FileTransferLog.Type.PUBLIC;
        } else
        if (type.equals(FileTransfer.Type.BURNED)) {
            sessionType = FileTransferLog.Type.BURN;
        }
        String extraContent = null;
        MmContent iconContent = null;
        if (type.equals(FileTransfer.Type.PROSECUTE)) {
            //fileicon = null;
        }

        try {
            // Initiate the session
            FileDescription desc = FileFactory.getFactory().getFileDescription(filename);
            MmContent content = ContentManager.createMmContentFromUrl(filename, desc.getSize());
            final FileSharingSession session = imService.initiateExtendedTransferSession(
                    chatId, contacts, content, null, duration, sessionType);

            String transferId = session.getSessionID();
            String fileIconUrl = null;

            // Add session listener
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    transferId, mMessagingLog);

            FileTransferImpl sessionApi = new FileTransferImpl(session,persistedStorage,mOneToOneFileTransferBroadcaster,this);
            //sessionApi.addEventListener(listener);

            if (content.getEncoding().contains("vnd.gsma.rcspushlocation"))
                session.setGeoLocFile();

            final long timestamp = System.currentTimeMillis();
            /* For outgoing file transfer, timestampSent = timestamp */
            final long timestampSent = timestamp;

            if (Disposition.RENDER == Disposition.valueOf(disposition)) {
                content.setPlayable(true);
            }

            session.setDirection(Direction.OUTGOING.toInt());

            String contact = PhoneUtils.generateContactsText(contacts);

            if (type.equals(FileTransfer.Type.NORMAL)) {

            } else
            if (type.equals(FileTransfer.Type.BURNED)) {
                session.setBurnMessage(true);
                OriginatingExtendedFileSharingSession exSession =
                        (OriginatingExtendedFileSharingSession)session;
                List<String> extraTags = new ArrayList<String>();
                extraTags.add(FeatureTags.FEATURE_CPM_BURNED_MSG);
                exSession.setExtraTags(extraTags);
            } else
            if (type.equals(FileTransfer.Type.PUBACCOUNT)) {
                session.setPublicChatFile(true);
                OriginatingExtendedFileSharingSession exSession =
                        (OriginatingExtendedFileSharingSession)session;
                List<String> extraTags = new ArrayList<String>();
                extraTags.add(FeatureTags.FEATURE_CMCC_IARI_PUBLIC_ACCOUNT);
                exSession.setExtraTags(extraTags);
                exSession.setPreferService(FeatureTags.FEATURE_CMCC_URN_PUBLIC_ACCOUNT);
            } else
            if (type.equals(FileTransfer.Type.PROSECUTE)) {
                OriginatingExtendedFileSharingSession exSession =
                        (OriginatingExtendedFileSharingSession)session;
                exSession.setExtraContent(extraContent, "text/plain;charset=UTF-8");
            }

            /*rmHistory.addExtendedFileTransfer(
                    chatId,
                    contact,
                    transferId,
                    Direction.OUTGOING.toInt(),
                    sessionType,
                    content);*/

            addOutgoingOneToOneFileTransfer(transferId,contact,content,iconContent,State.INITIATING,
                    timestamp,timestampSent);

            // Update rich messaging history
            if (duration > 0) {
                rmHistory.updateFileTransferDuration(transferId, duration);
            }
            PauseResumeFileObject info = addFileTransferPauseResumeData(
                                            transferId,
                                            filename,
                                            content.getSize(),
                                            contact,
                                            content.getEncoding(),
                                            sessionType);
            session.setPauseInfo(info);

            // Start the session
            Thread t = new Thread() {
                public void run() {
                    session.startSession();
                }
            };
            t.start();

            // Add session in the list
            addFileTransferSession(sessionApi);
            return sessionApi;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Returns the list of file transfers in progress
     *
     * @return List of file transfer
     * @throws ServerApiException
     */
    public List<IBinder> getFileTransfers() throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("FTS Get file transfer sessions");
        }

        try {
            ArrayList<IBinder> result = new ArrayList<IBinder>(ftSessions.size());
            /*for (Enumeration<IFileTransfer> e = ftSessions.elements(); e.hasMoreElements();) {
                IFileTransfer sessionApi = e.next;
                result.add(sessionApi.asBinder());
            }
            for (Enumeration<IFileTransfer> e1 = groupFtSessions.elements(); e1.hasMoreElements();) {
                IFileTransfer sessionApi = e1.nextElement();
                result.add(sessionApi.asBinder());
            }*/
            return result;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Returns a current file transfer from its unique ID
     *
     * @return File transfer
     * @throws ServerApiException
     */
    public IFileTransfer getFileTransfer(String transferId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("FTS Get file transfer session " + transferId);
        }

        IFileTransfer fileTransfer = ftSessions.get(transferId);
        if (fileTransfer != null) {
            return fileTransfer;
        }

        if (logger.isActivated()) {
            logger.info("getFileTransfer group " + transferId);
        }
        fileTransfer = groupFtSessions.get(transferId);

        if(fileTransfer != null){
            return fileTransfer;
        }

        if (mMessagingLog.isGroupFileTransfer(transferId)) {
            String chatId = mMessagingLog.getFileTransferChatId(transferId);
            return createGroupFileTransfer(chatId, transferId);
        }

        return createOneToOneFileTransfer(transferId);
    }

    /**
     * Create group file transfer
     *
     * @param chatId the chat ID
     * @param transferId th file transfer ID
     * @return GroupFileTransferImpl
     */
    public GroupFileTransferImpl createGroupFileTransfer(String chatId, String transferId) {
        GroupFileTransferImpl groupFileTransfer = groupFtSessions.get(transferId);
        if (groupFileTransfer == null) {
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    transferId, mMessagingLog);
            FileSharingSession session = null;
            new GroupFileTransferImpl(session,persistedStorage,mGroupFileTransferBroadcaster,
                    chatId,transferId,this);
            groupFtSessions.put(transferId, groupFileTransfer);
        }
        return groupFileTransfer;
    }

    /**
     * Create one-one file transfer
     *
     * @param transferId th file transfer ID
     * @return OneToOneFileTransferImpl
     */
    public FileTransferImpl createOneToOneFileTransfer(String transferId) {
        if (logger.isActivated()) {
            logger.info("createOneToOneFileTransfer " + transferId);
        }
        FileTransferImpl oneToOneFileTransfer = ftSessions.get(transferId);
        if (oneToOneFileTransfer == null) {
            FileTransferPersistedStorageAccessor persistedStorage = new FileTransferPersistedStorageAccessor(
                    transferId, mMessagingLog);
            FileSharingSession session = null;
            oneToOneFileTransfer = new FileTransferImpl(session, persistedStorage,
                    mOneToOneFileTransferBroadcaster, transferId, this);
            if (logger.isActivated()) {
                logger.info("createOneToOneFileTransfer  put in cache" + transferId);
            }
            ftSessions.put(transferId, oneToOneFileTransfer);
        }
        return oneToOneFileTransfer;
    }

    /**
     * Registers a file transfer invitation listener
     *
     * @param listener New file transfer listener
     * @throws ServerApiException
     */
    public void addEventListener2(IOneToOneFileTransferListener listener)
            throws ServerApiException {
        if (listener == null) {
            throw new ServerApiIllegalArgumentException("listener must not be null!");
        }
        try {
            synchronized (lock) {
                mOneToOneFileTransferBroadcaster.addOneToOneFileTransferListener(listener);
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServerApiGenericException(e);
        }
    }

    /**
     * Unregisters a file transfer invitation listener
     *
     * @param listener New file transfer listener
     * @throws ServerApiException
     */
    public void removeEventListener2(IOneToOneFileTransferListener listener)
            throws ServerApiException {
        if (listener == null) {
            throw new ServerApiIllegalArgumentException("listener must not be null!");
        }
        try {
            synchronized (lock) {
                mOneToOneFileTransferBroadcaster.removeOneToOneFileTransferListener(listener);
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServerApiGenericException(e);
        }
    }

    /**
     * Registers a file transfer invitation listener
     *
     * @param listener New file transfer listener
     * @throws ServerApiException
     */
    public void addEventListener3(IGroupFileTransferListener listener)
            throws ServerApiException {
        if (listener == null) {
            throw new ServerApiIllegalArgumentException("listener must not be null!");
        }
        try {
            synchronized (lock) {
                mGroupFileTransferBroadcaster.addGroupFileTransferListener(listener);
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServerApiGenericException(e);
        }
    }

    /**
     * Unregisters a file transfer invitation listener
     *
     * @param listener New file transfer listener
     * @throws ServerApiException
     */
    public void removeEventListener3(IGroupFileTransferListener listener)
            throws ServerApiException {
        try {
            synchronized (lock) {
                mGroupFileTransferBroadcaster.removeGroupFileTransferListener(listener);
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw new ServerApiGenericException(e);
        }
    }

    /**
     * File Transfer delivery status. In FToHTTP, Delivered status is done just after
     * download information are received by the terminating, and Displayed status is done
     * when the file is downloaded. In FToMSRP, the two status are directly done just
     * after MSRP transfer complete.
     *
     * @param ftSessionId File transfer session Id
     * @param status status of File transfer
     */
    public void handleFileDeliveryStatus(String ftSessionId, String status, String contact) {
        if (logger.isActivated()) {
            logger.info("FTS handleFileDeliveryStatus contact: " + contact + " FtId: "
                    + ftSessionId + " Status: " + status);
        }
        contact = PhoneUtils.extractNumberFromUri(contact);
        if (logger.isActivated()) {
            logger.info("FTS handleFileDeliveryStatus contact is: " + contact);
        }
        if (status.equalsIgnoreCase(ImdnDocument.DELIVERY_STATUS_DELIVERED)) {
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateFileTransferStatus(ftSessionId,
                    FileTransfer.State.DELIVERED.toInt());

            /*// Notify File transfer delivery listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("FTS handleFileDeliveryStatus N is: " + N);
            }
            for (int i = 0; i < N; i++) {
                try {
                    listeners.getBroadcastItem(i).onFileDeliveredReport(ftSessionId, contact);
                } catch (Exception e) {
                    if (logger.isActivated()) {
                        logger.error("FTS Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();*/
        } else if (status.equalsIgnoreCase(ImdnDocument.DELIVERY_STATUS_DISPLAYED)) {
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateFileTransferStatus(ftSessionId,
                    FileTransfer.State.DISPLAYED.toInt());

            /*// Notify File transfer delivery listeners
            final int N = listeners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("FTS handleFileDeliveryStatus N: " + N);
            }
            for (int i = 0; i < N; i++) {
                try {
                    listeners.getBroadcastItem(i).onFileDisplayedReport(ftSessionId, contact);
                } catch (Exception e) {
                    if (logger.isActivated()) {
                        logger.error("FTS Can't notify listener", e);
                    }
                }
            }
            listeners.finishBroadcast();*/
        }
        if (logger.isActivated()) {
            logger.info("handleFileDeliveryStatus MSGID is " + ftSessionId + ", status: " + status);
        }
        Intent intent = new Intent(FileTransferIntent.ACTION_DELIVERY_STATUS);
        intent.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        intent.putExtra(OneToOneChatIntent.EXTRA_CONTACT, contact);
        intent.putExtra(OneToOneChatIntent.EXTRA_MESSAGE_ID, ftSessionId);
        intent.putExtra(OneToOneChatIntent.EXTRA_STATUS, status);
        AndroidFactory.getApplicationContext().sendBroadcast(intent, "com.gsma.services.permission.RCS");
    }

    /**
     * Returns the maximum number of file transfer session simultaneously
     *
     * @return numenr of sessions
     */
    public int getMaxFileTransfers() {
        return RcsSettings.getInstance().getMaxFileTransferSessions();
    }

    /**
     * Returns service version
     *
     * @return Version
     * @see Build.VERSION_CODES
     * @throws ServerApiException
     */
    public int getServiceVersion() throws ServerApiException {
        return Build.API_VERSION;
    }

    /**
     * Add outgoing one to one file transfer to DB
     *
     * @param fileTransferId File transfer ID
     * @param contact ContactId
     * @param content Content of file
     * @param fileicon Content of file icon
     * @param state State of the file transfer
     * @param timestamp Local timestamp of the file transfer
     * @param timestampSent Timestamp sent in payload of the file transfer
     */
    private void addOutgoingOneToOneFileTransfer(String fileTransferId, String contact,
            MmContent content, MmContent fileicon, State state, long timestamp, long timestampSent) {
        mMessagingLog.addFileTransfer(fileTransferId, contact, Direction.OUTGOING.toInt(), content,
                fileicon, state.toInt(), ReasonCode.UNSPECIFIED.toInt(), timestamp, timestampSent,
                FileTransferData.UNKNOWN_EXPIRATION, FileTransferData.UNKNOWN_EXPIRATION);
    }

    /**
     * Add outgoing group file transfer to DB
     *
     * @param fileTransferId File transfer ID
     * @param chatId Chat ID of group chat
     * @param content Content of file
     * @param fileicon Content of fileicon
     * @param state state of file transfer
     * @param timestamp Local timestamp of the file transfer
     * @param timestampSent Timestamp sent in payload of the file transfer
     */
    private void addOutgoingGroupFileTransfer(String fileTransferId, String chatId,
            MmContent content, MmContent fileicon, State state, long timestamp, long timestampSent) {
        Set<ContactId> recipients = null;//(GroupChatImpl)(mChatService.getGroupChat(chatId)).getRecipients();
       /* if (recipients == null) {
            throw new ServerApiPersistentStorageException(
                    "Unable to determine recipients of the group chat " + chatId
                            + " to set as recipients for the the group file transfer "
                            + fileTransferId + "!");
        }*/
        mMessagingLog.addOutgoingGroupFileTransfer(fileTransferId, chatId, content, fileicon,
                state.toInt(), FileTransfer.ReasonCode.UNSPECIFIED.toInt(), timestamp, timestampSent);
    }

    public FileTransferProtocol getFileTransferProtocolForOneToOneFileTransfer(ContactId contact) {
        com.orangelabs.rcs.core.ims.service.capability.Capabilities myCapabilities = mRcsSettings.getMyCapabilities();
        Capabilities remoteCapabilities = ContactsManager.getInstance().getContactCapabilities(contact.toString());
        if (remoteCapabilities == null) {
            return null;
        }
        boolean ftMsrpSupportedforSelf = myCapabilities.isFileTransferSupported();
        boolean ftHttpSupportedforSelf = myCapabilities.isFileTransferHttpSupported();
        boolean ftMsrpSupportedforRemote = remoteCapabilities.isFileTransferSupported();
        boolean ftHttpSupportedforRemote = remoteCapabilities.isFileTransferHttpSupported();
        if (logger.isActivated()) {
            logger.debug("There are are no available capabilities : FTMsrp(Self)"
                    + ftMsrpSupportedforSelf + " FTHttp(Self)" + ftHttpSupportedforSelf
                    + " FTMsrp(Remote)" + ftMsrpSupportedforRemote + " FTHttp(Remote)"
                    + ftHttpSupportedforRemote);
        }

        if (ftMsrpSupportedforSelf && ftMsrpSupportedforRemote) {
            return FileTransferProtocol.MSRP;
        } else if (ftHttpSupportedforSelf && ftHttpSupportedforRemote) {
            return FileTransferProtocol.HTTP;
        } else {
            return null;
        }
    }
}
