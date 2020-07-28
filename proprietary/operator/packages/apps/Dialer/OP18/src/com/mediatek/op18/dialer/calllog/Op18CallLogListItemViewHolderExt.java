package com.mediatek.op18.dialer.calllog;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import com.android.dialer.app.calllog.CallLogListItemViewHolder;
import com.android.dialer.calllogutils.CallbackActionHelper.CallbackAction;
import com.mediatek.dialer.ext.DefaultCallLogListItemViewHolderExt;

public class Op18CallLogListItemViewHolderExt extends DefaultCallLogListItemViewHolderExt {
    private static final String TAG = "Op18CallLogListItemViewHolderExt ";

    private CallLogListItemViewHolder mCallLogListItemViewHolder;
    private Context mHostContext;

    Op18CallLogListItemViewHolderExt (Context context, RecyclerView.ViewHolder viewHolder) {
        mCallLogListItemViewHolder = (CallLogListItemViewHolder)viewHolder;
        mHostContext = context;
    }

    @Override
    public void bindActionButtons() {
        Log.d(TAG, "bindActionButtons : " + mCallLogListItemViewHolder.callbackAction);
    }

    @Override
    public void updatePrimaryActionButton(boolean show) {
        Log.d(TAG, "updatePrimaryActionButton mCallbackAction :"
               + mCallLogListItemViewHolder.callbackAction);
    }

}
