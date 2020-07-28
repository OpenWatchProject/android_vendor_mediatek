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
import com.gsma.services.rcs.ft.IGroupFileTransferListener;
import com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo;

import android.content.Intent;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * GroupFileTransferBroadcaster maintains the registering and unregistering of
 * IGroupFileTransferListener and also performs broadcast events on these listeners upon the trigger
 * of corresponding callbacks.
 */
public class GroupFileTransferBroadcaster implements IGroupFileTransferBroadcaster {

    private final RemoteCallbackList<IGroupFileTransferListener> mGroupFileTransferListeners = new RemoteCallbackList<>();

    private final Logger logger = Logger.getLogger(getClass().getName());

    public GroupFileTransferBroadcaster() {
    }

    public void addGroupFileTransferListener(IGroupFileTransferListener listener) {
        if (logger.isActivated()) {
            logger.info("addGroupFileTransferListener listener " + listener);
        }
        mGroupFileTransferListeners.register(listener);
    }

    public void removeGroupFileTransferListener(IGroupFileTransferListener listener) {
        if (logger.isActivated()) {
            logger.info("removeGroupFileTransferListener listener " + listener);
        }
        mGroupFileTransferListeners.unregister(listener);
    }

    @Override
    public void broadcastStateChanged(String chatId, String transferId, State state,
            ReasonCode reasonCode) {
        final int N = mGroupFileTransferListeners.beginBroadcast();
        if (logger.isActivated()) {
            logger.info("broadcastStateChanged chatId " + chatId + ", transferId: " + transferId + ", state: " + state.toInt()
                    + ", reasonCode: " + reasonCode + ", N: " + N);
        }
        int rcsState = state.toInt();
        int rcsReasonCode = reasonCode.toInt();
        for (int i = 0; i < N; i++) {
            try {
                mGroupFileTransferListeners.getBroadcastItem(i).onStateChanged(chatId, transferId,
                        rcsState, rcsReasonCode);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mGroupFileTransferListeners.finishBroadcast();
    }

    @Override
    public void broadcastProgressUpdate(String chatId, String transferId, long currentSize,
            long totalSize) {
        final int N = mGroupFileTransferListeners.beginBroadcast();
        if (logger.isActivated()) {
            logger.info("broadcastProgressUpdate chatId " + chatId + ", transferId: " + transferId + ", currentSize: " + currentSize
                    + ", currentSize: " + currentSize + ", N: " + N);
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupFileTransferListeners.getBroadcastItem(i).onProgressUpdate(chatId,
                        transferId, currentSize, totalSize);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mGroupFileTransferListeners.finishBroadcast();
    }

    @Override
    public void broadcastDeliveryInfoChanged(String chatId, ContactId contact, String transferId,
            GroupDeliveryInfo.Status status, GroupDeliveryInfo.ReasonCode reasonCode) {
        int rcsStatus = status.toInt();
        int rcsReasonCode = reasonCode.toInt();
        final int N = mGroupFileTransferListeners.beginBroadcast();
        if (logger.isActivated()) {
            logger.info("broadcastDeliveryInfoChanged chatId " + chatId + ", transferId: " + transferId + ", contact: " + contact.toString()
                    + ", status: " + status.toInt() + ", reasonCode: " + reasonCode.toInt() + ", N: " + N);
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupFileTransferListeners.getBroadcastItem(i).onDeliveryInfoChanged(chatId,
                        contact, transferId, rcsStatus, rcsReasonCode);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener per contact", e);
                }
            }
        }
        mGroupFileTransferListeners.finishBroadcast();
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
    public void broadcastFileTransfersDeleted(String chatId, Set<String> transferIds) {
        List<String> ids = new ArrayList<>(transferIds);
        final int N = mGroupFileTransferListeners.beginBroadcast();
        if (logger.isActivated()) {
            logger.info("broadcastFileTransfersDeleted chatId " + chatId + ", transferIds: " + transferIds.size());
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupFileTransferListeners.getBroadcastItem(i).onDeleted(chatId, ids);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener per contact", e);
                }
            }
        }
        mGroupFileTransferListeners.finishBroadcast();
    }
}
