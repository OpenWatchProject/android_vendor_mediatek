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

import java.io.ByteArrayInputStream;
import java.util.ArrayList;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax2.sip.ListeningPoint;

import org.xml.sax.SAXException;
import org.w3c.dom.Document;
import org.w3c.dom.Node;

import android.telephony.SubscriptionManager;
import android.os.AsyncTask;
import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;
import com.android.ims.ImsException;
import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMode;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMethod;

import com.mediatek.ims.rcsua.Capability;
import com.mediatek.ims.rcsua.RcsUaService;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.messaging.RichMessagingHistory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;
import com.orangelabs.rcs.utils.CloseableUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * Provisioning parser
 *
 * @author jexa7410
 */
public class ProvisioningParser {
    
    private boolean mFirst = false;
    /**
     * Parameter type text
     */
    private static final int TYPE_TXT = 0;
    
    /**
     * Parameter IVR
     */
    
    private static int ir94VideoAuthValue=0;
    
    /**
     * Parameter type integer
     */
    private static final int TYPE_INT = 1;
    
    /**
     * Provisioning info
     */
    private ProvisioningInfo provisioningInfo = new ProvisioningInfo();

    /**
     * Content
     */
    private String content;

    //rcs service
    boolean isModify = false; //add for auto config service
    Capabilities myCapabilities = null; //add for auto config service
    // feature tag
    Capability myFeatureCap = null; //add for auto config service
    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private enum RootNodeType {
        VERS, TOKEN, MSG, APPLICATION, IMS, PRESENCE, XDMS, IM,
        CAPDISCOVERY, APN, OTHER, SERVICES, SUPL, SERVICEPROVIDEREXT, CPM, UX
    }

    /**
     * Constructor
     *
     * @param content Content
     */
    public ProvisioningParser(String content) {
        this.content = content;
    }

    /**
     * Returns provisioning info
     *
     * @return Provisioning info
     */
    public ProvisioningInfo getProvisioningInfo() {
        return provisioningInfo;
    }

