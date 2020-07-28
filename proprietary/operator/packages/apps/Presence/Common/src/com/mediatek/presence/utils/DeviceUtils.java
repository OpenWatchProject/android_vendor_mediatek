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

import java.util.UUID;

import android.content.Context;
import android.os.Build;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;

import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;

/***
 * Device utility functions
 * 
 * @author jexa7410
 */
public class DeviceUtils {
    /**
     * UUID
     */
    private static UUID uuid = null;

    private static Logger logger = Logger.getLogger("DeviceUtils");

    /**
     * Returns unique UUID of the device
     *
     * @param slotId slot index
     * @param context Context 
     * @return UUID
     */
    public static UUID getDeviceUUID(int slotId, Context context) {
        if (context == null) {
            return null;
        }

        if (uuid == null) {
            String imei = getImei(slotId, context);
            if (imei == null) {
                // For compatibility with device without telephony
                imei = getSerial();
            }
            if (imei != null) {
                uuid = UUID.nameUUIDFromBytes(imei.getBytes());
            }
        }

        return uuid;
    }

    /**
     * Returns the serial number of the device. Only works from OS version Gingerbread.
     * 
     * @return Serial number
     */
    private static String getSerial() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD) {
            return android.os.Build.SERIAL;
        } else {
            return null;
        }
    }

    /**
     * Returns instance ID
     *
     * @param slotId slot index
     * @param context application context
     * @return instance Id
     */
    public static String getInstanceId(int slotId, Context context) {
        if (context == null) {
            return null;
        }

        String instanceId = null;
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isImeiUsedAsDeviceId()) {
            String imei = getImei(slotId, context);
            if (imei != null) { 
                instanceId = "\"<urn:gsma:imei:" + imei + ">\"";
            }
        } else {
            UUID uuid = getDeviceUUID(slotId, context);
            if (uuid != null) {
                instanceId = "\"<urn:uuid:" + uuid.toString() + ">\"";
            }
        }
        return instanceId;
    }

    /**
     * Returns the IMEI of the device
     *
     * @param slotId slot index
     * @param context application context
     * @return IMEI of the device
     */
    private static String getImei(int slotId, Context context) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds == null) {
            logger.debug("getImei fail. subIds is null");
            return "";
        }
        TelephonyManager tm = TelephonyManager.from(context).createForSubscriptionId(subIds[0]);
        return tm.getImei();
    }
}
