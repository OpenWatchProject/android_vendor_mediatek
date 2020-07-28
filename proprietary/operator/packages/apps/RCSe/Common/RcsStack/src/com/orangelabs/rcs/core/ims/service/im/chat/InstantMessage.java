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

package com.orangelabs.rcs.core.ims.service.im.chat;

import java.util.Date;

import com.gsma.services.rcs.chat.ChatLog;
import com.gsma.services.rcs.chat.ChatLog.Message.Content.ReasonCode;
import com.gsma.services.rcs.RcsService.Direction;

/**
* Instant message
* 
 * @author Jean-Marc AUFFRET
*/
public class InstantMessage {
    /**
     * MIME type
     */
    public static final String MIME_TYPE = "text/plain";
                
    /**
     * MIME type
     */
    public static final String PUBLIC_MIME_TYPE = "application/xml";

    /**
     * Cloud MIME type
     */
    public static final String CLOUD_MIME_TYPE = "application/cloudfile+xml";
                
    /**
     * Emoticons MIME type
     */
    public static final String EMOTICONS_MIME_TYPE = "application/vemoticon+xml";

    /**
     * Card message MIME type
     */
    public static final String CARD_MIME_TYPE = "application/card+xml";

    /**
     * Remote user
     */
    private String remote;
                
    public void setRemote(String remote) {
        this.remote = remote;
    }

    /**
     * Text message
     */
    private String message;

    /**
     * Receipt date of the message
     */
    private Date receiptAt;

    /**
     * Receipt date of the message on the server (i.e. CPIM date)
     */
    private Date serverReceiptAt;

    /**
     * Message Id
     */
    private String msgId;

    /**
     * Display Name 
     */
    private String displayName;

    /**
     * Flag indicating that an IMDN "displayed" is requested for this message
     */
    private boolean imdnDisplayedRequested = false;

    private int msgType = ChatLog.Message.Type.CONTENT;

    private int direction = Direction.IRRELEVANT.toInt();
    
    private int reason = ReasonCode.UNSPECIFIED.toInt();

    private boolean secondary = false;

    private String extraContent = null;
    
    /**
     * Mime type of the message
     */
    private String mimeType = MIME_TYPE;   

    /**
     * Constructor for outgoing message
     *
     * @param messageId Message Id
     * @param remote Remote user
     * @param message Text message
     * @param imdnDisplayedRequested Flag indicating that an IMDN "displayed" is requested
     */
    public InstantMessage(String messageId, String remote, String message, boolean imdnDisplayedRequested,String displayName) {
        this.msgId = messageId;
        this.remote = remote;
        this.message = message;
        this.imdnDisplayedRequested = imdnDisplayedRequested;
        Date date = new Date();
        this.receiptAt = date;
        this.serverReceiptAt = date;
        this.displayName = displayName;
    }

    /**
     * Constructor for incoming message
     * 
     * @param messageId Message Id
     * @param remote Remote user
     * @param message Text message
     * @param imdnDisplayedRequested Flag indicating that an IMDN "displayed" is requested
     * @param serverReceiptAt Receipt date of the message on the server
     */
    public InstantMessage(String messageId, String remote, String message, boolean imdnDisplayedRequested, Date serverReceiptAt,String displayName) {
        this.msgId = messageId;
        this.remote = remote;
        this.message = message;
        this.imdnDisplayedRequested = imdnDisplayedRequested;
        this.receiptAt = new Date();
        this.serverReceiptAt = serverReceiptAt;
        this.displayName = displayName;
    }

    /**
     * Returns the text message
     * 
     * @return String
     */
    public String getTextMessage() {
        return message;
    }

    /**
     * Returns display name
     * 
     * @return String
     */
    public String getDisplayName() {
        return displayName;
    }

    /**
     * Returns display name
     * 
     * @return String
     */
    public void setDisplayName(String displayName) {
        this.displayName = displayName;
    }

    /**
     * Returns the message Id
     * 
     * @return message Id
     */
    public String getMessageId(){
        return msgId;
    }

    /**
     * Returns the message Id
     * 
     * @return message Id
     */
    public String setMessageId(String msgId){
        return this.msgId = msgId;
    }

    /**
     * Returns the remote user
     * 
     * @return Remote user
     */
    public String getRemote() {
        return remote;
    }

    /**
     * Returns true if the IMDN "displayed" has been requested 
     * 
     * @return Boolean
     */
    public boolean isImdnDisplayedRequested() {
        return imdnDisplayedRequested;
    }

    /**
     * Returns the receipt date of the message
     * 
     * @return Date
     */
    public Date getDate() {
        return receiptAt;
    }

    /**
     * Returns the receipt date of the message on the server
     * 
     * @return Date
     */
    public Date getServerDate() {
        return serverReceiptAt;
    }

    public void setMessageType(int msgType) {
        this.msgType = msgType;
    }

    public int getMessageType() {
        return msgType;
    }

    public void setSecondary(boolean secondary) {
        this.secondary = secondary;
    }

    public boolean toSecondary() {
        return secondary;
    }

    public void setDirection(int msgDirection) {
        this.direction = msgDirection;
    }

    public int getDirection() {
        return direction;
    }

    public void setExtraContent(String content) {
        this.extraContent = content;
    }
    
    public String getExtraContent() {
        return extraContent;
    }
    
    public String getMimeType() {
        return mimeType;
    }

    public void setMimeType(String mimeType) {
        this.mimeType = mimeType;
    }
    
    public int getReason() {
        return reason;
    }

    public void setReason(int reason) {
        this.reason = reason;
    }
}
