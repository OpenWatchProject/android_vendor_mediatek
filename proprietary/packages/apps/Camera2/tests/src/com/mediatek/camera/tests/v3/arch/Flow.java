package com.mediatek.camera.tests.v3.arch;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;

class Flow {
    private static LogUtil.Tag TAG = Utils.getTestTag(Flow.class.getSimpleName());
    private String mManualTcId = "null";
    private String mAutoTcId = "null";
    private List<MetaCase.Event> mEventList = new ArrayList<>();
    private List<Integer> mBeginRepeatIndexList = new ArrayList<>();
    private List<Integer> mEndRepeatIndexList = new ArrayList<>();
    private List<Integer> mRepeatTimesList = new ArrayList<>();

    public Flow(String manualTcId, String autoTcId) {
        mManualTcId = manualTcId;
        mAutoTcId = autoTcId;
    }

    public void run() {
        print();
        int eventCount = mEventList.size();
        for (int eventIndex = 0, repeatIndex = 0; eventIndex < eventCount; ) {
            if (repeatIndex < mBeginRepeatIndexList.size() &&
                    eventIndex == mBeginRepeatIndexList.get(repeatIndex)) {
                List<MetaCase.Event> subEventList =
                        mEventList.subList(eventIndex, mEndRepeatIndexList.get(repeatIndex));
                boolean success = repeatRun(subEventList, mRepeatTimesList.get(repeatIndex));
                if (!success) {
                    LogHelper.d(TAG, "[run] repeatRun return false, break current flow");
                }
                repeatIndex++;
                eventIndex += subEventList.size();
            } else {
                if (mEventList.get(eventIndex).doEvent() == false) {
                    LogHelper.d(TAG, "[run] doEvent return false, break current flow");
                    break;
                }
                eventIndex++;
            }
        }
    }

    private boolean repeatRun(List<MetaCase.Event> subEventList, int times) {
        for (int i = 0; i < times; i++) {
            LogHelper.d(TAG, "[repeatRun] repeat times = " + i);
            for (MetaCase.Event event : subEventList) {
                if (event.doEvent() == false) {
                    LogHelper.d(TAG, "[repeatRun] doEvent return false, break current flow");
                    return false;
                }
            }
        }
        return true;
    }

    public void appendEvent(MetaCase.Event event) {
        if (mEventList.size() != 0) {
            event.setLastEvent(mEventList.get(mEventList.size() - 1));
        }
        mEventList.add(event);
    }

    public void appendFlow(Flow flow) {
        if (mEventList.size() != 0) {
            flow.mEventList.get(0).setLastEvent(mEventList.get(mEventList.size() - 1));
        }
        mEventList.addAll(flow.mEventList);
    }

    public void setBeginRepeat(int times) {
        mBeginRepeatIndexList.add(mEventList.size());
        mRepeatTimesList.add(times);
    }

    public void setEndRepeat() {
        mEndRepeatIndexList.add(mEventList.size());
    }

    public float getCheckCoverage() {
        float checkCoverageSum = 0.0f;
        int checkCount = 0;
        for (MetaCase.Event event : mEventList) {
            switch (event.mType) {
                case CHECK:
                    MetaCase.CheckEvent checkEvent = ((MetaCase.CheckEvent) event);
                    checkCoverageSum += checkEvent.mChecker.getCheckCoverage();
                    checkCount++;
                    break;
                case OBSERVE_END:
                    MetaCase.ObserveEndEvent observeEvent = ((MetaCase.ObserveEndEvent) event);
                    checkCoverageSum += observeEvent.mObserver.getObserveCoverage();
                    checkCount++;
                    break;
                default:
                    break;
            }
        }
        if (checkCount == 0) {
            return 1.f;
        } else {
            return checkCoverageSum / (float) checkCount;
        }
    }

    public void print() {
        LogHelper.d(TAG, "[print] ----------------------------------------------------");
        LogHelper.d(TAG, "[print] [Manual test case] = " + mManualTcId);
        LogHelper.d(TAG, "[print] [Auto test case] = " + mAutoTcId);
        int eventIndex = 0;
        for (; eventIndex < mEventList.size(); eventIndex++) {
            LogHelper.d(TAG, "[print] [Step " + eventIndex + "] " + mEventList.get(eventIndex)
                    .getDescription());
        }
        for (int repeatIndex = 0; repeatIndex < mBeginRepeatIndexList.size(); repeatIndex++,
                eventIndex++) {
            LogHelper.d(TAG, "[print] [Step " + eventIndex + "] Repeat step " +
                    mBeginRepeatIndexList.get(repeatIndex) + " to step " +
                    (mEndRepeatIndexList.get(repeatIndex) - 1) + " for " +
                    mRepeatTimesList.get(repeatIndex) + " times");
        }
        LogHelper.d(TAG, "[print] [Check coverage] " + (int) (getCheckCoverage() * 100) + "%");
        LogHelper.d(TAG, "[print] ----------------------------------------------------");
    }

    @Override
    protected Flow clone() {
        Flow cloneFlow = new Flow(mManualTcId, mAutoTcId);
        for (MetaCase.Event event : mEventList) {
            cloneFlow.mEventList.add(event);
        }
        return cloneFlow;
    }

    @Override
    public boolean equals(Object anotherFlow) {
        if (this == anotherFlow) {
            return true;
        }
        if (anotherFlow == null || !(anotherFlow instanceof Flow)) {
            return false;
        }

        Flow flow = (Flow) anotherFlow;

        if (mEventList.size() != flow.mEventList.size()) {
            return false;
        }
        for (int i = 0; i < mEventList.size(); i++) {
            if ((mEventList.get(i).mType != flow.mEventList.get(i).mType)
                    || (mEventList.get(i).mDoIndex != flow.mEventList.get(i).mDoIndex)
                    || (!mEventList.get(i).getDescription().equals(
                    flow.mEventList.get(i).getDescription()))) {
                return false;
            }
        }
        return true;
    }
}
