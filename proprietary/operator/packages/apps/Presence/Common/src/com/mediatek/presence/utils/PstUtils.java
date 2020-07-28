/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.presence.utils;

import android.content.Context;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.ims.ImsException;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.ims.internal.MtkImsConfig;

import java.lang.String;

public class PstUtils {
    /*
     * The logger
     */
    private static Logger logger = Logger.getLogger("PstUtils");
    private static PstUtils instance = null;
    private ImsConfigMonitorService mMonitor = null;

    private PstUtils(Context context) {
        logger.info("PstUtils has been created");
        mMonitor = ImsConfigMonitorService.getInstance(context);
    }

    public static void createInstance(Context context) {
        if (instance == null) {
            instance = new PstUtils(context);
        }
    }

    public static PstUtils getInstance() {
        return instance;
    }

    public static boolean isFeatureProvisioned(int slotId, Context context,
            int featureId, boolean defaultValue) {
        CarrierConfigManager configManager = (CarrierConfigManager)
                context.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        // Don't need provision.
        if (configManager != null) {
            PersistableBundle config = configManager.getConfig();
            if (config != null && !config.getBoolean(
                    CarrierConfigManager.KEY_CARRIER_VOLTE_PROVISIONED_BOOL)) {
                return true;
            }
        }

        boolean provisioned = defaultValue;
        ImsManager imsManager = ImsManager.getInstance(context, slotId);
        if (imsManager != null) {
            try {
                ImsConfig imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    provisioned = imsConfig.getProvisionedValue(featureId)
                            == ImsConfig.FeatureValueConstants.ON;
                }
            } catch (ImsException ex) {
            }
        }

