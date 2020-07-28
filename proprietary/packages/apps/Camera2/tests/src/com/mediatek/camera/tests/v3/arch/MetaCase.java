package com.mediatek.camera.tests.v3.arch;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

public class MetaCase {
    public MetaCase(String manualTcId) {
        this();
        if (manualTcId == null || manualTcId.equals("")) {
            mManualTcId = "null";
        } else {
            mManualTcId = manualTcId;
        }
    }

    public MetaCase() {
        initAutoTcId();
    }

    enum EventType {
        OBSERVE_BEGIN,
        OBSERVE_END,
        OPERATE,
        CHECK
    }

    enum Across {
        BEGIN,
        END
    }

    enum RepeatType {
        BEGIN,
        END
    }

    static class Repeat {
        RepeatType mRepeatType;
        int mRepeatTimes;
    }

    static abstract class Event {
        public static final int INVALID_INDEX = -1;
        EventType mType;
        int mDoIndex = INVALID_INDEX;
        int[] mDoIndexArray;
        Event mLastEvent;

        public abstract boolean doEvent();

        public abstract String getDescription();

        public abstract Page getCheckedPageAfterEvent();

        public Page getCheckedPageAfterLastEvent() {
            if (mLastEvent != null) {
                return mLastEvent.getCheckedPageAfterEvent();
            } else {
                return null;
            }
        }

        public void setLastEvent(Event lastEvent) {
            mLastEvent = lastEvent;
        }
    }

    static class OperateEvent extends Event {
        Operator mOperator;

        public OperateEvent(Operator operator, int doIndex) {
            mType = EventType.OPERATE;
            mDoIndex = doIndex;
            mOperator = operator;
        }

        @Override
        public boolean doEvent() {
            if (!mOperator.isSupported(mDoIndex)) {
                LogHelper.d(TAG, "[doEvent] operator = " + mOperator.getClass().getSimpleName()
                        + ", index = " + mDoIndex + ", not supported, return false");
                return false;
            }
            Page beforePage = mOperator.getPageBeforeOperate(mDoIndex);
            if (beforePage != null && beforePage == getCheckedPageAfterLastEvent()) {
                mOperator.ignoreBeforePageCheckByLogicThisTime();
            }
            mOperator.operate(mDoIndex);
            return true;
        }

        @Override
        public String getDescription() {
            return mOperator.getDescription(mDoIndex);
        }

        @Override
        public Page getCheckedPageAfterEvent() {
            return mOperator.getPageAfterOperate(mDoIndex);
        }
    }

    static class CheckEvent extends Event {
        Checker mChecker;

        public CheckEvent(Checker checker, int doIndex) {
            mType = EventType.CHECK;
            mDoIndex = doIndex;
            mChecker = checker;
        }

        @Override
        public boolean doEvent() {
            if (!mChecker.isSupported(mDoIndex)) {
                LogHelper.d(TAG, "[doEvent] checker = " + mChecker.getClass().getSimpleName()
                        + ", index = " + mDoIndex + ", not supported, return false");
                return false;
            }
            Page beforePage = mChecker.getPageBeforeCheck(mDoIndex);
            if (beforePage != null && beforePage == getCheckedPageAfterLastEvent()) {
                mChecker.ignoreBeforePageCheckByLogicThisTime();
            }
            mChecker.check(mDoIndex);
            return true;
        }

        @Override
        public String getDescription() {
            return mChecker.getDescription(mDoIndex)
                    + " <coverage = " + mChecker.getCheckCoverage() * 100 + "%>";
        }

        @Override
        public Page getCheckedPageAfterEvent() {
            return null;
        }
    }

    static class ObserveBeginEvent extends Event {
        Observer mObserver;

        public ObserveBeginEvent(Observer observer, int doIndex) {
            mType = EventType.OBSERVE_BEGIN;
            mDoIndex = doIndex;
            mObserver = observer;
        }

        @Override
        public boolean doEvent() {
            if (!mObserver.isSupported(mDoIndex)) {
                LogHelper.d(TAG, "[doEvent] observer = " + mObserver.getClass().getSimpleName()
                        + ", index = " + mDoIndex + ", not supported, return false");
                return false;
            }
            mObserver.beginObserve(mDoIndex);
            return true;
        }

        @Override
        public String getDescription() {
            return "Begin observe [" + mObserver.getDescription(mDoIndex) + "]"
                    + " <coverage = " + mObserver.getObserveCoverage() * 100 + "%>";
        }

