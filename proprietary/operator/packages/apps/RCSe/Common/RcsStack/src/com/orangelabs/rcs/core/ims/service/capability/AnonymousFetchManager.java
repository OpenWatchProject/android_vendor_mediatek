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

package com.orangelabs.rcs.core.ims.service.capability;

import java.io.ByteArrayInputStream;
import java.util.List;
import java.util.Vector;

import org.xml.sax.InputSource;

import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.PresenceMultipart;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.ContactInfo;
import com.orangelabs.rcs.core.ims.service.presence.PresenceUtils;
import com.orangelabs.rcs.core.ims.service.presence.pidf.CapabilityDetails;
import com.orangelabs.rcs.core.ims.service.presence.pidf.PidfDocument;
import com.orangelabs.rcs.core.ims.service.presence.pidf.PidfParser;
import com.orangelabs.rcs.core.ims.service.presence.pidf.Tuple;
import com.orangelabs.rcs.core.ims.service.presence.rlmi.ResourceInstance;
import com.orangelabs.rcs.core.ims.service.presence.rlmi.RlmiDocument;
import com.orangelabs.rcs.core.ims.service.presence.rlmi.RlmiParser;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.PhoneUtils;
import com.orangelabs.rcs.utils.logger.Logger;
import java.util.ArrayList;
import javax2.sip.header.SubscriptionStateHeader;

/**
 * Capability discovery manager using anonymous fetch procedure
 * 
 * @author Jean-Marc AUFFRET
 */
public class AnonymousFetchManager implements DiscoveryManager {
       /**
     * IMS module
     */
    private ImsModule imsModule;
    
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * local version of rlmi document
     */
    private static int rlmiLocalDocumentVersion = 0;
    
    /**
     * max subscription for contacts in Presence-list
     */
    private static int maxSubscriptionInPresenceList;
    
