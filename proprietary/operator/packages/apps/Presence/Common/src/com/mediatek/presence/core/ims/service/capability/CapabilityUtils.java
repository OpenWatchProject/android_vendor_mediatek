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

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.telephony.TelephonyManager;

import com.mediatek.presence.core.ims.network.sip.FeatureTags;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.core.ims.protocol.sdp.MediaAttribute;
import com.mediatek.presence.core.ims.protocol.sdp.MediaDescription;
import com.mediatek.presence.core.ims.protocol.sdp.SdpParser;
import com.mediatek.presence.core.ims.protocol.sdp.SdpUtils;
import com.mediatek.presence.core.ims.protocol.sip.SipMessage;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.utils.NetworkUtils;
import com.mediatek.presence.utils.StringUtils;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Capability utility functions
 *
 * @author jexa7410
 */
public class CapabilityUtils {

    /**
     * The logger
     */
    private static Logger logger = Logger.getLogger("CapabilityUtils");

    private static String mExternalSupportedFeatures = "";


    /**
     * Get supported feature tags for capability exchange
     *
     * @param richcall Rich call supported
     * @param ipcall IP call supported
     * @return List of tags
     */
    public static List<String> getSupportedFeatureTags(int slotId, boolean richcall, boolean ipcall) {
        List<String> tags = new ArrayList<String>();

        // Video share support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isVideoSharingSupported()
                && richcall
                && NetworkUtils.getNetworkAccessType(slotId) >= NetworkUtils.NETWORK_ACCESS_3G) {
            tags.add(FeatureTags.FEATURE_3GPP_VIDEO_SHARE);
        }

        String supported = "";

        // Chat support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isImSessionSupported()) {
            supported += FeatureTags.FEATURE_RCSE_CHAT + ",";

            /*M : TCT GSM IOT patch */
            //@tct-stack yuxin.li@tcl.com IOT add +g.oma.sip-im tag in contact header
            supported += FeatureTags.FEATURE_RCSE_CHAT_OMA + ",";
            /*@*/
        }

        // FT support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isFileTransferSupported()
                && isFileStorageAvailable(slotId)) {
            if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCPMSupported())
                supported += FeatureTags.FEATURE_CPM_FT + ",";
            else
                supported += FeatureTags.FEATURE_RCSE_FT + ",";
        }

        // FT over HTTP support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isFileTransferHttpSupported()
                && isFileStorageAvailable(slotId)) {
            supported += FeatureTags.FEATURE_RCSE_FT_HTTP + ",";
        }

        // Image share support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isImageSharingSupported()
                && (richcall || ipcall) && isFileStorageAvailable(slotId)) {
            supported += FeatureTags.FEATURE_RCSE_IMAGE_SHARE + ",";
        }

        // Presence discovery support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isPresenceDiscoverySupported()) {
            supported += FeatureTags.FEATURE_RCSE_PRESENCE_DISCOVERY + ",";
        }

        // Social presence support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isSocialPresenceSupported()) {
            supported += FeatureTags.FEATURE_RCSE_SOCIAL_PRESENCE + ",";
        }

/*M : TCT GSM IOT patch 0001
        // Geolocation push support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isGeoLocationPushSupported()) {
            supported += FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH + ",";
        }
@*/

        // FT thumbnail support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isFileTransferThumbnailSupported()) {
            supported += FeatureTags.FEATURE_RCSE_FT_THUMBNAIL + ",";
        }

        // FT S&F support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isFileTransferStoreForwardSupported()) {
            supported += FeatureTags.FEATURE_RCSE_FT_SF + ",";
        }

        // Group chat S&F support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isGroupChatStoreForwardSupported()) {
            supported += FeatureTags.FEATURE_RCSE_GC_SF + ",";
        }

        /*M: MTK integrated pathc */
        // Messaging Integerated Mode support
        if (!(RcsSettingsManager.getRcsSettingsInstance(slotId).getMessagingUx() ==  0)) {
                                  tags.add(FeatureTags.FEATURE_RCSE_INTEGERATED_IM);
                    //supported += FeatureTags.FEATURE_RCSE_INTEGERATED_IM + ",";
        }
        /*@*/

