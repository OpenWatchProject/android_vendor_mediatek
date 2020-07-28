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
package com.mediatek.rcse.plugin.message;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.RemoteException;
import android.telephony.SmsManager;
import android.telephony.TelephonyManager;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.binder.IRemoteChatWindow;
import com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage;
import com.mediatek.rcse.service.binder.IRemoteSentChatMessage;
import com.mediatek.rcse.mvc.ChatMessage;
import com.mediatek.rcse.service.MediatekFactory;

//import com.gsma.services.rcs.chat.ChatMessage;

/**
 * Plugin Group Chat Window.
 */
public class PluginChatWindow extends IRemoteChatWindow.Stub {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "PluginChatWindow";
    /**
     * The m message manager.
     */
    private IpMessageManager mMessageManager;
    /**
     * The msubject.
     */
    public String msubject = "Group Chat";
    /**
     * The m new group create.
     */
    public boolean mNewGroupCreate = false;

    /**
     * Checks if is m new group create.
     *
     * @return true, if is m new group create
     */
    public boolean ismNewGroupCreate() {
        return mNewGroupCreate;
    }
    /**
     * Sets the m new group create.
     *
     * @param mNewGroupCreate the new m new group create
     */
    public void setmNewGroupCreate(boolean mNewGroupCreate) {
        this.mNewGroupCreate = mNewGroupCreate;
    }
    /**
     * Instantiates a new plugin chat window.
     *
     * @param messageManager the message manager
     */
    public PluginChatWindow(IpMessageManager messageManager) {
        mMessageManager = messageManager;
    }
    /**
     * Adds the load history header.
     *
     * @param showLoader the show loader
     * @throws RemoteException the remote exception
     */
    @Override
    public void addLoadHistoryHeader(boolean showLoader) throws RemoteException {
        Logger.v(TAG, "addLoadHistoryHeader(), showLoader = " + showLoader);
    }
    /**
     * Adds the received message.
     *
     * @param message the message
     * @param isRead the is read
     * @return the i remote received chat message
     * @throws RemoteException the remote exception
     */
    @Override
    public IRemoteReceivedChatMessage addReceivedMessage(ChatMessage message,
            boolean isRead) throws RemoteException {
        Logger.v(TAG, "addReceivedMessage(), message = " + message
                + "isRead = " + isRead);
        PluginReceivedChatMessage receivedChatMessage =
                new PluginReceivedChatMessage(
                message, isRead);
        return receivedChatMessage;
    }
    /**
     * Adds the sent message.
     *
     * @param message the message
     * @param messageTag the message tag
     * @return the i remote sent chat message
     * @throws RemoteException the remote exception
     */
    @Override
    public IRemoteSentChatMessage addSentMessage(ChatMessage message,
            int messageTag) throws RemoteException {
        try {
        Logger.v(TAG, "addSentMessage(), message = " + message
                + " messageTag = " + messageTag);
        if (messageTag == 0) {
        	SmsManager manager = SmsManager.getDefault();
        	manager.sendTextMessage(message.getRemoteContact().toString(), null, message.getContent(), null, null);
        	ContentValues values = new ContentValues();
        	values.put("address", message.getRemoteContact().toString()); // phone number to send
        	values.put("date", System.currentTimeMillis()+"");
        	values.put("read", "1"); // if you want to mark is as unread set to 0
        	values.put("type", "2"); // 2 means sent message
        	values.put("body", message.getContent());
        	Uri uri = Uri.parse("content://sms/");
        	Uri rowUri =  MediatekFactory
                    .getApplicationContext().getContentResolver().insert(uri,values);
        	/* final Uri uri = Uri.fromParts("smsto", message.getContact(), null);
            Intent intent = new Intent(
                    TelephonyManager.ACTION_RESPOND_VIA_MESSAGE, uri);
            intent.putExtra(Intent.EXTRA_TEXT, message.getMessage());
            intent.putExtra("showUI", false);
            MediatekFactory.getApplicationContext().startService(intent);*/
            return null;
        }
        PluginSentChatMessage sentChatMessage;
        if (ismNewGroupCreate()) {
            mNewGroupCreate = false;
            sentChatMessage = new PluginSentChatMessage(mMessageManager,
                    message, messageTag, msubject);
        } else {
            sentChatMessage = new PluginSentChatMessage(mMessageManager,
                    message, messageTag);
        }
        return sentChatMessage;
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            return null;
        }
    }
    /**
     * Removes the all messages.
     *
     * @throws RemoteException the remote exception
     */
    @Override
    public void removeAllMessages() throws RemoteException {
        Logger.v(TAG, "removeAllMessages()");
    }
    /**
     * Update all msg as read.
     *
     * @throws RemoteException the remote exception
     */
    @Override
    public void updateAllMsgAsRead() throws RemoteException {
        Logger.v(TAG, "updateAllMsgAsRead()");
    }
    /**
     * Addgroup subject.
     *
     * @param subject the subject
     */
    public void addgroupSubject(String subject) {
        msubject = subject;
        Logger.v(TAG, "addgroupSubject() PluginChatWindow");
    }
    /**
     * Gets the sent chat message.
     *
     * @param messageId the message id
     * @return the sent chat message
     * @throws RemoteException the remote exception
     */
    @Override
    public IRemoteSentChatMessage getSentChatMessage(String messageId)
            throws RemoteException {
        Logger.v(TAG, "getSentChatMessage() messageId =" + messageId);
        return null;
    }
}