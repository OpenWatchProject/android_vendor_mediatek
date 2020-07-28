package com.mediatek.galleryfeature.drm;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.provider.MediaStore.Files;
import android.provider.MediaStore.MediaColumns;

import com.mediatek.gallerybasic.gl.MBitmapTexture;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.omadrm.OmaDrmUtils;

import java.util.ArrayList;
import java.util.HashMap;

public class DrmRightsManager {
    private static final String TAG = "MtkGallery2/DrmRightsManager";

    private volatile static DrmRightsManager sDrmRightManager;
    private static Context sContext;
    private static String sDrmContentUri = null;

    private HandlerThread mRightUpdateThread;
    private Handler mHandler;
    private DrmManagerClient mDrmManagerClient;
    //private long mMinUpdateTime = Long.MAX_VALUE;

    private HashMap<String, Boolean> mRightsMap =
            new HashMap<String, Boolean>();
    private ArrayList<String> mUpdateList = new ArrayList<String>();
    private ArrayList<String> mRightsChangeListenersPath =
            new ArrayList<String>();
    private RightsChangeListener mListener;

    private Bitmap mLockBitmap;
    private Bitmap mUnlockBitmap;
    private MBitmapTexture mLockMBitmapTexture;
    private MBitmapTexture mUnlockMBitmapTexture;

    private static final long TWENTY_DAYS = 20 * 24 * 60 * 60L;
    private static final long SECOND_TO_MS = 1000L;
    private static final long ONE_SECOND = 10L;
    private static final int MSG_UPDATE_RIGHTS = 1;

    private DrmRightsManager() {
        createHandleAndThread();
        initMBitmapTexture();
    }

    public  static DrmRightsManager getDrmRightManager(Context context) {
        sContext = context;

        if (sDrmRightManager == null) {
            synchronized (DrmRightsManager.class) {
                if (sDrmRightManager == null) {
                    sDrmRightManager = new DrmRightsManager();
                    sDrmContentUri = DrmHelper.getDrmContentUri();
                }
            }
        }
        return sDrmRightManager;
    }

