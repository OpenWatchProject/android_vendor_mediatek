/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcse.mvc;

import android.os.Message;
import android.os.ParcelUuid;

import com.mediatek.rcse.activities.widgets.GroupChatWindow;
import com.mediatek.rcse.activities.widgets.MultiChatWindow;
import com.mediatek.rcse.activities.widgets.OneOneChatWindow;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
import com.mediatek.rcse.fragments.ChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment;
import com.mediatek.rcse.fragments.GroupChatFragment.SentFileTransfer;
import com.mediatek.rcse.interfaces.ChatController;
import com.mediatek.rcse.interfaces.ChatView.IChatEventInformation;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowManager;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.IFileTransfer;
import com.mediatek.rcse.interfaces.ChatView.IGroupChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IMultiChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IOne2OneChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IReceivedChatMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ModelImpl.ChatEventStruct;
import com.mediatek.rcse.mvc.ChatMessage;
import com.mediatek.rcse.mvc.ModelImpl.FileStruct;
import com.mediatek.rcse.mvc.view.ReceivedChatMessage;
import com.mediatek.rcse.mvc.view.SentChatMessage;
import com.mediatek.rcse.provider.RichMessagingDataProvider;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.GroupChatWindowAdapter;
import com.mediatek.rcse.service.binder.ChatWindowManagerAdapter.One2OneChatWindowAdapter;
import com.mediatek.rcse.service.MediatekFactory;
import com.orangelabs.rcs.core.ims.service.im.chat.event.User;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

//import com.gsma.services.rcs.chat.ChatMessage;

/**
 * This is the virtual View part in the MVC pattern.
 */
public class ViewImpl implements IChatWindowManager {
    public static final String TAG = "ViewImpl";
    private static volatile ViewImpl sInstance = null;
    private final CopyOnWriteArrayList<IChatWindowManager> mChatWindowManagerList =
            new CopyOnWriteArrayList<IChatWindowManager>();
    private final ConcurrentHashMap<Object, ChatWindowDispatcher> mChatWindowDispatcherMap =
            new ConcurrentHashMap<Object, ViewImpl.ChatWindowDispatcher>();

    /**
     * Gets the single instance of ViewImpl.
     *
     * @return single instance of ViewImpl
     */
    public static synchronized ViewImpl getInstance() {
        if (null == sInstance) {
            sInstance = new ViewImpl();
        }
        return sInstance;
    }

    /**
     * Instantiates a new view impl.
     */
    protected ViewImpl() {
    }

