/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.server.wifi;

import android.content.Context;
import android.os.Message;
import android.util.Log;

import com.android.internal.util.State;
import com.android.server.wifi.ScanDetail;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader;
import com.mediatek.common.util.OperatorCustomizationFactoryLoader.OperatorFactoryInfo;
import com.mediatek.server.wifi.MtkWifiServiceAdapter.IMtkWifiService;
import com.mediatek.server.wifi.WifiOperatorFactoryBase;
import com.mediatek.server.wifi.WifiOperatorFactoryBase.IMtkWifiServiceExt;

import java.util.ArrayList;
import java.util.List;

public final class MtkWifiService implements IMtkWifiService {

    private static final String TAG = "MtkWifiService";

    private Context mContext;
    private IMtkWifiServiceExt mExt = null;

    private static final List<OperatorFactoryInfo> sFactoryInfoList = new ArrayList<>();

    static {
        sFactoryInfoList.add(
            new OperatorFactoryInfo(
            "Op01WifiService.apk",
            "com.mediatek.op.wifi.Op01WifiOperatorFactory",
            "com.mediatek.server.wifi.op01",
            "OP01"
        ));
    }

    private synchronized IMtkWifiServiceExt getOpExt() {
        if (mExt == null) {
            WifiOperatorFactoryBase factory =
                    (WifiOperatorFactoryBase) OperatorCustomizationFactoryLoader
                            .loadFactory(mContext, sFactoryInfoList);
            if (factory == null) {
                factory = new WifiOperatorFactoryBase();
            }
            Log.i(TAG, "Factory is : " + factory.getClass());
            mExt = factory.createWifiFwkExt(mContext);
            mExt.init();
        }
        return mExt;
    }

    public MtkWifiService(Context context) {
        Log.i(TAG, "[MtkWifiService] " + context);
        mContext = context;
    }

    // NOTE: this method should be invoked in an looper thread
    public void initialize() {
        Log.i(TAG, "[initialize]");
        getOpExt();
    }

    @Override
    public void handleScanResults(List<ScanDetail> full, List<ScanDetail> unsaved) {
        mExt.handleScanResults(full, unsaved);
    }

    @Override
    public void updateRSSI(Integer newRssi, int ipAddr, int lastNetworkId) {
        mExt.updateRSSI(newRssi, ipAddr, lastNetworkId);
    }

    @Override
    public boolean preProcessMessage(State state, Message msg) {
        return mExt.preProcessMessage(state, msg);
    }

    @Override
    public boolean postProcessMessage(State state, Message msg, Object... args) {
        return mExt.postProcessMessage(state, msg, args);
    }

    @Override
    public boolean needCustomEvaluator() {
        return mExt.needCustomEvaluator();
    }

    @Override
    public void triggerNetworkEvaluatorCallBack() {
        mExt.triggerNetworkEvaluatorCallBack();
    }
}
