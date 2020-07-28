package com.mediatek.op01.dialer.calldetails;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import com.android.dialer.calllogutils.CallbackActionHelper.CallbackAction;
import com.mediatek.dialer.ext.DefaultCallDetailsHeaderViewHolderExt;
import com.mediatek.op01.dialer.PhoneStateUtils;

public class Op01CallDetailsHeaderViewHolderExt extends DefaultCallDetailsHeaderViewHolderExt
     {
    private static final String TAG = "Op01CallDetailsHeaderViewHolderExt ";

    private int mCallbackAction;
    private ImageView mCallbackButton;
    private Context mHostContext;

    Op01CallDetailsHeaderViewHolderExt (Context context, RecyclerView.ViewHolder viewHolder) {
        Log.d(TAG, "Op01CallDetailsHeaderViewHolderExt");
        mHostContext = context;
    }

    @Override
    public void setCallbackAction(ImageView callbackButton, int callbackAction, String number) {
         Log.d(TAG, "setCallbackAction: " + callbackAction);
         this.mCallbackButton = callbackButton;
         this.mCallbackAction = callbackAction;
         if (mCallbackButton != null && mCallbackAction == CallbackAction.IMS_VIDEO) {
             if (PhoneStateUtils.getPhoneState() != TelephonyManager.CALL_STATE_IDLE) {
                 mCallbackButton.setVisibility(View.GONE);
             } else {
                 mCallbackButton.setVisibility(View.VISIBLE);
             }
         }
    }
}
