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

import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.utils.logger.Logger;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;

/**
 * Network utils
 *
 * @author hlxn7157
 */
public class NetworkUtils {

    private static Logger logger = Logger.getLogger("NetworkUtils");

    /**
     * Network access type unknown
     */
    public static int NETWORK_ACCESS_UNKNOWN = -1;

    /**
     * Network access type 2G
     */
    public static int NETWORK_ACCESS_2G = 0;
    
    /**
     * Network access type 3G
     */
    public static int NETWORK_ACCESS_3G = 1;
    
    /**
     * Network access type 3G+
     */
    public static int NETWORK_ACCESS_3GPLUS = 2;
    
    /**
     * Network access type Wi-Fi
     */
    public static int NETWORK_ACCESS_WIFI = 3;
    
    /**
     * Network access type 4G LTE
     */
    public static int NETWORK_ACCESS_4G = 4;

    /**
     * Get network access type
     *
     * @param slotId
     * @return Type
     */
    public static int getNetworkAccessType(int slotId) {
        int result = NETWORK_ACCESS_UNKNOWN;
        try {
            ConnectivityManager connectivityMgr = (ConnectivityManager) AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
            int[] subIds = SubscriptionManager.getSubId(slotId);
            if (subIds == null) {
                logger.debug("getNetworkAccessType fail. subIds is null");
                return NETWORK_ACCESS_UNKNOWN;
            }
            TelephonyManager tm = TelephonyManager.from(AndroidFactory.getApplicationContext()).createForSubscriptionId(subIds[0]);
            Network network = connectivityMgr.getActiveNetwork();
            if (network != null) {
                NetworkCapabilities networkCap = connectivityMgr.getNetworkCapabilities(network);
                if (networkCap == null) {
                    return result;
                }
                if (networkCap.hasTransport(NetworkCapabilities.TRANSPORT_WIFI)) {
                    result = NETWORK_ACCESS_WIFI;
                } else {
                    int type = tm.getDataNetworkType();
                    switch (type) {
                        case TelephonyManager.NETWORK_TYPE_GPRS:
                        case TelephonyManager.NETWORK_TYPE_EDGE:
                            result = NETWORK_ACCESS_2G;
                            break;
                        case TelephonyManager.NETWORK_TYPE_UMTS:    // ~ 400-7000 kbps
                        case TelephonyManager.NETWORK_TYPE_HSPA:    // ~ 700-1700 kbps
                            result = NETWORK_ACCESS_3G;
                            break;
                        case TelephonyManager.NETWORK_TYPE_HSDPA:   // ~ 2-14 Mbps
                        case TelephonyManager.NETWORK_TYPE_HSUPA:   // ~ 1-23 Mbps
                        case 15: //TelephonyManager.NETWORK_TYPE_HSPAP (available on API level 13) // ~ 10-20 Mbps
                            result = NETWORK_ACCESS_3GPLUS;
                            break;
                        case 13: //TelephonyManager.NETWORK_TYPE_LTE (available on API level 11) // ~ 10+ Mbps
                            result = NETWORK_ACCESS_4G;
                            break;
                    }
                }
            }
        } catch (Exception e) {
            // Nothing to do
        }
        return result;
    }
}
