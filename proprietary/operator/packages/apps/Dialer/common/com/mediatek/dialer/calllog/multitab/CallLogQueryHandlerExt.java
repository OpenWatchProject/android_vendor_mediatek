package com.mediatek.dialer.calllog.multitab;

import android.content.Context;
import android.util.Log;

import android.provider.CallLog.Calls;
import com.android.dialer.app.calllog.CallLogActivity;
import com.android.dialer.database.CallLogQueryHandler;

import com.mediatek.dialer.ext.DefaultCallLogQueryHandlerExt;
import com.mediatek.dialer.calllog.accountfilter.PhoneAccountUtils;
import com.mediatek.dialer.calllog.accountfilter.PhoneAccountInfoHelper;
import com.mediatek.dialer.common.LogUtils;

import java.util.List;

public class CallLogQueryHandlerExt extends DefaultCallLogQueryHandlerExt {
    private static final String TAG = "CallLogQueryHandler ";
    private Context mContext;
    //only callLogFragment in callLogActivity Need show calllog according to phone account.
    private Boolean mIsCallLogActivity = false;

    public CallLogQueryHandlerExt(Context context) {
        mContext = context;
        if (context instanceof CallLogActivity) {
            Log.d(TAG, "CallLogQueryHandlerExt mIsCallLogActivity true");
            mIsCallLogActivity = true;
        }
    }

    @Override
    public void appendQuerySelection(int callType, StringBuilder builder,
            List<String> selectionArgs) {
        if (!mIsCallLogActivity) {
            Log.d(TAG, "appendQuerySelection mIsCallLogActivity false");
            return;
        }

        String preferAccountId = PhoneAccountInfoHelper.getPreferAccountId();
        LogUtils.printSensitiveInfo(TAG, "appendQuerySelection preferAccountId:" + preferAccountId);

        if (!PhoneAccountInfoHelper.FILTER_ALL_ACCOUNT_ID.equals(preferAccountId)) {
            if (builder.length() > 0) {
                builder.append(" AND ");
            }
            // query the Call Log by account id
            builder.append(String.format("(%s = ?)", Calls.PHONE_ACCOUNT_ID));
            selectionArgs.add(preferAccountId);
        }
        LogUtils.printSensitiveInfo(TAG, "builder: " + builder);
    }
}
