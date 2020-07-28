package com.mediatek.dialer.search.presence;

import android.app.Fragment;
import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;

import com.mediatek.dialer.ext.DefaultDialerSearchExtension;
import com.mediatek.dialer.ext.ISearchActionViewHolderExt;
import com.mediatek.dialer.ext.ISearchFragmentExt;
import com.mediatek.dialer.search.presence.SearchActionViewHolderExt;

import java.util.function.Consumer;
import java.util.List;

public class DialerSearchExtension extends DefaultDialerSearchExtension {
    private static final String TAG = "DialerSearchExtension";

    public static Context mPluginContext;
    public DialerSearchExtension(Context context) {
        Log.d(TAG, "DialerSearchExtension");
        mPluginContext = context;
    }

    @Override
    public ISearchActionViewHolderExt getSearchActionViewHolderExt(Context context) {
        Log.d(TAG, "getSearchActionViewHolderExt");
        return new SearchActionViewHolderExt(context);
    }

    @Override
    public ISearchFragmentExt getSearchFragmentExt(Fragment fragment, RecyclerView.Adapter adapter,
            Consumer<List<Integer>> iUpdateSearchActions) {
        Log.d(TAG, "getSearchFragmentExt");
        return new SearchFragmentExt(fragment, adapter);
    }
}
