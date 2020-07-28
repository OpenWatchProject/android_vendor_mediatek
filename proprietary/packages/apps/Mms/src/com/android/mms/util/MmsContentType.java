package com.android.mms.util;

import com.google.android.mms.ContentType;

import java.util.ArrayList;

public class MmsContentType {

    public static final String MMS_MESSAGE       = ContentType.MMS_MESSAGE;
    // The phony content type for generic PDUs (e.g. ReadOrig.ind,
    // Notification.ind, Delivery.ind).
    public static final String MMS_GENERIC       = ContentType.MMS_GENERIC;
    public static final String MULTIPART_MIXED   = ContentType.MULTIPART_MIXED;
    public static final String MULTIPART_RELATED = ContentType.MULTIPART_RELATED;
    public static final String MULTIPART_ALTERNATIVE = ContentType.MULTIPART_ALTERNATIVE;

    public static final String TEXT_PLAIN        = ContentType.TEXT_PLAIN;
    public static final String TEXT_HTML         = ContentType.TEXT_HTML;
    public static final String TEXT_VCALENDAR    = ContentType.TEXT_VCALENDAR;
    public static final String TEXT_VCARD        = ContentType.TEXT_VCARD;

    public static final String IMAGE_UNSPECIFIED = ContentType.IMAGE_UNSPECIFIED;
    public static final String IMAGE_JPEG        = ContentType.IMAGE_JPEG;
    public static final String IMAGE_JPG         = ContentType.IMAGE_JPG;
    public static final String IMAGE_GIF         = ContentType.IMAGE_GIF;
    public static final String IMAGE_WBMP        = ContentType.IMAGE_WBMP;
    public static final String IMAGE_PNG         = ContentType.IMAGE_PNG;
    public static final String IMAGE_X_MS_BMP    = ContentType.IMAGE_X_MS_BMP;

    public static final String AUDIO_UNSPECIFIED = ContentType.AUDIO_UNSPECIFIED;
    public static final String AUDIO_AAC         = ContentType.AUDIO_AAC;
    public static final String AUDIO_AMR         = ContentType.AUDIO_AMR;
    public static final String AUDIO_IMELODY     = ContentType.AUDIO_IMELODY;
    public static final String AUDIO_MID         = ContentType.AUDIO_MID;
    public static final String AUDIO_MIDI        = ContentType.AUDIO_MIDI;
    public static final String AUDIO_MP3         = ContentType.AUDIO_MP3;
    public static final String AUDIO_MPEG3       = ContentType.AUDIO_MPEG3;
    public static final String AUDIO_MPEG        = ContentType.AUDIO_MPEG;
    public static final String AUDIO_MPG         = ContentType.AUDIO_MPG;
    public static final String AUDIO_MP4         = ContentType.AUDIO_MP4;
    public static final String AUDIO_X_MID       = ContentType.AUDIO_X_MID;
    public static final String AUDIO_X_MIDI      = ContentType.AUDIO_X_MIDI;
    public static final String AUDIO_X_MP3       = ContentType.AUDIO_X_MP3;
    public static final String AUDIO_X_MPEG3     = ContentType.AUDIO_X_MPEG3;
    public static final String AUDIO_X_MPEG      = ContentType.AUDIO_X_MPEG;
    public static final String AUDIO_X_MPG       = ContentType.AUDIO_X_MPG;
    public static final String AUDIO_3GPP        = ContentType.AUDIO_3GPP;
    public static final String AUDIO_X_WAV       = ContentType.AUDIO_X_WAV;
    public static final String AUDIO_OGG         = ContentType.AUDIO_OGG;

    public static final String VIDEO_UNSPECIFIED = ContentType.VIDEO_UNSPECIFIED;
    public static final String VIDEO_3GPP        = ContentType.VIDEO_3GPP;
    public static final String VIDEO_3G2         = ContentType.VIDEO_3G2;
    public static final String VIDEO_H263        = ContentType.VIDEO_H263;
    public static final String VIDEO_MP4         = ContentType.VIDEO_MP4;

    public static final String APP_SMIL          = ContentType.APP_SMIL;
    public static final String APP_WAP_XHTML     = ContentType.APP_WAP_XHTML;
    public static final String APP_XHTML         = ContentType.APP_XHTML;

    public static final String APP_DRM_CONTENT   = ContentType.APP_DRM_CONTENT;
    public static final String APP_DRM_MESSAGE   = ContentType.APP_DRM_MESSAGE;

    private static final ArrayList<String> sSupportedContentTypes = ContentType.getSupportedTypes();
    private static final ArrayList<String> sSupportedImageTypes = ContentType.getImageTypes();
    private static final ArrayList<String> sSupportedAudioTypes = ContentType.getAudioTypes();
    private static final ArrayList<String> sSupportedVideoTypes = ContentType.getVideoTypes();

    /** M: support some new ContentTypes */
    public static final String APP_OCET_STREAM   = "application/octet-stream";
    public static final String TEXT_TS           = "text/texmacs";

    public static final String IMAGE_BMP         = "image/x-ms-bmp";
    public static final String IMAGE_XBMP        = "image/bmp";

