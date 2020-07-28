/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.presence.core.ims.network;

import android.net.ConnectivityManager;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.access.OnlyApnNetworkAccess;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Only apn network interface
 */
public class OnlyApnNetworkInterface extends ImsNetworkInterface {
    /**
     * The logger
     */
    private Logger logger = null;

    /**
     * Constructor
     * 
     * @param imsModule IMS module
     * @return
     * @throws CoreException
     */
    public OnlyApnNetworkInterface(ImsModule imsModule) throws CoreException {
        super(imsModule, ConnectivityManager.TYPE_MOBILE, new OnlyApnNetworkAccess(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getAllImsProxyAddrForMobile(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getAllImsProxyPortForMobile(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getSipDefaultProtocolForMobile(),
                RcsSettingsManager.getRcsSettingsInstance(imsModule.getSlotId()).getImsAuhtenticationProcedureForMobile());
        int slotId = imsModule.getSlotId();
        logger = Logger.getLogger(slotId, "OnlyApnNetworkInterface");
        /**
         * M: add for MSRPoTLS
         */
        if(RcsSettingsManager.getRcsSettingsInstance(slotId).isSecureMsrpOverMobile()){
            logger.info("OnlyApnNetworkInterface initSecureTlsMsrp0");
            initSecureTlsMsrp(true);
        }
        else if(RcsSettingsManager.getRcsSettingsInstance(slotId).getSipDefaultProtocolForMobile() == "TLS"){
            logger.info("OnlyApnNetworkInterface initSecureTlsMsrp1");
            initSecureTlsMsrp(true);
        }
        else{
            logger.info("OnlyApnNetworkInterface initSecureTlsMsrp2");
            initSecureTlsMsrp(false);
            }
        /**
         * @}
         */
        if (logger.isActivated()) {
            logger.info("OnlyApn network interface has been loaded");
        }
    }
}
