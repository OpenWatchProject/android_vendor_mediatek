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

package com.mediatek.presence.service.api;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import com.gsma.services.rcs.Build;
import com.gsma.services.rcs.IJoynServiceRegistrationListener;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.ICapabilitiesListener;
import com.gsma.services.rcs.capability.ICapabilityService;
import com.gsma.services.rcs.contact.ContactId;

import android.os.Binder;
import android.os.RemoteCallbackList;
import android.text.TextUtils;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.content.Context;

import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.ims.service.capability.CapabilityService;
import com.mediatek.presence.core.ims.service.presence.PresenceService;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.platform.AndroidFactory;

/**
 * Capability service API implementation
 *
 * @author Jean-Marc AUFFRET
 */
public class CapabilityServiceImpl extends ICapabilityService.Stub {
    /**
     * List of service event listeners
     */
    private RemoteCallbackList<IJoynServiceRegistrationListener> serviceListeners = new RemoteCallbackList<IJoynServiceRegistrationListener>();

    /**
     * List of service event listeners for each slot
     */
    private ArrayList<RemoteCallbackList<IJoynServiceRegistrationListener>> serviceListenerSlotList = null;

    /**
     * List of capabilities listeners
     */
    private RemoteCallbackList<ICapabilitiesListener> capabilitiesListeners = new RemoteCallbackList<ICapabilitiesListener>();

    /**
     * List of listeners per contact
     */
    private Hashtable<String, RemoteCallbackList<ICapabilitiesListener>> contactCapalitiesListeners = new Hashtable<String, RemoteCallbackList<ICapabilitiesListener>>();

    /**
     * Lock used for synchronization
     */
    private Object lock = new Object();

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     */
    public CapabilityServiceImpl() {
        if (logger.isActivated()) {
            logger.info("Capability service API is loaded");
        }

        initListenerSlotList();
    }

    /**
     * Close API
     */
    public void close() {
        if (logger.isActivated()) {
            logger.info("Capability service API is closed");
        }
    }

    /**
     * Returns true if the service is registered to the platform, else returns false
     *
     * @param slotId Slot index
     * @return Returns true if registered else returns false
     */
    public boolean isServiceRegistered(int slotId) {
        return ServerApiUtils.isImsConnected(slotId);
    }

