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
 /*
 * Copyright (C) 2018 MediaTek Inc., this file is modified on 07/05/2018
 * by MediaTek Inc. based on Apache License, Version 2.0.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. See NOTICE for more details.
 */

package com.mediatek.presence.core.ims;

import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

import java.io.IOException;
import java.security.KeyStoreException;
import java.util.Enumeration;

import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.CoreListener;
import com.mediatek.presence.core.ims.network.ImsConnectionManager;
import com.mediatek.presence.core.ims.network.ImsNetworkInterface;
import com.mediatek.presence.core.ims.network.sip.SipManager;


import com.mediatek.presence.core.ims.protocol.sip.SipEventListener;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.security.cert.KeyStoreManager;
import com.mediatek.presence.core.ims.security.cert.KeyStoreManagerException;
import com.mediatek.presence.core.ims.service.ImsService;
import com.mediatek.presence.core.ims.service.ImsServiceDispatcher;
import com.mediatek.presence.core.ims.service.ImsServiceSession;
import com.mediatek.presence.core.ims.service.SubscriptionManager;
import com.mediatek.presence.core.ims.service.capability.CapabilityServiceFactory;
import com.mediatek.presence.core.ims.service.capability.CapabilityService;
//import com.mediatek.presence.core.ims.service.im.InstantMessagingService;
//import com.mediatek.presence.core.ims.service.im.filetransfer.http.HttpTransferManager;
//import com.mediatek.presence.core.ims.service.ipcall.IPCallService;
import com.mediatek.presence.core.ims.service.presence.PresenceService;
import com.mediatek.presence.core.ims.service.presence.PresenceServiceFactory;

//import com.mediatek.presence.core.ims.service.richcall.RichcallService;
import com.mediatek.presence.core.ims.service.sip.SipService;
import com.mediatek.presence.core.ims.service.terms.TermsConditionsService;
import com.mediatek.presence.core.ims.userprofile.UserProfile;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.PstUtils;
import com.mediatek.presence.utils.SimUtils;

/**
 * IMS module
 *
 * @author JM. Auffret
 */
public class ImsModule implements SipEventListener {
    /**
     * Core
     */
    private Core core;

    /**
     * IMS user profile
     */
    public static UserProfile IMS_USER_PROFILE = null;

    /**
     * IMS connection manager
     */
    private ImsConnectionManager connectionManager;

    /**
     * IMS services
     */
    private ImsService services[];

    /**
     * Service dispatcher
     */
    private ImsServiceDispatcher serviceDispatcher;

    /**
     * Subscription manager
     */
    private SubscriptionManager subscriptionManager;

    /**
     * flag to indicate whether instantiation is finished
     */
    private boolean isReady = false;

    /**
     * The logger
     */
    private Logger logger = null;

    private int mSlotId = 0;

    private boolean mIsProvisioned = true;

    /**
     * Constructor
     *
     * @param core Core
     * @throws CoreException
     */
    public ImsModule(Core core, int slotId) throws CoreException {
        this.core = core;
        mSlotId = slotId;
        logger = Logger.getLogger(mSlotId, "ImsModule");

        if (logger.isActivated()) {
            logger.info("IMS module initialization");
        }

        // Create the IMS connection manager
        try {
            connectionManager = new ImsConnectionManager(this);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("IMS connection manager initialization has failed", e);
            }
            throw new CoreException("Can't instanciate the IMS connection manager");
        }

        // Set general parameters
        SipManager.TIMEOUT = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .getSipTransactionTimeout();

