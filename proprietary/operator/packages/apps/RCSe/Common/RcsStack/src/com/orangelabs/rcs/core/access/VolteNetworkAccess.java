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

package com.orangelabs.rcs.core.access;

import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;

import com.orangelabs.rcs.core.CoreException;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Volte access network
 * 
 * @author 
 */
public class VolteNetworkAccess extends NetworkAccess {
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

        
    /**
     * Constructor
     * 
     * @throws CoreException
     */
    public VolteNetworkAccess() throws CoreException {
        super();
    }
            
    
    /**
     * Connect to the network access
     * 
     * @param ipAddress Local IP address
     */
    public void connect(String ipAddress) {
        if (logger.isActivated()) {
            logger.info("Network access connected (" + ipAddress + ")");
        }
        this.ipAddress = ipAddress;
    }
    
    /**
     * Disconnect from the network access
     */
    public void disconnect() {
        if (logger.isActivated()) {
            logger.info("Network access disconnected");
        }
        ipAddress = null;
    }
    
    /**
     * Return the type of access
     * 
     * @return Type
     */
    public String getType() {
        return "volte_rcs_proxy";
    }    
    
    /**
     * Return the network name
     * 
     * @return Name
     */
    public String getNetworkName() {
        String name = "volte_rcs_proxy";
        return name;
    }    
}
