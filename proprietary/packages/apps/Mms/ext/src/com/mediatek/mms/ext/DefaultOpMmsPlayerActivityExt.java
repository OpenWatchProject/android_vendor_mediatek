package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.view.MotionEvent;
import android.widget.TextView;

import com.google.android.mms.pdu.PduBody;

import com.mediatek.mms.callback.IMmsPlayerActivityCallback;
import com.mediatek.mms.callback.ITextSizeAdjustHost;

public class DefaultOpMmsPlayerActivityExt
        extends ContextWrapper implements IOpMmsPlayerActivityExt {

    public DefaultOpMmsPlayerActivityExt(Context base) {
        super(base);
    }

    @Override
    public boolean haveSomethingToCopyToSDCard(Context context,
            PduBody body, IMmsPlayerActivityCallback callback) {
        return false;
    }

    @Override
    public void initListAdapter(TextView text, int size) {

    }

    @Override
    public void onOptionsItemSelected(Intent intent) {

    }

    @Override
    public void onStart(ITextSizeAdjustHost host, Activity activity) {

    }

    public boolean dispatchTouchEvent(MotionEvent ev) {
        return false;
    }
}
