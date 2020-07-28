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

package com.mediatek.rcs.provisioning;

import com.mediatek.rcs.provisioning.IAcsCallback;
import com.mediatek.rcs.provisioning.AcsConfigInfo;

interface IAcsService {
    /**
     * Retrieve the ACS Configuration info
     *
     * @return the ACS configuration info.
     *
     */
    AcsConfigInfo getAcsConfiguration();

    /**
     * Retrieve specified numeric configuration item by config key.
     *
     * @param configItem config key used to retrieve specified item value.
     *
     * @return Integer config value.
     */
    int getAcsConfigInt(String configItem);

    /**
     * Retrieve specified text configuration item by config key.
     *
     * @param configItem config key used to retrieve specified item value.
     *
     * @return String config value.
     */
    String getAcsConfigString(String configItem);

    // /**
    //  * Set the Acs needed config parameters.
    //  *
    //  * @param clientVendor  : client vendor
    //  * @param clientVersion : client version
    //  * @param defaultSmsApp : default sms app
    //  */
    // void setAcsConfigParameters(String clientVendor, String clientVersion, String defaultSmsApp);

    void setRcsSwitchState(boolean state);

    /**
     * Get the Acs connection status.
     * 0: connected
     * 1: disconnected
     *
     * @return current Acs connection status.
     */
    int getAcsConnectionStatus();

    /**
     * Trigger the ACS service to do the Action
     *
     * @param reason
     * 0 : REASON_DEFAULT_SMS_UNKNOWN,
     * 1 : REASON_RCS_DEFAULT_SMS,
     * 2 : REASON_RCS_NOT_DEFAULT_SMS
     *
     * @return trigger request result.
     * true : trigger action success, the action will be executed.
     * false: trigger action fail, the action will not be executed.
     */
    boolean triggerAcsRequest(int reason);

    /**
     * Register callback to Acs service, to listen the event
     * notification.
     *
     * @param callback used to get ACS event notification.
     */
    void registerAcsCallback(IAcsCallback callback);

    /**
     * Unregister callback to Acs service.
     *
     * @param callback used to get ACS event notification.
     */
    void unregisterAcsCallback(IAcsCallback callback);

    /**
     * Set ACS support status.
     *
     * @param state
     * true : means switch on ACS.
     * false: means switch off ACS.
     *
     * @return Set ACS switch state result.
     * true : success to change ACS switch state.
     * false: fail to change ACS switch state.
     */
    boolean setAcsSwitchState(boolean state);

    /**
     * Get ACS support status.
     *
     * @return ACS switch state.
     * true : means ACS switchs on.
     * false: means ACS switchs off.
     *
     */
    boolean getAcsSwitchState();

    /**
     * Set ACS provisioning address.
     *
     * @param address ACS provisioing address.
     *
     * @return Set ACS provisioning address result.
     * true : set provisioning address success.
     * false: set provisioning address fail.
     *
     */
    boolean setAcsProvisioningAddress(String address);

    /**
     * Clear ACS configuration.
     *
     * @return Clear ACS configuration result.
     * true : success to clear ACS configuration.
     * false: fail to clear ACS configuration.
     */
    boolean clearAcsConfiguration();


    /**
     * Set MSISDN information to ACS
     *
     * @param msisdn MSISDN
     *
     * @return Set msisdn result
     * true : success to set msisdn to ACS
     * false: fail to set msisdn to ACS
     */
    boolean setAcsMsisdn(String msisdn);
}
