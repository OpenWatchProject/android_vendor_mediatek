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

package com.mediatek.settings.op01;

import android.content.Context;
import android.util.Log;

import com.mediatek.settings.ext.IDataUsageSummaryExt;
import com.mediatek.settings.ext.IDevExt;
import com.mediatek.settings.ext.IDeviceInfoSettingsExt;
import com.mediatek.settings.ext.IMobileNetworkSettingsExt;
import com.mediatek.settings.ext.ISettingsMiscExt;
import com.mediatek.settings.ext.ISimManagementExt;
import com.mediatek.settings.ext.IWifiExt;
import com.mediatek.settings.ext.IWifiSettingsExt;
import com.mediatek.settings.ext.OpSettingsCustomizationFactoryBase;

public class Op01SettingsCustomizationFactory extends
        OpSettingsCustomizationFactoryBase {

    private static String TAG = "Op01SettingsCustomizationFactory";

    private Context mContext;

    public Op01SettingsCustomizationFactory(Context context) {
        super(context);
        Log.d(TAG, "Op01SettingsCustomizationFactory, context=" + context);
        mContext = context;
    }

    @Override
    public IDataUsageSummaryExt makeDataUsageSummaryExt() {
        Log.d(TAG, "makeDataUsageSummaryExt, context=" + mContext);
        return new Op01DataUsageSummaryExt(mContext);
    }

    @Override
    public IDeviceInfoSettingsExt makeDeviceInfoSettingsExt() {
        Log.d(TAG, "makeDeviceInfoSettingsExt, context=" + mContext);
        return new Op01DeviceInfoSettingsExt(mContext);
    }

    @Override
    public ISettingsMiscExt makeSettingsMiscExt(Context context) {
        Log.d(TAG, "makeSettingsMiscExt, context=" + context);
        return new Op01SettingsMiscExt(mContext);
    }

    @Override
    public ISimManagementExt makeSimManagementExt() {
        Log.d(TAG, "makeSimManagementExt, context=" + mContext);
        return new Op01SimManagementExt(mContext);
    }

    @Override
    public IWifiExt makeWifiExt(Context context) {
        Log.d(TAG, "makeWifiExt, context=" + context);
        return new Op01WifiExt(mContext);
    }

    @Override
    public IWifiSettingsExt makeWifiSettingsExt() {
        Log.d(TAG, "makeWifiSettingsExt, context=" + mContext);
        return new Op01WifiSettingsExt(mContext);
    }

    @Override
    public IDevExt makeDevExt(Context context) {
        return new Op01DevExt(context);
    }

    @Override
    public IMobileNetworkSettingsExt makeMobileNetworkSettingsExt(Context context) {
        Log.d(TAG, "IMobileNetworkSettingsExt, context=" + mContext);
        return new Op01MobileNetworkSettingExt(mContext);
    }
}
