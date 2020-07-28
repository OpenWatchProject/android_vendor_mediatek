package com.mediatek.camera.common.widget;

import android.content.Context;
import android.util.AttributeSet;

public class TwoStateScaleAnimationButton extends ScaleAnimationButton {
    private static final float DISABLED_ALPHA = 0.4f;

    public TwoStateScaleAnimationButton(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void setEnabled(boolean enabled) {
        super.setEnabled(enabled);
        if (enabled) {
            setAlpha(1.0f);
        } else {
            setAlpha(DISABLED_ALPHA);
        }
    }
}
