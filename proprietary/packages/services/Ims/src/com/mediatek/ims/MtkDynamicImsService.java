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

package com.mediatek.ims;

import android.annotation.SystemApi;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsService;
import android.telephony.ims.aidl.IImsConfig;
import android.telephony.ims.aidl.IImsMmTelFeature;
import android.telephony.ims.aidl.IImsRcsFeature;
import android.telephony.ims.aidl.IImsRegistration;
import android.telephony.ims.aidl.IImsServiceController;
import android.telephony.ims.aidl.IImsServiceControllerListener;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.telephony.ims.stub.ImsFeatureConfiguration;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.Rlog;
import android.util.SparseArray;

import com.android.ims.internal.IImsFeatureStatusCallback;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.PhoneConstants;

import static android.Manifest.permission.MODIFY_PHONE_STATE;

import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import com.mediatek.ims.feature.MtkMmTelFeature;
import com.mediatek.ims.MtkImsRegistrationImpl;
import com.mediatek.ims.config.MtkImsConfigImpl;

/**
 * Main ImsService implementation, which binds via the Telephony ImsResolver. Services that extend
 * ImsService must register the service in their AndroidManifest to be detected by the framework.
 * First, the application must declare that they use the "android.permission.BIND_IMS_SERVICE"
 * permission. Then, the ImsService definition in the manifest must follow the following format:
 *
 * ...
 * <service android:name=".EgImsService"
 *     android:permission="android.permission.BIND_IMS_SERVICE" >
 *     ...
 *     <intent-filter>
 *         <action android:name="android.telephony.ims.ImsService" />
 *     </intent-filter>
 * </service>
 * ...
 *
 * The telephony framework will then bind to the ImsService you have defined in your manifest
 * if you are either:
 * 1) Defined as the default ImsService for the device in the device overlay using
 *    "config_ims_package".
 * 2) Defined as a Carrier Provided ImsService in the Carrier Configuration using
 *    {@link CarrierConfigManager#KEY_CONFIG_IMS_PACKAGE_OVERRIDE_STRING}.
 *
 * There are two ways to define to the platform which {@link ImsFeature}s this {@link ImsService}
 * supports, dynamic or static definitions.
 *
 * In the static definition, the {@link ImsFeature}s that are supported are defined in the service
 * definition of the AndroidManifest.xml file as metadata:
 * <!-- Apps must declare which features they support as metadata. The different categories are
 *      defined below. In this example, the MMTEL_FEATURE feature is supported. -->
 * <meta-data android:name="android.telephony.ims.MMTEL_FEATURE" android:value="true" />
 *
 * The features that are currently supported in an ImsService are:
 * - RCS_FEATURE: This ImsService implements the RcsFeature class.
 * - MMTEL_FEATURE: This ImsService implements the MmTelFeature class.
 * - EMERGENCY_MMTEL_FEATURE: This ImsService supports Emergency Calling for MMTEL, must be
 *   declared along with the MMTEL_FEATURE. If this is not specified, the framework will use
 *   circuit switch for emergency calling.
 *
 * In the dynamic definition, the supported features are not specified in the service definition
 * of the AndroidManifest. Instead, the framework binds to this service and calls
 * {@link #querySupportedImsFeatures()}. The {@link ImsService} then returns an
 * {@link ImsFeatureConfiguration}, which the framework uses to initialize the supported
 * {@link ImsFeature}s. If at any time, the list of supported {@link ImsFeature}s changes,
 * {@link #onUpdateSupportedImsFeatures(ImsFeatureConfiguration)} can be called to tell the
 * framework of the changes.
 *
 * @hide
 */
@SystemApi
public class MtkDynamicImsService extends android.telephony.ims.ImsService {

    private static final String LOG_TAG = "MtkDynamicImsService";

    protected final SparseArray<MmTelFeature> mMmTel = new SparseArray<>();
    protected final SparseArray<ImsConfigImplBase> mImsConfig = new SparseArray<>();
    protected final SparseArray<ImsRegistrationImplBase> mImsReg = new SparseArray<>();

    /**
     * @hide
     */
    @Override
    public IBinder onBind(Intent intent) {
        if(SERVICE_INTERFACE.equals(intent.getAction())) {
            log("MtkDynamicImsService Bound.");
            return mImsServiceController;
        }
        return null;
    }

