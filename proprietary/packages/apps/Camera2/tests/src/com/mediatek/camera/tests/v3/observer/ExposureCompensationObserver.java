package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observer used to check ExposureCompensation value is right.
 */
public class ExposureCompensationObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ExposureCompensationObserver.class
            .getSimpleName());
    private static final String LOG_TAG_EV_API1 = "CamAp_ExposureParamete";
    private static final String LOG_TAG_EV_API2 = "CamAp_ExposureCaptureR";
    private static final String LOG_TAG_AE_MGR = "ae_mgr";
    private CameraDeviceManagerFactory.CameraApi mCameraApi = null;
    private int mExpectedIndex = 0;
    private int mExpectedEv = 0;
    private boolean mIsOnlyCheckApLog = false;
    private String[] mTag = LOG_TAG_LIST_API1;
    private String[] mKey = new String[]{
            "[configParameters] exposureCompensationIndex = " + mExpectedEv,
            "m_i4EVIndex: 0"
    };

    private static final String[] LOG_TAG_LIST_API1 = new String[]{
            LOG_TAG_EV_API1,
            LOG_TAG_AE_MGR
    };

    private static final String[] LOG_TAG_LIST_API2 = new String[]{
            LOG_TAG_EV_API2,
            LOG_TAG_AE_MGR
    };

    /**
     * The constructor.
     *
     * @param ev The value of expected ev.
     */
    public ExposureCompensationObserver(int ev) {
        mExpectedEv = ev;
    }

    /**
     * Only need check application log when default configure ev to 0 or not.
     *
     * @param api              The camera api level.
     * @param isOnlyCheckApLog True if only check application log,false otherwise.
     */
    public void initEnv(CameraDeviceManagerFactory.CameraApi api, boolean isOnlyCheckApLog) {
        mCameraApi = api;
        mIsOnlyCheckApLog = isOnlyCheckApLog;
    }

    @Override
    protected String[] getObservedTagList(int index) {
        if (mCameraApi == null) {
            mCameraApi = CameraApiHelper.getCameraApiType(null);
        }
        switch (mCameraApi) {
            case API1:
                if (!mIsOnlyCheckApLog) {
                    mTag = LOG_TAG_LIST_API1;
                } else {
                    mTag = new String[]{LOG_TAG_EV_API1};
                }
                break;
            case API2:
                if (!mIsOnlyCheckApLog) {
                    mTag = LOG_TAG_LIST_API2;
                } else {
                    mTag = new String[]{LOG_TAG_EV_API2};
                }
                break;
            default:
                break;
        }
        return mTag;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        if (mCameraApi == null) {
            mCameraApi = CameraApiHelper.getCameraApiType(null);
        }
        String[] keyListApi1 = new String[]{
                "[configParameters] exposureCompensationIndex = " + mExpectedEv,
                "m_i4EVIndex: " + mExpectedEv
        };
        String[] keyListApi2 = new String[]{
                "[addBaselineCaptureKeysToRequest] exposureCompensationIndex " + mExpectedEv,
                "m_i4EVIndex: " + mExpectedEv
        };
        switch (mCameraApi) {
            case API1:
                if (!mIsOnlyCheckApLog) {
                    mKey = keyListApi1;
                } else {
                    mKey = new String[]{"[configParameters] exposureCompensationIndex = 0"};
                }
                break;
            case API2:
                if (!mIsOnlyCheckApLog) {
                    mKey = keyListApi2;
                } else {
                    mKey = new String[]{"[addBaselineCaptureKeysToRequest] " +
                            "exposureCompensationIndex 0"};
                }
                break;
            default:
                break;
        }
        return mKey;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mExpectedIndex == getObservedTagList(index).length;
    }

    @Override
    public int getObserveCount() {
        if (mIsOnlyCheckApLog) {
            return 1;
        } else {
            return 2;
        }
    }


    @Override
    protected void onLogComing(int index, String line) {
        if (mExpectedIndex >= getObservedTagList(index).length) {
            return;
        }
        if (line.contains(getObservedTagList(index)[mExpectedIndex])
                && line.contains(getObservedKeyList(index)[mExpectedIndex])) {
            mExpectedIndex++;
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        int length = getObservedTagList(index).length;
        Utils.assertRightNow(mExpectedIndex == length, "[onObserveEnd] mExpectedIndex " +
                mExpectedIndex + "," +
                "length " + length + ",mIsOnlyCheckApLog = " + mIsOnlyCheckApLog);
        mExpectedEv = 0;
    }

    @Override
    protected void onObserveBegin(int index) {
        mExpectedIndex = 0;
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
