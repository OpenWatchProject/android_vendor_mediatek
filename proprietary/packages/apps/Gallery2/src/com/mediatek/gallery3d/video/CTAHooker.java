package com.mediatek.gallery3d.video;

import android.app.Activity;
import android.content.Intent;
import android.drm.DrmManagerClient;
import android.os.Bundle;

import com.mediatek.galleryportable.Log;
import com.mediatek.omadrm.OmaDrmUtils;

public class CTAHooker extends MovieHooker {

    private static final String TAG = "VP_CTAHooker";
    private static final String CTA_ACTION = "com.mediatek.dataprotection.ACTION_VIEW_LOCKED_FILE";
    private DrmManagerClient mDrmClient;
    private Activity mActivity;
    private String mToken;
    private String mTokenKey;
    private boolean mIsCtaPlayback;

    public void onCreate(Bundle savedInstanceState) {
        mActivity = getContext();
        checkIntentAndToken();
    }

    public void onPause() {
        finishPlayIfNeed();
    }

    /**
     * Check videoplayer is launched by DataProtection app or not. if launched
     * by DataProtection, should check the token value is valid or not.
     */
    public void checkIntentAndToken() {
        mDrmClient = new DrmManagerClient(mActivity.getApplicationContext());
        Intent intent = mActivity.getIntent();
        String action = intent.getAction();
        Log.d(TAG, "checkIntentAndToken action = " + action);
        if (CTA_ACTION.equals(action)) {
            mToken = intent.getStringExtra("TOKEN");
            mTokenKey = intent.getStringExtra("TOKEN_KEY");
            if (mToken == null || !OmaDrmUtils.isTokenValid(mDrmClient, mTokenKey, mToken)) {
                mDrmClient.release();
                mDrmClient = null;
                mActivity.finish();
                return;
            }
            mIsCtaPlayback = true;
        }
    }

    /**
     * If videoplayer back to background when playing a cta file, it should
     * finish and return to the DataProtection app.
     */
    public void finishPlayIfNeed() {
        Log.d(TAG, "finishPlayIfNeed mIsCtaPlayback = " + mIsCtaPlayback);
        if (mIsCtaPlayback) {
            OmaDrmUtils.clearToken(mDrmClient, mTokenKey, mToken);
            mTokenKey = null;
            mToken = null;
            mIsCtaPlayback = false;
            mDrmClient.release();
            mDrmClient = null;
            mActivity.finish();
        } else if (mDrmClient != null) {
            mDrmClient.release();
            mDrmClient = null;
        }
    }
}
