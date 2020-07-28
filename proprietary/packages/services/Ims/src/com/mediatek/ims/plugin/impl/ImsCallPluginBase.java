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

package com.mediatek.ims.plugin.impl;

import android.util.Log;
import android.content.Context;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import com.android.ims.ImsException;

import com.mediatek.ims.plugin.ImsCallPlugin;

/**
 * Default Implementation of interface ImsCallPlugin
 */
public class ImsCallPluginBase implements ImsCallPlugin {

    private static final String TAG = "ImsConfigPluginBase";

    private static final String PROPERTY_CAPABILITY_SWITCH  = "persist.vendor.radio.simswitch";
    private static final String MULTI_IMS_SUPPORT           = "persist.vendor.mims_support";

    private Context                 mContext;

    public ImsCallPluginBase(Context context) {
        mContext = context;
    }

    @Override
    public boolean isSupportMims() {
        return (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) > 1);
    }

    @Override
    public int setImsFwkRequest(int request) {
        return request;
    }

    @Override
    public int getRealRequest(int request) {
        return request;
    }

    @Override
    public boolean isImsFwkRequest(int request) {
        return false;
    }

    @Override
    public int getUpgradeCancelFlag() {
        return 0x00010000;
    }

    @Override
    public int getUpgradeCancelTimeoutFlag() {
        return 0;
    }

    @Override
    public int getMainCapabilityPhoneId() {
        int phoneId = 0;
        phoneId = SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH, 1) - 1;
        Log.d(TAG, "getMainCapabilityPhoneId " + phoneId);
        return phoneId;
    }

    @Override
    public boolean isCapabilitySwitching() {
        return false;
    }

    @Override
    public int getSimCardState(int slotId) {
        return TelephonyManager.getDefault().getSimCardState();
    }

    @Override
    public int getSimApplicationState(int slotId) {
        return TelephonyManager.getDefault().getSimApplicationState();
    }
}