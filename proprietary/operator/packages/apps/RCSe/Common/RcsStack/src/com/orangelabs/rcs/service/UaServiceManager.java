package com.orangelabs.rcs.service;

import android.content.Context;
import android.os.Bundle;
import android.os.SystemProperties;
import android.util.Log;
import com.gsma.services.rcs.capability.Capabilities;
import com.mediatek.ims.rcsua.*;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provisioning.AcsTApiServiceManager;

import java.io.IOException;

public class UaServiceManager {

    public static UaServiceManager getInstance() {
        // Log.i("UaServiceManager", "getInstance:" + INSTANCE);
        if (INSTANCE == null) {
            synchronized (UaServiceManager.class) {
                if (INSTANCE == null) {
                    INSTANCE = new UaServiceManager();
                }
            }
        }

        return INSTANCE;
    }

    public void startService(Context context)  {
        Log.i("UaServiceManager", "startService");

        if (serviceStarted)
            return;

        if (RcsUaService.isAvailable(context)) {
            String optr = SystemProperties.get("persist.vendor.operator.optr");
            if ("op07".equalsIgnoreCase(optr)
                    || "op08".equalsIgnoreCase(optr)) {
                Bundle options = new Bundle();
                options.putBoolean(RcsUaService.OPTION_ROI_SUPPORT, true);
                uaService = RcsUaService.startService(context, serviceCallback, options);
                serviceStarted = true;
            } else {
                Log.d("UaServiceManager", "startService optr:" + optr);
            }
        }
    }

    public void startService(Context context, int phoneId)  {
        Log.i("UaServiceManager", "startService, phone id = " + phoneId);

        if (serviceStarted)
            return;

        if (RcsUaService.isAvailable(context)) {
            String optr = SystemProperties.get("persist.vendor.operator.optr");
            if ("op07".equalsIgnoreCase(optr) || "op08".equalsIgnoreCase(optr)) {
                Bundle options = new Bundle();
                options.putBoolean(RcsUaService.OPTION_ROI_SUPPORT, true);
                uaService = RcsUaService.startService(context, phoneId, serviceCallback, options);
                serviceStarted = true;
            } else {
                Log.i("UaServiceManager", "startService optr:" + optr);
            }
        }
    }

    public void stopService() {
        Log.i("UaServiceManager", "stopService:" + uaService);
        if (client != null) {
            uaService.unregisterClient(client);
            client = null;
        }
        if (uaService != null) {
            uaService.stopService();
            uaService = null;
        }
        serviceStarted = false;
    }

    public Client registerClient(ImsEventCallback callback) {
        Log.i("UaServiceManager", "registerClient:" + uaService);
        if (client != null)
            return client;

        if (uaService != null && uaService.isConnected()) {
            client = uaService.registerClient(callback);
        } else {
            imsCallback = callback;
        }

        return client;
    }

    public void unregisterClient(Client client) {
        Log.i("UaServiceManager", "unregisterClient:" + uaService);
        if (client == this.client) {
            this.client = null;
            if (client == null)
                imsCallback = null;
        }

        if (uaService != null) {
            uaService.unregisterClient(client);
        }
    }

    public void registerAcsCallback(AcsEventCallback callback) {
        Log.i("UaServiceManager", "registerAcsCallback:" + uaService);
        if (acsCallback != null)
            return;

        if (uaService != null && uaService.isConnected()) {
            uaService.registerAcsEventCallback(callback);
        } else {
            acsCallback = callback;
        }
    }

    public void unregisterAcsCallback(AcsEventCallback callback) {
        Log.i("UaServiceManager", "unregisterAcsCallback:" + uaService);
        if (callback == null)
            return;

        if (uaService != null && uaService.isConnected()) {
            uaService.unregisterAcsEventCallback(callback);
        }
    }

    public Capability getCapabilities() {
        Capability capability = null;

        if (uaService != null && uaService.isConnected())
            capability = uaService.getCapabilities();

        return capability;
    }

    public void updateCapabilities(Capability capability) {
        if (uaService != null && uaService.isConnected())
            uaService.updateCapabilities(capability);
    }

