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

package com.orangelabs.rcs.core.ims.protocol.sip;

import com.orangelabs.rcs.core.ims.network.ImsNetworkInterface;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PeriodicRefresher;
import com.orangelabs.rcs.utils.logger.Logger;

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
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     */
    public KeepAliveManager(SipInterface sip) {
        this.sip = sip;
        this.period = RcsSettings.getInstance().getSipKeepAlivePeriod();
    }

    // Send a double-CRLF at the beginning to setup the channel
    public void sendHeartBeat(){
        if (logger.isActivated()) {
            logger.debug(" sendHeartBeat: opening channel");
        }
        try {
         this.sip.getDefaultSipProvider().getListeningPoints()[0].sendHeartbeat(sip.getOutboundProxyAddr(), sip.getOutboundProxyPort());
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.debug("Error sending keep alive for opening channel");
            }
        }
    }

/*
    // setup the channel
    public void openChannel(){
        try {
            this.sip.getDefaultSipProvider().getListeningPoints()[0].openChannel(sip.getOutboundProxyAddr(), sip.getOutboundProxyPort());
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.debug("Error  opening channel");
            }
        }
    }
*/

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
