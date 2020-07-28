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


package com.mediatek.presence.core.ims.service.capability;

import java.io.ByteArrayInputStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import org.xml.sax.InputSource;

import com.mediatek.presence.core.ims.network.sip.Multipart;
import com.mediatek.presence.core.ims.network.sip.PresenceMultipart;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.service.ContactInfo;
import com.mediatek.presence.core.ims.service.presence.PresenceUtils;
import com.mediatek.presence.core.ims.service.presence.pidf.CapabilityDetails;
import com.mediatek.presence.core.ims.service.presence.pidf.PidfDocument;
import com.mediatek.presence.core.ims.service.presence.pidf.PidfParser;
import com.mediatek.presence.core.ims.service.presence.pidf.Tuple;
import com.mediatek.presence.core.ims.service.presence.rlmi.ResourceInstance;
import com.mediatek.presence.core.ims.service.presence.rlmi.RlmiDocument;
import com.mediatek.presence.core.ims.service.presence.rlmi.RlmiParser;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.DateUtils;
import com.mediatek.presence.utils.PhoneUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.service.api.PresenceServiceImpl;
import java.util.ArrayList;
import java.util.HashSet;
import javax2.sip.header.SubscriptionStateHeader;


public class NotificationHelper {

    private Logger logger = Logger.getLogger(this.getClass().getName());
    private static NotificationHelper mNotificationHelper;
    private ImsModule mImsModule = null;

    public class NotificationInfo {
        String subscriptionState;
        Vector<ResourceInstance> resourceList;
        List<PresentifyInfo> presentifyList = new ArrayList<PresentifyInfo>();
    }

    public class PresentifyInfo {
        int rcsStatus;
        String contact;
        Capabilities capabilities;
    }

    /**
     * Constructor
     */
    private NotificationHelper() {}

    public static NotificationHelper getInstance() {
        if (mNotificationHelper == null) {
            mNotificationHelper = new NotificationHelper();
        }

        return mNotificationHelper;
    }

    public void setImsModule(ImsModule imsModule) {
        mImsModule = imsModule;
    }

