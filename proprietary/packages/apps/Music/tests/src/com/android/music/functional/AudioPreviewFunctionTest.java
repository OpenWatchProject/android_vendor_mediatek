package com.android.music.tests.functional;

import java.io.File;
import java.lang.reflect.Field;

import com.android.music.*;
import com.jayway.android.robotium.solo.Solo;

import android.app.Activity;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.content.ComponentName;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.SeekBar;

public class AudioPreviewFunctionTest extends
 ActivityInstrumentationTestCase2<MusicBrowserActivity> implements
        ServiceConnection {

    private static final String TAG = "MusicFunctionalTest";
    private static final String TEST_PLAYLIST_NAME = "functiontest playlist";
    private static final String SONGS_LESS_THAN_TRHEE =
            "There are less than three songs or the system performance is bad, please push" +
             " more or reboot phone to make music pass test";
    private static final int WAIT_FOR_ACTIVITY_CREATED_TIME = 10000;
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final int WAIT_FOR_DELETED_TIME = 1500;
    private static final int WAIT_FOR_SERVICE_CONNECTED_TIME = 5000;
    private static final int MINIMUM_SONGS_COUNT_IN_MUSIC = 3;
    private static final int LONG_CLICK_TIME = 5000;

    private Instrumentation mInstrumentation = null;
    private MusicBrowserActivity mMusicBrowserActivity = null;
    private AudioPreview mAudioPreview = null;
    private ActivityMonitor mActivityMonitor = null;
    private ListView mListView = null;
    private Solo mSolo = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;

    public AudioPreviewFunctionTest(Class<MusicBrowserActivity> activityClass) {
        super(activityClass);
    }

    public AudioPreviewFunctionTest() {
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
        mService.play();
        assertTrue(isPlaying());
        // Start MusicPicker activity
        Intent intent = new Intent();
        intent.setClass(mMusicBrowserActivity, AudioPreviewStarter.class);
        intent.setData(ContentUris.withAppendedId(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                MusicUtils.getCurrentAudioId()));

        mActivityMonitor = new ActivityMonitor("com.android.music.AudioPreview", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mMusicBrowserActivity.startActivity(intent);
        AudioPreview audioPreview =
                (AudioPreview) (mInstrumentation.waitForMonitorWithTimeout
                (mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        mAudioPreview = audioPreview;
        // Assert all used to be not null
        assertNotNull(mInstrumentation);
        assertNotNull(mMusicBrowserActivity);
        assertNotNull(mAudioPreview);
        assertNotNull(mContext);
        assertNotNull(mSolo);
        assertNotNull(mService);
        Log.d(TAG, "setUp<<<");
    }

    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "tearDown>>>");

        mSolo.finishOpenedActivities();

        if (null != mActivityMonitor) {
            mInstrumentation.removeMonitor(mActivityMonitor);
            mActivityMonitor = null;
        }
        try {
            mSolo.finalize();
        } catch (Throwable t) {
            t.printStackTrace();
        }
        Thread.sleep(MusicTestUtils.WAIT_TEAR_DONW_FINISH_TIME);
        Log.d(TAG, "tearDown<<<");
        super.tearDown();
    }

    public void test01_playAndStop() throws Exception {
        Log.d(TAG, ">> test01_playAndStop");
        // Makesure there are more than three songs in music to pass auto test
        mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
        assertFalse(isPlaying());
        ImageButton playStopButton = (ImageButton) mAudioPreview.findViewById(R.id.playpause);
        mSolo.clickOnView(playStopButton);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // assertTrue(isPlaying());
        mSolo.clickOnView(playStopButton);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // assertFalse(isPlaying());
        Log.d(TAG, "<< test01_playAndStop");
    }

    public void test02_AudioFocusChange() throws Exception {
        Log.d(TAG, ">> test02_AudioFocusChange");
        mService.pause();
        assertTrue(MusicTestUtils.isStopping(mService));

        // 1.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK
        AudioManager audioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        audioManager.requestAudioFocus(null,
         AudioManager.STREAM_RING, AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        audioManager.abandonAudioFocus(null);
        // TODO assert AudioPreview stop

        // 2.AUDIOFOCUS_LOSS_TRANSIENT
        audioManager.requestAudioFocus(null,
         AudioManager.STREAM_RING, AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        // TODO assert AudioPreview stop
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        audioManager.abandonAudioFocus(null);

        // 3.AUDIOFOCUS_LOSS
        audioManager.requestAudioFocus(null,
         AudioManager.STREAM_RING, AudioManager.AUDIOFOCUS_GAIN);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // TODO assert AudioPreview stop
        audioManager.abandonAudioFocus(null);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
    }

    public void test03_ProgressBarSeek() throws Exception {
        Log.d(TAG, ">> test03_ProgressBarSeek");
        int[] xy = new int[2];
        SeekBar progress = (SeekBar) mAudioPreview.findViewById(R.id.progress);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        progress.getLocationOnScreen(xy);
        final int viewWidth = progress.getWidth();
        final int viewHeight = progress.getHeight();
        float x = 3.0f;
        for (int i = 3; i >= 1; i--) {
            x = xy[0] + (viewWidth / (float) i) - 10;
            float y = xy[1] + (viewHeight / 2.0f);
            mSolo.clickOnScreen(x, y);
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        }
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        Log.d(TAG, ">> test03_ProgressBarSeek");
    }

    public void test04_onKeyDown() throws Exception {
        Log.d(TAG, ">> test04_onKeyDown");
        mService.pause();
        assertTrue(MusicTestUtils.isStopping(mService));

        // 1.KEYCODE_HEADSETHOOK and KEYCODE_MEDIA_PLAY_PAUSE
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_HEADSETHOOK);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);

        // 2.KEYCODE_MEDIA_PLAY
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PLAY);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);

        // 3.KEYCODE_MEDIA_PAUSE
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PAUSE);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);

        // 4.KEYCODE_MEDIA_FAST_FORWARD, KEYCODE_MEDIA_NEXT, KEYCODE_MEDIA_PREVIOUS,
        // KEYCODE_MEDIA_REWIND
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_FAST_FORWARD);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_NEXT);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PREVIOUS);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_REWIND);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);

        // 4.KEYCODE_MEDIA_STOP, KEYCODE_BACK
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_STOP);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        Log.d(TAG, "<< test04_onKeyDown");
    }

    public void test05_CheckOptionMenu() throws Exception {
        Log.d(TAG, ">> test05_CheckOptionMenu");
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MENU);
        assertTrue(MusicTestUtils.isPlaying(mService));
        Log.d(TAG, "<< test05_CheckOptionMenu");
    }

    public void test06_onInfo() throws Exception {
        Log.d(TAG, ">> test05_CheckOptionMenu");
        // 1.check normal info message
        mAudioPreview.onInfo(null, 0, 0);

        // 2.MediaPlayer.MEDIA_INFO_AUDIO_NOT_SUPPORTED
        mAudioPreview.onInfo(null, MediaPlayer.MEDIA_INFO_AUDIO_NOT_SUPPORTED, 0);
        assertTrue(MusicTestUtils.isPlaying(mService));
        Log.d(TAG, "<< test05_CheckOptionMenu");
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
     * Get long value by string which is declared in activity.
     *
     * @param activity
     *            activity
     * @param long The member declared in activity
     * @return The long value of member declared in activity
     */
    public long getDeclaredLongValue(Activity activity, String string) {
        try {
            Field field = activity.getClass().getDeclaredField(string.toString());
            field.setAccessible(true);
            String fieldString = field.get(activity).toString();
            return Long.parseLong(fieldString);
        } catch (NoSuchFieldException ex) {
            Log.e(TAG, "No such field in runTestOnUiThread:", ex);
            return -1;
        } catch (IllegalAccessException ex) {
            Log.e(TAG, "Illegal access exception in runTestOnUiThread:", ex);
            return -1;
        } catch (IllegalArgumentException ex) {
            Log.e(TAG, "Illegal argument exception in runTestOnUiThread:", ex);
            return -1;
        }
    }

    /**
     * Check whether music is playing in 5000 ms.
     *
     * @return If music is playing,return true,else return false
     */
    public boolean isPlaying() {
        long startTime = System.currentTimeMillis();
        long waitTime = 5000;
        boolean isTimeOut = false;
        try {
            while (!mService.isPlaying()) {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
                if (System.currentTimeMillis() - startTime > waitTime) {
                    isTimeOut = true;
                    break;
                }
            }
        } catch (RemoteException e) {
            Log.e(TAG, "Fail to remote to service " + e);
        } catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException " + e);
        }

        return !isTimeOut;
    }

    /**
     * Makesure there are three songs in music list.
     *
     * @return If more than three song,return true,else return false
     */
    public boolean isMoreThanThreeSongs() {
        int count = -1;
        count = MusicUtils.getAllSongs(mContext).length;
        // Requery the songs number again for 6 times to makesure there are really no songs in
        // Music.
        // If after 6 times requery there are no songs but we sure there are some songs in sdcard,
        // maybe
        // the phone performance is too bad.
        for (int reQueryTimes = 6, i = 1; count <= MINIMUM_SONGS_COUNT_IN_MUSIC; i++) {
            try {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
            } catch (InterruptedException ex) {
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

    /**
     * Create a test playlist.
     *
     */
    public void createTestPlaylist() {
        deleteAllPlaylist();
        // Create a new playlist with name of TEST_PLAYLIST_NAME
        ContentValues cv = new ContentValues();
        cv.put(MediaStore.Audio.Playlists.NAME, TEST_PLAYLIST_NAME);
        Uri newPlaylistUri =
                mMusicBrowserActivity.getContentResolver().
                insert(MediaStore.Audio.Playlists.getContentUri("external"), cv);
        assertTrue(newPlaylistUri != null);
        // Add all songs to test playlist
        String idString = newPlaylistUri.getLastPathSegment();
        long newPlaylistId = Long.parseLong(idString);
        MusicUtils.addToPlaylist(mContext, MusicUtils.getAllSongs(mContext), newPlaylistId);
        mInstrumentation.waitForIdleSync();

    }

    /**
     * Delete a test playlist.
     *
     */
    public void deleteAllPlaylist() {
        Cursor c =
                mMusicBrowserActivity.getContentResolver().query
                (MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                        new String[] { MediaStore.Audio.Playlists._ID,
                         MediaStore.Audio.Playlists.NAME }, null, null,
                        MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
        if (c != null) {
            try {
                if (!c.moveToFirst()) {
                    return;
                }
                do {
                    Uri deleteUri = ContentUris.withAppendedId
                    (MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI, c.getInt(0));
                    Log.d(TAG,
                            "deleteAllPlaylist: deleteUri = " + deleteUri +
                             ", playlist = " + c.getString(1) + "("
                                    + c.getInt(0) + ")" + ", count = " + c.getCount());
                    mMusicBrowserActivity.getContentResolver().delete(deleteUri, null, null);
                } while (c.moveToNext());
            } finally {
                c.close();
                c = null;
            }
        }
    }

    /**
     * Get the track counts of playlist with given name.
     *
     * @param playlistName
     *            The playlist name
     * @return The track counts of the playlist
     */
    public int getPlaylistTrackCounts(String playlistName) {
        Cursor c =
                mMusicBrowserActivity.getContentResolver().query
                (MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                        new String[] { MediaStore.Audio.Playlists._ID, },
                        MediaStore.Audio.Playlists.NAME + "=" + "'" + playlistName + "'", null,
                        MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
        int playlistID = -1;
        if (c != null) {
            try {
                c.moveToFirst();
                playlistID = c.getInt(0);
            } catch (Exception e) {
                fail("cannot get valid playlist ID from Media DB!");
            } finally {
                c.close();
                c = null;
            }
        }
        c =
                mMusicBrowserActivity.getContentResolver().query(
                        MediaStore.Audio.Playlists.Members.getContentUri("external", playlistID),
                        new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID }, null, null,
                        MediaStore.Audio.Playlists.Members.DEFAULT_SORT_ORDER);
        if (c != null) {
            try {
                return c.getCount();
            } catch (Exception e) {
                fail("cannot get valid audio ID from Media DB!");
            } finally {
                c.close();
                c = null;
            }
        }
        return -1;
    }

    /**
     * Get the current listview count(such as using to get the search counts),we shoule requery the
     * songs number again for 6 times to makesure there are really no songs in Music. If after 6
     * times requery there are no songs but we sure there are some songs in sdcard, maybe the phone
     * performance is too bad.
     *
     * @return The count of search result
     */
    public int getListViewCount(QueryBrowserActivity queryActivity) {
        int count = queryActivity.getListView().getCount();
        // Requery the songs number again for 6 times to makesure there are really no songs in
        // Music.
        // If after 6 times requery there are no songs but we sure there are some songs in sdcard,
        // maybe
        // the phone performance is too bad.
        for (int reQueryTimes = 6, i = 1; count <= 0; i++) {
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
            count = queryActivity.getListView().getCount();
            Log.d(TAG, "getListViewCount: query times = " + i + ", song count = " + count);
            if (i > reQueryTimes) {
                break;
            }
        }
        return count;
    }

    /**
     * Delete all unknown artist and album tracks.
     *
     */
    public void deleteAllUnknownTracks() {
        Cursor c =
                mMusicBrowserActivity.getContentResolver().query
                (MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                        new String[] { MediaStore.Audio.Media._ID, MediaStore.Audio.Media.DATA },
                        MediaStore.Audio.Artists.ARTIST + "=?" + " OR " +
                         MediaStore.Audio.Albums.ALBUM + "=?",
                        new String[] { MediaStore.UNKNOWN_STRING, MediaStore.UNKNOWN_STRING },
                        MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
        if (c != null) {
            try {
                c.moveToFirst();
                do {
                    Uri deleteUri = ContentUris.withAppendedId
                    (MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, c.getInt(0));
                    mMusicBrowserActivity.getContentResolver().delete(deleteUri, null, null);
                    try {
                        String filePath = c.getString(1);
                        File file = new File(filePath);
                        file.delete();
                    } catch (SecurityException e) {
                        Log.e(TAG, "SecurityException in delete unknown tracks " + e);
                    }
                } while (c.moveToNext());
            } finally {
                c.close();
                c = null;
            }
        }
    }
}
