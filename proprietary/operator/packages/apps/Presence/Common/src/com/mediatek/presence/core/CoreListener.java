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

package com.mediatek.presence.core;

import android.content.Intent;

import com.mediatek.presence.core.ims.ImsError;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.core.ims.service.presence.pidf.PidfDocument;



import java.util.ArrayList;

/**
 * Observer of core events
 *
 * @author Jean-Marc AUFFRET
 */
public interface CoreListener {

    /**
     * M: Added to resolve the sip option timeout issue. @{
     */
    public static final ArrayList<String> OFFLINE_CONTACTS = new ArrayList<String>();
    /**
     * @}
     */

    /**
     * Core layer has been started
     */
    public void handleCoreLayerStarted();

    /**
     * Core layer has been stopped
     */
    public void handleCoreLayerStopped();

    /**
     * Registered to IMS
     * @param slotId Slot index
     */
    public void handleRegistrationSuccessful(int slotId);

    /**
     * IMS registration has failed
     *
     * @param slotId Slot index
     * @param error Error
     */
    public void handleRegistrationFailed(int slotId, ImsError error);

    /**
     * Unregistered from IMS
     *
     * @param slotId Slot index
     */
    public void handleRegistrationTerminated(int slotId);

    /**
     * A new presence sharing notification has been received
     *
     * @param contact Contact
     * @param status Status
     * @param reason Reason
     */
    public void handlePresenceSharingNotification(String contact, String status, String reason);

    /**
     * A new presence info notification has been received
     *
     * @param slotId Slot index
     * @param contact Contact
     * @param presense Presence info document
     */
    public void handlePresenceInfoNotification(int slotId,
            String contact, PidfDocument presence);

    /**
     * Capabilities update notification has been received
     *
     * @param contact Contact
     * @param capabilities Capabilities
     */
    public void handleCapabilitiesNotification(String contact, Capabilities capabilities);

    /**
     * A new presence sharing invitation has been received
     *
     * @param contact Contact
     */
    public void handlePresenceSharingInvitation(String contact);

    /**
     * New message delivery status
     *
     * @param contact Contact
     * @param msgId Message ID
     * @param status Delivery status
     */
    public void handleMessageDeliveryStatus(String contact, String msgId, String status);

    /**
    * New message delivery status
    *
    * @param contact Contact
    * @param msgId Message ID
    * @param status Delivery status
    */
   public void handleMessageDeliveryStatus(String contact, String msgId, String status, int errorCode, String statusCode);

    /**
     * New file delivery status
     *
     * @param ftSessionId File transfer session Id
     * @param status Delivery status
     */
    public void handleFileDeliveryStatus(String ftSessionId, String status, String contact);

    /**
     * New SIP instant message received
     *
     * @param intent Resolved intent
     * @param message Instant message request
     */
    public void handleSipInstantMessageReceived(Intent intent, SipRequest message);

    /**
     * User terms confirmation request
     *
     * @param remote Remote server
     * @param id Request ID
     * @param type Type of request
     * @param pin PIN number requested
     * @param subject Subject
     * @param text Text
     * @param btnLabelAccept Label of Accept button
     * @param btnLabelReject Label of Reject button
     * @param timeout Timeout request
     */
    public void handleUserConfirmationRequest(String remote, String id,
            String type, boolean pin, String subject, String text,
            String btnLabelAccept, String btnLabelReject, int timeout);

    /**
     * User terms confirmation acknowledge
     *
     * @param remote Remote server
     * @param id Request ID
     * @param status Status
     * @param subject Subject
     * @param text Text
     */
    public void handleUserConfirmationAck(String remote, String id, String status, String subject, String text);

    /**
     * User terms notification
     *
     * @param remote Remote server
     * @param id Request ID
     * @param subject Subject
     * @param text Text
     * @param btnLabel Label of OK button
     */
    public void handleUserNotification(String remote, String id, String subject, String text, String btnLabel);

    /**
     * SIM has changed
     */
    public void handleSimHasChanged();


    /**M
     * added to how notification of connecting and disconnecting states during registration
     */
    /**
     * handle try register
     */

    public void handleTryRegister();

    /**
     * handle try deregister
     */

    public void handleTryDeregister();
    /** @*/

}