/*M : TCT GSM IOT patch 0001
        // IP call support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isIPVoiceCallSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_IP_VOICE_CALL);
            tags.add(FeatureTags.FEATURE_3GPP_IP_VOICE_CALL);
        }
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isIPVideoCallSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL);
        }
 */

        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isSipAutomata()) {
            tags.add(FeatureTags.FEATURE_SIP_AUTOMATA);
        }

        // RCS extensions support
        String exts = RcsSettingsManager.getRcsSettingsInstance(slotId).getSupportedRcsExtensions();
        if ((exts != null) && (exts.length() > 0)) {
            String[] values = exts.split(",");
            for(int i=0; i < values.length; i++) {
                supported += FeatureTags.FEATURE_RCSE_EXTENSION + "." + values[i] + ",";
            }
        }
/*M : TCT GSM IOT patch 0001 */
        boolean isDrop2Active = true;
        if(isDrop2Active){
            // IP call support
            if (RcsSettingsManager.getRcsSettingsInstance(slotId).isIPVoiceCallSupported()) {
                tags.add(FeatureTags.FEATURE_RCSE_IP_VOICE_CALL);
                tags.add(FeatureTags.FEATURE_3GPP_IP_VOICE_CALL);
            }
            if (RcsSettingsManager.getRcsSettingsInstance(slotId).isIPVideoCallSupported()) {
                tags.add(FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL);
            }
         // Geolocation push support
            if (RcsSettingsManager.getRcsSettingsInstance(slotId).isGeoLocationPushSupported()) {
                supported += FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH + ",";
            }
        }