        @Override
        public Page getCheckedPageAfterEvent() {
            return null;
        }
    }

    static class ObserveEndEvent extends Event {
        Observer mObserver;

        public ObserveEndEvent(Observer observer, int doIndex) {
            mType = EventType.OBSERVE_END;
            mDoIndex = doIndex;
            mObserver = observer;
        }

        @Override
        public boolean doEvent() {
            if (!mObserver.isSupported(mDoIndex)) {
                LogHelper.d(TAG, "[doEvent] observer = " + mObserver.getClass().getSimpleName()
                        + ", index = " + mDoIndex + ", not supported, return false");
                return false;
            }
            mObserver.endObserve(mDoIndex);
            return true;
        }

        @Override
        public String getDescription() {
            return "End observe [" + mObserver.getDescription(mDoIndex) + "]"
                    + " <coverage = " + mObserver.getObserveCoverage() * 100 + "%>";
        }

        @Override
        public Page getCheckedPageAfterEvent() {
            return null;
        }
    }

    private static final LogUtil.Tag TAG = Utils.getTestTag(MetaCase.class.getSimpleName());

    private String mManualTcId = "null";
    private String mAutoTcId = "null";

    // event added by MetaCase.addXXX, observeXXX, acrossXXX
    private List<Object> mUserAddedEventList = new ArrayList<>();
    // info used when generate flows
    private int mIndex;
    private Stack<ObserveBeginEvent> mObserverStack = new Stack<>();
    // flow generated from mUserAddedEventList
    private List<Flow> mFlowList = new ArrayList<>();
    private boolean mIsComponentGetCountAvailable = true;

    public MetaCase acrossBegin() {
        mUserAddedEventList.add(Across.BEGIN);
        return this;
    }

    public MetaCase acrossEnd() {
        mUserAddedEventList.add(Across.END);
        return this;
    }

    public MetaCase repeatBegin(int repeatTimes) {
        Repeat repeat = new Repeat();
        repeat.mRepeatType = RepeatType.BEGIN;
        repeat.mRepeatTimes = repeatTimes;
        mUserAddedEventList.add(repeat);
        return this;
    }

    public MetaCase repeatEnd() {
        Repeat repeat = new Repeat();
        repeat.mRepeatType = RepeatType.END;
        mUserAddedEventList.add(repeat);
        return this;
    }

    public MetaCase observeBegin(Observer observer) {
        return observeBegin(observer, null);
    }

    public MetaCase observeBegin(Observer observer, int onlyObserveIndex) {
        return observeBegin(observer, new int[]{onlyObserveIndex});
    }

    public MetaCase observeBegin(Observer observer, int[] observeIndexArray) {
        ObserveBeginEvent event = new ObserveBeginEvent(observer, Event.INVALID_INDEX);
        event.mDoIndexArray = observeIndexArray;
        mUserAddedEventList.add(event);
        return this;
    }

    public MetaCase observeEnd() {
        mUserAddedEventList.add(new ObserveEndEvent(null, Event.INVALID_INDEX));
        return this;
    }

    public MetaCase addOperator(Operator operator) {
        return addOperator(operator, null);
    }

    public MetaCase addOperator(Operator operator, int onlyOperateIndex) {
        return addOperator(operator, new int[]{onlyOperateIndex});
    }

    public MetaCase addOperator(Operator operator, int[] operateIndexArray) {
        OperateEvent event = new OperateEvent(operator, Event.INVALID_INDEX);
        event.mDoIndexArray = operateIndexArray;
        mUserAddedEventList.add(event);
        return this;
    }


    public MetaCase addChecker(Checker checker) {
        return addChecker(checker, null);
    }

    public MetaCase addChecker(Checker checker, int onlyCheckIndex) {
        return addChecker(checker, new int[]{onlyCheckIndex});
    }

    public MetaCase addChecker(Checker checker, int[] checkIndexArray) {
        CheckEvent event = new CheckEvent(checker, Event.INVALID_INDEX);
        event.mDoIndexArray = checkIndexArray;
        mUserAddedEventList.add(event);
        return this;
    }

    public void run() {
        runForTimes(1);
    }

    public void runForTimes(int times) {
        runForTimesWithTimeInterval(times, 0);
    }

