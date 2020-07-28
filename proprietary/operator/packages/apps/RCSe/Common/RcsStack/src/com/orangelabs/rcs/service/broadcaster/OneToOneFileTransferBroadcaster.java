/*
 * Copyright (C) 2014 Sony Mobile Communications Inc.
 * Copyright (C) 2010-2016 Orange.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.orangelabs.rcs.service.broadcaster;

import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.FileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.TerminatingGroupFileSharingSession;
import com.orangelabs.rcs.core.ims.service.im.filetransfer.http.HttpFileTransferSession;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.utils.IntentUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.ft.FileTransfer.ReasonCode;
import com.gsma.services.rcs.ft.FileTransfer.State;
import com.gsma.services.rcs.ft.FileTransferIntent;
import com.gsma.services.rcs.ft.IOneToOneFileTransferListener;

import android.content.Intent;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * OneToOneFileTransferBroadcaster maintains the registering and unregistering of
 * IFileTransferListener and also performs broadcast events on these listeners upon the trigger of
 * corresponding callbacks.
 */
public class OneToOneFileTransferBroadcaster implements IOneToOneFileTransferBroadcaster {

    private final RemoteCallbackList<IOneToOneFileTransferListener> mOneToOneFileTransferListeners = new RemoteCallbackList<>();

    private final Logger logger = Logger.getLogger(getClass().getName());

    public OneToOneFileTransferBroadcaster() {
    }

    public void addOneToOneFileTransferListener(IOneToOneFileTransferListener listener) {
        if (logger.isActivated()) {
            logger.info("addOneToOneFileTransferListener listener " + listener);
        }
        mOneToOneFileTransferListeners.register(listener);
    }

    public void removeOneToOneFileTransferListener(IOneToOneFileTransferListener listener) {
        if (logger.isActivated()) {
            logger.info("removeOneToOneFileTransferListener listener " + listener);
        }
        mOneToOneFileTransferListeners.unregister(listener);
    }

    @Override
    public void broadcastStateChanged(ContactId contact, String transferId, State state,
            ReasonCode reasonCode) {
        final int N = mOneToOneFileTransferListeners.beginBroadcast();
        if(contact != null){
            if (logger.isActivated()) {
                logger.info("broadcastStateChanged contact " + contact + ", transferId: " + transferId + ", state: " + state.toInt()
                        + ", reasonCode: " + reasonCode + ", N: " + N);
            }
        } else {
            if (logger.isActivated()) {
                logger.info("broadcastStateChanged contact " + ", transferId: " + transferId + ", state: " + state.toInt()
                        + ", reasonCode: " + reasonCode + ", N: " + N);
            }
        }
        
        int rcsState = state.toInt();
        int rcsReasonCode = reasonCode.toInt();
        for (int i = 0; i < N; i++) {
            try {
                mOneToOneFileTransferListeners.getBroadcastItem(i).onStateChanged(contact,
                        transferId, rcsState, rcsReasonCode);
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mOneToOneFileTransferListeners.finishBroadcast();
    }

    @Override
    public void broadcastProgressUpdate(ContactId contact, String transferId, long currentSize,
            long totalSize) {
        final int N = mOneToOneFileTransferListeners.beginBroadcast();
        if(contact == null){
            if (logger.isActivated()) {
                logger.info("broadcastProgressUpdate transferId: " + transferId + ", currentSize: " + currentSize
                        + ", totalSize: " + totalSize + ", N: " + N);
            }
        } else {
            if (logger.isActivated()) {
                logger.info("broadcastProgressUpdate contact " + contact.toString() + ", transferId: " + transferId + ", currentSize: " + currentSize
                        + ", totalSize: " + totalSize + ", N: " + N);
            }
        }
        
        for (int i = 0; i < N; i++) {
            try {
                mOneToOneFileTransferListeners.getBroadcastItem(i).onProgressUpdate(contact,
                        transferId, currentSize, totalSize);
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mOneToOneFileTransferListeners.finishBroadcast();
    }

    @Override
    public void broadcastInvitation(String fileTransferId) {
        Intent invitation = new Intent(FileTransferIntent.ACTION_NEW_INVITATION);
        if (logger.isActivated()) {
            logger.info("broadcastInvitation fileTransferId " + fileTransferId);
        }
        invitation.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(invitation);
        invitation.putExtra(FileTransferIntent.EXTRA_TRANSFER_ID, fileTransferId);
        AndroidFactory.getApplicationContext().sendBroadcast(invitation,
                                                             "com.gsma.services.permission.RCS");
    }

    public void broadcastInvitation(String fileTransferId,boolean isGroup, FileSharingSession session) {
        Intent invitation = new Intent(FileTransferIntent.ACTION_NEW_INVITATION);
        if (logger.isActivated()) {
            logger.info("broadcastInvitation fileTransferId " + fileTransferId + ", isGroup: " + isGroup);
        }
        invitation.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(invitation);
        invitation.putExtra(FileTransferIntent.EXTRA_TRANSFER_ID, fileTransferId);
        invitation.putExtra("isGroupTransfer", isGroup);
        invitation.putExtra("autoAccept", session.shouldAutoAccept());
        invitation.putExtra(FileTransferIntent.EXTRA_MSG_ID, session.getMessageId());
        if (logger.isActivated()) {
            logger.info("broadcastInvitation fileTransferId " + fileTransferId + ", msgId: " + session.getMessageId() + ",autoAccept" + session.shouldAutoAccept());
        }
        String chatSessionId = null;
        if (session instanceof HttpFileTransferSession) {
            chatSessionId = ((HttpFileTransferSession) session).getChatSessionID();
            invitation.putExtra("chatSessionId", chatSessionId);
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
                logger.info("broadcastInvitation Receive file transfer invitation: " + "Chatsessionid: "
                        + chatSessionId);
            }
            invitation.putExtra("chatSessionId", chatSessionId);
        }

        AndroidFactory.getApplicationContext().sendBroadcast(invitation,
                                                             "com.gsma.services.permission.RCS");
    }


    @Override
    public void broadcastFileTransferDeleted(ContactId contact, Set<String> filetransferIds) {
        List<String> ids = new ArrayList<>(filetransferIds);
        final int N = mOneToOneFileTransferListeners.beginBroadcast();
        if (logger.isActivated()) {
            logger.info("broadcastFileTransferDeleted contact " + contact.toString() + ", fileIds: " + filetransferIds.size() + ", N: " + N);
        }
        for (int i = 0; i < N; i++) {
            try {
                mOneToOneFileTransferListeners.getBroadcastItem(i).onDeleted(contact, ids);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mOneToOneFileTransferListeners.finishBroadcast();
    }
}
