package com.mediatek.mail.utils;

import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;

import com.android.mail.utils.LogUtils;
import com.android.mail.utils.Throttle;

/**
 * M: This class aim to ignore some notification when database changed frequently.
 * This is a better solution to handle the case of data change too frequently.
 */
public class ThrottleContentObserver extends ContentObserver {

    private static final String TAG = "ThrottleContentObserver";
    private final Throttle mThrottle;
    private Context mInnerContext;
    private boolean mRegistered;
    private String mName;

    public ThrottleContentObserver(Handler handler, Context context,
            Runnable runnable, String name) {
        super(handler);
        mInnerContext = context;
        mName = name;
        mThrottle = new Throttle(name, runnable, handler);
    }

    public ThrottleContentObserver(Handler handler, Context context,
            String name, Throttle throttle) {
        super(handler);
        mInnerContext = context;
        mName = name;
        mThrottle = throttle;
    }

    @Override
    public void onChange(boolean selfChange) {
        if (mRegistered) {
            mThrottle.onEvent();
        }
    }

    public void unregister() {
        LogUtils.d(TAG, "%s unregister", mName);
        if (!mRegistered) {
            return;
        }
        mThrottle.cancelScheduledCallback();
        mInnerContext.getContentResolver().unregisterContentObserver(this);
        mRegistered = false;
    }

    public void register(Uri notifyUri) {
        unregister();
        mInnerContext.getContentResolver().registerContentObserver(notifyUri, true, this);
        mRegistered = true;
        LogUtils.d(TAG, "%s register", mName);
    }

}
