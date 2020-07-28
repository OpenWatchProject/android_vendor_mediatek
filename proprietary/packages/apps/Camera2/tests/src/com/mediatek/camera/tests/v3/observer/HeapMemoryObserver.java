package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.arch.Observer;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * For observe the heap change.
 */

public class HeapMemoryObserver extends Observer {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            HeapMemoryObserver.class.getSimpleName());
    private static final long BYTES_IN_KILOBYTE = 1024;
    private Runtime mRuntime = Runtime.getRuntime();
    private long mBegainUsedHeap;
    private long mEndUsedHeap;

    @Override
    public int getObserveCount() {
        return 1;
    }

    @Override
    public String getDescription(int index) {
        return "Observe memory heap size change";
    }

    @Override
    protected void doBeginObserve(int index) {
        mBegainUsedHeap = mRuntime.totalMemory() - mRuntime.freeMemory();
        LogHelper.d(TAG, "begin used heap size(KB): " + mBegainUsedHeap / BYTES_IN_KILOBYTE);
    }

    private static final int MEMORY_MONITOR_TIME_OUT = 20000;

    @Override
    protected void doEndObserve(int index) {
        //Utils.waitSafely(MEMORY_MONITOR_TIME_OUT);
        mRuntime.gc();
        mEndUsedHeap = mRuntime.totalMemory() - mRuntime.freeMemory();
        LogHelper.d(TAG, "end used heap size(KB): " + mEndUsedHeap / BYTES_IN_KILOBYTE
                + ", diff size(KB): " + (mEndUsedHeap - mBegainUsedHeap) / BYTES_IN_KILOBYTE);
    }
}