/*@*/

        // Add RCS-e prefix
        if (supported.length() != 0) {
            if (supported.endsWith(",")) {
                supported = supported.substring(0, supported.length()-1);
            }
            supported = FeatureTags.FEATURE_RCSE + "=\"" + supported + "\"";
            tags.add(supported);
        }

        //burn after reading tag
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCPMBurnAfterReadingSupported()) {
            tags.add(FeatureTags.FEATURE_CPM_BURNED_MSG);
        }

        return tags;
    }

    /**
     * Extract features tags
     *
     * @param msg Message
     * @return Capabilities
     */
    public static Capabilities extractCapabilities(SipMessage msg) {

         if (logger.isActivated()) {
                //logger.info("200 OK response received for " + contact);
                logger.info("extractCapabilities");
            }
        // Analyze feature tags
        Capabilities capabilities = new Capabilities();
        ArrayList<String> tags = msg.getFeatureTags();
        boolean iPCall_RCSE = false;
        boolean iPCall_3GPP = false;

        for(int i=0; i < tags.size(); i++) {
            String tag = tags.get(i);
            if (tag.contains(FeatureTags.FEATURE_3GPP_VIDEO_SHARE)) {
                /*
                 * M :+ @tct-stack-[IOT][ID_RCS_6_10_2]modified by fang.wu@tcl.com) { ||
                 * tag.contains(FeatureTags.FEATURE_3GPP_ICSI_MMTEL_VIDEO)) { //
                 * @tct-stack-[IOT][ID_RCS_6_10_2]modified by fang.wu@tcl.com)
                    */
                // Support video share service
                capabilities.setVideoSharingSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_IMAGE_SHARE)) {
                // Support image share service
                capabilities.setImageSharingSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_CHAT)) {
                // Support IM service
                capabilities.setRCSContact(true);
                capabilities.setImSessionSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_FT) ||
                tag.contains(FeatureTags.FEATURE_CPM_FT)) {
                // Support FT service
                capabilities.setFileTransferSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_FT_HTTP)) {
                // Support FT over HTTP service
                capabilities.setFileTransferHttpSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_OMA_IM)) {
                // Support both IM & FT services
                capabilities.setImSessionSupport(true);
                capabilities.setFileTransferSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_PRESENCE_DISCOVERY)) {
                // Support capability discovery via presence service
                capabilities.setPresenceDiscoverySupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_SOCIAL_PRESENCE)) {
                // Support social presence service
                capabilities.setSocialPresenceSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH)) {
                // Support geolocation push service
                capabilities.setGeolocationPushSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_FT_THUMBNAIL)) {
                // Support file transfer thumbnail service
                capabilities.setFileTransferThumbnailSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_IP_VOICE_CALL)) {
                // Support IP Call
                if (iPCall_3GPP) {
                    capabilities.setIPVoiceCallSupport(true);
                } else {
                    iPCall_RCSE = true;
                }
            } else if (tag.contains(FeatureTags.FEATURE_3GPP_IP_VOICE_CALL)) {
                // Support IP Call
                if (iPCall_RCSE) {
                    capabilities.setIPVoiceCallSupport(true);
                } else {
                    iPCall_3GPP = true;
                }
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL)) {
                capabilities.setIPVideoCallSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_FT_SF)) {
                // Support FT S&F service
                capabilities.setFileTransferStoreForwardSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_GC_SF)) {
                // Support FT S&F service
                capabilities.setGroupChatStoreForwardSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_INTEGERATED_IM)) {
                    // Support Integerated Messaging Mode
                    capabilities.setIntegeratedMessagingMode(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_OFFLINE)) {

                 if (logger.isActivated()) {
                        //logger.info("200 OK response received for " + contact);
                        logger.info("tag.contains(FeatureTags.FEATURE_RCSE_OFFLINE)");
                    }

                capabilities.setSipJoynOffline(true);
            } else if (tag.startsWith(FeatureTags.FEATURE_RCSE + "=\"" + FeatureTags.FEATURE_RCSE_EXTENSION)) {
                // Support a RCS extension
                String[] values = tag.split("=");
                String value =  StringUtils.removeQuotes(values[1]);
                String serviceId = value.substring(FeatureTags.FEATURE_RCSE_EXTENSION.length()+1, value.length());
                capabilities.addSupportedExtension(serviceId);
            } else if (tag.contains(FeatureTags.FEATURE_SIP_AUTOMATA)) {
                capabilities.setSipAutomata(true);
            } else if (tag.contains(FeatureTags.FEATURE_3GPP_IP_VOICE_CALL)) {
                capabilities.setIR94_VoiceCall(true);
            } else if (tag.contains(FeatureTags.FEATURE_3GPP_ICSI_MMTEL_VIDEO)) {
                capabilities.setIR94_VideoCall(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_PAGER_MSG)
                    || tag.contains(FeatureTags.FEATURE_RCSE_LARGE_MSG)) {
                capabilities.setStandaloneMsgSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_3GPP_VIDEO_SHARE)) {
                capabilities.setVideoSharingDuringCallSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_VIDEO_SAHRING_OUTSIDE_OF_VOICE_CALL)) {
                capabilities.setVideoSharingOutsideOfVoiceCallSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_GEO_LOCATION_PULL_USING_FILE_TRANSFER)) {
                capabilities.setGeoLocationPullUsingFileTransferSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_GEO_LOCATION_PULL)) {
                capabilities.setGeoLocationPullSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_IP_VOICE_CALL)) {
                capabilities.setRcsIpVoiceCallSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL)) {
                capabilities.setRcsIpVideoCallSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_RCS_VIDEO_ONLY_CALL)) {
                capabilities.setRcsIpVideoOnlyCallSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_CHATBOT)) {
                capabilities.setChatbotSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_CHATBOT_SA)) {
                capabilities.setChatbotSaSupport(true);
            } else if (tag.contains(FeatureTags.FEATURE_CALLCOMPOSER)) {
                capabilities.setCallcomposerSupport(true);
            }

            /*
             * BURN AFTER READ
             */
            else if (tag.contains(FeatureTags.FEATURE_CPM_BURNED_MSG.toLowerCase())) {

                          if (logger.isActivated()) {
                                  logger.info("capabilities.setBurnAfterRead(true)");
                             }
                capabilities.setBurnAfterRead(true);
            }
        }

        // Analyze SDP part
        byte[] content = msg.getContentBytes();
        if (content != null) {
            SdpParser parser = new SdpParser(content);

            // Get supported video codecs
            Vector<MediaDescription> mediaVideo = parser.getMediaDescriptions("video");
            Vector<String> videoCodecs = new Vector<String>();
            for (int i=0; i < mediaVideo.size(); i++) {
                MediaDescription desc = mediaVideo.get(i);
                MediaAttribute attr = desc.getMediaAttribute("rtpmap");
                if (attr !=  null) {
                    String rtpmap = attr.getValue();
                    String encoding = rtpmap.substring(rtpmap.indexOf(desc.payload)+desc.payload.length()+1);
                    String codec = encoding.toLowerCase().trim();
                    int index = encoding.indexOf("/");
                    if (index != -1) {
                        codec = encoding.substring(0, index);
                    }
                }
            }
            if (videoCodecs.size() == 0) {
                // No video codec supported between me and the remote contact
                capabilities.setVideoSharingSupport(false);
            }

            // Check supported image formats
            Vector<MediaDescription> mediaImage = parser.getMediaDescriptions("message");
            Vector<String> imgFormats = new Vector<String>();
            for (int i=0; i < mediaImage.size(); i++) {
                MediaDescription desc = mediaImage.get(i);
                MediaAttribute attr = desc.getMediaAttribute("accept-types");
                if (attr != null) {
                    String[] types = attr.getValue().split(" ");
                    for(int j = 0; j < types.length; j++) {
                        String fmt = types[j];
                        //if ((fmt != null) && MimeManager.isMimeTypeSupported(fmt)) { // Changed by Deutsche Telekom AG
                        //    imgFormats.addElement(fmt);
                        //}
                    }
                }
            }
            if (imgFormats.size() == 0) {
                // No image format supported between me and the remote contact
                capabilities.setImageSharingSupport(false);
            }
        }

        return capabilities;
    }

    /**
     * Update external supported features
     *
     * @param context Context
     */
    public static void updateExternalSupportedFeatures(Context context) {
        try {
            // Intent query on current installed activities
            PackageManager packageManager = context.getPackageManager();
            Intent intent = new Intent(
                    com.gsma.services.rcs.capability.CapabilityService.INTENT_EXTENSIONS);
            String mime = com.gsma.services.rcs.capability.CapabilityService.EXTENSION_MIME_TYPE
                    + "/*";
            intent.setType(mime);
            List<ResolveInfo> list = packageManager.queryIntentActivities(intent, PackageManager.GET_RESOLVED_FILTER);
            StringBuffer extensions = new StringBuffer();
            for(int i=0; i < list.size(); i++) {
                ResolveInfo info = list.get(i);
                for(int j =0; j < info.filter.countDataTypes(); j++) {
                    String tag = info.filter.getDataType(j);
                    String[] value = tag.split("/");
                    extensions.append("," + value[1]);
                }
            }
            if ((extensions.length() > 0) && (extensions.charAt(0) == ',')) {
                extensions.deleteCharAt(0);
            }

            // Save extensions in database
            TelephonyManager tm = (TelephonyManager) context.getSystemService(
                Context.TELEPHONY_SERVICE);
            if (tm == null) {
                logger.error("updateExternalSupportedFeatures fail. TelephonyManager is null");
                return;
            }
            int simCount = tm.getSimCount();
            for (int slotId = 0; slotId < simCount; ++slotId) {
                RcsSettingsManager.getRcsSettingsInstance(slotId).setSupportedRcsExtensions(extensions.toString());
            }
            mExternalSupportedFeatures = extensions.toString();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Build supported SDP part
     *
     * @param slotId slot index
     * @param ipAddress Local IP address
     * @param richcall Rich call supported
     * @return SDP
     */
    public static String buildSdp(int slotId, String ipAddress, boolean richcall) {
        String sdp = null;
        if (richcall) {
            boolean video = RcsSettingsManager.getRcsSettingsInstance(slotId).isVideoSharingSupported()
                    && NetworkUtils.getNetworkAccessType(slotId) >= NetworkUtils.NETWORK_ACCESS_3G;
            boolean image = RcsSettingsManager.getRcsSettingsInstance(slotId).isImageSharingSupported();
            boolean geoloc = RcsSettingsManager.getRcsSettingsInstance(slotId).isGeoLocationPushSupported();
            if (video | image) {
                // Build the local SDP
                String ntpTime = SipUtils.constructNTPtime(System.currentTimeMillis());
                sdp = "v=0" + SipUtils.CRLF +
                        "o=- " + ntpTime + " " + ntpTime + " " + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                        "s=-" + SipUtils.CRLF +
                        "c=" + SdpUtils.formatAddressType(ipAddress) + SipUtils.CRLF +
                        "t=0 0" + SipUtils.CRLF;

                // Add image and geoloc config
                if (image || geoloc) {
                    StringBuffer supportedTransferFormats = new StringBuffer();

                    // Update SDP
                    String imageSharingConfig = "m=message 0 TCP/MSRP *"  + SipUtils.CRLF +
                        "a=accept-types:" + supportedTransferFormats.toString().trim() + SipUtils.CRLF +
                        "a=file-selector" + SipUtils.CRLF;
                    int maxSize = 0; //ImageTransferSession.getMaxImageSharingSize();
                    if (maxSize > 0) {
                        imageSharingConfig += "a=max-size:" + maxSize + SipUtils.CRLF;
                    }
                    sdp += imageSharingConfig;
                }
            }
        }
        return sdp;
    }

    /**
     * Is the current storage conditions allow to receive files
     *
     * @param slotId slot index
     * @return <code>true</code> if supported, otherwise <code>false</code>
     */
    private static boolean isFileStorageAvailable(int slotId) {
        long minStockage = 1024 * (long)RcsSettingsManager.getRcsSettingsInstance(slotId)
                .getMinStorageCapacity();
        if (minStockage > 0) {
            long freeSpace = 0; //StorageUtils.getExternalStorageFreeSpace();
            if (freeSpace < minStockage) {
                return false;
            }
        }
        return true;
    }

    /**
     * Extract service ID from fetaure tag extension
     *
     * @param featureTag Feature tag
     * @return Service ID
     */
    public static String extractServiceId(String featureTag) {
        String serviceId;
        try {
            String[] values = featureTag.split("=");
            String value =  StringUtils.removeQuotes(values[1]);
            serviceId = value.substring(FeatureTags.FEATURE_RCSE_EXTENSION.length()+1, value.length());
        } catch(Exception e) {
            serviceId = null;
        }
        return serviceId;
    }

    /**
     * Get supported feature tags for capability exchange (AOSP procedure)
     */
    public static List<String> getSupportedFeatureTags(Capabilities caps) {
        List<String> tags = new ArrayList<String>();
        if (caps.isImSessionSupported()) {
            tags.add(FeatureTags.FEATURE_OMA_IM);
        }
        if (caps.isFileTransferSupported()) {
            tags.add(FeatureTags.FEATURE_OMA_IM);
            tags.add(FeatureTags.FEATURE_CPM_FT);
        }
        if (caps.isFileTransferThumbnailSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_FT_THUMBNAIL);
        }
        if (caps.isFileTransferStoreForwardSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_FT_SF);
        }
        if (caps.isFileTransferHttpSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_FT_HTTP);
        }
        if (caps.isImageSharingSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_IMAGE_SHARE);
        }
        if (caps.isSocialPresenceSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_SOCIAL_PRESENCE);
        }
        if (caps.isPresenceDiscoverySupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_PRESENCE_DISCOVERY);
        }
        if (caps.isIR94_VoiceCallSupported()) {
            tags.add(FeatureTags.FEATURE_3GPP_IP_VOICE_CALL);
        }
        if (caps.isIR94_VideoCallSupported()) {
            tags.add(FeatureTags.FEATURE_3GPP_ICSI_MMTEL_VIDEO);
        }
        if (caps.isGeolocationPushSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH);
        }
        if (caps.isStandaloneMsgSupport()) {
            tags.add(FeatureTags.FEATURE_RCSE_PAGER_MSG);
            tags.add(FeatureTags.FEATURE_RCSE_LARGE_MSG);
        }
        if (caps.isGroupChatStoreForwardSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_GC_SF);
        }
        if (caps.isVideoSharingDuringCallSupported()) {
            tags.add(FeatureTags.FEATURE_3GPP_VIDEO_SHARE);
        }
        if (caps.isVideoSharingOutsideOfVoiceCallSupported()) {
            tags.add(FeatureTags.FEATURE_VIDEO_SAHRING_OUTSIDE_OF_VOICE_CALL);
        }
        if (caps.isGeoLocationPullUsingFileTransferSupported()) {
            tags.add(FeatureTags.FEATURE_GEO_LOCATION_PULL_USING_FILE_TRANSFER);
        }
        if (caps.isGeoLocationPullSupported()) {
            tags.add(FeatureTags.FEATURE_GEO_LOCATION_PULL);
        }
        if (caps.isRcsIpVoiceCallSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_IP_VOICE_CALL);
        }
        if (caps.isRcsIpVideoCallSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL);
        }
        if (caps.isRcsIpVideoOnlyCallSupported()) {
            tags.add(FeatureTags.FEATURE_RCS_VIDEO_ONLY_CALL);
        }
        ArrayList<String> extList = caps.getSupportedExtensions();
        String supported = "";
        String prefix = "";
        for (String ext : extList) {
            supported += prefix;
            prefix = ",";
            supported += FeatureTags.FEATURE_RCSE_EXTENSION + "." + ext;
        }
        tags.add(supported);
        return tags;
    }

    public static String getExternalSupportedFeatures() {
        return mExternalSupportedFeatures;
    }
}
