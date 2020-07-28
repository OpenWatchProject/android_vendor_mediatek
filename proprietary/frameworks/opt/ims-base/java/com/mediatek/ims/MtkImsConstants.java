/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.ims;

/**
 * Provides constants for MTK IMS services
 * @hide
 */
public class MtkImsConstants {
    /// M: For IMS deregister done notification. @{
    public static final String ACTION_IMS_SERVICE_DEREGISTERED =
            "com.android.ims.IMS_SERVICE_DEREGISTERED";

    public static final String ACTION_MTK_IMS_SERVICE_UP =
            "com.mediatek.ims.MTK_IMS_SERVICE_UP";

    public static final String ACTION_MTK_MMTEL_READY =
            "com.mediatek.ims.MTK_MMTEL_READY";

    public static final String PROPERTY_IMS_SUPPORT = "persist.vendor.ims_support";
    public static final String MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    public static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";

    /**
     * For accessing the Mediatek IMS related service. Internal use only.
     * @hide
     */
    public static final String MTK_IMS_SERVICE = "mtkIms";

    /**
    * Define IMS service support emergency event, extend by AOSP
    * registrationServiceCapabilityChanged event parameter comment
    */
    public static final int SERVICE_REG_CAPABILITY_EVENT_ECC_SUPPORT = 2;
    public static final int SERVICE_REG_CAPABILITY_EVENT_ECC_NOT_SUPPORT = 4;

    /**
     * Key to retrieve the sequence number from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_SEQ_NUM = "android:imsSeqNum";

    /*
     * Key to retrieve the sequence number from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_DIAL_STRING = "android:imsDialString";
    /// @}

    /*
     * Key to retrieve the sequence number from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_MT_TO_NUMBER = "mediatek:mtToNumber";

    /// M: IMS VoLTE refactoring. @{
    /**
     * Key to retrieve the call mode from an incoming call intent.
     * @see #open(PendingIntent, ImsConnectionStateListener)
     * @hide
     */
    public static final String EXTRA_CALL_MODE = "android:imsCallMode";

    public static final String EXTRA_PHONE_ID = "android:phoneId";

    /**
     * Action for the incoming call indication intent for the Phone app.
     * Internal use only.
     * @hide
     */
    public static final String ACTION_IMS_INCOMING_CALL_INDICATION =
            "com.android.ims.IMS_INCOMING_CALL_INDICATION";

    /** @}*/
    /**
     * For notify UI wifi PDN Out Of Service State
     */
    public static final int SERVICE_REG_EVENT_WIFI_PDN_OOS_START            = 5;
    public static final int SERVICE_REG_EVENT_WIFI_PDN_OOS_END_WITH_DISCONN = 6;
    public static final int SERVICE_REG_EVENT_WIFI_PDN_OOS_END_WITH_RESUME  = 7;
    public static final int OOS_END_WITH_DISCONN = 0;
    public static final int OOS_START = 1;
    public static final int OOS_END_WITH_RESUME = 2; // default

    /** @}*/
    /**
     * MIMS WFC Settings Key
     */
    public static final String WFC_IMS_ENABLED_SIM2 = "wfc_ims_enabled_sim2";
    public static final String WFC_IMS_ENABLED_SIM3 = "wfc_ims_enabled_sim3";
    public static final String WFC_IMS_ENABLED_SIM4 = "wfc_ims_enabled_sim4";

    /**
    *    MTK carrier config
    */

    /*
    * For Call blokcing enhancement feature, reject call with cause.
    */
    public static final String MTK_KEY_SUPPORT_ENHANCED_CALL_BLOCKING_BOOL =
            "mtk_support_enhanced_call_blocking_bool";

    /**
    * MTK IMS Config
    *
    */
    public static final int MTK_CONFIG_START = 1000;

    public static class ConfigConstants {

        // Define IMS config items
        public static final int CONFIG_START = MTK_CONFIG_START;

        // Define operator provisioned config items
        public static final int PROVISIONED_CONFIG_START = CONFIG_START;

        // Expand the operator config items as needed here.
        /**
         * FQDN address for WFC ePDG.
         * Value is in String format.
         * Operator: Op06
         * Specification: R19.0.1, VoWiFi Provisioning utilising SMS
         */
        public static final int EPDG_ADDRESS = CONFIG_START;

        /**
         * Publish retry timer when receiving error code =
         * 408, 500, 503, and 603
         * Operator: Op12
         */
        public static final int PUBLISH_ERROR_RETRY_TIMER = CONFIG_START + 1;

        // VzW opt-in MDN requirement
        public static final int VOICE_OVER_WIFI_MDN = CONFIG_START + 2;

        /**
         * Voce domain preference.
         * Value is in Integer format.
         * Operator: Op236
         */
        public static final int VOICE_DOMAIN_PREFERENCE = CONFIG_START + 3;

        // Expand the operator config items as needed here, need to change
        // PROVISIONED_CONFIG_END after that.
        public static final int PROVISIONED_CONFIG_END = VOICE_DOMAIN_PREFERENCE;

        // Expand the operator config items as needed here.
    }

    public static final int IMS_REGISTERING = 0;
    public static final int IMS_REGISTERED = 1;
    public static final int IMS_REGISTER_FAIL = 2;
}
