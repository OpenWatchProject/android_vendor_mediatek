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

package com.mediatek.presence.provisioning;

import android.content.Context;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;

import com.mediatek.ims.rcsua.AcsEventCallback;
import com.mediatek.ims.rcsua.RcsUaService;

import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsData;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;


public class AcsProvisioningController {

    private Context mContext;
    private Logger logger = Logger.getLogger(
            AcsProvisioningController.class.getSimpleName());

    private static final String KEY_PUBLUSH_THROTTLE = "source-throttlepublish";
    private static final String KEY_SUBSCRIBE_EXPIRY = "pollingPeriod";
    private static final String KEY_NON_RCS_CAPABILITY_INFO_EXPIRY = "nonRCScapInfoExpiry";
    private static final String KEY_CAPABILITY_INFO_EXPIRY = "capInfoExpiry";

    public AcsProvisioningController(Context context) {
        logger.debug("Constructor");
        mContext = context;
    }

    public void initDataBase(int slotId) {
        logger.debug("initDataBase");
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds == null) {
            logger.debug("initDataBase fail. subIds is null");
            return;
        }
        TelephonyManager tm  = new TelephonyManager(mContext, subIds[0]);

            String impi = tm.getIsimImpi();
            String[] impu = tm.getIsimImpu();
            String domain = tm.getIsimDomain();
            String MSISDN="";
            if (impi == null || impu == null || domain == null) {
                logger.error("impi == null || impu == null || domain == null");
                return;
            }
            try {
                MSISDN = extractUserNamePart(impu[0]);
            } catch (Exception e) {
                MSISDN="";
                logger.error("updateSIMDetailsinDB is MSISDN null");
                e.printStackTrace();
            }
            if(MSISDN == null) {
                MSISDN="";
            }
            logger.error("setDefaultProvisioningValue for AT&T: impu[0]: " + impu[0] +
                    ", impi: " + impi + ", domain: " + domain + ", msisdn: " + MSISDN);
            for (int i = 0; i < impu.length; i++) {
                logger.error("impu[" + i + "]:" + impu[i]);
            }

            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setUserProfileImsUserName_full(impu[0]);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setUserProfileImsPrivateId(impi);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setUserProfileImsDomain(domain);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setUserProfileImsDisplayName(MSISDN);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setUserProfileImsUserName(MSISDN);

            //SERVICE CHANGES
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, Boolean.toString(true));
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY, Boolean.toString(true));
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.AUTO_ACCEPT_CHAT,RcsSettingsData.TRUE);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT,RcsSettingsData.TRUE);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.PERMANENT_STATE_MODE,RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_WIFI, RcsSettingsData.AKA_AUTHENT);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE, RcsSettingsData.AKA_AUTHENT);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .writeParameter(RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION, "1");
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setCPMSupported(true);
            RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .setServicePermissionState(true);
            //CAPABILITY INFO EXPIRY shall be 6 hours
            // NON RCS CAPABILITY INFO EXPIRY shall be 72 hours
            logger.debug("chage expiry time to 21600");
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_EXPIRY_TIMEOUT, "21600");
            //Configure RCS related capabilities
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_CS_VIDEO, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_IMAGE_SHARING, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_VIDEO_SHARING, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_IP_VOICE_CALL, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_IP_VIDEO_CALL, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_IM_GROUP_SESSION, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GROUP_CHAT_SF, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_SF, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_SMSOverIP, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_ICSI_MMTEL, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.CAPABILITY_ICSI_EMERGENCY, RcsSettingsData.FALSE);
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                    RcsSettingsData.STANDALONE_MSG_SUPPORT, RcsSettingsData.FALSE);
    }

    public void updateProvisionValues(int slotId, RcsUaService rcsUaService) {
        if (rcsUaService == null) {
            logger.error("rcsUaService is null. Update Provision Values fail.");
            return;
        }

        int publishThrottle = rcsUaService.getAcsConfigInt(KEY_PUBLUSH_THROTTLE);
        logger.debug("publishThrottle: " + publishThrottle);
        if (publishThrottle > 0) {
                RcsSettingsManager.getRcsSettingsInstance(slotId)
                        .writeParameter(RcsSettingsData.SOURCE_THROTTLE_PUBLISH, Integer.toString(publishThrottle));
        }

        int subscribeExpire = rcsUaService.getAcsConfigInt(KEY_SUBSCRIBE_EXPIRY);
        logger.debug("subscribeThrottle: " + subscribeExpire);
        if (subscribeExpire > 0) {
                RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                        RcsSettingsData.SUBSCRIBE_EXPIRE_PERIOD, Integer.toString(subscribeExpire));
        }

        int nonRcsExpiry = rcsUaService.getAcsConfigInt(KEY_NON_RCS_CAPABILITY_INFO_EXPIRY);
        logger.debug("nonRcsExpiry: " + nonRcsExpiry);
        if (nonRcsExpiry > 0) {
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                RcsSettingsData.NON_RCS_CAPABILITY_EXPIRY_TIMEOUT, Integer.toString(nonRcsExpiry));
        }

        int rcsExpiry = rcsUaService.getAcsConfigInt(KEY_CAPABILITY_INFO_EXPIRY);
        logger.debug("rcsExpiry: " + rcsExpiry);
        if (rcsExpiry > 0) {
            RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(
                RcsSettingsData.CAPABILITY_EXPIRY_TIMEOUT, Integer.toString(rcsExpiry));
        }
    }

    private  String extractUserNamePart(String uri) {
        if ((uri == null) || (uri.trim().length() == 0)) {
            return "";
        }

        try {
            uri = uri.trim();
            int index1 = uri.indexOf("sip:");
            if (index1 != -1) {
                int index2 = uri.indexOf("@", index1);
                String result = uri.substring(index1+4, index2);
                return result;
            } else {
                return uri;
            }
        } catch(Exception e) {
            return "";
        }
    }
}
