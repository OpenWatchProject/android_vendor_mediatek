package com.mediatek.gallerybasic.dynamic;

import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.Player;
import com.mediatek.gallerybasic.util.DebugUtils;
import com.mediatek.gallerybasic.util.Log;

class PlayList {
    private static final String TAG = "MtkGallery2/PlayList";

    public static final int INVALIDE = 0xffffffff;
    private Entry[] mList;
    private Entry[] mReleaseList;
    private EntryFilling mFilling;

    public static class Entry {
        public MediaData data;
        public Player player;
        public int threadIndex = INVALIDE;

        public String toString() {
            return "[ data = " + data + ", player = " + player + ", threadIndex = "
                    + threadIndex + "]";
        }
    }

    public interface EntryFilling {
        public void fillEntry(int index, Entry entry);

        public void updateEntry(int index, Entry entry);
    }

    public PlayList(int length, EntryFilling fill) {
        mList = new Entry[length];
        mFilling = fill;
    }

    public Entry get(int index) {
        return mList[index];
    }

    public void update(MediaData[] data) {
        assert (data.length == mList.length);
        mReleaseList = mList;
        mList = new Entry[mReleaseList.length];
        for (int i = 0; i < data.length; i++) {
            if (data[i] == null) {
                mList[i] = new Entry();
                mFilling.updateEntry(i, mList[i]);
                continue;
            }
            int findIndex = findEntryByMediaData(mReleaseList, data[i]);
            if (findIndex == INVALIDE) {
                mList[i] = new Entry();
                mList[i].data = data[i];
                mFilling.fillEntry(i, mList[i]);
            } else {
                mList[i] = mReleaseList[findIndex];
                mFilling.updateEntry(i, mList[i]);
                mReleaseList[findIndex] = null;
            }
        }
        logEntrys("<After update, mList>", mList);
        logEntrys("<After update, mReleaseList>", mReleaseList);
    }

    public final Entry[] getReleaseList() {
        return mReleaseList;
    }

    public final Entry[] getList() {
        return mList;
    }

    private static int findEntryByMediaData(Entry[] list, MediaData data) {
        int len = list.length;
        for (int i = 0; i < len; i++) {
            if (list[i] != null && list[i].data != null
                    && list[i].data.equals(data)) {
                return i;
            }
        }
        return INVALIDE;
    }

    private void logEntrys(String tag, Entry[] el) {
        if (!DebugUtils.DEBUG_PLAY_ENGINE) {
            return;
        }
        Log.d(TAG, tag + " begin ----------------------------------------");
        for (int i = 0; i < el.length; i++) {
            Log.d(TAG, tag + " [" + i + "] = " + el[i]);
        }
        Log.d(TAG, tag + " end ----------------------------------------");
    }
}