    public void createHandleAndThread() {
        Log.v(TAG, "<createHandleAndThread>");
        if (mRightUpdateThread != null) {
            mRightUpdateThread.quit();
            mRightUpdateThread = null;
        }

        mRightUpdateThread = new HandlerThread("Rights_updata_thread");
        mRightUpdateThread.start();

        mHandler = new Handler(mRightUpdateThread.getLooper()) {
            @Override
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                Log.v(TAG, "<handleMessage>,msg:" + msg);

                ArrayList<String> rightsChangeList = new ArrayList<String>();
                for (int i = mUpdateList.size() - 1; i >= 0; i--) {
                    String path = mUpdateList.get(i);
                    boolean rights = getDrmRights(path);
                    synchronized (mRightsMap) {
                        if (rights != mRightsMap.get(path)) {
                            rightsChangeList.add(path);
                            mUpdateList.remove(i);
                        }
                    }
                }
                for (int i = rightsChangeList.size() - 1; i >= 0; i--) {
                    String drmContentUri =
                        getDrmContentUriByPath(rightsChangeList.get(i));

                    if (drmContentUri != null && mListener != null) {
                        ArrayList<String> sameContentUriFile =
                            updateRightsByContentUri(drmContentUri);
                        mListener.onRightsChange(sameContentUriFile);

                    }
                }
            }
        };
    }

    public void addOrUpdateItem(String path) {
        //save rights status in map.
        boolean rights = getDrmRights(path);

        synchronized (mRightsMap) {
            mRightsMap.put(path, rights);
            Log.v(TAG, "<addOrUpdateItem> enter,path:" + path + ", rights:" + rights
                + ",size:" + mRightsMap.size());
        }

        if (rights) {
            String drmContentUri = getDrmContentUriByPath(path);
            if (drmContentUri == null) {
                Log.w(TAG, "<addOrUpdateItem> can not get drm_content_uri, return");
                return;
            }
            for (int i = mUpdateList.size() - 1; i >= 0; i--) {
                String updatePathContentUri =
                    getDrmContentUriByPath(mUpdateList.get(i));
                if (drmContentUri.equals(updatePathContentUri)) {
                    //has the same content uri file, no need add into mUpdateList;
                    return;
                }
            }

            String mimetype = DrmHelper.getOmaDrmClient(sContext)
                .getOriginalMimeType(path);
            ContentValues cv = DrmHelper.getOmaDrmClient(sContext)
                .getConstraints(path, OmaDrmUtils.getActionByMimetype(mimetype));

            if (cv == null) {
                return;
            }
            long startTime = cv.getAsLong(DrmStore.ConstraintsColumns.LICENSE_START_TIME);
            long expireTime = cv.getAsLong(
                    DrmStore.ConstraintsColumns.LICENSE_EXPIRY_TIME);
            long currentTime = System.currentTimeMillis() / SECOND_TO_MS; //use "s"

            Log.e(TAG, "<addOrUpdateItem> data-time type,path:" + path +
                ",cv:" + cv + ",currentTime:" + currentTime);
            if (startTime != -1 && expireTime != -1
                    && currentTime >= startTime && currentTime < expireTime) {
                long duration =  expireTime - currentTime;
                // if the duration is longer than 20 days or less than 0, no need update.
                if (duration > TWENTY_DAYS || duration <= 0) {
                    return;
                } else {
                    mUpdateList.add(path);

                    boolean result = mHandler.sendEmptyMessageDelayed(
                        MSG_UPDATE_RIGHTS, duration * SECOND_TO_MS + ONE_SECOND);
                    Log.e(TAG, "<addOrUpdateItem> sendEmptyMessageDelayed,path:" + path);

                    if (!result) {
                        createHandleAndThread();
                        mHandler.sendEmptyMessageDelayed(
                            MSG_UPDATE_RIGHTS, duration * SECOND_TO_MS + ONE_SECOND);
                    }
                }
            }
        }
    }

    private boolean getDrmRights(String path) {
        if (!DrmHelper.sSupportDrm || !DrmHelper.isDrmFile(path)) {
            return false;
        }

        String mimetype = DrmHelper.getOmaDrmClient(sContext)
            .getOriginalMimeType(path);
        int right = DrmHelper.getOmaDrmClient(sContext)
            .checkRightsStatus(path, OmaDrmUtils.getActionByMimetype(mimetype));

        return right == DrmStore.RightsStatus.RIGHTS_VALID;
    }

    public MBitmapTexture getDrmTexture(String path) {
        synchronized (mRightsMap) {
            if (mRightsMap.get(path) != null && mRightsMap.get(path)) {
                return mUnlockMBitmapTexture;
            }
        }
        return mLockMBitmapTexture;
    }

    public void initMBitmapTexture() {
        mLockBitmap = BitmapFactory.decodeResource(sContext.getResources(),
            com.mediatek.internal.R.drawable.drm_red_lock);
        mLockMBitmapTexture = new MBitmapTexture(mLockBitmap, false, false);
        mLockMBitmapTexture.setOpaque(false);

        mUnlockBitmap = BitmapFactory.decodeResource(sContext.getResources(),
            com.mediatek.internal.R.drawable.drm_green_lock);
        mUnlockMBitmapTexture = new MBitmapTexture(mUnlockBitmap, false, false);
        mUnlockMBitmapTexture.setOpaque(false);
    }

    private String getDrmContentUriByPath(String filePath) {
        Cursor cursor = null;
        String drmContentUri = null;
        String where = MediaColumns.DATA + "=" + "lower('" + filePath + "')";
        try {
            cursor = sContext.getContentResolver().query(Files.getContentUri("external"),
                new String[] {sDrmContentUri}, where, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                drmContentUri = cursor.getString(0);
                Log.d(TAG, "<getDrmContentUriByPath> drm_content_uri is " + drmContentUri);
            }
        } catch (SQLiteException e) {
            Log.d(TAG, "<getDrmContentUriByPath> excepttion " + e);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return drmContentUri;
    }

    private ArrayList<String> updateRightsByContentUri(String drmContentUri) {
        ArrayList<String> sameContentUriFile = new ArrayList<String>();

        Cursor cursor = null;
        String where = sDrmContentUri + "=" + "lower('" + drmContentUri + "')";
        try {
            cursor = sContext.getContentResolver().query(Files.getContentUri("external"),
                new String[] {MediaColumns.DATA}, where, null, null);
            while (cursor != null && cursor.moveToNext()) {
                String path = cursor.getString(0);
                synchronized (mRightsMap) {
                    mRightsMap.put(path, getDrmRights(path));
                }
                sameContentUriFile.add(path);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return sameContentUriFile;
    }

    public void release() {
        if (mRightUpdateThread != null) {
            mRightUpdateThread.quit();
        }

        if (mUpdateList != null) {
            mUpdateList.clear();
        }

        synchronized (mRightsMap) {
            if (mRightsMap != null) {
                mRightsMap.clear();
            }
        }

        if (mRightsChangeListenersPath != null) {
            mRightsChangeListenersPath.clear();
        }

        if (mLockBitmap != null && !mLockBitmap.isRecycled()) {
            mLockBitmap.recycle();
            mLockBitmap = null;
        }

        if (mUnlockBitmap != null && !mUnlockBitmap.isRecycled()) {
            mUnlockBitmap.recycle();
            mUnlockBitmap = null;
        }

        mListener = null;
        sDrmRightManager = null;
    }

    public void registerRightsChangeListener(String path,
            RightsChangeListener listener) {
        Log.e(TAG, "<registerRightsChangeListener>,path:" + path
            + ",listener:" + listener);
        mRightsChangeListenersPath.add(path);
        if (mListener == null) {
            mListener = listener;
        }
    }

    public static interface RightsChangeListener {
        public void onRightsChange(ArrayList<String> updateContentList);
    }
}
