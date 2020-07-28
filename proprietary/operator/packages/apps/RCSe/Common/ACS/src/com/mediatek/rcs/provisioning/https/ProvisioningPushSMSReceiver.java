/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2016 Sony Mobile Communications Inc.
 * Copyright (C) 2010-2016 Orange.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *******************************************************************************/

package com.mediatek.rcs.provisioning.https;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;

import com.mediatek.rcs.provisioning.ProvisioningInfo;
import com.mediatek.rcs.utils.LauncherUtils;
import com.mediatek.rcs.utils.NetworkUtils;
import com.mediatek.rcs.utils.RcsSettings;
import com.mediatek.rcs.utils.logger.Logger;

import static com.mediatek.rcs.utils.StringUtils.PDUS;
import static com.mediatek.rcs.utils.StringUtils.UTF16;

/**
 * Handles the network initiated configuration request i.e provisioning push sms sent to port 37273.
 * IMSI-rcscfg is sent for First time configuration request and Private_User_Identity-rcscfg is sent
 * for reconfiguration request.
 */
public class ProvisioningPushSMSReceiver extends BroadcastReceiver {

    private static final Logger sLogger = Logger.getLogger(ProvisioningPushSMSReceiver.class
            .getName());
    private static final String INTENT_KEY_BOOT = "boot";
    private static final String INTENT_KEY_USER = "user";

    @Override
    public void onReceive(Context ctx, Intent intent) {
        final boolean logActivated = sLogger.isActivated();
        String action = intent.getAction();
        if (logActivated) {
            sLogger.debug("Configuration SMS receiver - Received broadcast: " + action);
        }
        if (!HttpsProvisioningUtils.ACTION_BINARY_SMS_RECEIVED.equals(action)) {
            return;
        }
        Bundle bundle = intent.getExtras();
        if (bundle == null) {
            if (logActivated) {
                sLogger.debug("Bundle is received with null");
            }
            return;
        }
        Object[] pdus = (Object[]) bundle.get(PDUS);
        if (pdus == null || pdus.length == 0) {
            if (logActivated) {
                sLogger.debug("Bundle contains no raw PDUs");
            }
            return;
        }
        final SmsMessage msg = SmsMessage.createFromPdu((byte[]) pdus[0]);
        final String smsData = new String(msg.getUserData(), UTF16);
        if (sLogger.isActivatedDebug()) {
            sLogger.debug("Binary SMS received...");
        }

        if (smsData.endsWith(HttpsProvisioningUtils.RESET_CONFIG_SUFFIX)) {
            //final ContentResolver resolver = ctx.getContentResolver();
            //final RcsSettings rcsSettings = RcsSettings.getInstance(localResolver);
            final RcsSettings rcsSettings = RcsSettings.getInstance();
            final TelephonyManager telephonyManager = (TelephonyManager) ctx
                    .getSystemService(Context.TELEPHONY_SERVICE);

            if (RcsSettings.ATT_SUPPORT) {
                resetConfigurationThenRestart(ctx);
                return;
            }

            if (smsData.contains(telephonyManager.getSubscriberId())) {
                //resetConfigurationThenRestart(ctx, resolver, localResolver, rcsSettings);
                resetConfigurationThenRestart(ctx);
            } else if (smsData.contains(rcsSettings.getUserProfileImsPrivateId())) {
                if (NetworkUtils.getNetworkAccessType(ctx) == NetworkUtils.NETWORK_ACCESS_WIFI) {
                    //tryUnRegister();
                    /*
                     * Only set version number to 0 in order to keep MSISDN and token.
                     * Reprovisioning is done silently: the user is not prompted to enter its
                     * MSISDN.
                     */
                    rcsSettings.setProvisioningVersion(ProvisioningInfo.Version.RESETED.toInt());
                    AcsService.reProvisioning(ctx);
                } else {
                    resetConfigurationThenRestart(ctx);
                }
            }
        }
    }

    private void resetConfigurationThenRestart(Context ctx) {
        final RcsSettings rcsSettings = RcsSettings.getInstance();
        int version = rcsSettings.getProvisioningVersion();
        LauncherUtils.resetRcsConfig(ctx, RcsSettings.getInstance());
        if (version > 0) {
            rcsSettings.setProvisioningVersion(version);
        }
        LauncherUtils.saveProvisioningValidity(ctx, 1);
        AcsService.reProvisioning(ctx);
        //LauncherUtils.launchRcsService(ctx, true, false, rcsSettings);
        // launchAcsStartService(ctx, true, false, RcsSettings.getInstance());
    }


    // private void tryUnRegister() {
    //     new Thread(new Runnable() {
    //         @Override
    //         public void run() {
    //             try {
    //                 Core core = Core.getInstance();
    //                 if (core != null) {
    //                     ImsNetworkInterface networkInterface = core.getImsModule()
    //                             .getCurrentNetworkInterface();
    //                     if (networkInterface.isRegistered()) {
    //                         networkInterface.unregister();
    //                     }
    //                 }
    //             } catch (NetworkException e) {
    //                 if (sLogger.isActivated()) {
    //                     sLogger.debug("Unable to unregister, error=" + e.getMessage());
    //                 }
    //             } catch (PayloadException e) {
    //                 sLogger.error("Unable to unregister!", e);
    //             } catch (RuntimeException e) {
    //                 sLogger.error("Unable to unregister!", e);
    //             }

    //         }
    //     }).start();
    // }

    /**
     * Launch the ACS service
     *
     * @param context the context
     * @param boot start RCS service upon boot
     * @param user start RCS service upon user action
     */
    // void launchAcsStartService(Context context, boolean boot, boolean user, RcsSettings rcsSettings) {
    //     if (sLogger.isActivated())
    //         sLogger.debug("Launch ACS service (boot=" + boot + ") (user=" + user + ")");
    //     Intent intent = new Intent(context, AcsService.class);
    //     intent.putExtra(INTENT_KEY_BOOT, boot);
    //     intent.putExtra(INTENT_KEY_USER, user);
    //     context.startService(intent);
    // }
}
