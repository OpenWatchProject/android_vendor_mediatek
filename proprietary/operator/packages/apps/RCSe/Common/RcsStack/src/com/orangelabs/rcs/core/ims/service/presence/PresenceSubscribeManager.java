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

package com.orangelabs.rcs.core.ims.service.presence;

import gov2.nist.core.NameValue;
import java.io.ByteArrayInputStream;
import java.text.ParseException;
import java.util.Vector;

import javax2.sip.header.AcceptHeader;
import javax2.sip.header.ContactHeader;
import javax2.sip.header.EventHeader;
import javax2.sip.header.ExtensionHeader;
import javax2.sip.header.Header;
import javax2.sip.header.SubscriptionStateHeader;
import javax2.sip.header.SupportedHeader;

import org.xml.sax.InputSource;

import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.core.ims.network.sip.FeatureTags;
import com.orangelabs.rcs.core.ims.network.sip.Multipart;
import com.orangelabs.rcs.core.ims.network.sip.SipMessageFactory;
import com.orangelabs.rcs.core.ims.network.sip.SipUtils;
import com.orangelabs.rcs.core.ims.protocol.sip.SipDialogPath;
import com.orangelabs.rcs.core.ims.protocol.sip.SipException;
import com.orangelabs.rcs.core.ims.protocol.sip.SipRequest;
import com.orangelabs.rcs.core.ims.service.presence.pidf.Contact;
import com.orangelabs.rcs.core.ims.service.presence.pidf.PidfDocument;
import com.orangelabs.rcs.core.ims.service.presence.pidf.PidfParser;
import com.orangelabs.rcs.core.ims.service.presence.rlmi.ResourceInstance;
import com.orangelabs.rcs.core.ims.service.presence.rlmi.RlmiDocument;
import com.orangelabs.rcs.core.ims.service.presence.rlmi.RlmiParser;
import com.orangelabs.rcs.provider.eab.ContactsManager;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Subscribe manager for presence event
 * 
 * @author jexa7410
 */
public class PresenceSubscribeManager extends SubscribeManager {
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private static int rlmiLocalDocumentVersion = 0;
    /**
     * Constructor
     * 
     * @param parent IMS module
     */
    public PresenceSubscribeManager(ImsModule parent) {
        super(parent);
    }

    /**
     * Returns the presentity
     * 
     * @return Presentity
     */
    public String getPresentity() {
        //return ImsModule.IMS_USER_PROFILE.getPublicUri()+";pres-list=rcs";
        
        String temp_username = "sip:"+ImsModule.IMS_USER_PROFILE.getUsername()+"@"+ImsModule.IMS_USER_PROFILE.getHomeDomain();
        
    
        //added for tmo
        return temp_username+";pres-list=rcs";

//    return temp_username+";pres-list=rcs";
    }    
    
    public void setExpirePeriod(int expirePeriod){
        super.setExpirePeriod(expirePeriod);
    }
    
