package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public class CameraOpenFailedLogObserver extends LogPrintObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(CameraOpenFailedLogObserver.class
            .getSimpleName());

    @Override
    protected void onObserveEnd(int index) {
        if (mHasPrintOut) {
            LogHelper.d(TAG, "[onObserveEnd] open failed pop up, try to reboot phone");
            Utils.rebootDevice();
        }
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return true;
    }

    @Override
    protected String getObserveLogTag(int index) {
        return "CamAp_CameraEx";
    }

    @Override
    protected String getObserveLogKey(int index) {
        return "[openLegacy] exception:";
    }

    @Override
    public int getObserveCount() {
        return 1;
    }
}
