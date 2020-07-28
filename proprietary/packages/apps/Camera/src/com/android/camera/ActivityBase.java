package com.android.camera;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ActivityInfo;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.provider.MediaStore.Images.Media;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;

import java.util.ArrayList;
import java.util.regex.PatternSyntaxException;
/**
 * Superclass of Camera and VideoCamera activities.
 */
public abstract class ActivityBase extends Activity {
    private static final String TAG = "ActivityBase";

    private static final String INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE =
            "android.media.action.STILL_IMAGE_CAMERA_SECURE";
    public static final String ACTION_IMAGE_CAPTURE_SECURE =
            "android.media.action.IMAGE_CAPTURE_SECURE";
    // The intent extra for camera from secure lock screen. True if the gallery
    // should only show newly captured pictures. sSecureAlbumId does not
    // increment. This is used when switching between camera, camcorder, and
    // panorama. If the extra is not set, it is in the normal camera mode.
    public static final String SECURE_CAMERA_EXTRA = "secure_camera";

    // The activity is paused. The classes that extend this class should set
    // mPaused the first thing in onResume/onPause.
    protected boolean mPaused;

    // Secure album id. This should be incremented every time the camera is
    // launched from the secure lock screen. The id should be the same when
    // switching between camera, camcorder, and panorama.
    protected static int sSecureAlbumId;
    // True if the camera is started from secure lock screen.
    protected boolean mSecureCamera;
    private static boolean sFirstStartAfterScreenOn = true;
    private boolean mIsLockScreen = false;
    private boolean mNeedShowThumbnail = true;
    private boolean mIsGotoGallery = false;

    // just for test
    private int mResultCodeForTesting;
    private Intent mResultDataForTesting;

    private ArrayList<String> mSecureArray = new ArrayList<String>();
    private String mPath = null;
    // Add for API2 supported
    protected ICameraActivityBridge mCameraActivityBridge = null;

    protected abstract ICameraActivityBridge getCameraActivityBridge();

