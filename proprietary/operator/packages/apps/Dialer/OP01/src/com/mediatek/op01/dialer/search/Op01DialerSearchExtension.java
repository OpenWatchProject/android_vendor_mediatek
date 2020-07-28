package com.mediatek.op01.dialer.search;

import android.app.Fragment;
import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;

import com.mediatek.dialer.ext.DefaultDialerSearchExtension;
import com.mediatek.dialer.ext.ISearchFragmentExt;
import java.util.function.Consumer;
import java.util.List;

public class Op01DialerSearchExtension extends DefaultDialerSearchExtension {
    private static final String TAG = "Op01DialerSearchExtension";

    public static Context mPluginContext;
    public Op01DialerSearchExtension(Context context) {
        Log.d(TAG, "Op01DialerSearchExtension");
        mPluginContext = context;
    }

    @Override
    public ISearchFragmentExt getSearchFragmentExt(Fragment fragment, RecyclerView.Adapter adapter,
            Consumer<List<Integer>> iUpdateSearchActions) {
        Log.d(TAG, "getSearchFragmentExt");
        return new Op01SearchFragmentExt(fragment, adapter, iUpdateSearchActions);
    }
}
