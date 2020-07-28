/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 ******************************************************************************/

package com.mediatek.presence.core.ims.service.presence;

import com.gsma.services.rcs.JoynServiceConfiguration;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import com.mediatek.presence.core.CoreException;
import com.mediatek.presence.core.ims.ImsModule;
import com.mediatek.presence.core.ims.network.sip.SipUtils;

import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.core.ims.service.presence.extension.ViLTEExtension;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.provider.eab.ContactsManager;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.DateUtils;
import com.mediatek.presence.utils.StringUtils;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Presence service
 *
 * @author Jean-Marc AUFFRET
 */
public class PresenceServiceEx extends PresenceService {
    /**
     * The logger
     */
    private Logger logger = null;
    private int mSlotId = 0;

    public PresenceServiceEx(ImsModule parent) throws CoreException {
        super(parent);
        mSlotId = parent.getSlotId();
        logger = Logger.getLogger(mSlotId, "PresenceServiceEx");
    }

    /**
     * Start the IMS service
     */
    @Override
    public synchronized void start() {
        if (logger.isActivated()) {
            logger.debug("PresenceServiceEx Start");
        }
        if (isServiceStarted()) {
            // Already started
            return;
        }
        setServiceStarted(true);

        // attach with extension services
        attachExtensions();

        // Restore the last presence info from the contacts database
        presenceInfo = ContactsManager.getInstance().getMyPresenceInfo(mSlotId);
        if (logger.isActivated()) {
            logger.debug("Last presence info:\n" + presenceInfo.toString());
        }
        publisher.init();
        // publish capability
        publishCapability();
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
        if (logger.isActivated()) {
            logger.debug("PresenceServiceEx Stop");
        }
        setServiceStarted(false);

        // detach all the presence extension services
        detachExtension();

        // Stop publish
        publisher.terminate();
    }

