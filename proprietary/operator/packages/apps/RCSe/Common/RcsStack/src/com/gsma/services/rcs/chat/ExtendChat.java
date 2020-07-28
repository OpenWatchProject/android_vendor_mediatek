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

import java.util.List;

import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.Logger;

/**
 * SingleChat
 */
public class ExtendChat {

    /**
     * Direction of the group chat
     */
    public static class ErrorCodes {
        /**
         * Message being sent
         */
        public static final int TIMEOUT = 1;

        /**
         * Message sent
         */
        public static final int UNKNOWN = 2;

        /**
         * Message delivered to remote
         */
        public static final int INTERNAL = 3;

        /**
         * Message sending failed
         */
        public static final int OUTOFSIZE = 4;
    }

    /**
     * Chat interface
     */
    protected IExtendChat chatInf;

    public static final String TAG = "TAPI-Chat";

    /**
     * Constructor
     *
     * @param chatIntf Chat interface
     */
    ExtendChat(IExtendChat chatIntf) {
        this.chatInf = chatIntf;
    }

    /**
     * Returns the remote contact
     *
     * @return Contact
     * @throws JoynServiceException
     */
    public List<String> getRemoteContacts() throws JoynServiceException {
        Logger.i(TAG, "getRemoteContact entry");
        try {
            return chatInf.getRemoteContacts();
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    /**
     * Sends a chat message
     *
     * @param message Message
     * @return Unique message ID or null in case of error
     * @throws JoynServiceException
     */
    public ChatMessage sendMessage(String message) throws JoynServiceException {
        Logger.i(TAG, "sendMessage entry " + message);
        return sendMessage(message, ChatLog.Message.Type.CONTENT);
    }

    /**
     * Sends a chat message
     *
     * @param message Message
     * @return Unique message ID or null in case of error
     * @throws JoynServiceException
     */
    public ChatMessage sendMessage(String message, int msgType) throws JoynServiceException {
        Logger.i(TAG, "sendMessage entry " + message + " with Type " + msgType);
        try {
            return new ChatMessage(chatInf.sendMessage(message, msgType));
        } catch (Exception e) {
            e.printStackTrace();
            throw new JoynServiceException(e.getMessage());
        }
    }

    /**
     * Sends a burn report for a given burn message ID
     *
     * @param msgId Message ID
     * @throws JoynServiceException
     */
    public void sendBurnedDeliveryReport(String msgId) throws JoynServiceException {
        Logger.i(TAG, "sendDeliveryReport entry " + msgId);
        try {
            chatInf.sendBurnedDeliveryReport(msgId);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    /**
     * Sends a displayed delivery report for a given message ID
     *
     * @param msgId Message ID
     * @throws JoynServiceException
     */
    public void sendDisplayedDeliveryReport(String msgId) throws JoynServiceException {
        Logger.i(TAG, "sendDisplayedDeliveryReport entry " + msgId);
        try {
            chatInf.sendDisplayedDeliveryReport(msgId);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }

    /**
     * @param msgId message Id of message
     * @return state of the message
     * @throws JoynServiceException
     */
    public void resendMessage(String msgId) throws JoynServiceException {
        Logger.i(TAG, "resendMessage msgId " + msgId);
        try {
            chatInf.resendMessage(msgId);
        } catch (Exception e) {
            e.printStackTrace();
            throw new JoynServiceException(e.getMessage());
        }
    }

    /**
     * @param msgId message Id of message
     * @return state of the message
     * @throws JoynServiceException
     */
    public String getExtendChatId() throws JoynServiceException {
        Logger.i(TAG, "getExtendChatId ");
        try {
            return chatInf.getExtendChatId();
        } catch (Exception e) {
            e.printStackTrace();
            throw new JoynServiceException(e.getMessage());
        }
    }

   /* *//**
     * @param msgId message Id of message
     * @return state of the message
     * @throws JoynServiceException
     *//*
    public String prosecuteMessage(String msgId) throws JoynServiceException {
        Logger.i(TAG, "prosecute message of msgId " + msgId);
        try {
            return chatInf.prosecuteMessage(msgId);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }*/

   /* *//**
     * Adds a listener on chat events
     *
     * @param listener Chat event listener
     * @throws JoynServiceException
     *//*
    public void addEventListener(ExtendChatListener listener) throws JoynServiceException {
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
    public void removeEventListener(ExtendChatListener listener) throws JoynServiceException {
        Logger.i(TAG, "removeEventListener entry " + listener);
        try {
            chatInf.removeEventListener(listener);
        } catch (Exception e) {
            throw new JoynServiceException(e.getMessage());
        }
    }
*/
    /**
     * Judge whether interface binder alive
     *
     * @return whether interface alive
     */
    public boolean isAlive() {
        return chatInf.asBinder().isBinderAlive();
    }
}
