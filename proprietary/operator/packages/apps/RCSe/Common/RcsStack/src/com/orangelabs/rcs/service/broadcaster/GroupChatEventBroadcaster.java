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

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.core.ims.service.im.chat.GroupChatSession;
import com.orangelabs.rcs.utils.IntentUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import com.gsma.services.rcs.chat.ChatLog.Message.Content;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.Status;
import com.gsma.services.rcs.chat.GroupChat.ParticipantStatus;
import com.gsma.services.rcs.chat.GroupChat.ReasonCode;
import com.gsma.services.rcs.chat.GroupChat.State;
import com.gsma.services.rcs.chat.GroupChatIntent;
import com.gsma.services.rcs.chat.IGroupChatListener;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.groupdelivery.GroupDeliveryInfo;

import android.content.Intent;
import android.os.RemoteCallbackList;
import android.os.RemoteException;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * GroupChatEventBroadcaster maintains the registering and unregistering of IGroupChatListener and
 * also performs broadcast events on these listeners upon the trigger of corresponding callbacks.
 */
public class GroupChatEventBroadcaster implements IGroupChatEventBroadcaster {

    private final RemoteCallbackList<IGroupChatListener> mGroupChatListeners = new RemoteCallbackList<>();

    private final Logger logger = Logger.getLogger(getClass().getName());

    public GroupChatEventBroadcaster() {
    }

    public void addGroupChatEventListener(IGroupChatListener listener) {
        if (logger.isActivated()) {
            //logger.info("addGroupChatEventListener listener " + listener);
        }
        mGroupChatListeners.register(listener);
    }

    public void removeGroupChatEventListener(IGroupChatListener listener) {
        if (logger.isActivated()) {
            //logger.info("removeGroupChatEventListener listener" + listener);
        }
        mGroupChatListeners.unregister(listener);
    }