        // Load keystore for certificates
        try {
            KeyStoreManager.loadKeyStore(mSlotId);
        } catch(KeyStoreManagerException e) {
            if (logger.isActivated()) {
                logger.error("Can't load keystore manager", e);
            }
            throw new CoreException("Keystore manager exeception");
        } catch (KeyStoreException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        // Instanciates the IMS services
        services = new ImsService[7];

        // Create terms & conditions service
        services[ImsService.TERMS_SERVICE] = null; //new TermsConditionsService(this);

        // Create capability discovery service
        services[ImsService.CAPABILITY_SERVICE] = CapabilityServiceFactory.getInstance(this);

        // Create IM service (mandatory)
        services[ImsService.IM_SERVICE] = null; //new InstantMessagingService(this);

        // Create IP call service (optional)
        services[ImsService.IPCALL_SERVICE] = null; //new IPCallService(this);

        // Create richcall service (optional)
        services[ImsService.RICHCALL_SERVICE] = null; //new RichcallService(this);

        // Create presence service (optional)
        services[ImsService.PRESENCE_SERVICE] = PresenceServiceFactory.newInstance(this);

        // Create generic SIP service
        services[ImsService.SIP_SERVICE] = new SipService(this);

        // Create the service dispatcher
        serviceDispatcher = new ImsServiceDispatcher(this);

        // Create subscription manager
        subscriptionManager = new SubscriptionManager(this);

        isReady = true;

        if (SimUtils.isVzwSimCard(mSlotId)) {
            PstUtils.createInstance(AndroidFactory.getApplicationContext());
        }

        if (logger.isActivated()) {
            logger.info("IMS module has been created");
        }
    }

    /**
     * Returns the SIP manager
     *
     * @return SIP manager
     */
    public SipManager getSipManager() {
        return getCurrentNetworkInterface().getSipManager();
    }

    /**
     * Returns the current network interface
     *
     * @return Network interface
     */
    public ImsNetworkInterface getCurrentNetworkInterface() {
        return connectionManager.getCurrentNetworkInterface();
    }

    /**
     * Is connected to a Wi-Fi access
     *
     * @return Boolean
     */
    public boolean isConnectedToWifiAccess() {
        return connectionManager.isConnectedToWifi();
    }

    /**
     * Is connected to a mobile access
     *
     * @return Boolean
     */
    public boolean isConnectedToMobileAccess() {
        return connectionManager.isConnectedToMobile();
    }

    /**
     * Returns the ImsConnectionManager
     *
     * @return ImsConnectionManager
     */
    public ImsConnectionManager getImsConnectionManager(){
        return connectionManager;
    }

    /**
     * Start the IMS module
     */
    public void start() {
        if (logger.isActivated()) {
            logger.info("Start the IMS module");
        }

        // Start the service dispatcher
        new Thread(serviceDispatcher).start();

        if (logger.isActivated()) {
            logger.info("IMS module is started");
        }
    }

    /**
     * Stop the IMS module
     */
    public void stop() {
        if (logger.isActivated()) {
            logger.info("Stop the IMS module");
        }

        // Terminate the subscription manager
        subscriptionManager.terminate();

        // Terminate the connection manager
        connectionManager.terminate();

        // Terminate the service dispatcher
        serviceDispatcher.terminate();

        if (logger.isActivated()) {
            logger.info("IMS module has been stopped");
        }
    }

    /**
     * Start IMS services
     */
    public void startImsServices() {
        // Start each services
        for(int i=0; i < services.length; i++) {
            if (services[i] != null) {
                if (logger.isActivated()) {
                    logger.info("Start IMS service: " + services[i].getClass().getName());
                }
                services[i].start();
            }
        }

    }

    /**
     * Stop IMS services
     */
    public void stopImsServices() {
        // Abort all pending sessions
        abortAllSessions();

        // Stop each services
        for(int i=0; i < services.length; i++) {
            if (services[i] != null) {
                if (logger.isActivated()) {
                    logger.info("Stop IMS service: " + services[i].getClass().getName());
                }
                services[i].stop();
            }
        }

    }

    /**
     * Check IMS services
     */
    public void checkImsServices() {
        for(int i=0; i < services.length; i++) {
            if (services[i] != null && services[i].isActivated()) {
                if (logger.isActivated()) {
                    logger.info("Check IMS service: " + services[i].getClass().getName());
                }
                services[i].check();
            }
        }
    }

