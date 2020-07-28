package com.mediatek.dialer.search.presence;

import android.content.Context;
import android.view.View;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;
import com.mediatek.dialer.common.LogUtils;
import com.mediatek.dialer.ext.DefaultSearchActionViewHolderExt;
import com.mediatek.dialer.presence.PresenceApiManager;

public class SearchActionViewHolderExt extends DefaultSearchActionViewHolderExt {
    private static final String TAG = "SearchActionViewHolderExt ";
    private static final float ALPHA_TRANSPARENT_VALUE = 0.3f;
    private static final float ALPHA_OPAQUE_VALUE = 1.0f;
    private Context mHostContext;
    private PresenceApiManager mTapi = null;
    private final Integer MAKE_VILTE_CALL = 4;

    public SearchActionViewHolderExt (Context context) {
        Log.d(TAG, "SearchActionViewHolderExt");
        mHostContext = context;
        if (PresenceApiManager.initialize(mHostContext)) {
            mTapi = PresenceApiManager.getInstance();
        }
    }

    @Override
    public void setAction(ImageView actionImage, TextView actionText, String query, int action) {
        LogUtils.printSensitiveInfo(TAG, "setAction query: " + query);
            if (action == MAKE_VILTE_CALL && !mTapi.isVideoCallCapable(query)) {
                Log.i(TAG, "setAction has video capable");
                actionImage.setAlpha(ALPHA_TRANSPARENT_VALUE);
                actionText.setAlpha(ALPHA_TRANSPARENT_VALUE);
            } else {
                Log.i(TAG, "setAction no video capable");
                actionImage.setAlpha(ALPHA_OPAQUE_VALUE);
                actionText.setAlpha(ALPHA_OPAQUE_VALUE);
           }
    }
}