    /**
     * Adds the group chat window.
     *
     * @param tag the tag
     * @param participantList the participant list
     * @return the i group chat window
     */
    @Override
    public IGroupChatWindow addGroupChatWindow(Object tag,
            List<ParticipantInfo> participantList) {
        Logger.v(TAG, "addGroupChatWindow() entry, the tag is "
                + tag + ",participantList:" + participantList);
        GroupChatWindowDispatcher chatWindow = new GroupChatWindowDispatcher(
                tag, (CopyOnWriteArrayList<ParticipantInfo>) participantList);
        chatWindow.setmChatID(participantList.get(0).getmChatID());
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            chatWindow.onAddChatWindowManager(chatWindowManager);
        }
        mChatWindowDispatcherMap.put(tag, chatWindow);
        return chatWindow;
    }

    /**
     * Adds multi chat window.
     *
     * @param tag the tag
     * @param participantList the participant list
     * @return The tag of this chat window
     */
    @Override
    public IMultiChatWindow addMultiChatWindow(Object tag,
            List<ParticipantInfo> participantList) {
        MultiChatWindowDispatcher chatWindow = new MultiChatWindowDispatcher(
                tag, (CopyOnWriteArrayList<ParticipantInfo>) participantList);
        //chatWindow.setmChatID(participantList.get(0).getmChatID());
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            chatWindow.onAddChatWindowManager(chatWindowManager);
        }
        mChatWindowDispatcherMap.put(tag, chatWindow);
        return chatWindow;
    }

    /**
     * Adds the one2 one chat window.
     *
     * @param tag the tag
     * @param participant the participant
     * @return the i one2 one chat window
     */
    @Override
    public IOne2OneChatWindow addOne2OneChatWindow(Object tag,
            Participant participant) {
        Logger.v(TAG, "addOne2OneChatWindow() entry, the tag is "
                + tag + ",participant:" + participant);
        One2OneChatWindowDispatcher chatWindow = new One2OneChatWindowDispatcher(
                tag, participant);
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            chatWindow.onAddChatWindowManager(chatWindowManager);
        }
        mChatWindowDispatcherMap.put(tag, chatWindow);
        return chatWindow;
    }

    /**
     * Removes the chat window.
     *
     * @param chatWindow the chat window
     * @return true, if successful
     */
    @Override
    public boolean removeChatWindow(IChatWindow chatWindow) {
        if (chatWindow != null && chatWindow instanceof ChatWindowDispatcher) {
            ((ChatWindowDispatcher) chatWindow).onDestroy();
            return mChatWindowDispatcherMap.values().remove(chatWindow);
        } else {
            Logger.e(TAG,
                    "removeChatWindow() remove IChatWindow, invalid chatWindow :"
                            + chatWindow);
            return false;
        }
    }

    /**
     * Called by a single View to register itself to the Module.
     *
     * @param chatWindowManager
     *            The chat window manager of the View.
     * @param autoShow
     *            True if it should automatically show all window, else false.
     */
    public synchronized void addChatWindowManager(
            IChatWindowManager chatWindowManager, boolean autoShow) {

        if (null != chatWindowManager) {
            if (mChatWindowManagerList.contains(chatWindowManager)) {
            }
            mChatWindowManagerList.add(chatWindowManager);
            if (!autoShow) {
                Logger.v(TAG, "addChatWindowManager autoShow is false.");
                return;
            }
            for (ChatWindowDispatcher chatWindowDispatcher : mChatWindowDispatcherMap
                    .values()) {
                chatWindowDispatcher.onAddChatWindowManager(chatWindowManager);
            }
        } else {
            Logger.e(TAG, "addChatWindowManager() chatWindowManager is null");
        }
    }

    /**
     * Called by a single View to unregister itself from the Module.
     *
     * @param chatWindowManager            The chat window manager of the View
     */
    public synchronized void removeChatWindowManager(
            IChatWindowManager chatWindowManager) {
        Logger.v(TAG,
                "removeChatWindowManager() entry, the chatWindowManager is "
                        + chatWindowManager);
        if (null != chatWindowManager) {
            boolean result = false;
            int count = 0;
            do {
                result = mChatWindowManagerList.remove(chatWindowManager);
                if (result) {
                    count++;
                    for (ChatWindowDispatcher chatWindowDispatcher : mChatWindowDispatcherMap
                            .values()) {
                        chatWindowDispatcher
                                .onRemoveChatWindowManager(chatWindowManager);
                    }
                }
            } while (result);
            Logger.v(TAG, "removeChatWindowManager() done, count: " + count
                    + ", current size: " + mChatWindowManagerList.size());
        } else {
            Logger.e(TAG, "removeChatWindowManager() chatWindowManager is null");
        }
    }

    /**
     * The Class ChatWindowDispatcher.
     */
    private abstract static class ChatWindowDispatcher implements IChatWindow {
        protected Object mTag = null;
        protected final CopyOnWriteArrayList<IChatItemDispatcher> mChatItemList =
                new CopyOnWriteArrayList<IChatItemDispatcher>();
        protected ConcurrentHashMap<IChatWindowManager, IChatWindow> mChatWindowMap =
                new ConcurrentHashMap<IChatWindowManager, IChatWindow>();

        /**
         * Instantiates a new chat window dispatcher.
         *
         * @param tag the tag
         */
        public ChatWindowDispatcher(Object tag) {
            mTag = tag;
        }

        /**
         * Update all msg as read.
         */
        @Override
        public void updateAllMsgAsRead() {
            for (IChatItemDispatcher chatItem : mChatItemList) {
                if (chatItem instanceof IReceivedChatMessage) {
                    ((ReceivedChatMessage) chatItem).updateStatus(true);
                }
            }
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.updateAllMsgAsRead();
            }

        }

        /**
         * Update all msg as read for contact.
         *
         * @param participant the participant
         */
        @Override
        public void updateAllMsgAsReadForContact(Participant participant) {

            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.updateAllMsgAsReadForContact(participant);
            }

        }

        /**
         * On destroy.
         */
        abstract void onDestroy();

        /**
         * Gets the sent chat message.
         *
         * @param messageId the message id
         * @return the sent chat message
         */
        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            IChatWindowMessage message = getChatMessage(messageId);
            Logger.i(TAG, "getSentChatMessage() messageId: " + messageId
                    + " , message: " + message);
            if (message instanceof ISentChatMessage
                    || message instanceof IFileTransfer) {
                return message;
            }
            return null;
        }

        /**
         * Gets the chat message.
         *
         * @param messageId the message id
         * @return the chat message
         */
        private IChatWindowMessage getChatMessage(String messageId) {
            Logger.i(TAG, "getChatMessage() id is " + messageId + "count="+ mChatItemList.size() + "list="+ mChatItemList);
            if (null != messageId) {
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    if (chatItem instanceof BaseChatMessage
                            && messageId.equals(((BaseChatMessage) chatItem)
                                    .getId())) {
                        Logger.i(TAG, "BaseChatMessage() id is " + messageId);
                        return (IChatWindowMessage) chatItem;
                    } else if (chatItem instanceof FileTransferDispatcher) {
                        Logger.i(TAG, "FileTransferDispatcher() id is "
                                + ((FileTransferDispatcher) chatItem).
                                mFileStruct.mFileTransferTag);
                        if (messageId
                                .equals(((FileTransferDispatcher) chatItem).
                                        mFileStruct.mFileTransferTag)) {
                            Logger.i(TAG, "FileTransferDispatcher() id is "
                                    + messageId);
                            return (IChatWindowMessage) chatItem;
                            /*Collection<IFileTransfer> fileTransfers =
                                    ((FileTransferDispatcher) chatItem).mChatWindowMap.values();
                            for (IFileTransfer fileTransfer : fileTransfers) {
                                if (fileTransfer instanceof SentFileTransfer) {
                                    return (IChatWindowMessage) fileTransfer;
                                }
                            }*/
                        }
                    }
                }
            } else {
                Logger.e(TAG,
                        "getChatMessage() cannot find the target message with id, " +
                        "messageId is null!");
            }
            return null;
        }

        /**
         * Adds the received message.
         *
         * @param message the message
         * @param isRead the is read
         * @return the i received chat message
         */
        @Override
        public IReceivedChatMessage addReceivedMessage(ChatMessage message,
                boolean isRead) {
            BaseChatMessage receivedMessage = null;
            synchronized(this){
                Logger.i(TAG, "addRecievedMessage() message is " + message);
                try {
                    if (null != getChatMessage(message.getId())) {
                        Logger.v(TAG, "addReceivedMessage() already added this message");
                        return null;
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                Collection<IChatWindow> chatWindows = mChatWindowMap.values();
                  receivedMessage = new ReceivedChatMessage(message,
                        isRead);
                for (IChatWindow chatWindow : chatWindows) {
                    receivedMessage.onAddChatWindow(chatWindow);
                }
                mChatItemList.add(receivedMessage);
            }
            return (IReceivedChatMessage) receivedMessage;

        }

        /**
         * Adds the sent message.
         *
         * @param message the message
         * @param messageTag the message tag
         * @return the i sent chat message
         */
        @Override
        public ISentChatMessage addSentMessage(ChatMessage message,
                int messageTag) {
            Logger.i(TAG, "addSentMessage() message is " + message
                    + " messageTag:" + messageTag);
            try {
                if (null != getChatMessage(message.getId()) && !message.getId().equals("-1")) {
                    Logger.v(TAG, "addSentMessage() already added this message");
                    return null;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            BaseChatMessage sentMessage = new SentChatMessage(message,
                    messageTag);
            for (IChatWindow chatWindow : chatWindows) {
                sentMessage.onAddChatWindow(chatWindow);
            }
            try {
                if(message.getId() !=null) {
                mChatItemList.add(sentMessage);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
            return (ISentChatMessage) sentMessage;
        }

        /**
         * On add chat window manager.
         *
         * @param chatWindowManager the chat window manager
         */
        public abstract void onAddChatWindowManager(
                IChatWindowManager chatWindowManager);

        /**
         * On remove chat window manager.
         *
         * @param chatWindowManager the chat window manager
         */
        public void onRemoveChatWindowManager(
                IChatWindowManager chatWindowManager) {
            Logger.v(TAG,
                    "onRemoveChatWindowManager() entry, the chatWindowManager is "
                            + chatWindowManager);
            IChatWindow chatWindow = mChatWindowMap.get(chatWindowManager);
            if (null != chatWindow) {
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    chatItem.onRemoveChatWindow(chatWindow);
                }
                mChatWindowMap.remove(chatWindowManager);
            } else {
                Logger.e(
                        TAG,
                        "onRemoveChatWindowManager() mChatWindowMap " +
                        "doesn't contain this chatWindowManager");
            }
        }

        /**
         * Removes the all messages.
         */
        @Override
        public void removeAllMessages() {
            Logger.v(TAG, "removeAllMessages() entry");
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.removeAllMessages();
            }
            mChatItemList.clear();
        }

        /**
         * Adds the load history header.
         *
         * @param showHeader the show header
         */
        @Override
        public void addLoadHistoryHeader(boolean showHeader) {
            Logger.v(TAG, "addLoadHistoryHeader() entry" + showHeader);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                chatWindow.addLoadHistoryHeader(showHeader);
            }
        }

    }

    /**
     * The Class One2OneChatWindowDispatcher.
     */
    private static class One2OneChatWindowDispatcher extends
            ChatWindowDispatcher implements IOne2OneChatWindow {
        public static final String TAG = "One2OneChatWindow";
        private Participant mParticipant = null;
        private boolean mIsComposing = false;
        private boolean mIsOffline = false;
        private int mFileTransferDisableReason = 0;
        private boolean mChatLoadedFromDatabase = false;

        /**
         * Instantiates a new one2 one chat window dispatcher.
         *
         * @param tag the tag
         * @param participant the participant
         */
        public One2OneChatWindowDispatcher(Object tag, Participant participant) {
            super(tag);
            mParticipant = participant;
        }

        /**
         * On add chat window manager.
         *
         * @param chatWindowManager the chat window manager
         */
        @Override
        public void onAddChatWindowManager(IChatWindowManager chatWindowManager) {
            Logger.v(TAG,
                    "onAddChatWindowManager() entry, the chatWindowManager is "
                            + chatWindowManager);
            IChatWindow chatWindow = chatWindowManager.addOne2OneChatWindow(
                    mTag, mParticipant);
            if (null != chatWindow) {
                mChatWindowMap.put(chatWindowManager, chatWindow);
                ((IOne2OneChatWindow) chatWindow).setIsComposing(mIsComposing);
                ((IOne2OneChatWindow) chatWindow)
                        .setRemoteOfflineReminder(mIsOffline);
                ((IOne2OneChatWindow) chatWindow)
                        .setFileTransferEnable(mFileTransferDisableReason);

                if (chatWindow instanceof OneOneChatWindow) {
                    if (((OneOneChatWindow) chatWindow)
                            .getmOneOneChatFragment() != null
                            && !mChatLoadedFromDatabase) {
                        ArrayList<Integer> messageIdArray = null;
                        mChatLoadedFromDatabase = true;
                        mChatItemList.clear();
                        if (RichMessagingDataProvider.getInstance() == null) {
                            RichMessagingDataProvider
                                    .createInstance(MediatekFactory
                                            .getApplicationContext());
                        }
                        messageIdArray = RichMessagingDataProvider
                                .getInstance().getRecentChatForContact(
                                        mParticipant.getContact(), 0);
                        Collections.reverse(messageIdArray);
                        if (!messageIdArray.isEmpty()) {
                            try {
                                Message controllerMessage = ControllerImpl
                                        .getInstance()
                                        .obtainMessage(
                                                ChatController.EVENT_RELOAD_MESSAGE,
                                                mTag.toString(), messageIdArray);
                                controllerMessage.sendToTarget();
                            } catch (Exception e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                        }
                    }
                }
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    chatItem.onAddChatWindow(chatWindow);
                }

            } else {
                Logger.e(TAG, "onAddChatWindowManager() chatWindow is null");
            }
        }

        /**
         * Sets the file transfer enable.
         *
         * @param reason the new file transfer enable
         */
        @Override
        public void setFileTransferEnable(int reason) {
            mFileTransferDisableReason = reason;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    ((IOne2OneChatWindow) chatWindow)
                            .setFileTransferEnable(reason);
                }
            }
        }

        /**
         * Sets the checks if is composing.
         *
         * @param isComposing the new checks if is composing
         */
        @Override
        public void setIsComposing(boolean isComposing) {
            mIsComposing = isComposing;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    ((IOne2OneChatWindow) chatWindow)
                            .setIsComposing(isComposing);
                }
            }
        }

        /**
         * Sets the remote offline reminder.
         *
         * @param isOffline the new remote offline reminder
         */
        @Override
        public void setRemoteOfflineReminder(boolean isOffline) {
            Logger.v(TAG, "setRemoteOfflineReminder() entry, isOffline is "
                    + isOffline);
            mIsOffline = isOffline;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    ((IOne2OneChatWindow) chatWindow)
                            .setRemoteOfflineReminder(isOffline);
                }
            }
            Logger.v(TAG, "setRemoteOfflineReminder() exit");
        }

        /**
         * On destroy.
         */
        @Override
        void onDestroy() {
            Set<IChatWindowManager> managers = mChatWindowMap.keySet();
            for (IChatWindowManager manager : managers) {
                manager.removeChatWindow(mChatWindowMap.get(manager));
            }
        }

        /**
         * Adds the received file transfer.
         *
         * @param file the file
         * @param isAutoAccept the is auto accept
         * @return the i file transfer
         */
        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file,
                boolean isAutoAccept) {
            Logger.i(TAG, "addReceivedFileTransfer() FileStruct is" + file);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            FileTransferDispatcher fileTransferDispatcher = new ReceivedFileTransferDispatcher(
                    file, isAutoAccept);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    fileTransferDispatcher
                            .onAddChatWindow((IOne2OneChatWindow) chatWindow);
                } else {
                    Logger.v(TAG,
                            "addReceivedFileTransfer() not a one-2-one chat window, just pass it");
                }
            }
            mChatItemList.add(fileTransferDispatcher);
            return fileTransferDispatcher;
        }

        /**
         * Removes the chat message.
         *
         * @param messageId the message id
         */
        @Override
        public void removeChatMessage(String messageId) {
            if (null != messageId) {
                Collection<IChatWindow> chatWindows = mChatWindowMap.values();
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    if (chatItem instanceof BaseChatMessage
                            && messageId.equals(((BaseChatMessage) chatItem)
                                    .getId())) {
                        Logger.e(TAG, "removeChatMessage() found the message !"
                                +  chatItem);
                        mChatItemList.remove(chatItem);
                    }
                    if (chatItem instanceof FileTransferDispatcher
                            && messageId
                                    .equals(((FileTransferDispatcher) chatItem).
                                            mFileStruct.mFileTransferTag.toString())) {
                        Logger.e(TAG, "removeChatMessage() found the message !"
                                + (FileTransferDispatcher) chatItem);
                        mChatItemList.remove(chatItem);
                    }
                }

                for (IChatWindow chatWindow : chatWindows) {
                    chatWindow.removeChatMessage(messageId);
                }

            } else {
                Logger.e(
                        TAG,
                        "removeChatMessage() cannot find the target message with id, " +
                        "messageId is null!");
            }
        }

        /**
         * Adds the sent file transfer.
         *
         * @param file the file
         * @return the i file transfer
         */
        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            Logger.i(TAG, "addSentFileTransfer() FileStruct is:" + file);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            FileTransferDispatcher fileTransferDispatcher = new SentFileTransferDispatcher(
                    file);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IOne2OneChatWindow) {
                    fileTransferDispatcher
                            .onAddChatWindow((IOne2OneChatWindow) chatWindow);
                } else {
                    Logger.v(TAG,
                            "addSentFileTransfer() not a one-2-one chat window, just pass it");
                }
            }
            Logger.i(TAG, "addSentFileTransfer() filedispatcher" + fileTransferDispatcher);
            mChatItemList.add(fileTransferDispatcher);
            Logger.i(TAG, "addSentFileTransfer() mchatitemlist:" + mChatItemList.size());
            return fileTransferDispatcher;
        }
    }

    /**
     * The Class GroupChatWindowDispatcher.
     */
    private static class GroupChatWindowDispatcher extends ChatWindowDispatcher
            implements IGroupChatWindow {
        public static final String TAG = "GroupChatWindowDispatcher";
        private CopyOnWriteArrayList<ParticipantInfo> mParticipantInfos = null;
        private int mStatus = 0;
        private boolean mGroupChatLoadedFromDatabase = false;
        private String mGroupChatSubject = "";
        private int mFileTransferDisableReason = 0;
        private boolean mIsClosedGroup = false;
        private String mChatID = "";

        /**
         * Gets the m chat id.
         *
         * @return the m chat id
         */
        public String getmChatID() {
            return mChatID;
        }

        /**
         * Sets the m chat id.
         *
         * @param chatID the new m chat id
         */
        @Override
        public void setmChatID(String chatID) {
            this.mChatID = chatID;
            Logger.v(TAG, "setmChatID(): " + chatID);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).setmChatID(chatID);
                }
            }
        }

        /**
         * Instantiates a new group chat window dispatcher.
         *
         * @param tag the tag
         * @param participantList the participant list
         */
        public GroupChatWindowDispatcher(Object tag,
                CopyOnWriteArrayList<ParticipantInfo> participantList) {
            super(tag);
            if (participantList == null) {
                Logger.e(TAG,
                        "when create group chat, participantSet must not be null");
                throw new RuntimeException(
                        "when create group chat,participantSet must not be null");
            }
            if (participantList.size() < ChatFragment.GROUP_MIN_MEMBER_NUM) {
                Logger.e(TAG,
                        "when create group chat, participantSet's number must more than 2");
                throw new RuntimeException(
                        "when create group chat, participantSet's number must more than 2");
            }
            mTag = tag;
            mParticipantInfos = new CopyOnWriteArrayList<ParticipantInfo>(
                    participantList);
        }

        /**
         * Sets the file transfer enable.
         *
         * @param reason the new file transfer enable
         */
        @Override
        public void setFileTransferEnable(int reason) {
            Logger.i(TAG, "setFileTransferEnable() reason" + reason);
            mFileTransferDisableReason = reason;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow)
                            .setFileTransferEnable(reason);
                }
            }
        }

        /**
         * Adds the received file transfer.
         *
         * @param file the file
         * @param isAutoAccept the is auto accept
         * @return the i file transfer
         */
        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file,
                boolean isAutoAccept, boolean isRead) {
            Logger.i(TAG, "addReceivedFileTransfer() FileStruct is" + file);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            FileTransferDispatcher fileTransferDispatcher = new ReceivedFileTransferDispatcher(
                    file, isAutoAccept, isRead);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    fileTransferDispatcher
                            .onAddChatWindow((IGroupChatWindow) chatWindow);
                } else {
                    Logger.v(TAG,
                            "addReceivedFileTransfer() not a group chat window, just pass it");
                }
            }
            mChatItemList.add(fileTransferDispatcher);
            return fileTransferDispatcher;
        }

        /**
         * Adds the sent file transfer.
         *
         * @param file the file
         * @return the i file transfer
         */
        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            Logger.i(TAG, "addSentFileTransfer() FileStruct is:" + file);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            FileTransferDispatcher fileTransferDispatcher = new SentFileTransferDispatcher(
                    file);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    fileTransferDispatcher
                            .onAddChatWindow((IGroupChatWindow) chatWindow);
                } else {
                    Logger.v(TAG,
                            "addSentFileTransfer() not a one-2-one chat window, just pass it");
                }
            }
            mChatItemList.add(fileTransferDispatcher);
            return fileTransferDispatcher;
        }

        /**
         * On add chat window manager.
         *
         * @param chatWindowManager the chat window manager
         */
        @Override
        public void onAddChatWindowManager(IChatWindowManager chatWindowManager) {
            Logger.v(TAG,
                    "onAddChatWindowManager() entry, the chatWindowManager is "
                            + chatWindowManager);
            IGroupChatWindow chatWindow = null;
            chatWindow = chatWindowManager.addGroupChatWindow(mTag,
                    mParticipantInfos);
            if (null != chatWindow) {
                mChatWindowMap.put(chatWindowManager, chatWindow);
                ((IGroupChatWindow) chatWindow).updateChatStatus(mStatus);
                ((IGroupChatWindow) chatWindow)
                        .setFileTransferEnable(mFileTransferDisableReason);
                setmChatID(mChatID);
                loadMessageIdList();
                if (chatWindow instanceof GroupChatWindow) {
                    if (((GroupChatWindow) chatWindow).getmGroupChatFragment() != null) {

                        ((GroupChatWindow) chatWindow)
                                .addgroupSubject(mGroupChatSubject);
                        ((GroupChatWindow) chatWindow)
                        .setClosedGroupType(mIsClosedGroup);
                    }
                }
                /*
                 * if (chatWindow instanceof GroupChatWindow) { if
                 * (((GroupChatWindow) chatWindow) .getmGroupChatFragment() !=
                 * null && !groupChatLoadedFromDatabase) { ArrayList<Integer>
                 * messageIdArray = new ArrayList<Integer>();
                 * groupChatLoadedFromDatabase = true; mChatItemList.clear();
                 * ArrayList<Participant> participantList = new
                 * ArrayList<Participant>(); for(ParticipantInfo participantinfo
                 * : mParticipantInfos) {
                 * participantList.add(participantinfo.getParticipant()); }
                 * if(RichMessagingDataProvider.getInstance()==null)
                 * RichMessagingDataProvider
                 * .createInstance(MediatekFactory.getApplicationContext());
                 * Cursor cur = RichMessagingDataProvider
                 * .getInstance().getAllMessageforGroupChat(mChatID); if
                 * (!messageIdArray.isEmpty()) {
                 * Collections.reverse(messageIdArray); Cursor cursor = null;
                 * try{ cursor = RichMessagingDataProvider
                 * .getInstance().getAllMessageforGroupChat(mChatID);
                 * Logger.v(TAG, "onAddChatWindowManager() cursor count = " +
                 * cursor.getCount() + ",chatid =" + mChatID); if (cursor !=
                 * null && cursor.moveToFirst()) { do {
                 *
                 * Integer messageId = cursor.getInt(cursor
                 * .getColumnIndex(ChatLog.Message.MESSAGE_ID));
                 *
                 * messageIdArray.add(messageId); } while (cursor.moveToNext());
                 * } else { Logger.v(TAG,
                 * " onAddChatWindowManager() empty cursor"); } }
                 * catch(Exception e){ e.printStackTrace(); } finally { if (null
                 * != cursor) { cursor.close(); } } if(messageIdArray != null) {
                 * if (!messageIdArray.isEmpty()) {
                 * Collections.reverse(messageIdArray); Logger.v(TAG,
                 * "onAddChatWindowManager() message id array "+
                 * messageIdArray.size()); try { Message controllerMessage =
                 * ControllerImpl .getInstance() .obtainMessage(
                 * ChatController.EVENT_RELOAD_MESSAGE, mTag.toString(),
                 * messageIdArray); controllerMessage.sendToTarget(); } catch
                 * (Exception e) {
                 *
                 * e.printStackTrace(); } } try { Message controllerMessage =
                 * ControllerImpl .getInstance() .obtainMessage(
                 * ChatController.EVENT_RELOAD_MESSAGE, mTag.toString(),
                 * messageIdArray); controllerMessage.sendToTarget(); } catch
                 * (Exception e) { // TODO Auto-generated catch block
                 * e.printStackTrace(); } } } }
                 */

                for (IChatItemDispatcher chatItem : mChatItemList) {
                    chatItem.onAddChatWindow(chatWindow);
                }

            } else {
                Logger.e(TAG, "onAddChatWindowManager() chatWindow is null");
            }
        }

        /**
         * Clear extra message.
         */
        @Override
        public void clearExtraMessage() {
            // need to keep default number of messages, so remove extra message
            // from memory
            Logger.i(TAG, "clearExtraMessage entry");
            if (mChatItemList.size() > GroupChatFragment.LOAD_DEFAULT) {
                // List<IChatItemDispatcher> mChatItemListTemp =
                // mChatItemList.subList(mChatItemList.size() -
                // GroupChatFragment.LOAD_DEFAULT -1, mChatItemList.size()-1);
                // mChatItemList = new
                // CopyOnWriteArrayList<IChatItemDispatcher>(mChatItemListTemp);
                int diff = mChatItemList.size()
                        - GroupChatFragment.LOAD_DEFAULT;
                for (int i = 0; i < diff; i++) {
                    mChatItemList.remove(0);
                }
                Logger.v(TAG, "GroupChatDispatcher size of mChatItemList "
                        + mChatItemList.size());
            }

        }

        /**
         * Update participants.
         *
         * @param participantList the participant list
         */
        @Override
        public void updateParticipants(List<ParticipantInfo> participantList) {
            Set<IChatWindowManager> keys = mChatWindowMap.keySet();
            Logger.v(TAG, "updateParticipants entry :" + participantList);
            Logger.v(TAG, "updateParticipants list before :" + mParticipantInfos);
            /*mParticipantInfos = new CopyOnWriteArrayList<ParticipantInfo>(
                    participantList);*/
            boolean flag = true;
            ParticipantInfo tempInfo = null;
            for(ParticipantInfo info : participantList){
                String contact = info.getContact();
                flag = true;
                for(ParticipantInfo info1 : mParticipantInfos){
                    Logger.v(TAG, "updateParticipants current list contact:" + info1.getContact()
                            +"new contact="+ contact + "new state=" + info.getState());
                    if(contact.equals(info1.getContact())){
                        flag = false;
                        info1.setState(info.getState());
                        tempInfo = info1;
                        break;
                    }
                }
                if(flag == true){
                    Logger.v(TAG, "updateParticipants flag true, user joined +" + info.getContact());
                    mParticipantInfos.add(info);
                } else {
                    if(tempInfo != null && User.STATE_DEPARTED.equals(tempInfo.getState())){
                        Logger.v(TAG, "updateParticipants flag false, user departed +" + tempInfo.getContact());
                        mParticipantInfos.remove(tempInfo);
                    }
                }
            }
            Logger.v(TAG, "updateParticipants list after :" + mParticipantInfos);
            for (IChatWindowManager chatWindowManager : keys) {
                IChatWindow chatWindow = mChatWindowMap.get(chatWindowManager);
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow)
                            .updateParticipants(mParticipantInfos);
                } else {
                    Logger.e(TAG, "updateParticipants() chatWindow is "
                            + chatWindow);
                }
            }
        }

        /**
         * Sets the is composing.
         *
         * @param isComposing the is composing
         * @param participant the participant
         */
        @Override
        public void setIsComposing(boolean isComposing, Participant participant) {
            Logger.v(TAG, "setIsComposing isComposing" + isComposing
                    + "participant is " + participant);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).setIsComposing(isComposing,
                            participant);
                } else {
                    Logger.e(TAG, "the chatwindow is not a group chat window");
                }
            }
        }

        /**
         * On destroy.
         */
        @Override
        void onDestroy() {
            Set<IChatWindowManager> managers = mChatWindowMap.keySet();
            for (IChatWindowManager manager : managers) {
                manager.removeChatWindow(mChatWindowMap.get(manager));
            }
        }

        /**
         * Update chat status.
         *
         * @param status the status
         */
        @Override
        public void updateChatStatus(int status) {
            Logger.v(TAG, "updateChatStatus() status: " + status);
            mStatus = status;
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).updateChatStatus(status);
                }
            }
        }

        /**
         * Adds the chat event information.
         *
         * @param chatEventStruct the chat event struct
         * @return the i chat event information
         */
        @Override
        public IChatEventInformation addChatEventInformation(
                ChatEventStruct chatEventStruct) {
            Logger.i(TAG, "addChatEventInformation() chatEventStruct is"
                    + chatEventStruct);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            ChatEventInformationDispatcher chatEventDispatcher = new ChatEventInformationDispatcher(
                    chatEventStruct);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    chatEventDispatcher
                            .onAddChatWindow((IGroupChatWindow) chatWindow);
                } else {
                    Logger.v(TAG,
                            "addChatEventInformation() not a group chat window, just pass it");
                }
            }
            mChatItemList.add(chatEventDispatcher);
            return (IChatEventInformation) chatEventDispatcher;
        }

        /**
         * Removes the chat message.
         *
         * @param messageId the message id
         */
        @Override
        public void removeChatMessage(String messageId) {
            if (null != messageId) {
                Collection<IChatWindow> chatWindows = mChatWindowMap.values();
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    if (chatItem instanceof BaseChatMessage
                            && messageId.equals(((BaseChatMessage) chatItem)
                                    .getId())) {
                        Logger.e(TAG,
                                "removeChatMessage() found the message !="
                                        + chatItem);
                        mChatItemList.remove(chatItem);
                    } else if (chatItem instanceof FileTransferDispatcher
                            && messageId
                                    .equals(((FileTransferDispatcher) chatItem).mFileStruct.
                                            mFileTransferTag.toString())) {
                        Logger.e(TAG, "removeChatMessage() found the message !"
                                + (FileTransferDispatcher) chatItem);
                        mChatItemList.remove(chatItem);
                    }
                }
                for (IChatWindow chatWindow : chatWindows) {
                    chatWindow.removeChatMessage(messageId);
                }
            } else {
                Logger.e(
                        TAG,
                        "removeChatMessage() cannot find the target message with id, " +
                        "messageId is null!");
            }
        }

        /**
         * Addgroup subject.
         *
         * @param subject the subject
         */
        @Override
        public void addgroupSubject(String subject) {
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if ((chatWindow instanceof IGroupChatWindow)) {
                    mGroupChatSubject = subject;
                    ((IGroupChatWindow) chatWindow).addgroupSubject(subject);
                }

            }
        }


        /**
         * Type of group chat open or closed.
         *
         * @param type true- group is closed, false - group is open(default)
         */
        @Override
		public void setClosedGroupType(boolean type) {
            Logger.v(TAG, "setClosedGroupType() entry");
        	Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if ((chatWindow instanceof IGroupChatWindow)) {
                	mIsClosedGroup = type;
                    ((IGroupChatWindow) chatWindow).setClosedGroupType(type);
                }

            }

		}

        /**
         * Load message id list.
         */
        @Override
        public void loadMessageIdList() {
            Logger.v(TAG, "loadMessageIdList()");
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IGroupChatWindow) {
                    ((IGroupChatWindow) chatWindow).loadMessageIdList();
                }
            }

        }

        @Override
        public void onSessionStarted() {
            Logger.v(TAG, "onSessionStarted()");
            //there may be some invalid messages added when session was not established
            //remove them when session is successfully started
            for (IChatItemDispatcher chatItem : mChatItemList) {
                if (chatItem instanceof BaseChatMessage
                        && "-1".equals(((BaseChatMessage) chatItem)
                                .getId())) {
                    mChatItemList.remove(chatItem);
                }
            }
        }


    }

    /**
     * The Class MultiChatWindowDispatcher.
     */
    private static class MultiChatWindowDispatcher extends ChatWindowDispatcher
            implements IMultiChatWindow {
        public static final String TAG = "MultiChatWindowDispatcher";
        private CopyOnWriteArrayList<ParticipantInfo> mParticipantInfos = null;
        private int mStatus = 0;
        private boolean mGroupChatLoadedFromDatabase = false;



        /**
         * Instantiates a new group chat window dispatcher.
         *
         * @param tag the tag
         * @param participantList the participant list
         */
        public MultiChatWindowDispatcher(Object tag,
                CopyOnWriteArrayList<ParticipantInfo> participantList) {
            super(tag);
            if (participantList == null) {
                Logger.e(TAG,
                        "when create multi chat, participantSet must not be null");
                throw new RuntimeException(
                        "when create multi chat,participantSet must not be null");
            }
            if (participantList.size() < ChatFragment.GROUP_MIN_MEMBER_NUM) {
                Logger.e(TAG,
                        "when create multi chat, participantSet's number must more than 2");
                throw new RuntimeException(
                        "when create multi chat, participantSet's number must more than 2");
            }
            mTag = tag;
            mParticipantInfos = new CopyOnWriteArrayList<ParticipantInfo>(
                    participantList);
        }


        /**
         * Adds the sent file transfer.
         *
         * @param file the file
         * @return the i file transfer
         */
        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            Logger.i(TAG, "addSentFileTransfer() FileStruct is:" + file);
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            FileTransferDispatcher fileTransferDispatcher = new SentFileTransferDispatcher(
                    file);
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IMultiChatWindow) {
                    fileTransferDispatcher
                            .onAddChatWindow((IMultiChatWindow) chatWindow);
                } else {
                    Logger.v(TAG,
                            "addSentFileTransfer() not a multi chat window, just pass it");
                }
            }
            mChatItemList.add(fileTransferDispatcher);
            return fileTransferDispatcher;
        }

        /**
         * On add chat window manager.
         *
         * @param chatWindowManager the chat window manager
         */
        @Override
        public void onAddChatWindowManager(IChatWindowManager chatWindowManager) {
            Logger.v(TAG,
                    "onAddChatWindowManager() entry, the chatWindowManager is "
                            + chatWindowManager);
            IMultiChatWindow chatWindow = null;
            chatWindow = chatWindowManager.addMultiChatWindow(mTag,
                    mParticipantInfos);
            if (null != chatWindow) {
                mChatWindowMap.put(chatWindowManager, chatWindow);
                loadMessageIdList();
                /*if (chatWindow instanceof MultiChatWindow) {
                    if (((GroupChatWindow) chatWindow).getmGroupChatFragment() != null) {

                        ((GroupChatWindow) chatWindow)
                        .setClosedGroupType(mIsClosedGroup);
                    }
                }*/

                for (IChatItemDispatcher chatItem : mChatItemList) {
                    chatItem.onAddChatWindow(chatWindow);
                }

            } else {
                Logger.e(TAG, "onAddChatWindowManager() chatWindow is null");
            }
        }

        /**
         * Clear extra message.
         */
        @Override
        public void clearExtraMessage() {
            // need to keep default number of messages, so remove extra message
            // from memory
            Logger.i(TAG, "clearExtraMessage entry");
            if (mChatItemList.size() > GroupChatFragment.LOAD_DEFAULT) {
                // List<IChatItemDispatcher> mChatItemListTemp =
                // mChatItemList.subList(mChatItemList.size() -
                // GroupChatFragment.LOAD_DEFAULT -1, mChatItemList.size()-1);
                // mChatItemList = new
                // CopyOnWriteArrayList<IChatItemDispatcher>(mChatItemListTemp);
                int diff = mChatItemList.size()
                        - GroupChatFragment.LOAD_DEFAULT;
                for (int i = 0; i < diff; i++) {
                    mChatItemList.remove(0);
                }
                Logger.v(TAG, "MultiChatDispatcher size of mChatItemList "
                        + mChatItemList.size());
            }

        }

        /**
         * Update participants.
         *
         * @param participantList the participant list
         */
        @Override
        public void updateParticipants(List<ParticipantInfo> participantList) {
            Set<IChatWindowManager> keys = mChatWindowMap.keySet();
            Logger.v(TAG, "updateParticipants entry :" + participantList);
            Logger.v(TAG, "updateParticipants list :" + mParticipantInfos);
            for (IChatWindowManager chatWindowManager : keys) {
                IChatWindow chatWindow = mChatWindowMap.get(chatWindowManager);
                if (chatWindow instanceof IMultiChatWindow) {
                    ((IMultiChatWindow) chatWindow)
                            .updateParticipants(mParticipantInfos);
                } else {
                    Logger.e(TAG, "updateParticipants() chatWindow is "
                            + chatWindow);
                }
            }
        }


        /**
         * On destroy.
         */
        @Override
        void onDestroy() {
            Set<IChatWindowManager> managers = mChatWindowMap.keySet();
            for (IChatWindowManager manager : managers) {
                manager.removeChatWindow(mChatWindowMap.get(manager));
            }
        }


        /**
         * Removes the chat message.
         *
         * @param messageId the message id
         */
        @Override
        public void removeChatMessage(String messageId) {
            if (null != messageId) {
                Collection<IChatWindow> chatWindows = mChatWindowMap.values();
                for (IChatItemDispatcher chatItem : mChatItemList) {
                    if (chatItem instanceof BaseChatMessage
                            && messageId.equals(((BaseChatMessage) chatItem)
                                    .getId())) {
                        Logger.e(TAG,
                                "removeChatMessage() found the message !="
                                        + chatItem);
                        mChatItemList.remove(chatItem);
                    } else if (chatItem instanceof FileTransferDispatcher
                            && messageId
                                    .equals(((FileTransferDispatcher) chatItem).mFileStruct.
                                            mFileTransferTag.toString())) {
                        Logger.e(TAG, "removeChatMessage() found the message !"
                                + (FileTransferDispatcher) chatItem);
                        mChatItemList.remove(chatItem);
                    }
                }
                for (IChatWindow chatWindow : chatWindows) {
                    chatWindow.removeChatMessage(messageId);
                }
            } else {
                Logger.e(
                        TAG,
                        "removeChatMessage() cannot find the target message with id, " +
                        "messageId is null!");
            }
        }



        /**
         * Load message id list.
         */
        @Override
        public void loadMessageIdList() {
            Logger.v(TAG, "loadMessageIdList()");
            Collection<IChatWindow> chatWindows = mChatWindowMap.values();
            for (IChatWindow chatWindow : chatWindows) {
                if (chatWindow instanceof IMultiChatWindow) {
                    ((IMultiChatWindow) chatWindow).loadMessageIdList();
                }
            }

        }

    }

    /**
     * Switch chat window by tag.
     *
     * @param uuidTag the uuid tag
     */
    @Override
    public void switchChatWindowByTag(ParcelUuid uuidTag) {
        for (IChatWindowManager chatWindowManager : mChatWindowManagerList) {
            chatWindowManager.switchChatWindowByTag(uuidTag);
        }
    }

    /**
     * This is a common interface for the item listed in a chat window.
     */
    private interface IChatItemDispatcher {

        /**
         * Called by the ViewImpl while there is a new ChatWindowManager
         * registered.
         *
         * @param chatWindow the chat window
         */
        void onAddChatWindow(IChatWindow chatWindow);

        /**
         * On remove chat window.
         *
         * @param chatWindow the chat window
         */
        void onRemoveChatWindow(IChatWindow chatWindow);
    }

    /**
     * This class defines a base chat message type in a chat window, which is
     * used to transfer Model statuses to multiple Views.
     */
    public abstract static class BaseChatMessage implements IChatWindowMessage,
            IChatItemDispatcher {
        protected ChatMessage mMessage = null;

        protected ConcurrentHashMap<IChatWindow, IChatWindowMessage> mChatWindowMap =
                new ConcurrentHashMap<IChatWindow, IChatWindowMessage>();

        /**
         * Instantiates a new base chat message.
         *
         * @param message the message
         */
        protected BaseChatMessage(ChatMessage message) {
            mMessage = message;
        }

        /**
         * Gets the id.
         *
         * @return the id
         */
        @Override
        public final String getId() {
            if (null != mMessage) {
                return mMessage.getId();
            } else {
                Logger.e(TAG, "ChatMessage->getId() mMessage is null!");
                return null;
            }
        }
    }

    /**
     * The Class FileTransferDispatcher.
     */
    private abstract static class FileTransferDispatcher implements
            IFileTransfer, IChatItemDispatcher {
        public static final String TAG = "FileTransferDispatcher";

        private FileStruct mFileStruct = null;

        private Status mStatus = null;

        private long mProgress = -1;

        protected ConcurrentHashMap<IChatWindow, IFileTransfer> mChatWindowMap =
                new ConcurrentHashMap<IChatWindow, IFileTransfer>();

        /**
         * Instantiates a new file transfer dispatcher.
         *
         * @param fileStruct the file struct
         */
        public FileTransferDispatcher(FileStruct fileStruct) {
            mFileStruct = fileStruct;
        }

        /**
         * On add chat window.
         *
         * @param chatWindow the chat window
         */
        public void onAddChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onAddChatWindow() entry");
            if (null != chatWindow) {
                IFileTransfer fileTransfer = getFileTransfer(chatWindow,
                        mFileStruct);
                if (null != fileTransfer) {
                    if (null != mStatus) {
                        fileTransfer.setStatus(mStatus);
                    }
                    if (-1 != mProgress) {
                        fileTransfer.setProgress(mProgress);
                    }
                    mChatWindowMap.put(chatWindow, fileTransfer);
                } else {
                    Logger.e(TAG, "onAddChatWindow() fileTransfer is null");
                }
            } else {
                Logger.e(TAG, "onAddChatWindow() chatWindow is null");
            }
        }

        /**
         * On remove chat window.
         *
         * @param chatWindow the chat window
         */
        public void onRemoveChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onRemoveChatWindow() entry");
            mChatWindowMap.remove(chatWindow);
        }

        /**
         * Sets the progress.
         *
         * @param progress the new progress
         */
        @Override
        public void setProgress(long progress) {
            Logger.i(TAG, "setProgress() entry, progress is " + progress);
            mProgress = progress;
            Collection<IFileTransfer> fileTransfers = mChatWindowMap.values();
            for (IFileTransfer fileTransfer : fileTransfers) {
                fileTransfer.setProgress(mProgress);
            }
        }

        /**
         * Sets the status.
         *
         * @param status the new status
         */
        @Override
        public void setStatus(Status status) {
            Logger.i(TAG, "setStatus() entry, status is " + status + ", oldstatus = " + mStatus);
            if (mStatus == status) {
                Logger.v(TAG,
                        "setStatus() the new status equals the old one, so no need to update");
            } else if (mStatus == Status.DISPLAYED  || (mStatus == Status.FINISHED && (status!= Status.DELIVERED) && (status != Status.DISPLAYED))) {
                Logger.i(TAG, "status is finished or displayed, so no need to update ");
            } else {
                mStatus = status;
                Collection<IFileTransfer> fileTransfers = mChatWindowMap
                        .values();
                for (IFileTransfer fileTransfer : fileTransfers) {
                    fileTransfer.setStatus(mStatus);
                }
            }
        }

        /**
         * Sets the file path.
         *
         * @param filePath the new file path
         */
        @Override
        public void setFilePath(String filePath) {
            Logger.i(TAG, "setFilePath() entry, filePath is " + filePath);
            if (mFileStruct != null) {
                mFileStruct.mFilePath = filePath;
            } else {
                Logger.e(TAG, "setFilePath, mFileStruct is null!");
            }

            if (mChatWindowMap != null) {
                Collection<IFileTransfer> fileTransfers = mChatWindowMap
                        .values();
                for (IFileTransfer fileTransfer : fileTransfers) {
                    fileTransfer.setFilePath(filePath);
                }
            } else {
                Logger.e(TAG, "setFilePath, mChatWindowMap is null!");
            }

        }

        /**
         * Update tag.
         *
         * @param transferTag the transfer tag
         * @param size the size
         */
        @Override
        public void updateTag(String transferTag, long size) {
            Logger.i(TAG, "updateTag() entry,old transferTag is " + mFileStruct.mFileTransferTag);
            Logger.i(TAG, "updateTag() entry, transferTag is " + transferTag);
            if (mFileStruct != null) {
                mFileStruct.mFileTransferTag = transferTag;
            } else {
                Logger.e(TAG, "updateTag, mFileStruct is null!");
            }

            if (mChatWindowMap != null) {
                Collection<IFileTransfer> fileTransfers = mChatWindowMap
                        .values();
                for (IFileTransfer fileTransfer : fileTransfers) {
                    fileTransfer.updateTag(transferTag, size);
                }
            } else {
                Logger.e(TAG, "updateTag, mChatWindowMap is null!");
            }
        }

        /**
         * Gets the file transfer.
         *
         * @param chatWindow the chat window
         * @param fileStruct the file struct
         * @return the file transfer
         */
        abstract IFileTransfer getFileTransfer(IChatWindow chatWindow,
                FileStruct fileStruct);
    }

    /**
     * The Class SentFileTransferDispatcher.
     */
    private static class SentFileTransferDispatcher extends
            FileTransferDispatcher {

        /**
         * Instantiates a new sent file transfer dispatcher.
         *
         * @param fileStruct the file struct
         */
        public SentFileTransferDispatcher(FileStruct fileStruct) {
            super(fileStruct);
        }

        /**
         * Gets the file transfer.
         *
         * @param chatWindow the chat window
         * @param fileStruct the file struct
         * @return the file transfer
         */
        @Override
        IFileTransfer getFileTransfer(IChatWindow chatWindow,
                FileStruct fileStruct) {
            if (chatWindow instanceof OneOneChatWindow) {
                return ((IOne2OneChatWindow) chatWindow)
                        .addSentFileTransfer(fileStruct);
            } else if (chatWindow instanceof GroupChatWindow) {
                return ((IGroupChatWindow) chatWindow)
                        .addSentFileTransfer(fileStruct);
            } else if (chatWindow instanceof MultiChatWindow) {
                return ((IMultiChatWindow) chatWindow)
                        .addSentFileTransfer(fileStruct);
            } else if (chatWindow instanceof One2OneChatWindowAdapter) {
                return ((IOne2OneChatWindow) chatWindow)
                        .addSentFileTransfer(fileStruct);
            } else if (chatWindow instanceof GroupChatWindowAdapter) {
                return ((IGroupChatWindow) chatWindow)
                        .addSentFileTransfer(fileStruct);
            } else {
                return null;
            }
        }

        public String getId() {
            return "";
        }

    }

    /**
     * The Class ReceivedFileTransferDispatcher.
     */
    private static class ReceivedFileTransferDispatcher extends
            FileTransferDispatcher {

        private boolean mIsAutoAccept = false;
        private boolean mIsRead = true;

        /**
         * Instantiates a new received file transfer dispatcher.
         *
         * @param fileStruct the file struct
         * @param isAutoAccept the is auto accept
         */
        public ReceivedFileTransferDispatcher(FileStruct fileStruct,
                boolean isAutoAccept) {
            super(fileStruct);
            mIsAutoAccept = isAutoAccept;
        }

        public ReceivedFileTransferDispatcher(FileStruct fileStruct, boolean isAutoAccept, boolean isRead) {
            super(fileStruct);
            mIsAutoAccept = isAutoAccept;
            mIsRead = isRead;
        }

        /**
         * Gets the file transfer.
         *
         * @param chatWindow the chat window
         * @param fileStruct the file struct
         * @return the file transfer
         */
        @Override
        IFileTransfer getFileTransfer(IChatWindow chatWindow,
                FileStruct fileStruct) {
            if (chatWindow instanceof OneOneChatWindow) {
                return ((IOne2OneChatWindow) chatWindow)
                        .addReceivedFileTransfer(fileStruct, mIsAutoAccept);
            } else if (chatWindow instanceof GroupChatWindow) {
                return ((IGroupChatWindow) chatWindow).addReceivedFileTransfer(
                        fileStruct, mIsAutoAccept, mIsRead);
            } else if (chatWindow instanceof One2OneChatWindowAdapter) {
                return ((IOne2OneChatWindow) chatWindow)
                        .addReceivedFileTransfer(fileStruct, mIsAutoAccept);
            } else if (chatWindow instanceof GroupChatWindowAdapter) {
                return ((IGroupChatWindow) chatWindow).addReceivedFileTransfer(
                        fileStruct, mIsAutoAccept, mIsRead);
            } else {
                return null;
            }
        }

        public String getId() {
            return "";
        }
    }

    /**
     * The Class ChatEventInformationDispatcher.
     */
    private static class ChatEventInformationDispatcher implements
            IChatItemDispatcher, IChatEventInformation {
        public static final String TAG = "ChatEventInformationDispatcher";
        protected ConcurrentHashMap<IChatWindow, IChatEventInformation> mChatWindowMap =
                new ConcurrentHashMap<IChatWindow, IChatEventInformation>();
        private ChatEventStruct mChatEventStruct = null;

        /**
         * Instantiates a new chat event information dispatcher.
         *
         * @param chatEventStruct the chat event struct
         */
        public ChatEventInformationDispatcher(ChatEventStruct chatEventStruct) {
            mChatEventStruct = chatEventStruct;
        }

        /**
         * On add chat window.
         *
         * @param chatWindow the chat window
         */
        @Override
        public void onAddChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onAddChatWindow() entry");
            if (null != chatWindow) {
                IChatEventInformation chatEvent = ((IGroupChatWindow) chatWindow)
                        .addChatEventInformation(mChatEventStruct);
                if (null != chatEvent) {
                    mChatWindowMap.put(chatWindow, chatEvent);
                } else {
                    Logger.e(TAG, "onAddChatWindow() chatEvent is null");
                }
            } else {
                Logger.e(TAG, "onAddChatWindow() chatWindow is null");
            }

        }

        /**
         * On remove chat window.
         *
         * @param chatWindow the chat window
         */
        @Override
        public void onRemoveChatWindow(IChatWindow chatWindow) {
            Logger.v(TAG, "onRemoveChatWindow() entry");
            mChatWindowMap.remove(chatWindow);
        }
    }
}
