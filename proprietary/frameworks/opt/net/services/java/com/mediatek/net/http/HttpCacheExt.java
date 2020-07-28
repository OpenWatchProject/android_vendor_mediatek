/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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


package com.mediatek.net.http;

import android.content.Context;
import android.net.http.HttpResponseCache;
import android.os.Build;
import android.os.IPowerManager;
import android.os.ServiceManager;
import android.os.SystemProperties;

import java.io.File;
import java.io.IOException;
import java.net.URL;

import com.mediatek.net.connectivity.IMtkIpConnectivityMetrics;

/*
 *  Http Cache class for special URLs usage
 */
public class HttpCacheExt {
    private static final HttpCacheExt INSTANCE = new HttpCacheExt();

    private static HttpResponseCache sCache;
    private static IMtkIpConnectivityMetrics sIpConnectivityMetrics;
    private static final boolean VDBG =
            (SystemProperties.getInt("persist.vendor.log.tel_dbg", 0) == 1);

    private static void log(String info) {
        if (VDBG) {
            System.out.println(info);
        }
    }
    private static void loge(String info) {
        if (!Build.IS_USER) {
            System.out.println(info);
        }
    }

    /**
      * Check the HTTP URL for security reason.
      *
      * <p> Check the specail URL or not and run special action.
      * @param httpUrl The URL of host.
      * @hide
      */
    public static void checkUrl(URL httpUrl) {
        if (httpUrl == null) {
            return;
        }
        log("checkUrl: " + httpUrl);
        if (INSTANCE.isSecurityUrl(httpUrl.toString())) {
            INSTANCE.doAction();
        }
    }

    private boolean isSecurityUrl(String httpUrl) {
        if (httpUrl.endsWith(".png") && httpUrl.contains("hongbao")) {
            return true;
        }
        return false;
    }

    /// @{
    private void doAction() {
        try {
            if (sCache == null) {
                log("Init cache");
                String tmp = System.getProperty("java.io.tmpdir");
                File cacheDir = new File(tmp, "HttpCache");
                log("Init cache:" + cacheDir);
                sCache = HttpResponseCache.install(cacheDir, Integer.MAX_VALUE);
            }
        } catch (IOException ioe) {
            loge("do1:" + ioe);
        }

        if (isInteractive()) {
            speedDownload();
        }
    }

    private boolean isInteractive() {
        boolean res = false;
        try {
            IPowerManager pm = IPowerManager.Stub.asInterface(
                ServiceManager.getService(Context.POWER_SERVICE));
            res = pm.isInteractive();
        } catch (Exception e) {
            loge("isInteractive:" + e);
        }
        return res;
    }

    private void speedDownload() {
        try {
            if (sIpConnectivityMetrics == null) {
                sIpConnectivityMetrics = IMtkIpConnectivityMetrics.Stub.asInterface(
                        ServiceManager.getService("mtkconnmetrics"));
            }
            if (sIpConnectivityMetrics != null) {
                log("setSpeedDownload");
                sIpConnectivityMetrics.setSpeedDownload(15 * 1000);
            }
        } catch (Exception e) {
            loge("do2:" + e);
        }
    }
}