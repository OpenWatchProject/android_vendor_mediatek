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

package com.mediatek.presence.core.ims.service.sip;

import java.util.Enumeration;
import java.util.Vector;

import android.content.Intent;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipTransactionContext;
import com.mediatek.presence.core.ims.service.ImsService;
import com.mediatek.presence.core.ims.service.ImsServiceSession;
import com.mediatek.presence.core.ims.service.SessionAuthenticationAgent;
import com.mediatek.presence.utils.IdGenerator;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;

/**
 * SIP service
 * 
 * @author Jean-Marc AUFFRET
 */
public class SipService extends ImsService {
    /**
     * The logger
     */
    private Logger logger = null;

    /**
     * MIME-type for multimedia services
     */
    public final static String MIME_TYPE = "application/*";

    private int mSlotId = 0;

    /**
     * Constructor
     * 
     * @param parent IMS module
     * @throws CoreException
     */
    public SipService(ImsModule parent) throws CoreException {
        super(parent, true);
        logger = Logger.getLogger(mSlotId, "SipService");
    }

    /**
     * /** Start the IMS service
     */
    public synchronized void start() {
        if (isServiceStarted()) {
            // Already started
            return;
        }
        setServiceStarted(true);
    }

    /**
     * Stop the IMS service
     */
    public synchronized void stop() {
        if (!isServiceStarted()) {
            // Already stopped
            return;
        }
        setServiceStarted(false);
    }

    /**
     * Check the IMS service
     */
    public void check() {
    }

    /**
     *ImsService error handling
     */
    @Override
    public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
    }

    @Override
    public void onNotProvisionedReceived() {
    }

    /**
     * Receive an instant message
     * 
     * @param intent Resolved intent
     * @param message Instant message request
     */
    public void receiveInstantMessage(Intent intent, SipRequest message) {
        // Send a 200 OK response
        try {
            if (logger.isActivated()) {
                logger.info("Send 200 OK");
            }
             SipResponse response = SipMessageFactory.createResponse(message,
                    IdGenerator.getIdentifier(), 200);
            getImsModule().getSipManager().sendSipResponse(response);
        } catch(Exception e) {
               if (logger.isActivated()) {
                logger.error("Can't send 200 OK response", e);
            }
               return;
        }

        // Notify listener
        getImsModule().getCore().getListener().handleSipInstantMessageReceived(intent, message);
    }
}
