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

package com.mediatek.presence.core.ims.service.presence;

import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.telephony.ServiceState;

import com.mediatek.presence.addressbook.AddressBookEventListener;
import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.http.HttpResponse;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapter;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.ImsService;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.core.ims.service.presence.extension.PresenceExtension;
import com.mediatek.presence.core.ims.service.presence.extension.PresenceExtensionListener;
import com.mediatek.presence.core.ims.service.presence.extension.ViLTEExtension;
import com.mediatek.presence.core.ims.service.presence.xdm.XDMContact;
import com.mediatek.presence.core.ims.service.presence.xdm.XdmManager;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.eab.ContactsManagerException;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsData;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.service.LauncherUtils;
import com.mediatek.presence.service.StartService;
import com.mediatek.presence.utils.ContactUtils;
import com.mediatek.presence.utils.DateUtils;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.StringUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.SimUtils;

import com.android.ims.internal.uce.presence.PresServiceInfo;

import java.util.ArrayList;
import java.util.List;

/**
 * Presence service
 *
 * @author Jean-Marc AUFFRET
 */
public class PresenceService extends ImsService implements AddressBookEventListener , PresenceExtensionListener{
    /**
     * Permanent state feature
     */
    public boolean permanentState;

    /**
     * Presence info
     */
    protected PresenceInfo presenceInfo = new PresenceInfo();

    /**
     * Publish manager
     */
    protected PublishManager publisher;

    /**
     * XDM manager
     */
    private XdmManager xdm;

    /**
     * Watcher info subscribe manager
     */
    private SubscribeManager watcherInfoSubscriber;

    /**
     * Presence subscribe manager
     */
    private SubscribeManager presenceSubscriber;

    /**
     * The logger
     */
    private Logger logger = null;

    /**
     * Store feature tag information which should be included in Publush message (AOSP precedure)
     */
    protected String mNewFeatureTagInfo = "";

    private int mSlotId = 0;

    //presence extension

    //vilteExtension
    public PresenceExtension vilteService = null;

    private boolean isCheckInProgress = false;
    private boolean isRecheckNeeded = false;
    private boolean isFirstBoot = true;
    boolean mIsInitialize = false;
    private boolean mPresenceSubscribe = false;

    private int mServiceState = ServiceState.STATE_OUT_OF_SERVICE;

    /**
     * Constructor
     *
     * @param parent IMS module
     * @throws CoreException
     */
    public PresenceService(ImsModule parent) throws CoreException {
        super(parent, RcsSettingsManager.getRcsSettingsInstance(parent.getSlotId())
                .isSocialPresenceSupported());

        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "PresenceService");
        // Set presence service options
        this.permanentState = RcsSettingsManager.getRcsSettingsInstance(mSlotId).isPermanentStateModeActivated();

        // Instanciate the XDM manager
        xdm = new XdmManager(parent);

        // Instanciate the publish manager
        publisher = PublishManagerFactory.newInstance(parent);

        // Instanciate the subscribe manager for watcher info
        watcherInfoSubscriber = new WatcherInfoSubscribeManager(parent);

        // Instanciate the subscribe manager for presence
        presenceSubscriber = new PresenceSubscribeManager(parent);

