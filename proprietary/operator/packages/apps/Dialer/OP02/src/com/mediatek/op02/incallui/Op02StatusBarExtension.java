package com.mediatek.op02.incallui;

import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.telecom.Call;
import android.util.Log;

import com.android.incallui.call.DialerCall;
import com.mediatek.incallui.ext.DefaultStatusBarExt;

import mediatek.telecom.MtkCall;

public class Op02StatusBarExtension extends DefaultStatusBarExt {
    private static final String TAG = "Op02StatusBarExtension";
    private static final String HD_ACTION = "com.android.incallui.ACTION_SHOW_DISMISS_HD_ICON";
    private static final String HD_ACTION_FLAG = "FLAG_KEY_VISIBILITY";
    private static final String HD_PERMISSION = "com.mediatek.permission.BROADCAST_HD_INFO";
    private Context   mContext;
    private boolean   mIsShowingHdIcon;
    public Op02StatusBarExtension(Context context) {
        super();
        Log.d(TAG, "Op02StatusBarExtension");
        mContext = context;
    }

    /**
      * Interface to show statusbar's hd icon of OP02
      *
      * @param call    the incallui call
      */
    @Override
    public void updateInCallNotification(Object call) {
        if (call == null) {
            notifyHdIcon(false);
            return;
        }

        DialerCall incallCall = (DialerCall) call;
        Call telecomCall = incallCall.getTelecomCall();

        if (telecomCall.getDetails() == null) {
            Log.d(TAG, "showHdAudioIndicator, details is null");
            return;
        }

        int property = telecomCall.getDetails().getCallProperties();
        Log.d(TAG, "showHdAudioIndicator, property = " +
            telecomCall.getDetails().propertiesToString(property));
        if (!telecomCall.getDetails().hasProperty(MtkCall.MtkDetails.MTK_PROPERTY_VOLTE) &&
            telecomCall.getDetails().hasProperty(Call.Details.PROPERTY_HIGH_DEF_AUDIO) &&
            telecomCall.getState() == Call.STATE_ACTIVE) {
            notifyHdIcon(true);
        } else {
            notifyHdIcon(false);
        }
    }

    private void notifyHdIcon(boolean isHdIconOn) {
        Log.d(TAG, "notifyHdIcon, isHdIconOn = " + isHdIconOn +
            ", mIsShowingHdIcon = " + mIsShowingHdIcon);
        if (mIsShowingHdIcon == isHdIconOn || mContext == null) {
            return;
        }

        Intent intent = new Intent(HD_ACTION);
        intent.putExtra(HD_ACTION_FLAG, isHdIconOn);
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL, HD_PERMISSION);
        mIsShowingHdIcon = isHdIconOn;
        Log.d(TAG, "notifyHdIcon, send hd broadcast~");
    }

}
