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

import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.access.MobileNetworkAccess;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Mobile network interface
 *
 * @author JM. Auffret
 */
public class MobileNetworkInterface extends ImsNetworkInterface {
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     *
     * @param imsModule IMS module
     * @throws CoreException
     */
    public MobileNetworkInterface(ImsModule imsModule) throws CoreException {
        super(imsModule, NetworkCapabilities.TRANSPORT_CELLULAR,
                new MobileNetworkAccess(imsModule.getSlotId()),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId())
                        .getAllImsProxyAddrForMobile(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId())
                        .getAllImsProxyPortForMobile(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId())
                        .getSipDefaultProtocolForMobile(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId())
                        .getImsAuhtenticationProcedureForMobile());
        int slotId = imsModule.getSlotId();
      /**
         * M: add for MSRPoTLS
         */
        if(RcsSettingsManager.getRcsSettingsInstance(slotId).isSecureMsrpOverMobile()){
            logger.info("MobileNetworkInterface initSecureTlsMsrp0");
            initSecureTlsMsrp(true);
        }
        else if(RcsSettingsManager.getRcsSettingsInstance(slotId)
                .getSipDefaultProtocolForMobile() == "TLS"){
            logger.info("MobileNetworkInterface initSecureTlsMsrp1");
            initSecureTlsMsrp(true);
        }
        else{
            logger.info("MobileNetworkInterface initSecureTlsMsrp2");
            initSecureTlsMsrp(false);
            }
        /**
         * @}
         */
        if (logger.isActivated()) {
            logger.info("Mobile network interface has been loaded");
        }
    }
}
