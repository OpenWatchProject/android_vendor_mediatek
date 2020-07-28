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
package com.mediatek.rcse.mvc;

import java.util.Date;

import com.mediatek.rcse.api.Logger;


import android.os.Parcel;
import android.os.Parcelable;

/**
 * Chat message
 *
 * @author Jean-Marc AUFFRET
 */
public class ChatMessage implements Parcelable {
    /**
     * MIME type
     */
    public final static String MIME_TYPE = "text/plain";

    public static final String TAG = "TAPI-ChatMessage";
    /**
     * Unique message Id
     */
    private String id;

    /**
     * Contact who has sent the message
     */
    private String contact;

    /**
     * Message content
     */
    private String message;

    /**
     * Receipt date of the message
     */
    private Date receiptAt;

    /**
     * Display Name
     */
    private String displayName;

    /**
     * Flag indicating is a displayed report is requested
     */
    private boolean displayedReportRequested = false;

    /**
     * Constructor for outgoing message
     *
     * @param messageId Message Id
     * @param contact Contact
     * @param message Message content
     * @param receiptAt Receipt date
     * @param displayedReportRequested Flag indicating if a displayed report is requested
     * @hide
     */
    public ChatMessage(String messageId, String remote, String message, Date receiptAt, boolean displayedReportRequested, String displayName) {
        Logger.i(TAG, "ChatMessage entry " + "messageId=" + messageId + " remote=" + remote + " message=" + message +
                " receiptAt=" + receiptAt + " displayedReportRequested=" + displayedReportRequested);
        Logger.i(TAG, "ABCG ChatMessage entry " + "displayname=" + displayName);
        this.id = messageId;
        this.contact = remote;
        this.message = message;
        this.displayedReportRequested = displayedReportRequested;
        this.receiptAt = receiptAt;
        this.displayName = displayName;
    }

    /**
     * Constructor
     *
     * @param source Parcelable source
     * @hide
     */
    public ChatMessage(Parcel source) {
        this.id = source.readString();
        this.contact = source.readString();
        this.message = source.readString();
        this.receiptAt = new Date(source.readLong());
        this.displayedReportRequested = source.readInt() != 0;
        this.displayName = source.readString();
    }

    /**
     * Describe the kinds of special objects contained in this Parcelable's
     * marshalled representation
     *
     * @return Integer
     * @hide
     */
    public int describeContents() {
        return 0;
    }

    /**
     * Write parcelable object
     *
     * @param dest The Parcel in which the object should be written
     * @param flags Additional flags about how the object should be written
     * @hide
     */
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(id);
        dest.writeString(contact);
        dest.writeString(message);
        dest.writeLong(receiptAt.getTime());
        dest.writeInt(displayedReportRequested ? 1 : 0);
        dest.writeString(displayName);
    }

    /**
     * Parcelable creator
     *
     * @hide
     */
    public static final Parcelable.Creator<ChatMessage> CREATOR
            = new Parcelable.Creator<ChatMessage>() {
        public ChatMessage createFromParcel(Parcel source) {
            return new ChatMessage(source);
        }

        public ChatMessage[] newArray(int size) {
            return new ChatMessage[size];
        }
    };

    /**
     * Returns the message ID
     *
     * @return ID
     */
    public String getId() {
        return id;
    }

    /**
     * Returns the contact
     *
     * @return Contact
     */
    public String getContact() {
        return contact;
    }
    
    public String getRemoteContact() {
        return contact;
    }

    /**
     * Returns the message content
     *
     * @return String
     */
    public String getMessage() {
        return message;
    }
    
    public String getContent() {
        return message;
    }

    /**
     * Returns the display name
     *
     * @return String
     */
    public String getDisplayName() {
        return displayName;
    }

    /**
     * Returns the receipt date of chat message
     *
     * @return Date
     */
    public Date getReceiptDate() {
        return receiptAt;
    }

    /**
     * Is displayed delivery report requested
     *
     * @return Returns true if requested else returns false
     */
    public boolean isDisplayedReportRequested() {
        return displayedReportRequested;
    }
}
