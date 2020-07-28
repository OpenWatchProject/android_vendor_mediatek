package com.mediatek.mms.util;

import android.util.Log;

import java.util.Date;

public class DrmCacheItem {

    private static final String TAG = "DrmCacheItem";
    private static long sLatestTime = new Date().getTime();
    // time out is 5 minutes
    private static long sTimeOut = 1000 * 60 * 5;

    private String mKey; // uri
    private String mSrc;
    private boolean mIsDrm;
    private boolean mHasRight;
    private long mVisitTime;

    public DrmCacheItem() {

    }

    public DrmCacheItem(String key, String src, boolean isDrm, boolean hasRight) {
        this.mKey = key;
        this.mSrc = src;
        this.mIsDrm = isDrm;
        this.mHasRight = hasRight;
        this.mVisitTime = new Date().getTime();
        Log.d(TAG, "CacheItem create: " + mVisitTime);
    }

    public String getKey() {
        return mKey;
    }

    public void setKey(String key) {
        this.mKey = key;
    }

    public boolean isDrm() {
        if (DrmUtilsEx.isDrm(mSrc) || mIsDrm) {
            return true;
        } else {
            return false;
        }
    }

    public boolean hasRight() {
        return mHasRight;
    }

    public void visit() {
        mVisitTime = new Date().getTime();
        Log.d(TAG, "CacheItem visit: " + mVisitTime);
        if (sLatestTime < mVisitTime) {
            sLatestTime = mVisitTime;
        }
    }

    public boolean isExpired() {
        if (mVisitTime < sLatestTime - sTimeOut) {
            return true;
        } else {
            return false;
        }
    }
}