        //extension services for presence
        //vilte
        vilteService = new ViLTEExtension(mSlotId);
    }

    /**
     * Start the IMS service
     */
    @Override
    public synchronized void start() {
        if (isServiceStarted()) {
            // Already started
            return;
        }
        setServiceStarted(true);

        //attach with extension services
        attachExtensions();

        // Restore the last presence info from the contacts database
        presenceInfo = ContactsManager.getInstance().getMyPresenceInfo(mSlotId);
        if (logger.isActivated()) {
            logger.debug("Last presence info:\n" + presenceInfo.toString());
        }
        publisher.init();
        if (mServiceState == ServiceState.STATE_IN_SERVICE) {
            //publish capability
            publishCapability();
        }
        // Listen to address book changes for non ATT SIM
        if (!SimUtils.isAttSimCard(mSlotId)) {
            getImsModule().getCore().getAddressBookManager().addAddressBookListener(this);
        }
    }

    public void initXdmForStart() {
        //XDM operations start
        //check if XDM operations required , do XDM operations just once per boot
        boolean isXDMOperationRequired = StartService
                .getXDMBootFlag(mSlotId, AndroidFactory.getApplicationContext());

        mPresenceSubscribe = false;
        if (isXDMOperationRequired) {
            try {
                logger.debug("Xdmoperation is required : start XDM operations");
                // Initialize the XDM interface
                mIsInitialize = xdm.initialize();
                logger.debug("isInitialize :" + mIsInitialize);
                StartService.setXdmInitializeFlag(mSlotId, AndroidFactory.getApplicationContext(), mIsInitialize);

                // Add me in the granted list if necessary
                List<String> grantedContacts = xdm.getGrantedContacts();
                logger.debug("The XDM granted contacts are : "
                            + grantedContacts);

                //for TMO R&S Test Case 42278.
                if (grantedContacts.size() > 0)
                    mPresenceSubscribe = true;

                String me = ImsModule.IMS_USER_PROFILE.getPublicUri();
                if (!grantedContacts.contains(me)) {
                    if (logger.isActivated()) {
                        logger.debug("The enduser is not in the granted list: add it now");
                    }
                    xdm.addContactToGrantedList(me);
                }
                grantedContacts = xdm.getGrantedContacts();

                // It may be necessary to initiate the address book first launch
                // or account check procedure
                if (StartService.getNewUserAccount(mSlotId, AndroidFactory
                        .getApplicationContext())) {
                    List<String> blockedContacts = xdm.getBlockedContacts();
                    firstLaunchOrAccountChangedCheck(grantedContacts,
                            blockedContacts);
                }

                // set the xdm operation done flag
                StartService.setXDMBootFlag(
                        mSlotId, AndroidFactory.getApplicationContext(), false);
            } catch (Exception e) {
                if (logger.isActivated()) {
                    logger.error("Exception in XDM operations while, Presence start. Ignoring it.");
                }
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        //XDM operations ends

        if (mPresenceSubscribe) {
            if (presenceSubscriber.subscribe()) {
                logger.debug("initXdmAndPresenceCheck subscribe success");
            }
        }

        // Force a presence check
        handleAddressBookHasChanged();
    }

    /**
     * First launch or account changed check <br>
     * Check done at first launch of the service on the phone after install of
     * the application or when the user account changed <br>
     * We create a new contact with the adequate state for each RCS number in
     * the XDM lists that is not already existing on the phone
     *
     * @param list of granted contacts
     * @param list of blocked contacts
     */
    public void firstLaunchOrAccountChangedCheck(List<String> grantedContacts, List<String> blockedContacts){
        if (logger.isActivated()){
            logger.debug("First launch or account change check procedure");
        }

        // Flush the address book provider
        ContactsManager.getInstance().flushContactProvider();

        // Treat the buddy list
        for(int i=0;i<grantedContacts.size(); i++){
            String me = ImsModule.IMS_USER_PROFILE.getPublicUri();
            String contact = grantedContacts.get(i);
            if (!contact.equalsIgnoreCase(me)){
                // For each RCS granted contact, except me
                String rcsNumber = PhoneUtils.extractNumberFromUri(contact);

                //extract xdmcontact details
                XDMContact xdmContact = xdm.getXDMContactDetails(contact);
                String displayName = null;
                if(xdmContact!=null){
                    displayName = xdmContact.getDisplayname();
                }

                if (!ContactUtils.isNumberInAddressBook(rcsNumber)){
                    // If it is not present in the address book
                    if (logger.isActivated()){
                        logger.debug("The RCS number " + rcsNumber + " was not found in the address book: add it");
                    }

                    // => We create the entry in the regular address book
                    try {
                        ContactUtils.createRcsContactIfNeeded(AndroidFactory.getApplicationContext(), rcsNumber , displayName);
                    } catch (Exception e) {
                        if (logger.isActivated()){
                            logger.error("Something went wrong when creating contact "+rcsNumber,e);
                        }
                    }
                }

                // Add the contact to the rich address book provider
                ContactsManager.getInstance().modifyRcsContactInProvider(rcsNumber, ContactInfo.RCS_PENDING_OUT);
            }
        }

        // Treat the blocked contact list
        for(int i=0;i<blockedContacts.size(); i++){
            // For each RCS blocked contact
            String rcsNumber = PhoneUtils.extractNumberFromUri(blockedContacts.get(i));

            XDMContact xdmContact = xdm.getXDMContactDetails(blockedContacts.get(i));
            String displayName = null;
            if(xdmContact!=null){
                displayName = xdmContact.getDisplayname();
            }

            if (!ContactUtils.isNumberInAddressBook(rcsNumber)){
                // If it is not present in the address book
                if (logger.isActivated()){
                    logger.debug("The RCS number " + rcsNumber + " was not found in the address book: add it");
                }

                // => We create the entry in the regular address book
                try {
                    ContactUtils.createRcsContactIfNeeded(AndroidFactory.getApplicationContext(), rcsNumber , displayName);
                } catch (Exception e) {
                    if (logger.isActivated()){
                        logger.error("Something went wrong when creating contact "+rcsNumber,e);
                    }
                }
                // Set the presence sharing status to blocked
                try {
                    ContactsManager.getInstance().blockContact(rcsNumber);
                } catch (ContactsManagerException e) {
                    if (logger.isActivated()){
                        logger.error("Something went wrong when blocking contact "+rcsNumber,e);
                    }
                }

                // Add the contact to the rich address book provider
                ContactsManager.getInstance().modifyRcsContactInProvider(rcsNumber, ContactInfo.RCS_BLOCKED);
            }
        }
    }

    /**
     * Stop the IMS service
     */
    @Override
    public synchronized void stop() {
        if (!isServiceStarted()) {
            // Already stopped
            return;
        }
        setServiceStarted(false);

        //detach all the presence extension services
        detachExtension();

        //stop teh xdm module
        xdm.deInitialize();

        // Stop listening to address book changes for non ATT SIM
        if (!SimUtils.isAttSimCard(mSlotId)) {
            getImsModule().getCore().getAddressBookManager().removeAddressBookListener(this);
        }

        // Stop publish
        publisher.terminate();

        // Stop subscriptions
        //watcherInfoSubscriber.terminate();
        presenceSubscriber.terminate();
    }

    /**
     * Check the IMS service
     */
    @Override
    public void check() {
        if (logger.isActivated()) {
            logger.debug("Check presence service");
        }
/*
        // Check subscribe manager status for watcher-info events
        if (!watcherInfoSubscriber.isSubscribed()) {
            if (logger.isActivated()) {
                logger.debug("Subscribe manager not yet started for watcher-info");
            }

            if (watcherInfoSubscriber.subscribe()) {
                if (logger.isActivated()) {
                    logger.debug("Subscribe manager is started with success for watcher-info");
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("Subscribe manager can't be started for watcher-info");
                }
            }
        }
        */

        /*
        // Check subscribe manager status for presence events
        if (!presenceSubscriber.isSubscribed()) {
            if (logger.isActivated()) {
                logger.debug("Subscribe manager not yet started for presence");
            }

            if (presenceSubscriber.subscribe()) {
                if (logger.isActivated()) {
                    logger.debug("Subscribe manager is started with success for presence");
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("Subscribe manager can't be started for presence");
                }
            }
        }
        */
    }

    @Override
    public void onForbiddenReceived(ImsModule.EabActions action, String reason) {
    }

    @Override
    public void onNotProvisionedReceived() {
    }

    /**
     * Is permanent state procedure
     *
     * @return Boolean
     */
    public boolean isPermanentState() {
        return permanentState;
    }

    /**
     * Set the presence info
     *
     * @param info Presence info
     */
    public void setPresenceInfo(PresenceInfo info) {
        presenceInfo = info;
    }

    /**
     * Returns the presence info
     *
     * @return Presence info
     */
    public PresenceInfo getPresenceInfo() {
        return presenceInfo;
    }

    /**
     * Returns the publish manager
     *
     * @return Publish manager
     */
    public PublishManager getPublishManager() {
        return publisher;
    }

    /**
     * Returns the watcher-info subscribe manager
     *
     * @return Subscribe manager
     */
    public SubscribeManager getWatcherInfoSubscriber() {
        return watcherInfoSubscriber;
    }

    /**
     * Returns the presence subscribe manager
     *
     * @return Subscribe manager
     */
    public SubscribeManager getPresenceSubscriber() {
        return presenceSubscriber;
    }

    /**
     * Returns the XDM manager
     *
     * @return XDM manager
     */
    public XdmManager getXdmManager() {
        return xdm;
    }

    /**
     * Build boolean status value
     *
     * @param state Boolean state
     * @return String
     */
    protected String buildBooleanStatus(boolean state) {
        if (state) {
            return "open";
        } else {
            return "closed";
        }
    }

    /**
     * Build capabilities document
     *
     * @param timestamp Timestamp
     * @param capabilities Capabilities
     * @return Document
     */
protected String buildCapabilities(String timestamp, Capabilities capabilities) {

    String FileTransfer = "FileTransfer";
    String FileTransferThumbnail = "FileTransferThumbnail";
    String SessModeMessa = "SessModeMessa";
    String DiscoveryPres = "DiscoveryPres";
    String FileTransferH = "FileTransferH";
    String VOLTE = "VOLTECALL";
    String Standalone = "StandAlone";
    String chatbot = "Chatbot";
    String chatbotSa = "ChatbotSa";
    String chatbotRole = "ChatbotRole";
    String callcomposer = "Callcomposer";

    String publishXML = "";

    if(RcsSettingsManager.getRcsSettingsInstance(mSlotId).isStandaloneMsgSupport()){
  //Standalone Mode Messaging
    publishXML+="<tuple id=\""+Standalone+"\">" + SipUtils.CRLF +
    "  <status><basic>" + buildBooleanStatus(RcsSettingsManager.getRcsSettingsInstance(mSlotId).isStandaloneMsgSupport()) +
    "</basic></status>" + SipUtils.CRLF +
    "  <op:service-description>" + SipUtils.CRLF +
    "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_STANDALONEMSG + "</op:service-id>" + SipUtils.CRLF +
    "    <op:version>2.0</op:version>" + SipUtils.CRLF +
    "    <op:description>Standalone Mode Messaging</op:description>" + SipUtils.CRLF +
    "  </op:service-description>" + SipUtils.CRLF +
    "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
    "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
    "</tuple>" + SipUtils.CRLF;
    }

    if(capabilities.isImSessionSupported()){
    //Session Mode Messaging
    publishXML+="<tuple id=\""+SessModeMessa+"\">" + SipUtils.CRLF +
    "  <status><basic>" + buildBooleanStatus(capabilities.isImSessionSupported()) + "</basic></status>" + SipUtils.CRLF +
    "  <op:service-description>" + SipUtils.CRLF +
    "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_CHAT_2 + "</op:service-id>" + SipUtils.CRLF +
    "    <op:version>2.0</op:version>" + SipUtils.CRLF +
    "    <op:description>Session Mode Messaging</op:description>" + SipUtils.CRLF +
    "  </op:service-description>" + SipUtils.CRLF +
    "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
    "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
    "</tuple>" + SipUtils.CRLF;
     }

    if(capabilities.isPresenceDiscoverySupported()){
    //PRESENCE DEISCOVERY
    publishXML+="<tuple id=\""+DiscoveryPres+"\">" + SipUtils.CRLF +
    "  <status><basic>" + buildBooleanStatus(capabilities.isPresenceDiscoverySupported()) + "</basic></status>" + SipUtils.CRLF +
    "  <op:service-description>" + SipUtils.CRLF +
    "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_DISCOVERY_VIA_PRESENCE + "</op:service-id>" + SipUtils.CRLF +
    "    <op:version>1.0</op:version>" + SipUtils.CRLF +
    "  </op:service-description>" + SipUtils.CRLF +
    "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
    "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
    "</tuple>" + SipUtils.CRLF ;
    }

    if(capabilities.isFileTransferSupported()){
    //File Transfer
    publishXML+="<tuple id=\""+FileTransfer+"\">" + SipUtils.CRLF +
        "  <status><basic>" + buildBooleanStatus(capabilities.isFileTransferSupported()) + "</basic></status>" + SipUtils.CRLF +
        "  <op:service-description>" + SipUtils.CRLF +
        "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_FT + "</op:service-id>" + SipUtils.CRLF +
        "    <op:version>1.0</op:version>" + SipUtils.CRLF +
        "    <op:description>File Transfer</op:description>" + SipUtils.CRLF +
        "  </op:service-description>" + SipUtils.CRLF +
        "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
        "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
        "</tuple>" + SipUtils.CRLF ;
    }

    if(capabilities.isFileTransferThumbnailSupported()){
        //File Transfer
        publishXML+="<tuple id=\""+FileTransferThumbnail+"\">" + SipUtils.CRLF +
            "  <status><basic>" + buildBooleanStatus(capabilities.isFileTransferThumbnailSupported()) + "</basic></status>" + SipUtils.CRLF +
            "  <op:service-description>" + SipUtils.CRLF +
            "    <op:service-id>" + "org.openmobilealliance:File-Transfer-thumb"+ "</op:service-id>" + SipUtils.CRLF +
            "    <op:version>2.0</op:version>" + SipUtils.CRLF +
            "    <op:description>File Transfer Thumbnail</op:description>" + SipUtils.CRLF +
            "  </op:service-description>" + SipUtils.CRLF +
            "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
            "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
            "</tuple>" + SipUtils.CRLF ;
        }

    if(capabilities.isFileTransferHttpSupported()){
    //FileTransferHTTP
    publishXML+="<tuple id=\""+FileTransferH+"\">" + SipUtils.CRLF +
        "  <status><basic>" + buildBooleanStatus(capabilities.isFileTransferHttpSupported()) + "</basic></status>" + SipUtils.CRLF +
        "  <op:service-description>" + SipUtils.CRLF +
        "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_FT_HTTP + "</op:service-id>" + SipUtils.CRLF +
        "    <op:version>1.0</op:version>" + SipUtils.CRLF +
        "    <op:description>FileTransferHTTP</op:description>" + SipUtils.CRLF +
        "  </op:service-description>" + SipUtils.CRLF +
        "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
        "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
        "</tuple>" + SipUtils.CRLF ;
    }

    if(capabilities.isIR94_VoiceCallSupported()) {
        //IR94 VIDEO AND VOICE CALL
        int[] subIds = SubscriptionManager.getSubId(mSlotId);
        if (subIds == null) {
            logger.debug("buildCapabilities fail. subIds is null");
            return "";
        }
        TelephonyManager tm = TelephonyManager.from(AndroidFactory.getApplicationContext()).createForSubscriptionId(subIds[0]);
        boolean isVideoCallingEnabled = tm.isVideoCallingEnabled();
        boolean isIR94_VCSupported  = capabilities.isIR94_VideoCallSupported();
        logger.debug("buildCapabilities, isVideoCallingEnabled =" + isVideoCallingEnabled +
                " isIR94_VCSupported =" + isIR94_VCSupported);

        publishXML += "<tuple id=\""+VOLTE+"\">" + SipUtils.CRLF +
            "  <status><basic>" + buildBooleanStatus(capabilities.isIR94_VoiceCallSupported()) + "</basic></status>" + SipUtils.CRLF +
            "  <caps:servcaps>" + SipUtils.CRLF +
            " <caps:audio>"+capabilities.isIR94_VoiceCallSupported()+"</caps:audio>" + SipUtils.CRLF +
            " <caps:video>"+(isVideoCallingEnabled && isIR94_VCSupported)+"</caps:video>" + SipUtils.CRLF;

        if(capabilities.isIR94_DuplexModeSupported() && ((ViLTEExtension)vilteService).isDuplexSupported()){
            publishXML +=" <caps:duplex>" + SipUtils.CRLF + "<caps:supported>" + SipUtils.CRLF +
                    "<caps:full/>" + SipUtils.CRLF +
                    "</caps:supported>" + SipUtils.CRLF +"</caps:duplex>" + SipUtils.CRLF;
        }

        publishXML += "  </caps:servcaps>" + SipUtils.CRLF +
            "  <op:service-description>" + SipUtils.CRLF +
            "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL + "</op:service-id>" + SipUtils.CRLF +
            "    <op:version>1.0</op:version>" + SipUtils.CRLF +
            "    <op:description>IPVideoCall</op:description>" + SipUtils.CRLF +
            "  </op:service-description>" + SipUtils.CRLF +
            "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
            "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
            "</tuple>" + SipUtils.CRLF;
   }

    //IMAGE SHARE
    if(capabilities.isImageSharingSupported()){
       publishXML += "<tuple id=\"t2\">" + SipUtils.CRLF +
        "  <status><basic>" + buildBooleanStatus(capabilities.isImageSharingSupported()) + "</basic></status>" + SipUtils.CRLF +
        "  <op:service-description>" + SipUtils.CRLF +
        "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_IMAGE_SHARE + "</op:service-id>" + SipUtils.CRLF +
        "    <op:version>1.0</op:version>" + SipUtils.CRLF +
        "  </op:service-description>" + SipUtils.CRLF +
        "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
        "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
        "</tuple>" + SipUtils.CRLF;

    }

     //VIDEO SHARE
    if(capabilities.isVideoSharingSupported()){

    publishXML += "<tuple id=\"t3\">" + SipUtils.CRLF +
        "  <status><basic>" + buildBooleanStatus(capabilities.isVideoSharingSupported()) + "</basic></status>" + SipUtils.CRLF +
        "  <op:service-description>" + SipUtils.CRLF +
        "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_VIDEO_SHARE + "</op:service-id>" + SipUtils.CRLF +
        "    <op:version>1.0</op:version>" + SipUtils.CRLF +
        "  </op:service-description>" + SipUtils.CRLF +
        "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
        "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
        "</tuple>" + SipUtils.CRLF;
    }

    //CHATBOT
    if (capabilities.isChatbotSupported()){
        publishXML+="<tuple id=\""+chatbot+"\">" + SipUtils.CRLF +
            "  <status><basic>" + buildBooleanStatus(capabilities.isChatbotSupported()) + "</basic></status>" + SipUtils.CRLF +
            "  <op:service-description>" + SipUtils.CRLF +
            "    <op:service-id>" + PresenceUtils.FEATURE_CHATBOT + "</op:service-id>" + SipUtils.CRLF +
            "    <op:version>" + capabilities.getBotVersion() + "</op:version>" + SipUtils.CRLF +
            "  </op:service-description>" + SipUtils.CRLF +
            "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
            "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
            "</tuple>" + SipUtils.CRLF;
    }

    if (capabilities.isChatbotSaSupported()){
        publishXML+="<tuple id=\""+chatbotSa+"\">" + SipUtils.CRLF +
            "  <status><basic>" + buildBooleanStatus(capabilities.isChatbotSaSupported()) + "</basic></status>" + SipUtils.CRLF +
            "  <op:service-description>" + SipUtils.CRLF +
            "    <op:service-id>" + PresenceUtils.FEATURE_CHATBOT_SA + "</op:service-id>" + SipUtils.CRLF +
            "    <op:version>" + capabilities.getBotVersion() + "</op:version>" + SipUtils.CRLF +
            "  </op:service-description>" + SipUtils.CRLF +
            "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
            "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
            "</tuple>" + SipUtils.CRLF;
    }

    if (capabilities.isChatbotSupported()
            || capabilities.isChatbotSaSupported()){

        publishXML+="<tuple id=\""+chatbotRole+"\">" + SipUtils.CRLF +
            "  <status><basic>" + buildBooleanStatus(true) + "</basic></status>" + SipUtils.CRLF +
            "  <op:service-description>" + SipUtils.CRLF +
            "    <op:service-id>" + PresenceUtils.FEATURE_CHATBOT_ROLE + "</op:service-id>" + SipUtils.CRLF +
            "    <op:version>1.0</op:version>" + SipUtils.CRLF +
            "  </op:service-description>" + SipUtils.CRLF +
            "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
            "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
            "</tuple>" + SipUtils.CRLF;
    }

    //enriched ccalling
    if (capabilities.isCallcomposerSupported()) {
        publishXML+="<tuple id=\""+callcomposer+"\">" + SipUtils.CRLF +
            "  <status><basic>" + buildBooleanStatus(capabilities.isCallcomposerSupported()) + "</basic></status>" + SipUtils.CRLF +
            "  <op:service-description>" + SipUtils.CRLF +
            "    <op:service-id>" + PresenceUtils.FEATURE_CALLCOMPOSER + "</op:service-id>" + SipUtils.CRLF +
            "    <op:version>2.0</op:version>" + SipUtils.CRLF +
            "  </op:service-description>" + SipUtils.CRLF +
            "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
            "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
            "</tuple>" + SipUtils.CRLF;
    }

    return publishXML;
}

    /**
     * Build geoloc document
     *
     * @param timestamp Timestamp
     * @param geolocInfo Geoloc info
     * @return Document
     */
    protected String buildGeoloc(String timestamp, Geoloc geolocInfo) {
        String document = "";
        if (geolocInfo != null) {
            document +=
                 "<tuple id=\"g1\">" + SipUtils.CRLF +
                 "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                 "   <gp:geopriv>" + SipUtils.CRLF +
                 "    <gp:location-info><gml:location>" + SipUtils.CRLF +
                 "        <gml:Point srsDimension=\"3\"><gml:pos>" + geolocInfo.getLatitude() + " " +
                                 geolocInfo.getLongitude() + " " +
                                 geolocInfo.getAltitude() + "</gml:pos>" + SipUtils.CRLF +
                 "        </gml:Point></gml:location>" + SipUtils.CRLF +
                 "    </gp:location-info>" + SipUtils.CRLF +
                 "    <gp:method>GPS</gp:method>" + SipUtils.CRLF +
                 "   </gp:geopriv>"+SipUtils.CRLF +
                 "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                 "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                 "</tuple>" + SipUtils.CRLF;
        }
        return document;
    }

    /**
     * Build person info document
     *
     * @param info Presence info
     * @return Document
     */
    protected String buildPersonInfo(PresenceInfo info) {
        String document = "  <op:overriding-willingness>" + SipUtils.CRLF +
                "    <op:basic>" + info.getPresenceStatus() + "</op:basic>" + SipUtils.CRLF +
                "  </op:overriding-willingness>" + SipUtils.CRLF;

        FavoriteLink favoriteLink = info.getFavoriteLink();
        if ((favoriteLink != null) && (favoriteLink.getLink() != null)) {
            document += "  <ci:homepage>" + StringUtils.encodeUTF8(StringUtils.encodeXML(favoriteLink.getLink())) + "</ci:homepage>" + SipUtils.CRLF;
        }

        PhotoIcon photoIcon = info.getPhotoIcon();
        if ((photoIcon != null) && (photoIcon.getEtag() != null)) {
            document +=
                "  <rpid:status-icon opd:etag=\"" + photoIcon.getEtag() +
                "\" opd:fsize=\"" + photoIcon.getSize() +
                "\" opd:contenttype=\"" + photoIcon.getType() +
                "\" opd:resolution=\"" + photoIcon.getResolution() + "\">" + xdm.getEndUserPhotoIconUrl() +
                "</rpid:status-icon>" + SipUtils.CRLF;
        }

        String freetext = info.getFreetext();
        if (freetext != null){
            document += "  <pdm:note>" + StringUtils.encodeUTF8(StringUtils.encodeXML(freetext)) + "</pdm:note>" + SipUtils.CRLF;
        }

        return document;
    }

    /**
     * Build presence info document (RCS 1.0)
     *
     * @param info Presence info
     * @return Document
     */
    protected String buildPresenceInfoDocument(PresenceInfo info, Capabilities capabilities) {
        String document= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
            " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"" +
            " xmlns:opd=\"urn:oma:xml:pde:pidf:ext\"" +
            " xmlns:pdm=\"urn:ietf:params:xml:ns:pidf:data-model\"" +
            " xmlns:cipid=\"urn:ietf:params:xml:ns:pidf:cipid\""
                + " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\""
                + " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\""
                + " entity=\""+ ImsModule.IMS_USER_PROFILE.getPublicUri() + "\">" + SipUtils.CRLF;

        // Encode timestamp
        String timestamp = DateUtils.encodeDate(info.getTimestamp());

        // Build capabilities
        document += buildCapabilities(timestamp, capabilities);

        // Add feature tag info from client (AOSP procedure)
        document += mNewFeatureTagInfo;
        mNewFeatureTagInfo = "";

        // Build geoloc
        document += buildGeoloc(timestamp, info.getGeoloc());

        // Build person info
        document += "<pdm:person id=\"p1\">" + SipUtils.CRLF +
                    buildPersonInfo(info) +
                    "  <pdm:timestamp>" + timestamp + "</pdm:timestamp>" + SipUtils.CRLF +
                    "</pdm:person>" + SipUtils.CRLF;

        // Add last header
        document += "</presence>" + SipUtils.CRLF;

        return document;
    }

    /**
     * Build partial presence info document (all presence info except permanent
     * state info)
     *
     * @param info Presence info
     * @return Document
     */
    protected String buildPartialPresenceInfoDocument(PresenceInfo info) {
        if (logger.isActivated()) {
            logger.debug("buildPartialPresenceInfoDocument");
        }

        /*String document= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
            " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"" +
            " xmlns:opd=\"urn:oma:xml:pde:pidf:ext\"" +
            " xmlns:pdm=\"urn:ietf:params:xml:ns:pidf:data-model\"" +
 " xmlns:ci=\"urn:ietf:params:xml:ns:pidf:cipid\""
                + " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\""
                + " xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\""
                + " xmlns:gml=\"urn:opengis:specification:gml:schema-xsd:feature:v3.0\""
                +
            " entity=\""+ ImsModule.IMS_USER_PROFILE.getPublicUri() + "\">" + SipUtils.CRLF;*/

        String document= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
                " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"" +
                //" xmlns:opd=\"urn:oma:xml:pde:pidf:ext\"" +
                " xmlns:pdm=\"urn:ietf:params:xml:ns:pidf:data-model\"" +
                " xmlns:cipid=\"urn:ietf:params:xml:ns:pidf:cipid\"" +
                " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\"" +
                //" xmlns:ci=\"urn:ietf:params:xml:ns:pidf:cipid\"" +
                " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\"" +
            " entity=\""+ ImsModule.IMS_USER_PROFILE.getPublicUri() + "\">" + SipUtils.CRLF;

        // Encode timestamp
        String timestamp = DateUtils.encodeDate(info.getTimestamp());

        // Build capabilities
        document += buildCapabilities(timestamp, RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                .getMyCapabilities());

        // Add feature tag info from client (AOSP procedure)
        document += mNewFeatureTagInfo;
        mNewFeatureTagInfo = "";

        // Build geoloc
        document += buildGeoloc(timestamp, info.getGeoloc());

        // Add last header
        document += "</presence>" + SipUtils.CRLF;

        return document;
    }

    /**
     * Build permanent presence info document (RCS R2.0)
     *
     * @param info Presence info
     * @return Document
     */
    protected String buildPermanentPresenceInfoDocument(PresenceInfo info) {
        /*String document= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
            " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"" +
            " xmlns:opd=\"urn:oma:xml:pde:pidf:ext\"" +
            " xmlns:pdm=\"urn:ietf:params:xml:ns:pidf:data-model\"" +
            " xmlns:ci=\"urn:ietf:params:xml:ns:pidf:cipid\"" +
            " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\"" +
            " entity=\""+ ImsModule.IMS_USER_PROFILE.getPublicUri() + "\">" + SipUtils.CRLF;*/

        String document= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
                " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"" +
                //" xmlns:opd=\"urn:oma:xml:pde:pidf:ext\"" +
                " xmlns:pdm=\"urn:ietf:params:xml:ns:pidf:data-model\"" +
                " xmlns:cipid=\"urn:ietf:params:xml:ns:pidf:cipid\"" +
                " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\"" +
                //" xmlns:ci=\"urn:ietf:params:xml:ns:pidf:cipid\"" +
                " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\"" +
            " entity=\""+ ImsModule.IMS_USER_PROFILE.getPublicUri() + "\">" + SipUtils.CRLF;

        // Encode timestamp
        String timestamp = DateUtils.encodeDate(info.getTimestamp());

        // Build person info (freetext, favorite link and photo-icon)
        document += "<pdm:person id=\"p1\">" + SipUtils.CRLF +
                    buildPersonInfo(info) +
                    "  <pdm:timestamp>" + timestamp + "</pdm:timestamp>" + SipUtils.CRLF +
                    "</pdm:person>" + SipUtils.CRLF;

        // Add last header
        document += "</presence>" + SipUtils.CRLF;

        return document;
    }

    /**
     * Update photo-icon
     *
     * @param photoIcon Photo-icon
     * @return Boolean result
     */
    protected boolean updatePhotoIcon(PhotoIcon photoIcon) {
        boolean result = false;

        // Photo-icon management
        PhotoIcon currentPhoto = presenceInfo.getPhotoIcon();
        if ((photoIcon != null) && (photoIcon.getEtag() == null)) {
            // Test photo icon size
            int maxSize = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .getMaxPhotoIconSize()*1024;
            if ((maxSize != 0) && (photoIcon.getSize() > maxSize)) {
                if (logger.isActivated()) {
                    logger.debug("Max photo size achieved");
                }
                return false;
            }

            // Upload the new photo-icon
            if (logger.isActivated()) {
                logger.info("Upload the photo-icon");
            }
            result = uploadPhotoIcon(photoIcon);
        } else
        if ((photoIcon == null) && (currentPhoto != null)) {
            // Delete the current photo-icon
            if (logger.isActivated()) {
                logger.info("Delete the photo-icon");
            }
            result = deletePhotoIcon();
        } else {
            // Nothing to do
            result = true;
        }
        return result;
    }

    /**
     * Upload photo icon
     *
     * @param photo Photo icon
     * @returns Boolean result
     */
    public boolean uploadPhotoIcon(PhotoIcon photo) {
        // Upload the photo to the XDM server
        HttpResponse response = xdm.uploadEndUserPhoto(photo);
        if ((response != null) && response.isSuccessfullResponse()) {
            // Extract the Etag value in the 200 OK response
            String etag = response.getHeader("Etag");
            if (etag != null) {
                // Removed quotes
                etag = StringUtils.removeQuotes(etag);
            } else {
                etag = "" + System.currentTimeMillis();
            }

            // Set the Etag of the photo-icon
            photo.setEtag(etag);

            return true;
        } else {
            return false;
        }
    }

    /**
     * Delete photo icon
     *
     * @returns Boolean result
     */
    public boolean deletePhotoIcon(){
        // Delete the photo from the XDM server
        HttpResponse response = xdm.deleteEndUserPhoto();
        if ((response != null) && (response.isSuccessfullResponse() || response.isNotFoundResponse())) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Invite a contact to share its presence
     *
     * @param contact Contact
     * @returns Returns true if XDM request was successful, else false
     */
    public boolean inviteContactToSharePresence(String contact) {
        // Remove contact from the blocked contacts list
        String contactUri = PhoneUtils.formatNumberToSipUri(contact);
        xdm.removeContactFromBlockedList(contactUri);

        // Remove contact from the revoked contacts list
        xdm.removeContactFromRevokedList(contactUri);

        // Add contact in the granted contacts list
        HttpResponse response = xdm.addContactToGrantedList(contactUri);
        if ((response != null) && response.isSuccessfullResponse()) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Revoke a shared contact
     *
     * @param contact Contact
     * @returns Returns true if XDM request was successful, else false
     */
    public boolean revokeSharedContact(String contact){
        // Add contact in the revoked contacts list
        String contactUri = PhoneUtils.formatNumberToSipUri(contact);
        HttpResponse response = xdm.addContactToRevokedList(contactUri);
        if ((response == null) || (!response.isSuccessfullResponse())) {
            return false;
        }

        // Remove contact from the granted contacts list
        response = xdm.removeContactFromGrantedList(contactUri);
        if ((response != null)
                && (response.isSuccessfullResponse() || response.isNotFoundResponse())) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Accept a presence sharing invitation
     *
     * @param contact Contact
     * @returns Returns true if XDM request was successful, else false
     */
    public boolean acceptPresenceSharingInvitation(String contact) {
        // Add contact in the granted contacts list
        String contactUri = PhoneUtils.formatNumberToSipUri(contact);
        HttpResponse response = xdm.addContactToGrantedList(contactUri);
        if ((response != null) && response.isSuccessfullResponse()) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Block a presence sharing invitation
     *
     * @param contact Contact
     * @returns Returns true if XDM request was successful, else false
     */
    public boolean blockPresenceSharingInvitation(String contact){
        // Add contact in the blocked contacts list
        String contactUri = PhoneUtils.formatNumberToSipUri(contact);
        HttpResponse response = xdm.addContactToBlockedList(contactUri);
        if ((response != null) && response.isSuccessfullResponse()) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Remove a revoked contact
     *
     * @param contact Contact
     * @returns Returns true if XDM request was successful, else false
     */
    public boolean removeRevokedContact(String contact) {
        // Remove contact from the revoked contacts list
        String contactUri = PhoneUtils.formatNumberToSipUri(contact);
        HttpResponse response = xdm.removeContactFromRevokedList(contactUri);
        if ((response != null)
                && (response.isSuccessfullResponse() || response.isNotFoundResponse())) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Remove a blocked contact
     *
     * @param contact Contact
     * @returns Returns true if XDM request was successful, else false
     */
    public boolean removeBlockedContact(String contact) {
        // Remove contact from the blocked contacts list
        String contactUri = PhoneUtils.formatNumberToSipUri(contact);
        HttpResponse response = xdm.removeContactFromBlockedList(contactUri);
        if ((response != null)
                && (response.isSuccessfullResponse() || response.isNotFoundResponse())) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Address book content has changed
     */
    public void handleAddressBookHasChanged() {
        if (logger.isActivated()){
            logger.debug(" handleAddressBookHasChanged entry ");
        }
        if(isCheckInProgress){
            isRecheckNeeded = true;
            return;
        }

        isCheckInProgress = true;
        isRecheckNeeded = false;

        // If a contact used to be in a RCS relationship with us but is not in the address book any more, we may have to remove or unblock it
        // Get a list of all RCS numbers
        List<String> rcsNumbers = ContactsManager.getInstance().getRcsContactsWithSocialPresence();
        for(int i=0;i<rcsNumbers.size(); i++){
            // For each RCS number
            String rcsNumber = rcsNumbers.get(i);
            if (!ContactUtils.isNumberInAddressBook(rcsNumber)){
                // If it is not present in the address book
                if (logger.isActivated()){
                    logger.debug("The RCS number " + rcsNumber + " was not found in the address book any more.");
                }

                if (ContactsManager.getInstance().isNumberShared(rcsNumber)
                        || ContactsManager.getInstance().isNumberInvited(rcsNumber)){
                    // Active or Invited
                    if (logger.isActivated()){
                        logger.debug(rcsNumber + " is either active or invited");
                        logger.debug("We remove it from the buddy list");
                    }
                    // We revoke it
                    boolean result = revokeSharedContact(rcsNumber);
                    if (result){
                        // The contact should be automatically unrevoked after a given timeout. Here the
                        // timeout period is 0, so the contact can receive invitations again now
                        result = removeRevokedContact(rcsNumber);
                        if (result){
                            // Remove entry from rich address book provider
                            ContactsManager.getInstance().modifyRcsContactInProvider(rcsNumber, ContactInfo.RCS_CAPABLE);
                        }else{
                            if (logger.isActivated()){
                                logger.error("Something went wrong when revoking shared contact");
                            }
                        }
                    }
                }else if (ContactsManager.getInstance().isNumberBlocked(rcsNumber)){
                    // Blocked
                    if (logger.isActivated()){
                        logger.debug(rcsNumber + " is blocked");
                        logger.debug("We remove it from the blocked list");
                    }
                    // We unblock it
                    boolean result = removeBlockedContact(rcsNumber);
                    if (result){
                        // Remove entry from rich address book provider
                        ContactsManager.getInstance().modifyRcsContactInProvider(rcsNumber, ContactInfo.RCS_CAPABLE);
                    }else{
                        if (logger.isActivated()){
                            logger.error("Something went wrong when removing blocked contact");
                        }
                    }
                } else if (ContactsManager.getInstance().isNumberWilling(rcsNumber)){
                    // Willing
                    if (logger.isActivated()){
                        logger.debug(rcsNumber + " is willing");
                        logger.debug("Nothing to do");
                    }
                } else if (ContactsManager.getInstance().isNumberCancelled(rcsNumber)){
                    // Cancelled
                    if (logger.isActivated()){
                        logger.debug(rcsNumber + " is cancelled");
                        logger.debug("We remove it from rich address book provider");
                    }
                    // Remove entry from rich address book provider
                    ContactsManager.getInstance().modifyRcsContactInProvider(rcsNumber, ContactInfo.RCS_CAPABLE);
                }
            }
        }

        /*
         * sync the new numbers in phonebook to xdm server.
         *
         */
        try {
        //check for contacts neeed to be added to xdm granted lits
        List<String> rcsEnabledNumbers =  ContactsManager.getInstance().getRcsContacts();
        List<String> addNumbersToXDMGrantList = new ArrayList<String>();
        for(String rcsEnabledNumber : rcsEnabledNumbers)
        {
            //if the RCS enabled no is in contact book.
            //check is its in the granted contact list
            String formattedNumber = PhoneUtils.formatNumberToXDMSipUri(rcsEnabledNumber);
            if(!xdm.getGrantedContactsList().contains(formattedNumber)){
                if (logger.isActivated()){
                    logger.debug(" rcsNumber : " + rcsEnabledNumber + " add to xdm granted list");
                }
                //if(!addNumbersToXDMGrantList.contains(formattedNumber)){
                    addNumbersToXDMGrantList.add(formattedNumber);
                //}
            }
        }

        isFirstBoot = StartService.getFirstBootFlag(mSlotId, AndroidFactory.getApplicationContext());
        mIsInitialize = StartService.getXdmInitializeFlag(mSlotId, AndroidFactory.getApplicationContext());
        if (logger.isActivated()) {
            logger.debug(" isFirstBoot value : " + isFirstBoot + "mIsInitalize value:" + mIsInitialize );
        }
        if (mIsInitialize) {
            if(isFirstBoot){
                xdm.addContactToGrantedList(addNumbersToXDMGrantList);
            } else {
                for(String rcsEnabledNumber : addNumbersToXDMGrantList){
                    xdm.addContactToGrantedList(rcsEnabledNumber);
                }
            }
        }
        StartService.setFirstBootFlag(mSlotId, AndroidFactory.getApplicationContext(), false);

        } catch (Exception e) {
            if (logger.isActivated()){
                    logger.error(" Exception in handling addressbook change : " + e.getMessage());
            }
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        isCheckInProgress = false;
        if(isRecheckNeeded){
            handleAddressBookHasChanged();
        }
    }

    //AOSP procedure
    public void setNewFeatureTagInfo(String featureTag, PresServiceInfo serviceInfo) {
        String status = null;
        boolean audioSupport = false;
        boolean videoSupport = false;
        String timestamp = DateUtils.encodeDate(presenceInfo.getTimestamp());
        switch (serviceInfo.getMediaType()) {
            case PresServiceInfo.UCE_PRES_MEDIA_CAP_FULL_AUDIO_ONLY:
                status = "open";
                audioSupport = true;
                videoSupport = false;
                break;
            case PresServiceInfo.UCE_PRES_MEDIA_CAP_FULL_AUDIO_AND_VIDEO:
                status = "open";
                audioSupport = true;
                videoSupport = true;
                break;
            default:
                status = "close";
                audioSupport = false;
                videoSupport = false;
                break;
        }
        //TODO: Need to confirm the tagId
        String tupleId = "";
        mNewFeatureTagInfo +=
        "<tuple id=\"" + tupleId + "\">" + SipUtils.CRLF +
        "  <status><basic>" + status + "</basic></status>" + SipUtils.CRLF +
        "  <caps:servcaps>" + SipUtils.CRLF +
        "    <caps:audio>" + String.valueOf(audioSupport) + "</caps:audio>" + SipUtils.CRLF +
        "  </caps:servcaps>" + SipUtils.CRLF +
        "  <op:service-description>" + SipUtils.CRLF +
        "    <op:service-id>" + serviceInfo.getServiceId() + "</op:service-id>" + SipUtils.CRLF +
        "    <op:version>" + serviceInfo.getServiceVer() + "</op:version>" + SipUtils.CRLF +
        "    <op:description>" + serviceInfo.getServiceDesc() + "</op:description>" + SipUtils.CRLF +
        "  </op:service-description>" + SipUtils.CRLF +
        "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
        "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
        "</tuple>" + SipUtils.CRLF;
        publishCapability();
    }

    //AOSP procedure
    public synchronized void updateMyCapability(int requestId, Capabilities caps) {
        //AOSP UCE only support single SIM so using slotId = 0
        int slotId = 0;
        RcsSettingsManager.getRcsSettingsInstance(slotId).setImsCapControlFlag(true);
        RcsSettingsManager.getRcsSettingsInstance(slotId).setIR92VoiceCallSupport(
                caps.isIR94_VoiceCallSupported());
        RcsSettingsManager.getRcsSettingsInstance(slotId).setIR94VideoCallSupport(
                caps.isIR94_VideoCallSupported());
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
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH,
                Boolean.toString(caps.isGeolocationPushSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH,
                Boolean.toString(caps.isGeolocationPushSupported()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.STANDALONE_MSG_SUPPORT,
                Boolean.toString(caps.isStandaloneMsgSupport()));
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_GROUP_CHAT_SF,
                Boolean.toString(caps.isGroupChatStoreForwardSupported()));
        StringBuilder extensionsWithSemicolon = new StringBuilder();
        String delimiter = "";
        for (String extension : caps.getSupportedExtensions()) {
            extensionsWithSemicolon.append(delimiter);
            delimiter = ";";
            extensionsWithSemicolon.append(extension);
        }
        RcsSettingsManager.getRcsSettingsInstance(slotId).writeParameter(RcsSettingsData.CAPABILITY_RCS_EXTENSIONS,
                extensionsWithSemicolon.toString());
        presenceInfo.setTimestamp(caps.getTimestamp());

        publishCapability(requestId);
    }

    //AOSP procedure
    public void publishCapability(int requestId) {
        publisher.setRequestId(requestId);
        publishCapability();
    }

    public void publishCapability() {

        if (isServiceStarted() == false) {
            logger.debug("imsService is not start, ignore the request");
            return;
        }
        // Publish initial presence info
        String xml;
        if (permanentState) {
            xml = buildPartialPresenceInfoDocument(presenceInfo);
        } else {
            Capabilities capabilities = RcsSettingsManager.getRcsSettingsInstance(mSlotId)
                    .getMyCapabilities();
            xml = buildPresenceInfoDocument(presenceInfo, capabilities);
        }
        if (publisher.publish(xml)) {
            if (logger.isActivated()) {
                logger.debug("Publish manager is started with success");
            }
        } else {
            if (logger.isActivated()) {
                logger.debug("Publish manager can't be started");
            }
        }
    }

    @Override
    public void handleCapabilityChanged(PresenceExtension extension) {

        if (logger.isActivated()) {
            logger.debug("handleCapabilityChanged : due to extension : "+extension.getName());
        }
        // TODO Auto-generated method stub
        publishCapability();
    }

    public void handleServiceStateChanged(ServiceState state) {
        logger.debug("handleServiceStateChanged : new state : " +
                state.getDataRegState() + " old state: " + mServiceState);

        if (mServiceState != state.getDataRegState()) {
            mServiceState = state.getDataRegState();
            if (mServiceState == ServiceState.STATE_IN_SERVICE) {
                publishCapability();
            }
        }
    }

    protected void attachExtensions(){
        //vilte extension
        if(vilteService !=null){
            vilteService.attachExtension(this);
        }
    }

    protected void detachExtension(){
        //vilte extension
        if(vilteService !=null){
            vilteService.detachExtension();
        }
    }

    public boolean isPresenceSubscribed() {
        return mPresenceSubscribe;
    }

    public PresenceExtension getVilteService() {
        return vilteService;
    }

    public void updateRcsCapabilities(boolean isModify, Capabilities caps) {
        boolean needPublish = false;
        Capabilities myCaps = RcsSettingsManager.getRcsSettingsInstance(mSlotId).getMyCapabilities();
        logger.debug("updateRcsCapabilities : isModify:" + isModify);
        logger.debug("updateRcsCapabilities : stackCaps : "+ myCaps);
        logger.debug("updateRcsCapabilities : apSet: " + caps);

        if (isModify == true) {
            if(myCaps.isImageSharingSupported() != caps.isImageSharingSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_IMAGE_SHARING,
                        Boolean.toString(caps.isImageSharingSupported()));
            }

            if(myCaps.isVideoSharingSupported() != caps.isVideoSharingSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_VIDEO_SHARING,
                    Boolean.toString(caps.isVideoSharingSupported()));
            }

            if(myCaps.isIPVoiceCallSupported() != caps.isIPVoiceCallSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_IP_VOICE_CALL,
                    Boolean.toString(caps.isIPVoiceCallSupported()));
            }

            if(myCaps.isIPVideoCallSupported() != caps.isIPVideoCallSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_IP_VIDEO_CALL,
                    Boolean.toString(caps.isIPVideoCallSupported()));
            }

            if(myCaps.isImSessionSupported() != caps.isImSessionSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_IM_SESSION,
                    Boolean.toString(caps.isImSessionSupported()));
            }

            if(myCaps.isFileTransferSupported() != caps.isFileTransferSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER,
                    Boolean.toString(caps.isFileTransferSupported()));
            }

            if(myCaps.isCsVideoSupported() != caps.isCsVideoSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_CS_VIDEO,
                    Boolean.toString(caps.isCsVideoSupported()));
            }

            if(myCaps.isFileTransferHttpSupported() != caps.isFileTransferHttpSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP,
                    Boolean.toString(caps.isFileTransferHttpSupported()));
            }

            if(myCaps.isGeolocationPushSupported() != caps.isGeolocationPushSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH,
                    Boolean.toString(caps.isGeolocationPushSupported()));
            }

            if(myCaps.isFileTransferThumbnailSupported()!= caps.isFileTransferThumbnailSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL,
                    Boolean.toString(caps.isFileTransferThumbnailSupported()));
            }

            if(myCaps.isStandaloneMsgSupport() != caps.isStandaloneMsgSupport()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.STANDALONE_MSG_SUPPORT,
                    Boolean.toString(caps.isStandaloneMsgSupport()));
            }

            if(myCaps.isChatbotSupported() != caps.isChatbotSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CHATBOT_SUPPORT,
                    Boolean.toString(caps.isChatbotSupported()));
            }

            if(myCaps.isChatbotSupported() != caps.isChatbotSaSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CHATBOT_SA_SUPPORT,
                    Boolean.toString(caps.isChatbotSaSupported()));
            }

            if(myCaps.isCallcomposerSupported() != caps.isCallcomposerSupported()) {
                needPublish = true;
                RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(RcsSettingsData.CALLCOMPOSER_SUPPORT,
                    Boolean.toString(caps.isCallcomposerSupported()));
            }

            if (needPublish) {
                logger.debug("do publish");
                publishCapability();
            }
        }else {
            logger.debug("do initial publish");
            String xml = buildPresenceInfoDocument(presenceInfo, caps);
            if (publisher.publish(xml)) {
                if (logger.isActivated()) {
                    logger.debug("Publish manager is started with success");
                }
            } else {
                if (logger.isActivated()) {
                    logger.debug("Publish manager can't be started");
                }
            }
        }
    }

    public int getSlotId() {
        return mSlotId;
    }
}
