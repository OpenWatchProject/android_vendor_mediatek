package com.mediatek.camera.tests.v3.observer;

public class IntentVideoRecordingObserver extends LogPrintObserver {
    @Override
    protected String getObserveLogTag(int index) {
        return "CamAp_VideoUI";
    }

    @Override
    protected String getObserveLogKey(int index) {
        return "[updateRecordingSize] mUIState = STATE_RECORDING";
    }

    @Override
    public int getObserveCount() {
        return 1;
    }
}
