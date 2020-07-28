package com.mediatek.ims;

import com.android.ims.ImsConnectionStateListener;
import android.telephony.ims.feature.MmTelFeature;
import android.net.Uri;

import android.annotation.ProductApi;

@ProductApi
public class MtkImsConnectionStateListener extends ImsConnectionStateListener {

    /**
     * M: Called when IMS emergency capability changed.
     */
    public void onImsEmergencyCapabilityChanged(boolean eccSupport) {
        // no-op
    }

    /**
     * M: Called when VoWifi wifi PDN Out Of Service state changed.
     */
    public void onWifiPdnOOSStateChanged(int oosState) {
        // no-op
    }

    /**
         * The status of the feature's capabilities has changed to either available or unavailable.
         * If unavailable, the feature is not able to support the unavailable capability at this
         * time.
         *
         * @param config The new availability of the capabilities.
         */
        public void onCapabilitiesStatusChanged(MmTelFeature.Capabilities config) {
        }

    /**
     * Hook listener for IMS registration state report indication
     * @param indType MtkImsManager.IMS_REGISTERING/IMS_REGISTERED/IMS_REGISTER_FAIL
     * @param uris The profile URIs
     * @param expireTime The expire time of the IMS registration connection. 0 denote registering
     * @param errCode Error code of registration failed
     * @param errMsg Error message of registration failed
     */
    public void onRegistrationImsStateInd(int indType, Uri[] uris, int expireTime,
            int errCode, String errMsg) {
    }

    public void onRedirectIncomingCallInd(int phoneId, String[] info) {
    }
}
