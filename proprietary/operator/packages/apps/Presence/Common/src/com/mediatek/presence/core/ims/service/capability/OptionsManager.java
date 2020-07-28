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
package com.mediatek.presence.core.ims.service.capability;

import java.util.HashSet;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipMessageFactory;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipDialogPath;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.service.api.OptionsServiceImpl;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsData;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.PhoneUtils;

/**
 * Capability discovery manager using options procedure
 *  
 * @author jexa7410
 */
public class OptionsManager implements DiscoveryManager {
    /**
     * Max number of threads for background processing
     */
    private final static int MAX_PROCESSING_THREADS = 15;
    
    /**
     * IMS module
     */
    private ImsModule imsModule;
    
    /**
     * Thread pool to request capabilities in background
     */
    private ExecutorService threadPool;

    /**
     * The logger
     */
    private Logger logger = null;

    private int mSlotId = 0;

    /**
     * Constructor
     * 
     * @param parent IMS module
     */
    public OptionsManager(ImsModule parent) {
        this.imsModule = parent;
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger("[" + mSlotId + "] OptionsManager");
    }

    /**
     * Start the manager
     */
    public void start() {
        threadPool = Executors.newFixedThreadPool(MAX_PROCESSING_THREADS);
    }

    /**
     * Stop the manager
     */
    public void stop() {
        try {
            threadPool.shutdown();
        } catch (SecurityException e) {
            if (logger.isActivated()) {
                logger.error("Could not stop all threads");
            }
        }
    }
    
