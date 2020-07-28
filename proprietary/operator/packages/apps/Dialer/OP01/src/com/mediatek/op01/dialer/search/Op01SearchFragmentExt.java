package com.mediatek.op01.dialer.search;

import android.app.Fragment;
import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.android.dialer.searchfragment.list.SearchAdapter;
import com.mediatek.dialer.ext.DefaultSearchFragmentExt;
import com.mediatek.op01.dialer.PhoneStateUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Consumer;

public class Op01SearchFragmentExt extends DefaultSearchFragmentExt
    implements PhoneStateUtils.OnChangedListener {
    private static final String TAG = "Op01SearchFragmentExt ";
    //Copy it from SearchActionViewHOlder Action.MAKE_VILTE_CALL
    private final Integer MAKE_VILTE_CALL = 4;
    private Context mHostContext;
    private Fragment mFragment;
    private SearchAdapter mSearchAdapter;
    private Consumer<List<Integer>> mUpdateSearchActions;
    private List<Integer> mOriginalSearchActions;

    Op01SearchFragmentExt(Fragment fragment, RecyclerView.Adapter adapter,
            Consumer<List<Integer>> iUpdateSearchActions) {
        Log.i(TAG, "Op01SearchFragmentExt");
        mFragment = fragment;
        mHostContext = mFragment.getActivity();
        mSearchAdapter = (SearchAdapter)adapter;
        mUpdateSearchActions = iUpdateSearchActions;

        PhoneStateUtils.getInstance(mHostContext).addPhoneStateListener(this);
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy");
        PhoneStateUtils.getInstance(mHostContext).removePhoneStateListener(this);
    }

    @Override
    public void setActions(List<Integer> actions, String query) {
        Log.i(TAG, "setActions actions:" + actions);
        mOriginalSearchActions = new ArrayList<>();
        mOriginalSearchActions.addAll(actions);

        int state = PhoneStateUtils.getPhoneState();
        Log.i(TAG, "setActions state : " + state);
        if (state != TelephonyManager.CALL_STATE_IDLE
                && actions != null && actions.contains(MAKE_VILTE_CALL)) {
            actions.remove(MAKE_VILTE_CALL);
            Log.i(TAG, "setActions remove(MAKE_VILTE_CALL and set adapter");
            mUpdateSearchActions.accept(actions);
        }
    }

    @Override
    public void onCallStatusChange(final int state) {
        Log.i(TAG, "onCallStatusChange update ui. state:" + state);
        Log.i(TAG, "onCallStatusChange update ui. SearchActions:" + mOriginalSearchActions);
        if (mOriginalSearchActions != null && mOriginalSearchActions.contains(MAKE_VILTE_CALL)) {
            if (state != TelephonyManager.CALL_STATE_IDLE) {
                ArrayList<Integer> action = new ArrayList<>();
                action.addAll(mOriginalSearchActions);
                action.remove(MAKE_VILTE_CALL);
                Log.i(TAG, "onCallStatusChange mUpdateSearchActions.accept remove");
                mUpdateSearchActions.accept(action);
            } else {
                Log.i(TAG, "onCallStatusChange mUpdateSearchActions.accept restore");
                mUpdateSearchActions.accept(mOriginalSearchActions);
            }
        }
    }
}
