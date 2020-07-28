package com.mediatek.op01.dialer.calllog;

import android.app.Fragment;
import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.widget.ImageView;
import com.mediatek.dialer.ext.DefaultCallLogExt;
import com.mediatek.dialer.ext.ICallLogListItemViewHolderExt;
import com.mediatek.dialer.ext.ICallLogFragmentExt;
import java.util.function.Consumer;

public class Op01CallLogExt extends DefaultCallLogExt {
    private static final String TAG = "Op01CallLogExt";

    public static Context mPluginContext;
    public Op01CallLogExt(Context context) {
        Log.d(TAG, "Op01CallLogExt");
        mPluginContext = context;
    }

    @Override
    public ICallLogListItemViewHolderExt getCallLogListItemViewHolderExt(
            Context context, RecyclerView.ViewHolder viewHolder) {
      Log.d(TAG, "getCallLogListItemViewHolderExt");
      return new Op01CallLogListItemViewHolderExt(context, viewHolder);
    }

    @Override
    public ICallLogFragmentExt getCallLogFragmentExt(
            Fragment fragment, boolean isCallLogActivity,
            int callTypeFilter, Consumer<Boolean> forceFresh) {
      Log.d(TAG, "getCallLogFragmentExt");
      return new Op01CallLogFragmentExt(fragment, mPluginContext);
    }
}
