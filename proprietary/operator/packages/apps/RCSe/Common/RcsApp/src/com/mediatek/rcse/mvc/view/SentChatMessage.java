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

package com.mediatek.rcse.mvc.view;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.interfaces.ChatView.IChatWindow;
import com.mediatek.rcse.interfaces.ChatView.IChatWindowMessage;
import com.mediatek.rcse.interfaces.ChatView.ISentChatMessage;
import com.mediatek.rcse.mvc.ChatMessage;
import com.mediatek.rcse.mvc.ViewImpl.BaseChatMessage;

import java.util.Collection;
import java.util.Date;

//import com.gsma.services.rcs.chat.ChatMessage;

/**
 * This class defines a outgoing chat message type, which is used to transfer
 * the Model statuses to multiple Views.
 */
public class SentChatMessage extends BaseChatMessage implements
        ISentChatMessage {
    public static final String TAG = "SentChatMessage";

    private Status mStatus = Status.SENDING;
    private Date mDate = new Date();
    protected int mMessageTag = -1;

    /**
     * Instantiates a new sent chat message.
     *
     * @param message the message
     * @param messageTag the message tag
     */
    public SentChatMessage(ChatMessage message, int messageTag) {
        super(message);
        mMessageTag = messageTag;
        mDate = message.getReceiptDate();
    }

    /**
     * Get message tag.
     *
     * @return The message tag.
     */
    public int getMessageTag() {
        return mMessageTag;
    }

    /**
     * Update status.
     *
     * @param status the status
     */
    @Override
    public void updateStatus(Status status) {
        Logger.i(TAG, "updateStatus() entry, status is " + status + ",mChatWindowMap:" + mChatWindowMap.values());
        if (Status.DISPLAYED == mStatus) {
            return;
        }
        if (Status.DELIVERED == mStatus && status == Status.FAILED) {
            return;
        }
        mStatus = status;
        Collection<IChatWindowMessage> chatWindowMessages = mChatWindowMap
                .values();
        for (IChatWindowMessage chatWindowMessage : chatWindowMessages) {
            if (chatWindowMessage instanceof ISentChatMessage) {
                ((ISentChatMessage) chatWindowMessage).updateStatus(mStatus);
            } else {
                Logger.e(TAG,
                        "updateStatus() chatWindowMessage in not a ISentChatMessage");
            }
        }
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
            ISentChatMessage sentChatMessage = chatWindow.addSentMessage(
                    mMessage, mMessageTag);
            if (null != sentChatMessage) {
                sentChatMessage.updateStatus(mStatus);
                mChatWindowMap.put(chatWindow, sentChatMessage);
            } else {
                Logger.e(TAG, "onAddChatWindow() sentChatMessage is null");
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

    /**
     * Update date.
     *
     * @param date the date
     */
    @Override
    public void updateDate(Date date) {
        if (date == null || date.compareTo(mDate) == 0) {
            Logger.w(TAG, "updateDate() invalid date: " + date + " , mDate: "
                    + mDate);
            return;
        }
        mDate = (Date) date.clone();
        // mMessage.setDate(date); //TODO, need to check
        Collection<IChatWindowMessage> chatWindowMessages = mChatWindowMap
                .values();
        for (IChatWindowMessage chatWindowMessage : chatWindowMessages) {
            if (chatWindowMessage instanceof ISentChatMessage) {
                ((ISentChatMessage) chatWindowMessage).updateDate(date);
            } else {
                Logger.e(TAG,
                        "updateDate() chatWindowMessage in not a ISentChatMessage");
            }
        }
    }

    /**
     * Update message.
     *
     * @param message the message
     */
    public void updateMessage(ChatMessage message) {
        mMessage = message;        
        Collection<IChatWindowMessage> chatWindowMessages = mChatWindowMap
                .values();
        try {
            Logger.i(TAG, "updateMessage() entry, message " + message.getId() + "&chatWindowMessages=" + chatWindowMessages.size());
        } catch (Exception e) {
            e.printStackTrace();
        }
        for (IChatWindowMessage chatWindowMessage : chatWindowMessages) {
            if (chatWindowMessage instanceof ISentChatMessage) {
                ((ISentChatMessage) chatWindowMessage).updateMessage(message);
            } else {
                Logger.e(TAG,
                        "updateMessage() chatWindowMessage in not a ISentChatMessage");
            }
        }

    
    }

    /**
     * Update status.
     *
     * @param status the status
     * @param contact the contact
     */
    public void updateStatus(Status status, String contact, Date date) {
        Logger.i(TAG, "updateStatus() entry, status is " + status
                + " contact =" + contact + " Date:" + date);
        /*
         * if (Status.DISPLAYED == mStatus) { return; } if (Status.DELIVERED ==
         * mStatus && status == Status.FAILED) { return; }
         */
        // mStatus = status;
        Collection<IChatWindowMessage> chatWindowMessages = mChatWindowMap
                .values();
        for (IChatWindowMessage chatWindowMessage : chatWindowMessages) {
            if (chatWindowMessage instanceof ISentChatMessage) {
                ((ISentChatMessage) chatWindowMessage).updateStatus(status,
                        contact, date);
            } else {
                Logger.e(TAG,
                        "updateStatus() chatWindowMessage in not a ISentChatMessage");
            }
        }

    }
}