    /**
     * Registers a listener on service registration events
     *
     * @param slotId Slot index
     * @param listener Service registration listener
     */
    public void addServiceRegistrationListener(int slotId, IJoynServiceRegistrationListener listener) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Add a service listener");
            }

            serviceListenerSlotList.get(slotId).register(listener);
        }
    }

    /**
     * Unregisters a listener on service registration events
     *
     * @param slotId Slot index
     * @param listener Service registration listener
     */
    public void removeServiceRegistrationListener(int slotId, IJoynServiceRegistrationListener listener) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Remove a service listener");
            }

            serviceListenerSlotList.get(slotId).unregister(listener);
        }
    }

    /**
     * Receive registration event
     *
     * @param slotId Slot index
     * @param state Registration state
     */
    public void notifyRegistrationEvent(int slotId, boolean state) {
        // Notify listeners
        synchronized(lock) {
            final int N = serviceListenerSlotList.get(slotId).beginBroadcast();
            for (int i = 0; i < N; i++) {
                try {
                    if (state) {
                        serviceListenerSlotList.get(slotId)
                                .getBroadcastItem(i).onServiceRegistered();
                    } else {
                        serviceListenerSlotList.get(slotId)
                                .getBroadcastItem(i).onServiceUnregistered();
                    }
                } catch(Exception e) {
                    if (logger.isActivated()) {
                        logger.error("Can't notify listener", e);
                    }
                }
            }
            serviceListenerSlotList.get(slotId).finishBroadcast();
        }
    }

    /**
     * Returns the capabilities supported by the local end user. The supported
     * capabilities are fixed by the MNO and read during the provisioning.
     *
     * @param slotId Slot index
     * @return Capabilities
     */
    public Capabilities getMyCapabilities(int slotId) {
        com.mediatek.presence.core.ims.service.capability.Capabilities capabilities =
                RcsSettingsManager.getRcsSettingsInstance(slotId).getMyCapabilities();
        Set<String> exts = new HashSet<String>(capabilities.getSupportedExtensions());
        Capabilities caps = new Capabilities(capabilities.isImageSharingSupported(),
                capabilities.isVideoSharingSupported(),
                capabilities.isImSessionSupported(),
                capabilities.isFileTransferSupported(),
                capabilities.isGeolocationPushSupported(),
                capabilities.isIPVoiceCallSupported(),
                capabilities.isIPVideoCallSupported(),
                exts,
                capabilities.isSipAutomata(),
                capabilities.isFileTransferHttpSupported(),
                capabilities.isRCSContact(),
                capabilities.isIntegratedMessagingMode(),
                capabilities.isCsVideoSupported(),
                capabilities.isBurnAfterRead()
                );
        caps.setIR94_VoiceCall(capabilities.isIR94_VoiceCallSupported());
        caps.setIR94_VideoCall(capabilities.isIR94_VideoCallSupported());
        caps.setIR94_DuplexMode(capabilities.isIR94_DuplexModeSupported());
        caps.setPresenceDiscoverySupport(capabilities.isPresenceDiscoverySupported());
        caps.setSocialPresenceSupport(capabilities.isSocialPresenceSupported());
        caps.setFileTransferThumbnailSupport(capabilities.isFileTransferThumbnailSupported());
        caps.setFileTransferStoreForwardSupport(capabilities.isFileTransferStoreForwardSupported());
        caps.setGroupChatStoreForwardSupport(capabilities.isGroupChatStoreForwardSupported());
        caps.setStandaloneMsgSupport(capabilities.isStandaloneMsgSupport());
        caps.setTimestamp(capabilities.getTimestamp());
        caps.setNoResourceContactSupport(capabilities.isNoResourceContact());
        caps.setChatbotSupport(capabilities.isChatbotSupported());
        caps.setChatbotSaSupport(capabilities.isChatbotSaSupported());
        caps.setCallcomposerSupport(capabilities.isCallcomposerSupported());

        return caps;

    }

    /**
     * Returns the capabilities of a given contact from the local database. This
     * method doesnt request any network update to the remote contact. The parameter
     * contact supports the following formats: MSISDN in national or international
     * format, SIP address, SIP-URI or Tel-URI. If the format of the contact is not
     * supported an exception is thrown.
     *
     * @param contact Contact
     * @return Capabilities
     */
    public Capabilities getContactCapabilities (ContactId contactId) throws ServerApiException {
        int subId = SubscriptionManager.getDefaultSubscriptionId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            slotId = 0;
        }
        return getContactCapabilities(slotId, contactId);
    }

    public Capabilities getContactCapabilities(
            int slotId, ContactId contactId) throws ServerApiException {
        int rcsExpiry  = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .getCapabilityExpiryTimeout();
        int nonRcsExpiry  = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .getNonRcsCapabilityExpiryTimeout();

        String contact = getContactNumber(contactId);
        if (logger.isActivated()) {
            logger.info("Get capabilities for contact " + contact + " CPID="
                    + Binder.getCallingPid());
        }
        if (contact == null) {
            return null;
        }

        // Check if if it is a valid RCS number
        if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug(contact + " is not a valid joyn number");
            }
            return null;
        }

        // check service state
        boolean serviceActive = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .isServiceActivated();
        // Read capabilities in the local database
        com.mediatek.presence.core.ims.service.capability.Capabilities capabilities = ContactsManager
                .getInstance().getContactCapabilities(contact);
        if (serviceActive && capabilities != null) {

            /// Add for OP07, request capability if time expired. @{
            if (SimUtils.isAttSimCard(slotId)) {
                long delta = (System.currentTimeMillis() - capabilities.getTimestamp()) / 1000;
                boolean isRCSContact = capabilities.isRCSContact();
                boolean expired = (isRCSContact &&
                        delta >= rcsExpiry)
                        || (!isRCSContact &&
                        delta >= nonRcsExpiry);
                if (expired) {
                    if (logger.isActivated()) {
                        logger.info("requery the capability, is RCS: " + isRCSContact
                                + ", delta: " + delta
                                + ", expired: " + rcsExpiry);
                    }
                    requestContactCapabilities(slotId, contactId);
                }
            }
            /// @}

            Set<String> exts = new HashSet<String>(capabilities.getSupportedExtensions());

            Capabilities contactCapabilities =  new Capabilities(
                    capabilities.isImageSharingSupported(),
                    capabilities.isVideoSharingSupported(),
                    capabilities.isImSessionSupported(),
                    capabilities.isFileTransferSupported(),
                    capabilities.isGeolocationPushSupported(),
                    capabilities.isIPVoiceCallSupported(),
                    capabilities.isIPVideoCallSupported(),
                    exts,
                    capabilities.isSipAutomata(),
                    capabilities.isFileTransferHttpSupported(),
                    capabilities.isRCSContact(),
                    capabilities.isIntegratedMessagingMode(),
                    capabilities.isCsVideoSupported(),
                    capabilities.isBurnAfterRead()
                    );
            contactCapabilities.setIR94_VoiceCall(capabilities.isIR94_VoiceCallSupported());
            contactCapabilities.setIR94_VideoCall(capabilities.isIR94_VideoCallSupported());
            contactCapabilities.setIR94_DuplexMode(capabilities.isIR94_DuplexModeSupported());
            contactCapabilities.setStandaloneMsgSupport(capabilities.isStandaloneMsgSupport());
            contactCapabilities.setChatbotSupport(capabilities.isChatbotSupported());
            contactCapabilities.setChatbotSaSupport(capabilities.isChatbotSaSupported());
            contactCapabilities.setCallcomposerSupport(capabilities.isCallcomposerSupported());
            logger.info("Get capabilities, Msg: " + capabilities.isStandaloneMsgSupport()
                    + ", IR94: " + capabilities.isIR94_VideoCallSupported());

            return contactCapabilities;
        } else {
            return null;
        }
    }

    /**
     * Requests capabilities to a remote contact. This method initiates in background
     * a new capability request to the remote contact by sending a SIP OPTIONS. The
     * result of the capability request is sent asynchronously via callback method of
     * the capabilities listener. A capability refresh is only sent if the timestamp
     * associated to the capability has expired (the expiration value is fixed via MNO
     * provisioning). The parameter contact supports the following formats: MSISDN in
     * national or international format, SIP address, SIP-URI or Tel-URI. If the format
     * of the contact is not supported an exception is thrown. The result of the
     * capability refresh request is provided to all the clients that have registered
     * the listener for this event.
     *
     * @param contact Contact
     * @throws ServerApiException
     */
    public void requestContactCapabilities(final ContactId contactId) throws ServerApiException {
        int subId = SubscriptionManager.getDefaultSubscriptionId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            slotId = 0;
        }
        requestContactCapabilities(slotId, contactId);
    }

    public void requestContactCapabilities(int slotId,
            final ContactId contactId) throws ServerApiException {
        String contact = getContactNumber(contactId);
        if (logger.isActivated()) {
            logger.info("Request capabilities for contact " + contact + " CPID="
                    + Binder.getCallingPid());
        }
        if (contact == null) {
            return;
        }

        // check service state
        boolean serviceActive = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .isServiceActivated();
        if (!serviceActive) {
            return;
        }
        // Test IMS connection
        ServerApiUtils.testIms(slotId);

        // Request contact capabilities
        try {
            Thread t = new Thread() {
                public void run() {
                    CapabilityService capabilityService =
                            Core.getInstance().getImsModule(slotId).getCapabilityService();
                    capabilityService.requestContactCapabilities(contact);
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    public void requestContactAvailability(
            final ContactId contactId) throws ServerApiException {
        int subId = SubscriptionManager.getDefaultSubscriptionId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            slotId = 0;
        }
        requestContactAvailability(slotId, contactId);
    }
    public void requestContactAvailability(
            int slotId, final ContactId contactId) throws ServerApiException {
        String contact = getContactNumber(contactId);
        if (logger.isActivated()) {
            logger.info("Request availability for contact " + contact + " CPID="
                    + Binder.getCallingPid());
        }
        if (contact == null) {
            return;
        }

        com.mediatek.presence.core.ims.service.capability.Capabilities capabilities = ContactsManager
                .getInstance().getContactCapabilities(contact);
        if (capabilities == null) {
            //VZ_REQ_RCSVOLTE_4074
            logger.info("contact capabilities is null, skip availability check");
            return;
        }

        // check service state
        boolean serviceActive = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .isServiceActivated();
        if (!serviceActive) {
            return;
        }
        // Test IMS connection
        ServerApiUtils.testIms(slotId);

        // Request contact availability
        try {
            Thread t = new Thread() {
                public void run() {
                    CapabilityService capabilityService =
                            Core.getInstance().getImsModule(slotId).getCapabilityService();
                    capabilityService.requestContactAvailability(contact);
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }


    /**
     * Receive capabilities from a contact
     *
     * @param contact Contact
     * @param capabilities Capabilities
     */
    public void receiveCapabilities(String contact, com.mediatek.presence.core.ims.service.capability.Capabilities capabilities) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Receive capabilities for " + contact);
            }

            // Create capabilities instance
            Set<String> exts = new HashSet<String>(capabilities.getSupportedExtensions());
            Capabilities c = new Capabilities(
                    capabilities.isImageSharingSupported(),
                    capabilities.isVideoSharingSupported(),
                    capabilities.isImSessionSupported(),
                    capabilities.isFileTransferSupported(),
                    capabilities.isGeolocationPushSupported(),
                    capabilities.isIPVoiceCallSupported(),
                    capabilities.isIPVideoCallSupported(),
                    exts,
                    capabilities.isSipAutomata(),
                    capabilities.isFileTransferHttpSupported(),
                    capabilities.isRCSContact(),
                    capabilities.isIntegratedMessagingMode(),
                    capabilities.isCsVideoSupported(),
                    capabilities.isBurnAfterRead()
                    );

            c.setIR94_VoiceCall(capabilities.isIR94_VoiceCallSupported());
            c.setIR94_VideoCall(capabilities.isIR94_VideoCallSupported());
            c.setIR94_DuplexMode(capabilities.isIR94_DuplexModeSupported());
            c.setChatbotSupport(capabilities.isChatbotSupported());
            c.setChatbotSaSupport(capabilities.isChatbotSaSupported());
            c.setCallcomposerSupport(capabilities.isCallcomposerSupported());

            // Notify capabilities listeners
            notifyListeners(contact, c, capabilitiesListeners);

            // Notify capabilities listeners for a given contact
            RemoteCallbackList<ICapabilitiesListener> listeners = contactCapalitiesListeners.get(contact);
            if (listeners != null) {
                notifyListeners(contact, c, listeners);
            }
        }
    }

    /**
     * Notify listeners
     *
     * @param capabilities Capabilities
     * @param listeners Listeners
     */
    private void notifyListeners(String contact, Capabilities capabilities, RemoteCallbackList<ICapabilitiesListener> listeners) {
        final int N = listeners.beginBroadcast();
        for (int i=0; i < N; i++) {
            try {
                listeners.getBroadcastItem(i).onCapabilitiesReceived(contact, capabilities);
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't notify listener", e);
                }
            }
        }
        listeners.finishBroadcast();
    }

    /**
     * Requests capabilities for all contacts existing in the local address book. This
     * method initiates in background new capability requests for each contact of the
     * address book by sending SIP OPTIONS. The result of a capability request is sent
     * asynchronously via callback method of the capabilities listener. A capability
     * refresh is only sent if the timestamp associated to the capability has expired
     * (the expiration value is fixed via MNO provisioning). The result of the capability
     * refresh request is provided to all the clients that have registered the listener
     * for this event.
     *
     * @param slotId Slot index
     * @throws ServerApiException
     */
    public void requestAllContactsCapabilities() throws ServerApiException {
        int subId = SubscriptionManager.getDefaultSubscriptionId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            slotId = 0;
        }
        requestAllContactsCapabilities(slotId);
    }

    public void requestAllContactsCapabilities(int slotId) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Request all contacts capabilities" + " CPID=" + Binder.getCallingPid());
        }

        // check service state
        boolean serviceActive = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .isServiceActivated();
        if (!serviceActive) {
            return;
        }

        // Test IMS connection
        ServerApiUtils.testIms(slotId);

        // Request all contacts capabilities
        try {
            Thread t = new Thread() {
                public void run() {
                    CapabilityService capabilityService =
                            Core.getInstance().getImsModule(slotId).getCapabilityService();
                    List<String> contactList = ContactsManager.getInstance().getAllContacts();
                    capabilityService.requestContactsCapabilities(contactList);
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    public boolean publishMyCap(int slotId,
            boolean isModify, Capabilities caps)  throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("publishMyCap: " + caps + " CPID="
                    + Binder.getCallingPid());
        }

        // check service state
        boolean serviceActive = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .isServiceActivated();
        if (!serviceActive) {
            return false;
        }

        // Request contact capabilities
        try {
            Thread t = new Thread() {
                public void run() {
                    PresenceService presenceService =
                            Core.getInstance().getImsModule(slotId).getPresenceService();
                    com.mediatek.presence.core.ims.service.capability.Capabilities presCaps =
                            new com.mediatek.presence.core.ims.service.capability.Capabilities();
                    //Capabilities fromat translate
                    presCaps.setImageSharingSupport(caps.isImageSharingSupported());
                    presCaps.setVideoSharingSupport(caps.isVideoSharingSupported());
                    presCaps.setIPVoiceCallSupport(caps.isIPVoiceCallSupported());
                    presCaps.setIPVideoCallSupport(caps.isIPVideoCallSupported());
                    presCaps.setImSessionSupport(caps.isImSessionSupported());
                    presCaps.setFileTransferSupport(caps.isFileTransferSupported());
                    presCaps.setCsVideoSupport(caps.isCsVideoSupported());
                    presCaps.setFileTransferThumbnailSupport(caps.isFileTransferThumbnailSupported());
                    presCaps.setFileTransferHttpSupport(caps.isFileTransferHttpSupported());
                    presCaps.setGeolocationPushSupport(caps.isGeolocPushSupported());
                    presCaps.setStandaloneMsgSupport(caps.isStandaloneMsgSupported());
                    presCaps.setChatbotSupport(caps.isChatbotSupported());
                    presCaps.setChatbotSaSupport(caps.isChatbotSaSupported());
                    presCaps.setCallcomposerSupport(caps.isCallcomposerSupported());
                    presenceService.updateRcsCapabilities(isModify, presCaps);
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
        return true;
    }

    /**
     * Registers a capabilities listener on any contact
     *
     * @param listener Capabilities listener
     */
    public void addCapabilitiesListener(ICapabilitiesListener listener) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Add a listener: " + listener +" CPID=" + Binder.getCallingPid());
            }

            capabilitiesListeners.register(listener);
        }
    }

    /**
     * Unregisters a capabilities listener
     *
     * @param listener Capabilities listener
     */
    public void removeCapabilitiesListener(ICapabilitiesListener listener) {
        synchronized(lock) {
            if (logger.isActivated()) {
                logger.info("Remove a listener: " + listener +" CPID=" + Binder.getCallingPid());
            }

            capabilitiesListeners.unregister(listener);
        }
    }

    /**
     * Registers a listener for receiving capabilities of a given contact
     *
     * @param contact Contact
     * @param listener Capabilities listener
     */
    public void addContactCapabilitiesListener(ContactId contactId, ICapabilitiesListener listener) {
        synchronized(lock) {
            String contact = getContactNumber(contactId);
            if (logger.isActivated()) {
                logger.info("Add a listener for contact " + contact);
            }
            if (contact == null) {
                return;
            }

            RemoteCallbackList<ICapabilitiesListener> listeners = contactCapalitiesListeners
                    .get(contact);
            if (listeners == null) {
                listeners = new RemoteCallbackList<ICapabilitiesListener>();
                contactCapalitiesListeners.put(contact, listeners);
            }
            listeners.register(listener);
        }
    }

    /**
     * Unregisters a listener of capabilities for a given contact
     *
     * @param contact Contact
     * @param listener Capabilities listener
     */
    public void removeContactCapabilitiesListener(ContactId contactId, ICapabilitiesListener listener) {
        synchronized (lock) {
            String contact = getContactNumber(contactId);
            if (logger.isActivated()) {
                logger.info("Remove a listener for contact " + contact);
            }
            if (contact == null) {
                return;
            }

            RemoteCallbackList<ICapabilitiesListener> listeners = contactCapalitiesListeners
                    .get(contact);
            if (listeners != null) {
                listeners.unregister(listener);
            }
        }
    }

    /**
     * Returns service version
     *
     * @return Version
     * @see Build.VERSION_CODES
     * @throws ServerApiException
     */
    public int getServiceVersion() throws ServerApiException {
        return Build.API_VERSION;
    }

    /**
     * MTK Added function
     * Requests capabilities to a remote contact.
     *
     * @param contact Contact
     * @param force force send request to server if true
     * @throws ServerApiException
     */
    public void forceRequestContactCapabilities(
            final ContactId contactId) throws ServerApiException {
        int subId = SubscriptionManager.getDefaultSubscriptionId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            slotId = 0;
        }
        forceRequestContactCapabilities(slotId, contactId);
    }

    public void forceRequestContactCapabilities(
            int slotId, final ContactId contactId) throws ServerApiException {
        final String contact = getContactNumber(contactId);
        if (logger.isActivated()) {
            logger.info("Force request capabilities for contact " + contact + " CPID="
                    + Binder.getCallingPid());
        }
        if (contact == null) {
            return;
        }

        // Check if if it is a valid RCS number
        if (!ContactsManager.getInstance().isRcsValidNumber(contact)) {
            if (logger.isActivated()) {
                logger.debug(contact + " is not a valid joyn number");
            }
            return;
        }

        // check service state
        boolean serviceActive = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .isServiceActivated();
        if (!serviceActive) {
            return;
        }

        // Test IMS connection
        ServerApiUtils.testIms(slotId);

        // Request contact capabilities
        try {
            Thread t = new Thread() {
                public void run() {
                    CapabilityService capabilityService =
                            Core.getInstance().getImsModule(slotId).getCapabilityService();
                    capabilityService.forceRequestContactCapabilities(contact);
                }
            };
            t.start();
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
            throw new ServerApiException(e.getMessage());
        }
    }

    /**
     * Requests capabilities for a group of remote contacts.
     *
     * @param contact Contact
     * @throws ServerApiException
     */
    public void requestContactsCapabilities(
            final List<ContactId> srcContacts) throws ServerApiException {
        int subId = SubscriptionManager.getDefaultSubscriptionId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            slotId = 0;
        }
        requestContactsCapabilities(slotId, srcContacts);
    }
    public void requestContactsCapabilities(
            int slotId, final List<ContactId> srcContacts) throws ServerApiException {
        if (logger.isActivated()) {
            logger.info("Request capabilities for contact " + srcContacts + " CPID="
                    + Binder.getCallingPid());
        }

        List<String> contacts = new ArrayList();
        for (int i = 0; i < srcContacts.size(); i++) {
            contacts.add(srcContacts.get(i).toString());
        }
        doRequestContactsCapabilities(slotId, contacts);
    }

    private void doRequestContactsCapabilities(
            int slotId, final List<String> srcContacts) throws ServerApiException {
            // check service state
            boolean serviceActive = RcsSettingsManager.getRcsSettingsInstance(slotId)
                    .isServiceActivated();
            if (!serviceActive) {
                return;
            }

            // Test IMS connection
            ServerApiUtils.testIms(slotId);

            // Request contact capabilities
            try {
                Thread t = new Thread() {
                    public void run() {
                        CapabilityService capabilityService =
                                Core.getInstance().getImsModule(slotId).getCapabilityService();
                        capabilityService.requestContactsCapabilities(srcContacts);
                    }
                };
                t.start();
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Unexpected error", e);
                }
                throw new ServerApiException(e.getMessage());
            }
    }

    public HashMap<String, Capabilities> query (
            int slotId, final String numbers) throws ServerApiException{
        if (TextUtils.isEmpty(numbers)) {
            return null;
        }

        int rcsExpiry  = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .getCapabilityExpiryTimeout();
        int nonRcsExpiry  = RcsSettingsManager.getRcsSettingsInstance(slotId)
                .getNonRcsCapabilityExpiryTimeout();

        HashMap<String, Capabilities> capMap = new HashMap<String, Capabilities>();
        ArrayList<String> numList = getNumbers(numbers);
        ArrayList<String> requsetNumbers = new ArrayList<String>();
        for (String number : numList) {
            // Read capabilities in the local database
            com.mediatek.presence.core.ims.service.capability.Capabilities capabilities = ContactsManager
                    .getInstance().getContactCapabilities(number);
            Capabilities contactCapabilities = null;
            if (capabilities != null) {
                Set<String> exts = new HashSet<String>(capabilities.getSupportedExtensions());
                contactCapabilities =  new Capabilities(
                        capabilities.isImageSharingSupported(),
                        capabilities.isVideoSharingSupported(),
                        capabilities.isImSessionSupported(),
                        capabilities.isFileTransferSupported(),
                        capabilities.isGeolocationPushSupported(),
                        capabilities.isIPVoiceCallSupported(),
                        capabilities.isIPVideoCallSupported(),
                        exts,
                        capabilities.isSipAutomata(),
                        capabilities.isFileTransferHttpSupported(),
                        capabilities.isRCSContact(),
                        capabilities.isIntegratedMessagingMode(),
                        capabilities.isCsVideoSupported(),
                        capabilities.isBurnAfterRead()
                        );
                contactCapabilities.setIR94_VoiceCall(capabilities.isIR94_VoiceCallSupported());
                contactCapabilities.setIR94_VideoCall(capabilities.isIR94_VideoCallSupported());
                contactCapabilities.setIR94_DuplexMode(capabilities.isIR94_DuplexModeSupported());
                contactCapabilities.setChatbotSupport(capabilities.isChatbotSupported());
                contactCapabilities.setChatbotSaSupport(capabilities.isChatbotSaSupported());
                contactCapabilities.setCallcomposerSupport(capabilities.isCallcomposerSupported());
            }
            capMap.put(number, contactCapabilities);

            // check whether expired
            long delta = (System.currentTimeMillis() - capabilities.getTimestamp()) / 1000;
            boolean expired = (capabilities.isRCSContact() &&
                    delta >= rcsExpiry)
                    || (!capabilities.isRCSContact() &&
                    delta >= nonRcsExpiry);
            if (capabilities == null || expired) {
                requsetNumbers.add(number);
            }
        }
        if (requsetNumbers.size() == 1) {
            requestContactCapabilities(slotId, new ContactId(requsetNumbers.get(0)));
        } else if (requsetNumbers.size() > 1) {
            doRequestContactsCapabilities(slotId, requsetNumbers);
        }
        return capMap;
    }

    public void updateAfterSuccessfulServiceAttempt(
            int slotId, final String numbers) throws ServerApiException{
        doRequestContactsCapabilities(slotId, getNumbers(numbers));
    }

    public ArrayList<Capabilities> realTimeQuery(final String numbers) {
        return null;
    }

    public static ArrayList<String> getNumbers(String numbers) {
        String numString = numbers;
        ArrayList<String> numList = new ArrayList<String>();
        while (numString != null && !numString.equals("")) {
            if (numString.contains(",")) {
                numList.add(numString.substring(0, numString.indexOf(",")).trim());
                numString = numString.substring(numString.indexOf(",") + 1);
            } else {
                numList.add(numString.trim());
                numString = null;
            }
        };
        return numList;
    }

    private String getContactNumber(ContactId contact) {
        String number = null;
        if (contact != null) {
            number = contact.toString();
        }
        return number;
    }

    private void initListenerSlotList() {
        Context context = AndroidFactory.getApplicationContext();
        TelephonyManager tm = (TelephonyManager) context.getSystemService(
                Context.TELEPHONY_SERVICE);
        if (tm == null) {
            logger.error("Core init fail. TelephonyManager is null");
        }
        int simCount = tm.getSimCount();
        serviceListenerSlotList =
                new ArrayList<RemoteCallbackList<IJoynServiceRegistrationListener>>(simCount);
        for (int i = 0; i < simCount; ++i) {
            serviceListenerSlotList.add(new RemoteCallbackList<IJoynServiceRegistrationListener>());
        }
        
    }
}
