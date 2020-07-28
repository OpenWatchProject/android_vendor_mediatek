package com.mediatek.camera.tests.v3.observer;

public class ThumbnailUpdatedObserver extends LogListPrintObserver {
    private static final String[] LOG_TAG_LIST = new String[]{
            "CamAp_ThumbnailViewMan",
            "CamAp_ThumbnailViewMan",
    };
    private static final String[] LOG_KEY_LIST = new String[]{
            "[updateThumbnailView]...",
            "[updateThumbnailView] set created thumbnail",
    };

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "Observe if thumbnail is updated";
    }

    @Override
    protected String[] getObservedTagList(int index) {
        return LOG_TAG_LIST;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return LOG_KEY_LIST;
    }
}
