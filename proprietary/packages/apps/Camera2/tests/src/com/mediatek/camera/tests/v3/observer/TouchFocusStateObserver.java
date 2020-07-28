package com.mediatek.camera.tests.v3.observer;


import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observer used to check focus state is right.
 */
public class TouchFocusStateObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(TouchFocusStateObserver.class
            .getSimpleName());
    private static final String LOG_TAG_FOCUS = "CamAp_Focus-";
    private static final String LOG_TAG_AF_MGR = "af_mgr_v3";
    private static final String LOG_TAG_FRAMEWORK = "CameraFramework";
    private int mExpectedIndex = 0;
    private boolean mIsTouchFocusDone = false;
    private CameraDeviceManagerFactory.CameraApi mCameraApi = null;
    private boolean mIsAutoModeOnly = false;
    private boolean mIsApLogOnly = false;
    public static final int INDEX_BACK = 0;
    public static final int INDEX_FRONT = 1;

    private static final String[] LOG_KEY_LIST_API1 = new String[]{
            "[onSingleTapUp] +",
            "[onSingleTapUp]-",
            "ctl_afmode(1)",
            "EVENT_CMD_CHANGE_MODE",
            "EVENT_CMD_SET_AF_REGION",
            "EVENT_CMD_AUTOFOCUS",
            "handleMessage: 4",
            "active focus done"
    };

    private static final String[] LOG_KEY_LIST_API1_AUTO_ONLY = new String[]{
            "[onSingleTapUp] +",
            "[onSingleTapUp]-",
            "EVENT_CMD_SET_AF_REGION",
            "EVENT_CMD_AUTOFOCUS",
            "handleMessage: 4",
            "active focus done"
    };

    private static final String[] LOG_KEY_LIST_API2 = new String[]{
            "[onSingleTapUp] +",
            "[onSingleTapUp]-",
            "ctl_afmode(1)",
            "EVENT_CMD_CHANGE_MODE",
            "EVENT_CMD_SET_AF_REGION",
            "EVENT_CMD_AUTOFOCUS",
            "active focus done"
    };

    private static final String[] LOG_KEY_LIST_API2_AUTO_ONLY = new String[]{
            "[onSingleTapUp] +",
            "[onSingleTapUp]-",
            "EVENT_CMD_SET_AF_REGION",
            "EVENT_CMD_AUTOFOCUS",
            "active focus done"
    };

    /**
     * Init touch focus state environment,set camera api level and whether current focus mode is
     * restricted to auto only by case.
     * Slow motion case will run in API2,the log checked in test
     * case should be API2,but the platform is still in API1.so force set camera api level to
     * API2 in slow motion cases.
     *
     * @param api         The camera api level.
     * @param isApLogOnly Whether is app log need to be checked only.
     * @param isAutoMode  Whether current focus mode is auto only.
     */
    public void initEnv(CameraDeviceManagerFactory.CameraApi api, boolean
            isApLogOnly, boolean isAutoMode) {
        mCameraApi = api;
        mIsApLogOnly = isApLogOnly;
        mIsAutoModeOnly = isAutoMode;
    }

    @Override
    protected String[] getObservedTagList(int index) {
        if (mCameraApi == null) {
            mCameraApi = CameraApiHelper.getCameraApiType(null);
        }
        String focusTag = LOG_TAG_FOCUS + index;
        if (mIsApLogOnly) {
            return new String[]{
                    focusTag,
                    focusTag,
                    focusTag
            };
        }
        switch (mCameraApi) {
            case API1:
                if (mIsAutoModeOnly) {
                    return new String[]{
                            focusTag,
                            focusTag,
                            LOG_TAG_AF_MGR,
                            LOG_TAG_AF_MGR,
                            LOG_TAG_FRAMEWORK,
                            focusTag
                    };
                }
                return new String[]{
                        focusTag,
                        focusTag,
                        LOG_TAG_AF_MGR,
                        LOG_TAG_AF_MGR,
                        LOG_TAG_AF_MGR,
                        LOG_TAG_AF_MGR,
                        LOG_TAG_FRAMEWORK,
                        focusTag
                };
            case API2:
                if (mIsAutoModeOnly) {
                    return new String[]{
                            focusTag,
                            focusTag,
                            LOG_TAG_AF_MGR,
                            LOG_TAG_AF_MGR,
                            focusTag,
                    };
                }
                return new String[]{
                        focusTag,
                        focusTag,
                        LOG_TAG_AF_MGR,
                        LOG_TAG_AF_MGR,
                        LOG_TAG_AF_MGR,
                        LOG_TAG_AF_MGR,
                        focusTag,
                };
            default:
                return null;
        }
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        if (mCameraApi == null) {
            mCameraApi = CameraApiHelper.getCameraApiType(null);
        }
        if (mIsApLogOnly) {
            return new String[]{
                    "[onSingleTapUp] +",
                    "[onSingleTapUp]-",
                    "active focus done"
            };
        }
        switch (mCameraApi) {
            case API1:
                if (mIsAutoModeOnly) {
                    return LOG_KEY_LIST_API1_AUTO_ONLY;
                }
                return LOG_KEY_LIST_API1;
            case API2:
                if (mIsAutoModeOnly) {
                    return LOG_KEY_LIST_API2_AUTO_ONLY;
                }
                return LOG_KEY_LIST_API2;
            default:
                return null;
        }
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mExpectedIndex == getObservedTagList(index).length && mIsTouchFocusDone;
    }

    @Override
    public int getObserveCount() {
        return 2;
    }


    @Override
    protected void onLogComing(int index, String line) {
        if (mExpectedIndex >= getObservedTagList(index).length) {
            return;
        }
        if (line.contains(getObservedTagList(index)[mExpectedIndex])
                && line.contains(getObservedKeyList(index)[mExpectedIndex])) {
            if (line.contains("active focus done")) {
                mIsTouchFocusDone = true;
            }
            mExpectedIndex++;
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        int length = getObservedTagList(index).length;
        Utils.assertRightNow(mExpectedIndex == length, "[onObserveEnd] mExpectedIndex " +
                mExpectedIndex + "," +
                "length " + length + ",mIsAutoModeOnly " + mIsAutoModeOnly);
        Utils.assertRightNow(mIsTouchFocusDone, "[onObserveEnd] touch focus does not done");
    }

    @Override
    protected void onObserveBegin(int index) {
        mExpectedIndex = 0;
        mIsTouchFocusDone = false;
    }

    @Override
    public String getDescription(int index) {
        StringBuilder sb = new StringBuilder("Observe this group tag has printed out as order, ");
        for (int i = 0; i < getObservedTagList(index).length; i++) {
            sb.append("[TAG-" + i + "] ");
            sb.append(getObservedTagList(index)[i]);
            sb.append(",[KEY-" + i + "] ");
            sb.append(getObservedKeyList(index)[i]);
            sb.append("\n");
        }
        return sb.toString();
    }

}