    public NotificationInfo parsingNotification(SipRequest notify) {

        NotificationInfo info = new NotificationInfo();
        SubscriptionStateHeader stateHeader = (SubscriptionStateHeader)notify.getHeader(
                SubscriptionStateHeader.NAME);
        info.subscriptionState = stateHeader.getState();

        //sample out the data for contacts
        String presenceContent = notify.getContent();
        if(presenceContent != null) {
            presenceContent = presenceContent.substring(0, presenceContent.lastIndexOf(">") + 1);
            if (logger.isActivated()) {
                logger.debug("Anonymous fetch notification presence content is not null");
            }

            int slotId = mImsModule.getSlotId();

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
                    if (logger.isActivated()) {
                        logger.debug("rlmiPartArray: " + rlmiPartArray);
                    }
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

                            info.resourceList= rlmiInfo.getResourceList();

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
                    String rlmiPart = rlmiPartArray.get(0);
                    InputSource rlmiInput = new InputSource(
                            new ByteArrayInputStream(rlmiPart.getBytes()));
                    RlmiParser rlmiParser = new RlmiParser(rlmiInput);
                    RlmiDocument rlmiInfo = rlmiParser.getResourceInfo();
                    String callId = notify.getCallId();
                    HashMap<String,PidfDocument> pidfDocumentList =
                            new HashMap<String,PidfDocument>();
                    ArrayList<String> pidfPartArray = multi.getPart("application/pidf-diff+xml");
                    if (logger.isActivated()) {
                        logger.debug("pidfPartArray "+ pidfPartArray);
                    }
                    try {
                        if(pidfPartArray != null) {
                            for (String pidfPart : pidfPartArray) {
                                // Parse PIDF part
                                InputSource pidfInput = new InputSource(new ByteArrayInputStream(pidfPart.getBytes()));
                                PidfParser pidfParser = new PidfParser(slotId, pidfInput);
                                PidfDocument presenceInfo = pidfParser.getPresence();

                                if (logger.isActivated()) {
                                    logger.debug("pidfPartArray presenceInfo:"+ presenceInfo);
                                }
                                info.presentifyList.add(parsingPresenceInfo(presenceInfo));
                                pidfDocumentList.put(presenceInfo.getEntity(), presenceInfo);
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
                        if(pidffullPartArray != null) {
                            for (String pidfPart : pidffullPartArray) {
                                // Parse PIDF part
                                InputSource pidfInput = new InputSource(new ByteArrayInputStream(pidfPart.getBytes()));
                                PidfParser pidfParser = new PidfParser(slotId, pidfInput);
                                PidfDocument presenceInfo = pidfParser.getPresence();
                                if (logger.isActivated()) {
                                    logger.debug("pidffullPartArray presenceInfo:"+ presenceInfo);
                                }
                                info.presentifyList.add(parsingPresenceInfo(presenceInfo));
                                pidfDocumentList.put(presenceInfo.getEntity(), presenceInfo);
                            }
                        }
                    } catch(Exception e) {
                        if (logger.isActivated()) {
                            logger.error("Can't parse PIDF Full notification", e);
                        }
                    }
                    //AOSP procedure
                    PresenceServiceImpl.receiveContactListCapability(
                            callId, rlmiInfo, pidfDocumentList);

                } else if(boundary == null && presenceContent != null) {
                    //handle content without boundary
                    String contentType = notify.getContentType();
                    if (logger.isActivated()) {
                        logger.debug("receiveNotification without boundary, contentType:"+ contentType);
                    }
                    if(contentType.contains("application/pidf+xml") || contentType.contains("application/pidf-diff+xml")) {
                        InputSource pidfInput = new InputSource(new ByteArrayInputStream(presenceContent.getBytes()));
                        PidfParser pidfParser = new PidfParser(slotId, pidfInput);
                        PidfDocument presenceInfo = pidfParser.getPresence();
                        if (presenceInfo != null) {
                            String number = PhoneUtils.extractNumberFromUri(notify.getFrom());
                            if (presenceInfo.getEntity() == null) {
                                presenceInfo.setEntity(number);
                            }
                            if (logger.isActivated()) {
                                logger.debug("receiveNotification without boundary, presenceInfo:"+ presenceInfo + ",number:" + number);
                            }
                            info.presentifyList.add(parsingPresenceInfo(presenceInfo));
                            //AOSP procedure
                            PresenceServiceImpl.receiveContactCapability(presenceInfo);

                        }
                    }
                }


            } catch(Exception e) {
                if (logger.isActivated()) {
                    logger.error("Can't parse presence notification", e);
                }
            }
        }
        return info;
    }

