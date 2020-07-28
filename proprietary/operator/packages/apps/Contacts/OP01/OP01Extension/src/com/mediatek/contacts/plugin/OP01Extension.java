package com.mediatek.contacts.plugin;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.PorterDuff;
import android.net.Uri;

import android.provider.CallLog.Calls;
import android.telephony.PhoneStateListener;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.contacts.CallUtil;

import com.mediatek.contacts.ext.DefaultOp01Extension;
import com.mediatek.provider.MtkCallLog;

public class OP01Extension extends DefaultOp01Extension {
    private static final String TAG = "OP01Extension";

    private static final int CALL_ARROW_ICON_RES = R.drawable.ic_call_arrow;
    private static final int CALL_AUTOREJECT_ICON_RES = R.drawable.ic_call_autoreject_arrow;

    //Must keep same as com.android.contacts.CallUtil.VIDEO_CALLING_ENABLED = 1
    private static final int VIDEO_CALLING_ENABLED = 1;

    private Context mContext;
    private static Context sContextHost;
    private TelephonyManager mTM;
    private Uri mLookupUri;
    private boolean mVideoButton = false;
    private boolean mCallRegistered = false;
    private PhoneStateListener mCallListener;

    public boolean isVideoEnabled(Context context) {
        int videoCapabilities = CallUtil.getVideoCallingAvailability(context);
        return (videoCapabilities & VIDEO_CALLING_ENABLED) != 0;
    }

    public OP01Extension(Context context) {
        mContext = context;
        mTM = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        Log.d(TAG, "create mCallListener. looper=" + mContext.getMainLooper());
        mCallListener = new PhoneStateListener(mContext.getMainLooper()) {
            public void onCallStateChanged(int state, String number) {
                boolean enabled = isVideoEnabled(mContext);
                Log.d(TAG, "onCallStateChanged video: " + enabled + " mVideoButton: " +
                        mVideoButton + " call: " + state);
                if (enabled && mLookupUri != null) {
                    if ((mVideoButton && state != TelephonyManager.CALL_STATE_IDLE) ||
                        (!mVideoButton && state == TelephonyManager.CALL_STATE_IDLE)) {
                        Log.i(TAG, "notifyChange: " + mLookupUri);
                        mContext.getContentResolver().notifyChange(mLookupUri, null, false);
                    }
                }
            }
        };
    }

    /**
     * Op01 will check video button visibility or not.
     * @param enabled Host Video Enabled
     * @param uri Contact's Uri
     * @param params Extend Parameters
     * @return True or False
     */
    @Override
    public boolean isVideoButtonEnabled(boolean enabled, Uri uri, Object...params) {
        boolean result = enabled;
        Log.d(TAG, "isVideoButtonEnabled video: " + enabled + " mCallRegistered: " +
                mCallRegistered + " uri: " + uri);
        int state = -1;
        if (enabled) {
            state = mTM.getCallState();
            if (state != TelephonyManager.CALL_STATE_IDLE) {
                result = false;
            }
            if (!mCallRegistered) {
                mTM.listen(mCallListener, PhoneStateListener.LISTEN_CALL_STATE);
                mCallRegistered = true;
                Log.d(TAG, "PhoneStateListener Registered");
            }
        }
        mVideoButton = result;
        mLookupUri = uri;
        Log.d(TAG, "isVideoButtonEnabled mVideoButton: " + mVideoButton + " call: " + state);
        return result;
    }

    /**
     * Op01 will reset values of video state.
     */
    @Override
    public void resetVideoState() {
        Log.d(TAG, "resetVideoState mCallRegistered: " + mCallRegistered);
        if (mCallRegistered) {
            mTM.listen(mCallListener, PhoneStateListener.LISTEN_NONE);
            mCallRegistered = false;
        }
        mVideoButton = false;
        mLookupUri = null;
    }
}
