package com.mediatek.camera.feature.mode.slowmotion;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

/**
 * Slow motion restriction.
 */

public class SlowMotionRestriction {
    private static final String SLOW_MOTION_KEY =
            "com.mediatek.camera.feature.mode.slowmotion.SlowMotionMode";
    private static final String VALUE_OFF = "off";
    private static RelationGroup sRelationGroup = new RelationGroup();

    private static final String KEY_EIS = "key_eis";
    private static final String KEY_SCENE_MODE = "key_scene_mode";
    private static final String KEY_MICROPHONE = "key_microphone";
    private static final String KEY_HDR = "key_hdr";
    private static final String KEY_COLOR_EFFECT = "key_color_effect";
    private static final String KEY_FLASH = "key_flash";
    private static final String KEY_WHITE_BALANCE = "key_white_balance";
    private static final String KEY_NOISE_REDUCTION = "key_noise_reduction";
    private static final String KEY_CAMERA_SWITCHER = "key_camera_switcher";
    private static final String KEY_VIDEO_QUALITY = "key_video_quality";
    private static final String KEY_FOCUS = "key_focus";
    private static final String KEY_ANTI_FLICKER = "key_anti_flicker";
    private static final String KEY_IMAGE_PROPERTIES = "key_image_properties";
    private static final String KEY_BRIGHTNESS = "key_brightness";
    private static final String KEY_CONTRAST = "key_contrast";
    private static final String KEY_HUE = "key_hue";
    private static final String KEY_SATURATION = "key_saturation";
    private static final String KEY_SHARPNESS = "key_sharpness";
    private static final String KEY_FACE_DETECTION = "key_face_detection";
    private static final String KEY_CSHOT = "key_continuous_shot";
    private static final String KEY_DUAL_ZOOM = "key_dual_zoom";
    private static final String KEY_FPS60 = "key_fps60";

    static {
        sRelationGroup.setHeaderKey(SLOW_MOTION_KEY);
        sRelationGroup.setBodyKeys(
                KEY_EIS + "," +
                KEY_SCENE_MODE + "," +
                KEY_MICROPHONE + "," +
                KEY_HDR + "," +
                KEY_COLOR_EFFECT + "," +
                KEY_FLASH + "," +
                KEY_WHITE_BALANCE + "," +
                KEY_NOISE_REDUCTION + "," +
                KEY_CAMERA_SWITCHER + "," +
                KEY_VIDEO_QUALITY + "," +
                KEY_FOCUS + "," +
                KEY_ANTI_FLICKER + "," +
                KEY_IMAGE_PROPERTIES + "," +
                KEY_BRIGHTNESS + "," +
                KEY_CONTRAST + "," +
                KEY_HUE + "," +
                KEY_SATURATION + "," +
                KEY_SHARPNESS + "," +
                KEY_FACE_DETECTION + "," +
                KEY_CSHOT + "," +
                KEY_DUAL_ZOOM + "," +
                KEY_FPS60);
        sRelationGroup.addRelation(
                new Relation.Builder(SLOW_MOTION_KEY, "preview")
                        .addBody(KEY_EIS, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_SCENE_MODE, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_HDR, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_NOISE_REDUCTION, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_MICROPHONE, VALUE_OFF, VALUE_OFF)
                        //flash restriction will change in slowmotionmode.java dynamically.
                        .addBody(KEY_FLASH, VALUE_OFF, VALUE_OFF)
                        .addBody(KEY_BRIGHTNESS, "middle", "middle")
                        .addBody(KEY_CONTRAST, "middle", "middle")
                        .addBody(KEY_HUE, "middle", "middle")
                        .addBody(KEY_SATURATION, "middle", "middle")
                        .addBody(KEY_SHARPNESS, "middle", "middle")
                        .addBody(KEY_WHITE_BALANCE, "auto", "auto")
                        .addBody(KEY_ANTI_FLICKER, "auto", "auto")
                        .addBody(KEY_CAMERA_SWITCHER, "back", "back")
                        .addBody(KEY_COLOR_EFFECT, "none", "none")
                        //quality value will change in slowmotionmode.java according current value.
                        .addBody(KEY_VIDEO_QUALITY, "109", "109")
                        .addBody(KEY_FOCUS, "continuous-video", "continuous-video,auto")
                        .addBody(KEY_FACE_DETECTION, "off", "off")
                        .addBody(KEY_CSHOT, "off", "off")
                        .addBody(KEY_DUAL_ZOOM, "off", "off")
                        .addBody(KEY_FPS60,"off", "off")
                        .build());
    }

    /**
     * Slow motion restriction witch are have setting ui.
     * @return restriction list.
     */
    static RelationGroup getPreviewRelation() {
        return sRelationGroup;
    }

}
