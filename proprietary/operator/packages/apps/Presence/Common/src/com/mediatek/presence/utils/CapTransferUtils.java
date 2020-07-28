package com.mediatek.presence.utils;

import com.android.ims.internal.uce.common.CapInfo;
import com.mediatek.presence.core.ims.service.capability.Capabilities;

public class CapTransferUtils {
    public static Capabilities transferCapInfoToCapabilities(CapInfo capInfo) {
        Capabilities cap = new Capabilities();
        cap.setImSessionSupport(capInfo.isImSupported());
        cap.setFileTransferSupport(capInfo.isFtSupported());
        cap.setFileTransferThumbnailSupport(capInfo.isFtThumbSupported());
        cap.setFileTransferStoreForwardSupport(capInfo.isFtSnFSupported());
        cap.setFileTransferHttpSupport(capInfo.isFtHttpSupported());
        cap.setImageSharingSupport(capInfo.isIsSupported());
        cap.setSocialPresenceSupport(capInfo.isSpSupported());
        cap.setPresenceDiscoverySupport(capInfo.isCdViaPresenceSupported());
        cap.setIR94_VoiceCall(capInfo.isIpVoiceSupported());
        cap.setIR94_VideoCall(capInfo.isIpVideoSupported());
        cap.setGeolocationPushSupport(capInfo.isGeoPushSupported());
        cap.setStandaloneMsgSupport(capInfo.isSmSupported());
        cap.setGroupChatStoreForwardSupport(capInfo.isFullSnFGroupChatSupported());
        cap.setVideoSharingDuringCallSupport(capInfo.isVsDuringCSSupported());
        cap.setVideoSharingOutsideOfVoiceCallSupport(capInfo.isVsSupported());
        cap.setGeoLocationPullUsingFileTransferSupport(capInfo.isGeoPullFtSupported());
        cap.setGeoLocationPullSupport(capInfo.isGeoPullSupported());
        cap.setRcsIpVoiceCallSupport(capInfo.isRcsIpVoiceCallSupported());
        cap.setRcsIpVideoCallSupport(capInfo.isRcsIpVideoCallSupported());
        cap.setRcsIpVideoOnlyCallSupport(capInfo.isRcsIpVideoOnlyCallSupported());
        cap.setTimestamp(capInfo.getCapTimestamp());
        String[] extensions = capInfo.getExts();
        for (int i = 0; i < extensions.length; ++i) {
            cap.addSupportedExtension(extensions[i]);
        }
        return cap;
        //TODO: Still some capa not add yet
    }

    public static CapInfo transferCapabilitiesToCapInfo(Capabilities caps) {
        CapInfo capInfo = new CapInfo();
        capInfo.setImSupported(caps.isImSessionSupported());
        capInfo.setFtSupported(caps.isFileTransferSupported());
        capInfo.setFtThumbSupported(caps.isFileTransferThumbnailSupported());
        capInfo.setFtSnFSupported(caps.isFileTransferStoreForwardSupported());
        capInfo.setFtHttpSupported(caps.isFileTransferHttpSupported());
        capInfo.setIsSupported(caps.isImageSharingSupported());
        capInfo.setSpSupported(caps.isSocialPresenceSupported());
        capInfo.setCdViaPresenceSupported(caps.isPresenceDiscoverySupported());
        capInfo.setIpVoiceSupported(caps.isIR94_VoiceCallSupported());
        capInfo.setIpVideoSupported(caps.isIR94_VideoCallSupported());
        capInfo.setGeoPushSupported(caps.isGeolocationPushSupported());
        capInfo.setSmSupported(caps.isStandaloneMsgSupport());
        capInfo.setFullSnFGroupChatSupported(caps.isGroupChatStoreForwardSupported());
        capInfo.setVsDuringCSSupported(caps.isVideoSharingDuringCallSupported());
        capInfo.setVsSupported(caps.isVideoSharingOutsideOfVoiceCallSupported());
        capInfo.setGeoPullFtSupported(caps.isGeoLocationPullUsingFileTransferSupported());
        capInfo.setGeoPullSupported(caps.isGeoLocationPullSupported());
        capInfo.setRcsIpVoiceCallSupported(caps.isRcsIpVoiceCallSupported());
        capInfo.setRcsIpVideoCallSupported(caps.isRcsIpVideoCallSupported());
        capInfo.setRcsIpVideoOnlyCallSupported(caps.isRcsIpVideoOnlyCallSupported());

        return capInfo;
    }

    public static String transferCapInfoToString(CapInfo capInfo) {
        String info = "isImsSupported = " + capInfo.isImSupported()
                + ", isFtThumbSupported = " + capInfo.isFtThumbSupported()
                + ", isFtSnFSupported = " + capInfo.isFtSnFSupported()
                + ", isFtHttpSupported = " + capInfo.isFtHttpSupported()
                + ", isFtSupported = " + capInfo.isFtSupported()
                + ", isIsSupported = " + capInfo.isIsSupported()
                + ", isVsDuringCSSupported = " + capInfo.isVsDuringCSSupported()
                + ", isVsSupported = " + capInfo.isVsSupported()
                + ", isSpSupported = " + capInfo.isSpSupported()
                + ", isCdViaPresenceSupported = " + capInfo.isCdViaPresenceSupported()
                + ", isIpVideoSupported = " + capInfo.isIpVideoSupported()
                + ", isGeoPullFtSupported = " + capInfo.isGeoPullFtSupported()
                + ", isGeoPullSupported = " + capInfo.isGeoPullSupported()
                + ", isGeoPushSupported = " + capInfo.isGeoPushSupported()
                + ", isSmSupported = " + capInfo.isSmSupported()
                + ", isFullSnFGroupChatSupported = " + capInfo.isFullSnFGroupChatSupported()
                + ", isRcsIpVoiceCallSupported = " + capInfo.isRcsIpVoiceCallSupported()
                + ", isRcsIpVideoCallSupported = " + capInfo.isRcsIpVideoCallSupported()
                + ", isRcsIpVideoOnlyCallSupported = " + capInfo.isRcsIpVideoOnlyCallSupported()
                + ", timestamp = " + capInfo.getCapTimestamp();
        String[] extensions = capInfo.getExts();
        for (int i = 0; i < extensions.length; ++i) {
            info += ",";
            info += extensions[i];
        }
        return info;
    }
}
