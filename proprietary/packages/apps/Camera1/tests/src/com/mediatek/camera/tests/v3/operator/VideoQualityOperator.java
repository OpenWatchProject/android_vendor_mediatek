package com.mediatek.camera.tests.v3.operator;

/**
 * video quality operator.
 */
public class VideoQualityOperator extends SettingRadioButtonOperator {
    public static final int INDEX_BACK_4K = 0;
    public static final int INDEX_BACK_FHD = 1;
    public static final int INDEX_BACK_HD = 2;
    public static final int INDEX_BACK_QCIF = 3;

    public static final int INDEX_FRONT_VGA = 0;
    public static final int INDEX_FRONT_QCIF = 1;

    private static final String TITLE = "Video quality";
    private static final String[] OPTION_TITLE_BACK = {"4K", "FHD", "HD", "QCIF"};
    private static final String[] OPTION_TITLE_FRONT = {"VGA", "QCIF"};

    private boolean mIsBackCamera = true;

    public VideoQualityOperator(boolean isBackCamera) {
        mIsBackCamera = isBackCamera;
    }

    @Override
    protected int getSettingOptionsCount() {
        if (mIsBackCamera) {
            return OPTION_TITLE_BACK.length;
        } else {
            return OPTION_TITLE_FRONT.length;
        }

    }

    @Override
    protected String getSettingTitle() {
        return TITLE;
    }

    @Override
    protected String getSettingOptionTitle(int index) {
        if (mIsBackCamera) {
            return OPTION_TITLE_BACK[index];
        } else {
            return OPTION_TITLE_FRONT[index];
        }

    }
}