    public void runForTimesWithTimeInterval(int times, int randomRangeInSecond) {
        generateFlows();
        if (mFlowList.size() == 0) {
            LogHelper.d(TAG, "[runForTimesWithTimeInterval] no flows, return");
            return;
        }
        for (int i = 0; i < times; i++) {
            LogHelper.d(TAG, "[runForTimesWithTimeInterval] times = " + i);
            int j = 0;
            for (Flow flow : mFlowList) {
                LogHelper.d(TAG, "[runForTimesWithTimeInterval] flow id = " + (j++));
                flow.run();
            }
            if (randomRangeInSecond > 0) {
                int waitMilliSeconds = (int) (Math.random() * randomRangeInSecond * 1000);
                LogHelper.d(TAG, "[runForTimesWithTimeInterval] wait " + waitMilliSeconds +
                        " ms");
                Utils.waitSafely(waitMilliSeconds);
            }
        }
    }

    /**
     * Repeat run this meta case for duration
     *
     * @param duration The seconds
     */
    public void runForDuration(long duration) {
        runForDurationWithTimeInterval(duration, 0);
    }

    public void runForDurationWithTimeInterval(long duration, int randomRangeInSecond) {
        generateFlows();
        if (mFlowList.size() == 0) {
            LogHelper.d(TAG, "[runForDurationWithTimeInterval] no flows, return");
            return;
        }
        long endTime = System.currentTimeMillis() + duration * 1000;
        int times = 0;
        while (System.currentTimeMillis() <= endTime) {
            LogHelper.d(TAG, "[runForDurationWithTimeInterval] times = " + (times++));
            int j = 0;
            for (Flow flow : mFlowList) {
                LogHelper.d(TAG, "[runForDurationWithTimeInterval] flow id = " + (j++));
                flow.run();
            }
            if (randomRangeInSecond > 0) {
                int waitMilliSeconds = (int) (Math.random() * randomRangeInSecond * 1000);
                LogHelper.d(TAG, "[runForDurationWithTimeInterval] wait " + waitMilliSeconds +
                        " ms");
                Utils.waitSafely(waitMilliSeconds);
            }
        }
    }

    private void initAutoTcId() {
        StackTraceElement[] traces = new Throwable().getStackTrace();
        int lastMetaCaseTraceIndex = -1;
        for (int i = 0; i < traces.length; i++) {
            if (traces[i].getClassName().equals(MetaCase.class.getName())) {
                lastMetaCaseTraceIndex = i;
            }
        }
        StackTraceElement trace = traces[lastMetaCaseTraceIndex + 1];
        String className = trace.getClassName();
        className = className.substring(className.lastIndexOf('.') + 1);
        mAutoTcId = className + "." + trace.getMethodName();
    }

