/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.provisioning;

import android.content.Context;
import android.os.Binder;

import com.gsma.services.rcs.capability.Capabilities;
import com.mediatek.ims.rcsua.AcsConfiguration;
import com.mediatek.ims.rcsua.AcsEventCallback;
import com.mediatek.ims.rcsua.Capability;

import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.service.UaServiceManager;
import com.orangelabs.rcs.utils.SettingUtils;
import com.orangelabs.rcs.utils.logger.Logger;


public class AcsEventCallbackImpl {

    private Logger logger = Logger.getLogger(
            AcsEventCallbackImpl.class.getSimpleName());
    //private RcsUaService mRcsUaService;
    private RcsEventCallback mRcsEventCallback;

    public AcsEventCallbackImpl(Context context) {
        //mRcsUaService = rcsService;
        if (!AcsTApiServiceManager.initialize(context)) {
            logger.error("AcsTApiServiceManager initialization failed!");
        }
    }

    public void requestAcsService() {
        logger.debug("requestAcsService");
        mRcsEventCallback = new RcsEventCallback();
        UaServiceManager.getInstance().registerAcsCallback(mRcsEventCallback);
        //mRcsUaService.registerAcsEventCallback(mRcsEventCallback);
    }

    public void releaseAcsService() {
        logger.debug("releaseAcsService");
        UaServiceManager.getInstance().unregisterAcsCallback(mRcsEventCallback);
        mRcsEventCallback = null;
    }

    public class RcsEventCallback extends AcsEventCallback {

        public RcsEventCallback(){}

        public void onConfigurationStatusChanged(boolean valid, int version) {
            logger.debug("onConfigurationStatusChanged: valid: " + valid + " version: " + version);

            final long identity = Binder.clearCallingIdentity();
            try {
                if (valid) {
                    processConfigXMLData();
                } else if (!SettingUtils.isTestSim()
                           || RcsSettings.getInstance().isOP08SupportedByPlatform()) {
                    updateCapabilities();
                }
            } finally {
                Binder.restoreCallingIdentity(identity);
            }
        }

        public void onAcsConnected() {
             logger.debug("onAcsConnected");
             //processConfigXMLData();
        }

        public void onAcsDisconnected() {
            logger.debug("onAcsDisconnected");
        }
    }

    private void processConfigXMLData() {
        //getConfigfile
        AcsConfiguration config = null;
        config = UaServiceManager.getInstance().getService().getAcsConfiguration();
        if (config == null) {
            return;
        }

        if (config.readXmlData() == null) {
            return;
        }
        // Parse the received content
        RcsSettings settings = RcsSettings.getInstance();
        ProvisioningParser parser = new ProvisioningParser(config.readXmlData());

        parser.parse(settings.getGsmaRelease(), settings.getMessagingMode(), true);

        updateCapabilities();
    }

    private void updateCapabilities() {
        AcsTApiServiceManager tapi = AcsTApiServiceManager.getInstance();
        UaServiceManager service = UaServiceManager.getInstance();
        RcsSettings settings = RcsSettings.getInstance();
        boolean supported = false, changed = false;

        Capabilities myCapabilities = tapi.getMyCapabilities();
        Capability featureTags = service.getCapabilities();

        if (myCapabilities == null || featureTags == null)
            return;

        if ((supported = settings.isImSessionSupported()) != myCapabilities.isImSessionSupported()) {
            myCapabilities.setImSessionSupport(supported);
            changed = true;
        }
        if (supported)
            featureTags.add(FeatureTags.FEATURE_RCSE_CPM_SESSION);
        else
            featureTags.remove(FeatureTags.FEATURE_RCSE_CPM_SESSION);

        if ((supported = settings.isFileTransferSupported()) != myCapabilities.isFileTransferSupported()) {
            myCapabilities.setFileTransferSupport(supported);
            changed = true;
        }
        if (supported)
            featureTags.add(FeatureTags.FEATURE_RCSE_CPM_FT);
        else
            featureTags.remove(FeatureTags.FEATURE_RCSE_CPM_FT);

        if ((supported = settings.isVideoSharingSupported()) != myCapabilities.isVideoSharingSupported()) {
            myCapabilities.setVideoSharingSupport(supported);
            changed = true;
        }

        if ((supported = settings.isImageSharingSupported()) != myCapabilities.isImageSharingSupported()) {
            myCapabilities.setImageSharingSupport(supported);
            changed = true;
        }

        if ((supported = settings.isGeoLocationPushSupported()) != myCapabilities.isGeolocPushSupported()) {
            myCapabilities.setGeolocationPushSupport(supported);
            changed = true;
        }
        if (supported)
            featureTags.add(FeatureTags.FEATURE_3GPP_LOCATION_SHARE);
        else
            featureTags.remove(FeatureTags.FEATURE_3GPP_LOCATION_SHARE);

        if ((supported = settings.isIPVoiceCallSupported()) != myCapabilities.isIPVoiceCallSupported()) {
            myCapabilities.setIPVoiceCallSupport(supported);
            changed = true;
        }

        if ((supported = settings.isIPVideoCallSupported()) != myCapabilities.isIPVideoCallSupported()) {
            myCapabilities.setIPVideoCallSupport(supported);
            changed = true;
        }

        if ((supported = Boolean.parseBoolean(settings.readParameter(RcsSettingsData.STANDALONE_MSG_SUPPORT)))
                != myCapabilities.isStandaloneMsgSupported()) {
            myCapabilities.setStandaloneMsgSupport(supported);
            changed = true;
        }
        if (supported) {
            featureTags.add(FeatureTags.FEATURE_RCSE_PAGER_MSG);
            featureTags.add(FeatureTags.FEATURE_RCSE_LARGE_MSG);
        } else {
            featureTags.remove(FeatureTags.FEATURE_RCSE_PAGER_MSG);
            featureTags.remove(FeatureTags.FEATURE_RCSE_LARGE_MSG);
        }

        if ((supported = settings.isFileTransferThumbnailSupported())
                != myCapabilities.isFileTransferThumbnailSupported()) {
            myCapabilities.setFileTransferThumbnailSupport(supported);
            changed = true;
        }

        String ftUri = settings.getFtHttpServer();
        if ((supported = (ftUri != null && !ftUri.isEmpty() && !ftUri.equalsIgnoreCase("null")))
                != myCapabilities.isFileTransferHttpSupported()) {
            myCapabilities.setFileTransferHttpSupport(supported);
            changed = true;
        }
        if (supported)
            featureTags.add(FeatureTags.FEATURE_RCSE
                    + "=\"" + FeatureTags.FEATURE_RCSE_FT_HTTP + "\"");
        else
            featureTags.remove(FeatureTags.FEATURE_RCSE
                    + "=\"" + FeatureTags.FEATURE_RCSE_FT_HTTP + "\"");

        service.updateCapabilities(featureTags);

        if (changed) {
            logger.debug("updateCapabilities changed "
                    + "myCapabilities[" + myCapabilities.toString() + "]");
            tapi.publishMyCapabilities(true, myCapabilities);
        }
        else
            logger.debug("updateCapabilities no change");
    }
}