    // close activity when screen turns off
    private BroadcastReceiver mScreenOffReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            finish();
            Log.d(TAG, "mScreenOffReceiver receive");
        }
    };

    private static BroadcastReceiver sScreenOffReceiver;

    private static class ScreenOffReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            sFirstStartAfterScreenOn = true;
        }
    }

    public static boolean isFirstStartAfterScreenOn() {
        return sFirstStartAfterScreenOn;
    }

    public static void resetFirstStartAfterScreenOn() {
        sFirstStartAfterScreenOn = false;
    }

    @Override
    public void onCreate(Bundle icicle) {
        Log.i(TAG, "ActivityBase oncreate");
        if (Util.isWfdEnabled(this) || FeatureSwitcher.isTablet()) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
            setRequestedOrientation(calculateCurrentScreenOrientation());
        }
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_ACTION_BAR_OVERLAY);

        getActionBar().hide();
        // Check if this is in the secure camera mode.
        Intent intent = getIntent();
        String action = intent.getAction();
        if (INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE.equals(action)) {
            mSecureCamera = true;
            mIsLockScreen = true;
            // Use a new album when this is started from the lock screen.
            sSecureAlbumId++;
        } else if (ACTION_IMAGE_CAPTURE_SECURE.equals(action)) {
            mSecureCamera = true;
        } else {
            mSecureCamera = intent.getBooleanExtra(SECURE_CAMERA_EXTRA, false);
        }
        if (mSecureCamera) {
            setScreenFlags();
            mNeedShowThumbnail = !mIsLockScreen;
            mPath = "/secure/all/" + sSecureAlbumId;
            IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
            registerReceiver(mScreenOffReceiver, filter);
            if (sScreenOffReceiver == null) {
                sScreenOffReceiver = new ScreenOffReceiver();
                getApplicationContext().registerReceiver(sScreenOffReceiver,
                        filter);
            }
        }

        super.onCreate(icicle);
    }

    @Override
    protected void onResume() {
        mIsGotoGallery = false;
        mPaused = false;
        updateSecureThumbnail();
        super.onResume();
    }

    @Override
    protected void onPause() {
        mPaused = true;
        mNeedShowThumbnail = true;
        if (mIsLockScreen && (mSecureArray.isEmpty()
                || !mIsGotoGallery)) {
            Log.i(TAG, "[onPause] Secure Camera go to Gallery" + mIsGotoGallery);
            mNeedShowThumbnail = false;
        }
        if (!mIsGotoGallery) {
            mSecureArray.clear();
        }
        super.onPause();
    }

    @Override
    public boolean onSearchRequested() {
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // Prevent software keyboard or voice search from showing up.
        if (keyCode == KeyEvent.KEYCODE_SEARCH
                || keyCode == KeyEvent.KEYCODE_MENU) {
            if (event.isLongPress()) {
                return true;
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    protected void setResultEx(int resultCode) {
        mResultCodeForTesting = resultCode;
        setResult(resultCode);
    }

    public void setResultEx(int resultCode, Intent data) {
        mResultCodeForTesting = resultCode;
        mResultDataForTesting = data;
        setResult(resultCode, data);
    }

    public int getResultCode() {
        return mResultCodeForTesting;
    }

    public Intent getResultData() {
        return mResultDataForTesting;
    }

    @Override
    protected void onDestroy() {
        if (mSecureCamera) {
            unregisterReceiver(mScreenOffReceiver);
        }
        super.onDestroy();
    }

    protected void addSecureAlbumItemIfNeeded(boolean isVideo, Uri uri) {
        // There are two cases that we should add secure array
        // in secure camera.
        // 1. pausing secure camera and going to gallery.
        // 2. secure camera is resumed.
        if (mSecureCamera && (!mPaused || (mPaused && mIsGotoGallery))) {
            Log.i(TAG, "addSecureAlbumItemIfNeeded uri = " + uri);
            int id = Integer.parseInt(uri.getLastPathSegment());
            // Notify Gallery the secure albums through String format
            // such as "4321+true", means file's id = 4321 and is video
            String videoIndex = isVideo ? "+true" : "+false";
            String secureAlbum = String.valueOf(id) + videoIndex;
            mSecureArray.add(secureAlbum);
        }
    }

    protected ArrayList<String> getSecureAlbum() {
        return mSecureArray;
    }

    public void setPath(String setPath) {
        // mAppBridge.setCameraPath(setPath);
    }

    public String getPath() {
        return mPath;
    }
    public void notifyGotoGallery() {
        mIsGotoGallery = true;
    }
    public int getSecureAlbumCount() {
        Log.d(TAG, "[getSecureAlbumCount] mNeedShowThumbnail = " + mNeedShowThumbnail);
        return mNeedShowThumbnail ? 1 : 0;
    }

    public boolean isActivityOnpause() {
        Log.i(TAG, "isActivityOnpause , mpaused = " + mPaused);
        return mPaused;
    }

    public boolean isFullScreen() {
        return true;
    }

    public boolean isSecureCamera() {
        return mSecureCamera;
    }

    private void setScreenFlags() {
        final Window win = getWindow();
        final WindowManager.LayoutParams params = win.getAttributes();
        params.flags |= WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED;
        win.setAttributes(params);
    }

    private void updateSecureThumbnail() {
        if (mIsLockScreen && !mSecureArray.isEmpty()) {
            if (checkSecureAlbumLive()) {
                mNeedShowThumbnail = true;
            } else {
                mNeedShowThumbnail = false;
            }
            Log.i(TAG, "mNeedShowThumbnail = " + mNeedShowThumbnail);
        }
    }
    // Check file whether exist in provider by id.
    private boolean isSecureUriLive(int id) {
        Cursor cursor = null;
        try {
            // for file kinds of uri, query media database
            cursor = Media.query(getContentResolver(), MediaStore.Files
                    .getContentUri("external"),
                     null, "_id=(" + id + ")", null, null);
            if (null != cursor) {
                Log.w(TAG, "<isSecureUriLive> cursor " + cursor.getCount());
                return cursor.getCount() > 0;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }
        return true;
    }
    //check all files in Secure array whether exit in provider
    private boolean checkSecureAlbumLive() {
        if (mSecureArray != null && !mSecureArray.isEmpty()) {
            int albumCount = mSecureArray.size();
            Log.d(TAG, "<checkSecureAlbum> albumCount " + albumCount);
            for (int i = 0; i < albumCount; i++) {
                try {
                    String[] albumItem = mSecureArray.get(i).split("\\+");
                    int albumItemSize = albumItem.length;
                    Log.d(TAG, "<checkSecureAlbum> albumItemSize "
                               + albumItemSize);
                    if (albumItemSize == 2) {
                        int id = Integer.parseInt(albumItem[0].trim());
                        boolean isVideo = Boolean.parseBoolean(albumItem[1]
                                .trim());
                        Log.d(TAG, "<checkSecureAlbum> secure item : id " + id
                                + ", isVideo " + isVideo);
                        if (isSecureUriLive(id)) {
                            return true;
                        }
                    }
                } catch (NullPointerException ex) {
                    Log.e(TAG, "<checkSecureAlbum> exception " + ex);
                } catch (PatternSyntaxException ex) {
                    Log.e(TAG, "<checkSecureAlbum> exception " + ex);
                } catch (NumberFormatException ex) {
                    Log.e(TAG, "<checkSecureAlbum> exception " + ex);
                }
            }
        }
        return false;
    }
    private int calculateCurrentScreenOrientation() {
        int displayRotation = Util.getDisplayRotation(this);
        Log.i(TAG, "calculateCurrentScreenOrientation displayRotation = " + displayRotation);
        if (displayRotation == 0) {
            return ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
        } else if (displayRotation == 90) {
            return ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE;
        } else if (displayRotation == 180) {
            return ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT;
        } else if (displayRotation == 270) {
            return ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
        }
        return ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
    }
}
