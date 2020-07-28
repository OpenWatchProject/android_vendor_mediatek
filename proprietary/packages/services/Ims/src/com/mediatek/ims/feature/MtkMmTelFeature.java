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


package com.mediatek.ims.feature;

import android.annotation.IntDef;
import android.annotation.SystemApi;
import android.os.Bundle;
import android.os.Message;
import android.os.RemoteException;
import android.os.Build;
import android.os.SystemProperties;
import android.telecom.TelecomManager;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.ims.aidl.IImsCapabilityCallback;
import android.telephony.ims.aidl.IImsMmTelFeature;
import android.telephony.ims.aidl.IImsMmTelListener;
import android.telephony.ims.aidl.IImsSmsListener;
import android.telephony.ims.stub.ImsEcbmImplBase;
import android.telephony.ims.stub.ImsMultiEndpointImplBase;
import android.telephony.ims.stub.ImsUtImplBase;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import android.telephony.ims.ImsCallProfile;
import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsEcbm;
import com.android.ims.internal.IImsMultiEndpoint;
import com.android.ims.internal.IImsUt;
// Old IImsConfig
import com.android.ims.internal.IImsConfig;;
import com.android.ims.ImsConfigListener;
import com.android.ims.ImsConfig;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.ImsCallSession;
import com.android.internal.annotations.VisibleForTesting;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

import com.mediatek.ims.feature.MtkImsSmsImpl;
import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.ImsService;
import com.mediatek.ims.ImsCallSessionProxy;
import com.mediatek.ims.ImsUtImpl;
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.LegacyComponentFactory;

import android.content.Context;

// Import for ImsConfig-related function
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import android.text.TextUtils;

/**
 * Base implementation for Voice and SMS (IR-92) and Video (IR-94) IMS support.
 *
 * Any class wishing to use MmTelFeature should extend this class and implement all methods that the
 * service supports.
 * @hide
 */
@SystemApi
public class MtkMmTelFeature extends MmTelFeature {

    private static final String LOG_TAG = "MtkMmTelFeature";
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean DEBUG = TextUtils.equals(Build.TYPE, "eng")
            || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private int mSlotId;
    private ImsService mImsServiceImpl = null;
    private Context mContext;

    // Delay between ImsService queries.
    private static final int DELAY_IMS_SERVICE_IMPL_QUERY_MS = 5000;
    private static final int MAXMUIM_IMS_SERVICE_IMPL_RETRY = 3;
    // Wait timeout for capability query from ImsConfig
    private static final int WAIT_TIMEOUT_MS = 2000;

    private static final Map<Integer, Integer> REG_TECH_TO_NET_TYPE = new HashMap<>(2);

    static {
        REG_TECH_TO_NET_TYPE.put(ImsRegistrationImplBase.REGISTRATION_TECH_LTE,
                TelephonyManager.NETWORK_TYPE_LTE);
        REG_TECH_TO_NET_TYPE.put(ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN,
                TelephonyManager.NETWORK_TYPE_IWLAN);
    }

    // Feature Type for compatibility with old "feature" updates
    // These defined value are deprecated in ImsConfig
    public static final int FEATURE_TYPE_UNKNOWN = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
    public static final int FEATURE_TYPE_VOICE_OVER_LTE =
            ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE;
    public static final int FEATURE_TYPE_VIDEO_OVER_LTE =
            ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE;
    public static final int FEATURE_TYPE_VOICE_OVER_WIFI =
            ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI;
    public static final int FEATURE_TYPE_VIDEO_OVER_WIFI =
            ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI;
    public static final int FEATURE_TYPE_UT_OVER_LTE =
            ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE;
    public static final int FEATURE_TYPE_UT_OVER_WIFI =
            ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI;

    public static final int FEATURE_UNKNOWN = ImsConfig.FeatureValueConstants.ERROR;
    public static final int FEATURE_DISABLED = ImsConfig.FeatureValueConstants.OFF;
    public static final int FEATURE_ENABLED = ImsConfig.FeatureValueConstants.ON;

