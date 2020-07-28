package com.mediatek.op01.dialer.calllog;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import com.android.dialer.app.calllog.CallLogListItemViewHolder;
import com.android.dialer.calllogutils.CallbackActionHelper.CallbackAction;
import com.mediatek.dialer.ext.DefaultCallLogListItemViewHolderExt;
import com.mediatek.op01.dialer.PhoneStateUtils;

public class Op01CallLogListItemViewHolderExt extends DefaultCallLogListItemViewHolderExt {
    private static final String TAG = "Op01CallLogListItemViewHolderExt ";

    private CallLogListItemViewHolder mCallLogListItemViewHolder;
    private Context mHostContext;

    Op01CallLogListItemViewHolderExt (Context context, RecyclerView.ViewHolder viewHolder) {
        mCallLogListItemViewHolder = (CallLogListItemViewHolder)viewHolder;
        mHostContext = context;
    }

    @Override
    public void bindActionButtons() {
        Log.d(TAG, "bindActionButtons : " + mCallLogListItemViewHolder.callbackAction);
        if (mCallLogListItemViewHolder.callbackAction == CallbackAction.IMS_VIDEO) {
            Log.d(TAG, "bindActionButtons , no video call button");
            return;
        }
        int state = PhoneStateUtils.getPhoneState();
        Log.d(TAG, "bindActionButtons state : " + state);
        if (state != TelephonyManager.CALL_STATE_IDLE
                && isVisible(mCallLogListItemViewHolder.videoCallButtonView)) {
            mCallLogListItemViewHolder.videoCallButtonView.setVisibility(View.GONE);
        }
    }

    @Override
    public void updatePrimaryActionButton(boolean show) {
        Log.d(TAG, "updatePrimaryActionButton mCallbackAction :"
               + mCallLogListItemViewHolder.callbackAction);
        if (mCallLogListItemViewHolder.callbackAction != CallbackAction.IMS_VIDEO) {
            return;
        }
        int state = PhoneStateUtils.getPhoneState();
        Log.d(TAG, "updatePrimaryActionButton state : " + state);
        if (state != TelephonyManager.CALL_STATE_IDLE) {
            mCallLogListItemViewHolder.primaryActionButtonView.setVisibility(View.GONE);
        }
    }

    private boolean isVisible(View view) {
        return view != null && view.getVisibility() == View.VISIBLE;
    }
}
