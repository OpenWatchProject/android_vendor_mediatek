package com.mediatek.op18.dialer;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.util.Log;

import com.android.dialer.app.list.ListsFragment;
import com.mediatek.dialer.ext.DefaultDialPadExtension;



public class Op18DialPadExtension extends DefaultDialPadExtension {
    private static final String TAG = "Op18DialPadExtension";
    private static final int CALL_INITIATION_DIALPAD = 2;
    private Context mContext;
    private Activity mHostActivity;
    private String mHostPackage;
    private Resources mHostResources;
    private static final int TAB_INDEX_RECENTS = 1;

    public Op18DialPadExtension(Context context) {
        mContext = context;
    }

    @Override
    public void customizeDefaultTAB(Object object) {
        log("customizeDefaultTAB");
        ListsFragment listsFragment = (ListsFragment) object;
        log("getTABIndex");
        listsFragment.showTab(TAB_INDEX_RECENTS);
    }

    private void log(String msg) {
        Log.d(TAG, msg);
    }
}