    /**
     * Returns the IMS service
     *
     * @param id Id of the IMS service
     * @return IMS service
     */
    public ImsService getImsService(int id) {
        return services[id];
    }

    /**
     * Returns the IMS services
     *
     * @return Table of IMS service
     */
    public ImsService[] getImsServices() {
        return services;
    }

    /**
     * Returns the terms & conditions service
     *
     * @return Terms & conditions service
     */
    public TermsConditionsService getTermsConditionsService() {
        return (TermsConditionsService)services[ImsService.TERMS_SERVICE];
    }

    /**
     * Returns the capability service
     *
     * @return Capability service
     */
    public CapabilityService getCapabilityService() {
        return (CapabilityService)services[ImsService.CAPABILITY_SERVICE];
    }

    /**
     * Returns the presence service
     *
     * @return Presence service
     */
    public PresenceService getPresenceService() {
        return (PresenceService)services[ImsService.PRESENCE_SERVICE];
    }

    /**
     * Returns the SIP service
     *
     * @return SIP service
     */
    public SipService getSipService() {
        return (SipService)services[ImsService.SIP_SERVICE];
    }

    /**
     * Return the core instance
     *
     * @return Core instance
     */
    public Core getCore() {
        return core;
    }

    /**
     * Return the core listener
     *
     * @return Core listener
     */
    public CoreListener getCoreListener() {
        return core.getListener();
    }

    /**
     * Receive SIP request
     *
     * @param request SIP request
     */
    public void receiveSipRequest(SipRequest request) {
        // Post the incoming request to the service dispatcher
        serviceDispatcher.postSipRequest(request);
    }

    /**
     * Abort all sessions
     */
    public void abortAllSessions() {
        try {
            if (logger.isActivated()) {
                logger.debug("Abort all pending sessions");
            }
            ImsService[] services = getImsServices();
            for (int i = 0; i < services.length; i++) {
                ImsService service = services[i];
                if (service == null)
                    continue;
                for (Enumeration<ImsServiceSession> e = service.getSessions(); e.hasMoreElements();) {
                    ImsServiceSession session = (ImsServiceSession) e.nextElement();
                    if (logger.isActivated()) {
                        logger.debug("Abort session " + session.getSessionID());
                    }
                    session.abortSession(ImsServiceSession.TERMINATION_BY_SYSTEM);
                }
            }
        } catch (Exception e) {
            // Aborting sessions may fail (e.g. due to ConcurrentModificationException)
            // we don't want the whole shutdown to be interrupted just because of this
            if (logger.isActivated()) {
                logger.error("Aborting all sessions failed", e);
            }
        }
    }

    /**
     * Check whether ImsModule instantiation has finished
     *
     * @return true if ImsModule is completely initialized
     */
    public boolean isReady(){
        return isReady;
    }

    public boolean isProvisioned() {
        return mIsProvisioned;
    }

    public static enum EabActions {
        NONE, PUBLISH, CAPABILITY, AVAILABILITY
    }

    public void onForbiddenReceived(EabActions action, String reason) {
        logger.debug("onForbiddenReceived reason: " + reason);

        for(int i=0; i < services.length; i++) {
            if (services[i] != null) {
                services[i].onForbiddenReceived(action, reason);
            }
        }

        if (reason.indexOf("User Not Registered") > -1) {
            Intent intent = new Intent(RcsUaAdapter.ACTION_IMS_RECOVER_REGISTER);
            intent.putExtra(RcsUaAdapter.EXTRA_SLOT_ID, mSlotId);
            LocalBroadcastManager.getInstance(AndroidFactory.getApplicationContext())
                    .sendBroadcast(intent);
        }
    }

    public void onNotProvisionedReceived() {
        logger.debug("onNotProvisionedReceived");

        mIsProvisioned = false;

        for(int i=0; i < services.length; i++) {
            if (services[i] != null) {
                services[i].onNotProvisionedReceived();
            }
        }
    }

    public int getSlotId() {
        return mSlotId;
    }
}
