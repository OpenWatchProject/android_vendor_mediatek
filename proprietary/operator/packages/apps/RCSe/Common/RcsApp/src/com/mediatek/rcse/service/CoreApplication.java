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

package com.mediatek.rcse.service;

import android.app.Application;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.pm.PackageManager;
import android.preference.PreferenceManager;

import com.mediatek.rcse.activities.ChatMainActivity;
import com.mediatek.rcse.activities.PluginProxyActivity;
import com.mediatek.rcse.activities.RcsPermissionCheckActivity;
import com.mediatek.rcse.activities.SettingsFragment;
import com.mediatek.rcse.activities.widgets.ContactsListManager;
import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.emoticons.EmoticonsModelImpl;
import com.mediatek.rcse.service.MediatekFactory;
import com.gsma.services.rcs.JoynServiceConfiguration;


/**
 * This application class will activate the ApiService when this process created.
 */
public class CoreApplication extends Application {
    public static final String TAG = "CoreApplication";
    public static final String APP_NAME = "com.mediatek.rcs";
    public static final String LAUNCH_SERVICE = "com.mediatek.intent.rcs.stack.LaunchService";
    public static final String STOP_SERVICE = "com.mediatek.intent.rcs.stack.StopService";

    @Override
    public void onCreate() {
        com.mediatek.common.jpe.a aa = new com.mediatek.common.jpe.a();
        aa.a();
        super.onCreate();
        MediatekFactory.setApplicationContext(getApplicationContext());

        if (!isRcsSupported()) {
            android.os.Process.killProcess(android.os.Process.myPid());
            return;
        }

        Logger.initialize(getApplicationContext());
        Logger.v(TAG, "onCreate() entry");
        if (!ApiManager.initialize(getApplicationContext())) {
            Logger.e(TAG, "onCreate() ApiManager initialization failed!");
        }
        //startService(new Intent(this, ApiService.class));
        // Init UnreadMessageManager
        UnreadMessageManager.getInstance();
        EmoticonsModelImpl.init(getApplicationContext());
        controlRcseComponent();
        SharedPreferences prefer = PreferenceManager
              .getDefaultSharedPreferences(this);
        boolean alwaysDeny = prefer.getBoolean("alwaysdenypermission", false);
        Logger.v(TAG, "onCreate()alwaysdeny:" + alwaysDeny);
        if (!RcsOsUtils.hasRequiredPermissions()) {
            Logger.v(TAG, "critical permission is not there");
            if(alwaysDeny == false){
                SharedPreferences sPrefer = PreferenceManager
                        .getDefaultSharedPreferences(getApplicationContext());
                Editor alwaysdeny = sPrefer.edit();
                alwaysdeny.putBoolean("denypermission", true);
                alwaysdeny.commit();
                launchPermissionCheckActivity(getApplicationContext());
            }
        } else {
            Logger.v(TAG, "critical permission is there");
            if(!JoynServiceConfiguration.isServicePermission(MediatekFactory.getApplicationContext())) {
                Logger.v(TAG, "critical permission is there, set permission state true");
                JoynServiceConfiguration.setServicePermissionState(true, MediatekFactory.getApplicationContext());
            }
            ContactsListManager.initialize(getApplicationContext());

        }
        Logger.v(TAG, "onCreate() exit");
    }

    public void launchPermissionCheckActivity(Context context) {
        Intent intent = new Intent(context, RcsPermissionCheckActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }
    /**
     * control the RCS component according to the configuration and active
     * status.
     */
    private void controlRcseComponent() {
        Context context = getApplicationContext();
        if (Logger.getIsIntegrationMode()) {
            Logger.d(TAG, "controlRcseComponent() is integration mode ");
            setIntegrationModeComponent(context);
        } else {
            setComponentStatus(context, true);
        }
    }

    /**
     * Set component class enable or disable in packageManager.
     *
     * @param clazz            the class need to be set.
     * @param enabled            whether to enable or disable the clazz.
     * @param context the context
     */
    private static void setComponentEnabled(Class<?> clazz, boolean enabled,
            Context context) {
        Logger.d(TAG, "setComponentEnabled() enable is " + enabled
                + " and the context is " + context);
        final ComponentName c = new ComponentName(context, clazz.getName());
        context.getPackageManager().setComponentEnabledSetting(
                c,
                enabled ? PackageManager.COMPONENT_ENABLED_STATE_ENABLED
                        : PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
                PackageManager.DONT_KILL_APP);
    }

    /**
     * Set component status in integration mode.
     *
     * @param context            The context.
     */
    public static void setIntegrationModeComponent(Context context) {
        Logger.d(TAG, "setIntegrationModeComponent() entry");
        setComponentEnabled(ChatMainActivity.class, false, context);
        setComponentEnabled(PluginProxyActivity.class, true, context);
        Logger.d(TAG, "setIntegrationModeComponent() exit");
    }

    /**
     * Set component status in packageManager.
     *
     * @param context
     *            The context
     * @param status
     *            The component status
     */
    public static void setComponentStatus(Context context, boolean status) {
        Logger.d(TAG, "setComponentStatus() entry with the context is "
                + context + " and the status is " + status);
        setComponentEnabled(ChatMainActivity.class, status, context);
        setComponentEnabled(PluginProxyActivity.class, status, context);
        Logger.d(TAG, "setComponentStatus() exit");
    }

    @Override
    public void onLowMemory() {
        Logger.d(TAG, "onLowMemory() entry");
        super.onLowMemory();
    }

    private boolean isRcsSupported() {
        boolean mtkRcsSupport =
            (android.os.SystemProperties.getInt("persist.vendor.mtk_rcs_support", 0) == 1);
        Logger.d(TAG, "isMtkRcsSupport:" + mtkRcsSupport);
        return mtkRcsSupport;
    }

}