    private final ImsService.IMtkMmTelFeatureCallback mImsServiceCallback =
            new ImsService.IMtkMmTelFeatureCallback() {
        // Implement module callback function here
        @Override
        public void notifyContextChanged(Context context) {
            mContext = context;
            log("Set context to " + mContext);
        }

        @Override
        public void sendSmsRsp(int token, int messageRef,
            @ImsSmsImplBase.SendStatusResult int status, int reason) {
            log("sendSmsRsp, token " + token + ", messageRef " + messageRef
                + ", status " + status + ", reason " + reason);
            MtkImsSmsImpl smsImpl = (MtkImsSmsImpl) getSmsImplementation();
            if (smsImpl != null) {
                smsImpl.sendSmsRsp(token, messageRef, status, reason);
            }
        }

        @Override
        public void newStatusReportInd(byte[] pdu, String format) {
            MtkImsSmsImpl smsImpl = (MtkImsSmsImpl) getSmsImplementation();
            if (smsImpl != null) {
                smsImpl.newStatusReportInd(pdu, format);
            }
        }

        @Override
        public void newImsSmsInd(byte[] pdu, String format) {
            MtkImsSmsImpl smsImpl = (MtkImsSmsImpl) getSmsImplementation();
            if (smsImpl != null) {
                smsImpl.newImsSmsInd(pdu, format);
            }
        }

        @Override
        public void notifyCapabilitiesChanged(MmTelFeature.MmTelCapabilities c) {
            log("notifyCapabilitiesStatusChanged " + c);
            onCapabilitiesStatusChanged(c);
        }

        @Override
        public void notifyIncomingCall(ImsCallSessionImplBase c, Bundle extras) {
            log("notifyIncomingCall ImsCallSessionImplBase " + c + " extras "
                    + Rlog.pii(LOG_TAG, extras));
            onNotifyIncomingCall(c, extras);
        }

        @Override
        public void notifyIncomingCallSession(IImsCallSession c, Bundle extras) {
            log("notifyIncomingCallSession IImsCallSession " + c + " extras "
                    + Rlog.pii(LOG_TAG, extras));
            onNotifyIncomingCallSession(c, extras);
        }

        @Override
        public void updateCapbilities(CapabilityChangeRequest request) {
            log("updateCapbilities " + request);
            changeEnabledCapabilities(request, null);
        }
    };

    public MtkMmTelFeature(int slotId) {
        mSlotId = slotId;

        int retry = 0;
        while ((mImsServiceImpl == null) && (retry < MAXMUIM_IMS_SERVICE_IMPL_RETRY)) {
            mImsServiceImpl = ImsService.getInstance(null);
            try {
                if (mImsServiceImpl == null) {
                    log("ImsService is not initialized yet. Query later - " + retry);
                    Thread.sleep(DELAY_IMS_SERVICE_IMPL_QUERY_MS);
                    retry++;
                }
            } catch (InterruptedException er) {
                loge("Fail to get ImsService " + er);
            }
        }
        if (mImsServiceImpl != null) {
            mImsServiceImpl.setMmTelFeatureCallback(slotId, mImsServiceCallback);
            log("initialize mContext " + mContext + " slotId " + slotId);
            initialize(mContext, slotId);
            setFeatureState(STATE_READY);
        }
        log("[" + mSlotId +"] MtkMmTelFeature created");
    }

    private static class ConfigListener extends ImsConfigListener.Stub {

        private final int mCapability;
        private final int mTech;
        private final CountDownLatch mLatch;

        public ConfigListener(int capability, int tech, CountDownLatch latch) {
            mCapability = capability;
            mTech = tech;
            mLatch = latch;
        }

        @Override
        public void onGetFeatureResponse(int feature, int network, int value, int status)
                throws RemoteException {
            if (feature == mCapability && network == mTech) {
                mLatch.countDown();
                getFeatureValueReceived(value);
            } else {
                Rlog.e(LOG_TAG, "onGetFeatureResponse: response different than requested: feature="
                        + feature + " and network=" + network);
            }
        }

        @Override
        public void onSetFeatureResponse(int feature, int network, int value, int status)
                throws RemoteException {
            if (feature == mCapability && network == mTech) {
                mLatch.countDown();
                setFeatureValueReceived(value);
            } else {
                Rlog.e(LOG_TAG, "onSetFeatureResponse: response different than requested: feature="
                        + feature + " and network=" + network);
            }
        }

        @Override
        public void onGetVideoQuality(int status, int quality) throws RemoteException {
        }

        @Override
        public void onSetVideoQuality(int status) throws RemoteException {
        }

        public void getFeatureValueReceived(int value) {
        }

        public void setFeatureValueReceived(int value) {
        }
    }

