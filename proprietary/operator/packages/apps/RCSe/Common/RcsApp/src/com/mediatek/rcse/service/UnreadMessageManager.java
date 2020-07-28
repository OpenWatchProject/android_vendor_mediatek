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

package com.mediatek.rcse.service;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.ParcelUuid;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.api.Participant;
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
import com.mediatek.rcse.mvc.ParticipantInfo;
import com.mediatek.rcse.mvc.ViewImpl;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

//import com.gsma.services.rcs.chat.ChatMessage;

/**
 * A unread message manager which help notify launcher to show the unread
 * message number.
 */
public class UnreadMessageManager implements IChatWindowManager {
    private static final String TAG = "UnreadMessageManager";
    private static Object sLock = new Object();
    private final ConcurrentHashMap<String, Integer> mUnreadMessageCountMap =
            new ConcurrentHashMap<String, Integer>();
    /**
     * Broadcast Action: action used for launcher unread number feature. The
     * broadcat is sent when the unread number of application changes.
     */
    private static final String ACTION_UNREAD_CHANGED =
            "com.mediatek.action.UNREAD_CHANGED";

    private static final String MTK_ACTION_UNREAD_CHANGED_CONTACT =
            "com.mediatek.action.UNREAD_CHANGED_CONTACT";
    /**
     * Extra used to indicate the unread number of which component changes.
     */
    private static final String MTK_EXTRA_UNREAD_COMPONENT =
            "com.mediatek.intent.extra.UNREAD_COMPONENT";
    /**
     * The number of unread messages.
     */
    private static final String MTK_EXTRA_UNREAD_NUMBER =
            "com.mediatek.intent.extra.UNREAD_NUMBER";
    /**
     * The package name of RCS-e.
     */
    private static final String RCSE_PACKAGE_NAME =
            "com.orangelabs.rcs";
    /**
     * The Activity name of RCS-e chat window.
     */
    private static final String RCSE_ACTIVITY_NAME =
            "com.mediatek.rcse.activities.ChatMainActivity";

    public static final int MIN_STEP_UNREAD_MESSAGE_NUM = 1;

    private static UnreadMessageManager sInstance = null;
    private final AtomicInteger mUnreadMessageNum = new AtomicInteger(0);
    private final Object mLock = new Object();

    /**
     * Instantiates a new unread message manager.
     */
    private UnreadMessageManager() {
        Logger.d(TAG, "UnreadMessageManager() constructor");
        ViewImpl.getInstance().addChatWindowManager(this, true);
        resetUnreadMessageNum();
    }

    /**
     * Get a instance of UnreadMessageManager.
     *
     * @return A instance of UnreadMessageManager
     */
    public static UnreadMessageManager getInstance() {
        synchronized (sLock) {
            if (sInstance == null) {
                sInstance = new UnreadMessageManager();
            }
        }
        return sInstance;
    }

    /**
     * Let launcher show a unread message number on APP icon.
     *
     * @param unReadMessageNum
     *            The number of unread message.
     */
    private void sendUnreadBroadcast(final int unReadMessageNum) {
        Logger.d(TAG, "sendUnreadBroadcast(), unReadMessageNum is "
                + unReadMessageNum);
        ApiManager apiManager = ApiManager.getInstance();
        Context context = null;
        if (apiManager != null) {
            context = apiManager.getContext();
        }
        if (context != null) {
            Intent intent = new Intent(ACTION_UNREAD_CHANGED);
            intent.putExtra(MTK_EXTRA_UNREAD_NUMBER, unReadMessageNum);
            intent.putExtra(MTK_EXTRA_UNREAD_COMPONENT, new ComponentName(
                    RCSE_PACKAGE_NAME, RCSE_ACTIVITY_NAME));
            context.sendBroadcast(intent);
        } else {
            Logger.w(TAG,
                    "sendUnreadBroadcast(), context is null. Did not send broadcast to launcher");
        }
        Logger.d(TAG, "sendUnreadBroadcast() exit");
    }

    /**
     * Change unread message number and update to launcher.
     *
     * @param number
     *            The number of unread message.
     * @param isInc
     *            If isInc is true then the total unread message will be added
     *            by number, otherwise will be subtracted by number
     */
    public void changeUnreadMessageNum(final int number, boolean isInc) {
        Logger.d(TAG, "changeUnreadMessageNum(), number is " + number
                + ", isInc = " + isInc + ",mUnreadMessageNum = "
                + mUnreadMessageNum);
        int unReadMessageNum = 0;
        synchronized (mLock) {
            if (isInc) {
                unReadMessageNum = mUnreadMessageNum.addAndGet(number);
            } else {
                unReadMessageNum = mUnreadMessageNum.addAndGet(-number);
                if (unReadMessageNum < 0) {
                    Logger.d(TAG, "Impossible... why coming here?");
                    unReadMessageNum = 0;
                    mUnreadMessageNum.set(0);
                }
            }
        }
        sendUnreadBroadcast(unReadMessageNum);
        Logger.d(TAG, "changeUnreadMessageNum() exit");
    }

