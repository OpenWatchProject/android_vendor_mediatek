package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.tests.helper.LoggerService;
import com.mediatek.camera.tests.v3.arch.Observer;
import com.mediatek.camera.tests.v3.util.Condition;
import com.mediatek.camera.tests.v3.util.Utils;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;

public abstract class AbstractLogObserver extends Observer {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            AbstractLogObserver.class.getSimpleName());

    private String[] mObservedKeyList;
    private String[] mObservedTagList;
    private LoggerService mLoggerService;
    private LoggerService.LoggerCallback mLoggerCallback;
    private boolean mIsInCatchingLog = false;
    private List<String> mResult = new LinkedList<String>();

    @Override
    protected final void doBeginObserve(int index) {
        onObserveBegin(index);
        mLoggerService = LoggerService.getInstance();
        mIsInCatchingLog = true;
        applyAdbCommandBeforeStarted();
        mLoggerCallback = new LoggerCallbackImpl(index);
        mObservedTagList = getObservedTagList(index);
        mObservedKeyList = getObservedKeyList(index);
        if (mObservedKeyList == null) {
            mObservedKeyList = new String[mObservedTagList.length];
            for (int i = 0; i < mObservedKeyList.length; i++) {
                mObservedKeyList[i] = "";
            }
        } else {
            for (int i = 0; i < mObservedKeyList.length; i++) {
                if (mObservedKeyList[i] == null) {
                    mObservedKeyList[i] = "";
                }
            }
        }
        assert (mObservedTagList.length == mObservedKeyList.length);
        mLoggerService.startCatchingLog(mObservedTagList, mObservedKeyList, mLoggerCallback);
    }

    @Override
    protected final void doEndObserve(int index) {
        if (SystemProperties.getInt("vendor.mtk.camera.app.enablelogobserve", 0) != 0) {
            Utils.waitCondition(new Condition() {
                @Override
                public boolean isSatisfied() {
                    return isAlreadyFindTarget(index);
                }
            });
        }

        mLoggerService.stopCatchingLog(mObservedTagList, mLoggerCallback);
        clearAdbCommandAfterStopped();
        mLoggerService = null;
        mIsInCatchingLog = false;
        mObservedTagList = null;
        mObservedKeyList = null;
        if (SystemProperties.getInt("vendor.mtk.camera.app.enablelogobserve", 0) != 0) {
            onObserveEnd(index);
        }
    }

    // When find the log you filter, will call this function, you can override this function to
    // do other things
    protected abstract void onLogComing(int index, String line);

    // After end observe, will call this function, you can assert pass and fail in this time
    protected abstract void onObserveEnd(int index);

    // Before begin observe, will call this function, you can reset some variable in this time
    protected abstract void onObserveBegin(int index);

    // Before end observe, will call this function, if return false, it will delay stop watching.
    // If not observe target in 10s, it will stop watching by force. If return true, it will stop
    // watching right now.
    protected abstract boolean isAlreadyFindTarget(int index);

    protected abstract String[] getObservedTagList(int index);

    protected abstract String[] getObservedKeyList(int index);

    /**
     * Some observer need to open some adb command to catch special log. The observer should
     * override this method to invoke setAdbCommand to set adb command before log start catching.
     */
    protected void applyAdbCommandBeforeStarted() {
    }

    /**
     * Clear adb command after log catching stopped.
     */
    protected void clearAdbCommandAfterStopped() {
    }

    /**
     * Set adb command to open or close some log.
     *
     * @param key   The command string.
     * @param value The command value.
     */
    protected final void setAdbCommand(String key, String value) {
        mLoggerService.setAdbCommand(key, value);
    }

    private static long getTimeMillsFromLog(String logLine) {
        String logDateString = logLine.substring(0, "01-02 23:36:48.561".length());
        logDateString = Calendar.getInstance().get(Calendar.YEAR) + "-" + logDateString;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        try {
            Date logDate = sdf.parse(logDateString);
            return logDate.getTime();
        } catch (ParseException e) {
            return 0;
        }
    }

    private boolean isWantedLog(String logLine) {
        if (logLine.indexOf("CamAp_AT_") >= 0) {
            return false;
        }

        if (mObservedTagList == null && mObservedKeyList != null) {
            for (String key : mObservedKeyList) {
                if (logLine.contains(key)) {
                    return true;
                }
            }
            return false;
        }

        if (mObservedTagList != null && mObservedKeyList == null) {
            return true;
        }

        for (int i = 0; i < mObservedTagList.length; i++) {
            if (logLine.contains(mObservedTagList[i])
                    && logLine.contains(mObservedKeyList[i])) {
                return true;
            }
        }
        return false;
    }

    /**
     * Log callback implement.
     */
    private class LoggerCallbackImpl implements LoggerService.LoggerCallback {
        private long mStartTime = System.currentTimeMillis();
        private boolean mNeedCheckLogTime = true;
        private int mIndex;

        /**
         * Logger callback implement constructor.
         *
         * @param index The index to indicator flow.
         */
        public LoggerCallbackImpl(int index) {
            mIndex = index;
        }

        @Override
        public void onLogStarted() {

        }

        @Override
        public void onLogStopped() {

        }

        @Override
        public void onRecordingFileChanged(String filePath) {

        }

        @Override
        public void onLogReceived(String line) {
            //LogHelper.d(TAG, "[onLogReceived] find line <" + line + ">");
            if (!mIsInCatchingLog) {
                //LogHelper.d(TAG, "[onLogReceived] not in catching log, return");
                return;
            }

            if (mNeedCheckLogTime && getTimeMillsFromLog(line) < mStartTime) {
                //LogHelper.d(TAG, "[onLogReceived] in check log time, return");
                return;
            } else {
                mNeedCheckLogTime = false;
            }

            if (isWantedLog(line)) {
                mResult.add(line);
                //LogHelper.d(TAG, "[onLogReceived] call onLogComing");
                onLogComing(mIndex, line);
            } else {
                //LogHelper.d(TAG, "[onLogReceived] not wanted log, return");
            }
        }
    }
}
