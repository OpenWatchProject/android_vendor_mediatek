package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

public abstract class LogKeyValueObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(LogKeyValueObserver.class
            .getSimpleName());

    public static enum ValueType {
        INTEGER,
        LONG,
        FLOAT
    }

    @Override
    protected String[] getObservedTagList(int index) {
        return new String[]{getObservedTag(index)};
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return new String[]{getObservedKey(index)};
    }

    @Override
    protected void onLogComing(int index, String line) {
        int startIndex = line.indexOf(getObservedKey(index)) + getObservedKey(index).length();
        int endIndex = startIndex + getMaxValueStringLength(index);
        String valueString = line.substring(startIndex, endIndex);
        Object value = null;
        boolean exceptionPopUp = true;
        while (exceptionPopUp) {
            try {
                switch (getValueType(index)) {
                    case INTEGER:
                        value = Integer.valueOf(valueString);
                        break;
                    case LONG:
                        value = Long.valueOf(valueString);
                        break;
                    case FLOAT:
                        value = Float.valueOf(valueString);
                        break;
                    default:
                        break;
                }
                exceptionPopUp = false;
            } catch (NumberFormatException e) {
                exceptionPopUp = true;
                if (valueString.length() == getMinValueStringLength(index)) {
                    LogHelper.d(TAG, "[onLogComing] transfer format fail, fail value string = " +
                            valueString);
                    Utils.assertRightNow(false);
                    break;
                } else {
                    valueString = valueString.substring(0, valueString.length() - 1);
                }
            }
        }

        LogHelper.d(TAG, "[onLogComing] value = " + value);
        onValueComing(index, value);
    }

    protected abstract void onValueComing(int index, Object value);

    protected abstract String getObservedTag(int index);

    protected abstract String getObservedKey(int index);

    protected abstract ValueType getValueType(int index);

    protected abstract int getMinValueStringLength(int index);

    protected abstract int getMaxValueStringLength(int index);
}
