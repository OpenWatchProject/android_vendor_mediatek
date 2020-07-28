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

package com.mediatek.rcse.activities;

import com.gsma.services.rcs.JoynServiceException;
import com.gsma.services.rcs.JoynServiceRegistrationListener;

import android.app.ActionBar;
import android.app.Activity;
import android.os.Bundle;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;

import com.mediatek.rcs.R;
import com.mediatek.rcse.service.MediatekFactory;
import com.orangelabs.rcs.provisioning.https.HttpsProvisioningManager;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

/*
 * Displays RCS-e settings UI. If you want to start this Activity, please use an intent with
 * action "com.mediatek.rcse.RcseSettings". Sample code:
 *       Intent intent = new Intent();
 *       intent.setAction("com.mediatek.rcse.RCSE_SETTINGS");
 *       startActivity(intent);
 */
public class RcseSystemSettingsActivity extends Activity {

    /**
     * The logger
     */
    /*private static com.orangelabs.rcs.utils.logger.Logger logger = com.orangelabs.rcs.utils.logger.Logger
            .getLogger("RcseSystemSettingsActivity");*/

    private static final String TAG = "RcseSystemSettingsActivity";
    private static BroadcastReceiver rcsProvisioningListner = null;
    // private static BroadcastReceiver rcsRegistrationListner = null;
    public static RegistrationListener regListener = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        createViewsAndFragments(savedInstanceState);

        if (Logger.isActivated()) {
            // register provisioning listner
            registerProvisioningListner();
            // registerRegistrationListner();
            regListener = new RegistrationListener();
            /*
             * try {
             * ApiManager.getInstance().getChatApi().addServiceRegistrationListener
             * (regListener); } catch (JoynServiceException e) {
             *
             * e.printStackTrace(); }
             */
        }

    }

    @Override
    protected void onStop() {
        Logger.v(TAG, "onStop entry");
        super.onStop();
        // this.finish();
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onDestroy();

        // unregister
        if (Logger.isActivated()) {
            // unregister provisioning listner

            if (rcsProvisioningListner != null) {
                getApplicationContext().unregisterReceiver(
                        rcsProvisioningListner);
                rcsProvisioningListner = null;
            }
            /*
             * try { ApiManager.getInstance().getChatApi().
             * removeServiceRegistrationListener(regListener); } catch
             * (JoynServiceException e) { e.printStackTrace(); }
             */
            /*
             * if(rcsRegistrationListner!=null){
             * getApplicationContext().unregisterReceiver
             * (rcsRegistrationListner); rcsRegistrationListner = null; }
             */
        }

    }

    private void createViewsAndFragments(Bundle savedState) {
        Logger.v(TAG, "createViewsAndFragments entry");
        setContentView(R.layout.rcse_system_setting);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_HOME
                | ActionBar.DISPLAY_SHOW_TITLE);
        actionBar.setIcon(R.drawable.rcs_icon);
        Logger.v(TAG, "createViewsAndFragments exit");
    }

    private void registerProvisioningListner() {
        // Register network state listener

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(HttpsProvisioningManager.PROVISIONING_INTENT);

        if (null == rcsProvisioningListner) {
            rcsProvisioningListner = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, final Intent intent) {

                    boolean status = false;
                    String reason = "";
                    status = intent.getBooleanExtra("status", false);
                    reason = intent.getStringExtra("reason");
                    Logger.d(TAG, "provsioning listner : status  " + status
                            + "; reason: " + reason);

                    if (!status) {
                        Toast.makeText(context,
                                "provisioning faile: reason :-  " + reason,
                                Toast.LENGTH_LONG).show();
                    } else {
                        Toast.makeText(context, "provisioning success :-  ",
                                Toast.LENGTH_LONG).show();
                    }

                    // getApplicationContext().unregisterReceiver(rcsProvisioningListner);
                    // rcsProvisioningListner = null;

                }
            };
            MediatekFactory.getApplicationContext().registerReceiver(
                    rcsProvisioningListner, intentFilter);
        }
    }

    private class RegistrationListener extends JoynServiceRegistrationListener {

        @Override
        public void onServiceRegistered() {
            Logger.d(TAG, "onServiceRegistered entry");
            Toast.makeText(getApplicationContext(),
                    "REGISTRATION success :-  ", Toast.LENGTH_LONG).show();

        }

        @Override
        public void onServiceUnregistered() {
            Toast.makeText(getApplicationContext(),
                    "REGISTRATION FAILED :- reason :  ", Toast.LENGTH_LONG)
                    .show();
        }

    }

    /*
     * private void registerRegistrationListner(){ // Register network state
     * listener
     *
     *
     * IntentFilter intentFilter = new IntentFilter();
     * intentFilter.addAction(ClientApiIntents.SERVICE_REGISTRATION);
     *
     * if (null == rcsRegistrationListner) { rcsRegistrationListner = new
     * BroadcastReceiver() {
     *
     * @Override public void onReceive(Context context, final Intent intent) {
     *
     * boolean status = true; String reason = "";
     *
     * status = intent.getBooleanExtra("status", false); reason =
     * intent.getStringExtra("reason");
     *
     * if(!status){ Toast.makeText(context, "REGISTRATION FAILED :- reason :  "+
     * reason, Toast.LENGTH_LONG).show(); }else{ Toast.makeText(context,
     * "REGISTRATION success :-  ", Toast.LENGTH_LONG).show(); } } };
     * MediatekFactory
     * .getApplicationContext().registerReceiver(rcsRegistrationListner,
     * intentFilter); } }
     */
}