    /**
     * Constructor
     * 
     * @param parent IMS module
     */
    public AnonymousFetchManager(ImsModule parent) {
        this.imsModule = parent;
        rlmiLocalDocumentVersion = 0;
        maxSubscriptionInPresenceList = RcsSettings.getInstance().getMaxSubscriptionPresenceList();
        if (logger.isActivated()) {
            logger.debug("AnonymousFetchManager maxSubscriptionInPresenceList value " + maxSubscriptionInPresenceList);
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
        AnonymousFetchRequestTask task = new AnonymousFetchRequestTask(imsModule, contact);
        task.start();
        return true;
    }
    
    /**
     * Request contact capabilities
     * 
     * @param contact Remote contact
     * @return Returns true if success
     */
    public boolean requestCapabilities(List<String> contacts) {
        if (logger.isActivated()) {
            logger.debug("Request capabilities in background for " + contacts);
        }
        
        /**M : added to send multiple request if subscription list is greater than maxSubscriptionInPresenceList mentioned in provisioning
         * 
         */
        int currentIndex = 0;
        int batch = 0;
        int contactsSize = contacts.size();
        List<String> subContactList;
        
        
        while(contactsSize >0){
        	if(contactsSize > maxSubscriptionInPresenceList){
        		if (logger.isActivated())
                logger.debug("Number of contacts are greater than maxSubscriptionInPresenceList-> "+maxSubscriptionInPresenceList+" ; so send request in batches");
        		subContactList = contacts.subList(currentIndex, maxSubscriptionInPresenceList);
        		contactsSize -= maxSubscriptionInPresenceList;
        		currentIndex +=  maxSubscriptionInPresenceList;
        	}else{
        		subContactList = contacts.subList(currentIndex,contacts.size());
        		contactsSize = 0;
        		currentIndex += contacts.size() -1;
        	}
        	
        	batch++;
        	if (logger.isActivated())
             logger.debug("Request capabilities for  "+batch+" batch with " + subContactList.size()+ " contacts ; contacts -> "+subContactList);
        	 AnonymousFetchRequestTask task = new AnonymousFetchRequestTask(imsModule, subContactList);
             task.start();
        }
        /**M :ends */
        
        return true;
       
    }
    
    
    public void receiveNotification(SipRequest notify) {
        if (logger.isActivated()) {
            logger.debug("Anonymous fetch notification received");
        }
        
        //sample out the data for contacts
        String presenceContent = notify.getContent();
        
        if(presenceContent != null)
        {
            if (logger.isActivated()) {
                logger.debug("Anonymous fetch notification presence content is not null");
            }
            try {
                String boundary = notify.getBoundaryContentType();
                if (logger.isActivated()) {
                    logger.debug("Anonymous fetch notification boundary:" + boundary);
                }
                PresenceMultipart multi = new PresenceMultipart(presenceContent, boundary);
                if (multi.isMultipart()) {
                    if (logger.isActivated()) {
                        logger.debug("Anonymous fetch notification multipart data");
                    }
                    // RLMI PART
                    ArrayList<String> rlmiPartArray = multi.getPart("application/rlmi+xml");
                    if (rlmiPartArray != null) {
                        try {
                            String rlmiPart = rlmiPartArray.get(0);
                            // Parse RLMI part
                            InputSource rlmiInput = new InputSource(new ByteArrayInputStream(rlmiPart.getBytes()));
                            RlmiParser rlmiParser = new RlmiParser(rlmiInput);
                            RlmiDocument rlmiInfo = rlmiParser.getResourceInfo();
                            
                            int docVersion = rlmiInfo.getVersion();
                            boolean isFullState = rlmiInfo.isFullState();
                            
                            if (logger.isActivated()) {
                                logger.debug("rlmi document received, version-> "+docVersion+" ; fullstate ->"+isFullState);
                            }
                            //process presence document without checking version for anonymous subscribe,
                            //version rules apply on resource lists as per rfc 4462
                            
                            Vector<ResourceInstance> list = rlmiInfo.getResourceList();
                            for(int i=0; i < list.size(); i++) {
                                ResourceInstance res = (ResourceInstance)list.elementAt(i);
                                String contact = res.getUri();
                                String state = res.getState();
                                String reason = res.getReason();
                                
                                /*
                                 * The <registration-state> element SHALL include either
                                    a) the value active indicating that the Presentity has an active registration with a specific service; or
                                    b) the value terminated indicating that the Presentity does not have an active registration with a specific service. 
                                 * 
                                 */
                                
                                if ((contact != null) && (state != null) && (reason != null)) {
                                    if (state.equalsIgnoreCase("terminated") && reason.equalsIgnoreCase("rejected")) {
                                        // It's a "terminated" event with status "rejected" the contact
                                        // should be removed from the "rcs" list
                                      
                                        if (logger.isActivated()) {
                                            logger.debug("contact : "+contact + " ; rejected the presence sharing request ");
                                    }    

                                        //set the state as offline and reset capabilities 
                                        ContactsManager.getInstance().setContactCapabilities(contact, new Capabilities(), ContactInfo.RCS_REVOKED, ContactInfo.REGISTRATION_STATUS_OFFLINE);
                                        imsModule.getPresenceService().getXdmManager().removeContactFromGrantedList(contact);

                                    }    
                                    else if (state.equalsIgnoreCase("terminated") && reason.equalsIgnoreCase("deactivated")) {
                                        // It's a "terminated" event with status "deactivate" 
                                        //the contacts is a RCS contact but offline
                                      
                                        if (logger.isActivated()) {
                                            logger.debug("contact : "+contact + " ; offline now ");
                                        }
                                        
                                        //set the state as offline and reset capabilities 
                                        ContactsManager.getInstance().setContactCapabilities(contact, new Capabilities(), ContactInfo.RCS_CAPABLE, ContactInfo.REGISTRATION_STATUS_OFFLINE);
                                        
                                        //ImsModule.getPresenceService().getXdmManager().removeContactFromGrantedList(contact);
                                    } else if(state.equalsIgnoreCase("terminated") && reason.equalsIgnoreCase("noresource")) {
                                        // It's a "terminated" event with status "noresource" 
                                        //the contact is a Non-RCS contact as per RFC 3265
                                        if (logger.isActivated()) {
                                            logger.debug("contact : "+contact + " ; not rcs contact ");
                                        }
                                        ContactsManager.getInstance().setContactCapabilities(contact, new Capabilities(), ContactInfo.NOT_RCS, ContactInfo.REGISTRATION_STATUS_UNKNOWN);
                                    }    
                                    
                                    // Notify listener
                                    //getImsModule().getCore().getListener().handlePresenceSharingNotification(contact, state, reason);
                                }
                            }
                        } catch(Exception e) {
                            if (logger.isActivated()) {
                                logger.error("Can't parse RLMI notification", e);
                            }
                        }
                    }
                    
                    
                    /**
                     * PIDF DIFF PART
                     * 
                     */
                    ArrayList<String> pidfPartArray = multi.getPart("application/pidf-diff+xml");
                    if (logger.isActivated()) {
                        logger.debug("pidfPartArray "+ pidfPartArray);
                    }
                    try {
                        
                        if(pidfPartArray != null)
                        {
                        for (String pidfPart : pidfPartArray) 
                        {
                            // Parse PIDF part
                            InputSource pidfInput = new InputSource(new ByteArrayInputStream(pidfPart.getBytes()));
                            PidfParser pidfParser = new PidfParser(pidfInput);
                            PidfDocument presenceInfo = pidfParser.getPresence();
                            if (logger.isActivated()) {
                                logger.debug("pidfPartArray presenceInfo:"+ presenceInfo);
                            }
                            handlePresenceInfo(presenceInfo);
                        }
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't parse PIDF notification", e);
                        }
                    }
                    
                    
                    /**
                     * PIDF PART
                     * 
                     */
                    ArrayList<String> pidffullPartArray = multi.getPart("application/pidf+xml");
                    try {
                        if (logger.isActivated()) {
                            logger.debug("pidffullPartArray "+ pidffullPartArray);
                        }
                        if(pidffullPartArray != null)
                        {
                        for (String pidfPart : pidffullPartArray) 
                        {
                            // Parse PIDF part
                            InputSource pidfInput = new InputSource(new ByteArrayInputStream(pidfPart.getBytes()));
                            PidfParser pidfParser = new PidfParser(pidfInput);
                            PidfDocument presenceInfo = pidfParser.getPresence();
                            if (logger.isActivated()) {
                                logger.debug("pidffullPartArray presenceInfo:"+ presenceInfo);
                            }
                            handlePresenceInfo(presenceInfo);
                        }
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't parse PIDF Full notification", e);
                        }
                    }
                    
                } else if(boundary == null && presenceContent != null) {
                    //handle content without boundary
                    String contentType = notify.getContentType();
                    if (logger.isActivated()) {
                        logger.debug("receiveNotification without boundary, contentType:"+ contentType);
                    }
                    if(contentType.contains("application/pidf+xml") || contentType.contains("application/pidf-diff+xml")){
                        InputSource pidfInput = new InputSource(new ByteArrayInputStream(presenceContent.getBytes()));
                        PidfParser pidfParser = new PidfParser(pidfInput);
                        PidfDocument presenceInfo = pidfParser.getPresence();
                        String number = PhoneUtils.extractNumberFromUri(notify.getFrom());
                        presenceInfo.setEntity(number);
                        if (logger.isActivated()) {
                            logger.debug("receiveNotification without boundary, presenceInfo:"+ presenceInfo + ",number:" + number);
                        }
                        handlePresenceInfo(presenceInfo);
                    }
                }
                
                
            }
            catch(Exception e){
                if (logger.isActivated()) {
                    logger.error("Can't parse presence notification", e);
                }
            }
            
            
            //SUBSCRIPTION STATE HEADER
            // Check subscription state
            SubscriptionStateHeader stateHeader = (SubscriptionStateHeader)notify.getHeader(SubscriptionStateHeader.NAME);
            if ((stateHeader != null) && stateHeader.getState().equalsIgnoreCase("terminated")) {
                if (logger.isActivated()) {
                    logger.info("Presence subscription has been terminated by server");
                }
                //terminatedByServer();
            }
        }  
}



private void handlePresenceInfo(PidfDocument presence)
{
   if(presence!=null)
   {

        // Extract capabilities
       Capabilities capabilities =  new Capabilities();
       int registrationState =  ContactInfo.REGISTRATION_STATUS_UNKNOWN;
       
       // We queried via anonymous fetch procedure, so set presence discovery to true
       //capabilities.setPresenceDiscoverySupport(true);
       
        String contact = presence.getEntity();
        if (logger.isActivated()) {
            logger.debug("receiveCapNotification :" + contact);
        } 
        Vector<Tuple> tuples = presence.getTuplesList();
        
        if(tuples.size()>0)
        {
          //set the registration as true as the device published some capablity
            // We queried via anonymous fetch procedure, so set presence discovery to true
           registrationState =  ContactInfo.REGISTRATION_STATUS_ONLINE;
        
        
        for(int i=0; i < tuples.size(); i++) {
            Tuple tuple = (Tuple)tuples.elementAt(i);
            boolean state = false; 
            if (tuple.getStatus().getBasic().getValue().equals("open")) {
                state = true;
            }
            String id = tuple.getService().getId();
            
            if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_VIDEO_SHARE)) {
                capabilities.setVideoSharingSupport(state);
            } else
            if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_VIDEO_SHARE)) {
                capabilities.setVideoSharingSupport(state);
            } else
            if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IMAGE_SHARE)) {
                capabilities.setImageSharingSupport(state);
            } else
            if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_FT)) {
                //file transfer
                capabilities.setFileTransferSupport(state);
            } else
            if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CS_VIDEO)) {
                capabilities.setCsVideoSupport(state);
            } 
            else
            if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL)) {
            
                if(state)
                {
                    boolean audio = false;
                    boolean video = false;
                    boolean duplexmode = false;
                    
                  for(CapabilityDetails c : tuple.getServiceCapability().getCapabilityList())
                  {
                      //check for audio 
                      if(c.getName().equalsIgnoreCase("audio")){
                          if(c.getValue().equalsIgnoreCase("true"))
                          {
                              audio = true;
                              continue;
                          }
                      }
                      
                    //check for audio 
                      if(c.getName().equalsIgnoreCase("video")){
                          if(c.getValue().equalsIgnoreCase("true"))
                          {
                              video = true;
                              continue;
                          }
                      }
                      
                    //check for duplex
                      if(c.getName().equalsIgnoreCase("duplex")){
                          if(c.getValue().equalsIgnoreCase("full"))
                          {
                              duplexmode = true;
                              continue;
                          }
                      }

                  }
                  
                  if (logger.isActivated()) {
                           logger.debug("Video capability for : "+contact + "  - audio : "+audio + " ; video : " + video + " ; duplex :- " +duplexmode);
                  }
                  
                  capabilities.setIR94_VoiceCall(audio);
                  capabilities.setIR94_VideoCall(video);
                  capabilities.setIR94_DuplexMode(duplexmode);
                }

            }
            else
            if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_DISCOVERY_VIA_PRESENCE)) {
            	 if (logger.isActivated()) {
                     logger.debug("Presence discovery capability for : "+contact+ " : "+state);
            	 }
                    capabilities.setPresenceDiscoverySupport(state);
            }
            else
            if ((id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT))||
                (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT_2))
                ) {
                //IM capability
                capabilities.setImSessionSupport(state);
            }
        }
      }else{
     	 if (logger.isActivated()) {
             logger.debug("No capability tuples found for : "+contact + " ; assuming its offline and update capabilities in database");
            }
     	 
     	  registrationState =  ContactInfo.REGISTRATION_STATUS_OFFLINE;
      }

        if (logger.isActivated()) {
           logger.debug("Set Contact Capability via Presence : "+contact);
          }
        
        if(capabilities.isFileTransferSupported() == false && capabilities.isImSessionSupported() == false){
            if (logger.isActivated()) {
                logger.debug("IM & FT capability is false, so contact is offline");
            }
            registrationState =  ContactInfo.REGISTRATION_STATUS_OFFLINE;
        }
        
        // Update capabilities in database
        ContactsManager.getInstance().setContactCapabilities(contact, capabilities, ContactInfo.RCS_CAPABLE, registrationState);

        // Notify listener
        imsModule.getCore().getListener().handleCapabilitiesNotification(contact, capabilities);
    
   }

  }

}
