package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.view.MotionEvent;

import com.mediatek.mms.callback.ITextSizeAdjustHost;

public class DefaultOpSlideEditorActivityExt extends ContextWrapper implements
        IOpSlideEditorActivityExt {

    public DefaultOpSlideEditorActivityExt(Context base) {
        super(base);
    }

    @Override
    public void onStart(ITextSizeAdjustHost host, Activity activity) {

    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        return false;
    }

}