    public void publishRcsCap() {
        Log.i("UaServiceManager", "publishRcsCap: " + uaService);
        if (AcsTApiServiceManager.getInstance() == null) {
            AcsTApiServiceManager.initialize(AndroidFactory.getApplicationContext());
        }
        try {
            Capabilities myCapabilities =
                AcsTApiServiceManager.getInstance().getCapabilityApi().getMyCapabilities();
            //support internal test only
            if (RcsSettings.getInstance().isServiceActivated()) {
                myCapabilities.setImageSharingSupport(RcsSettings.getInstance().isImageSharingSupported());
                myCapabilities.setVideoSharingSupport(RcsSettings.getInstance().isVideoSharingSupported());
                myCapabilities.setIPVoiceCallSupport(RcsSettings.getInstance().isIPVoiceCallSupported());
                myCapabilities.setIPVideoCallSupport(RcsSettings.getInstance().isIPVideoCallSupported());
                myCapabilities.setImSessionSupport(RcsSettings.getInstance().isImSessionSupported());
                myCapabilities.setFileTransferSupport(RcsSettings.getInstance().isFileTransferSupported());
                myCapabilities.setFileTransferThumbnailSupport(RcsSettings.getInstance().isFileTransferThumbnailSupported());
                myCapabilities.setFileTransferHttpSupport(RcsSettings.getInstance().isFileTransferHttpSupported());
                myCapabilities.setGeolocationPushSupport(RcsSettings.getInstance().isGeoLocationPushSupported());
                myCapabilities.setStandaloneMsgSupport(RcsSettings.getInstance().isStandaloneMsgSupport());
            } else {
                myCapabilities.setImageSharingSupport(false);
                myCapabilities.setVideoSharingSupport(false);
                myCapabilities.setIPVoiceCallSupport(false);
                myCapabilities.setIPVideoCallSupport(false);
                myCapabilities.setImSessionSupport(false);
                myCapabilities.setFileTransferSupport(false);
                myCapabilities.setFileTransferThumbnailSupport(false);
                myCapabilities.setFileTransferHttpSupport(false);
                myCapabilities.setGeolocationPushSupport(false);
                myCapabilities.setStandaloneMsgSupport(false);
            }
            AcsTApiServiceManager.getInstance().getCapabilityApi().publishMyCap(true, myCapabilities);
        } catch (Exception e) {
            Log.i("UaServiceManager", "Exception");
        }
    }

    public void activate() {
        Log.i("UaServiceManager", "activate:" + uaService);
        if (uaService != null)
            uaService.activate();
    }

    public void deactivate() {
        Log.i("UaServiceManager", "deactivate:" + uaService);
        if (uaService != null)
            uaService.deactivate();
    }

    public void triggerReregistration() {
        Log.i("UaServiceManager", "triggerReregistration:" + uaService);
        if (serviceConnected()) {
            uaService.triggerReregistration();
        }
    }

    public SipChannel openChannel(SipChannel.EventCallback callback) {
        Log.i("UaServiceManager", "openChannel:" + client);
        if (client != null) {
            try {
                channel = client.openSipChannel(callback);
            } catch (IOException e) {

            }
        }
        return channel;
    }

    public void closeChannel(SipChannel channel) {
        Log.i("UaServiceManager", "closeChannel:" + channel);
        channel.close();
        if (channel == this.channel)
            this.channel = null;
    }

    public boolean serviceConnected() {
        return uaService != null && uaService.isConnected();
    }

    public boolean imsRegistered() {
        return client != null && client.getRegistrationInfo().isRegistered();
    }

    public RcsUaService getService() {
        return uaService;
    }

    public Client getActiveClient() {
        return client;
    }

    public SipChannel getSipChannel() {
        return channel;
    }

    public Configuration readConfiguraion() {
        if (imsRegistered()) {
            return client.getRegistrationInfo().readImsConfiguration();
        }
        return null;
    }

    private RcsUaService.Callback serviceCallback = new RcsUaService.Callback() {
        @Override
        public void serviceConnected(RcsUaService service) {
            Log.i("UaServiceManager", "serviceConnected:" + service);
            if (imsCallback != null) {
                client = uaService.registerClient(imsCallback);
                imsCallback = null;
            }
            if (acsCallback != null) {
                uaService.registerAcsEventCallback(acsCallback);
                acsCallback = null;
            }
        }

        @Override
        public void serviceDisconnected(RcsUaService service) {
            Log.i("UaServiceManager", "serviceDisconnected:" + service);
            if (client != null) {
                RcsUaService.getInstance().unregisterClient(client);
                client = null;
                channel = null;
            }
            if (uaService != null) {
                uaService = null;
            }
            serviceStarted = false;
        }
    };

    private UaServiceManager() {
    }

    private static UaServiceManager INSTANCE = null;
    private RcsUaService uaService = null;
    private Client client = null;
    private ImsEventCallback imsCallback = null;
    private AcsEventCallback acsCallback = null;
    private SipChannel channel = null;
    private Capability capability = null;
    private boolean serviceStarted;
}
