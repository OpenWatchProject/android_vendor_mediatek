/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.wfo.impl;

import android.os.RemoteException;
import android.util.Log;

import vendor.mediatek.hardware.wfo.V1_0.IWifiOffloadCallback;

public class WifiOffloadHIDLCallback extends IWifiOffloadCallback.Stub {

    static final String TAG = "WifiOffloadHIDLCallback";
    private WifiOffloadService mWifiOffloadService = null;

    WifiOffloadHIDLCallback(WifiOffloadService wifiOffloadService) {
        mWifiOffloadService = wifiOffloadService;
    }

    // Methods from ::vendor::mediatek::hardware::wfo::V1_0::IWifiOffloadCallback follow.
    @Override
    public void onRequestLocationInfo(int simIdx) throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onRequestLocationInfo(simIdx);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onRssiMonitorRequest(int simId, int size,
        java.util.ArrayList<Integer> rssiThresholds) throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onRssiMonitorRequest(simId, size,
                rssiThresholds.stream().mapToInt(Integer::intValue).toArray());
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onRequestImsSwitch(int simIdx, boolean isImsOn) throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onRequestImsSwitch(simIdx, isImsOn);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onRequestSetWifiDisabled(int pdnCount) throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onRequestSetWifiDisabled(pdnCount);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onDisconnectCauseNotify(int simIdx, int lastErr, int lastSubErr)
        throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onDisconnectCauseNotify(simIdx, lastErr, lastSubErr);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onMalReset() throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onMalReset();
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onPdnRanTypeChanged(int simIdx, int interfaceId, int ranType)
        throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onPdnRanTypeChanged(simIdx, interfaceId, ranType);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onPdnStateChanged(int simIdx, boolean active) throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onPdnStateChanged(simIdx, active);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onRoveOut(int simIdx, boolean roveOut, int rssi) throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onRoveOut(simIdx, roveOut, rssi);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

    @Override
    public void onHandover(int simIdx, int stage, int ratType) throws RemoteException {
        if (mWifiOffloadService != null) {
            mWifiOffloadService.onHandover(simIdx, stage, ratType);
        } else {
            Log.d(TAG, "mWifiOffloadService is null");
        }
    }

}
