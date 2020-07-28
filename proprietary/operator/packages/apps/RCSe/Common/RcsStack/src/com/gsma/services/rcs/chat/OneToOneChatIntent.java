/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2014 Sony Mobile Communications Inc.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.gsma.services.rcs.chat;

/**
 * Intent for one-to-one chat conversation
 * 
 * @author Jean-Marc AUFFRET
 */
public class OneToOneChatIntent {

    /**
     * Broadcast action: a new chat message has been received.
     * <p>
     * Intent includes the following extras:
     * <ul>
     * <li> {@link #EXTRA_MESSAGE_ID} containing the message ID of chat message.
     * <li> {@link #EXTRA_MIME_TYPE} containing the MIME-type of chat message.
     * </ul>
     */
    public final static String ACTION_NEW_ONE_TO_ONE_CHAT_MESSAGE = "com.gsma.services.rcs.chat.action.NEW_ONE_TO_ONE_CHAT_MESSAGE";
    
    /**
     * Broadcast action: a new standalone chat message has been received.
     * <p>
     * Intent includes the following extras:
     * <ul>
     * <li> {@link #EXTRA_MESSAGE_ID} containing the message ID of chat message.
     * <li> {@link #EXTRA_MIME_TYPE} containing the MIME-type of chat message.
     * </ul>
     */
    public final static String ACTION_NEW_ONE_TO_ONE_STANDALONE_CHAT_MESSAGE = 
            "com.gsma.services.rcs.chat.action.NEW_ONE_TO_ONE_STANDALONE_CHAT_MESSAGE";

    /**
     * MIME-type of received message
     */
    public final static String EXTRA_MIME_TYPE = "mimeType";

    /**
     * Message ID of received message
     */
    public final static String EXTRA_MESSAGE_ID = "messageId";

    /**
     * Broadcast action: message delivery expired.
     * <p>
     * Intent includes the following extras:
     * <ul>
     * <li> {@link #EXTRA_CONTACT} containing the contact ID of remote contact.
     * <li> {@link #EXTRA_MESSAGE_ID} containing the message ID of the chat message that triggered
     * the timeout.
     * </ul>
     */
    public final static String ACTION_MESSAGE_DELIVERY_EXPIRED = "com.gsma.services.rcs.chat.action.MESSAGE_DELIVERY_EXPIRED";

    /**
     * ContactId of remote contact
     */
    public final static String EXTRA_CONTACT = "contact";
    
    /**
     * Broadcast action: a new chat message has been received.
     * <p>Intent includes the following extras:
     * <ul>
     * <li> {@link #EXTRA_CONTACT} containing the MSISDN of the remote contact
     * <li> {@link #EXTRA_DISPLAY_NAME} containing the display name of the
     *  contact sending the invitation (extracted from the SIP address).
     * <li> {@link #EXTRA_MESSAGE} containing the chat message (parcelable object).
     * </ul>
     */
    //public final static String ACTION_NEW_CHAT = "com.gsma.services.rcs.chat.action.NEW_CHAT";

     /**
     * Broadcast action: a delivery status of a chat message has been received.
     * <p>Intent includes the following extras:
     * <ul>
     * <li> {@link #EXTRA_CONTACT} containing the MSISDN of the remote contact
     * <li> {msgId} containing the message id of the message.
     * <li> {status} containing the delivery status of chat message.
     * </ul>
     */
    public final static String ACTION_DELIVERY_STATUS = "com.gsma.services.rcs.chat.action.DELIEVRY_STATUS";

    /**
     * Broadcast action: any message or delivery status is received and
     * listener is not added in chat.
     * <p>Intent includes the following extras:
     * <ul>
     * <li> {@link #EXTRA_CONTACT} containing the MSISDN of the remote contact
     * <li> {msgId} containing the message id of the message.
     * <li> {status} containing the delivery status of chat message.
     * </ul>
     */
    public final static String ACTION_REINITIALIZE_LISTENER =
    "com.gsma.services.rcs.chat.action.REINITIALIZE_LISTENER";


    /**
     * MSISDN of the contact sending the invitation
     */
    public final static String EXTRA_CLOUD_MESSAGE = "cloudMessage";

    /**
     * Display name of the contact sending the invitation (extracted from the SIP address)
     */
    public final static String EXTRA_DISPLAY_NAME = "contactDisplayname";

    /**
     * Remote status of the corresponding message
     */
    public final static String EXTRA_STATUS = "status";

    /**
     * Received message
     *
     * @see ChatMessage
     * @see GeolocMessage
     */
    //public final static String EXTRA_MESSAGE = "firstMessage";

    private OneToOneChatIntent() {
    }
}
