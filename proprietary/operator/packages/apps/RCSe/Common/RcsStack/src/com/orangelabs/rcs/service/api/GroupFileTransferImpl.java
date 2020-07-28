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

import java.util.Date;

import org.xbill.DNS.MFRecord;

import com.orangelabs.rcs.provider.messaging.FileTransferData;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ft.FileTransfer;
import com.gsma.services.rcs.ft.IFileTransfer;
import com.gsma.services.rcs.ft.IGroupFileTransferListener;
import com.gsma.services.rcs.ft.IOneToOneFileTransferListener;
import com.gsma.services.rcs.RcsService;
import com.gsma.services.rcs.RcsService.Direction;
import com.gsma.services.rcs.ft.FileTransfer.ReasonCode;
import com.gsma.services.rcs.ft.FileTransfer.Disposition;
import com.gsma.services.rcs.ft.FileTransfer.State;
import com.gsma.services.rcs.ft.FileTransfer.Disposition;

import android.net.Uri;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

import com.orangelabs.rcs.provider.messaging.FileTransferData;
import com.orangelabs.rcs.service.broadcaster.GroupFileTransferBroadcaster;
import com.orangelabs.rcs.service.broadcaster.IGroupFileTransferBroadcaster;
import com.orangelabs.rcs.core.Core;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.ImsFileSharingSession;
import com.orangelabs.rcs.core.content.MmContent;
import com.orangelabs.rcs.provider.messaging.FileTransferPersistedStorageAccessor;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.service.ImsServiceSession;
import com.orangelabs.rcs.core.ims.service.im.InstantMessagingService;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingError;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSessionListener;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.OriginatingExtendedFileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.OriginatingFileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.HttpFileTransferSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.HttpTransferState;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.OriginatingHttpFileSharingSession;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.ContactIdUtils;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * File transfer implementation
 * 
 * @author Jean-Marc AUFFRET
 */
public class GroupFileTransferImpl extends IFileTransfer.Stub implements FileSharingSessionListener {
    
    /**
     * Core session
     */
    private FileSharingSession session;
    
    /**
     * List of listeners
     */
    //private RemoteCallbackList<IFileTransferListener> listeners = new RemoteCallbackList<IFileTransferListener>();    
    
    private IGroupFileTransferBroadcaster mBroadcaster = null;
    
    /**
     * List of file transfer invitation listeners
     */
    private RemoteCallbackList<IGroupFileTransferListener> groupftlisteners =
            new RemoteCallbackList<IGroupFileTransferListener>();
    
    private final RcsSettings mRcsSettings = RcsSettings.getInstance();
    
    private  ChatServiceImpl mChatService = null;
    
    private  FileTransferServiceImpl mFileService = null;
    
    private InstantMessagingService mImsService;
    
    private RichMessagingHistory mMessagingLog = RichMessagingHistory.getInstance();
    
    private String mFileTransferId = null;
    
    private FileTransferPersistedStorageAccessor mPersistentStorage = null;
    
    private String mChatId = null;

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
     * @param session Session
     */
    public GroupFileTransferImpl(FileSharingSession session,
            FileTransferPersistedStorageAccessor persistentStorage,
            IGroupFileTransferBroadcaster broadcaster,
            String chatId,
            FileTransferServiceImpl fileService) {
        if (logger.isActivated()) {
            logger.info("GroupFileTransferImpl constructor1");
        }
        this.session = session;
        
        session.addListener(this);
        mFileTransferId = session.getSessionID();
        mPersistentStorage = persistentStorage;
        mBroadcaster = broadcaster;
        mChatId = chatId;
        mFileService = fileService;
        mMessagingLog = RichMessagingHistory.getInstance();
        mImsService = Core.getInstance().getImService();
    }
    
