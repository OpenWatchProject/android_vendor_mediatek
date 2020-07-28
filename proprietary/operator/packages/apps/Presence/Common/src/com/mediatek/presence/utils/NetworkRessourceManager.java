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

package com.mediatek.presence.utils;

import com.mediatek.presence.platform.network.DatagramConnection;
import com.mediatek.presence.platform.network.NetworkFactory;
import com.mediatek.presence.platform.network.SocketServerConnection;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Network ressource manager
 *
 * @author jexa7410
 */
public class NetworkRessourceManager {
    /**
     * Generate a default free SIP port number
     *
     * @return Local SIP port
     */
    public static synchronized int generateLocalSipPort(int slotId) {
        int localSipPortBase = RcsSettingsManager.getRcsSettingsInstance(slotId).getSipListeningPort();
        return generateLocalUdpPort(localSipPortBase);
    }

    private static Logger logger = Logger.getLogger(NetworkRessourceManager.class.getName());
    /**
     * @}
     */
    
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
            DatagramConnection conn = NetworkFactory.getFactory().createDatagramConnection();
            conn.open(port);
            conn.close();
            res = true;
        } catch(IOException e) {
            res = false;
        }
        return res;
    }

    /**
     * Is a valid IP address
     *
     * @param ipAddress IP address
     * @return Boolean
     */
    public static boolean isValidIpAddress(String ipAddress) {
        boolean result = false;
        if ((ipAddress != null) &&
                (!ipAddress.equals("127.0.0.1")) &&
                    (!ipAddress.equals("localhost"))) {
            result = true;
        }
        return result;
    }

    /**
     * Convert an IP address to its integer representation
     *
     * @param addr IP address
     * @return Integer
     */
    public static int ipToInt(String addr) {
        String[] addrArray = addr.split("\\.");
        int num = 0;
        for (int i=0; i<addrArray.length; i++) {
            int power = 3-i;
            num += ((Integer.parseInt(addrArray[i])%256 * Math.pow(256,power)));
        }
        return num;
    }
}
