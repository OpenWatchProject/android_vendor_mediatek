package com.mediatek.mms.ext;

import android.app.Activity;
import android.view.MotionEvent;

import com.mediatek.mms.callback.ITextSizeAdjustHost;

public interface IOpSlideEditorActivityExt {
    /**
     * @internal
     */
    void onStart(ITextSizeAdjustHost host, Activity activity);
    /**
     * @internal
     */
    boolean dispatchTouchEvent(MotionEvent ev);
}