    /**
     * Constructor
     * 
     * @param session Session
     */
    public GroupFileTransferImpl(FileSharingSession session,
            FileTransferPersistedStorageAccessor persistentStorage,
            IGroupFileTransferBroadcaster broadcaster,
            String chatId,
            String fileTransferId,
            FileTransferServiceImpl fileService) {
        if (logger.isActivated()) {
            logger.info("GroupFileTransferImpl constructor2");
        }
        this.session = session;
        
        mFileTransferId = session.getSessionID();
        mPersistentStorage = persistentStorage;
        mBroadcaster = broadcaster;
        mChatId = chatId;
        mFileTransferId  = fileTransferId;
        mFileService = fileService;
        mMessagingLog = RichMessagingHistory.getInstance();
        mImsService = Core.getInstance().getImService();
    }
    
    /**
     * Constructor
     * 
     * @param session Session
     */
    public GroupFileTransferImpl(FileSharingSession session) {
        if (logger.isActivated()) {
            logger.info("GroupFileTransferImpl constructor3");
        }
        this.session = session;
        
        session.addListener(this);
        mFileTransferId = session.getSessionID();
        mMessagingLog = RichMessagingHistory.getInstance();
        mImsService = Core.getInstance().getImService();
    }
    
    public void setFileSession(FileSharingSession session){
        if (logger.isActivated()) {
            logger.info("setFileSession session: " + session);
        }
        if(session != null){
            if (logger.isActivated()) {
                logger.info("setFileSession1");
            }
            this.session = session;
            session.addListener(this);
        }
    }

    private ReasonCode getRcsReasonCode(FileSharingSession session) {
        if (isSessionPaused(session)) {
            /*
             * If session is paused and still established it must have been paused by user
             */
            return ReasonCode.PAUSED_BY_USER;
        }
        return ReasonCode.UNSPECIFIED;
    }