    private void generateFlows() {
        try {
            mFlowList = generateFlowsRecursive(mUserAddedEventList, 0);
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "[generateFlows] " + e.getMessage());
            return;
        }
        removeSameFlowUntilNone(mFlowList);
        if (mIsComponentGetCountAvailable == false) {
            LogHelper.d(TAG, "[generateFlows] mIsComponentGetCountAvailable false, clear flows");
            mFlowList.clear();
            mIsComponentGetCountAvailable = true;
        }
        LogHelper.d(TAG, "[generateFlows] total flow count = " + mFlowList.size());
    }

    private List<Flow> generateFlowsRecursive(List<Object> originEvent, int beginIndex)
            throws IllegalArgumentException {
        //LogHelper.d(TAG, "[generateFlowsRecursive] beginIndex = " + beginIndex);
        List<Flow> flows = new ArrayList<Flow>();
        for (int i = beginIndex; i < originEvent.size(); i++) {
            Object object = originEvent.get(i);
            //LogHelper.d(TAG, "[generateFlowsRecursive] index = " + i + ", object = " + object);
            if (object instanceof Across && object == Across.BEGIN) {
                acrossFlowsToFlows(flows, generateFlowsRecursive(originEvent, i + 1));
                i = mIndex;
                printFlows(flows);
            } else if (object instanceof Across && object == Across.END) {
                //LogHelper.d(TAG, "[generateFlowsRecursive] Across.END, return");
                mIndex = i;
                printFlows(flows);
                return flows;
            } else if (object instanceof Repeat
                    && ((Repeat) object).mRepeatType == RepeatType.BEGIN) {
                setRepeatBeginToFlows(flows, ((Repeat) object).mRepeatTimes);
                printFlows(flows);
            } else if (object instanceof Repeat
                    && ((Repeat) object).mRepeatType == RepeatType.END) {
                setRepeatEndToFlows(flows);
                printFlows(flows);
            } else if (object instanceof ObserveBeginEvent) {
                mObserverStack.push((ObserveBeginEvent) object);
                List<Event> events = new ArrayList<>();
                if (((ObserveBeginEvent) object).mDoIndexArray == null) {
                    int count = ((ObserveBeginEvent) object).mObserver.getObserveCount();
                    if (count == 0) {
                        throw new IllegalArgumentException("getObserveCount return 0 on "
                                + ((ObserveBeginEvent) object).mObserver);
                    }
                    mIsComponentGetCountAvailable = mIsComponentGetCountAvailable && count > 0;
                    for (int j = 0; j < count; j++) {
                        events.add(
                                new ObserveBeginEvent(((ObserveBeginEvent) object).mObserver, j));
                    }
                } else {
                    int count = ((ObserveBeginEvent) object).mDoIndexArray.length;
                    for (int j = 0; j < count; j++) {
                        events.add(new ObserveBeginEvent(((ObserveBeginEvent) object).mObserver, (
                                (ObserveBeginEvent) object).mDoIndexArray[j]));
                    }
                }

                appendEventsToFlows(flows, events);
                printFlows(flows);
            } else if (object instanceof ObserveEndEvent) {
                List<Event> events = new ArrayList<>();
                if (mObserverStack.peek().mDoIndexArray == null) {
                    int count = mObserverStack.peek().mObserver.getObserveCount();
                    if (count == 0) {
                        throw new IllegalArgumentException("getObserveCount return 0 on "
                                + ((ObserveBeginEvent) object).mObserver);
                    }
                    mIsComponentGetCountAvailable = mIsComponentGetCountAvailable && count > 0;
                    for (int j = 0; j < count; j++) {
                        events.add(new ObserveEndEvent(mObserverStack.peek().mObserver, j));
                    }
                } else {
                    int count = mObserverStack.peek().mDoIndexArray.length;
                    for (int j = 0; j < count; j++) {
                        events.add(new ObserveEndEvent(mObserverStack.peek().mObserver,
                                mObserverStack.peek().mDoIndexArray[j]));
                    }
                }

                mObserverStack.pop();
                appendEventsToFlows(flows, events);
                printFlows(flows);
            } else if (object instanceof OperateEvent) {
                List<Event> events = new ArrayList<>();
                if (((OperateEvent) object).mDoIndexArray == null) {
                    int count = ((OperateEvent) object).mOperator.getOperatorCount();
                    if (count == 0) {
                        throw new IllegalArgumentException("getOperatorCount return 0 on "
                                + ((OperateEvent) object).mOperator);
                    }
                    mIsComponentGetCountAvailable = mIsComponentGetCountAvailable && count > 0;
                    for (int j = 0; j < count; j++) {
                        events.add(new OperateEvent(((OperateEvent) object).mOperator, j));
                    }
                } else {
                    int count = ((OperateEvent) object).mDoIndexArray.length;
                    for (int j = 0; j < count; j++) {
                        events.add(new OperateEvent(((OperateEvent) object).mOperator,
                                ((OperateEvent) object).mDoIndexArray[j]));
                    }

                }

                appendEventsToFlows(flows, events);
                printFlows(flows);
            } else if (object instanceof CheckEvent) {
                List<Event> events = new ArrayList<>();
                if (((CheckEvent) object).mDoIndexArray == null) {
                    int count = ((CheckEvent) object).mChecker.getCheckCount();
                    if (count == 0) {
                        throw new IllegalArgumentException("getCheckCount return 0 on "
                                + ((CheckEvent) object).mChecker);
                    }
                    mIsComponentGetCountAvailable = mIsComponentGetCountAvailable && count > 0;
                    for (int j = 0; j < count; j++) {
                        events.add(new CheckEvent(((CheckEvent) object).mChecker, j));
                    }
                } else {
                    int count = ((CheckEvent) object).mDoIndexArray.length;
                    for (int j = 0; j < count; j++) {
                        events.add(new CheckEvent(((CheckEvent) object).mChecker,
                                ((CheckEvent) object).mDoIndexArray[j]));
                    }
                }

                appendEventsToFlows(flows, events);
                printFlows(flows);
            }
        }
        //LogHelper.d(TAG, "[generateFlowsRecursive] return");
        printFlows(flows);
        return flows;
    }

    private void removeSameFlow(List<Flow> flows) {
        //LogHelper.d(TAG, "[removeSameFlow] flow size = " + flows.size());
        List<Object> remove = new ArrayList<>();
        for (int i = 0; i < flows.size(); i++) {
            for (int j = flows.size() - 1; j >= 0; j--) {
                if (i < j && flows.get(i).equals(flows.get(j))) {
                    //LogHelper.d(TAG, "[removeSameFlow] isSameFlow i = " + i + ", j = " + j);
                    if (!remove.contains(flows.get(i))) {
                        remove.add(flows.get(i));
                    }
                }
            }
        }
        //LogHelper.d(TAG, "[removeSameFlow] before remove flow size = " + flows.size());
        //LogHelper.d(TAG, "[removeSameFlow] remove size = " + remove.size());
        for (Object o : remove) {
            if (flows.contains(o)) {
                flows.remove(o);
            }
        }
        //LogHelper.d(TAG, "[removeSameFlow] after remove flow size = " + flows.size());
    }

    private void removeSameFlowUntilNone(List<Flow> flows) {
        int originSize;
        do {
            originSize = flows.size();
            removeSameFlow(flows);

        } while (flows.size() != originSize);
    }

    // Before:      After:
    // A1           A1 C1
    // A2           A2 C2
    // A3           A3 C1
    private void appendEventsToFlows(List<Flow> flows, List<Event> events) {
        if (flows.size() == 0) {
            for (int index = 0; index < events.size(); index++) {
                flows.add(new Flow(mManualTcId, mAutoTcId));
            }
        }

        for (int flowIndex = 0, eventIndex = 0; flowIndex < flows.size(); flowIndex++) {
            flows.get(flowIndex).appendEvent(events.get(eventIndex));
            eventIndex++;
            eventIndex = eventIndex % events.size();
        }
    }

    // Before:      After:
    // F1           F1 f1
    // F2           F2 f1
    // F3           F3 f1
    //              F1 f2
    //              F2 f2
    //              F3 f2
    //              F1 f3
    //              F2 f3
    //              F3 f1
    //              F3 f2
    //              F3 f3
    private void acrossFlowsToFlows(List<Flow> targetFlow, List<Flow>
            addFlows) {
        if (targetFlow.size() == 0) {
            for (int i = 0; i < addFlows.size(); i++) {
                targetFlow.add(new Flow(mManualTcId, mAutoTcId));
            }
            for (int i = 0; i < addFlows.size(); i++) {
                targetFlow.get(i).appendFlow(addFlows.get(i).clone());
            }
            return;
        } else {
            int targetFlowOriginCount = targetFlow.size();
            repeatFlows(targetFlow, addFlows.size());
            for (int targetFlowIndex = 0, addFlowIndex = 0; targetFlowIndex < targetFlow.size(); ) {
                for (int i = 0; i < targetFlowOriginCount; i++, targetFlowIndex++) {
                    targetFlow.get(targetFlowIndex).appendFlow(addFlows.get(addFlowIndex));
                }
                addFlowIndex++;
            }
        }
    }

    // Before:      After:
    // A1           A1
    // A2           A2
    // A3           A3
    //              A1
    //              A2
    //              A3
    //              A3
    // times = 1, means not do repeat
    // result size = origin size * times
    private void repeatFlows(List<Flow> flows, int times) {
        List<Flow> temp = cloneList(flows);
        flows.clear();
        for (int i = 0; i < temp.size() * times; ) {
            for (int j = 0; j < temp.size(); i++, j++) {
                flows.add(temp.get(j).clone());
            }
        }
    }

    private void setRepeatBeginToFlows(List<Flow> flows, int repeatTimes) {
        if (flows.size() == 0) {
            flows.add(new Flow(mManualTcId, mAutoTcId));
        }
        for (Flow flow : flows) {
            flow.setBeginRepeat(repeatTimes);
        }
    }

    private void setRepeatEndToFlows(List<Flow> flows) {
        for (Flow flow : flows) {
            flow.setEndRepeat();
        }
    }

    private <T> List<T> cloneList(List<T> list) {
        List<T> result = new ArrayList<T>(list.size());
        for (T element : list) {
            result.add(element);
        }
        return result;
    }

    private void printFlows(List<Flow> flows) {
//        for (int flowIndex = 0; flowIndex < flows.size(); flowIndex++) {
//            LogHelper.d(TAG, "[printFlows] flow [" + flowIndex + "] ");
//            flows.get(flowIndex).print();
//        }
    }
}