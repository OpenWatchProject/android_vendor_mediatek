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

package com.mediatek.presence.core.ims.network;

import android.net.NetworkCapabilities;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.access.WifiNetworkAccess;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Wi-Fi network interface
 *
 * @author JM. Auffret
 */
public class WifiNetworkInterface extends ImsNetworkInterface {
    /**
     * The logger
     */
    private Logger logger = null;

    /**
     * Constructor
     *
     * @param imsModule IMS module
     * @throws CoreException
     */
    public WifiNetworkInterface(ImsModule imsModule) throws CoreException {
        super(imsModule, NetworkCapabilities.TRANSPORT_WIFI,
                new WifiNetworkAccess(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getAllImsProxyAddrForWifi(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getAllImsProxyPortForWifi(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getSipDefaultProtocolForWifi(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getImsAuhtenticationProcedureForWifi());

        int slotId = imsModule.getSlotId();
        logger = Logger.getLogger(slotId, "WifiNetworkInterface");
        /**
         * M: add for MSRPoTLS 
         */
        if(RcsSettingsManager.getRcsSettingsInstance(slotId).isSecureMsrpOverWifi()){
            logger.info("WifiNetworkInterface initSecureTlsMsrp0");
            initSecureTlsMsrp(true);
        }
        else if(RcsSettingsManager.getRcsSettingsInstance(slotId).getSipDefaultProtocolForWifi() == "TLS"){
            logger.info("WifiNetworkInterface initSecureTlsMsrp1");
            initSecureTlsMsrp(true);
        }
        else{
            logger.info("WifiNetworkInterface initSecureTlsMsrp2");
            initSecureTlsMsrp(false);
            }
        /**
         * @}
         */
       
        if (logger.isActivated()) {
            logger.info("Wi-Fi network interface has been loaded");
        }
    }
}
