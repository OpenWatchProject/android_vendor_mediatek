package com.mediatek.op01.dialer.calllog;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import com.android.dialer.app.calllog.CallLogFragment;
import com.mediatek.dialer.ext.DefaultCallLogFragmentExt;
import com.mediatek.op01.dialer.PhoneStateUtils;

public class Op01CallLogFragmentExt extends DefaultCallLogFragmentExt
    implements PhoneStateUtils.OnChangedListener {
    private static final String TAG = "Op01CallLogFragmentExt ";

    private Context mPluginContext;
    private CallLogFragment mFragment;
    private RecyclerView.Adapter mAdapter;
    private Context mHostContext;

    Op01CallLogFragmentExt(Fragment fragment, Context pluginContext) {
        mFragment = (CallLogFragment)fragment;
        mPluginContext = pluginContext;
        mHostContext = mFragment.getActivity();
        Log.d(TAG, "new CallLogFragmentExt fragment: " + fragment);
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume ");
        PhoneStateUtils.getInstance(mHostContext).addPhoneStateListener(this);
        mAdapter = mFragment.getAdapter();
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause ");
        PhoneStateUtils.getInstance(mHostContext).removePhoneStateListener(this);
    }

    @Override
    public void onCallStatusChange(final int state) {
        Log.d(TAG, "onCallStatusChange update ui. state:" + state);
        mAdapter.notifyDataSetChanged();
    }
}
