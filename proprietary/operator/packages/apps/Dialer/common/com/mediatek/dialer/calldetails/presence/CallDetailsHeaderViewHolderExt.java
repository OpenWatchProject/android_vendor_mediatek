package com.mediatek.dialer.calldetails.presence;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import com.android.dialer.calllogutils.CallbackActionHelper.CallbackAction;
import com.mediatek.dialer.ext.DefaultCallDetailsHeaderViewHolderExt;
import com.mediatek.dialer.presence.PresenceApiManager;
import com.mediatek.dialer.common.LogUtils;

public class CallDetailsHeaderViewHolderExt extends DefaultCallDetailsHeaderViewHolderExt {
    private static final String TAG = "CallDetailsHeaderViewHolderExt ";

    private int mCallbackAction;
    private ImageView mCallbackButton;
    private Context mHostContext;
    private PresenceApiManager mTapi = null;

    CallDetailsHeaderViewHolderExt (Context context, RecyclerView.ViewHolder viewHolder) {
        Log.d(TAG, "CallDetailsHeaderViewHolderExt");
        mHostContext = context;
        if (PresenceApiManager.initialize(mHostContext)) {
            mTapi = PresenceApiManager.getInstance();
        }
    }

    @Override
    public void setCallbackAction(ImageView callbackButton, int callbackAction, String number) {
        LogUtils.printSensitiveInfo(TAG,
                "setCallbackAction: " + callbackAction + "number: " + number);
         this.mCallbackButton = callbackButton;
         this.mCallbackAction = callbackAction;
         if (mCallbackButton != null && mCallbackAction == CallbackAction.IMS_VIDEO) {
             if (mTapi != null && mTapi.isVideoCallCapable(number)) {
                 Log.d(TAG, "setCallbackAction: isVideoCallCapable true, show button" );
                 mCallbackButton.setVisibility(View.VISIBLE);
             } else {
                 Log.d(TAG, "setCallbackAction: isVideoCallCapable false, set button gone" );
                 mCallbackButton.setVisibility(View.GONE);
             }
         }
    }
}
