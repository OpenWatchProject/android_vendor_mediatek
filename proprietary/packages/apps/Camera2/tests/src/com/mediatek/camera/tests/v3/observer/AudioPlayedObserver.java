package com.mediatek.camera.tests.v3.observer;

public class AudioPlayedObserver extends LogPrintObserver {
    @Override
    protected String getObserveLogTag(int index) {
        return "AudioSink";
    }

    @Override
    protected String getObserveLogKey(int index) {
        return "";
    }

    @Override
    public int getObserveCount() {
        return 1;
    }
}