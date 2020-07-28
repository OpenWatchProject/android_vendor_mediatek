package com.mediatek.camera.feature.setting.whitebalance;

import android.app.Activity;
import android.content.res.TypedArray;
import android.view.View;
import android.widget.ImageView;

import com.mediatek.camera.R;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

/**
 * White balance indicator view.
 */

public class WhiteBalanceIndicatorView {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(WhiteBalanceIndicatorView.class.getSimpleName());
    private static final int VIEW_PRIORITY = 8;
    private String[] mOriginalEntryValues;
    private int[] mOriginalIndicator;
    private Activity mActivity;
    private ImageView mIndicatorView;

    /**
     * White balance indicator view constructor.
     *
     * @param activity The camera activity.
     */
    public WhiteBalanceIndicatorView(Activity activity) {
        mActivity = activity;
        mIndicatorView = (ImageView) activity.getLayoutInflater().inflate(
                R.layout.white_balance_indicator, null);

        mOriginalEntryValues = activity.getResources()
                .getStringArray(R.array.white_balance_entryvalues);

        TypedArray array = activity.getResources()
                .obtainTypedArray(R.array.white_balance_indicators);
        int n = array.length();
        mOriginalIndicator = new int[n];
        for (int i = 0; i < n; ++i) {
            mOriginalIndicator[i] = array.getResourceId(i, 0);
        }
        array.recycle();
    }

    /**
     * Get indicator view.
     *
     * @return The indicator view.
     */
    public ImageView getView() {
        return mIndicatorView;
    }

    /**
     * Get the indicator view priority in showing.
     *
     * @return The indicator view priority.
     */
    public int getViewPriority() {
        return VIEW_PRIORITY;
    }

    /**
     * Update indicator icon.
     *
     * @param mode Current white balance value.
     */
    public void updateIndicator(final String mode) {
        LogHelper.d(TAG, "[updateIndicator], scene:" + mode);

        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                int index = -1;
                for (int i = 0; i < mOriginalEntryValues.length; i++) {
                    if (mOriginalEntryValues[i].equals(mode)) {
                        index = i;
                        break;
                    }
                }
                LogHelper.d(TAG, "[updateIndicator], index:" + index);
                if (index == 0) {
                    // index ==0 means scene is auto, don't show indicator
                    mIndicatorView.setVisibility(View.GONE);
                } else {
                    mIndicatorView.setImageDrawable(mActivity
                            .getResources().getDrawable(mOriginalIndicator[index]));
                    mIndicatorView.setVisibility(View.VISIBLE);
                }
            }
        });
    }
}
