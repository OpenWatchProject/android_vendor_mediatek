package com.mediatek.dialer.calllog.presence;

import android.content.Context;
import android.view.View;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.widget.ImageView;
import com.android.dialer.app.calllog.CallLogListItemViewHolder;
import com.android.dialer.calllogutils.CallbackActionHelper.CallbackAction;

import com.mediatek.dialer.common.LogUtils;

import com.mediatek.dialer.ext.DefaultCallLogListItemViewHolderExt;
import com.mediatek.dialer.presence.ContactNumberUtils;
import com.mediatek.dialer.presence.PresenceApiManager;
import com.mediatek.dialer.presence.PresenceApiManager.CapabilitiesChangeListener;
import com.mediatek.dialer.presence.PresenceApiManager.ContactInformation;

public class CallLogListItemViewHolderExt extends DefaultCallLogListItemViewHolderExt {
    private static final String TAG = "CallLogListItemViewHolderExt ";

    private static final float ALPHA_TRANSPARENT_VALUE = 0.3f;
    private static final float ALPHA_OPAQUE_VALUE = 1.0f;
    private CallLogListItemViewHolder mCallLogListItemViewHolder;
    private Context mHostContext;
    private PresenceApiManager mTapi = null;
    public boolean mShow = false;
    public String mNumber;

    public CallLogListItemViewHolderExt (Context context, RecyclerView.ViewHolder viewHolder) {
        mCallLogListItemViewHolder = (CallLogListItemViewHolder)viewHolder;
        mHostContext = context;
        if (PresenceApiManager.initialize(context)) {
            mTapi = PresenceApiManager.getInstance();
        }
    }

    @Override
    public void bindActionButtons() {
        Log.d(TAG, "bindActionButtons : " + mCallLogListItemViewHolder.callbackAction);
        if (mCallLogListItemViewHolder.callbackAction == CallbackAction.IMS_VIDEO) {
            Log.d(TAG, "bindActionButtons , no video call button");
            return;
        }

        if (isVisible(mCallLogListItemViewHolder.videoCallButtonView) && mTapi != null) {
            if (mTapi.isVideoCallCapable(mCallLogListItemViewHolder.number)) {
                Log.d(TAG, "OPCallLogListItemViewHolder showAction has video capable");
                mCallLogListItemViewHolder.videoCallButtonView.setAlpha(ALPHA_OPAQUE_VALUE);
            } else {
                Log.d(TAG, "OPCallLogListItemViewHolder showAction no video capable");
                mCallLogListItemViewHolder.videoCallButtonView.setAlpha(ALPHA_TRANSPARENT_VALUE);
            }
        } else {
            Log.d(TAG, "OPCallLogListItemViewHolder showAction  mTapi:"
                    + mTapi + " isVisible(videoButton): false");
        }
    }

    @Override
    public void updatePrimaryActionButton(boolean show) {
        mShow = show;
        mNumber = mCallLogListItemViewHolder.number;
        LogUtils.printSensitiveInfo(TAG,
                "updatePrimaryActionButton show :" + show + " number:" + mNumber);
        updatePrimaryActionButton();
    }

    void updatePrimaryActionButton() {
        Log.d(TAG, "updatePrimaryActionButton no param mCallbackAction :"
               + mCallLogListItemViewHolder.callbackAction);
        ImageView actionButtion = mCallLogListItemViewHolder.primaryActionButtonView;
        if (mCallLogListItemViewHolder.callbackAction == CallbackAction.IMS_VIDEO) {
            if (mTapi != null
                    && mTapi.isVideoCallCapable(mCallLogListItemViewHolder.number)) {
                Log.d(TAG, "customizePrimaryActionButton has video capable");
                actionButtion.setAlpha(ALPHA_OPAQUE_VALUE);
            } else {
                actionButtion.setAlpha(ALPHA_TRANSPARENT_VALUE);
                Log.d(TAG, "customizePrimaryActionButton no video capable");
            }
        } else if (isVisible(actionButtion)) {
            actionButtion.setAlpha(ALPHA_OPAQUE_VALUE);
        }
    }

    private boolean isVisible(View view) {
        return view != null && view.getVisibility() == View.VISIBLE;
    }
}
