package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observe log to check matrix display restriction is worked.
 */

public class MatrixDisplayRestrictionObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            MatrixDisplayRestrictionObserver.class.getSimpleName());

    private static final String LOG_TAG_FOR_API1 = "MtkCam/ParamsManager";
    private static final String LOG_TAG_FOR_API2 = "MtkCam/MetadataConverter";
    private static final String FACE_DETECTION_TAG_FOR_API1 = "SWFD_MAIN";

    private static final String FACE_DETECTION_KEY_STRING_FOR_API1 = "face";
    private static final String FACE_DETECTION_KEY_STRING_FOR_API2
            = "android.statistics.faceDetectMode";
    private static final String NOISE_REDUCTION_KEY_STRING_FOR_API1 = "3dnr-mode=";
    private static final String NOISE_REDUCTION_KEY_STRING_FOR_API2
            = "com.mediatek.nrfeature.3dnrmode";
    private static final String EIS_KEY_STRING_FOR_API1 = "video-stabilization=";
    private static final String EIS_KEY_STRING_FOR_API2
            = "android.control.videoStabilizationMode";

    public static final int OBSERVE_IN_PHOTO = 0;
    public static final int OBSERVE_IN_VIDEO = 1;

    private boolean mPass = false;
    private int mObserveType = OBSERVE_IN_PHOTO;
    private boolean mFaceDetectionPassed = false;
    private boolean mNoiseReductionPassed = false;
    private boolean mEisPassed = false;
    private String mExpectedFdValue;
    private String mExpectedNoiseReductionValue;
    private String mExpectedEisValue;

    /**
     * Constructor with type to indicate observe under photo mode or video mode.
     *
     * @param type to Indicate observe under photo mode or video mode.
     */
    public MatrixDisplayRestrictionObserver(int type) {
        mObserveType = type;
    }

    @Override
    protected void onLogComing(int index, String line) {
        if (OBSERVE_IN_PHOTO == mObserveType) {
            // just observe FD.
            if (CameraApiHelper.getCameraApiType(null)
                    == CameraDeviceManagerFactory.CameraApi.API1) {
                if ("on".equalsIgnoreCase(mExpectedFdValue)) {
                    mFaceDetectionPassed = (line.indexOf(FACE_DETECTION_KEY_STRING_FOR_API1) > -1);
                } else if ("off".equalsIgnoreCase(mExpectedFdValue)) {
                    mFaceDetectionPassed = false;
                }
            } else if (CameraApiHelper.getCameraApiType(null)
                    == CameraDeviceManagerFactory.CameraApi.API2) {
                int beginIndex = line.indexOf("=> ") + "=> ".length();
                String subString = line.substring(beginIndex, line.length() - 1);
                LogHelper.d(TAG, "[onLogComing], " + FACE_DETECTION_KEY_STRING_FOR_API2
                        + "=" + subString);
                if ("on".equalsIgnoreCase(mExpectedFdValue)) {
                    mFaceDetectionPassed = (Integer.parseInt(subString) == 1);
                } else if ("off".equalsIgnoreCase(mExpectedFdValue)) {
                    mFaceDetectionPassed = (Integer.parseInt(subString) == 0);
                }
            }
            mPass = mFaceDetectionPassed;

        } else if (OBSERVE_IN_VIDEO == mObserveType) {
            if (CameraApiHelper.getCameraApiType(null)
                    == CameraDeviceManagerFactory.CameraApi.API1) {
                int beginIndex;
                int endIndex;

                int indexStr = line.indexOf(NOISE_REDUCTION_KEY_STRING_FOR_API1);
                if (indexStr >= 0) {
                    beginIndex = line.indexOf(NOISE_REDUCTION_KEY_STRING_FOR_API1)
                            + NOISE_REDUCTION_KEY_STRING_FOR_API1.length();
                    endIndex = line.indexOf(";", beginIndex);
                    String subString = line.substring(beginIndex, endIndex);
                    LogHelper.d(TAG, "[onLogComing], 3dnr-mode=" + subString);
                    mNoiseReductionPassed = (subString != null
                            && subString.equals(mExpectedNoiseReductionValue));
                }

                indexStr = line.indexOf(EIS_KEY_STRING_FOR_API1);
                if (indexStr >= 0) {
                    beginIndex = line.indexOf(EIS_KEY_STRING_FOR_API1)
                            + EIS_KEY_STRING_FOR_API1.length();
                    endIndex = line.indexOf(";", beginIndex);
                    String subString = line.substring(beginIndex, endIndex);
                    LogHelper.d(TAG, "[onLogComing], video-stabilization=" + subString);
                    boolean isExpectedOn = "on".equalsIgnoreCase(mExpectedEisValue);
                    mEisPassed = (subString != null
                            && subString.equalsIgnoreCase(
                            isExpectedOn ? Boolean.TRUE.toString() : Boolean.FALSE.toString()));
                }
            } else if (CameraApiHelper.getCameraApiType(null)
                    == CameraDeviceManagerFactory.CameraApi.API2) {
                int beginIndex;

                int indexStr = line.indexOf(NOISE_REDUCTION_KEY_STRING_FOR_API2);
                if (indexStr >= 0) {
                    beginIndex = line.indexOf("=> ") + "=> ".length();
                    String subString = line.substring(beginIndex, line.length() - 1);
                    LogHelper.d(TAG, "[onLogComing], " + NOISE_REDUCTION_KEY_STRING_FOR_API2
                            + "=" + subString);
                    mNoiseReductionPassed = ("on".equalsIgnoreCase(mExpectedNoiseReductionValue)
                            ? (Integer.parseInt(subString) == 1)
                            : (Integer.parseInt(subString) == 0));
                }

                indexStr = line.indexOf(EIS_KEY_STRING_FOR_API2);
                if (indexStr > 0) {
                    beginIndex = line.indexOf("=> ") + "=> ".length();
                    String subString = line.substring(beginIndex, line.length() - 1);
                    LogHelper.d(TAG, "[onLogComing], " + EIS_KEY_STRING_FOR_API2
                            + "=" + subString);
                    mEisPassed = ("on".equalsIgnoreCase(mExpectedEisValue)
                            ? (Integer.parseInt(subString) == 1)
                            : (Integer.parseInt(subString) == 0));
                }
            }
            mPass = mNoiseReductionPassed && mEisPassed;
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mPass);
    }

    @Override
    protected void onObserveBegin(int index) {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            if (OBSERVE_IN_PHOTO == mObserveType) {
                if ("on".equalsIgnoreCase(mExpectedFdValue)) {
                    mPass = false;
                } else if ("off".equalsIgnoreCase(mExpectedFdValue)) {
                    mPass = true;
                }
                return;
            }
        }
        mPass = false;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mPass;
    }

    @Override
    protected void applyAdbCommandBeforeStarted() {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            setAdbCommand("debug.camera.log.AppStreamMgr", "2");
        }
    }

    @Override
    protected void clearAdbCommandAfterStopped() {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            setAdbCommand("debug.camera.log.AppStreamMgr", "0");
        }
    }

    @Override
    protected String[] getObservedTagList(int index) {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            if (OBSERVE_IN_PHOTO == mObserveType) {
                return new String[]{FACE_DETECTION_TAG_FOR_API1};

            } else if (OBSERVE_IN_VIDEO == mObserveType) {
                return new String[]{LOG_TAG_FOR_API1, LOG_TAG_FOR_API1};

            }
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            if (OBSERVE_IN_PHOTO == mObserveType) {
                return new String[]{LOG_TAG_FOR_API2};

            } else if (OBSERVE_IN_VIDEO == mObserveType) {
                return new String[]{LOG_TAG_FOR_API2, LOG_TAG_FOR_API2};

            }
        }
        return null;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            if (OBSERVE_IN_PHOTO == mObserveType) {
                return new String[]{FACE_DETECTION_KEY_STRING_FOR_API1};

            } else if (OBSERVE_IN_VIDEO == mObserveType) {
                return new String[]{NOISE_REDUCTION_KEY_STRING_FOR_API1, EIS_KEY_STRING_FOR_API1};
            }
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            if (OBSERVE_IN_PHOTO == mObserveType) {
                return new String[]{FACE_DETECTION_KEY_STRING_FOR_API2};

            } else if (OBSERVE_IN_VIDEO == mObserveType) {
                return new String[]{NOISE_REDUCTION_KEY_STRING_FOR_API2, EIS_KEY_STRING_FOR_API2};
            }
        }
        return null;
    }

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "check matrix display restriction";
    }

    /**
     * Expect the observer can observe the input result.
     *
     * @param faceDetectionValue  The expected face detection value.
     * @param noiseReductionValue The expected noise reduction value.
     * @param eisValue            The expeceted EIS value.
     * @return This instance.
     */
    public MatrixDisplayRestrictionObserver expectResult(String faceDetectionValue,
                                                         String noiseReductionValue,
                                                         String eisValue) {
        mExpectedFdValue = faceDetectionValue;
        mExpectedNoiseReductionValue = noiseReductionValue;
        mExpectedEisValue = eisValue;
        return this;
    }

    @Override
    public boolean isSupported(int index) {
        return Utils.isFeatureSupported("com.mediatek.camera.at.matrix-display");
    }
}