    /**
     * Reset unread message number and update to launcher.
     */
    public void resetUnreadMessageNum() {
        Logger.d(TAG, "resetUnreadMessageNum()");
        sendUnreadBroadcast(0);
        mUnreadMessageNum.set(0);
        Logger.d(TAG, "resetUnreadMessageNum() exit");
    }

    /**
     * Get the number of unread message.
     *
     * @return The total number of unread message
     */
    public int getUnreadMessageNum() {
        Logger.d(TAG, "getUnreadMessageNum()");
        return mUnreadMessageNum.get();
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
        Logger.d(TAG, "addOne2OneChatWindow()");
        return new UnreadMessageChatWindow();
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
        Logger.d(TAG, "addGroupChatWindow()");
        return new UnreadMessageChatWindow();
    }

    /**
     * Removes the chat window.
     *
     * @param chatWindow the chat window
     * @return true, if successful
     */
    @Override
    public boolean removeChatWindow(IChatWindow chatWindow) {
        Logger.d(TAG, "removeChatWindow() chatWindow = " + chatWindow);
        // Should count down the number of unread message in this window
        if (chatWindow instanceof UnreadMessageChatWindow) {
            int unReadMessageNum = ((UnreadMessageChatWindow) chatWindow)
                    .getUnreadMessageNum();
            changeUnreadMessageNum(unReadMessageNum, false);
        } else {
            Logger.w(TAG, "Impossbile.... why coming here??");
        }
        return false;
    }

    /**
     * Switch chat window by tag.
     *
     * @param uuidTag the uuid tag
     */
    @Override
    public void switchChatWindowByTag(ParcelUuid uuidTag) {
    }