    /**
     * Request contact capabilities
     * 
     * @param contact Remote contact
     * @return Returns true if success
     */
    public boolean requestCapabilities(String contact) {
        if (logger.isActivated()) {
            logger.debug("Request capabilities in background for " + contact);
        }
        
        // Update capability timestamp
        ContactsManager.getInstance().setContactCapabilitiesTimestamp(contact, System.currentTimeMillis());
        
        // Start request in background
        try {
            boolean richcall = false;//imsModule.getCallManager().isRichcallSupportedWith(contact);
            boolean ipcall = false;//imsModule.getIPCallService().isCallConnectedWith(contact);
            
            if (logger.isActivated()) {
                logger.debug("richcall :  " + richcall + " ; ipcall : "+ ipcall);
            }
            OptionsRequestTask task = new OptionsRequestTask(
                    imsModule, contact, CapabilityUtils.getSupportedFeatureTags(mSlotId, richcall, ipcall));
            threadPool.submit(task);
            return true;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't submit task", e);
            }
            return false;
        }
    }

    /**
     * Request contact capability with a requestId (AOSP procedure)
     *
     * @param contact contact
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public boolean requestCapabilities(String contact, int requestId) {
        if (logger.isActivated()) {
            logger.debug("Request capabilities in background for " + contact);
        }
        // Update capability timestamp
        ContactsManager.getInstance().setContactCapabilitiesTimestamp(contact,
                System.currentTimeMillis());
        // Start request in background
        try {
            boolean richcall = false;//imsModule.getCallManager().isRichcallSupportedWith(contact);
            boolean ipcall = false;//imsModule.getIPCallService().isCallConnectedWith(contact);
            if (logger.isActivated()) {
                logger.debug("richcall :  " + richcall + " ; ipcall : "+ ipcall);
            }
            OptionsRequestTask task = new OptionsRequestTask(imsModule, contact,
                    CapabilityUtils.getSupportedFeatureTags(mSlotId, richcall, ipcall), requestId);
            threadPool.submit(task);
            return true;
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't submit task", e);
            }
            return false;
        }
    }

    /**
     * Request capabilities for a list of contacts
     *
     * @param contactList Contact list
     */
    public void requestCapabilities(List<String> contactList) {
        // Remove duplicate values
        HashSet<String> setContacts = new HashSet<String>(contactList);
        if (logger.isActivated()) {
            logger.debug("Request capabilities for " + setContacts.size() + " contacts");
        }

        for (String contact : setContacts) {
            if (!requestCapabilities(contact)) {
                if (logger.isActivated()) {
                    logger.debug("Processing has been stopped");
                }
                break;
            }
        }
    }

    /**
     * Request capabilities for a list of contacts with a requestId (AOSP procedure)
     *
     * @param contactList contactList
     * @param requestId When device receives SIP response, presence statck shall send
     *                  a callback with requestId to client
     */
    public void requestCapabilities(List<String> contactList, int requestId) {
        // Remove duplicate values
        HashSet<String> setContacts = new HashSet<String>(contactList);
        if (logger.isActivated()) {
            logger.debug("Request capabilities for " + setContacts.size() + " contacts");
        }

        for (String contact : setContacts) {
            if (!requestCapabilities(contact, requestId)) {
                if (logger.isActivated()) {
                    logger.debug("Processing has been stopped");
                }
                break;
            }
        }
    }

    /**
     * Receive a capability request (options procedure)
     * 
     * @param options Received options message
     */
    public void receiveCapabilityRequest(SipRequest options) {
        String contact = SipUtils.getAssertedIdentity(options);

        //AOSP Procedure
        OptionsServiceImpl.receiveIncomingOptions(options,
                ContactsManager.getInstance().getContactCapabilities(contact));

        if (logger.isActivated()) {
            logger.debug("OPTIONS request received from " + contact);
        }
        
        try {
            // Create 200 OK response
            String ipAddress = imsModule.getCurrentNetworkInterface().getNetworkAccess().getIpAddress();
            SipResponse resp = SipMessageFactory.create200OkOptionsResponse(options,
                    imsModule.getSipManager().getSipStack().getContact(),
                    CapabilityUtils.getSupportedFeatureTags(mSlotId, false, false),
                    CapabilityUtils.buildSdp(mSlotId, ipAddress, false));

            // Send 200 OK response
            imsModule.getSipManager().sendSipResponse(resp);
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't send 200 OK for OPTIONS", e);
            }
        }

        // Read features tag in the request
        Capabilities capabilities = CapabilityUtils.extractCapabilities(options);

        // Update capabilities in database
        if (capabilities.isImSessionSupported()) {
            // RCS-e contact
            ContactsManager.getInstance().setContactCapabilities(contact, capabilities, ContactInfo.RCS_CAPABLE, ContactInfo.REGISTRATION_STATUS_ONLINE);
        } else {
            // Not a RCS-e contact
            ContactsManager.getInstance().setContactCapabilities(contact, capabilities, ContactInfo.NOT_RCS, ContactInfo.REGISTRATION_STATUS_UNKNOWN);
        }
        
    /*M: TCT GSM IOT patches */    
   // add for update contact capability timeStamp begin
               // Update capability timestamp
               ContactsManager.getInstance().setContactCapabilitiesTimestamp(contact, System.currentTimeMillis());
  
   /*@*/
        
        // Notify listener
        imsModule.getCore().getListener().handleCapabilitiesNotification(contact, capabilities);        
    }

    /**
     * Sets the capabilities information of the self device. (AOSP procedure)
     * @param capInfo, capability information to store.
     */
    public void setMyInfo(Capabilities caps) {
        //AOSP UCE only supports single SIM, so just handle slotId = 0
        int slotId = 0;
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_IM_SESSION,
                Boolean.toString(caps.isImSessionSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER,
                Boolean.toString(caps.isFileTransferSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL,
                Boolean.toString(caps.isFileTransferThumbnailSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_SF,
                Boolean.toString(caps.isFileTransferStoreForwardSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP,
                Boolean.toString(caps.isFileTransferHttpSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_IMAGE_SHARING,
                Boolean.toString(caps.isImageSharingSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE,
                Boolean.toString(caps.isSocialPresenceSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY,
                Boolean.toString(caps.isPresenceDiscoverySupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_IP_VOICE_CALL,
                Boolean.toString(caps.isIR94_VoiceCallSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_IP_VIDEO_CALL,
                Boolean.toString(caps.isIR94_VideoCallSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH,
                Boolean.toString(caps.isGeolocationPushSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH,
                Boolean.toString(caps.isGeolocationPushSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.STANDALONE_MSG_SUPPORT,
                Boolean.toString(caps.isStandaloneMsgSupport()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GROUP_CHAT_SF,
                Boolean.toString(caps.isGroupChatStoreForwardSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_VIDEO_SHARING_CS,
                Boolean.toString(caps.isVideoSharingDuringCallSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_VIDEO_SHARING_OUTSIDE_VOICE_CALL,
                Boolean.toString(caps.isVideoSharingOutsideOfVoiceCallSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PULL_FT,
                Boolean.toString(caps.isGeoLocationPullUsingFileTransferSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PULL,
                Boolean.toString(caps.isGeoLocationPullSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_RCS_VOICE_CALL,
                Boolean.toString(caps.isRcsIpVoiceCallSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_RCS_VIDEO_CALL,
                Boolean.toString(caps.isRcsIpVideoCallSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_RCS_VIDEO_ONLY_CALL,
                Boolean.toString(caps.isRcsIpVideoOnlyCallSupported()));
        StringBuilder extensionsWithSemicolon = new StringBuilder();
        String delimiter = "";
        for (String extension : caps.getSupportedExtensions()) {
            extensionsWithSemicolon.append(delimiter);
            delimiter = ";";
            extensionsWithSemicolon.append(extension);
        }
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_RCS_EXTENSIONS,
                extensionsWithSemicolon.toString());
    }

    /**
     * Requests my capabilities information (AOSP) procedure
     * @return current device capability
     */
     public Capabilities getMyInfo() {
         //AOSP UCE only supports single SIM, so just handle slotId = 0
         int slotId = 0;
         return RcsSettingsManager.getRcsSettingsInstance(slotId).getMyCapabilities();
     }

     /**
      * Requests the capabilities information of specified contacts (AOSP procedure)
      * @param contactUri, URI of the remote entity
      * @param sipResponseCode, SIP response code the UE needs to share to network
      * @param reasonPharse response phrase corresponding to the response code
      * @param myCaps capabilities to share in the resonse to network
      */
     public void responseIncomingOptions(SipRequest options, int sipResponseCode, String reasonPharse,
             Capabilities myCaps) {
         try {
             //AOSP supports single SIM only
             int defaultSlotId = 0;
             String ipAddress = imsModule.getCurrentNetworkInterface().getNetworkAccess().getIpAddress();
             SipResponse resp = SipMessageFactory.createOptionsResponse(options,
                     imsModule.getSipManager().getSipStack().getContact(),
                     CapabilityUtils.getSupportedFeatureTags(myCaps),
                     CapabilityUtils.buildSdp(defaultSlotId, ipAddress, false),
                     sipResponseCode, reasonPharse);

             // Send 200 OK response
             imsModule.getSipManager().sendSipResponse(resp);
         } catch(Exception e) {
             if (logger.isActivated()) {
                 logger.error("Can't send for OPTIONS", e);
             }
         }
     }

}
