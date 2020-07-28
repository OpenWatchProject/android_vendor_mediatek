package com.mediatek.op09clib.dialer;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.util.Log;
import com.mediatek.dialer.calllog.multitab.CallLogActivityExt;
import com.mediatek.dialer.calllog.multitab.CallLogFragmentExt;
import com.mediatek.dialer.calllog.multitab.CallLogQueryHandlerExt;
import com.mediatek.dialer.ext.DefaultCallLogExt;
import com.mediatek.dialer.ext.ICallLogActivityExt;
import com.mediatek.dialer.ext.ICallLogFragmentExt;
import com.mediatek.dialer.ext.ICallLogQueryHandlerExt;
import java.util.function.Consumer;

public class Op09CallLogExt extends DefaultCallLogExt {
    private static final String TAG = "Op09CallLogExt";

    public static Context mPluginContext;
    public Op09CallLogExt(Context context) {
        mPluginContext = context;
    }

    @Override
    public ICallLogActivityExt getCallLogActivityExt(Activity callLogActivity) {
      Log.d(TAG, "getCallLogActivityExt");
      return new CallLogActivityExt(callLogActivity, mPluginContext);
    }

    @Override
    public ICallLogFragmentExt getCallLogFragmentExt(
            Fragment fragment, boolean isCallLogActivity,
            int callTypeFilter,  Consumer<Boolean> forceFresh) {
      Log.d(TAG, "getCallLogFragmentExt");
      if (isCallLogActivity) {
          Log.d(TAG, "Op09CallLogFragmentExt");
          return new CallLogFragmentExt(fragment, isCallLogActivity,
                  callTypeFilter, mPluginContext, forceFresh);
      } else {
          Log.d(TAG, "super.getCallLogFragmentExt");
          return super.getCallLogFragmentExt(fragment,
              isCallLogActivity, callTypeFilter, forceFresh);
      }
    }

    @Override
    public ICallLogQueryHandlerExt getCallLogQueryHandlerExt(Context context) {
      Log.d(TAG, "getCallLogQueryHandler");
      return new CallLogQueryHandlerExt(context);
     }
}
