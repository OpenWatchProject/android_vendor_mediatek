package com.mediatek.calendar.features;


import java.util.ArrayList;

/**
 * A class to manage the features and operators.
 * TODO: to Change this class to some small classes, to get a better performance and comprehension.
 */
public class Features {

    private static final int FEATURE_CLEAR_ALL_EVENTS = 3;
    private static final int FEATURE_THEME_MANAGER = 4;
    private static final int FEATURE_BEAM_PLUS = 5;

    private static final ArrayList<Integer> COMMON_FEATURES = new ArrayList<Integer>();


    public static boolean isThemeManagerEnabled() {
        return isFeatureEnabled(FEATURE_THEME_MANAGER);
    }

    /**
     * is the given feature name is needed by current operator
     * @param feature
     * @return
     */
    private static boolean isFeatureEnabled(int feature) {
        return COMMON_FEATURES.contains(feature);
    }

    /**
     * In fact, NFC(in beam plus) is a common feature.
     * @return
     */
    public static boolean isBeamPlusEnabled() {
        return isFeatureEnabled(FEATURE_BEAM_PLUS);
    }
}
