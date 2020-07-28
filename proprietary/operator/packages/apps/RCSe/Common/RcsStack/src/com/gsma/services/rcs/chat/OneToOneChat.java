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
package com.gsma.services.rcs.chat;

import com.gsma.services.rcs.chat.Geoloc;
import com.gsma.services.rcs.RcsGenericException;
import com.gsma.services.rcs.RcsIllegalArgumentException;
import com.gsma.services.rcs.RcsPersistentStorageException;
import com.gsma.services.rcs.contact.ContactId;

import java.util.ArrayList;
import java.util.HashSet;

import com.gsma.services.rcs.JoynServiceException;

import com.gsma.services.rcs.Logger;

/**
 * OneToOneChat
 *
 * @author Jean-Marc AUFFRET
 */
public class OneToOneChat {

    /**
     * Direction of the group chat
     */
    public static class MessageState {
        /**
         * Message being sent
         */
        public static final int SENDING = 0;

        /**
         * Message sent
         */
        public static final int SENT = 1;

        /**
         * Message delivered to remote
         */
        public static final int DELIVERED = 2;

        /**
         * Message sending failed
         */
        public static final int FAILED = 3;
    }

    /**
     * Chat interface
     */
    protected IOneToOneChat mOneToOneChatInf;

    public static final String TAG = "TAPI-Chat";

    /**
     * Constructor
     *
     * @param chatIntf Chat interface
     */
    OneToOneChat(IOneToOneChat chatIntf) {
        this.mOneToOneChatInf = chatIntf;
    }
    
    /**
     * Returns the remote contact
     * 
     * @return ContactId
     * @throws RcsGenericException
     */
    public ContactId getRemoteContact() throws RcsGenericException {
        try {
            return mOneToOneChatInf.getRemoteContact();

        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Returns true if it is possible to send messages in this one to one chat right now, else
     * return false.
     * 
     * @return boolean
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public boolean isAllowedToSendMessage() throws RcsPersistentStorageException,
            RcsGenericException {
        try {
            return mOneToOneChatInf.isAllowedToSendMessage();

        } catch (Exception e) {
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Sends a chat message
     * 
     * @param message Message
     * @return ChatMessage
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public ChatMessage sendMessage(String message) throws RcsPersistentStorageException,
            RcsGenericException {
        try {
            return new ChatMessage(mOneToOneChatInf.sendMessage(message));
            

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * Sends a geoloc message
     * 
     * @param geoloc Geoloc info
     * @return ChatMessage
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public ChatMessage sendMessage(Geoloc geoloc) throws RcsPersistentStorageException,
            RcsGenericException {
        try {
            return new ChatMessage(mOneToOneChatInf.sendMessage2(geoloc));
            

        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

    /**
     * This method should be called to notify the stack of if there is ongoing composing or not in
     * this OneToOneChat. If there is an ongoing chat session established with the remote side
     * corresponding to this OneToOneChat this means that a call to this method will send the
     * 'is-composing' event or the 'is-not-composing' event to the remote side. However since this
     * method can be called at any time even when there is no chat session established with the
     * remote side or when the stack is not even connected to the IMS server then the stack
     * implementation needs to hold the last given information (i.e. composing or not composing) in
     * memory and then send it later when there is an established session available to relay this
     * information on. Note: if this OneToOneChat corresponds to an incoming pending chat session
     * and the parameter IM SESSION START is 1 then the session is accepted before sending the
     * 'is-composing' event.
     * 
     * @param ongoing True is client application is composing
     * @throws RcsGenericException
     */
    public void setComposingStatus(boolean ongoing) throws RcsGenericException {
        try {
            mOneToOneChatInf.setComposingStatus(ongoing);
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * open the chat conversation.<br>
     * Note: if it is an incoming pending chat session and the parameter IM SESSION START is 0 then
     * the session is accepted now.
     * 
     * @throws RcsGenericException
     */
    public void openChat() throws RcsGenericException {
        try {
            mOneToOneChatInf.openChat();
        } catch (Exception e) {
            throw new RcsGenericException(e);
        }
    }

    /**
     * Resend a message which previously failed.
     * 
     * @param msgId the message ID
     * @throws RcsPersistentStorageException
     * @throws RcsGenericException
     */
    public void resendMessage(String msgId) throws RcsPersistentStorageException,
            RcsGenericException {
        try {
            mOneToOneChatInf.resendMessage(msgId);
        } catch (Exception e) {
            RcsIllegalArgumentException.assertException(e);
            RcsPersistentStorageException.assertException(e);
            throw new RcsGenericException(e);
        }
    }

/*
    *//**
     * Sends a displayed delivery report for a given message ID
     *
     * @param msgId Message ID
     * @throws JoynServiceException
     *//*
    public void sendDisplayedDeliveryReport(String msgId) throws JoynServiceException {
        Logger.i(TAG, "sendDisplayedDeliveryReport entry " + msgId);
        try {
            chatInf.sendDisplayedDeliveryReport(msgId);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }*/

/*    *//**
     * Adds a listener on chat events
     *
     * @param listener Chat event listener
     * @throws JoynServiceException
     *//*
    public void addEventListener(OneToOneChatListener listener) throws JoynServiceException {
        Logger.i(TAG, "addEventListener entry " + listener);
        try {
            chatInf.addEventListener(listener);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    *//**
     * Removes a listener on chat events
     *
     * @param listener Chat event listener
     * @throws JoynServiceException
     *//*
    public void removeEventListener(OneToOneChatListener listener) throws JoynServiceException {
        Logger.i(TAG, "removeEventListener entry " + listener);
        try {
            chatInf.removeEventListener(listener);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }*/
}