    /**
     * Create a SUBSCRIBE request
     * 
     * @param dialog SIP dialog path
     * @param expirePeriod Expiration period
     * @return SIP request
     * @throws SipException
     */
    public SipRequest createSubscribe(SipDialogPath dialog, int expirePeriod) throws SipException {
        // Create SUBSCRIBE message
        SipRequest subscribe = null;
        
        if(!RcsSettings.getInstance().isCPMSupported()){        
             subscribe = SipMessageFactory.createSubscribe(dialog, expirePeriod);
        }
        else{
            if (logger.isActivated()) {
                logger.info("createSubscribe CPMS");
            } 
            subscribe = SipMessageFactory.createPresenceSubscribe(dialog, expirePeriod,null);
        }

        
        //check if video capab supported
        if(RcsSettings.getInstance().isIR94VideoCallSupported()){
            try {
                String vilteString  = "" +FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL;
                ContactHeader contactHeader = (ContactHeader)subscribe.getHeader(SipUtils.HEADER_CONTACT_);
                contactHeader.setParameter(new NameValue(vilteString, null, true));
            } catch (ParseException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        
        
        // Set the Event header
        subscribe.addHeader(EventHeader.NAME, "presence");

        subscribe.addHeader("Accept-Contact", "*;+g.oma.sip-im;explicit;require");
        // Set the Accept header
 //      subscribe.addHeader(AcceptHeader.NAME, "application/pidf+xml, application/rlmi+xml, multipart/related");
      String acceptHeaderValue ;   
     if(RcsSettings.getInstance().isSupportOP08()){ 
           acceptHeaderValue = "application/pidf+xml, application/rlmi+xml, multipart/related";
      }else{
           acceptHeaderValue = "application/pidf+xml";
      }    

        
        subscribe.addHeader(AcceptHeader.NAME, acceptHeaderValue);

        // Set the Supported header
        subscribe.addHeader(SupportedHeader.NAME, "eventlist");

        subscribe.addHeader("Proxy-Require", "sec-agree");
        subscribe.addHeader("Require", "sec-agree");
        

        return subscribe;
    }

    
   

    
    /**
     * Receive a notification
     * 
     * @param notify Received notify
     */
    public void receiveNotification(SipRequest notify) {
        // Check notification
        if (!isNotifyForThisSubscriber(notify)) {
            return;
        }        
        
        if (logger.isActivated()) {
            logger.debug("New presence notification received");
        }        

        // Parse XML part
        String content = notify.getContent();
        if (content != null) {
            try {
                String boundary = notify.getBoundaryContentType();
                if (logger.isActivated()) {
                    logger.debug("Presence notification :" + boundary);
                } 
                Multipart multi = new Multipart(content, boundary);
                if (multi.isMultipart()) {
                    // RLMI
                    String rlmiPart = multi.getPart("application/rlmi+xml");
                    if (rlmiPart != null) {
                        try {
                            // Parse RLMI part
                            InputSource rlmiInput = new InputSource(new ByteArrayInputStream(rlmiPart.getBytes()));
                            RlmiParser rlmiParser = new RlmiParser(rlmiInput);
                            RlmiDocument rlmiInfo = rlmiParser.getResourceInfo();
                            int docVersion = rlmiInfo.getVersion();
                            boolean isFullState = rlmiInfo.isFullState();
                            
                            if(docVersion <= rlmiLocalDocumentVersion){
                            	if (logger.isActivated()) {
                                    logger.debug("server document version "+docVersion+" is discarded");
                                }
                            	return;
                            }else{
                            	//update the version
                            	rlmiLocalDocumentVersion = docVersion;
                            	if (logger.isActivated()) {
                                    logger.debug("local rlmi document version is updated to  : "+rlmiLocalDocumentVersion);
                                }
                            	
                            	if(isFullState){
                            		if (logger.isActivated()) {
                                        logger.debug("fullstate true for higher version: "+docVersion +" ; flush all eab contacts");
                                    }
                                	
                            		//check full state
                                	ContactsManager.getInstance().flushContactProvider();	
                            	}
                            	
                            }
                            
                            
                            
                            
                            Vector<ResourceInstance> list = rlmiInfo.getResourceList();
                            for(int i=0; i < list.size(); i++) {
                                ResourceInstance res = (ResourceInstance)list.elementAt(i);
                                String contact = res.getUri();
                                String state = res.getState();
                                String reason = res.getReason();
                                
                                if ((contact != null) && (state != null) && (reason != null)) {
                                    if (state.equalsIgnoreCase("terminated") && reason.equalsIgnoreCase("rejected")) {
                                        // It's a "terminated" event with status "rejected" the contact
                                        // should be removed from the "rcs" list
                                        getImsModule().getPresenceService().getXdmManager().removeContactFromGrantedList(contact);
                                    }                
                                    
                                    // Notify listener
                                    getImsModule().getCore().getListener().handlePresenceSharingNotification(
                                            contact, state, reason);
                                }
                            }
                        } catch(Exception e) {
                            if (logger.isActivated()) {
                                logger.error("Can't parse RLMI notification", e);
                            }
                        }
                    }

                    // PIDF 
                    String pidfPart = multi.getPart("application/pidf+xml");
                    try {
                        // Parse PIDF part
                        InputSource pidfInput = new InputSource(new ByteArrayInputStream(pidfPart.getBytes()));
                        PidfParser pidfParser = new PidfParser(pidfInput);
                        PidfDocument presenceInfo = pidfParser.getPresence();
                        
                        // Notify listener
                        getImsModule().getCore().getListener().handlePresenceInfoNotification(
                                presenceInfo.getEntity(), presenceInfo);
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't parse PIDF notification", e);
                        }
                    }
                }
            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't parse presence notification", e);
                }
            }
            
            // Check subscription state
            SubscriptionStateHeader stateHeader = (SubscriptionStateHeader)notify.getHeader(SubscriptionStateHeader.NAME);
            if ((stateHeader != null) && stateHeader.getState().equalsIgnoreCase("terminated")) {
                if (logger.isActivated()) {
                    logger.info("Presence subscription has been terminated by server");
                }
                terminatedByServer();
            }
        }
    }   
}
