package com.mediatek.dialer.calllog.presence;

import android.app.Activity;
import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import com.mediatek.dialer.calllog.presence.CallLogListItemViewHolderExt;
import com.mediatek.dialer.ext.DefaultCallLogExt;
import com.mediatek.dialer.ext.ICallLogListItemViewHolderExt;
import com.mediatek.dialer.ext.ICallLogAdapterExt;

public class CallLogExt extends DefaultCallLogExt {
    private static final String TAG = "CallLogExt";

    public static Context mPluginContext;
    public CallLogExt(Context context) {
        Log.d(TAG, "CallLogExt");
        mPluginContext = context;
    }

    @Override
    public ICallLogListItemViewHolderExt getCallLogListItemViewHolderExt(
            Context context, RecyclerView.ViewHolder viewHolder) {
      Log.d(TAG, "getCallLogListItemViewHolderExt");
      return new CallLogListItemViewHolderExt(context, viewHolder);
    }

    @Override
    public ICallLogAdapterExt getCallLogAdapterExt(Activity activity,
            RecyclerView.Adapter adapter) {
        Log.d(TAG, "getCallLogAdapterExt");
        return new CallLogAdapterExt(activity, adapter);
    }
}
