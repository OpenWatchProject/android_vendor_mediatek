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

package com.mediatek.rcse.rtp;

import com.mediatek.rcse.rtp.Logger;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Network ressource manager
 *
 * @author jexa7410
 */
public class NetworkRessourceManager {    
    
    private static Logger logger = Logger.getLogger(NetworkRessourceManager.class.getName());
    public static final int DEFAULT_LOCAL_RTP_PORT_BASE = 10000;//RAppSettings.getInstance().getDefaultRtpPort();
    private static final AtomicInteger sFreeRtpPort = new AtomicInteger(DEFAULT_LOCAL_RTP_PORT_BASE);
    /**
     * Generate a default free RTP port number
     *
     * @return Local RTP port
     */
    public static synchronized int generateLocalRtpPort() {
        /**
         * M: Modified to resolve the issue of reusing network port. @{
         */
        // return generateLocalUdpPort(DEFAULT_LOCAL_RTP_PORT_BASE);
        logger.debug("generateLocalRtpPort()");
        int port = generateLocalUdpPort(sFreeRtpPort.get());
        int nextFreePort = (port + 2) % Integer.MAX_VALUE;
        if(nextFreePort < DEFAULT_LOCAL_RTP_PORT_BASE){
            nextFreePort = nextFreePort + DEFAULT_LOCAL_RTP_PORT_BASE;
        }
        sFreeRtpPort.set(nextFreePort);
        logger.debug("free rtp port = " + port + ", nextFreePort = " + nextFreePort);
        return port;
        /**
         * @}
         */
    }
   

    /**
     * Generate a free UDP port number from a specific port base
     *
     * @param portBase UDP port base
     * @return Local UDP port
     */
    private static int generateLocalUdpPort(int portBase) {
    	int resp = -1;
		int port = portBase;
		while((resp == -1) && (port < Integer.MAX_VALUE)) {
			if (isLocalUdpPortFree(port)) {
				// Free UDP port found
				resp = port;
			} else {
                // +2 needed for RTCP port
                port += 2;
			}
		}
    	return resp;
    }

	/**
     * Test if the given local UDP port is really free (not used by
     * other applications)
     *
     * @param port Port to check
     * @return Boolean
     */
    private static boolean isLocalUdpPortFree(int port) {
    	boolean res = false;
    	try {
    		DatagramConnection conn = new AndroidDatagramConnection();
    		conn.open(port);
            conn.close();
    		res = true;
    	} catch(IOException e) {
    		res = false;
    	}
    	return res;
    }
   
}
