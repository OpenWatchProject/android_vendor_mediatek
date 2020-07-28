
package com.android.music.tests.mtbf;



import com.android.music.*;
import com.jayway.android.robotium.solo.Solo;

import android.app.Instrumentation;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.media.MediaScannerConnection;
import android.os.IBinder;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.View;
import android.widget.ListView;

public class MusicMTBFTest extends ActivityInstrumentationTestCase2<MusicBrowserActivity> implements
        ServiceConnection {

    private static final String TAG = "MusicMTBFTest";
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final int WAIT_FOR_SERVICE_CONNECTED_TIME = 5000;

    private Instrumentation mInstrumentation = null;
    private MusicBrowserActivity mMusicBrowserActivity = null;
    private ListView mListView = null;
    private Solo mSolo = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;
    private static final String SDCARD_PATH_MNT_PREFIX = "/mnt/";
    private static final String SDCARD_PATH_STORAGE_PREFIX = "/storage/";
    private static final String SDCARD0_STR = "sdcard0/";
    private static final String MUSIC_FILE_PATH = "sdcard0/Music/";
    private static final String MUSIC_PODCASTS_FILE_PATH = "sdcard0/Podcasts/";

    public MusicMTBFTest(Class<MusicBrowserActivity> activityClass) {
        super(activityClass);
    }

    public MusicMTBFTest() {
        super(MusicBrowserActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.d(TAG, "setUp>>>");
        setActivityInitialTouchMode(true);
        mInstrumentation = getInstrumentation();
        mMusicBrowserActivity = (MusicBrowserActivity) getActivity();
        mContext = mMusicBrowserActivity.getApplicationContext();
        mSolo = new Solo(mInstrumentation, mMusicBrowserActivity);
        // Bind service
        Intent iService = new Intent(mInstrumentation.getTargetContext(),
                MediaPlaybackService.class);
        mInstrumentation.getContext().bindService(iService, this, Context.BIND_AUTO_CREATE);
        if (!mIsMusicServiceConnected) {
            synchronized (mLock) {
                while (!mIsMusicServiceConnected) {
                    mLock.wait(WAIT_FOR_SERVICE_CONNECTED_TIME);
                }
            }
        }
        // Assert all used to be not null
        assertNotNull(mInstrumentation);
        assertNotNull(mMusicBrowserActivity);
        assertNotNull(mContext);
        assertNotNull(mSolo);
        assertNotNull(mService);

        // mListView = mMusicBrowserActivity.getListView();
        // assertNotNull(mListView);
        Log.d(TAG, "setUp<<<");
    }

    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "tearDown>>>");
        try {
            mSolo.finalize();
        } catch (Throwable t) {
            t.printStackTrace();
        }
        mSolo.finishOpenedActivities();
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Log.d(TAG, "tearDown<<<");
        super.tearDown();
    }

    public void test00_DeleteAllSongs() throws Exception {
        Log.d(TAG, ">> test00_DeleteAllSongs");
        deleteAllSongsFromExternalSdcard();
        Log.d(TAG, "<< test00_DeleteAllSongs");
    }

    public void test01_ToAlbumInterface() throws Exception {
        Log.d(TAG, ">> test01_ToAlbumInterface");
        // Click album tab to test in the MusicBrowserActivity
        View alubmTab = (View) mMusicBrowserActivity.findViewById(R.id.albumtab);
        mSolo.clickOnView(alubmTab);
        mInstrumentation.waitForIdleSync();
        Log.d(TAG, "<< test01_ToAlbumInterface");
    }

    public void test02_ScanTestAudios() {
        Log.d(TAG, ">> test02_ScanTestAudios");
        try {
            scanTestAudios();
        } catch (Exception e) {
            // TODO: handle exception
        }
        Log.d(TAG, "<< test02_ScanTestAudios");
    }

    @Override
    public void onServiceConnected(ComponentName name, IBinder service) {
        Log.d(TAG, "onServiceConnected");
        mService = IMediaPlaybackService.Stub.asInterface(service);
        mIsMusicServiceConnected = true;
        synchronized (mLock) {
            mLock.notify();
        }
    }

    @Override
    public void onServiceDisconnected(ComponentName name) {
        Log.d(TAG, "onServiceDisconnected");
        mService = null;
        mIsMusicServiceConnected = false;
    }

    public int deleteAllSongsFromExternalSdcard() {
        long[] list;
        String[] cols = new String[] {
                MediaStore.Audio.Media._ID, MediaStore.Audio.Media.DATA
        };
        Cursor c = MusicUtils.query(mContext, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, cols,
                null, null, null);

        if (c != null) {
            if (c.getCount() > 0) {
                list = new long[c.getCount()];
            } else {
                return -1;
            }
            c.moveToFirst();
            int index = 0;
            while (!c.isAfterLast()) {
                // remove from current playlist
                long id = c.getLong(0);
                list[index] = id;
                c.moveToNext();
                index++;
            }
            c.close();
            return MusicUtils.deleteTracks(mContext, list);
        }
        return -1;
    }

    public void scanTestAudios() throws Exception {
        Log.d(TAG, ">> scanTestAudios");
        String[] paths = {
/*                SDCARD_PATH_MNT_PREFIX + MUSIC_FILE_PATH,
                SDCARD_PATH_STORAGE_PREFIX + MUSIC_FILE_PATH,
                SDCARD_PATH_MNT_PREFIX + MUSIC_PODCASTS_FILE_PATH,
                SDCARD_PATH_STORAGE_PREFIX + MUSIC_PODCASTS_FILE_PATH*/
                SDCARD_PATH_MNT_PREFIX + SDCARD0_STR, SDCARD_PATH_STORAGE_PREFIX + SDCARD0_STR
        };
        MediaScannerConnection.scanFile(mContext, paths, null, null);
        Log.d(TAG, "<< scanTestAudios");
    }

    public void switchToAlbumTab() {
        Log.d(TAG, ">> switchToAlbumTab");
        // Click album tab to test in the MusicBrowserActivity
        View alubmTab = (View) mMusicBrowserActivity.findViewById(R.id.albumtab);
        mSolo.clickOnView(alubmTab);
        mInstrumentation.waitForIdleSync();
        Log.d(TAG, "<< switchToAlbumTab");
    }


}
