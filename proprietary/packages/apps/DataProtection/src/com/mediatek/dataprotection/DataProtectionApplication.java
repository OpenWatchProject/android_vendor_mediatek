package com.mediatek.dataprotection;

import java.util.concurrent.atomic.AtomicBoolean;

import android.app.ActivityManager;
import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.drm.DrmManagerClient;
import android.graphics.Bitmap;
import android.os.SystemClock;
import android.preference.PreferenceManager;
import android.util.LruCache;

public class DataProtectionApplication extends Application {

    private ThumbnailCache mThumbnails;
    private static boolean sIsLowRamDevice = false;
    public static final long LOCKPATTERN_ATTEMPT_TIMEOUT = 30000L;
    private static final String LOCKOUT_DEADLINE = "lockout_deadline";
    private static final String TAG = "DataProtectionApplication";
    private AtomicBoolean mIsActivityActive = new AtomicBoolean(false);
    private DrmManagerClient mClient = null;

    private long mLockoutAttempDeadLine = 0;
    private boolean mNeedShowCancel = false;
    private boolean mNeedShowDecryptFailFiles = false;
    private boolean mHasPriviledge = false;

    public static ThumbnailCache getThumbnailsCache(Context context) {
        final DataProtectionApplication app =
                (DataProtectionApplication) context.getApplicationContext();
        return app.mThumbnails;
    }

    public static void setActivityState(Context context, boolean state) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        app.setActivityActive(state);
    }

    public static DrmManagerClient getCtaClient(Context context) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        return app.getCtaClient();
    }

    public static boolean getActivityState(Context context) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        return app.isActivityActive();
    }

    public static long getLockoutAttemptDeadline(Context context) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        return app.getLockoutAttemptDeadline();
    }

    public static long setLockoutAttemptDeadline(Context context) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        return app.setLockoutAttemptDeadline();
    }

    public static void setNeedShowCancel(Context context,
            boolean isNeedShowCancel) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        app.setNeedShowCancel(isNeedShowCancel);
    }

    public static boolean isNeedShowCancel(Context context) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        return app.isNeedShowCancel();
    }

    public static void setNeedShowDecryptFail(Context context,
            boolean isNeedShow) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        app.setNeedShowDecryptFail(isNeedShow);
    }

    public static boolean isNeedShowDecryptFail(Context context) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        return app.isNeedShowDecryptFail();
    }

    public static boolean getPriviledge(Context context) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        return app.getPriviledge();
    }

    public static void setPriviledge(Context context,
            boolean isNeedShow) {
        final DataProtectionApplication app = (DataProtectionApplication) context
                .getApplicationContext();
        app.setPriviledge(isNeedShow);
    }

    private DrmManagerClient getCtaClient() {
        if (mClient == null) {
            mClient = new DrmManagerClient(this);
        }
        return mClient;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        final ActivityManager am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        final int memoryClassBytes = am.getMemoryClass() * 1024 * 1024;
        sIsLowRamDevice = am.isLowRamDevice();
        mThumbnails = new ThumbnailCache(memoryClassBytes / 4);
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);

        if (level >= TRIM_MEMORY_MODERATE) {
            mThumbnails.evictAll();
        } else if (level >= TRIM_MEMORY_BACKGROUND) {
            mThumbnails.trimToSize(mThumbnails.size() / 2);
        }
    }

    public class ThumbnailCache extends LruCache<String, Bitmap> {
        public ThumbnailCache(int maxSizeBytes) {
            super(maxSizeBytes);
        }

        @Override
        protected int sizeOf(String key, Bitmap value) {
            return value.getByteCount();
        }
    }

    private boolean isActivityActive() {
        return mIsActivityActive.get();
    }

    private void setActivityActive(boolean state) {
        mIsActivityActive.set(state);
    }

    private long setLockoutAttemptDeadline() {
        long deadline = LOCKPATTERN_ATTEMPT_TIMEOUT
                + SystemClock.elapsedRealtime();
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        SharedPreferences.Editor editor = prefs.edit();
        editor.putLong(LOCKOUT_DEADLINE, deadline);
        editor.commit();
        return deadline;
    }

    private long getLockoutAttemptDeadline() {
        SharedPreferences prefs = PreferenceManager
                .getDefaultSharedPreferences(getApplicationContext());
        long deadline = prefs.getLong(LOCKOUT_DEADLINE, 0);
        final long now = SystemClock.elapsedRealtime();
        if (deadline < now || deadline > (now + LOCKPATTERN_ATTEMPT_TIMEOUT)) {
            return 0L;
        }
        return deadline;
    }

    private void setNeedShowCancel(boolean isNeedShowCancel) {
        mNeedShowCancel = isNeedShowCancel;
    }

    private boolean isNeedShowCancel() {
        return mNeedShowCancel;
    }

    private void setNeedShowDecryptFail(boolean isNeedShowDecryptFail) {
        mNeedShowDecryptFailFiles = isNeedShowDecryptFail;
    }

    private boolean isNeedShowDecryptFail() {
        return mNeedShowDecryptFailFiles;
    }

    private void setPriviledge(boolean invoke) {
        mHasPriviledge = invoke;
    }

    private boolean getPriviledge() {
        return mHasPriviledge;
    }
}