    /**
     * A virtual chat window. Its main usage is to count the number of unread
     * message
     */
    private class UnreadMessageChatWindow implements IOne2OneChatWindow,
            IGroupChatWindow {
        // This number is responsible this window
        private int mChatWindowUnreadMessageNum = 0;

        /**
         * Get the number of unread message of this window.
         *
         * @return The number of unread message of this window
         */
        public int getUnreadMessageNum() {
            return mChatWindowUnreadMessageNum;
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
            Logger.d(TAG, "addReceivedMessage(), isRead = " + isRead);
            if (!isRead) {
                changeUnreadMessageNum(MIN_STEP_UNREAD_MESSAGE_NUM, true);

                // update the map
                String contact = message.getRemoteContact().toString();
                if(contact == null){
                    Logger.d(TAG, "addReceivedMessage(), contact = " + contact);
                    return null;
                }
                int unreadCount = 0;
                if (mUnreadMessageCountMap != null) {
                    if (mUnreadMessageCountMap.containsKey(contact)) {
                        unreadCount = mUnreadMessageCountMap.get(contact);
                    }
                    synchronized (mUnreadMessageCountMap) {
                        mUnreadMessageCountMap.put(contact, unreadCount + 1);
                    }
                }

                // send intent to update unread count
                ApiManager apiManager = ApiManager.getInstance();
                Context context = null;
                if (apiManager != null) {
                    context = apiManager.getContext();
                }
                if (context != null) {
                    Intent intent = new Intent(
                            MTK_ACTION_UNREAD_CHANGED_CONTACT);
                    intent.putExtra("number", message.getRemoteContact().toString());
                    intent.putExtra("count", unreadCount + 1);
                    context.sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
                }
                // end sending intent
                ++mChatWindowUnreadMessageNum;
            }
            return null;
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
            return null;
        }

        /**
         * Removes the all messages.
         */
        @Override
        public void removeAllMessages() {
            Logger.d(TAG, "removeAllMessages. mUnreadMessageNum = "
                    + mUnreadMessageNum);
        }

        /**
         * Gets the sent chat message.
         *
         * @param messageId the message id
         * @return the sent chat message
         */
        @Override
        public IChatWindowMessage getSentChatMessage(String messageId) {
            return null;
        }

        /**
         * Adds the load history header.
         *
         * @param showLoader the show loader
         */
        @Override
        public void addLoadHistoryHeader(boolean showLoader) {

        }

        /**
         * Update all msg as read.
         */
        @Override
        public void updateAllMsgAsRead() {
            // This function will count down unread file transfer number
            Logger.d(TAG, "updateAllMsgAsRead. mUnreadMessageNum = "
                    + mUnreadMessageNum);
            changeUnreadMessageNum(mChatWindowUnreadMessageNum, false);
            mChatWindowUnreadMessageNum = 0;
        }

        /**
         * Update all msg as read for contact.
         *
         * @param participant the participant
         */
        @Override
        public void updateAllMsgAsReadForContact(Participant participant) {
            Logger.d(TAG, "updateAllMsgAsReadForContact. contact = "
                    + participant.getContact());

            int n = 8; // workaround for wrong value coming in participant

            String contact = participant.getContact();
            if (contact.length() > n) {
                String substring = contact.substring(0, n);
                if ("+349++++".equals(substring)) {
                    contact = contact.substring(8, contact.length());
                    contact = "+" + contact;
                }
            }
            // update the map
            if (mUnreadMessageCountMap != null) {
                if (mUnreadMessageCountMap.containsKey(contact)) {
                    mUnreadMessageCountMap.put(contact, 0);
                }
            }

            // send intent to update unread count
            ApiManager apiManager = ApiManager.getInstance();
            Context context = null;
            if (apiManager != null) {
                context = apiManager.getContext();
            }
            if (context != null) {
                Intent intent = new Intent(MTK_ACTION_UNREAD_CHANGED_CONTACT);
                intent.putExtra("number", contact);
                intent.putExtra("updateall", true);
                intent.putExtra("count", 0);
                context.sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
            }
            // end sending intent
        }

        /**
         * Sets the file transfer enable.
         *
         * @param reason the new file transfer enable
         */
        @Override
        public void setFileTransferEnable(int reason) {
        }

        /**
         * Sets the checks if is composing.
         *
         * @param isComposing the new checks if is composing
         */
        @Override
        public void setIsComposing(boolean isComposing) {
        }

        /**
         * Sets the remote offline reminder.
         *
         * @param isOffline the new remote offline reminder
         */
        @Override
        public void setRemoteOfflineReminder(boolean isOffline) {
        }

        /**
         * Adds the sent file transfer.
         *
         * @param file the file
         * @return the i file transfer
         */
        @Override
        public IFileTransfer addSentFileTransfer(FileStruct file) {
            return null;
        }

        @Override
        public IFileTransfer addReceivedFileTransfer(FileStruct file, boolean isAutoAccept, boolean isRead) {
        	return null;
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
            Logger.d(
                    TAG,
                    "Receive a file transfer, " +
                    "then increate the number of unread message of this window");

            // update the map
            String contact = file.mRemote;
            int unreadCount = 0;
            if (mUnreadMessageCountMap != null) {
                if (mUnreadMessageCountMap.containsKey(contact)) {
                    unreadCount = mUnreadMessageCountMap.get(contact);
                }
                synchronized (mUnreadMessageCountMap) {
                    mUnreadMessageCountMap.put(contact, unreadCount + 1);
                }
            }

            // send intent to update unread count
            ApiManager apiManager = ApiManager.getInstance();
            Context context = null;
            if (apiManager != null) {
                context = apiManager.getContext();
            }
            if (context != null) {
                Intent intent = new Intent(MTK_ACTION_UNREAD_CHANGED_CONTACT);
                intent.putExtra("number", contact);
                intent.putExtra("count", unreadCount + 1);
                context.sendBroadcast(intent, "com.mediatek.rcse.permission.RCS_ACCESS_MESSAGE");
            }
            // end sending intent
            ++mChatWindowUnreadMessageNum;
            changeUnreadMessageNum(MIN_STEP_UNREAD_MESSAGE_NUM, true);
            return null;
        }

        /**
         * Update participants.
         *
         * @param participants the participants
         */
        @Override
        public void updateParticipants(List<ParticipantInfo> participants) {
        }

        /**
         * Sets the is composing.
         *
         * @param isComposing the is composing
         * @param participant the participant
         */
        @Override
        public void setIsComposing(boolean isComposing, Participant participant) {
        }

        /**
         * Update chat status.
         *
         * @param status the status
         */
        @Override
        public void updateChatStatus(int status) {
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
            return null;
        }

        /**
         * Removes the chat message.
         *
         * @param messageId the message id
         */
        @Override
        public void removeChatMessage(String messageId) {
            // TODO Auto-generated method stub

        }

        /**
         * Addgroup subject.
         *
         * @param subject the subject
         */
        @Override
        public void addgroupSubject(String subject) {
            // TODO Auto-generated method stub

        }

        /**
         * Sets the m chat id.
         *
         * @param mChatID the new m chat id
         */
        @Override
        public void setmChatID(String mChatID) {
            // TODO Auto-generated method stub

        }

        /**
         * Clear extra message.
         */
        @Override
        public void clearExtraMessage() {
            // TODO Auto-generated method stub

        }

        /**
         * Load message id list.
         */
        @Override
        public void loadMessageIdList() {
            // TODO Auto-generated method stub

        }

		@Override
		public void setClosedGroupType(boolean type) {
			// TODO Auto-generated method stub

		}

        @Override
        public void onSessionStarted() {
            // TODO Auto-generated method stub

        }
    }

    @Override
    public IMultiChatWindow addMultiChatWindow(Object tag,
            List<ParticipantInfo> participantList) {
        Logger.d(TAG, "addMultiChatWindow()");
        return null;
    }
}
