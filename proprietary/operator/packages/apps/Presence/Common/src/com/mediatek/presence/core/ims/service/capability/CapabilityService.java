/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 ******************************************************************************/
 /*
 * Copyright (C) 2018 MediaTek Inc., this file is modified on 07/05/2018
 * by MediaTek Inc. based on Apache License, Version 2.0.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. See NOTICE for more details.
 */

package com.mediatek.presence.core.ims.service.capability;

import java.util.List;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.service.ImsService;

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Capability discovery service
 *
 * @author jexa7410
 */
public abstract class CapabilityService extends ImsService {

    public Logger logger = null;
    protected int mSlotId = 0;

    /**
     * Constructor
     *
     * @param parent
     *            IMS module
     * @throws CoreException
     */
    public CapabilityService(ImsModule parent) throws CoreException {
        super(parent, true);
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "CapabilityService");
        // Get capability extensions
        CapabilityUtils.updateExternalSupportedFeatures(AndroidFactory.getApplicationContext());
    }

    /**
     * Start the IMS service
     */
    public abstract void start();

    /**
     * Stop the IMS service
     */
    public abstract void stop();

    /**
     * Check the IMS service
     */
    public abstract void check();

    public abstract void requestContactCapabilities(String contact);

    /**
     * Request contact capabilities.
     * Request contact capabilities, force send request to server.
     *
     * @param contact Contact number string
     * @return Capabilities
     */
    public synchronized void forceRequestContactCapabilities(String contact) {
        logger.debug("forceRequestContactCapabilities not support");
    }

    /**
     * Request capabilities for a list of contacts
     *
     * @param contactList
     *            List of contacts
     */
    public abstract void requestContactsCapabilities(List<String> contactList);

    /**
     * Request contact availability.
     *
     * @param contact
     *            Contact
     */
    public abstract void requestContactAvailability(String contact);

    /**
     * Receive a notification (anonymous fecth procedure)
     *
     * @param notify
     *            Received notify
     */
    public abstract void receiveNotification(SipRequest notify);

    /**
     * Receive a capability request (options procedure)
     *
     * @param options
     *            Received options message
     */
    public void receiveCapabilityRequest(SipRequest subscribe) {
        logger.debug("receiveCapabilityRequest not support");
    }

    /**
     * Reset the content sharing capabities for a given contact
     *
     * @param contact
     *            Contact
     */
     //TODO phase out in the future
    public void resetContactCapabilitiesForContentSharing(String contact) {
        logger.debug("resetContactCapabilitiesForContentSharing not support");
    }

    /**
     * Request contact capability with a requestId (AOSP procedure)
     *
     * @param contact contact
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public abstract void requestContactCapabilities(String contact, int requestId);

    /**
     * Request capabilities for a list of contacts with a requestId (AOSP procedure)
     *
     * @param contactList contactList
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public abstract void requestContactsCapabilities(List<String> contactList, int requestId);

    /**
     * Request contact availability (AOSP procedure)
     *
     * @param contact contact
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public abstract void requestContactAvailability(String contact, int requestId);
}