    @Override
    public void broadcastMessageStatusChanged(String chatId, String mimeType, String msgId,
            Status status, Content.ReasonCode reasonCode) {
        int rcsStatus = status.toInt();
        int rcsReasonCode = reasonCode.toInt();
        final int N = mGroupChatListeners.beginBroadcast();
        if (logger.isActivated()) {
            /*logger.info("broadcastMessageStatusChanged chatId: " + chatId + ", msgId: " + msgId + ", status: " + status.toInt()
                    + ", reasonCode: " + reasonCode + ", mime: " + mimeType + ", N: " + N);*/
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupChatListeners.getBroadcastItem(i).onMessageStatusChanged(chatId, mimeType,
                        msgId, rcsStatus, rcsReasonCode);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener.", e);
                }
            }
        }
        mGroupChatListeners.finishBroadcast();
    }

    @Override
    public void broadcastMessageGroupDeliveryInfoChanged(String chatId, ContactId contact,
            String apiMimeType, String msgId, GroupDeliveryInfo.Status status,
            GroupDeliveryInfo.ReasonCode reasonCode) {
        final int N = mGroupChatListeners.beginBroadcast();
        if (logger.isActivated()) {
            /*logger.info("broadcastMessageGroupDeliveryInfoChanged chatId: " + chatId + ", msgId: " + msgId + ", status: " + status.toInt()
                    + ", reasonCode: " + reasonCode + ", contact: " + contact.toString() + ", N: " + N);*/
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupChatListeners.getBroadcastItem(i).onMessageGroupDeliveryInfoChanged(chatId,
                        contact, apiMimeType, msgId, status.toInt(), reasonCode.toInt());
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener.", e);
                }
            }
        }
        mGroupChatListeners.finishBroadcast();
    }

    @Override
    public void broadcastParticipantStatusChanged(String chatId, ContactId contact,
            ParticipantStatus status) {
        final int N = mGroupChatListeners.beginBroadcast();
        if (logger.isActivated()) {
            /*logger.info("broadcastParticipantStatusChanged chatId: " + chatId + ", status: " + status.toInt() +
                    ", contact: " + contact.toString() + ", N: " + N);*/
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupChatListeners.getBroadcastItem(i).onParticipantStatusChanged(chatId, contact,
                        status.toInt());
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mGroupChatListeners.finishBroadcast();
    }

    @Override
    public void broadcastStateChanged(String chatId, State state, ReasonCode reasonCode) {
        final int N = mGroupChatListeners.beginBroadcast();
        if (logger.isActivated()) {
            /*logger.info("broadcastStateChanged chatId: " + chatId + ", state: " + state.toInt() +
                    ", reasonCode: " + reasonCode.toInt() + ", N: " + N);*/
        }
        int rcsState = state.toInt();
        int rcsReasonCode = reasonCode.toInt();
        for (int i = 0; i < N; i++) {
            try {
                mGroupChatListeners.getBroadcastItem(i).onStateChanged(chatId, rcsState,
                        rcsReasonCode);
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mGroupChatListeners.finishBroadcast();
    }

    @Override
    public void broadcastComposingEvent(String chatId, ContactId contact, boolean status) {
        final int N = mGroupChatListeners.beginBroadcast();
        if (logger.isActivated()) {
            //logger.info("broadcastComposingEvent chatId: " + chatId + ", status: " + status  + ", N: " + N);
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupChatListeners.getBroadcastItem(i).onComposingEvent(chatId, contact, status);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        mGroupChatListeners.finishBroadcast();
    }

    public void broadcastInvitation(String chatId, boolean isClosed, boolean offline, boolean isChatExist, GroupChatSession session) {
        Intent invitation = new Intent(GroupChatIntent.ACTION_NEW_INVITATION);
        if (logger.isActivated()) {
            /*logger.info("broadcastInvitation chatId: " + chatId + ", isClosed: " + isClosed  + ", offline: " + offline +
                    ", isChatExist: " + isChatExist);*/
        }
        boolean autoAccept = RcsSettings.getInstance().isGroupChatAutoAccepted();

        invitation.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(invitation);
        invitation.putExtra(GroupChatIntent.EXTRA_CHAT_ID, chatId);
        invitation.putExtra("isClosedGroupChat", session.isClosedGroup());
        invitation.putExtra(GroupChatIntent.EXTRA_DISPLAY_NAME, session.getGroupRemoteDisplayName());
        if (RcsSettings.getInstance().supportOP01()) {
            invitation.putExtra("offlineInvite", offline);
        }
        invitation.putExtra("isGroupChatExist", isChatExist);
        //invitation.putExtra(GroupChatIntent.EXTRA_SUBJECT, sessionApi.getSubject());
        invitation.putExtra("autoAccept", autoAccept);
        AndroidFactory.getApplicationContext().sendBroadcast(invitation,
                                                             "com.gsma.services.permission.RCS");
    }

    public void broadcastInvitation(String chatId) {
        Intent invitation = new Intent(GroupChatIntent.ACTION_NEW_INVITATION);
        if (logger.isActivated()) {
            //logger.info("broadcastInvitation chatId: " + chatId);
        }
        invitation.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(invitation);
        invitation.putExtra(GroupChatIntent.EXTRA_CHAT_ID, chatId);
        AndroidFactory.getApplicationContext().sendBroadcast(invitation,
                                                             "com.gsma.services.permission.RCS");
    }

    @Override
    public void broadcastMessageReceived(String apiMimeType, String msgId) {
        Intent newGroupChatMessage = new Intent(GroupChatIntent.ACTION_NEW_GROUP_CHAT_MESSAGE);
        if (logger.isActivated()) {
            //logger.info("broadcastMessageReceived msgId: " + msgId + ", apiMime: " + apiMimeType);
        }
        newGroupChatMessage.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(newGroupChatMessage);
        newGroupChatMessage.putExtra(GroupChatIntent.EXTRA_MIME_TYPE, apiMimeType);
        newGroupChatMessage.putExtra(GroupChatIntent.EXTRA_MESSAGE_ID, msgId);
        AndroidFactory.getApplicationContext().sendBroadcast(newGroupChatMessage, "com.gsma.services.permission.RCS");
    }

    public void broadcastMessageReceived(String apiMimeType, String msgId, String displayName) {
        Intent newGroupChatMessage = new Intent(GroupChatIntent.ACTION_NEW_GROUP_CHAT_MESSAGE);
        if (logger.isActivated()) {
            //logger.info("broadcastMessageReceived msgId: " + msgId + ", apiMime: " + apiMimeType + ",displayName: " + displayName);
        }
        newGroupChatMessage.addFlags(Intent.FLAG_EXCLUDE_STOPPED_PACKAGES);
        IntentUtils.tryToSetReceiverForegroundFlag(newGroupChatMessage);
        newGroupChatMessage.putExtra(GroupChatIntent.EXTRA_MIME_TYPE, apiMimeType);
        newGroupChatMessage.putExtra(GroupChatIntent.EXTRA_MESSAGE_ID, msgId);
        newGroupChatMessage.putExtra(GroupChatIntent.EXTRA_DISPLAY_NAME, displayName);
        AndroidFactory.getApplicationContext().sendBroadcast(newGroupChatMessage, "com.gsma.services.permission.RCS");
    }

    @Override
    public void broadcastMessagesDeleted(String chatId, Set<String> msgIds) {
        List<String> msgIds2 = new ArrayList<>(msgIds);
        final int N = mGroupChatListeners.beginBroadcast();
        if (logger.isActivated()) {
            //logger.info("broadcastMessageReceived chatId: " + chatId + ", N: " + N + ", size: " + msgIds.size());
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupChatListeners.getBroadcastItem(i).onMessagesDeleted(chatId, msgIds2);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener.", e);
                }
            }
        }
        mGroupChatListeners.finishBroadcast();
    }

    @Override
    public void broadcastGroupChatsDeleted(Set<String> chatIds) {
        List<String> ids = new ArrayList<>(chatIds);
        final int N = mGroupChatListeners.beginBroadcast();
        if (logger.isActivated()) {
            //logger.info("broadcastMessageReceived chatIds: " + chatIds.size() + ", N: " + N);
        }
        for (int i = 0; i < N; i++) {
            try {
                mGroupChatListeners.getBroadcastItem(i).onDeleted(ids);
            } catch (RemoteException e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener.", e);
                }
            }
        }
        mGroupChatListeners.finishBroadcast();
    }
}
