package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.view.MotionEvent;
import android.widget.TextView;

import com.google.android.mms.pdu.PduBody;

import com.mediatek.mms.callback.IMmsPlayerActivityCallback;
import com.mediatek.mms.callback.ITextSizeAdjustHost;

public interface IOpMmsPlayerActivityExt {
    /**
     * @internal
     */
    boolean haveSomethingToCopyToSDCard(Context context, PduBody body,
            IMmsPlayerActivityCallback callback);
    /**
     * @internal
     */
    void initListAdapter(TextView text, int size);
    /**
     * @internal
     */
    void onOptionsItemSelected(Intent intent);
    /**
     * @internal
     */
    void onStart(ITextSizeAdjustHost host, Activity activity);
    /**
     * @internal
     */
    boolean dispatchTouchEvent(MotionEvent ev);
}
