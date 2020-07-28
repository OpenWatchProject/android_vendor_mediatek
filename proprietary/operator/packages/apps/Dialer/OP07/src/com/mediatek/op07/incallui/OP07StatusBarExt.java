package com.mediatek.op07.incallui;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;

import com.android.incallui.call.DialerCall;
import com.android.incallui.call.state.DialerCallState;
import com.mediatek.incallui.ext.DefaultStatusBarExt;

public class OP07StatusBarExt extends DefaultStatusBarExt {
    private static final String TAG = "OP07StatusBarExt";
    private Context   mContext;

    private static final String KEY_EXTRA_VERSTAT        = "verstat";
    private static final String VERSTAT_VALID_STATUS = " (Valid Number)";
    private static final String VERSTAT_INVALID_STATUS = " (Invalid Number)";
    private static final int DEFAULT_VAULE_EXTRA_VERSTAT = -1;

    public OP07StatusBarExt(Context context) {
        super();
        Log.d(TAG, "OP07StatusBarExt");
        mContext = context;
    }

    /**
     * Get StatusBarNotifier instance.
     * @param obj Object
     * @param contentTitle String
     * @return String the customized String title
     */
    @Override
    public String getContentTitle(Object object, String contentTitle) {
        if (object == null || contentTitle == null) {
            return contentTitle;
        }

        DialerCall call = (DialerCall) object;
        String resultVaule = contentTitle;
        if (call.getState() == DialerCallState.INCOMING
                || call.getState() == DialerCallState.CALL_WAITING) {
            Bundle bundle = call.getExtras();
            if (bundle != null) {
                int value = bundle.getInt(KEY_EXTRA_VERSTAT, DEFAULT_VAULE_EXTRA_VERSTAT);
                if (value == 1) {
                    resultVaule += VERSTAT_VALID_STATUS;
                } else if (value == 0) {
                    resultVaule += VERSTAT_INVALID_STATUS;
                } else {
                    Log.d(TAG, "getContentTitle, will return original title");
                }
            }
        }
        /// dumpBundleInfo(call);
        return resultVaule;
    }

    private void dumpBundleInfo(DialerCall call) {
        Log.d(TAG, "dumpBundleInfo");
        Bundle bundle = call.getExtras();
        if (bundle != null) {
            for (String key : bundle.keySet()) {
                Log.d(TAG, "dumpBundleInfo, Key = " + key);
            }
        }
    }
}
