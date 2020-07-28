package com.mediatek.camera.tests.v3.arch;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.annotation.CoverPoint;
import com.mediatek.camera.tests.v3.annotation.NotCoverPoint;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.lang.annotation.Annotation;

public abstract class Observer {

    public final void beginObserve(int index) {
        LogHelper.d(getLogTag(), "[beginObserve] " + getDescription(index));
        long timeBegin = System.currentTimeMillis();
        doBeginObserve(index);
        long timeEnd = System.currentTimeMillis();
        LogHelper.d(getLogTag(), "[beginObserve] cost " + (timeEnd - timeBegin)
                + " ms");
        Utils.writeComponentCostTime(getClass().getName() + "#beginObserve", timeEnd - timeBegin);
    }

    public final void endObserve(int index) {
        LogHelper.d(getLogTag(), "[endObserve] " + getDescription(index));
        long timeBegin = System.currentTimeMillis();
        doEndObserve(index);
        long timeEnd = System.currentTimeMillis();
        LogHelper.d(getLogTag(), "[endObserve] cost " + (timeEnd - timeBegin)
                + " ms");
        Utils.writeComponentCostTime(getClass().getName() + "#endObserve", timeEnd - timeBegin);
    }

    public float getObserveCoverage() {
        Annotation[] annotations = this.getClass().getAnnotations();
        int coverPointNum = 0;
        int notCoverPointNum = 0;
        for (Annotation a : annotations) {
            if (a instanceof CoverPoint) {
                coverPointNum += ((CoverPoint) a).pointList().length;
            } else if (a instanceof NotCoverPoint) {
                notCoverPointNum += ((NotCoverPoint) a).pointList().length;
            }
        }
        float coverage;
        if (coverPointNum == 0 && notCoverPointNum == 0) {
            coverage = 1.f;
        } else if (coverPointNum == 0 && notCoverPointNum != 0) {
            coverage = 0.f;
        } else if (coverPointNum != 0 && notCoverPointNum == 0) {
            coverage = 1.f;
        } else {
            coverage = (float) coverPointNum / (float) (coverPointNum + notCoverPointNum);
        }
        return coverage;
    }

    public boolean isSupported(int index) {
        return true;
    }

    public final boolean isSupported() {
        int count = getObserveCount();
        for (int i = 0; i < count; i++) {
            if (isSupported(i)) {
                return true;
            }
        }
        return false;
    }

    private final LogUtil.Tag getLogTag() {
        return Utils.getTestTag(getClass().getSimpleName());
    }

    public abstract int getObserveCount();

    public abstract String getDescription(int index);

    protected abstract void doBeginObserve(int index);

    protected abstract void doEndObserve(int index);
}
