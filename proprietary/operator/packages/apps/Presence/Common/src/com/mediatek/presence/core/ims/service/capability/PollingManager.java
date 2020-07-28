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
  /*
 * Copyright (C) 2018 MediaTek Inc., this file is modified on 07/05/2018
 * by MediaTek Inc. based on Apache License, Version 2.0.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. See NOTICE for more details.
 */

package com.mediatek.presence.core.ims.service.capability;

import java.util.List;

import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.PeriodicRefresher;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Polling manager which updates capabilities periodically
 *
 * @author Jean-Marc AUFFRET
 */
public class PollingManager extends PeriodicRefresher {
    /**
     * Capability service
     */
    private NACapabilityService imsService;

    /**
     * The logger
     */
    private Logger logger = null;

    private int mSlotId = 0;

    /**
     * Constructor
     *
     * @param parent IMS service
     */
    public PollingManager(NACapabilityService parent) {
        super("Polling");
        this.imsService = parent;
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "PollingManager");
    }

    /**
     * Start polling
     */
    public void start() {
        periodicProcessing();
    }
    
    /**
     * Stop polling
     */
    public void stop() {
        stopTimer();
    }
    
    /**
     * Update processing
     */
    public void periodicProcessing() {
        int pollingPeriod =  RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .getCapabilityPollingPeriod();
        if (pollingPeriod == 0) {
            logger.info("polling period can not less than 1");
            return;
        }
        // Make a registration
        if (logger.isActivated()) {
            logger.info("periodicProcessing : start");
            logger.info("Execute new capabilities update");
        }
        List<String> contactList = ContactsManager.getInstance().getAllContacts();
        requestContactCapabilities(contactList);
            // Restart timer
        startTimer(pollingPeriod, 1);
     }

    /**
     * Request contact capabilities 
     * 
     * @param contact Contact
     */
    private void requestContactCapabilities(List<String> contact) {
        if (logger.isActivated()) {
            logger.debug("Request capabilities for " + contact);
        }
        // New contacts: request capabilities from the network
        if (RcsSettingsManager.getRcsSettingsInstance(mSlotId).isPresenceDiscoverySupported()) {
            imsService.getAnonymousFetchManager().requestCapabilities(contact);
        } else {
            //TODO Need to refactory
            imsService.getOptionsManager().requestCapabilities(contact);
        }
    }

    /**
     * Request contact capabilities
     *
     * @param contact Contact
     */
    private void requestContactCapabilities(String contact) {
        if (logger.isActivated()) {
            logger.debug("Request capabilities for " + contact);
        }

        // Read capabilities from the database
        Capabilities capabilities = ContactsManager.getInstance().getContactCapabilities(contact);
        if (capabilities == null) {
            if (logger.isActivated()) {
                logger.debug("No capability exist for " + contact);
            }
            // New contact: request capabilities from the network
            if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .isPresenceDiscoverySupported()) {
                imsService.getAnonymousFetchManager().requestCapabilities(
                        contact);
            } else {
            //TODO Need to refactory
            imsService.getOptionsManager().requestCapabilities(contact);
            }
        } else {
            if (logger.isActivated()) {
                logger.debug("Capabilities exist for " + contact);
            }
            int cacheExpiry = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .getCapabilityExpiryTimeout();
            long delta = (System.currentTimeMillis()-capabilities.getTimestamp())/1000;
            if ((delta >= cacheExpiry) || (delta < 0)) {
                if (logger.isActivated()) {
                    logger.debug("Capabilities have expired for " + contact);
                }
                // New contact: request capabilities from the network
                if (RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                        .isPresenceDiscoverySupported()) {
                    imsService.getAnonymousFetchManager().requestCapabilities(
                            contact);
                } else {
                     //TODO Need to refactory
                    imsService.getOptionsManager().requestCapabilities(contact);
                }
            }
        }
    }
}