    /**
     * Parse the provisioning document
     *
     * @param gsmaRelease
     *            The GSMA release (Albatros, Blackbird, Crane...) before parsing
     * 
     * @return Boolean result
     */
    public boolean parse(int gsmaRelease,  MessagingMode messagingMode, boolean first) {
        ByteArrayInputStream inputStream = null;
        try {
            if (logger.isActivated()) {
                logger.debug("Start the parsing of content");
            }
            mFirst = first;
            inputStream = new ByteArrayInputStream(content.getBytes());
            DocumentBuilderFactory dfactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dbuilder = dfactory.newDocumentBuilder();
            Document doc = dbuilder.parse(inputStream);
            if (doc == null) {
                throw new SAXException("The provisioning document is null");
            }

            if (logger.isActivated()) {
                logger.debug("Parsed Doc ="+doc);
            }
            
            Node rootnode = doc.getDocumentElement();
            Node childnode = rootnode.getFirstChild();
            if (childnode == null) {
                throw new SAXException("The first child node in document is null");
            }

            int nodeNumber = 0;
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        Node typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null && typenode.getNodeValue() != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            nodeNumber++;
                            String nodeType = typenode.getNodeValue().toUpperCase();
                            try {
                                RootNodeType rootNodeType = RootNodeType.valueOf(nodeType);
                                switch (rootNodeType) {
                                case VERS:
                                    parseVersion(childnode);
                                    break;
                                case TOKEN:
                                    parseToken(childnode);
                                    break;
                                case MSG:
                                    parseTermsMessage(childnode);
                                    break;
                                case APPLICATION:
                                    parseApplication(childnode);
                                    break;
                                case IMS:
                                    parseIMS(childnode);
                                    break;
                                case PRESENCE:
                                    parsePresence(childnode);
                                    break;
                                case XDMS:
                                    parseXDMS(childnode);
                                    break;
                                case IM:
                                    parseIM(childnode);
                                    break;
                                case CAPDISCOVERY:
                                    parseCapabilityDiscovery(childnode);
                                    break;
                                case APN:
                                    parseAPN(childnode);
                                    break;
                                case OTHER:
                                    parseOther(childnode);
                                    break;
                                case SERVICES:
                                    parseServices(childnode);
                                    break;
                                case SUPL:
                                    parseSupl(childnode);
                                    break;
                                case SERVICEPROVIDEREXT:
                                    parseServiceProviderExt(childnode);
                                    break;
                                case CPM:
                                    parseCPM(childnode);
                                    break;
                                default:
                                    if (logger.isActivated()) {
                                        logger.warn("unhandled node type: " + nodeType);
                                    }
                                    break;
                                }
                            } catch (IllegalArgumentException e) {
                                if (logger.isActivated()) {
                                    logger.warn("invalid node type: " + nodeType);
                                }
                            }
                        }
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
            if (nodeNumber ==1) {
                // We received a single node (the version one) !
                // This is the case if the version number is negative or in order to extend the validity of the provisioning.
                // In that case we restore the relevant GSMA release saved before parsing.
                RcsSettings.getInstance().setGsmaRelease("" + gsmaRelease);
                RcsSettings.getInstance().setMessagingMode(messagingMode);
            }
            return true;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't parse content", e);
            }
            return false;
        } finally {
            CloseableUtils.tryToClose(inputStream);
        }
    }

    /**
     * Parse the provisioning message document
     *
     * 
     * @return Boolean result
     */
    public boolean parseMsg() {
        try {
            if (logger.isActivated()) {
                logger.debug("Start the parsing msg of content");
            }
            ByteArrayInputStream mInputStream = new ByteArrayInputStream(content.getBytes());
            DocumentBuilderFactory dfactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dbuilder = dfactory.newDocumentBuilder();
            Document doc = dbuilder.parse(mInputStream);
            mInputStream.close();
            mInputStream = null;
            if (doc == null) {
                if (logger.isActivated()) {
                    logger.debug("The document is null");
                }
                return false;
            }
            
            Node rootnode = doc.getDocumentElement();
            Node childnode = rootnode.getFirstChild();
            if (childnode == null) {
                if (logger.isActivated()) {
                    logger.debug("The first chid node is null");
                }
                return false;
            }

            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        Node typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("MSG")) {
                                parseTermsMessage(childnode);
                            } 
                        }
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
            
            return true;
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("Can't parse content", e);
            }
            return false;
        }
    }

    /**
     * Parse the provisioning version
     *
     * @param node Node
     */
    private void parseVersion(Node node) {
        String version = null;
        String validity = null;
        if (node == null) {
            return;
        }
        Node versionchild = node.getFirstChild();

        if (versionchild != null) {
            do {
                if (version == null) {
                    if ((version = getValueByParamName("version", versionchild, TYPE_TXT)) != null) {
                        provisioningInfo.setVersion(version);
                        continue;
                    }
                }
                if (validity == null) {
                    if ((validity = getValueByParamName("validity", versionchild, TYPE_INT)) != null) {
                        provisioningInfo.setValidity(Long.parseLong(validity));
                        continue;
                    }
                }
            } while((versionchild = versionchild.getNextSibling()) != null);
        }
    }
    
    /**
     * Parse the provisioning Token
     *
     * @param node Node
     */
    private void parseToken(Node node) {
        String version = null;
        String validity = null;
        if (node == null) {
            return;
        }
        Node tokenChild = node.getFirstChild();

        if (tokenChild != null) {
            do {
                if (version == null) {
                    if ((version = getValueByParamName("token", tokenChild, TYPE_TXT)) != null) {
                        provisioningInfo.setToken(version);
                        continue;
                    }
                }
                if (validity == null) {
                    if ((validity = getValueByParamName("validity", tokenChild, TYPE_INT)) != null) {
                        provisioningInfo.setTokenValidity(Long.parseLong(validity));
                        continue;
                    }
                }
            } while((tokenChild = tokenChild.getNextSibling()) != null);
        }
    }

    /**
     * Parse terms message
     *
     * @param node Node
     */
    private void parseTermsMessage(Node node) {
        String title = null;
        String message = null;
        String acceptBtn = null;
        String rejectBtn = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (title == null) {
                    if ((title = getValueByParamName("title", childnode, TYPE_TXT)) != null) {
                        provisioningInfo.setTitle(title);
                        continue;
                    }
                }
                if (message == null) {
                    if ((message = getValueByParamName("message", childnode, TYPE_TXT)) != null) {
                        provisioningInfo.setMessage(message);
                        continue;
                    }
                }
                if (acceptBtn == null) {
                    if ((acceptBtn = getValueByParamName("Accept_btn", childnode, TYPE_INT)) != null) {
                        if (acceptBtn.equals("1")) {
                            provisioningInfo.setAcceptBtn(true);
                        } else {
                            provisioningInfo.setAcceptBtn(false);
                        }
                        continue;
                    }
                }
                if (rejectBtn == null) {
                    if ((rejectBtn = getValueByParamName("Reject_btn", childnode, TYPE_INT)) != null) {
                        if (rejectBtn.equals("1")) {
                            provisioningInfo.setRejectBtn(true);
                        } else {
                            provisioningInfo.setRejectBtn(false);
                        }
                        continue;
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse the application infos
     *
     * @param node Node
     */
    private void parseApplication(Node node) {
        String appId = null;
        String name = null;
        String appRef = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (appId == null) {
                    if ((appId = getValueByParamName("AppID", childnode, TYPE_TXT)) != null) {
                        if (logger.isActivated()) {
                            logger.debug("App ID: " + appId);
                        }
                        continue;
                    }
                }
                if (name == null) {
                    if ((name = getValueByParamName("Name", childnode, TYPE_TXT)) != null) {
                        if (logger.isActivated()) {
                            logger.debug("App name: " + name);
                        }
                        continue;
                    }
                }
                if (appRef == null) {
                    if ((appRef = getValueByParamName("AppRef", childnode, TYPE_TXT)) != null) {
                        if (logger.isActivated()) {
                            logger.debug("App ref: " + appRef);
                        }
                        continue;
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
        }

        if (appRef != null && (appRef.equalsIgnoreCase("IMS-Settings") || appRef.equalsIgnoreCase("ims-rcse")|| appRef.equalsIgnoreCase("ims"))){
            parseIMS(node);
        }


        if (appRef != null && (appRef.equalsIgnoreCase("RCSe-Settings") || appRef.equalsIgnoreCase("rcs"))) {
            parseRCSe(node);
        }
    }

    /**
     * Parse presence favorite link
     *
     * @param node Node
     */
    private void parseFavoriteLink(Node node) {
        // Not supported
    }

    /**
     * Parse presence watcher
     *
     * @param node Node
     */
    private void parsePresenceWatcher(Node node) {
        // TODO: "FetchAuth"
        // TODO: "ContactCapPresAut"
    }

    /**
     * Parse presentity watcher
     * 
     * @param node Node
     */
    private void parsePresentityWatcher(Node node) {
        // TODO: "WATCHERFETCHAUTH"
    }

    /**
     * Parse Vip Contacts for presence
     * 
     * @param node Node
     */
    private void parseVipContacts(Node node) {
        String nonVipPollPeriodSetting = null;
        String nonVipMaxPollPerPeriod = null;
        
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (nonVipPollPeriodSetting == null) {
                    if ((nonVipPollPeriodSetting = getValueByParamName("NonVipPollPeriodSetting", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.NON_VIP_POLL_PERIOD,
                                nonVipPollPeriodSetting);
                        continue;
                    }
                } else
                if (nonVipMaxPollPerPeriod == null) {
                    if ((nonVipMaxPollPerPeriod = getValueByParamName("NonVipMaxPollPerPeriod", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.NON_VIP_MAX_POLL_PER_PERIOD,
                                nonVipMaxPollPerPeriod);
                        continue;
                    }
                }    

                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }


    /**
     * Parse presence
     *
     * @param node Node
     */
    private void parsePresence(Node node) {
        String usePresence = null;
        String presencePrfl = null;
        String iconMaxSize = null;
        String noteMaxSize = null;
        String publishTimer = null;
        String clientObjDataLimit = null;
        String contentServerUri = null;
        String sourceThrottlePublish = null;
        String maxSubPresence = null;
        String serviceUriTemplate = null;
        String rlsUri = null;
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("FAVLINK")) {
                                parseFavoriteLink(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("SERVCAPWATCH")) {
                                parsePresenceWatcher(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("ServCapPresentity")) {
                                parsePresentityWatcher(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("VIPCONTACTS")) {
                                parseVipContacts(childnode);
                            }
                        }
                    }
                }

                if (usePresence == null) {
                    if ((usePresence = getValueByParamName("usePresence", childnode, TYPE_INT)) != null) {
                        if (usePresence.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (presencePrfl == null) {
                    if ((presencePrfl = getValueByParamName("presencePrfl", childnode, TYPE_INT)) != null) {
                        if (presencePrfl.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (iconMaxSize == null) {
                    if ((iconMaxSize = getValueByParamName("IconMaxSize", childnode, TYPE_INT)) != null) {
                        int kb = Integer.parseInt(iconMaxSize) / 1024;
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_PHOTO_ICON_SIZE, ""+kb);
                        continue;
                    }
                }

                if (noteMaxSize == null) {
                    if ((noteMaxSize = getValueByParamName("NoteMaxSize", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_FREETXT_LENGTH, noteMaxSize);
                        continue;
                    }
                }

                if (publishTimer == null) {
                    if ((publishTimer = getValueByParamName("PublishTimer", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.PUBLISH_EXPIRE_PERIOD, publishTimer);
                        continue;
                    }
                }

                if (clientObjDataLimit == null) {
                    if ((publishTimer = getValueByParamName("client-obj-datalimit", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.CLIENT_OBJ_DATA_LIMIT, clientObjDataLimit);
                        continue;
                    }
                }
                
                if (contentServerUri == null) {
                    if ((contentServerUri = getValueByParamName("content-serveruri", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.CONTENT_SERVER_URI, contentServerUri);
                        continue;
                    }
                }
                
                if (sourceThrottlePublish == null) {
                    if ((sourceThrottlePublish = getValueByParamName("source-throttlepublish", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.SOURCE_THROTTLE_PUBLISH, sourceThrottlePublish);
                        continue;
                    }
                }
                
                if (maxSubPresence == null) {
                    if ((maxSubPresence = getValueByParamName("max-number-ofsubscriptions-inpresence-list", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_SUBSCRIPTION_PRESENCE, maxSubPresence);
                        continue;
                    }
                }
                
                if (serviceUriTemplate == null) {
                    if ((serviceUriTemplate = getValueByParamName("service-uritemplate", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.SERVICE_URI_TEMPLATE, serviceUriTemplate);
                        continue;
                    }
                }
                
                if (rlsUri == null) {
                    if ((rlsUri = getValueByParamName("RLS-URI", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.RLS_URI, rlsUri);
                        continue;
                    }
                }

                // Not supported: "AvailabilityAuth"
                // Not supported: "client-obj-datalimit"
                // Not used for RCS: "content-serveruri"
                // Not supported: "source-throttlepublish"
                // Not supported: "max-number-ofsubscriptions-inpresence-list"
                // TODO: "service-uritemplate"
                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    
    /**
     * Parse CPM
     *
     * @param node Node
     */
    private void parseCPM(Node node) {
        /*String usePresence = null;
        String presencePrfl = null;
        String iconMaxSize = null;
        String noteMaxSize = null;
        String publishTimer = null;*/
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("StandaloneMsg")) {
                                parseStandaloneMsg(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("MessageStore")) {
                                //parseMessageStore(childnode);
                            } 
                        }
                    }
                }
                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }
    
    
    
    /**
     * Parse StandaloneMsg
     *
     * @param node Node
     */
    private void parseStandaloneMsg(Node node) {
        
        String maxSizeStandalone = null;
       
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (maxSizeStandalone == null) {
                    if ((maxSizeStandalone = getValueByParamName("MaxSizeStandalone", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.MAX_STANDALONE_SIZE,
                                maxSizeStandalone);
                        continue;
                    }
                }

                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse services
     *
     * @param node Node
     */
    private void parseServices(Node node) {
        String presencePrfl = null;
        String chatAuth = null;        
        String groupChatAuth = null;        
        String ftAuth = null;
        String geolocPushAuth = null;
        String vsAuth = null;
        String isAuth = null;
        String rcsIPVoiceCallAuth = null;
        String rcsIPVideoCallAuth = null;
        String ir94VideoAuth = null;
        String standaloneMsgAuth = null;
        
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            // Node "SERVICES" is mandatory in GSMA release Blackbird and not present in previous one Albatros.
            // Only if the parsing result contains a SERVICE tree, Blackbird is assumed as release.
            // This trick is used to detect the GSMA release as provisioned by the network.
            RcsSettings.getInstance().setGsmaRelease(RcsSettingsData.VALUE_GSMA_REL_BLACKBIRD );
            do {

                if (chatAuth == null) {
                    if ((chatAuth = getValueByParamName("ChatAuth", childnode, TYPE_INT)) != null) {
                        if (chatAuth.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                 if (groupChatAuth == null) {
                     if ((groupChatAuth = getValueByParamName("groupChatAuth", childnode, TYPE_INT)) != null) {
                         if (groupChatAuth.equals("1")) {
                             RcsSettings.getInstance().writeParameter(
                                     RcsSettingsData.CAPABILITY_IM_GROUP_SESSION, RcsSettingsData.TRUE);
                         } else {
                             RcsSettings.getInstance().writeParameter(
                                     RcsSettingsData.CAPABILITY_IM_GROUP_SESSION, RcsSettingsData.FALSE);
                         }
                         continue;
                     }
                 }

                if (ftAuth == null) {
                    if ((ftAuth = getValueByParamName("ftAuth", childnode, TYPE_INT)) != null) {
                        if (ftAuth.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (vsAuth == null) {
                    if ((vsAuth = getValueByParamName("vsAuth", childnode, TYPE_INT)) != null) {
                        if (vsAuth.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_VIDEO_SHARING, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_VIDEO_SHARING, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (isAuth == null) {
                    if ((isAuth = getValueByParamName("isAuth", childnode, TYPE_INT)) != null) {
                        if (isAuth.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IMAGE_SHARING, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IMAGE_SHARING, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                if (geolocPushAuth == null) {
                    if ((geolocPushAuth = getValueByParamName("geolocPushAuth", childnode, TYPE_INT)) != null) {
                        if (geolocPushAuth.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (presencePrfl == null) {
                    if ((presencePrfl = getValueByParamName("presencePrfl", childnode, TYPE_INT)) != null) {
                        if (presencePrfl.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                if (rcsIPVoiceCallAuth == null) {
                    if ((rcsIPVoiceCallAuth = getValueByParamName("rcsIPVoiceCallAuth", childnode, TYPE_INT)) != null) {
                        if (logger.isActivated()) {
                            logger.debug("node name = rcsIPVoiceCallAuth");
                        }
                        int value =Integer.decode(rcsIPVoiceCallAuth);
                        if ((value % 16) == 0){
                            RcsSettings.getInstance().writeParameter(
                                  RcsSettingsData.CAPABILITY_IP_VOICE_CALL, RcsSettingsData.FALSE);
                        }
                        else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IP_VOICE_CALL, RcsSettingsData.TRUE);
                        }

                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.BE_IPVOICECALL_AUTH, String.valueOf(value));
                        
                        continue;
                    }
                }
                
                if (rcsIPVideoCallAuth == null) {
                    if ((rcsIPVideoCallAuth = getValueByParamName("rcsIPVideoCallAuth", childnode, TYPE_INT)) != null) {
                        if (logger.isActivated()) {
                            logger.debug("node name = rcsIPVideoCallAuth");
                        }
                        int value =Integer.decode(rcsIPVideoCallAuth);
                        if ((value % 16) == 0){
                            RcsSettings.getInstance().writeParameter(
                                  RcsSettingsData.CAPABILITY_IP_VIDEO_CALL, RcsSettingsData.FALSE);
                        }
                        else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IP_VIDEO_CALL, RcsSettingsData.TRUE);
                        }
                        
                      RcsSettings.getInstance().writeParameter(
                      RcsSettingsData.BE_IPVIDEOCALL_AUTH, String.valueOf(value));
                        
                        continue;
                    }
                }

                if (ir94VideoAuth == null) {
                    if ((ir94VideoAuth = getValueByParamName("IR94VideoAuth", childnode, TYPE_INT)) != null) {
                        if (ir94VideoAuth.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IR94_VIDEO_AUTH, RcsSettingsData.TRUE);
                            ir94VideoAuthValue=1;
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IR94_VIDEO_AUTH, RcsSettingsData.FALSE);
                            ir94VideoAuthValue=0;
                        }
                        AsyncTask.execute(new Runnable() {
                            public void run() {
                                int subId=0;
                                subId=(int)SubscriptionManager.getDefaultDataSubscriptionId();
                                if (logger.isActivated()) {
                                    logger.error("setIVR value subId" + subId);
                                }
                                if(subId == -1){
                                     subId=(int)SubscriptionManager.getDefaultSubscriptionId();
                                }
                                ImsConfig imsCfg = null;
                                ImsManager imsMgr = ImsManager.getInstance(AndroidFactory.getApplicationContext(),subId);
                                
                                
                                try {
                                    imsCfg = imsMgr.getConfigInterface();
                                    imsCfg.setProvisionedValue(ImsConfig.ConfigConstants.LVC_SETTING_ENABLED,ir94VideoAuthValue);
                                } catch (ImsException e) {
                                  
                                    e.printStackTrace();
                                }
                        }
                        });
                        continue;
                    }
                }
                
                if (standaloneMsgAuth == null) {
                    if ((standaloneMsgAuth = getValueByParamName("standaloneMsgAuth", childnode, TYPE_INT)) != null) {
                        if (standaloneMsgAuth.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.STANDALONE_MSG_SUPPORT, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.STANDALONE_MSG_SUPPORT, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                // Not used: "standaloneMsgAuth"
                // Not used: "geolocPullAuth"
                // TODO: "beIPVoiceCallAuth"
                // TODO: "beIPVideoCallAuth"
                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }


    /**
     * Parse XDMS
     *
     * @param node Node
     */
    private void parseXDMS(Node node) {
        String revokeTimer = null;
        String xcapRootURI = null;
        String xcapAuthenticationUsername = null;
        String xcapAuthenticationSecret = null;
        String xcapAuthenticationType = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (revokeTimer == null) {
                    if ((revokeTimer = getValueByParamName("RevokeTimer", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.REVOKE_TIMEOUT,
                                revokeTimer);
                        continue;
                    }
                }

                if (xcapRootURI == null) {
                    if ((xcapRootURI = getValueByParamName("XCAPRootURI", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.XDM_SERVER, xcapRootURI);
                        continue;
                    }
                }

                if (xcapAuthenticationUsername == null) {
                    if ((xcapAuthenticationUsername = getValueByParamName(
                            "XCAPAuthenticationUserName", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.XDM_LOGIN,
                                xcapAuthenticationUsername);
                        continue;
                    }
                }

                if (xcapAuthenticationSecret == null) {
                    if ((xcapAuthenticationSecret = getValueByParamName(
                            "XCAPAuthenticationSecret", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.XDM_PASSWORD,
                                xcapAuthenticationSecret);
                        continue;
                    }
                }
                
                if (xcapAuthenticationType == null) {
                    if ((xcapAuthenticationSecret = getValueByParamName(
                            "XCAPAuthenticationType", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.XDM_AUTH_TYPE,
                                xcapAuthenticationSecret);
                        continue;
                    }
                }

                // used (Digest or GBA ): "XCAPAuthenticationType"
                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse supl services
     *
     * @param node Node
     */
    private void parseSupl(Node node) {
        String textMaxLength = null;
        String locInfoMaxValidTime = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (textMaxLength == null) {
                    if ((textMaxLength = getValueByParamName("TextMaxLength", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.MAX_GEOLOC_LABEL_LENGTH,
                                textMaxLength);
                        continue;
                    }
                }

                if (locInfoMaxValidTime == null) {
                    if ((locInfoMaxValidTime = getValueByParamName("LocInfoMaxValidTime", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.GEOLOC_EXPIRATION_TIME, locInfoMaxValidTime);
                        continue;
                    }
                }

                // Not used: "geolocPullOpenValue"
                // Not used: "geolocPullApiGwAddress"
                // Not used: "geolocPullBlockTimer"
                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }
    
    /**
     * Parse  Profile
     *
     * @param node Node
     */
    private void parseMessageStore(Node node) {
        String addr = null;
        String addrType = null;

        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (addr == null) {
                    if ((addr = getValueByParamName("Url", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.MSGSTORE_ADDRESS,addr);   
                        continue;
                    }
                }

                if (addrType == null) {
                    if ((addrType = getValueByParamName("UserName", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.MSGSTORE_USER,addrType);    
                        continue;
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse service provider ext
     * 
     * @param node Node
     */
    private void parseServiceProviderExt(Node node) {
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();
        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (typenode.getNodeValue().equalsIgnoreCase("joyn")) {
                                parseJoyn(childnode);
                            }
                        }
                    }
                }
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }
    
    /**
     * Parse joyn
     * 
     * @param node Node
     */
    private void parseJoyn(Node node) {
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();
        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (typenode.getNodeValue().equalsIgnoreCase("UX")) {
                                parseUx(childnode);
                            }
                            
                            // @tct-stack huangbo add begin
                            if (typenode.getNodeValue().equalsIgnoreCase("Messaging")) {
                                parseMessaging(childnode);
                            }
                            // @tct-stack huangbo add end
                        }
                    }
                }
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }
    
    
    
    /**
     * Parse service capability ext
     * 
     * @param node Node
     */
    private void parseCapExt(Node node) {
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();
        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (typenode.getNodeValue().equalsIgnoreCase("joyn")) {
                                parseCapJoyn(childnode);
                            }
                        }
                    }
                }
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }
    
    /**
     * Parse Capability joyn
     * 
     * @param node Node
     */
    private void parseCapJoyn(Node node) {
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();
        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (typenode.getNodeValue().equalsIgnoreCase("msgCapValidity")) {
                                parseCapValidity(childnode);
                            }
                        }
                    }
                }
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }
    
    
    /**
     * Parse Capability Validity
     * 
     * @param node Node
     */
    private void parseCapValidity(Node node) {
        String capValidity = null;

        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {

                if (capValidity == null) {
                    if ((capValidity = getValueByParamName("msgCapValidity", childnode, TYPE_INT)) != null) {
                        
                            RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MESSAGING_CAPABILITIES_VALIDITY, capValidity);
                        
                        continue;
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
        }

        
    }
    
    
    /**
     * Parse Ux
     * 
     * @param node Node
     */
    private void parseUx(Node node) {
        String messagingUX = null;
        String e2eIPCallLabel = null;
        String breakoutIPCallLabel = null;
        String e2eVoiceCapabilityHandling = null;
        
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {

                if (messagingUX == null) {
                    if ((messagingUX = getValueByParamName("messagingUX", childnode, TYPE_INT)) != null) {
                        if (messagingUX.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.CONVERGENT_MESSAGING_UX, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.CONVERGENT_MESSAGING_UX, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (e2eIPCallLabel == null) {
                    if ((e2eIPCallLabel = getValueByParamName("e2eIPCallLabel", childnode, TYPE_TXT)) != null) {                       
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.IPCALL_E2E_LABEL, e2eIPCallLabel);
                        continue;
                    }
                }
                
                if (breakoutIPCallLabel == null) {
                    if ((breakoutIPCallLabel = getValueByParamName("breakoutIPCallLabel", childnode, TYPE_TXT)) != null) {                       
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.IPCALL_BREAKOUT_LABEL, breakoutIPCallLabel);
                        continue;
                    }
                }
                
                if ((e2eVoiceCapabilityHandling = getValueByParamName(")", childnode, TYPE_INT)) != null) {
                    if (e2eVoiceCapabilityHandling.equals("0")) {
                        RcsSettings.getInstance().writeParameter(
                            RcsSettingsData.IPCALL_E2E_VOICECAPABILITYHANDLING, RcsSettingsData.FALSE);
                    } else {
                        RcsSettings.getInstance().writeParameter(
                            RcsSettingsData.IPCALL_E2E_VOICECAPABILITYHANDLING, RcsSettingsData.TRUE);
                    }
                    continue;
                }

            } while((childnode = childnode.getNextSibling()) != null);
        }

        // Not used: oneButtonVoiceCall
        // Not used: oneButtonVideoCall
    }
    
    /**
     * Parse IM
     *
     * @param node Node
     */
    private void parseIM(Node node) {
        String imMsgTech = null;
        String firstMessageInvite = null;
        String multiMediaChat = null;
        String imCapAlwaysOn = null;
        String ftCapAlwaysOn = null;
        String imWarnSF = null;
        String imSessionStart = null;
        String ftWarnSize = null;
        String autoAcceptFt = null;
        String ftHttpCsUri = null;
        String ftHttpCsUser = null;
        String ftHttpCsPwd = null;
        String ftDefaultMech = null;
        String ftSF = null;
        String chatAuth = null;
        String smsFallBackAuth = null;
        String autoAcceptChat = null;
        String autoAcceptGroupChat = null;
        String maxSize1to1 = null;
        String maxSize1toM = null;
        String timerIdle = null;
        String maxSizeFileTransfer = null;
        String ftThumb = null;
        String maxAdhocGroupSize = null;
        String confFctyUri = null;
        String confMultiFctyUri = null;
        String groupChatSF = null;
        String maxConcurrentSession = null;
        String massFctyUri= null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (imCapAlwaysOn == null) {
                    if ((imCapAlwaysOn = getValueByParamName("imCapAlwaysON", childnode, TYPE_INT)) != null) {
                        boolean _imCapAlwaysOn = !imCapAlwaysOn.equals("0");
                        if (imCapAlwaysOn.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IM_CAPABILITY_ALWAYS_ON,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IM_CAPABILITY_ALWAYS_ON,
                                    RcsSettingsData.TRUE);
                        }
                        if (mFirst) {
                            if (_imCapAlwaysOn) {
                                RcsSettings.getInstance().setDefaultMessagingMethod(MessagingMethod.RCS);
                            } else {
                                RcsSettings.getInstance().setDefaultMessagingMethod(MessagingMethod.AUTOMATIC);
                            }
                        }
                        continue;
                    }
                }
                
                if (ftCapAlwaysOn == null) {
                    if ((ftCapAlwaysOn = getValueByParamName("ftCapAlwaysON", childnode, TYPE_INT)) != null) {
                        if (ftCapAlwaysOn.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.FT_CAPABILITY_ALWAYS_ON,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.FT_CAPABILITY_ALWAYS_ON,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (maxConcurrentSession == null) {
                    if ((maxConcurrentSession = getValueByParamName("MaxConcurrentSession", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_CHAT_SESSIONS,
                                maxConcurrentSession);
                        continue;
                    }
                }

                if (groupChatSF == null) {
                    if ((groupChatSF = getValueByParamName("GroupChatFullStandFwd", childnode, TYPE_INT)) != null) {
                        if (groupChatSF.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_GROUP_CHAT_SF,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_GROUP_CHAT_SF,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
                
                if (imWarnSF == null) {
                    if ((imWarnSF = getValueByParamName("imWarnSF", childnode, TYPE_INT)) != null) {
                        if (imWarnSF.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.WARN_SF_SERVICE,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.WARN_SF_SERVICE,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (autoAcceptFt == null) {
                    if ((autoAcceptFt = getValueByParamName("ftAutAccept", childnode, TYPE_INT)) != null) {
                        boolean aaModeChangeable = !autoAcceptFt.equals("0");
                        if (mFirst
                                ||(aaModeChangeable !=RcsSettings.getInstance().isFtAutoAcceptedModeChangeable())) {
                        if (autoAcceptFt.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER,
                                    RcsSettingsData.FALSE);
                                RcsSettings.getInstance().setFileTransferAutoAcceptedInRoaming(false);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER,
                                    RcsSettingsData.TRUE);
                                RcsSettings.getInstance().setFileTransferAutoAcceptedInRoaming(false);
                            }
                        }
                        continue;
                    }
                }

                if (ftSF == null) {
                    if ((ftSF = getValueByParamName("ftStAndFwEnabled", childnode, TYPE_INT)) != null) {
                        if (ftSF.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_SF,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_SF,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (ftHttpCsUri == null) {
                    if ((ftHttpCsUri = getValueByParamName("ftHTTPCSURI", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.FT_HTTP_SERVER, ftHttpCsUri);
                        continue;
                    }
                }

                if (ftHttpCsUser == null) {
                    if ((ftHttpCsUser = getValueByParamName("ftHTTPCSUser", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.FT_HTTP_LOGIN, ftHttpCsUser);
                        continue;
                    }
                }

                if (ftHttpCsPwd == null) {
                    if ((ftHttpCsPwd = getValueByParamName("ftHTTPCSPwd", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.FT_HTTP_PASSWORD, ftHttpCsPwd);
                        continue;
                    }
                }

                if (ftDefaultMech == null) {
                    if ((ftDefaultMech = getValueByParamName("ftDefaultMech", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.FT_PROTOCOL, ftDefaultMech);
                        continue;
                    }
                }

                if (imSessionStart == null) {
                    if ((imSessionStart = getValueByParamName("imSessionStart", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.IM_SESSION_START,
                                imSessionStart);
                        continue;
                    }
                }

                if (ftWarnSize == null) {
                    if ((ftWarnSize = getValueByParamName("ftWarnSize", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.WARN_FILE_TRANSFER_SIZE, ftWarnSize);
                        continue;
                    }
                }

                if (chatAuth == null) {
                    if ((chatAuth = getValueByParamName("ChatAuth", childnode, TYPE_INT)) != null) {
                        if (chatAuth.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (smsFallBackAuth == null) {
                    if ((smsFallBackAuth = getValueByParamName("SmsFallBackAuth", childnode, TYPE_INT)) != null) {
                        if (smsFallBackAuth.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.SMS_FALLBACK_SERVICE, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.SMS_FALLBACK_SERVICE, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (autoAcceptChat == null) {
                    if ((autoAcceptChat = getValueByParamName("AutAccept", childnode, TYPE_INT)) != null) {
                        if (autoAcceptChat.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.AUTO_ACCEPT_CHAT,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.AUTO_ACCEPT_CHAT,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (autoAcceptGroupChat == null) {
                    if ((autoAcceptGroupChat = getValueByParamName("AutAcceptGroupChat", childnode, TYPE_INT)) != null) {
                        if (autoAcceptGroupChat.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (maxSize1to1 == null) {
                    if ((maxSize1to1 = getValueByParamName("MaxSize1to1", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_CHAT_MSG_LENGTH, maxSize1to1);
                        continue;
                    }
                }

                if (maxSize1toM == null) {
                    if ((maxSize1toM = getValueByParamName("MaxSize1toM", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_GROUPCHAT_MSG_LENGTH, maxSize1toM);
                        continue;
                    }
                }

                if (timerIdle == null) {
                    if ((timerIdle = getValueByParamName("TimerIdle", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.CHAT_IDLE_DURATION, timerIdle);
                        continue;
                    }
                }

                if (maxSizeFileTransfer == null) {
                    if ((maxSizeFileTransfer = getValueByParamName("MaxSizeFileTr", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_FILE_TRANSFER_SIZE, maxSizeFileTransfer);
                        continue;
                    }
                }

                if (ftThumb == null) {
                    if ((ftThumb = getValueByParamName("ftThumb", childnode, TYPE_INT)) != null) {
                        if (ftThumb.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL, RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL, RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
                
                if (maxAdhocGroupSize == null) {
                    if ((maxAdhocGroupSize = getValueByParamName("max_adhoc_group_size", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_CHAT_PARTICIPANTS, maxAdhocGroupSize);
                        continue;
                    }
                }

                if (confFctyUri == null) {
                    if ((confFctyUri = getValueByParamName("conf-fcty-uri", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.IM_CONF_URI, formatSipUri(confFctyUri));
                        continue;
                    }
                }

                if (massFctyUri == null) {
                    if ((massFctyUri = getValueByParamName("mass-fcty-uri", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.IM_MASS_URI, formatSipUri(massFctyUri));
                        continue;
                    }
                }
                
                if (confMultiFctyUri == null) {
                    if ((confMultiFctyUri = getValueByParamName("multi-conf-fcty-uri", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.IM_MULTI_CONF_URI, formatSipUri(confMultiFctyUri));
                        continue;
                    }
                }


                if (imMsgTech == null) {
                    if ((imMsgTech = getValueByParamName("imMsgTech", childnode, TYPE_INT)) != null) {
                        if (imMsgTech.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CPM_TECHNOLOGY,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CPM_TECHNOLOGY,
                                    RcsSettingsData.TRUE);
                            
                            //add conversation id column in database in case CPM is supported
                            if(RichMessagingHistory.getInstance()==null)  {
                                RichMessagingHistory.createInstance(AndroidFactory.getApplicationContext());
                            }
                            RichMessagingHistory.getInstance().addColumnConversationID();
                        }
                        continue;
                    }
                }
                
                if (firstMessageInvite == null) {
                    if ((firstMessageInvite = getValueByParamName("firstMessageInvite", childnode, TYPE_INT)) != null) {
                        if (firstMessageInvite.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.FIRST_MSG_IN_INVITE,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.FIRST_MSG_IN_INVITE,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
                
                if (multiMediaChat == null) {
                    if ((multiMediaChat = getValueByParamName("multiMediaChat", childnode, TYPE_INT)) != null) {
                        if (multiMediaChat.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.MULTIMEDIA_IN_CHAT,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.MULTIMEDIA_IN_CHAT,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
                // Not used for RCS: "pres-srv-cap"
                // Not used for RCS: "deferred-msg-func-uri"
                // Not used for RCS: "exploder-uri"
                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }


    /**
     * Parse capability discovery
     * 
     * @param node Node
     */
    private void parseCapabilityDiscovery(Node node) {
        String pollingPeriod = null;
        String capInfoExpiry = null;
        String presenceDiscovery = null;
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("Ext")) {
                                parseCapExt(childnode);
                            }
                        }
                    }
                }
                
                if (pollingPeriod == null) {
                    if ((pollingPeriod = getValueByParamName("pollingPeriod", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.CAPABILITY_POLLING_PERIOD, pollingPeriod);
                        continue;
                    }
                }

                if (capInfoExpiry == null) {
                    if ((capInfoExpiry = getValueByParamName("capInfoExpiry", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.CAPABILITY_EXPIRY_TIMEOUT, capInfoExpiry);
                        continue;
                    }
                }

                if (presenceDiscovery == null) {
                    if ((presenceDiscovery = getValueByParamName("presenceDisc", childnode, TYPE_INT)) != null) {
                        if (presenceDiscovery.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY,
                                    RcsSettingsData.FALSE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse APN
     *
     * @param node Node
     */
    private void parseAPN(Node node) {
        // Not supported: "rcseOnlyAPN"
        // Not supported: "enableRcseSwitch"
        
        String enableRcseSwitch = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (enableRcseSwitch == null) {
                    if ((enableRcseSwitch = getValueByParamName("enableRcseSwitch", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.ENABLE_RCS_SWITCH, enableRcseSwitch);
                        continue;
                    }
                }
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse transport protocol
     *
     * @param node Node
     */
    private void parseTransportProtocol(Node node) {
        String psSignalling = null;
        String psMedia = null;
        String psRTMedia= null;
        String wifiSignalling = null;
        String wifiMedia = null;
        String wifiRtMedia = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (psSignalling == null) {
                    if ((psSignalling = getValueByParamName("psSignalling", childnode, TYPE_TXT)) != null) {
                        if (psSignalling.equals("SIPoUDP")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_MOBILE,
                                    ListeningPoint.UDP);
                        } else
                        if (psSignalling.equals("SIPoTCP")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_MOBILE,
                                    ListeningPoint.TCP);
                        } else
                        if (psSignalling.equals("SIPoTLS")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_MOBILE,
                                    ListeningPoint.TLS);
                        }
                        continue;
                    }
                }

                /**
                 * M: added 2 new paarmeters for mobile 
                 */
                if (psMedia == null) {
                    if ((psMedia = getValueByParamName("psMedia", childnode, TYPE_TXT)) != null) {
                        if (psMedia.equals("MSRP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_MSRP_OVER_MOBILE,
                                    RcsSettingsData.FALSE);
                        } else
                        if (psMedia.equals("MSRPoTLS")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_MSRP_OVER_MOBILE,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
                      
                
                if (psRTMedia == null) {
                    if ((psRTMedia = getValueByParamName("psRTMedia", childnode, TYPE_TXT)) != null) {
                        if (psRTMedia.equals("RTP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_RTP_OVER_MOBILE,
                                    RcsSettingsData.FALSE);
                        } else
                        if (psRTMedia.equals("SRTP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_RTP_OVER_MOBILE,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
                  
                /**@
                 * */
                
                
                if (wifiSignalling == null) {
                    if ((wifiSignalling = getValueByParamName("wifiSignalling", childnode, TYPE_TXT)) != null) {
                        if (wifiSignalling.equals("SIPoUDP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_WIFI,
                                    ListeningPoint.UDP);
                        } else
                        if (wifiSignalling.equals("SIPoTCP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_WIFI,
                                    ListeningPoint.TCP);
                        } else
                        if (wifiSignalling.equals("SIPoTLS")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_WIFI,
                                    ListeningPoint.TLS);
                        }
                        continue;
                    }
                }

                if (wifiMedia == null) {
                    if ((wifiMedia = getValueByParamName("wifiMedia", childnode, TYPE_TXT)) != null) {
                        if (wifiMedia.equals("MSRP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_MSRP_OVER_WIFI,
                                    RcsSettingsData.FALSE);
                        } else
                        if (wifiMedia.equals("MSRPoTLS")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_MSRP_OVER_WIFI,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }

                if (wifiRtMedia == null) {
                    if ((wifiRtMedia = getValueByParamName("wifiRTMedia", childnode, TYPE_TXT)) != null) {
                        if (wifiMedia.equals("RTP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_RTP_OVER_WIFI,
                                    RcsSettingsData.FALSE);
                        } else
                        if (wifiMedia.equals("SRTP")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.SECURE_RTP_OVER_WIFI,
                                    RcsSettingsData.TRUE);
                        }
                        continue;
                    }
                }
                
                // Not supported: "psMedia"
                // Not supported: "psRTMedia"
                
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse other
     *
     * @param node Node
     */
    private void parseOther(Node node) {
        String endUserConfReqId = null;
        String deviceID = null;
        String beIPCallBreakOut = null;
        String beIPCallBreakOutCS = null;
        String beIPVideoCallUpgradeFromCS = null;
        String beIPVideoCallUpgradeOnCapError = null;
        String beIPVideoCallUpgradeAttemptEarly = null;
        
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("transportProto")) {
                                parseTransportProtocol(childnode);
                            }
                        }
                    }
                }

                if (endUserConfReqId == null) {
                    if ((endUserConfReqId = getValueByParamName("endUserConfReqId", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.ENDUSER_CONFIRMATION_URI, formatSipUri(endUserConfReqId));
                        continue;
                    }
                }

                if (deviceID == null) {
                    if ((deviceID = getValueByParamName("deviceID", childnode, TYPE_INT)) != null) {
                        if (deviceID.equals("0")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.USE_IMEI_AS_DEVICE_ID, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.USE_IMEI_AS_DEVICE_ID, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (beIPCallBreakOut == null) {
                    if ((beIPCallBreakOut = getValueByParamName("beIPCallBreakOut", childnode, TYPE_INT)) != null) {
                        if (beIPCallBreakOut.equals("1")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVOICECALL_BREAKOUT, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVOICECALL_BREAKOUT, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                if (beIPCallBreakOutCS == null) {
                    if ((beIPCallBreakOutCS = getValueByParamName("beIPCallBreakOutCS", childnode, TYPE_INT)) != null) {
                        if (beIPCallBreakOutCS.equals("1")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVOICECALL_BREAKOUT_CS, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVOICECALL_BREAKOUT_CS, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                if (beIPVideoCallUpgradeFromCS == null) {
                    if ((beIPVideoCallUpgradeFromCS = getValueByParamName("beIPVideoCallUpgradeFromCS", childnode, TYPE_INT)) != null) {
                        if (beIPVideoCallUpgradeFromCS.equals("1")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_FROM_CS, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_FROM_CS, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                if (beIPVideoCallUpgradeOnCapError == null) {
                    if ((beIPVideoCallUpgradeOnCapError = getValueByParamName("beIPVideoCallUpgradeOnCapError", childnode, TYPE_INT)) != null) {
                        if (beIPVideoCallUpgradeOnCapError.equals("1")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_ON_CAPERROR, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_ON_CAPERROR, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                if (beIPVideoCallUpgradeAttemptEarly == null) {
                    if ((beIPVideoCallUpgradeAttemptEarly = getValueByParamName("beIPVideoCallUpgradeAttemptEarly", childnode, TYPE_INT)) != null) {
                        if (beIPVideoCallUpgradeAttemptEarly.equals("1")) {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_ATTEMPT_EARLY, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_ATTEMPT_EARLY, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                // Not supported: "WarnSizeImageShare"
                
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse connection reference
     *
     * @param node Node
     */
    private void parseConRefs(Node node) {
        String conRef = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (conRef == null) {
                    if ((conRef = getValueByParamName("ConRef", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.RCS_APN, conRef);
                        continue;
                    }
                }
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse public user identity
     *
     * @param node Node
     */
    private void parsePublicUserIdentity(Node node) {
        String publicUserIdentity = null;
        String publicUserIdentity1 = null;
        String publicUserIdentity2 = null;
        String publicUserIdentity3 = null;
        String publicUserIdentityPC = null;
        String publicUserIdentity_tel = null;
        ArrayList<String> address= new ArrayList<String>();
        String telURI= null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (publicUserIdentity == null) {
                    if ((publicUserIdentity = getValueByParamName("Public_User_Identity", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_USERNAME_FULL, publicUserIdentity);//@tct-stack wuquan add for full publi_user_identity but not only number part
                        String username = extractUserNamePart(publicUserIdentity);
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_USERNAME, username);
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_DISPLAY_NAME, username);
                        continue;
                    }
                }

                if (publicUserIdentity1 == null) {
                    if ((publicUserIdentity1 = getValueByParamName("Public_User_Identity1", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.PUBLIC_USER_IDENTITY_1, publicUserIdentity1);
                        if(isTelURI(publicUserIdentity1))
                            telURI=extractMSISDNPart(publicUserIdentity1);
                        else 
                            address.add(publicUserIdentity1);
                        continue;
                    }
                    
                }

                if (publicUserIdentity2 == null) {
                    if ((publicUserIdentity2 = getValueByParamName("Public_User_Identity2", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.PUBLIC_USER_IDENTITY_2, publicUserIdentity2);
                        if(isTelURI(publicUserIdentity2))
                            telURI=extractMSISDNPart(publicUserIdentity2);
                        else 
                            address.add(publicUserIdentity2);
                        continue;
                    }
                }

                if (publicUserIdentity3 == null) {
                    if ((publicUserIdentity3 = getValueByParamName("Public_User_Identity3", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.PUBLIC_USER_IDENTITY_3, publicUserIdentity3);
                        if(isTelURI(publicUserIdentity3))
                            telURI=extractMSISDNPart(publicUserIdentity3);
                        else 
                            address.add(publicUserIdentity3);
                        continue;
                    }
                }

                if (publicUserIdentityPC == null) {
                    if ((publicUserIdentityPC = getValueByParamName("Public_User_IdentityPC", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.PUBLIC_USER_IDENTITY_PC, publicUserIdentityPC);
                        continue;
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
        }
        if(telURI != null) {
            if((publicUserIdentity_tel= getTelURI(address,telURI)) != null) {
                RcsSettings.getInstance().writeParameter(
                        RcsSettingsData.USERPROFILE_IMS_USERNAME_FULL, publicUserIdentity_tel);
                String username = extractUserNamePart(publicUserIdentity_tel);
                RcsSettings.getInstance().writeParameter(
                        RcsSettingsData.USERPROFILE_IMS_USERNAME, username);
                RcsSettings.getInstance().writeParameter(
                        RcsSettingsData.USERPROFILE_IMS_DISPLAY_NAME, username);
            }
        }
        else if(!address.isEmpty()) {
            publicUserIdentity1 = address.get(0);
            RcsSettings.getInstance().writeParameter(
                    RcsSettingsData.USERPROFILE_IMS_USERNAME_FULL, publicUserIdentity1);
            String username = extractUserNamePart(publicUserIdentity1);
                RcsSettings.getInstance().writeParameter(
                        RcsSettingsData.USERPROFILE_IMS_USERNAME, username);
                RcsSettings.getInstance().writeParameter(
                        RcsSettingsData.USERPROFILE_IMS_DISPLAY_NAME, username);
            }
    }

    /**
     * Parse the secondary device parameter
     *
     * @param node Node
     */
    private void parseSecondaryDevicePar(Node node) {
        // TODO: to be managed thks to a flag main/secondary device
        /*
        String voiceCall = null;
        String chat = null;
        String sendSms = null;
        String fileTranfer = null;
        String videoShare = null;
        String imageShare = null;
        String geolocPush = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (chat == null) {
                    if ((chat = getValueByParamName("Chat", childnode, TYPE_INT)) != null) {
                        if (chat.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (fileTranfer == null) {
                    if ((fileTranfer = getValueByParamName("FileTranfer", childnode, TYPE_INT)) != null) {
                        if (fileTranfer.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_FILE_TRANSFER, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (videoShare == null) {
                    if ((videoShare = getValueByParamName("VideoShare", childnode, TYPE_INT)) != null) {
                        if (videoShare.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_VIDEO_SHARING, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_VIDEO_SHARING, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (imageShare == null) {
                    if ((imageShare = getValueByParamName("ImageShare", childnode, TYPE_INT)) != null) {
                        if (imageShare.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IMAGE_SHARING, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_IMAGE_SHARING, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                if (geolocPush == null) {
                    if ((geolocPush = getValueByParamName("GeoLocPush", childnode, TYPE_INT)) != null) {
                        if (geolocPush.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }
                
                // Not used for RCS: "SendSms"
                // Not used for RCS: "VoiceCall"
                
            } while ((childnode = childnode.getNextSibling()) != null);
        }*/
    }

    /**
     * Parse ext
     *
     * @param node Node
     */
    private void parseExt(Node node) {
        String intUrlFmt = null;
        String maxSizeImageShare = null;
        String maxTimeVideoShare = null;
        String rcsVolteSingleRegistration = null;
        String maxTimeAudiomessage = null;
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("SecondaryDevicePar")) {
                                parseSecondaryDevicePar(childnode);
                            }
                        }
                    }
                }

                if (intUrlFmt == null) {
                    if ((intUrlFmt = getValueByParamName("IntUrlFmt", childnode, TYPE_INT)) != null) {
                        if (intUrlFmt.equals("0")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.TEL_URI_FORMAT, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.TEL_URI_FORMAT, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (maxSizeImageShare == null) {
                    if ((maxSizeImageShare = getValueByParamName("MaxSizeImageShare", childnode, TYPE_INT)) != null) {
                        int kb = Integer.parseInt(maxSizeImageShare) / 1024;
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_IMAGE_SHARE_SIZE, ""+kb);
                        continue;
                    }
                }

                if (maxTimeVideoShare == null) {
                    if ((maxTimeVideoShare = getValueByParamName("MaxTimeVideoShare", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.MAX_VIDEO_SHARE_DURATION, maxTimeVideoShare);
                        continue;
                    }
                }
                
                if (rcsVolteSingleRegistration == null) {
                    if ((rcsVolteSingleRegistration = getValueByParamName("rcsVolteSingleRegistration", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION, rcsVolteSingleRegistration);
                        continue;
                    }
                }
                
                if (maxTimeAudiomessage == null) {
                    if ((maxTimeAudiomessage = getValueByParamName("MaxTimeAudioMessage",
                            childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.MAX_AUDIO_MESSAGE_DURATION,
                                maxTimeAudiomessage);
                    }
                }
                // Not used (all number are formatted in international format): "NatUrlFmt"
                // Not supported: "Q-Value"
                
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse ICSI
     *
     * @param node Node
     */
    private void parseICSI(Node node) {
        // Not used for RCS: "ICSI"
        // Not used for RCS: "ICSI_Resource_Allocation_Mode"
    }

    /**
     * Parse PCSCF address
     *
     * @param node Node
     */
    private void parsePcscfAddress(Node node) {
        String addr = null;
        if (node == null) {
            return;
        }
        ArrayList<String> address= new ArrayList<String>();
        ArrayList<String> port= new ArrayList<String>();
        Node childnode = node.getFirstChild();
        int nbAddress=0;
        if (logger.isActivated()) {
            logger.debug("Rcse Parsing Pcscf Address Start");
        }
        if (childnode != null) {
            do {
                if (addr == null) {
                    if ((addr = getValueByParamName("Address", childnode, TYPE_TXT)) != null) {
                        
                        String[] address1 = addr.split(":");
                        if (logger.isActivated()) {
                            logger.debug("Rcse Parsing Pcscf Address Start"+ nbAddress);
                        }
                       
                        
                        address.add(nbAddress,address1[0]);
                        if (logger.isActivated()) {
                            logger.debug("Rcse Parsing inserting"+ address1[0]+"value  "+address.get(nbAddress));
                        }
                        if (address1.length > 1) {
                            port.add(nbAddress,address1[1]);
                        }
                        nbAddress++;
                        if (logger.isActivated()) {
                            logger.debug("Rcse Parsing Pcscf Address Start"+ nbAddress);
                        }
              
                        continue;
                    }
                }

                // Not used: "AddressType"
                
            } while((childnode = childnode.getNextSibling()) != null);
            RcsSettings.getInstance().ResetPCSCFAddress();
            RcsSettings.getInstance().writeParameter(
                    RcsSettingsData.NUMBER_OF_PCSCF_ADDRESS,Integer.toString(nbAddress));
            for(int i=0;i<nbAddress;i++){
                
                if (logger.isActivated()) {
                    logger.debug("Rcse Parsing Pcscf Address adding table entry"+ i);
                }
                if (logger.isActivated()) {
                    logger.debug("Rcse Parsing inserting value inside For loop "+address.get(i));
                }
                String key_mobile="ImsOutboundProxyAddrForMobile";
                String key_wifi="ImsOutboundProxyAddrForWifi";
                String key_mobile_port="ImsOutboundProxyPortForMobile";
                String key_wifi_port="ImsOutboundProxyPortForWifi";
                if(i==0)
                {
                    
                    RcsSettings.getInstance().writeParameter(key_mobile,address.get(i));
                    RcsSettings.getInstance().writeParameter(key_wifi,address.get(i));
                    if(!port.isEmpty()){
                    RcsSettings.getInstance().writeParameter(key_mobile_port,port.get(i));
                    RcsSettings.getInstance().writeParameter(key_wifi_port,port.get(i));
                    }
                    else {
                        RcsSettings.getInstance().writeParameter(key_mobile_port,"5060");
                        RcsSettings.getInstance().writeParameter(key_wifi_port,"5060");
                    } 
                }else{
                int sufix=i+1;
                 key_mobile="ImsOutboundProxyAddrForMobile"+sufix;
                 key_wifi="ImsOutboundProxyAddrForWifi"+sufix;
                 key_mobile_port="ImsOutboundProxyPortForMobile"+sufix;
                 key_wifi_port="ImsOutboundProxyPortForWifi"+sufix;
                RcsSettings.getInstance().insertParameter(key_mobile,address.get(i));
                RcsSettings.getInstance().insertParameter(key_wifi,address.get(i));
                if(!port.isEmpty()){
                RcsSettings.getInstance().insertParameter(key_mobile_port,port.get(i));
                RcsSettings.getInstance().insertParameter(key_wifi_port,port.get(i));
                }
                else {
                    RcsSettings.getInstance().insertParameter(key_mobile_port,"5060");
                    RcsSettings.getInstance().insertParameter(key_wifi_port,"5060");
                    }
                }    
            }
        }
    }

    /**
     * Parse phone context
     *
     * @param node Node
     */
    private void parsePhoneContextList(Node node) {
        // Not used: "PhoneContext"
        // Not used: "Public_user_identity"
    }

    /**
     * Parse application authentication
     *
     * @param node Node
     */
    private void parseAppAuthent(Node node) {
        String authType = null;
        String realm = null;
        String userName = null;
        String userPwd = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (authType == null) {
                    if ((authType = getValueByParamName("AuthType", childnode, TYPE_TXT)) != null) {
                        if (authType.equals("EarlyIMS")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE,
                                    RcsSettingsData.GIBA_AUTHENT);
                        } else if(authType.equalsIgnoreCase("aka")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE,
                                    RcsSettingsData.AKA_AUTHENT);
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IMS_AUTHENT_PROCEDURE_WIFI,
                                    RcsSettingsData.AKA_AUTHENT);
                        }
                        else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE,
                                    RcsSettingsData.DIGEST_AUTHENT);
                        }
                        continue;
                    }
                }

                if (realm == null) {
                    if ((realm = getValueByParamName("Realm", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_REALM, realm);
                        continue;
                    }
                }

                if (userName == null) {
                    if ((userName = getValueByParamName("UserName", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_PRIVATE_ID,
                                userName);
                        continue;
                    }
                }

                if (userPwd == null) {
                    if ((userPwd = getValueByParamName("UserPwd", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_PASSWORD, userPwd);
                        continue;
                    }
                }
            } while((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse RCSe settings
     *
     * @param node Node
     */
    private void parseRCSe(Node node) {
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("IMS")) {
                                parseIMS(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("PRESENCE")) {
                                parsePresence(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("XDMS")) {
                                parseXDMS(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("IM")) {
                                parseIM(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("CAPDISCOVERY")) {
                                parseCapabilityDiscovery(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("APN")) {
                                parseAPN(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("OTHER")) {
                                parseOther(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("SERVICES")) {
                                parseServices(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("SUPL")) {
                                parseSupl(childnode);
                            } else
                            if (RcsSettings.getInstance().supportOP01()) {
                                if (typenode.getNodeValue().equalsIgnoreCase("MessageStore")) {
                                    parseMessageStore(childnode);
                                }
                            }

                        }
                    }
                }
            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Parse IMS settings
     *
     * @param node IMS settings node
     */
    private void parseIMS(Node node) {
        String timert1 = null;
        String timert2 = null;
        String timert4 = null;
        String privateUserIdentity = null;
        String homeDomain = null;
        String keepAliveEnabled = null;
        String regRetryBasetime = null;
        String regRetryMaxtime = null;
        Node typenode = null;
        if (node == null) {
            return;
        }
        Node childnode = node.getFirstChild();

        if (childnode != null) {
            do {
                if (childnode.getNodeName().equals("characteristic")) {
                    if (childnode.getAttributes().getLength() > 0) {
                        typenode = childnode.getAttributes().getNamedItem("type");
                        if (typenode != null) {
                            if (logger.isActivated()) {
                                logger.debug("Node " + childnode.getNodeName() + " with type "
                                        + typenode.getNodeValue());
                            }
                            if (typenode.getNodeValue().equalsIgnoreCase("ConRefs")) {
                                parseConRefs(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("Public_user_identity_List")) {
                                parsePublicUserIdentity(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("Ext")) {
                                parseExt(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("ICSI_List")) {
                                parseICSI(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("LBO_P-CSCF_Address")) {
                                parsePcscfAddress(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("PhoneContext_List")) {
                                parsePhoneContextList(childnode);
                            } else
                            if (typenode.getNodeValue().equalsIgnoreCase("APPAUTH")) {
                                parseAppAuthent(childnode);
                            }
                        }
                    }
                }

                if (timert1 == null) {
                    if ((timert1 = getValueByParamName("Timer_T1", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.SIP_TIMER_T1,
                                timert1);
                        continue;
                    }
                }

                if (timert2 == null) {
                    if ((timert2 = getValueByParamName("Timer_T2", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.SIP_TIMER_T2,
                                timert2);
                        continue;
                    }
                }

                if (timert4 == null) {
                    if ((timert4 = getValueByParamName("Timer_T4", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(RcsSettingsData.SIP_TIMER_T4,
                                timert4);
                        continue;
                    }
                }

                if (privateUserIdentity == null) {
                    if ((privateUserIdentity = getValueByParamName("Private_User_Identity", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_PRIVATE_ID,
                                privateUserIdentity);
                        continue;
                    }
                }

                if (homeDomain == null) {
                    if ((homeDomain = getValueByParamName("Home_network_domain_name", childnode, TYPE_TXT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.USERPROFILE_IMS_HOME_DOMAIN,
                                homeDomain);
                        continue;
                    }
                }

                if (keepAliveEnabled == null) {
                    if ((keepAliveEnabled = getValueByParamName("Keep_Alive_Enabled", childnode, TYPE_INT)) != null) {
                        if (keepAliveEnabled.equals("1")) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.SIP_KEEP_ALIVE, RcsSettingsData.TRUE);
                        } else {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.SIP_KEEP_ALIVE, RcsSettingsData.FALSE);
                        }
                        continue;
                    }
                }

                if (regRetryBasetime == null) {
                    if ((regRetryBasetime = getValueByParamName("RegRetryBaseTime", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.REGISTER_RETRY_BASE_TIME, regRetryBasetime);
                        continue;
                    }
                }

                if (regRetryMaxtime == null) {
                    if ((regRetryMaxtime = getValueByParamName("RegRetryMaxTime", childnode, TYPE_INT)) != null) {
                        RcsSettings.getInstance().writeParameter(
                                RcsSettingsData.REGISTER_RETRY_MAX_TIME, regRetryMaxtime);
                        continue;
                    }
                }
                
                // Not supported under Android: "PDP_ContextOperPref"
                // Not used for RCS: "Voice_Domain_Preference_E_UTRAN"
                // Not used for RCS: "SMS_Over_IP_Networks_Indication"
                // Not used for RCS: "Voice_Domain_Preference_UTRAN"
                // Not used for RCS: "Mobility_Management_IMS_Voice_Termination"

            } while ((childnode = childnode.getNextSibling()) != null);
        }
    }

    /**
     * Get value of a parameter
     *
     * @param paramName Parameter name
     * @param node Node
     * @param type Parameter type
     * @return Value or null
     */
    private String getValueByParamName(String paramName, Node node, int type) {
        Node nameNode = null;
        Node valueNode = null;
        
        if ((node == null) ||
                !(node.getNodeName().equals("parm") ||
                        node.getNodeName().equals("param"))) {
            return null;
        }

        if ((node != null) && (node.getAttributes().getLength() > 0)) {
            nameNode = node.getAttributes().getNamedItem("name");
            if (nameNode == null) {
                return null;
            }
            valueNode = node.getAttributes().getNamedItem("value");
            if (valueNode == null) {
                return null;
            }
            if (nameNode.getNodeValue().equalsIgnoreCase(paramName)) {
                String value = valueNode.getNodeValue();
                if (logger.isActivated()) {
                    // logger.debug("Read parameter " + paramName + ": " + value);
                    logger.debug("Read parameter " + paramName);
                }
                
                // Check type
                if (type == TYPE_INT) {
                    try {
                        Integer.parseInt(value);
                    } catch(NumberFormatException e) {
                        if (logger.isActivated()) {
                            logger.warn("Bad value for integer parameter " + paramName);
                        }
                        return null;
                    }
                } 
                
                return value;
            } else {
                return null;
            }
        }
        return null;
    }
    
    /**
     * Extract the username part of the SIP-URI
     * 
     * @param uri SIP-URI
     * @return Username
     */
    private String extractUserNamePart(String uri) {
        if ((uri == null) || (uri.trim().length() == 0)) {
            return "";
        }

        try {
            uri = uri.trim();
            int index1 = uri.indexOf("sip:");
            if (index1 != -1) {
                int index2 = uri.indexOf("@", index1);
                String result = uri.substring(index1+4, index2);
                return result;
            } else {
                return uri;
            }
        } catch(Exception e) {
            return "";
        }
    }
    
    /**
     * Extract the username part of the SIP-URI
     * 
     * @param uri SIP-URI
     * @return Username
     */
    private String extractMSISDNPart(String uri) {
        if ((uri == null) || (uri.trim().length() == 0)) {
            return "";
        }

        try {
            uri = uri.trim();
            int index1 = uri.indexOf("tel:");
            if (index1 != -1) {
                String result = uri.substring(index1+4);
                return result;
            } else {
                return uri;
            }
        } catch(Exception e) {
            return "";
        }
    }
    
    /**
     * Is Tel URI
     * 
     * @param uri SIP-URI
     * @return Username
     */
    private boolean isTelURI(String uri) {
        if ((uri == null) || (uri.trim().length() == 0)) {
            return false;
        }

        try {
            
            if(uri.startsWith("tel"))
                return true;
            else 
                return false;
            
            
        } catch(Exception e) {
            return false;
        }
    }
    
    
    /**
     * Format to SIP-URI
     * 
     * @param uri URI
     * @return SIP-URI
     */
    private String formatSipUri(String uri) {
        if ((uri == null) || (uri.trim().length() == 0)) {
            return "";
        }

        try {
            uri = uri.trim();
            if (!uri.startsWith("sip:")) {
                uri = "sip:" + uri;
            }
            return uri;
        } catch(Exception e) {
            return "";
        }
    }
    
    private String getTelURI(ArrayList<String> address,String telURI) {
        String result = null;
        
        for(int i=0; i<address.size();i++) {
            if (address.get(i).contains(telURI)) {
               result= address.get(i);
            }        
        }
        return result;
    }
    
    // @tct-stack huangbo add begin
        private void parseMessaging(Node node) {
            String deliveryTimeout = null;
            String ftHTTPCapAlwaysOn = null;
    
            if (node == null) {
                return;
            }
            Node childnode = node.getFirstChild();
    
            if (childnode != null) {
                do {
    
                    if (deliveryTimeout == null) {
                        if ((deliveryTimeout = getValueByParamName("deliveryTimeout", childnode, TYPE_INT)) != null) {
                            RcsSettings.getInstance().writeParameter(
                                    RcsSettingsData.DELIVERY_TIMEOUT, deliveryTimeout);
                            continue;
                       }
                    }
    
                    if (ftHTTPCapAlwaysOn == null) {
                        if ((ftHTTPCapAlwaysOn = getValueByParamName("ftHTTPCapAlwaysOn", childnode, TYPE_INT)) != null) {
                            if (ftHTTPCapAlwaysOn.equals("1")) {
                                RcsSettings.getInstance().writeParameter(
                                        RcsSettingsData.FT_HTTP_CAP_ALWAYS_ON, RcsSettingsData.TRUE);
                            } else {
                                RcsSettings.getInstance().writeParameter(
                                        RcsSettingsData.FT_HTTP_CAP_ALWAYS_ON, RcsSettingsData.FALSE);
                            }
                            continue;
                        }
                    }
    
                } while ((childnode = childnode.getNextSibling()) != null);
            }
        }
        // @tct-stack huangbo add end
}
