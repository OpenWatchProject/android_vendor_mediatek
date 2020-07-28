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

package com.mediatek.op.wifi;

import android.net.wifi.ScanResult;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiScanner;
import android.os.Message;

import com.android.internal.util.IState;
import com.android.internal.util.Protocol;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
import com.android.server.wifi.ClientModeImpl;
import com.android.server.wifi.WifiInjector;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.List;

import static android.net.wifi.WifiConfiguration.INVALID_NETWORK_ID;

public class ClientModeAdapter {

    // NOTE: keep these values sync with ClientModeImpl
    private static final int BASE = Protocol.BASE_WIFI;
    public static final int CMD_REMOVE_NETWORK = BASE + 53;
    public static final int CMD_ENABLE_NETWORK = BASE + 54;

    public static final int SUCCESS = 1;

    private final ClientModeImpl mClientModeImpl;
    private final Class<?> mClientModeImplClass;

    public ClientModeAdapter(ClientModeImpl wsm) {
        mClientModeImpl = wsm;
        mClientModeImplClass = wsm.getClass();
    }

    public void replyToMessage(Message msg, int what) {
        try {
            Method method = mClientModeImplClass.getDeclaredMethod(
                "replyToMessage",
                Message.class,
                Integer.class);
            method.setAccessible(true);
            method.invoke(mClientModeImpl, msg, what);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void replyToMessage(Message msg, int what, int arg1) {
        try {
            Method method = mClientModeImplClass.getDeclaredMethod(
                "replyToMessage",
                Message.class,
                Integer.class,
                Integer.class);
            method.setAccessible(true);
            method.invoke(mClientModeImpl, msg, what, arg1);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void replyToMessage(Message msg, int what, Object obj) {
        try {
            Method method = mClientModeImplClass.getDeclaredMethod(
                "replyToMessage",
                Message.class,
                Integer.class,
                Object.class);
            method.setAccessible(true);
            method.invoke(mClientModeImpl, msg, what, obj);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public WifiInfo getWifiInfo() {
        return mClientModeImpl.getWifiInfo();
    }

    public int getLastNetworkId() {
        int id = INVALID_NETWORK_ID;
        try {
            Field field = mClientModeImplClass.getDeclaredField("mLastNetworkId");
            field.setAccessible(true);
            id = (Integer) field.get(mClientModeImpl);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            return id;
        }
    }

    public IState getCurrentState() {
        State state = null;
        try {
            Method method = StateMachine.class.getDeclaredMethod("getCurrentState");
            method.setAccessible(true);
            state = (State) method.invoke(mClientModeImpl);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            return state;
        }
    }

    public void startScan() {
        WifiInjector injector = WifiInjector.getInstance();
        WifiScanner.ScanSettings settings = new WifiScanner.ScanSettings();
        settings.type = WifiScanner.TYPE_HIGH_ACCURACY; // always do high accuracy scans.
        settings.band = WifiScanner.WIFI_BAND_BOTH_WITH_DFS;
        settings.reportEvents = WifiScanner.REPORT_EVENT_FULL_SCAN_RESULT
                            | WifiScanner.REPORT_EVENT_AFTER_EACH_SCAN;
        settings.numBssidsPerScan = 0;

        List<WifiScanner.ScanSettings.HiddenNetwork> hiddenNetworkList =
                injector.getWifiConfigManager().retrieveHiddenNetworkList();
        settings.hiddenNetworks = hiddenNetworkList.toArray(new WifiScanner.ScanSettings.
                HiddenNetwork[hiddenNetworkList.size()]);

        WifiScanner.ScanListener scanListener = new WifiScanner.ScanListener() {
            @Override
            public void onSuccess() {
            }
            @Override
            public void onFailure(int reason, String description) {
            }
            @Override
            public void onResults(WifiScanner.ScanData[] results) {
            }
            @Override
            public void onFullResult(ScanResult fullScanResult) {
            }
            @Override
            public void onPeriodChanged(int periodInMs) {
            }
        };
        injector.getWifiScanner().startScan(settings, scanListener);
    }

    public void sendMessage(int what) {
        mClientModeImpl.sendMessage(what);
    }

    public void sendMessage(Message message) {
        mClientModeImpl.sendMessage(message);
    }

    public Message obtainMessage(int what, int arg1) {
        return mClientModeImpl.obtainMessage(what, arg1);
    }

    public Message obtainMessage(int what, int arg1, int arg2) {
        return mClientModeImpl.obtainMessage(what, arg1, arg2);
    }

    public String getInterfaceName() {
        String interfaceName = null;
        try {
            Field field = mClientModeImplClass.getDeclaredField("mInterfaceName");
            field.setAccessible(true);
            interfaceName = (String) field.get(mClientModeImpl);
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            return interfaceName;
        }
    }

    public int syncGetWifiState() {
        return mClientModeImpl.syncGetWifiState();
    }
}