    /**
     * @hide
     */
    @Override
    public boolean onUnbind (Intent intent) {
        logi("onUnbind..." );
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            MtkMmTelFeature feature = (MtkMmTelFeature)mMmTel.get(i);
            if (feature != null) {
                feature.close();
                mMmTel.delete(i);
            }
            MtkImsRegistrationImpl reg = (MtkImsRegistrationImpl)mImsReg.get(i);
            if (reg != null) {
                reg.close();
                mImsReg.delete(i);
            }
        }
        return super.onUnbind(intent);
    }

    /**
     * When called, provide the {@link ImsFeatureConfiguration} that this {@link ImsService}
     * currently supports. This will trigger the framework to set up the {@link ImsFeature}s that
     * correspond to the {@link ImsFeature}s configured here.
     *
     * Use {@link #onUpdateSupportedImsFeatures(ImsFeatureConfiguration)} to change the supported
     * {@link ImsFeature}s.
     *
     * @return an {@link ImsFeatureConfiguration} containing Features this ImsService supports.
     */
    public ImsFeatureConfiguration querySupportedImsFeatures() {
        ImsFeatureConfiguration.Builder builder = new ImsFeatureConfiguration.Builder();
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            builder.addFeature(i, ImsFeature.FEATURE_MMTEL);
            builder.addFeature(i, ImsFeature.FEATURE_EMERGENCY_MMTEL);
        }
        log("Supported Ims Features: " + builder.build());
        // Return empty for base implementation
        return builder.build();
    }

    /**
     * The ImsService has been bound and is ready for ImsFeature creation based on the Features that
     * the ImsService has registered for with the framework, either in the manifest or via
     * {@link #querySupportedImsFeatures()}.
     *
     * The ImsService should use this signal instead of onCreate/onBind or similar to perform
     * feature initialization because the framework may bind to this service multiple times to
     * query the ImsService's {@link ImsFeatureConfiguration} via
     * {@link #querySupportedImsFeatures()}before creating features.
     */
    public void readyForFeatureCreation() {
    }

    /**
     * The framework has enabled IMS for the slot specified, the ImsService should register for IMS
     * and perform all appropriate initialization to bring up all ImsFeatures.
     */
    public void enableIms(int slotId) {
        MtkMmTelFeature feature = (MtkMmTelFeature)mMmTel.get(slotId);
        if (feature != null && SubscriptionManager.isValidPhoneId(slotId)) {
            feature.enableIms(slotId);
        }
    }

    /**
     * The framework has disabled IMS for the slot specified. The ImsService must deregister for IMS
     * and set capability status to false for all ImsFeatures.
     */
    public void disableIms(int slotId) {
        MtkMmTelFeature feature = (MtkMmTelFeature)mMmTel.get(slotId);
        if (feature != null && SubscriptionManager.isValidPhoneId(slotId)) {
            feature.disableIms(slotId);
        }
    }

    /**
     * When called, the framework is requesting that a new {@link MmTelFeature} is created for the
     * specified slot.
     *
     * @param slotId The slot ID that the MMTEL Feature is being created for.
     * @return The newly created {@link MmTelFeature} associated with the slot or null if the
     * feature is not supported.
     */
    public MmTelFeature createMmTelFeature(int slotId) {
        MmTelFeature feature = mMmTel.get(slotId);
        if (feature == null && SubscriptionManager.isValidPhoneId(slotId)) {
            feature = new MtkMmTelFeature(slotId);
            mMmTel.put(slotId, feature);
        }
        log("[" + slotId + "] createMmTelFeature " + feature);
        return feature;
    }

    /**
     * Return the {@link ImsConfigImplBase} implementation associated with the provided slot. This
     * will be used by the platform to get/set specific IMS related configurations.
     *
     * @param slotId The slot that the IMS configuration is associated with.
     * @return ImsConfig implementation that is associated with the specified slot.
     */
    public ImsConfigImplBase getConfig(int slotId) {
        ImsConfigImplBase config = mImsConfig.get(slotId);
        if (config == null && SubscriptionManager.isValidPhoneId(slotId)) {
            MtkMmTelFeature feature = (MtkMmTelFeature)mMmTel.get(slotId);
            if (feature != null) {
                config = new MtkImsConfigImpl(feature.getConfigInterface());
                mImsConfig.put(slotId, config);
            }
        }
        log("[" + slotId + "] getConfig " + config);
        return config;
    }

    /**
     * Return the {@link ImsRegistrationImplBase} implementation associated with the provided slot.
     *
     * @param slotId The slot that is associated with the IMS Registration.
     * @return the ImsRegistration implementation associated with the slot.
     */
    public ImsRegistrationImplBase getRegistration(int slotId) {
        ImsRegistrationImplBase reg = mImsReg.get(slotId);
        if (reg == null && SubscriptionManager.isValidPhoneId(slotId)) {
            reg = new MtkImsRegistrationImpl(slotId);
            mImsReg.put(slotId, reg);
        }
        log("[" + slotId + "] getRegistration " + reg);
        return reg;
    }

    private static void log(String msg) {
        Rlog.d(LOG_TAG, msg);
    }

    private static void logi(String msg) {
        Rlog.i(LOG_TAG, msg);
    }

    private static void loge(String msg) {
        Rlog.e(LOG_TAG, msg);
    }
}