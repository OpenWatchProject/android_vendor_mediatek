/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.presence.utils;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.ims.ProvisioningManager;
import android.os.PatternMatcher;
import android.os.SystemProperties;
import com.android.ims.ImsException;
import com.android.ims.ImsConfig;
import com.android.ims.ImsConfigListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.mediatek.presence.utils.logger.Logger;

public class ImsConfigMonitorService {
    private final static String TAG = "ImsConfigMonitorService";
    private final static String EXTRA_PHONE_ID = "phone_id";
    private final static Logger logger = Logger.getLogger(TAG);
    private Context mContext;
    private boolean mIsLvcEnabled = false;
    private static ImsConfigMonitorService instance = null;

    private static HashMap<FeatureValueListener, Integer> mFeatureListenerMap =
            new HashMap<FeatureValueListener, Integer>();

    public interface FeatureValueListener {
        void onFeatureValueChanged(int slotId, int feature, int value);
    }

    public static ImsConfigMonitorService getInstance(Context context) {
        if (instance == null) {
            logger.info("ImsConfigMonitorService has been created");
            instance = new ImsConfigMonitorService(context);
        }
        return instance;
    }

    private BroadcastReceiver mFeatureValueReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }

            if (intent.getAction().equals(ImsConfig.ACTION_IMS_FEATURE_CHANGED)) {
                int feature = intent.getIntExtra(ImsConfig.EXTRA_CHANGED_ITEM, -1);
                int phoneId = intent.getIntExtra(EXTRA_PHONE_ID, -1);
                int value  = intent.getIntExtra(ImsConfig.EXTRA_NEW_VALUE, -1);
                logger.debug("onReceived IMS feature changed phoneId: "
                                + phoneId + ", feature: " + feature
                                + ", value: " + value);

                if (mFeatureListenerMap.containsValue(feature)) {
                    for(FeatureValueListener listener : mFeatureListenerMap.keySet()){
                        if(mFeatureListenerMap.get(listener).equals(feature)) {
                            listener.onFeatureValueChanged(phoneId, feature, value);
                            logger.debug("feature(" + feature + ") callback triggered, value = " + value);
                        }
                    }
                }
            }
        }

        private List<FeatureValueListener> getKeysFromValue(HashMap<FeatureValueListener, Integer> map, Integer value){
            List<FeatureValueListener> list = new ArrayList<>();
            logger.debug("getKeysFromValue, value = " + value);
            for(FeatureValueListener o : map.keySet()){
                if(map.get(o).equals(value)) {
                    list.add(o);
                    logger.debug("getKeysFromValue, mapped " + value);
                }
            }
            return list;
        }
    };

    private ImsConfigMonitorService(Context context) {
        mContext = context;
        setupImsConfigMonitoring();
    }

    public boolean isLvcEnabled() {
        mIsLvcEnabled = SystemProperties.getInt(
                "persist.vendor.mtk.vilte.enable", 0) == 1;
        logger.debug("isLvcEnabled, mIsLvcEnabled:" + mIsLvcEnabled);
        return mIsLvcEnabled;
    }

    public void registerFeatureValueListener(Integer featureKey, FeatureValueListener listener) {
        mFeatureListenerMap.put(listener, featureKey);
    }

    public void unRegisterFeatureValueListener(FeatureValueListener listener) {
        mFeatureListenerMap.remove(listener);
    }

    public void registerProvisionedValueCallback(int slotId, ProvisioningManager.Callback callback) {
        ImsManager imsMgr = ImsManager.getInstance(mContext, slotId);
        if (imsMgr == null) {
            logger.debug("registerProvisionedValueCallback fail, ImsManager = null");
            return;
        }
        try {
            imsMgr.getConfigInterface().addConfigCallback(callback);
        } catch (ImsException e) {
            logger.error("Unable to register callback for provisioning changes" + e);
        }
    }

    public void unRegisterProvisionedValueCallback(int slotId, ProvisioningManager.Callback callback) {
        ImsManager imsMgr = ImsManager.getInstance(mContext, slotId);
        if (imsMgr == null) {
            logger.debug("unRegisterProvisionedValueCallback fail, ImsManager = null");
            return;
        }
        try {
            imsMgr.getConfigInterface().removeConfigCallback(callback.getBinder());
        } catch (ImsException e) {
            logger.error("Unable to unRegister callback for provisioning changes" + e);
        }
    }

    private void setupImsConfigMonitoring() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ImsConfig.ACTION_IMS_FEATURE_CHANGED);
        mContext.registerReceiver(mFeatureValueReceiver, filter);
    }
}
