package com.mediatek.dialer.search.presence;

import android.app.Fragment;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import com.mediatek.dialer.common.LogUtils;
import com.mediatek.dialer.ext.DefaultSearchFragmentExt;
import com.mediatek.dialer.presence.ContactNumberUtils;
import com.mediatek.dialer.presence.PresenceApiManager;
import com.mediatek.dialer.presence.PresenceApiManager.CapabilitiesChangeListener;
import com.mediatek.dialer.presence.PresenceApiManager.ContactInformation;
import java.util.ArrayList;
import java.util.List;

public class SearchFragmentExt extends DefaultSearchFragmentExt
    implements CapabilitiesChangeListener {
    private static final String TAG = "SearchFragmentExt ";
    //Copy it from SearchActionViewHOlder Action.MAKE_VILTE_CALL
    private final Integer MAKE_VILTE_CALL = 4;
    private Context mHostContext;
    private Fragment mFragment;
    private RecyclerView.Adapter mAdapter;
    private List<Integer> mOriginalSearchActions;
    private String mQuery;
    private String mFormatNumber;
    private Handler mHandler = null;
    private PresenceApiManager mTapi = null;

    SearchFragmentExt(Fragment fragment, RecyclerView.Adapter adapter) {
        Log.i(TAG, "SearchFragmentExt");
        mFragment = fragment;
        mHostContext = mFragment.getActivity();
        mAdapter = adapter;
        mHandler = new Handler(Looper.getMainLooper());
        if (PresenceApiManager.initialize(mHostContext)) {
            mTapi = PresenceApiManager.getInstance();
            mTapi.addCapabilitiesChangeListener(this);
        }
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy");
        if (mTapi != null) {
            mTapi.removeCapabilitiesChangeListener(this);
        }
    }

    @Override
    public void setActions(List<Integer> actions, String query) {
        Log.i(TAG, "setActions actions:" + actions);
        LogUtils.printSensitiveInfo(TAG, "setActions query:" + query);
        mOriginalSearchActions = actions;
        mQuery = query;

        mFormatNumber = ContactNumberUtils.getDefault().getFormatNumber(mQuery);
        LogUtils.printSensitiveInfo(TAG,
                "setActions requestContactPresence mFormatNumber:" + mFormatNumber);
        if (mTapi != null) {
            mTapi.requestContactPresence(mFormatNumber, true);
        }
     }

    @Override
    public void onCapabilitiesChanged(String number, ContactInformation info) {
        LogUtils.printSensitiveInfo(TAG, "onCapabilitiesChanged number: " + number);

        if (mOriginalSearchActions != null && mOriginalSearchActions.contains(MAKE_VILTE_CALL)
                && mFormatNumber != null && number.equals(mFormatNumber)) {
             Log.i(TAG, "onCapabilitiesChanged notifyDataSetChanged");
             mHandler.post(() -> mAdapter.notifyDataSetChanged());
        }
    }

    @Override
    public void onErrorReceived(String contact, int type, int status, String reason) {
    }
}
