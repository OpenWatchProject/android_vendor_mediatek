package com.mediatek.presence.core.ims.network.registration;

import java.util.ArrayList;
import java.util.List;

import com.mediatek.presence.core.ims.network.sip.FeatureTags;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsManager;

/**
 * Registration utility functions
 *
 * @author jexa7410
 */
public class RegistrationUtils {
    /**
     * Get supported feature tags for registration
     *
     * @param slotId slot index
     * @return List of tags
     */
    public static List<String> getSupportedFeatureTags(int slotId) {
        List<String> tags = new ArrayList<String>();

        // IM support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isImSessionSupported()) {
            tags.add(FeatureTags.FEATURE_OMA_IM);
        }

        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCPMSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_CPM_SESSION);
        }

        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCPMPagerModeSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_PAGER_MSG);
        }
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCPMLargeModeSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_LARGE_MSG);
        }
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCPMFTSupported()) {
            tags.add(FeatureTags.FEATURE_RCSE_CPM_FT);
        }

        // Video share support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isVideoSharingSupported()) {
            tags.add(FeatureTags.FEATURE_3GPP_VIDEO_SHARE);
        }

        // IP call support
        boolean isDrop2Active = false;
        if (isDrop2Active) {
            if (RcsSettingsManager.getRcsSettingsInstance(slotId).isIPVoiceCallSupported()) {
                tags.add(FeatureTags.FEATURE_RCSE_IP_VOICE_CALL);
                tags.add(FeatureTags.FEATURE_3GPP_IP_VOICE_CALL);
            }
            if (RcsSettingsManager.getRcsSettingsInstance(slotId).isIPVideoCallSupported()) {
                tags.add(FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL);
            }
        }
        // Automata support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isSipAutomata()) {
            tags.add(FeatureTags.FEATURE_SIP_AUTOMATA);
        }

        String additionalTags = "";

        // Image share support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isImageSharingSupported()) {
            additionalTags += FeatureTags.FEATURE_RCSE_IMAGE_SHARE + ",";
        }

        // Geoloc push support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isGeoLocationPushSupported()) {
            additionalTags += FeatureTags.FEATURE_RCSE_GEOLOCATION_PUSH + ",";
        }

        // File transfer HTTP support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isFileTransferHttpSupported()) {
            additionalTags += FeatureTags.FEATURE_RCSE_FT_HTTP;
        }

        // Cloud File Support
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isCloudFileTransferSupported()) {
            tags.add(FeatureTags.FEATURE_CMCC_IARI_CLOUD_FILE);
            // additionalTags += FeatureTags.FEATURE_CMCC_CLOUD_FILE;
        }
        if (RcsSettingsManager.getRcsSettingsInstance(slotId).isPayedEmoticonSupported()) {
            tags.add(FeatureTags.FEATURE_CMCC_IARI_EMOTICON);
        }
        // Add RCS-e prefix
        if (additionalTags.length() != 0) {
            if (additionalTags.endsWith(",")) {
                additionalTags = additionalTags.substring(0, additionalTags.length() - 1);
            }
            additionalTags = FeatureTags.FEATURE_RCSE + "=\"" + additionalTags + "\"";
            tags.add(additionalTags);
        }

        return tags;
    }
}