    private PresentifyInfo parsingPresenceInfo(PidfDocument presence) {

        String contact = presence.getEntity();
        if (logger.isActivated()) {
            logger.debug("paringPresenceInfo :" + contact);
        }

        PresentifyInfo info = new PresentifyInfo();
        Capabilities capabilities =  ContactsManager.getInstance().getContactCapabilities(contact);
        if (capabilities == null) {
            capabilities = new Capabilities();
        }
        int registrationState =  ContactInfo.REGISTRATION_STATUS_UNKNOWN;

        long timestamp;
        if (presence.getPerson() == null) {
             timestamp = System.currentTimeMillis();
             logger.debug("Person is null, so use system time: " + DateUtils.encodeDate(timestamp));
        }else {
            timestamp = presence.getPerson().getTimestamp();
            logger.debug("timestamp :" + DateUtils.encodeDate(timestamp));
        }
        capabilities.setTimestamp(timestamp);

        // We queried via anonymous fetch procedure, so set presence discovery to true
        //capabilities.setPresenceDiscoverySupport(true);

        Vector<Tuple> tuples = presence.getTuplesList();
        logger.debug("tuples size: " + tuples.size());
        if(tuples.size()>0) {
            //set the registration as true as the device published some capablity
            // We queried via anonymous fetch procedure, so set presence discovery to true
            registrationState =  ContactInfo.REGISTRATION_STATUS_ONLINE;

            HashSet <String> serviceList = new HashSet <String>();

            for(int i=0; i < tuples.size(); i++) {
                Tuple tuple = (Tuple)tuples.elementAt(i);
                logger.debug("tuple id: " + tuple.getId());
                boolean state = false;
                if (tuple.getStatus().getBasic().getValue().equals("open")) {
                    state = true;
                }
                String id = tuple.getService().getId();

                if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_VIDEO_SHARE)) {
                    if (serviceList.contains(PresenceUtils.FEATURE_RCS2_VIDEO_SHARE)) {
                        state = state |capabilities.isVideoSharingSupported();
                    }
                    capabilities.setVideoSharingSupport(state);
                } else if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IMAGE_SHARE)) {
                    if (serviceList.contains(PresenceUtils.FEATURE_RCS2_IMAGE_SHARE)) {
                        state = state |capabilities.isImageSharingSupported();
                    }
                    capabilities.setImageSharingSupport(state);
                } else if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_FT)) {
                    //file transfer
                    if (serviceList.contains(PresenceUtils.FEATURE_RCS2_FT)) {
                        state = state |capabilities.isFileTransferSupported();
                    }
                    capabilities.setFileTransferSupport(state);
                } else if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CS_VIDEO)) {
                    if (serviceList.contains(PresenceUtils.FEATURE_RCS2_CS_VIDEO)) {
                        state = state |capabilities.isCsVideoSupported();
                    }
                    capabilities.setCsVideoSupport(state);
                } else if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL)) {

                    if(state) {
                        boolean audio = false;
                        boolean video = false;
                        boolean duplexmode = false;

                        for(CapabilityDetails c : tuple.getServiceCapability().getCapabilityList()) {
                            //check for audio
                            if(c.getName().equalsIgnoreCase("audio")) {
                                if(c.getValue().equalsIgnoreCase("true")) {
                                    audio = true;
                                    continue;
                                }
                            }

                            //check for audio
                            if(c.getName().equalsIgnoreCase("video")) {
                                if(c.getValue().equalsIgnoreCase("true")) {
                                    video = true;
                                    continue;
                                }
                            }

                            //check for duplex
                            if(c.getName().equalsIgnoreCase("duplex")) {
                                if(c.getValue().equalsIgnoreCase("full")) {
                                    duplexmode = true;
                                    continue;
                                }
                            }

                        }
                        if (serviceList.contains(PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL)) {
                            audio = audio | capabilities.isIR94_VoiceCallSupported();
                            video = video | capabilities.isIR94_VideoCallSupported();
                            duplexmode = duplexmode | capabilities.isIR94_DuplexModeSupported();
                        }

                        if (logger.isActivated()) {
                            logger.debug("Video capability for : "+contact + "  - audio : "+audio + " ; video : " + video + " ; duplex :- " +duplexmode);
                        }

                        capabilities.setIR94_VoiceCall(audio);
                        capabilities.setIR94_VideoCall(video);
                        capabilities.setIR94_DuplexMode(duplexmode);
                    }

                } else if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_DISCOVERY_VIA_PRESENCE)) {
                    capabilities.setPresenceDiscoverySupport(state);
                } else if ((id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT)) ||
                           (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT_2))) {
                     if (serviceList.contains(PresenceUtils.FEATURE_RCS2_CHAT) ||
                        serviceList.contains(PresenceUtils.FEATURE_RCS2_CHAT_2) ) {
                        state = state |capabilities.isImSessionSupported();
                    }
                    //IM capability
                    capabilities.setImSessionSupport(state);
                } else if (id.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_STANDALONEMSG)) {
                    if (serviceList.contains(PresenceUtils.FEATURE_RCS2_STANDALONEMSG)) {
                        state = state |capabilities.isStandaloneMsgSupport();
                    }
                    capabilities.setStandaloneMsgSupport(state);
                }
                serviceList.add(id);
            }
        } else {
            if (logger.isActivated()) {
                logger.debug("No capability tuples found for : "+contact + " ; assuming its offline and update capabilities in database");
            }
        }

        info.contact = contact;
        info.capabilities = capabilities;
        info.rcsStatus = registrationState;
        return info;
    }
}
