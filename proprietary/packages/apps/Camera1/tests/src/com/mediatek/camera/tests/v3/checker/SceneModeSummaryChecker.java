package com.mediatek.camera.tests.v3.checker;

/**
 * Check the scene mode setting summary.
 */

public class SceneModeSummaryChecker extends SettingRadioOptionChecker {
    private static final String SCENE_MODE_NAME = "Scene mode";

    public static final String VALUE_OFF = "Off";
    public static final String VALUE_AUTO = "Auto";
    public static final String VALUE_NIGHT = "Night";
    public static final String VALUE_SUNSET = "Sunset";
    public static final String VALUE_PARTY = "Party";
    public static final String VALUE_PORTRAIT = "Portrait";
    public static final String VALUE_LANDSCAPE = "Landscape";
    public static final String VALUE_NIGHT_PORTRAIT = "Night portrait";
    public static final String VALUE_THEATRE = "Theatre";
    public static final String VALUE_BEACH = "Beach";
    public static final String VALUE_SNOW = "Snow";
    public static final String VALUE_STEADY_PHOTO = "Steady photo";
    public static final String VALUE_FIREWORKS = "Fireworks";
    public static final String VALUE_SPORTS = "Sports";
    public static final String VALUE_CANDLE_LIGHTS = "Candle light";

    private static final String[] OPTIONS = {
            "Off",
            "Auto",
            "Night",
            "Sunset",
            "Party",
            "Portrait",
            "Landscape",
            "Night portrait",
            "Theatre",
            "Beach",
            "Snow",
            "Steady photo",
            "Fireworks",
            "Sports",
            "Candle light"
    };

    /**
     * Check the scene mode setting summary is equal to the input option.
     *
     * @param option The check option.
     */
    public SceneModeSummaryChecker(String option) {
        super(SCENE_MODE_NAME, option);
    }
}
