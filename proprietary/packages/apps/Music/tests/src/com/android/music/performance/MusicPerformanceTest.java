package com.android.music.tests.performance;

import java.util.Random;


import com.android.music.*;
import com.android.music.tests.functional.*;
import com.jayway.android.robotium.solo.Solo;

import android.app.Instrumentation;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.sqlite.SQLiteConstraintException;
import android.media.MediaScannerConnection;
import android.os.IBinder;
import android.provider.MediaStore;
import android.provider.MediaStore.Audio;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.View;
import android.widget.ListView;

public class MusicPerformanceTest extends
        ActivityInstrumentationTestCase2<MusicBrowserActivity> implements ServiceConnection {

    private static final String TAG = "MusicPerformanceTest";
    private static final String SONGS_LESS_THAN_TRHEE = "There are less than three songs or the" +
    " system performance is bad, please push more or reboot phone to make music pass test";
    private static final int WAIT_FOR_ACTIVITY_CREATED_TIME = 10000;
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final int WAIT_FOR_NEXT_TEST_TIME = 1000;
    private static final int WAIT_FOR_SERVICE_CONNECTED_TIME = 5000;
    private static final int MINIMUM_SONGS_COUNT_IN_MUSIC = 3;

    private Instrumentation mInstrumentation = null;
    private TrackBrowserActivity mTrackBrowserActivity = null;
    private MediaPlaybackActivity mMediaPlaybackActivity = null;
    private MusicBrowserActivity mMusicBrowserActivity = null;
    private ListView mListView = null;
    private Solo mSolo = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;

    public MusicPerformanceTest(Class<MusicBrowserActivity> activityClass) {
        super(activityClass);
    }

    public MusicPerformanceTest() {
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
        Intent iService =
         new Intent(mInstrumentation.getTargetContext(), MediaPlaybackService.class);
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

        // Click song tab to test in the TrackBrowserActivity
        View songTab = (View) mMusicBrowserActivity.findViewById(R.id.songtab);
        mSolo.clickOnView(songTab);
        mInstrumentation.waitForIdleSync();
        // Get TrackBrowserActivity and it's listview
        mTrackBrowserActivity = (TrackBrowserActivity)
         MusicTestUtils.getDeclaredLocalActivityManager(
                mMusicBrowserActivity, "mActivityManager").getActivity("Song");
        mListView = mTrackBrowserActivity.getListView();
        assertNotNull(mTrackBrowserActivity);
        assertNotNull(mListView);
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

    public void test00_PlaySongAndNextAndPrev() throws Exception {
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // click a listview item to play
        Log.d(TAG, ">> PlaySong");
        mSolo.clickOnView(mListView.getChildAt(0));
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        Log.d(TAG, "<< PlaySong");

        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);
        mMediaPlaybackActivity = (MediaPlaybackActivity) mSolo.getCurrentActivity();
        final View buttonPrev = mMediaPlaybackActivity.findViewById(R.id.prev);
        final View buttonNext = mMediaPlaybackActivity.findViewById(R.id.next);

        // Click on next button
        Log.d(TAG, ">> NextSong");
        mSolo.clickOnView(buttonNext);
        mInstrumentation.waitForIdleSync();
        Log.d(TAG, "<< NextSong");
        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);

        // Click on prev button
        Log.d(TAG, ">> PrevSong");
        mSolo.clickOnView(buttonPrev);
        mInstrumentation.waitForIdleSync();
        Log.d(TAG, "<< PrevSong");
        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);
    }

    public void test01_MusicAutoPerformanceTest() throws Exception {
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // 1. Music-01(The duration of open mp3 file embedded ID3 tag)
        long startTime = System.currentTimeMillis();
        mSolo.clickOnView(mListView.getChildAt(0));
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mInstrumentation.waitForIdleSync();
        long endtime = System.currentTimeMillis();
        Log.d(TAG, "MusicAutoPerformanceTest Music-01 " + (endtime - startTime));

        // 2. Music-02 Check the time of switching a music
        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);
        mMediaPlaybackActivity = (MediaPlaybackActivity) mSolo.getCurrentActivity();
        final View buttonNext = mMediaPlaybackActivity.findViewById(R.id.next);

        startTime = System.currentTimeMillis();
        mSolo.clickOnView(buttonNext);
        mInstrumentation.waitForIdleSync();
        endtime = System.currentTimeMillis();
        Log.d(TAG, "MusicAutoPerformanceTest Music-02 " + (endtime - startTime));

        // 3. Music-03 check the time of start playing a music
        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);
        mSolo.goBack();
        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);
        mInstrumentation.waitForIdleSync();
        startTime = System.currentTimeMillis();
        mSolo.clickOnView(mListView.getChildAt(0));
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mInstrumentation.waitForIdleSync();
        endtime = System.currentTimeMillis();
        Log.d(TAG, "MusicAutoPerformanceTest Music-03 " + (endtime - startTime));

        // 4. Music-07 check back time
        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);
        mSolo.goBack();
        // Click artist tab to test in the ArtistAlbumBrowserActivity
        View songTab = (View) mMusicBrowserActivity.findViewById(R.id.artisttab);
        mSolo.clickOnView(songTab);
        Thread.sleep(WAIT_FOR_NEXT_TEST_TIME);
        mInstrumentation.waitForIdleSync();
        startTime = System.currentTimeMillis();
        Log.d(TAG, "MusicAutoPerformanceTest begin Music-07");
        mSolo.goBack();
        long waitingTime = 0;
        boolean isDestroyed = mMusicBrowserActivity.isDestroyed();
        while (!isDestroyed && waitingTime < 5000) {
            isDestroyed = mMusicBrowserActivity.isDestroyed();
            mSolo.sleep(10);
            waitingTime = System.currentTimeMillis() - startTime;
            Log.v(TAG, "test01_MusicAutoPerformanceTest: isDestroyed =" + isDestroyed);
        }
        endtime = System.currentTimeMillis();
        Log.d(TAG, "MusicAutoPerformanceTest Music-07 " + (endtime - startTime));
    }

    public void switchToArtistTab() throws Exception {
        View songTab = (View) mMusicBrowserActivity.findViewById(R.id.artisttab);
        mSolo.clickOnView(songTab);
        mInstrumentation.waitForIdleSync();
    }

    public void insertFiftyAudiosToDB() throws Exception {
        try {
            int insertNum = 50;
            ContentResolver contentResolver = mMusicBrowserActivity.getContentResolver();
            ContentValues values[] = new ContentValues[insertNum];
            Random random = new Random();
            int baseYear = 1969 + random.nextInt(30);
            for (int i = 0; i < insertNum; i++) {
                ContentValues map = new ContentValues(10);
                map.put(MediaStore.MediaColumns.DATA,
                        "http://bogus/" + "test/music" + "/" + "test_music_" + (i + 1));
                map.put(MediaStore.MediaColumns.TITLE, "test music " + (i + 1));
                map.put(MediaStore.MediaColumns.MIME_TYPE, "audio/mp3");

                map.put(Audio.Media.ARTIST, "test artist " + (i + 1));
                map.put("album_artist", "test album artist");
                map.put(Audio.Media.ALBUM, "test album " + (i + 1));
                map.put(Audio.Media.TRACK, i + 1);
                map.put(Audio.Media.DURATION, 4 * 60 * 1000);
                map.put(Audio.Media.IS_MUSIC, 1);
                map.put(Audio.Media.YEAR, baseYear + random.nextInt(10));
                values[i] = map;
            }
            contentResolver.bulkInsert(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, values);
        } catch (SQLiteConstraintException ex) {
            Log.d(TAG, "insert failed", ex);
        }
    }
    public void deleteAllAudiosFromDB() throws Exception {
        ContentResolver contentResolver = mMusicBrowserActivity.getContentResolver();
        contentResolver.delete(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, null, null);
    }

    public void scanTestAudios() throws Exception {
        String[] paths = { "/storage/sdcard0/Music/test_4M_audio.mp3",
         "/storage/sdcard0/Music/test_albumart_audio.mp3"};
        MediaScannerConnection.scanFile(mContext, paths, null, null);
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

    /**
     * Makesure there are three songs in music list.
     *
     * @return If more than three song,return true,else return false
     */
    public boolean isMoreThanThreeSongs() {
        int count = -1;
        count = MusicUtils.getAllSongs(mContext).length;
        // Requery the songs number again for 6 times to makesure there are
        // really no songs in Music.
        // If after 6 times requery there are no songs but we sure there
        // are some songs in sdcard, maybe
        // the phone performance is too bad.
        for (int reQueryTimes = 6, i = 1; count <= MINIMUM_SONGS_COUNT_IN_MUSIC; i++) {
            try {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
            }
            catch (InterruptedException ex) {
                Log.e(TAG, "InterruptedException in sleep", ex);
            }
            count = MusicUtils.getAllSongs(mContext).length;
            Log.d(TAG, "isMoreThanThreeSongs: requery times = " + i + ", song count = " + count);
            if (i > reQueryTimes) {
                break;
            }
        }
        return count > MINIMUM_SONGS_COUNT_IN_MUSIC;
    }

}