    private String buildRCSCapablities(String timestamp, Capabilities capabilities) {
        if (logger.isActivated()) {
            logger.debug("buildRCSCapablities");
        }
        String publishXML = "";
        // Always inslude StandaloneMsg, ImSession(chat), FileTransfer tuples if RCS enabled
        if (capabilities.isStandaloneMsgSupport()) {
            // Standalone Mode Messaging
            publishXML +=
                "<tuple id=\"StandAlone\">" + SipUtils.CRLF +
                "   <status><basic>open</basic></status>" + SipUtils.CRLF +
                "   <op:service-description>" + SipUtils.CRLF +
                "       <op:service-id>" + PresenceUtils.FEATURE_RCS2_STANDALONEMSG + "</op:service-id>" + SipUtils.CRLF +
                "       <op:version>2.0</op:version>" + SipUtils.CRLF +
                "       <op:description>Standalone Mode Messaging</op:description>" + SipUtils.CRLF +
                "   </op:service-description>" + SipUtils.CRLF +
                "   <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "   <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }

        if (capabilities.isImSessionSupported()) {
            // Session Mode Messaging
            publishXML +=
                "<tuple id=\"SessModeMessa\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_CHAT_2 + "</op:service-id>" + SipUtils.CRLF +
                "    <op:version>2.0</op:version>" + SipUtils.CRLF +
                "    <op:description>Session Mode Messaging</op:description>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }

        if (capabilities.isFileTransferSupported()) {
            // File Transfer
            publishXML +=
                "<tuple id=\"FileTransfer\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_FT + "</op:service-id>" + SipUtils.CRLF +
                "    <op:version>1.0</op:version>" + SipUtils.CRLF +
                "    <op:description>File Transfer</op:description>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }

        if (capabilities.isFileTransferThumbnailSupported()) {
            // File Transfer
            publishXML +=
                "<tuple id=\"FileTransferThumbnail\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>org.openmobilealliance:File-Transfer-thumb</op:service-id>" + SipUtils.CRLF +
                "    <op:version>2.0</op:version>" + SipUtils.CRLF +
                "    <op:description>File Transfer Thumbnail</op:description>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }

        if (capabilities.isFileTransferHttpSupported()) {
            // FileTransferHTTP
            publishXML +=
                "<tuple id=\"FileTransferH\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_FT_HTTP + "</op:service-id>" + SipUtils.CRLF +
                "    <op:version>1.0</op:version>" + SipUtils.CRLF +
                "    <op:description>FileTransferHTTP</op:description>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }

        // IMAGE SHARE
        if (capabilities.isImageSharingSupported()) {
            publishXML +=
                "<tuple id=\"t2\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_IMAGE_SHARE + "</op:service-id>" + SipUtils.CRLF +
                "    <op:version>1.0</op:version>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;

        }

        // VIDEO SHARE
        if (capabilities.isVideoSharingSupported()) {
            publishXML +=
                "<tuple id=\"t3\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_VIDEO_SHARE + "</op:service-id>" + SipUtils.CRLF +
                "    <op:version>1.0</op:version>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }
        return publishXML;
    }

    private String buildVolteCapabilities(String timestamp, Capabilities capabilities) {
        String publishXML = "";
        if (capabilities.isIR94_VoiceCallSupported()) {
            // IR94 VIDEO AND VOICE CALL
            publishXML +=
                "<tuple id=\"VOLTECALL\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <caps:servcaps>" + SipUtils.CRLF +
                "    <caps:audio>" +
                    ((ViLTEExtension) vilteService).isVoiceCallSupported() +
                "</caps:audio>" + SipUtils.CRLF;
            if (capabilities.isIR94_VideoCallSupported() &&
                ((ViLTEExtension) vilteService).isVideoCallSupported()) {
                publishXML += "<caps:video>true</caps:video>" + SipUtils.CRLF;
            }

            if (capabilities.isIR94_DuplexModeSupported() &&
                ((ViLTEExtension) vilteService).isDuplexSupported()) {
                publishXML +=
                    " <caps:duplex>" + SipUtils.CRLF +
                    " <caps:supported>" + SipUtils.CRLF +
                    "   <caps:full/>" + SipUtils.CRLF +
                    " </caps:supported>" + SipUtils.CRLF +
                    " </caps:duplex>" + SipUtils.CRLF;
            }

            publishXML +=
                "  </caps:servcaps>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL + "</op:service-id>" + SipUtils.CRLF +
                "    <op:version>1.0</op:version>" + SipUtils.CRLF +
                "    <op:description>IPVideoCall</op:description>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }
        return publishXML;
    }

    @Override
    protected String buildCapabilities(String timestamp, Capabilities capabilities) {
        String publishXML = "";
        if (capabilities.isPresenceDiscoverySupported()) {
            // PRESENCE DEISCOVERY
            publishXML +=
                "<tuple id=\"DiscoveryPres\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "  <op:service-description>" + SipUtils.CRLF +
                "    <op:service-id>" + PresenceUtils.FEATURE_RCS2_DISCOVERY_VIA_PRESENCE + "</op:service-id>" + SipUtils.CRLF +
                "    <op:version>1.0</op:version>" + SipUtils.CRLF +
                "  </op:service-description>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }

        publishXML += buildRCSCapablities(timestamp, capabilities);
        publishXML += buildVolteCapabilities(timestamp, capabilities);
        return publishXML;
    }

    @Override
    protected String buildGeoloc(String timestamp, Geoloc geolocInfo) {
        String document = "";

        if (geolocInfo != null) {
            document +=
                "<tuple id=\"g1\">" + SipUtils.CRLF +
                "  <status><basic>open</basic></status>" + SipUtils.CRLF +
                "   <gp:geopriv>" + SipUtils.CRLF +
                "    <gp:location-info><gml:location>" + SipUtils.CRLF +
                "      <gml:Point srsDimension=\"3\">" + SipUtils.CRLF +
                "        <gml:pos>" +
                                geolocInfo.getLatitude() + " " +
                                geolocInfo.getLongitude() + " " +
                                geolocInfo.getAltitude() +
                "        </gml:pos>" + SipUtils.CRLF +
                "      </gml:Point></gml:location>" + SipUtils.CRLF +
                "    </gp:location-info>" + SipUtils.CRLF +
                "    <gp:method>GPS</gp:method>" + SipUtils.CRLF +
                "   </gp:geopriv>" + SipUtils.CRLF +
                "  <contact>" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "</contact>" + SipUtils.CRLF +
                "  <timestamp>" + timestamp + "</timestamp>" + SipUtils.CRLF +
                "</tuple>" + SipUtils.CRLF;
        }

        return document;
    }


    @Override
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
                            "\" opd:resolution=\"" + photoIcon.getResolution() + "\">" +
                            "</rpid:status-icon>" + SipUtils.CRLF;
        }

        String freetext = info.getFreetext();
        if (freetext != null) {
            document += "  <pdm:note>" + StringUtils.encodeUTF8(StringUtils.encodeXML(freetext)) + "</pdm:note>" + SipUtils.CRLF;
        }

        return document;
    }


    @Override
    protected String buildPresenceInfoDocument(PresenceInfo info, Capabilities capabilities) {
        String document = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + SipUtils.CRLF +
                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"" +
                " xmlns:op=\"urn:oma:xml:prs:pidf:oma-pres\"" +
                " xmlns:opd=\"urn:oma:xml:pde:pidf:ext\"" +
                " xmlns:pdm=\"urn:ietf:params:xml:ns:pidf:data-model\"" +
                " xmlns:cipid=\"urn:ietf:params:xml:ns:pidf:cipid\""
                + " xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\""
                + " xmlns:caps=\"urn:ietf:params:xml:ns:pidf:caps\""
                + " entity=\"" + ImsModule.IMS_USER_PROFILE.getPublicUri() + "\">" + SipUtils.CRLF;

        // Encode timestamp
        String timestamp = DateUtils.encodeDate(info.getTimestamp());

        // Build capabilities
        logger.debug("capabilities: " + capabilities);
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
}