    @Override
    public String getChatId() throws RemoteException {
        try {
            if (mChatId != null) {
                return mChatId;
            }
            return mPersistentStorage.getChatId();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public ContactId getRemoteContact() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            String contact = null;
            if (session == null) {
                return mPersistentStorage.getRemoteContact();
            }
            contact = session.getRemoteContact();
            ContactId contactId = ContactIdUtils.createContactIdFromTrustedData(contact);
            return contactId;

        }catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public String getFileName() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getFileName();
            }
            return session.getContent().getUrl().toString();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }
    
    public MmContent getFileContent() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            return session.getContent();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public Uri getFile() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getFile();
            }
            return session.getContent().getUri();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public long getFileSize() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getFileSize();
            }
            return session.getContent().getSize();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public String getMimeType() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getMimeType();
            }
            return session.getContent().getEncoding();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public Uri getFileIcon() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getFileIcon();
            }
            MmContent fileIcon = session.getFileicon();
            return fileIcon != null ? fileIcon.getUri() : null;

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public String getFileIconMimeType() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getFileIconMimeType();
            }
            MmContent fileIconContent = session.getFileicon();
            return fileIconContent != null ? fileIconContent.getEncoding() : null;

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    public int getReasonCode() {
        try {
            if(session == null)
                session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getReasonCode().toInt();
            }
            return getRcsReasonCode(session).toInt();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public int getDisposition() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getDisposition().toInt();
            }
            if (session.getContent().isPlayable()) {
                return Disposition.RENDER.toInt();
            }
            return Disposition.ATTACH.toInt();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public int getDirection() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getDirection().toInt();
            }
            if (session.isInitiatedByRemote()) {
                return Direction.INCOMING.toInt();
            }
            return Direction.OUTGOING.toInt();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public long getTimestamp() throws RemoteException {
        try {
            return mPersistentStorage.getTimestamp();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public long getTimestampSent() throws RemoteException {
        try {
            return mPersistentStorage.getTimestampSent();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public long getTimestampDelivered() throws RemoteException {
        try {
            return mPersistentStorage.getTimestampDelivered();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public long getTimestampDisplayed() throws RemoteException {
        try {
            return mPersistentStorage.getTimestampDisplayed();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    /**
     * Returns the file transfer ID of the file transfer
     * 
     * @return Transfer ID
     */
    public String getTransferId() {
        return session.getSessionID();
    }
    
    public long getFileExpiration() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getFileExpiration();
            }
            return session.getFileExpiration();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }
    
    public long getFileIconExpiration() throws RemoteException {
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getFileIconExpiration();
            }
            return session.getIconExpiration();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    @Override
    public boolean isExpiredDelivery() throws RemoteException {
        try {
            return mPersistentStorage.isExpiredDelivery();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    /**
     * Returns the duration of the file to be transferred
     *
     * @return Filename
     */
    public int getTransferDuration() {
        return session.getTimeLen();
    }
    
    public int getState() throws RemoteException {
        try {
            if(session == null)
                session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                return mPersistentStorage.getState().toInt();
            }
            return getRcsState(session);

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }

    /**
     * Returns the state of the file transfer
     * 
     * @return State 
     */
    public int getRcsState(FileSharingSession session) {
        int result = FileTransfer.State.UNKNOWN.toInt();
        if (session instanceof HttpFileTransferSession) {
            // HTTP transfer
            int state = ((HttpFileTransferSession)session).getSessionState(); 
            if (logger.isActivated()) {
                logger.info("FTS getHttpState state: " + state);
            }
            if (state == HttpTransferState.CANCELLED) {
                // Session canceled
                result = FileTransfer.State.ABORTED.toInt();
            } else
            if (state == HttpTransferState.PAUSED) {
                // Session paused
                result = FileTransfer.State.PAUSED.toInt();
            } else
            if (state == HttpTransferState.ESTABLISHED) {
                // Session started
                result = FileTransfer.State.STARTED.toInt();
            } else
            if (state == HttpTransferState.TERMINATED) {
                // Session terminated
                if (session.isFileTransfered()) {
                    result = FileTransfer.State.TRANSFERRED.toInt();
                } else {
                    result = FileTransfer.State.ABORTED.toInt();
                }
            } else
            if (state == HttpTransferState.PENDING) {
                // Session pending
                if (session instanceof OriginatingHttpFileSharingSession) {
                    result = FileTransfer.State.INITIATED.toInt();
                } else {
                    result = FileTransfer.State.INVITED.toInt();
                }
            }
        } else {
            // MSRP transfer
            SipDialogPath dialogPath = session.getDialogPath();
            if (dialogPath != null) {
                if (dialogPath.isSessionCancelled()) {
                    // Session canceled
                    result = FileTransfer.State.ABORTED.toInt();
                } else
                if (dialogPath.isSessionEstablished()) {
                    // Session started
                    result = FileTransfer.State.STARTED.toInt();
                } else
                if (dialogPath.isSessionTerminated()) {
                    // Session terminated
                    if (session.isFileTransfered()) {
                        result = FileTransfer.State.TRANSFERRED.toInt();
                    } else {
                        result = FileTransfer.State.ABORTED.toInt();
                    }
                } else {
                    // Session pending
                    if (session instanceof OriginatingFileSharingSession) {
                        result = FileTransfer.State.INITIATED.toInt();
                    } else if (session instanceof OriginatingExtendedFileSharingSession){
                        return FileTransfer.State.INITIATED.toInt();
                    } else {
                        result = FileTransfer.State.INVITED.toInt();
                    }
                }
            }
        }
        if (logger.isActivated()) {
            logger.info("FTS getState return state: " + result);
        }
        return result;
    }

    public boolean isAllowedToResendTransfer(){
        return false;
    }
    
    public boolean isAllowedToPauseTransfer(){
        try {
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session == null) {
                if (logger.isActivated()) {
                    logger.debug("Cannot pause transfer with file transfer Id '"
                            + mFileTransferId
                            + "' as there is no ongoing session corresponding to the fileTransferId.");
                }
                return false;
            }
            int stateValue = getRcsState(session);
            State state = State.valueOf(stateValue);
            if (State.STARTED != state) {
                if (logger.isActivated()) {
                    logger.debug("Cannot pause transfer with file transfer Id '" + mFileTransferId
                            + "' as it is in state " + state);
                }
                return false;
            }
            if (mPersistentStorage.getFileTransferProgress() == mPersistentStorage.getFileSize()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot pause transfer with file transfer Id '" + mFileTransferId
                            + "' as full content is transferred");
                }
                return false;
            }
            return true;

        } catch (Exception e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.info("isAllowedToPauseTransfer exception");
            }
            return false;
        }
    }
    
    public boolean isAllowedToResumeTransfer(){
        try {
            ReasonCode reasonCode;
            FileSharingSession session = mImsService.getFileSharingSession(mFileTransferId);
            if (session != null) {
                reasonCode = getRcsReasonCode(session);
            } else {
                reasonCode = mPersistentStorage.getReasonCode();
            }
            if (ReasonCode.PAUSED_BY_USER != reasonCode) {
                if (logger.isActivated()) {
                    logger.debug("Cannot resume transfer with file transfer Id '"
                            + mFileTransferId + "' as it is " + reasonCode);
                }
                return false;
            }
            if (!ServerApiUtils.isImsConnected()) {
                if (logger.isActivated()) {
                    logger.debug("Cannot resume transfer with file transfer Id '"
                            + mFileTransferId + "' as it there is no IMS connection right now.");
                }
                return false;
            }
            if (session == null) {
                if (!mImsService.isFileTransferSessionAvailable()) {
                    if (logger.isActivated()) {
                        logger.debug("Cannot resume transfer with file transfer Id '"
                                + mFileTransferId
                                + "' as the limit of available file transfer session is reached.");
                    }
                    return false;
                }
                if (Direction.OUTGOING == mPersistentStorage.getDirection()) {
                    if (mImsService.isMaxConcurrentOutgoingFileTransfersReached()) {
                        if (logger.isActivated()) {
                            logger.debug("Cannot resume transfer with file transfer Id '"
                                    + mFileTransferId
                                    + "' as the limit of maximum concurrent outgoing file transfer is reached.");
                        }
                        return false;
                    }
                }
            }
            return true;

        } catch (Exception e) {
            e.printStackTrace();
            if (logger.isActivated()) {
                logger.info("isAllowedToResumeTransfer exception");
            }
            return false;
        }
    }
    
    public boolean isRead() throws RemoteException {
        try {
            return mPersistentStorage.isRead();

        } catch (Exception e) {
            e.printStackTrace();
            throw e;
        }
    }
    
    public void resendTransfer() throws RemoteException{
        if (logger.isActivated()) {
            logger.info("resendTransfer Invalid operation for group");
        }
        throw new RemoteException(
                "Resend operation not supported for group file transfer with file transfer ID "
                        + mFileTransferId);
    }

    public boolean isHttpFileTransfer(){
        if (logger.isActivated()) {
            logger.info("FTS isHttpFileTransfer ");
        }
        return (session instanceof HttpFileTransferSession);
    }
    
    private boolean isSessionPaused(FileSharingSession session) {
        if (session == null) {
            if (logger.isActivated()) {
                logger.info("isSessionPaused session is null");
            }
            return false;
        }
        if (!isHttpFileTransfer()) {
            if (logger.isActivated()) {
                logger.info("Pause available only for HTTP transfer");
            }
            return false;
        }
        return session.isFileTransferPaused();
    }

    /**
     * Returns the transfer type
     *
     * @return TransferType
     * @see FileTransfer.Type
     */
    public String getTransferType() {
        String result = FileTransfer.Type.NORMAL;
        
        if (session.isBurnMessage()) {
            result = FileTransfer.Type.BURNED;
        } else if (session.isPublicChatFile()) {
            result = FileTransfer.Type.PUBACCOUNT;
        }

        return result;
    }
    
    public boolean isTransferFromSecondaryDevice() {
        return session.toSecondary();
    }

    /**
     * Accepts file transfer invitation
     */
    public void acceptInvitation() {
        if (logger.isActivated()) {
            logger.info("FTS Accept session invitation");
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
     * Rejects file transfer invitation
     */
    public void rejectInvitation() {
        if (logger.isActivated()) {
            logger.info("FTS Reject session invitation");
        }
        
        synchronized (lock) {
            mFileService.removeFileTransferSession(mFileTransferId);
            setStateAndReasonCode(State.REJECTED, ReasonCode.REJECTED_BY_USER);
        }

          // Reject invitation
        Thread t = new Thread() {
            public void run() {
                session.rejectSession(603);
            }
        };
        t.start();
    }

    /**
     * Aborts the file transfer
     */
    public void abortTransfer() {
        if (logger.isActivated()) {
            logger.info("FTS Cancel session");
        }
        
        if (session == null) {
            try{
                /*
                 * File transfer can be aborted only if it is in state QUEUED/ PAUSED when
                 * there is no session.
                 */
                State state = mPersistentStorage.getState();
                switch (state) {
                    case QUEUED:
                    case PAUSED:
                        setStateAndReasonCode(State.ABORTED,
                                ReasonCode.ABORTED_BY_SYSTEM);
                        return;
                    default:
                        logger.error("Session with file transfer ID '" + mFileTransferId
                                + "' not available!");
                        return;
                }
            } catch(Exception e){
                e.printStackTrace();
                return;
            }
        }
        
        if (session.isFileTransfered()) {
            // File already transferred and session automatically closed after transfer
            return;
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
     * Pauses the file transfer
     */
    public void pauseTransfer() {
        if (logger.isActivated()) {
            logger.info("FTS pauseTransfer12");
        }
        session.fileTransferPaused();
        // TODO
        ((HttpFileTransferSession)session).pauseFileTransfer();
    }
    
    /**
     * Resumes the file transfer
     */
    public void resumeTransfer() {
        if (logger.isActivated()) {
            logger.info("FTS resumeTransfer2");
        }
        // TODO
        ((HttpFileTransferSession)session).resumeFileTransfer();
    }    
    
    /*------------------------------- SESSION EVENTS ----------------------------------*/
    
    /**
     * Session is started
     */
    public void handleSessionStarted() {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("FTS Session started");
            }
            // Update rich messaging history
            RichMessagingHistory.getInstance().updateFileTransferStatus(session.getSessionID(), FileTransfer.State.STARTED.toInt());

            // Notify event listeners
            final int N = groupftlisteners.beginBroadcast();
            if (logger.isActivated()) {
                logger.info("FTS Session started N:" + N);
            }
            for (int i=0; i < N; i++) {
                try {
                    //listeners.getBroadcastItem(i).onTransferStarted();
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("FTS Can't notify listener", e);
                    }
                }
            }
            groupftlisteners.finishBroadcast();        
        }
    }
    
    public void handleTransferTerminated(){
        if (logger.isActivated()) {
                logger.info("FTS Session handleTransferTerminated");
            }
        // Remove session from the list
            FileTransferServiceImpl.removeGroupFileTransferSession(session.getSessionID());
            removeFileListener();
      }
    
    public void handleInviteError(FileSharingError error){
        if (logger.isActivated()) {
            logger.info("FTS Session handleInviteError code: " + error.getErrorCode());
        }
        /*// Update rich messaging history
        RichMessagingHistory.getInstance().updateFileTransferStatus(session.getSessionID(), FileTransfer.State.FAILED.toInt());*/
    
        ReasonCode code = FileTransfer.ReasonCode.UNSPECIFIED;
        try {
            switch(error.getErrorCode()) {
                case FileSharingError.SESSION_INITIATION_FAILED:
                    code = FileTransfer.ReasonCode.REJECTED_BY_SYSTEM;
                    break;
                default:
                    code = FileTransfer.ReasonCode.UNSPECIFIED;
            }
            setStateAndReasonCode(FileTransfer.State.FAILED, code);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("FTS Can't notify listener", e);
            }
        }
        
        // Remove session from the list
        FileTransferServiceImpl.removeFileTransferSession(session.getSessionID());
        removeFileListener();

      }
    
    /**
     * Session has been aborted
     * 
     * @param reason Termination reason
     */
    public void handleSessionAborted(int reason) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("FTS Session aborted (reason " + reason + ")");
            }
    
            /*// Update rich messaging history
            RichMessagingHistory.getInstance().updateFileTransferStatus(session.getSessionID(), FileTransfer.State.ABORTED.toInt());*/
            
            try {
                switch (reason) {
                    case ImsServiceSession.TERMINATION_BY_TIMEOUT:
                    case ImsServiceSession.TERMINATION_BY_SYSTEM:
                        setStateAndReasonCode(State.ABORTED, ReasonCode.ABORTED_BY_SYSTEM);
                        break;
                    case ImsServiceSession.TERMINATION_BY_CONNECTION_LOST:
                        setStateAndReasonCode(State.FAILED, ReasonCode.FAILED_DATA_TRANSFER);
                        break;
                    case ImsServiceSession.TERMINATION_BY_USER:
                        setStateAndReasonCode(State.ABORTED, ReasonCode.ABORTED_BY_USER);
                        break;
                    case ImsServiceSession.TERMINATION_BY_REMOTE:
                        /*
                         * TODO : Fix sending of SIP BYE by sender once transfer is completed and media
                         * session is closed. Then this check of state can be removed.
                         */
                        if (State.TRANSFERRED != mPersistentStorage.getState()) {
                            setStateAndReasonCode(State.ABORTED, ReasonCode.ABORTED_BY_REMOTE);
                        }
                        break;
                    default:
                        throw new IllegalArgumentException(
                                "Unknown reason in OneToOneFileTransferImpl.handleSessionAborted; terminationReason="
                                        + reason + "!");
                }
            } catch(Exception e){
                if (logger.isActivated()) {
                    logger.info("FTS Session aborted exception");
                }
                e.printStackTrace();
            }
            
            // Remove session from the list
            FileTransferServiceImpl.removeGroupFileTransferSession(session.getSessionID());
            removeFileListener();
        }
    }
    
    /**
     * Session has been terminated by remote
     */
    public void handleSessionTerminatedByRemote() {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("FTS Session terminated by remote");
            }
    
            // Check if the file has been transferred or not
              if (session.isFileTransfered()) {
                // Remove session from the list
                  FileTransferServiceImpl.removeGroupFileTransferSession(session.getSessionID());
              } else {
                  /*// Update rich messaging history
                  RichMessagingHistory.getInstance().updateFileTransferStatus(session.getSessionID(), FileTransfer.State.ABORTED.toInt());*/
                  handleSessionAborted(ImsServiceSession.TERMINATION_BY_USER);
              }
        }
    }
    
    /**
     * File transfer error
     * 
     * @param error Error
     */
    public void handleTransferError(FileSharingError error) {
        synchronized(lock) {
            if (error.getErrorCode() == FileSharingError.SESSION_INITIATION_CANCELLED) {
                // Do nothing here, this is an aborted event
                if (logger.isActivated()) {
                    logger.info("FTS Sharing error aborted event");
                }
                return;
            }

            if (logger.isActivated()) {
                logger.info("FTS Sharing error " + error.getErrorCode());
            }

            ReasonCode code = FileTransfer.ReasonCode.UNSPECIFIED;
            try {
                switch(error.getErrorCode()) {
                    case FileSharingError.SESSION_INITIATION_DECLINED:
                        code = FileTransfer.ReasonCode.REJECTED_BY_REMOTE;
                        break;
                    case FileSharingError.MEDIA_SAVING_FAILED:
                        code = FileTransfer.ReasonCode.FAILED_SAVING;
                        break;
                    case FileSharingError.MEDIA_FALLBACK_MMS:
                        code = FileTransfer.ReasonCode.FAILED_FALLBACK_MMS;
                        break;
                    case FileSharingError.MEDIA_SIZE_TOO_BIG:
                        code = FileTransfer.ReasonCode.REJECTED_MAX_SIZE;
                        break;
                    case FileSharingError.MEDIA_TRANSFER_FAILED:
                        code = FileTransfer.ReasonCode.REJECTED_MEDIA_FAILED;
                        break;
                    case FileSharingError.MEDIA_RESUME_FAILED:
                        code = FileTransfer.ReasonCode.PAUSED_BY_SYSTEM;
                        break;
                    default:
                        code = FileTransfer.ReasonCode.UNSPECIFIED;
                }
                synchronized (lock) {
                    mFileService.removeFileTransferSession(mFileTransferId);
                    setStateAndReasonCode(FileTransfer.State.FAILED, code);
                }
            }
            catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("FTS Can't notify listener", e);
                }
            }
            
            /* // Update rich messaging history
            RichMessagingHistory.getInstance().updateFileTransferStatus(session.getSessionID(), FileTransfer.State.FAILED.toInt());*/
            
            // Remove session from the list
            FileTransferServiceImpl.removeGroupFileTransferSession(session.getSessionID());
            removeFileListener();
        }
    }
    
    /**
     * File transfer progress
     * 
     * @param currentSize Data size transferred 
     * @param totalSize Total size to be transferred
     */
    public void handleTransferProgress(long currentSize, long totalSize) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.debug("Sharing progress currentSize: " + currentSize);
            }
            
            if(session.isFileTransferPaused()){
                currentSize += session.getPauseInfo().bytesTransferrred;
                if (logger.isActivated()) {
                    logger.debug("Sharing progress Resume file size: " + currentSize + "FT ID is " + session.getSessionID());
                }
            }

            /*// Update rich messaging history
            RichMessagingHistory.getInstance().updateFileTransferProgress(session.getSessionID(), currentSize, totalSize);*/

            if (mPersistentStorage.setProgress(currentSize)) {
                mBroadcaster.broadcastProgressUpdate(mChatId, mFileTransferId, currentSize,
                        totalSize);
            }    
         }
    }
    
    /**
     * File has been transfered
     * 
     * @param filename Filename associated to the received content
     */
    public void handleFileTransfered(String filename,MmContent fileContent) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Content transferred");
            }
    
           /* // Update rich messaging history
            RichMessagingHistory.getInstance().updateFileTransferUrl(session.getSessionID(), filename);*/
    
            long deliveryExpiration = 0;
            try {
                if (mPersistentStorage.setTransferred(fileContent, FileTransferData.UNKNOWN_EXPIRATION, FileTransferData.UNKNOWN_EXPIRATION,
                        deliveryExpiration)) {
                    mBroadcaster.broadcastStateChanged(mChatId, mFileTransferId, State.TRANSFERRED,
                            ReasonCode.UNSPECIFIED);
                }
            }catch(Exception e){
                if (logger.isActivated()) {
                    logger.info("Content transferred exception");
                }
                e.printStackTrace();
            }
            
            // Remove session from the list            
            FileTransferServiceImpl.removeGroupFileTransferSession(session.getSessionID());
            removeFileListener();
        }    
    }

    public void removeFileListener() {
        synchronized(lock) {
            if (logger.isActivated()) {
                    logger.info("removeFileListener session ID: " + session.getSessionID());
            }
            session.removeListener(this);
        }
    }
    
    /**
     * File transfer has been paused
     */
    public void handleFileTransferPaused() {
        // TODO
    }

    /**
     * File transfer has been resumed
     */
    public void handleFileTransferResumed() {
        // TODO
    }
    
    public void onSessionInvited() {
        if (logger.isActivated()) {
            logger.info("Invited to one-to-one file transfer session");
        }
        String contactString = session.getRemoteContact();
        ContactId contact = ContactIdUtils.createContactIdFromTrustedData(contactString);
        MmContent file = session.getContent();
        MmContent fileIcon = session.getFileicon();
        long timestamp = new Date().getTime();
        long timestampSent = new Date().getTime();        
        
        synchronized (lock) {
                mPersistentStorage.addIncomingGroupFileTransfer(mChatId,contact, file,
                        fileIcon, State.INVITED, ReasonCode.UNSPECIFIED, timestamp, timestampSent,
                        FileTransferData.UNKNOWN_EXPIRATION, FileTransferData.UNKNOWN_EXPIRATION);
        }

        ((GroupFileTransferBroadcaster)mBroadcaster).broadcastInvitation(mFileTransferId,true,session);
    }
    
    private void setStateAndReasonCode(State state, ReasonCode reasonCode) {
        if (logger.isActivated()) {
            logger.info("setStateAndReasonCode: state: " + state.toInt() + ", reasonCode: " + reasonCode.toInt() + ", chatId: " + mChatId);
        }
        if (mPersistentStorage.setStateAndReasonCode(state, reasonCode)) {
            mBroadcaster.broadcastStateChanged(mChatId, mFileTransferId, state, reasonCode);
        }
    }
}
