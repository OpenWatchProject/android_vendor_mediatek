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

package com.mediatek.presence.core.ims.protocol.sip;

import com.mediatek.presence.core.ims.network.ImsNetworkInterface;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.PeriodicRefresher;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Keep-alive manager (see RFC 5626)
 *
 * @author BJ
 */
public class KeepAliveManager extends PeriodicRefresher {
    /**
     * Keep-alive period (in seconds)
     */
    private int period;
        
    /**
     * SIP interface
     */
    private SipInterface sip;
    private ImsNetworkInterface networkInterface = null;
    
    /**
     * The logger
     */
    private Logger logger = null;

    /**
     * Constructor
     */
    public KeepAliveManager(int slotId, SipInterface sip) {
        super("KeepAlive");
        this.sip = sip;
        this.period = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .getSipKeepAlivePeriod();
        logger = Logger.getLogger(slotId, "KeepAliveManager");
    }
    
    public void setIMSNetworkInterface (ImsNetworkInterface m){
        
        networkInterface = m;
    }
    
    /**
     * Start
     */
    public void start() {
        if (logger.isActivated()) {
            logger.debug("ABC Start keep-alive");
        }
        startTimer(period, 1);
    }
    
    /**
     * Start
     */
    public void stop() {
        if (logger.isActivated()) {
            logger.debug("ABC Stop keep-alive");
        }
        stopTimer();
    }
    
    /**
     * Keep-alive processing
     */
    public void periodicProcessing() {
        try {
            if (logger.isActivated()) {
                logger.debug("ABC Send keep-alive");
            }

            // Send a double-CRLF
            sip.getDefaultSipProvider().getListeningPoints()[0].sendHeartbeat(sip.getOutboundProxyAddr(), sip.getOutboundProxyPort());
            
            // Start timer
            startTimer(period, 1);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("ABC SIP heartbeat has failed", e);
            }
            
            //ReRegister if exception is RST exception
            if(networkInterface != null){
            networkInterface.getRegistrationManager().doPreReRegistrationProcess();
            networkInterface.getRegistrationManager().registration();

            }
        }
    }

    /**
     * @param period the keep alive period in seconds
     */
    public void setPeriod(int period) {
        this.period = period;
        if (logger.isActivated()) {
            logger.debug("ABC Set keep-alive period \"" + period + "\"");
        }
    }
}
