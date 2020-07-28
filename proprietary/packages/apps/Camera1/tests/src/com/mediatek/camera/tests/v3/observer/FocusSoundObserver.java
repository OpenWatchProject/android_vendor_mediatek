package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observer used to check focus sound is right.
 */
public class FocusSoundObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(FocusSoundObserver.class
            .getSimpleName());
    private static final String LOG_TAG_FOCUS_SOUND = "CamAp_SoundPlaybackImpl";
    private int mExpectedIndex = 0;

    public static final int INDEX_NO_FOCUS_SOUND = 0;
    public static final int INDEX_HAS_FOCUS_SOUND = 1;

    @Override
    protected String[] getObservedTagList(int index) {
        return new String[]{LOG_TAG_FOCUS_SOUND};
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return new String[]{"play sound with action 0"};
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        if (index == INDEX_NO_FOCUS_SOUND) {
            return mExpectedIndex == 0;
        } else {
            return mExpectedIndex == getObservedTagList(index).length;
        }
    }

    @Override
    public int getObserveCount() {
        return 1;
    }


    @Override
    protected void onLogComing(int index, String line) {
        if (mExpectedIndex >= getObservedTagList(index).length) {
            return;
        }
        if (line.contains(getObservedTagList(index)[mExpectedIndex])
                && line.contains(getObservedKeyList(index)[mExpectedIndex])) {
            mExpectedIndex++;
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        if (index == INDEX_NO_FOCUS_SOUND) {
            Utils.assertRightNow(mExpectedIndex == 0, "mExpectedIndex " + mExpectedIndex);
        } else {
            int length = getObservedTagList(index).length;
            Utils.assertRightNow(mExpectedIndex == length, "mExpectedIndex " + mExpectedIndex +
                    ",length = " + length);
        }
    }

    @Override
    protected void onObserveBegin(int index) {
        mExpectedIndex = 0;
    }

    @Override
    public String getDescription(int index) {
        StringBuilder sb = new StringBuilder("Observe this group tag has printed out as order, ");
        for (int i = 0; i < getObservedTagList(index).length; i++) {
            sb.append("[TAG-" + i + "] ");
            sb.append(getObservedTagList(index)[i]);
            sb.append(",[KEY-" + i + "] ");
            sb.append(getObservedKeyList(index)[i]);
            sb.append("\n");
        }
        return sb.toString();
    }

}