    public final void onCapabilitiesStatusChanged(MmTelCapabilities c) {
        try {
            super.notifyCapabilitiesStatusChanged(c);
        } catch (IllegalStateException e) {
            loge("onCapabilitiesStatusChanged error. msg " + e.getMessage());
        }
    }

    public void onNotifyIncomingCall(ImsCallSessionImplBase c, Bundle extras) {
        super.notifyIncomingCall(c, extras);
    }

    public void onNotifyIncomingCallSession(IImsCallSession c, Bundle extras) {
        super.notifyIncomingCallSession(c, extras);
    }

    private MmTelCapabilities convertCapabilities(int[] enabledFeatures) {
        boolean[] featuresEnabled = new boolean[enabledFeatures.length];
        for (int i = FEATURE_TYPE_VOICE_OVER_LTE; i <= FEATURE_TYPE_UT_OVER_WIFI
                && i < enabledFeatures.length; i++) {
            if (enabledFeatures[i] == i) {
                featuresEnabled[i] = true;
            } else if (enabledFeatures[i] == FEATURE_TYPE_UNKNOWN) {
                // FEATURE_TYPE_UNKNOWN indicates that a feature is disabled.
                featuresEnabled[i] = false;
            }
        }
        MmTelCapabilities capabilities = new MmTelCapabilities();
        if (featuresEnabled[FEATURE_TYPE_VOICE_OVER_LTE]
                || featuresEnabled[FEATURE_TYPE_VOICE_OVER_WIFI]) {
            // voice is enabled
            capabilities.addCapabilities(MmTelCapabilities.CAPABILITY_TYPE_VOICE);
        }
        if (featuresEnabled[FEATURE_TYPE_VIDEO_OVER_LTE]
                || featuresEnabled[FEATURE_TYPE_VIDEO_OVER_WIFI]) {
            // video is enabled
            capabilities.addCapabilities(MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
        }
        if (featuresEnabled[FEATURE_TYPE_UT_OVER_LTE]
                || featuresEnabled[FEATURE_TYPE_UT_OVER_WIFI]) {
            // ut is enabled
            capabilities.addCapabilities(MmTelCapabilities.CAPABILITY_TYPE_UT);
        }
        log("convertCapabilities - capabilities: " + capabilities);
        return capabilities;
    }

    private int convertCapability(int capability, int radioTech) {
        int capConverted = FEATURE_TYPE_UNKNOWN;
        if (radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_LTE) {
            switch (capability) {
                case MmTelCapabilities.CAPABILITY_TYPE_VOICE:
                    capConverted = FEATURE_TYPE_VOICE_OVER_LTE;
                    break;
                case MmTelCapabilities.CAPABILITY_TYPE_VIDEO:
                    capConverted = FEATURE_TYPE_VIDEO_OVER_LTE;
                    break;
                case MmTelCapabilities.CAPABILITY_TYPE_UT:
                    capConverted = FEATURE_TYPE_UT_OVER_LTE;
                    break;
            }
        } else if (radioTech == ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN) {
            switch (capability) {
                case MmTelCapabilities.CAPABILITY_TYPE_VOICE:
                    capConverted = FEATURE_TYPE_VOICE_OVER_WIFI;
                    break;
                case MmTelCapabilities.CAPABILITY_TYPE_VIDEO:
                    capConverted = FEATURE_TYPE_VIDEO_OVER_WIFI;
                    break;
                case MmTelCapabilities.CAPABILITY_TYPE_UT:
                    capConverted = FEATURE_TYPE_UT_OVER_WIFI;
                    break;
            }
        }
        return capConverted;
    }

    /**
     * Provides the MmTelFeature with the ability to return the framework Capability Configuration
     * for a provided Capability. If the framework calls {@link #changeEnabledCapabilities} and
     * includes a capability A to enable or disable, this method should return the correct enabled
     * status for capability A.
     * @param capability The capability that we are querying the configuration for.
     * @return true if the capability is enabled, false otherwise.
     */
    public boolean queryCapabilityConfiguration(
            @MmTelCapabilities.MmTelCapability int capability,
            @ImsRegistrationImplBase.ImsRegistrationTech int radioTech) {
        int capConverted = convertCapability(capability, radioTech);
        // Wait for the result from the ImsService
        CountDownLatch latch = new CountDownLatch(1);
        final int[] returnValue = new int[1];
        returnValue[0] = FEATURE_UNKNOWN;
        int regTech = REG_TECH_TO_NET_TYPE.getOrDefault(radioTech,
                ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
        try {
            getConfigInterface().getFeatureValue(capConverted, regTech,
                    new ConfigListener(capConverted, regTech, latch) {
                        @Override
                        public void getFeatureValueReceived(int value) {
                            returnValue[0] = value;
                            log("Feature " + capability + " tech " + radioTech + "enable? "
                                    + returnValue[0]);
                        }
                    });
        } catch (RemoteException e) {
            loge("Fail to queryCapabilityConfiguration " + e.getMessage());
        }
        try {
            latch.await(WAIT_TIMEOUT_MS, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            loge("queryCapabilityConfiguration - error waiting: " + e.getMessage());
        }
        return returnValue[0] == FEATURE_ENABLED;
    }

    /**
     * The MmTelFeature should override this method to handle the enabling/disabling of
     * MmTel Features, defined in {@link MmTelCapabilities.MmTelCapability}. The framework assumes
     * the {@link CapabilityChangeRequest} was processed successfully. If a subset of capabilities
     * could not be set to their new values,
     * {@link CapabilityCallbackProxy#onChangeCapabilityConfigurationError} must be called
     * individually for each capability whose processing resulted in an error.
     *
     * Enabling/Disabling a capability here indicates that the capability should be registered or
     * deregistered (depending on the capability change) and become available or unavailable to
     * the framework.
     */
    @Override
    public void changeEnabledCapabilities(CapabilityChangeRequest request,
            CapabilityCallbackProxy c) {
        if (request == null) {
            return;
        }
        try {
            IImsConfig imsConfig = getConfigInterface();
            // Disable Capabilities
            for (CapabilityChangeRequest.CapabilityPair cap : request.getCapabilitiesToDisable()) {
                CountDownLatch latch = new CountDownLatch(1);
                int capConverted = convertCapability(cap.getCapability(), cap.getRadioTech());
                int radioTechConverted = REG_TECH_TO_NET_TYPE.getOrDefault(cap.getRadioTech(),
                        ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
                if (DEBUG) {
                    log("changeEnabledCapabilities - cap: " + capConverted + " radioTech: "
                            + radioTechConverted + " disabled");
                }
                imsConfig.setFeatureValue(capConverted, radioTechConverted, FEATURE_DISABLED,
                        new ConfigListener(capConverted, radioTechConverted, latch) {
                            @Override
                            public void setFeatureValueReceived(int value) {
                                if (value != FEATURE_DISABLED) {
                                    if (c == null) {
                                        return;
                                    }
                                    c.onChangeCapabilityConfigurationError(cap.getCapability(),
                                            cap.getRadioTech(), CAPABILITY_ERROR_GENERIC);
                                }
                                if (DEBUG) {
                                    log("changeEnabledCapabilities - setFeatureValueReceived"
                                            + " with value " + value);
                                }
                            }
                        });
                latch.await(WAIT_TIMEOUT_MS, TimeUnit.MILLISECONDS);
            }
            // Enable Capabilities
            for (CapabilityChangeRequest.CapabilityPair cap : request.getCapabilitiesToEnable()) {
                CountDownLatch latch = new CountDownLatch(1);
                int capConverted = convertCapability(cap.getCapability(), cap.getRadioTech());
                int radioTechConverted = REG_TECH_TO_NET_TYPE.getOrDefault(cap.getRadioTech(),
                        ImsRegistrationImplBase.REGISTRATION_TECH_NONE);
                if (DEBUG) {
                    log("changeEnabledCapabilities - cap: " + capConverted + " radioTech: "
                            + radioTechConverted + " enabled");
                }
                imsConfig.setFeatureValue(capConverted, radioTechConverted, FEATURE_ENABLED,
                        new ConfigListener(capConverted, radioTechConverted, latch) {
                            @Override
                            public void setFeatureValueReceived(int value) {
                                if (value != FEATURE_ENABLED) {
                                    if (c == null) {
                                        return;
                                    }
                                    c.onChangeCapabilityConfigurationError(cap.getCapability(),
                                            cap.getRadioTech(), CAPABILITY_ERROR_GENERIC);
                                }
                                if (DEBUG) {
                                    log("changeEnabledCapabilities - setFeatureValueReceived"
                                            + " with value " + value);
                                }
                            }
                        });
                latch.await(WAIT_TIMEOUT_MS, TimeUnit.MILLISECONDS);
            }
        } catch (RemoteException | InterruptedException e) {
            log("changeEnabledCapabilities: Error processing: " + e.getMessage());
        }
    }

    /**
     * Creates a {@link ImsCallProfile} from the service capabilities & IMS registration state.
     *
     * @param callSessionType a service type that is specified in {@link ImsCallProfile}
     *        {@link ImsCallProfile#SERVICE_TYPE_NONE}
     *        {@link ImsCallProfile#SERVICE_TYPE_NORMAL}
     *        {@link ImsCallProfile#SERVICE_TYPE_EMERGENCY}
     * @param callType a call type that is specified in {@link ImsCallProfile}
     *        {@link ImsCallProfile#CALL_TYPE_VOICE}
     *        {@link ImsCallProfile#CALL_TYPE_VT}
     *        {@link ImsCallProfile#CALL_TYPE_VT_TX}
     *        {@link ImsCallProfile#CALL_TYPE_VT_RX}
     *        {@link ImsCallProfile#CALL_TYPE_VT_NODIR}
     *        {@link ImsCallProfile#CALL_TYPE_VS}
     *        {@link ImsCallProfile#CALL_TYPE_VS_TX}
     *        {@link ImsCallProfile#CALL_TYPE_VS_RX}
     * @return a {@link ImsCallProfile} object
     */
    public ImsCallProfile createCallProfile(int callSessionType, int callType) {

        log("createCallProfile: callSessionType = " + callSessionType + ",  callType = " + callType);

        ImsCallProfile profile = null;
        if (mImsServiceImpl != null) {
            profile = mImsServiceImpl.onCreateCallProfile(mSlotId, callSessionType, callType);
        }
        return profile;
    }

    /**
     * Creates an {@link ImsCallSession} with the specified call profile.
     * Use other methods, if applicable, instead of interacting with
     * {@link ImsCallSession} directly.
     *
     * @param profile a call profile to make the call
     */
    public ImsCallSessionImplBase createCallSession(ImsCallProfile profile) {

        log("createCallSession");

        ImsCallSessionProxy callSessionProxy = null;
        if (mImsServiceImpl != null) {
            callSessionProxy = mImsServiceImpl.onCreateCallSessionProxy(mSlotId, profile, null);
        }
        return callSessionProxy;
    }

    /**
     * Called by the framework to determine if the outgoing call, designated by the outgoing
     * {@link String}s, should be processed as an IMS call or CSFB call. If this method's
     * functionality is not overridden, the platform will process every call as IMS as long as the
     * MmTelFeature reports that the {@link MmTelCapabilities#CAPABILITY_TYPE_VOICE} capability is
     * available.
     * @param numbers An array of {@link String}s that will be used for placing the call. There can
     *         be multiple {@link String}s listed in the case when we want to place an outgoing
     *         call as a conference.
     * @return a {@link ProcessCallResult} to the framework, which will be used to determine if the
     *        call will be placed over IMS or via CSFB.
     */
    public @ProcessCallResult int shouldProcessCall(String[] numbers) {

        log("shouldProcessCall");

        return PROCESS_CALL_IMS;
    }

    /**
     *
     * @hide
     */
    protected IImsUt getUtInterface() throws RemoteException {
        ImsUtImplBase utImpl = getUt();
        return utImpl != null ? utImpl.getInterface() : null;
    }

    /**
     * @hide
     */
    protected IImsEcbm getEcbmInterface() throws RemoteException {

        log("getEcbmInterface");

        ImsEcbmImplBase ecbm = getEcbm();
        if (ecbm != null) {
            return ecbm.getImsEcbm();
        }
        return null;
    }

    /**
     * @hide
     */
    public IImsMultiEndpoint getMultiEndpointInterface() throws RemoteException {

        log("getMultiEndpointInterface");

        ImsMultiEndpointImplBase multiendpoint = getMultiEndpoint();
        if (multiendpoint != null) {
            return multiendpoint.getIImsMultiEndpoint();
        }
        return null;
    }

    /**
     * @return The {@link ImsUtImplBase} Ut interface implementation for the supplementary service
     * configuration.
     */
    public ImsUtImplBase getUt() {
        ImsUtImplBase inst;
        if (ImsCommonUtil.supportMdAutoSetupIms()) {
            inst = ImsUtImpl.getInstance(mContext, mSlotId, mImsServiceImpl);
        } else {
            LegacyComponentFactory factory = ExtensionFactory.makeLegacyComponentFactory(mContext);
            inst = factory.makeImsUt(mContext, mSlotId, mImsServiceImpl);
        }

        return inst;
    }

    /**
     * @return The {@link ImsEcbmImplBase} Emergency call-back mode interface for emergency VoLTE
     * calls that support it.
     */
    public ImsEcbmImplBase getEcbm() {

        log("getEcbm");

        ImsEcbmImplBase ecbm = null;
        if (mImsServiceImpl != null) {
            ecbm = mImsServiceImpl.onGetEcbmProxy(mSlotId);
        }
        return ecbm;

    }

    /**
     * @return The {@link ImsMultiEndpointImplBase} implementation for implementing Dialog event
     * package processing for multi-endpoint.
     */
    public ImsMultiEndpointImplBase getMultiEndpoint() {

        log("getMultiEndpoint");

        ImsMultiEndpointImplBase multiendpoint = null;
        if (mImsServiceImpl != null) {
            multiendpoint = mImsServiceImpl.onGetMultiEndpointProxy(mSlotId);
        }
        return multiendpoint;
    }

    /**
     * Sets the current UI TTY mode for the MmTelFeature.
     * @param mode An integer containing the new UI TTY Mode, can consist of
     *         {@link TelecomManager#TTY_MODE_OFF},
     *         {@link TelecomManager#TTY_MODE_FULL},
     *         {@link TelecomManager#TTY_MODE_HCO},
     *         {@link TelecomManager#TTY_MODE_VCO}
     * @param onCompleteMessage If non-null, this MmTelFeature should call this {@link Message} when
     *         the operation is complete by using the associated {@link android.os.Messenger} in
     *         {@link Message#replyTo}. For example:
     * {@code
     *     // Set UI TTY Mode and other operations...
     *     try {
     *         // Notify framework that the mode was changed.
     *         Messenger uiMessenger = onCompleteMessage.replyTo;
     *         uiMessenger.send(onCompleteMessage);
     *     } catch (RemoteException e) {
     *         // Remote side is dead
     *     }
     * }
     */
    public void setUiTtyMode(int mode, Message onCompleteMessage) {
        // Base Implementation - Should be overridden
    }

    /**
     * @return The config interface for IMS Configuration
     */
    public IImsConfig getConfigInterface() {
        IImsConfig configInterface = null;
        if (mImsServiceImpl != null) {
            configInterface = mImsServiceImpl.onGetConfigInterface(mSlotId);
        }
        return configInterface;
    }

    /**
     * Must be overridden by IMS Provider to be able to support SMS over IMS. Otherwise a default
     * non-functional implementation is returned.
     *
     * @return an instance of {@link ImsSmsImplBase} which should be implemented by the IMS
     * Provider.
     */
    public ImsSmsImplBase getSmsImplementation() {
        return MtkImsSmsImpl.getInstance(mContext, mSlotId, mImsServiceImpl);
    }

    private String getSmsFormat() {
        return getSmsImplementation().getSmsFormat();
    }

    /**{@inheritDoc}*/
    @Override
    public void onFeatureRemoved() {
        // Base Implementation - Should be overridden
    }

    /**{@inheritDoc}*/
    @Override
    public void onFeatureReady() {
        // Base Implementation - Should be overridden
        log("onFeatureReady called!");
    }

    public void enableIms(int slotId) {
        if (mImsServiceImpl != null) {
            mImsServiceImpl.enableIms(slotId);
        }
    }

    /**
     * The framework has disabled IMS for the slot specified. The ImsService must deregister for IMS
     * and set capability status to false for all ImsFeatures.
     */
    public void disableIms(int slotId) {
        if (mImsServiceImpl != null) {
            mImsServiceImpl.disableIms(slotId);
        }
    }

    public void close() {
        if (mImsServiceImpl != null) {
            mImsServiceImpl.setMmTelFeatureCallback(mSlotId, null);
            logi("Unregister callback from ImsService");
        }
    }

    private void log(String msg) {
        Rlog.d(LOG_TAG, "[" + mSlotId + "] " + msg);
    }

    private void logi(String msg) {
        Rlog.i(LOG_TAG, "[" + mSlotId + "] " + msg);
    }

    private void loge(String msg) {
        Rlog.e(LOG_TAG, "[" + mSlotId + "] " + msg);
    }
}
