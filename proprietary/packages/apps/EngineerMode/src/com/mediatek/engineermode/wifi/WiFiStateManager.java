/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.engineermode.wifi;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.os.SystemClock;

import com.mediatek.engineermode.Elog;

/**
 * Controller for Wifi state.
 *
 */
public class WiFiStateManager {

    private static final String TAG = "WifiStateManager";
    private final WifiManager mWifiManager;
    private int mChipID = 0x0;
    public static final int ENABLE_WIFI_FAIL = -1;
    public static final int INVALID_CHIP_ID = -2;
    public static final int SET_TEST_MODE_FAIL = -3;
    public static final int CHIP_READY = -4;
    protected static final int CHIP_ID_6620 = 0x6620;
    protected static final int CHIP_ID_5921 = 0x5920;
    private static final long DEFAULT_WAIT_TIME = 100;
    private static final long TIME_1000 = 1000;

    /**
     * Constructor function.
     *
     * @param activityContext
     *            Context of the activity
     */
    public WiFiStateManager(Context activityContext) {
        mWifiManager = (WifiManager) activityContext
                .getSystemService(Context.WIFI_SERVICE);
    }

    /**
     * Check WiFi chip status.
     *
     * @param activityContext
     *            Context of activity
     * @return WiFi chip status
     */
    public int checkState(Context activityContext) {
        if (mWifiManager != null) {
            if (mWifiManager.getWifiState() != WifiManager.WIFI_STATE_ENABLED) {
                if (mWifiManager.setWifiEnabled(true)) {

                    while (mWifiManager.getWifiState() != WifiManager.WIFI_STATE_ENABLED) {
                        SystemClock.sleep(DEFAULT_WAIT_TIME);
                    }
                } else {
                    Elog.e(TAG, "enable wifi power failed");
                    return ENABLE_WIFI_FAIL;
                }
            }
            Elog.i(TAG, "Wifi state: " + mWifiManager.getWifiState());
            if (EMWifi.sIsInitialed) {
                return CHIP_READY;
            } else {
                mChipID = EMWifi.initial();
                if (mChipID == CHIP_ID_6620 || mChipID == CHIP_ID_5921) {
                    Elog.d(TAG, "Initialize succeed");
                    long result = -1;
                    result = EMWifi.setTestMode();
                    Elog.i(TAG, "result:" + result);
                    if (result == 0) {
                        synchronized (this) {
                            EMWifi.sIsInitialed = true;
                        }
                    } else {
                        return SET_TEST_MODE_FAIL;
                    }
                } else {
                    return INVALID_CHIP_ID;
                }
            }
        }
        return mChipID;
    }

    /**
     * Uninit WiFi EM test.
     */
    public void uninitWifiTest() {
        if (EMWifi.sIsInitialed) {
            EMWifi.setNormalMode();
            EMWifi.unInitial();
            EMWifi.sIsInitialed = false;

            if (mWifiManager != null) {
                mWifiManager.setWifiEnabled(false);
                for (int k = 0; k < 15; k++) {
                    if (WifiManager.WIFI_STATE_DISABLED != mWifiManager.getWifiState()) {
                        SystemClock.sleep(TIME_1000);
                    } else {
                        Elog.i(TAG, "uninitWifiTest succeed");
                        return;
                    }
                }

            }
        }

    }

    /**
     * Disable WiFi.
     */
    public void disableWiFi() {
        if (mWifiManager != null) {
            if (!mWifiManager.setWifiEnabled(false)) {
                Elog.w("@M_" + TAG, "disable wifi power failed");
            }
        }
    }
}