    public static final String AUDIO_WAV         = "audio/wav";
    public static final String AUDIO_AWB         = "audio/amr-wb";
    public static final String AUDIO_WMA         = "audio/x-ms-wma";
    public static final String AUDIO_VORBIS      = "audio/vorbis";
    public static final String AUDIO_AAC_ADTS    = "audio/aac-adts";
    public static final String AUDIO_OGG_EXT     = "audio/ogg";
    public static final String AUDIO_DLNA_ADTS    = "audio/vnd.dlna.adts";

    public static final String VIDEO_TS          = "video/mp2ts";

    /** M: support Restricted mode */
    private static final ArrayList<String> sUnrestrictedContentTypes = new ArrayList<String>();

    static {
        sSupportedContentTypes.add(TEXT_TS);
        sSupportedContentTypes.add(IMAGE_BMP);
        sSupportedContentTypes.add(IMAGE_XBMP);
        sSupportedContentTypes.add(AUDIO_WAV);
        sSupportedContentTypes.add(AUDIO_AWB);
        sSupportedContentTypes.add(AUDIO_WMA);
        sSupportedContentTypes.add(AUDIO_VORBIS);
        sSupportedContentTypes.add(VIDEO_TS);

        // add supported image types
        sSupportedImageTypes.add(IMAGE_BMP);
        sSupportedImageTypes.add(IMAGE_XBMP);

        // add supported audio types
        sSupportedAudioTypes.add(AUDIO_WAV);
        sSupportedAudioTypes.add(AUDIO_AWB);
        sSupportedAudioTypes.add(AUDIO_WMA);
        sSupportedAudioTypes.add(AUDIO_VORBIS);
        sSupportedAudioTypes.add(AUDIO_AAC_ADTS);
        sSupportedAudioTypes.add(AUDIO_OGG_EXT);
        sSupportedAudioTypes.add(AUDIO_DLNA_ADTS);

        // add supported video types
        sSupportedVideoTypes.add(VIDEO_TS);
        sSupportedVideoTypes.add(TEXT_TS);
        //add for attachment enhance by feng
        //add supported ocet_stream type
        sSupportedContentTypes.add(APP_OCET_STREAM);

        /// M:Code analyze 007,add IMAGE,TEXT,AUDIO,VIDEO type into restricted content set @{
        sUnrestrictedContentTypes.add(IMAGE_JPEG);
        sUnrestrictedContentTypes.add(IMAGE_JPG);
        sUnrestrictedContentTypes.add(IMAGE_WBMP);
        sUnrestrictedContentTypes.add(IMAGE_GIF);
        sUnrestrictedContentTypes.add(TEXT_PLAIN);
        sUnrestrictedContentTypes.add(TEXT_TS);
        sUnrestrictedContentTypes.add(AUDIO_MID);
        sUnrestrictedContentTypes.add(AUDIO_MIDI);
        sUnrestrictedContentTypes.add(AUDIO_AMR);
        sUnrestrictedContentTypes.add(AUDIO_VORBIS);
        sUnrestrictedContentTypes.add(AUDIO_X_MID);
        sUnrestrictedContentTypes.add(AUDIO_X_MIDI);
        sUnrestrictedContentTypes.add(VIDEO_3GPP);
        sUnrestrictedContentTypes.add(VIDEO_3G2);
        sUnrestrictedContentTypes.add(VIDEO_MP4);
        sUnrestrictedContentTypes.add(VIDEO_TS);
    }

    public static boolean isSupportedType(String contentType) {
        return ContentType.isSupportedType(contentType);
    }

    /** M: support Restricted mode */
    public static boolean isUnrestrictedType(String contentType) {
        return (null != contentType) && sUnrestrictedContentTypes.contains(contentType);
    }

    public static boolean isTextType(String contentType) {
        return ContentType.isTextType(contentType);
    }

    public static boolean isImageType(String contentType) {
        return ContentType.isImageType(contentType);
    }

    public static boolean isAudioType(String contentType) {
        return ContentType.isAudioType(contentType);
    }

    public static boolean isVideoType(String contentType) {
        return ContentType.isVideoType(contentType);
    }

    public static boolean isDrmType(String contentType) {
        return ContentType.isDrmType(contentType);
    }

    public static boolean isUnspecified(String contentType) {
        return ContentType.isUnspecified(contentType);
    }

    @SuppressWarnings("unchecked")
    public static ArrayList<String> getImageTypes() {
        return (ArrayList<String>) sSupportedImageTypes.clone();
    }

    @SuppressWarnings("unchecked")
    public static ArrayList<String> getAudioTypes() {
        return (ArrayList<String>) sSupportedAudioTypes.clone();
    }

    @SuppressWarnings("unchecked")
    public static ArrayList<String> getVideoTypes() {
        return (ArrayList<String>) sSupportedVideoTypes.clone();
    }

    @SuppressWarnings("unchecked")
    public static ArrayList<String> getSupportedTypes() {
        return (ArrayList<String>) sSupportedContentTypes.clone();
    }
}
