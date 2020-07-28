package com.mediatek.mms.callback;

import android.net.Uri;

public interface IMessageItemCallback {
    boolean isMmsCallback();
    boolean isDownloadedCallback();
    String getType();
    String getBody();
    int getBoxIdCallback();

    /**
     * IsFailedMessage.
     * @return true if the message is failed, else return false.
     */
    boolean isFailedMessage();

    /**
     * IsOutgoingMessage.
     * @return true if the message is outgoing message.
     */
    boolean isOutgoingMessage();

    /**
     * IsReceivedMessage.
     * @return true if the message is received message; else return false.
     */
    boolean isReceivedMessage();

    /**
     * Get sub id of the message.
     * @return sub id.
     */
    int getSubId();

    /**
     * Get the address of the message.
     * @return address of the message.
     */
    String getAddress();

    /**
     * Get MMS status.
     * @return mms status.
     */
    int getMmsStatus();

    /**
     * Get MMS Message type.
     * @return MMS Message type.
     */
    int getMessageType();

    /**
     * Get Sub Message type.
     * @return Sub Message type.
     */
    boolean getIsSubMessage();

    /**
     * Get message date .
     * @return Message date-time.
     */
    long getMessageDate();

    /**
     * Get message URI.
     * @return Message URI.
     */
    Uri getMessageUri();

    /**
     * setAddress.
     * @param address address
     */
    void setAddress(String address);

    /**
     * setSubId.
     * @param subId int
     */
    void setSubId(int subId);

    /**
     * setBody.
     * @param body String
     */
    void setBody(String body);

    /**
     * Set Uri callback.
     * @param uri Uri
     */
    void setUri(Uri uri);

    /**
     * Set time stamp.
     * @param timeStamp String
     */
    void setTimeStamp(String timeStamp);

    /**
     * set locked.
     * @param locked boolean
     */
    void setLocked(boolean locked);

    /**
     * Set delivery status.
     * @param status must be MessageItem.DeliveryStatus
     */
    void setDeliveryStatus(Object status);

    /**
     * Set time divider string content.
     * @param dividerString String
     */
    void setTimeDivider(String dividerString);
}
