/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.op236.ims;

import android.content.Context;
import android.os.Build;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.ims.internal.ext.ImsManagerExt;
import com.mediatek.ims.MtkImsConstants;

import java.util.Arrays;
import java.util.List;

/**
 * Plugin implementation for Imsmanager.
 */

public class Op236ImsManagerExt extends ImsManagerExt {

    private static final String TAG = "Op236ImsManagerExt";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private Context mContext;

    /**
     * Constructor of plugin.
     * @param context context
     */
    public Op236ImsManagerExt(Context context) {
        super();
        mContext = context;
    }

    @Override
    /**
      * An API to customize platform enabled status.
      * @param context The context for retrive plug-in.
      * @param feature The IMS feature defined in ImsConfig.FeatureConstants.
      * @param phoneId PhoneId for which feature to be enabled.
      * @return return enabled status.
      */
    public boolean isFeatureEnabledByPlatform(Context context, int feature, int phoneId) {
        if (ENG) Log.d(TAG, "feature:" + feature + ", phoneId:" + phoneId);
        if (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE) {
            return isVdpProvisionedOnDevice(phoneId);
        } else {
            return super.isFeatureEnabledByPlatform(context, feature, phoneId);
        }
    }

    private boolean isVdpProvisionedOnDevice(int phoneId) {
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(mContext, phoneId);
        int value = 0;
        boolean result = false;
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
                if (imsConfig != null) {
                    value = imsConfig.getProvisionedValue(
                        MtkImsConstants.ConfigConstants.VOICE_DOMAIN_PREFERENCE);
                    if (ENG || TELDBG) Log.d(TAG, "Vdp provisioned value = " + value);
                    if (value == 3) {
                        result = true;
                    }
                }
            } catch (ImsException e) {
                Log.e(TAG, "Vdp not updated, ImsConfig null");
                e.printStackTrace();
            } catch (RuntimeException e) {
                Log.e(TAG, "ImsConfig not ready");
                e.printStackTrace();
            }
        } else {
            Log.e(TAG, "Vdp not updated, ImsManager null");
        }

        if (ENG) Log.d(TAG, "isVdpProvisionedOnDevice returns " + result);
        return result;
    }
}