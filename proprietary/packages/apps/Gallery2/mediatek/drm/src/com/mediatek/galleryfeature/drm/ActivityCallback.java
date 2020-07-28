package com.mediatek.galleryfeature.drm;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;

import com.mediatek.gallerybasic.base.IActivityCallback;


public class ActivityCallback implements IActivityCallback {
    public static final String ACTION_REVIEW = "com.android.camera.action.REVIEW";
    public static final String CTA_PROPERTY = "ro.vendor.mtk_cta_set";
    private String mToken = null;
    private String mTokenKey = null;

    @Override
    public void onCreate(Activity activity) {

        Intent intent = activity.getIntent();
        if (intent.getData() == null) {
           return;
        }
        if (DrmHelper.sSupportCTA) {
            mToken = intent.getStringExtra("TOKEN");
            mTokenKey = intent.getStringExtra("TOKEN_KEY");
            if (null == mToken || !DrmHelper.isTokenValid((Context) activity, mTokenKey, mToken)) {
                return;
            }
            intent.setAction(ACTION_REVIEW);
            Uri uri = intent.getData();
        }

    }

    @Override
    public void onStart(Activity activity) {

    }

    @Override
    public void onResume(Activity activity) {

    }

    @Override
    public void onPause(Activity activity) {
        if (mTokenKey == null) {
            return;
        }
        DrmHelper.clearToken((Context) activity, mTokenKey, mToken);
    }

    @Override
    public void onStop(Activity activity) {

    }

    @Override
    public void onDestroy(Activity activity) {

    }

}
