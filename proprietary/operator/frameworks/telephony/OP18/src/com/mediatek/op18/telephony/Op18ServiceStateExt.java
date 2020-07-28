package com.mediatek.op18.telephony;

import android.content.Context;
import android.util.Log;



import com.mediatek.internal.telephony.ServiceStateTrackerExt;

public class Op18ServiceStateExt extends ServiceStateTrackerExt {
    private static final String TAG = "Op18ServiceStateExt";
    private Context mContext;
    public Op18ServiceStateExt() {
    }

    public Op18ServiceStateExt(Context context) {
        mContext = context;
    }
    /**
     * Disable IVSR featur
     * Return if need disable IVSR.
     * @return if need disable IVSR
     */
    public boolean isNeedDisableIVSR() {
        Log.i(TAG, "Disable IVSR");
        return true;
    }
}
