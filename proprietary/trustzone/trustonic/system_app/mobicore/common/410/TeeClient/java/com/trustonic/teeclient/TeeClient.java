/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
package com.trustonic.teeclient;
import com.trustonic.teeclient.TeeClientJni;
import com.trustonic.teeclient.TeeException;
import com.trustonic.teeclient.kinibichecker.TEEError;
import com.trustonic.teeclient.kinibichecker.KinibiChecker;

import java.util.EnumSet;
import java.util.List;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.util.Log;

public class TeeClient  {
    // Private constants
    private static final String LOG_TAG = TeeClient.class.getSimpleName();
    private static final String TEE_PROXY_SERVICE_PACKAGE = "com.trustonic.teeproxyservice";

    // Public constants
    public static final int MC_DEVICE_ID_DEFAULT = TeeClientJni.MC_DEVICE_ID_DEFAULT;

    // Class variables
    private Context application_context_ = null;
    private Activity client_activity_ = null;
    private TeeClientJni jni_wrapper_ = null;
    private KinibiChecker kc_ = null;

    public TeeClient(Context applicationContext) {
        application_context_ = applicationContext;
        jni_wrapper_ = new TeeClientJni(applicationContext);
        kc_ = new KinibiChecker(applicationContext);
    }

    // libMcClient
    public void mcOpenDevice(int deviceId) throws TeeException {
        jni_wrapper_.mcOpenDevice(deviceId);
    }

    public void mcCloseDevice(int deviceId) throws TeeException {
        jni_wrapper_.mcCloseDevice(deviceId);
    }

    // TeeProxyService
    public void launchPlayStoreOnTeeProxyService(Activity activity) {
        client_activity_ = activity;
        launchPlayStoreOnApp(client_activity_, TEE_PROXY_SERVICE_PACKAGE);
    }

    public boolean isTeeProxyServiceInstalled() {
        return isAppInstalled(TEE_PROXY_SERVICE_PACKAGE);
    }

    public EnumSet<TEEError> getDeviceErrata() {
        return kc_.getDeviceErrata();
    }

    public boolean isTuiAvailable() {
        return kc_.isTuiAvailable();
    }


    // Helper functions
    private void launchPlayStoreOnApp(Activity activity, String packageName) {
        try {
            Intent intent = new Intent(
                Intent.ACTION_VIEW,
                Uri.parse("market://details?id=" + packageName)
            );
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            activity.startActivity(intent);
        } catch (android.content.ActivityNotFoundException e) {
            Intent intent = new Intent(
                Intent.ACTION_VIEW,
                Uri.parse("https://play.google.com/store/apps/details?id=" + packageName)
            );
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            activity.startActivity(intent);
        }
    }

    private boolean isAppInstalled(String packageName) {
        PackageManager pm = application_context_.getPackageManager();
        if (pm == null) {
            return false;
        }
        List<PackageInfo> apps = pm.getInstalledPackages(0);
        for (int i=0; i<apps.size(); i++) {
            PackageInfo p = apps.get(i);
            if (p.packageName.equals(packageName)) {
                return true;
            }
        }
        return false;
    }

};
