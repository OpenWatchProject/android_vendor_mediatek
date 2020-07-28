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
package com.mediatek.rcse.plugin.message;

import android.content.Context;
import android.content.Intent;

import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.plugin.message.IpMessageConsts.FeatureId;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcse.service.PluginApiManager.ContactInformation;
import com.mediatek.rcse.settings.AppSettings;

import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;
import com.orangelabs.rcs.service.LauncherUtils;

/**
 * This class manages the rcse service and monitor the status of Registration or
 * RcsCoreServiceStatus.
 */
public class IpMessageServiceMananger implements
        PluginApiManager.CapabilitiesChangeListener {
    /**
     * The Constant TAG.
     */
    protected static final String TAG = "IpMessageServiceManger";
    /**
     * The Constant PROVISION_INFO_VERSION_ONE.
     */
    private static final String PROVISION_INFO_VERSION_ONE = "-1";
    /**
     * The Constant PROVISION_INFO_VERSION_ZERO.
     */
    private static final String PROVISION_INFO_VERSION_ZERO = "0";
    /**
     * The Constant PERMENTLY_DISABLE_VALUE.
     */
    private static final String PERMENTLY_DISABLE_VALUE = "-3";
    /**
     * The Constant PROVISION_INFO_VILIDITY_ONE.
     */
    private static final long PROVISION_INFO_VILIDITY_ONE = -1;
    /**
     * The Constant PROVISION_INFO_VILIDITY_ZERO.
     */
    private static final long PROVISION_INFO_VILIDITY_ZERO = 0;

    private static IpMessageServiceMananger sIpMessageServiceMananger;

    private Context mContext;

    public static synchronized IpMessageServiceMananger getInstance(Context context) {
        if (sIpMessageServiceMananger == null) {
            sIpMessageServiceMananger = new IpMessageServiceMananger(context);
        }
        return sIpMessageServiceMananger;
    }

    /**
     * Instantiates a new ip message service mananger.
     *
     * @param context the context
     */
    private IpMessageServiceMananger(Context context) {
        mContext = context;
        PluginApiManager.getInstance().addCapabilitiesChangeListener(this);
    }
    /**
     * Checks if is activated.
     *
     * @return true, if is activated
     */
    public boolean isActivated() {
        return isRcseActivated();
    }
    /**
     * Gets the ip message service id.
     *
     * @return the ip message service id
     */
    public int getIpMessageServiceId() {
        return 0;
    }
    /**
     * Service is ready.
     *
     * @return true, if successful
     */
    public boolean serviceIsReady() {
        return true;
    }
    /**
     * Start ip service.
     */
    public void startIpService() {
        Logger.d(TAG, "startIpService() entry ");
    }
    /**
     * Checks if is enabled.
     *
     * @return true, if is enabled
     */
    public boolean isEnabled() {
        Logger.d(TAG, "isEnabled() entry ");
        return isRcseEnabled();
    }
    /* If joyn messaging disabled, will only use XMS */
    /**
     * Checks if is always send message by joyn.
     *
     * @return true, if is always send message by joyn
     */
    public boolean isAlwaysSendMessageByJoyn() {
        boolean imCaps = false;
        if (RcsSettings.getInstance() == null) {
            RcsSettings.createInstance(mContext);
        }
        imCaps = RcsSettings.getInstance().isImAlwaysOn();
        Logger.d(TAG, "isAlwaysSendMessageByJoyn imcapability is " + imCaps);
        return imCaps;
    }
    /**
     * Checks if is rcse enabled.
     *
     * @return true, if is rcse enabled
     */
    private boolean isRcseEnabled() {
        Logger.d(TAG, "isRcseEnabled() entry");
        RcsSettings.createInstance(mContext);
        RcsSettings rcsSettings = RcsSettings.getInstance();
        boolean isEnabled = false;
        if (rcsSettings != null) {
            isEnabled = rcsSettings.isServiceActivated();
        } else {
            Logger.w(TAG, "isRcseEnabled(), rcsSettings is null");
        }
        Logger.d(TAG, "isRcseEnabled() exit with isEnabled is " + isEnabled);
        return isEnabled;
    }
    /**
     * Checks if is joyn permanently disabled.
     *
     * @return true, if is joyn permanently disabled
     */
    public boolean isJoynPermanentlyDisabled() {
        boolean joynStatus = false;
        if (RcsSettings.getInstance().getProvisioningVersion()
                .equals(PERMENTLY_DISABLE_VALUE)) {
            Logger.d(TAG, "isJoynPermanentlyDisabled" + true);
            return true;
        }
        return joynStatus;
    }
    /**
     * Gets the disable service status.
     *
     * @return the disable service status
     */
    public int getDisableServiceStatus() {
        int status = 0;
        if (AppSettings.getInstance() == null) {
            AppSettings.createInstance(MediatekFactory.getApplicationContext());
        }
        status = AppSettings.getInstance().getDisableServiceStatus();
        Logger.d(TAG, "getDisableServiceStatus" + status);
        return status;
    }
    /**
     * Checks if is always send file by joyn.
     *
     * @return true, if is always send file by joyn
     */
    public boolean isAlwaysSendFileByJoyn() {
        int ftCaps = 0;
        if (RcsSettings.getInstance() == null) {
            RcsSettings.createInstance(mContext);
        }
        ftCaps = RcsSettings.getInstance().getFtHttpCapAlwaysOn();
        Logger.d(TAG, "isAlwaysSendFileByJoyn FTcapability is " + ftCaps);
        if (ftCaps == 1) {
            return true;
        } else {
            return false;
        }
    }
    /* Need to be overridden from host app */
    /**
     * Gets the integration mode.
     *
     * @return the integration mode
     */
    public int getIntegrationMode() {
        int integrationMode = PluginUtils.getMessagingMode();
        if (integrationMode == 1) {
            return IpMessageConsts.IntegrationMode.FULLY_INTEGRATED;
        } else {
            return IpMessageConsts.IntegrationMode.CONVERGED_INBOX;
        }
    }
    /**
     * Get configuration value to check whether RCSe can be used.
     *
     * @return True if RCSe can be used, otherwise false.
     */
    private boolean isRcseActivated() {
        Logger.d(TAG, "isRcseActivated() entry");
        if (false /*LauncherUtils.getDebugMode(mContext)*/) {
            Logger.d(TAG,
                    "isRcseActivated() debug mode,do not care configuration");
            return true;
        }
        return checkIsRcseActivated();
    }
    /**
     * Check is rcse activated.
     *
     * @return true, if successful
     */
    private boolean checkIsRcseActivated() {
        RcsSettings.createInstance(mContext);
        RcsSettings rcsSettings = RcsSettings.getInstance();
        boolean isActivated = false;
        if (rcsSettings != null) {
            long validity = rcsSettings.getProvisionValidity();
            String version = rcsSettings.getProvisioningVersion();
            Logger.d(TAG, "isRcseActivated(),validity is " + validity
                    + ", version is " + version);
            if ((version.equals(PROVISION_INFO_VERSION_ONE)
                    && validity == PROVISION_INFO_VILIDITY_ONE)
                    || (version.equals(PROVISION_INFO_VERSION_ZERO)
                            && validity == PROVISION_INFO_VILIDITY_ZERO)) {
                isActivated = false;
            } else {
                isActivated = true;
            }
        } else {
            Logger.w(TAG, "isRcseActivated(), rcsSettings is null");
        }
        return isActivated;
    }
    /**
     * Checks if is feature supported.
     *
     * @param featureId the feature id
     * @return true, if is feature supported
     */
    public boolean isFeatureSupported(int featureId) {
        Logger.d(TAG, "isFeatureSupported() featureId is " + featureId);
        switch (featureId) {
        case FeatureId.CHAT_SETTINGS:
        case FeatureId.ACTIVITION:
        case FeatureId.ACTIVITION_WIZARD:
        case FeatureId.MEDIA_DETAIL:
        case FeatureId.GROUP_MESSAGE:
        case FeatureId.TERM:
        case FeatureId.FILE_TRANSACTION:
        case FeatureId.EXTEND_GROUP_CHAT:
        case FeatureId.EXPORT_CHAT:
            return true;
        case FeatureId.PARSE_EMO_WITHOUT_ACTIVATE: {
            if (RcsSettings.getInstance().getIMSProfileValue() != null) {
                return true;
            } else {
                return false;
            }
        }
        case FeatureId.APP_SETTINGS: {
            if (AppSettings.getInstance().getDisableServiceStatus() == 2) {
                return false;
            } else {
                return true;
            }
        }
        default:
                return false;
        }
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.service.PluginApiManager.
     * CapabilitiesChangeListener#onCapabilitiesChanged(java.lang.String,
     * com.mediatek.rcse.service.PluginApiManager.ContactInformation)
     */
    public void onCapabilitiesChanged(String contact,
            ContactInformation contactInformation) {
        Logger.d(TAG, "onCapabilitiesChanged() entry capabilities is "
                + contact);
        if (contactInformation.isRcsContact == 1
                && !ContactsListManager.getInstance().isLocalContact(contact)
                && !ContactsListManager.getInstance().isStranger(contact)) {
            ContactsListManager.getInstance().setStrangerList(contact, true);
        }
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.service.PluginApiManager.
     * CapabilitiesChangeListener#onApiConnectedStatusChanged(boolean)
     */
    public void onApiConnectedStatusChanged(boolean isConnected) {
    }
}