        logger.debug("featureId=" + featureId + " provisioned=" + provisioned);
        return provisioned;
    }

    public static boolean isVowifiProvisioned(int slotId, Context context) {
        return isFeatureProvisioned(slotId, context,
                ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED, false);
    }

    public static boolean isLvcProvisioned(int slotId, Context context) {
        return isFeatureProvisioned(slotId, context,
                ImsConfig.ConfigConstants.LVC_SETTING_ENABLED, false);
    }

    public static boolean isEabProvisioned(int slotId, Context context) {
        return isFeatureProvisioned(slotId, context,
                ImsConfig.ConfigConstants.EAB_SETTING_ENABLED, false);
    }

    public boolean isLvcEnabled(Context context) {
        return mMonitor.isLvcEnabled();
    }

    private static int getImsConfig(int slotId, Context context, int target, int defaultValue) {
        int targetValue = defaultValue;
        ImsManager imsManager = ImsManager.getInstance(context, slotId);
        if (imsManager != null) {
            try {
                ImsConfig imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    targetValue = imsConfig.getProvisionedValue(target);
                }
            } catch (ImsException ex) {
            }
        }
        return targetValue;
    }

    public static int getSIPT1Timer(int slotId, Context context) {
        int defaultValue = 0;
        int sipT1Timer = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.SIP_T1_TIMER, defaultValue);
        logger.debug("sipT1Timer=" + sipT1Timer);
        return sipT1Timer;
    }

    /**
     * Capability discovery status of Enabled (1), or Disabled (0).
     */
    public static boolean getCapabilityDiscoveryEnabled(int slotId, Context context) {
        int defaultValue = ImsConfig.FeatureValueConstants.OFF;
        boolean capabilityDiscoveryEnabled = getImsConfig(slotId, context,
                        ImsConfig.ConfigConstants.CAPABILITY_DISCOVERY_ENABLED,
                        defaultValue) ==
                        ImsConfig.FeatureValueConstants.ON;
        logger.debug("capabilityDiscoveryEnabled=" + capabilityDiscoveryEnabled);
        return capabilityDiscoveryEnabled;
    }

    /**
     * The Maximum number of MDNs contained in one Request Contained List.
     */
    public static int getMaxNumbersInRCL(int slotId, Context context) {
        int defaultValue = 100;
        int maxNumbersInRCL = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.MAX_NUMENTRIES_IN_RCL,
                    defaultValue);
        logger.debug("maxNumbersInRCL=" + maxNumbersInRCL);
        return maxNumbersInRCL;
    }

    /**
     * Expiration timer for subscription of a Request Contained List, used in capability polling.
     */
    public static int getCapabilityPollListSubExp(int slotId, Context context) {
        int defaultValue = 30;
        int capabPollListSubExp = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.CAPAB_POLL_LIST_SUB_EXP,
                    defaultValue);
        logger.debug("getCapabilityPollListSubExp=" + capabPollListSubExp);
        return capabPollListSubExp;
    }

    /**
     * Peiod of time the availability information of a contact is cached on device.
     */
    public static int getAvailabilityCacheExpiration(int slotId, Context context) {
        int defaultValue = 30;
        int availabilityCacheExpiration = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.AVAILABILITY_CACHE_EXPIRATION,
                    defaultValue);
        logger.debug("availabilityCacheExpiration=" + availabilityCacheExpiration);
        return availabilityCacheExpiration;
    }

    public static boolean isMobileDataEnabled(int slotId, Context context) {
        int defaultValue = ImsConfig.FeatureValueConstants.OFF;
        boolean mobileDataEnabled = getImsConfig(slotId, context,
                        ImsConfig.ConfigConstants.CAPABILITY_DISCOVERY_ENABLED,
                        defaultValue) ==
                        ImsConfig.FeatureValueConstants.ON;
        logger.debug("mobileDataEnabled=" + mobileDataEnabled);
        return mobileDataEnabled;
    }

    public static void setMobileDataEnabled(int slotId, Context context, boolean mobileDataEnabled) {
        logger.debug("mobileDataEnabled=" + mobileDataEnabled);
        ImsManager imsManager = ImsManager.getInstance(context, slotId);
        if (imsManager != null) {
            try {
                ImsConfig imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    imsConfig.setProvisionedValue(
                            ImsConfig.ConfigConstants.MOBILE_DATA_ENABLED, mobileDataEnabled?
                            ImsConfig.FeatureValueConstants.ON:ImsConfig.FeatureValueConstants.OFF);
                }
            } catch (ImsException ex) {
                logger.debug("ImsException:" + ex);
            }
        }
    }

    public static int getPublishThrottle(int slotId, Context context) {
        int defaultValue = 60000;
        int publishThrottle = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.SOURCE_THROTTLE_PUBLISH,
                    defaultValue);
        logger.debug("publishThrottle=" + publishThrottle);
        return publishThrottle;
    }

    public static int getPublishTimer(int slotId, Context context) {
        int defaultValue = 1200;
        int publishTimer = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.PUBLISH_TIMER,
                    defaultValue);
        logger.debug("publishTimer =" + publishTimer);
        return publishTimer;
    }

    public static int getPublishTimerExtended(int slotId, Context context) {
        int defaultValue = 86400;
        int publishTimerExt = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.PUBLISH_TIMER_EXTENDED,
                    defaultValue);
        logger.debug("publishTimerExt =" + publishTimerExt);
        return publishTimerExt;
    }

    public static int getCapabilitiesCacheExpiration(int slotId, Context context) {
        int defaultValue = 7776000;
        int capabilitiesCacheExpiration = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.CAPABILITIES_CACHE_EXPIRATION,
                    defaultValue);
        logger.debug("capabilitiesCacheExpiration=" + capabilitiesCacheExpiration);
        return capabilitiesCacheExpiration;
    }

    public static int getSourceThrottlePublishTimer(int slotId, Context context) {
        int defaultValue = 60;
        int throttleTimer = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.SOURCE_THROTTLE_PUBLISH,
                    defaultValue);
        logger.debug("throttleTimer=" + throttleTimer);
        return throttleTimer;
    }

    public static int getCapabilityPollInterval(int slotId, Context context) {
        int defaultValue = 625000;
        int capabilityPollInterval = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.CAPABILITIES_POLL_INTERVAL,
                    defaultValue);
        logger.debug("capabilityPollInterval=" + capabilityPollInterval);
        return capabilityPollInterval;
    }

    public static int getCapabilityPollListSubscriptionExpiration(int slotId, Context context) {
        int defaultValue = 30;
        int expiration = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.CAPAB_POLL_LIST_SUB_EXP,
                    defaultValue);
        logger.debug("CPLS expiration=" + expiration);
        return expiration;
    }

    public static boolean getGzipEnable(int slotId, Context context) {
        int defaultValue = 1;
        int gzipEnable = getImsConfig(slotId, context,
                    ImsConfig.ConfigConstants.GZIP_FLAG,
                    defaultValue);
        logger.debug("gzipEnable=" + gzipEnable);
        return (gzipEnable == 1 ? true : false);
    }

    public static int getPublishErrorRetryTimer(int slotId, Context context) {
        int defaultValue = 21600;
        int publishErrorRetryTimer = getImsConfig(slotId, context,
                    MtkImsConfig.ConfigConstants.PUBLISH_ERROR_RETRY_TIMER,
                    defaultValue);
        logger.debug("publishErrorRetryTimer=" + publishErrorRetryTimer);
        return publishErrorRetryTimer;
    }
}
