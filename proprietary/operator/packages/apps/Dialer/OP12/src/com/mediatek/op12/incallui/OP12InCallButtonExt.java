package com.mediatek.op12.incallui;

import android.content.Context;
import android.util.Log;

import com.mediatek.incallui.ext.DefaultInCallButtonExt;

/**
 * Plug in implementation for OP12 InCallButton interfaces.
 */
public class OP12InCallButtonExt extends DefaultInCallButtonExt {
    private static final String TAG = "OP12InCallButtonExt";
    private Context mContext;

    /** Constructor.
     * @param context context
     */
    public OP12InCallButtonExt(Context context) {
        mContext = context;
    }

    /**
     * Check if one way video call is supported
     * @param call call object
     */
    @Override
    public boolean isOneWayVideoSupportedForCall(Object call) {
      Log.d(TAG, "isOneWayVideoSupportedForCall entry");
      return true;
    }

    @Override
    public boolean isOneWayVideoSupported() {
      Log.d(TAG, "isOneWayVideoSupported true");
      return true;
    }
}