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
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.widget.ListView;


public class MusicPickerFunctionTest extends
        ActivityInstrumentationTestCase2<MusicBrowserActivity> implements ServiceConnection {

    private static final String TAG = "MusicFunctionalTest";
    private static final String TEST_PLAYLIST_NAME = "functiontest playlist";
    private static final String SONGS_LESS_THAN_TRHEE = "There are less than three songs or the" +
     " system performance is bad, please push more or reboot phone to make music pass test";
    private static final int WAIT_FOR_ACTIVITY_CREATED_TIME = 10000;
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final int WAIT_FOR_DELETED_TIME = 1500;
    private static final int WAIT_FOR_SERVICE_CONNECTED_TIME = 5000;
    private static final int MINIMUM_SONGS_COUNT_IN_MUSIC = 3;
    private static final int LONG_CLICK_TIME = 5000;

    private Instrumentation mInstrumentation = null;
    private MusicBrowserActivity mMusicBrowserActivity = null;
    private MusicPicker mMusicPicker = null;
    private ActivityMonitor mActivityMonitor = null;
    private ListView mListView = null;
    private Solo mSolo = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;

    public MusicPickerFunctionTest(Class<MusicBrowserActivity> activityClass) {
        super(activityClass);
    }

    public MusicPickerFunctionTest() {
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
        /*Intent iService = new Intent
          (mInstrumentation.getTargetContext(), MediaPlaybackService.class);
           mInstrumentation.getContext().bindService(iService, this, Context.BIND_AUTO_CREATE);
           if (!mIsMusicServiceConnected) {
            synchronized (mLock) {
                while (!mIsMusicServiceConnected) {
                    mLock.wait(WAIT_FOR_SERVICE_CONNECTED_TIME);
                }
            }
        }

        // Start MusicPicker activity
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.addCategory(Intent.CATEGORY_DEFAULT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("audio/*");
        intent.setType("application/ogg");
        intent.setType("application/x-ogg");
        mActivityMonitor = new ActivityMonitor("com.android.music.MusicPicker", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mMusicBrowserActivity.startActivityForResult(intent, -1);
        MusicPicker musicPicker = (MusicPicker) (mInstrumentation.waitForMonitorWithTimeout(
                mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        mMusicPicker = musicPicker;
        // Assert all used to be not null
        assertNotNull(mInstrumentation);
        assertNotNull(mMusicBrowserActivity);
        assertNotNull(mMusicPicker);
        assertNotNull(mContext);
        assertNotNull(mSolo);
        assertNotNull(mService);*/
        Log.d(TAG, "setUp<<<");
    }

    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "tearDown>>>");

        mSolo.finishOpenedActivities();

        /*if (null != mActivityMonitor) {
            mInstrumentation.removeMonitor(mActivityMonitor);
            mActivityMonitor = null;
        }
        try {
            mSolo.finalize();
        } catch (Throwable t) {
            t.printStackTrace();
        }
        Thread.sleep(MusicTestUtils.WAIT_TEAR_DONW_FINISH_TIME);*/
        Log.d(TAG, "tearDown<<<");
        super.tearDown();
    }

    public void test01_clickToPlayAndStop() throws Exception {
          return;
//        Log.d(TAG, ">> test01_clickToPlayAndStop");
//        // Makesure there are more than three songs in music to pass auto test
//        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
//        // Click one item to play and click again to stop
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mSolo.clickInList(0);
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mInstrumentation.waitForIdleSync();
//        mSolo.sleep(2000);
//        // Get play icon
//        //TODO assert audio is playing
//        ImageView playicon = (ImageView) mMusicPicker.getListView().
//        findViewById(R.id.play_indicator);
//        //assertEquals(View.VISIBLE, playicon.getVisibility());
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mSolo.clickInList(0);
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mInstrumentation.waitForIdleSync();
//        mSolo.sleep(1000);
//        //assertEquals(View.GONE, playicon.getVisibility());
//        Log.d(TAG, "<< test01_clickToPlayAndStop");
    }
//
//    public void test02_clickOkButton() throws Exception {
//        Log.d(TAG, ">> test02_clickOkButton");
//        // Makesure there are more than three songs in music to pass auto test
//        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
//        // Click one item to play and click again to stop
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mSolo.clickInList(0);
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mInstrumentation.waitForIdleSync();
//        mSolo.clickOnButton(mContext.getString(android.R.string.ok));
//        mInstrumentation.waitForIdleSync();
//        Intent intent = mMusicBrowserActivity.getIntent();
//        Uri uri = (Uri) intent.getData();
//        //assertNotNull(uri);
//        Log.d(TAG, "<< test02_clickOkButton");
//    }
//
//    public void test03_clickCancelButton() throws Exception {
//        Log.d(TAG, ">> test03_clickCancelButton");
//        // Makesure there are more than three songs in music to pass auto test
//        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
//        // Click one item to play and click again to stop
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mSolo.clickInList(0);
//        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
//        mInstrumentation.waitForIdleSync();
//        mSolo.clickOnButton(mContext.getString(android.R.string.cancel));
//        mInstrumentation.waitForIdleSync();
//        Intent intent = mMusicBrowserActivity.getIntent();
//        Uri uri = (Uri) intent.getData();
//        //assertNotNull(uri);
//        Log.d(TAG, "<< test03_clickCancelButton");
//    }
//
//    public void test04_sortByTrackAlbumAartist() throws Exception {
//        Log.d(TAG, ">> test04_sortByTrackAlbumAartist");
//        // Makesure there are more than three songs in music to pass auto test
//        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
//        // Track
//        mSolo.clickOnMenuItem(mContext.getString(R.string.sort_by_track));
//        mInstrumentation.waitForIdleSync();
//        // Album
//        mSolo.clickOnMenuItem(mContext.getString(R.string.sort_by_album));
//        mInstrumentation.waitForIdleSync();
//        // Artist
//        mSolo.clickOnMenuItem(mContext.getString(R.string.sort_by_artist));
//        mInstrumentation.waitForIdleSync();
//        //assertNotNull(uri);
//        Log.d(TAG, "<< test04_sortByTrackAlbumAartist");
//    }
//
//    public void test05_CallOnCreate() throws Exception {
//        Log.d(TAG, ">> test05_playComplete");
//        // Makesure there are more than three songs in music to pass auto test
//        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
//
//        // Test these method in UI thread
//        try {
//            runTestOnUiThread(new Runnable() {
//                @Override
//                public void run() {
//                    Bundle icicle = new Bundle();
//                    ListView listView = mMusicPicker.getListView();
//                    icicle.putParcelable("liststate", listView.onSaveInstanceState());
//                    icicle.putBoolean("focused", listView.hasFocus());
//                    icicle.putInt("sortMode", 1);
//                    /// M: store selected position and Drmlevel for re_create this activity @{
//                    icicle.putInt("selectedpos", 1);
//                    icicle.putInt("drmlevel", OmaDrmStore.DrmExtra.LEVEL_FL);
//                    mInstrumentation.callActivityOnSaveInstanceState(mMusicPicker, icicle);
//                    mInstrumentation.callActivityOnCreate(mMusicPicker, icicle);
//                }
//            });
//        } catch (Throwable t) {
//            Log.d(TAG, "test05_CallOnCreate with Throwable:" + t);
//        }
//
//        Log.d(TAG, "<< test05_playComplete");
//    }
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
     * @param long
     *            The member declared in activity
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
        }
        catch (RemoteException e) {
            Log.e(TAG, "Fail to remote to service " + e);
        }
        catch (InterruptedException e) {
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
        // Requery the songs number again for 6 times to makesure there
        // are really no songs in Music.
        // If after 6 times requery there are no songs but we sure
        // there are some songs in sdcard, maybe
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

    /**
     * Create a test playlist.
     *
     */
    public void createTestPlaylist() {
        deleteAllPlaylist();
        // Create a new playlist with name of TEST_PLAYLIST_NAME
        ContentValues cv = new ContentValues();
        cv.put(MediaStore.Audio.Playlists.NAME, TEST_PLAYLIST_NAME);
        Uri newPlaylistUri = mMusicBrowserActivity.getContentResolver().insert(
                MediaStore.Audio.Playlists.getContentUri("external"), cv);
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
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID, MediaStore.Audio.Playlists.NAME},
                null,
                null,
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
        if (c != null) {
            try {
                if (!c.moveToFirst()) {
                    return;
                }
                do {
                    Uri deleteUri = ContentUris.withAppendedId
                    (MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI, c.getInt(0));
                    Log.d(TAG, "deleteAllPlaylist: deleteUri = " + deleteUri + ", playlist = " +
                     c.getString(1) + "(" + c.getInt(0) + ")" + ", count = " + c.getCount());
                    mMusicBrowserActivity.getContentResolver().delete(deleteUri, null, null);
                } while (c.moveToNext());
            }
            finally {
                c.close();
                c = null;
            }
        }
    }

    /**
     * Get the track counts of playlist with given name.
     * @param playlistName
     *            The playlist name
     * @return The track counts of the playlist
     */
    public int getPlaylistTrackCounts(String playlistName) {
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID, },
                MediaStore.Audio.Playlists.NAME + "=" + "'" + playlistName + "'",
                null,
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
        c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.Members.getContentUri("external", playlistID),
                new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID },
                null,
                null,
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
     * Get the current listview count(such as using to get the search counts),we shoule requery
     * the songs number again for 6 times to makesure there are really no songs in Music.
     * If after 6 times requery there are no songs but we sure there are some songs in sdcard,
     * maybe the phone performance is too bad.
     *
     * @return The count of search result
     */
    public int getListViewCount(QueryBrowserActivity queryActivity) {
        int count = queryActivity.getListView().getCount();
        // Requery the songs number again for 6 times to makesure there are
        // really no songs in Music.
        // If after 6 times requery there are no songs but we sure there are some
        // songs in sdcard, maybe
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
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                new String[] {MediaStore.Audio.Media._ID, MediaStore.Audio.Media.DATA},
                MediaStore.Audio.Artists.ARTIST + "=?" + " OR " +
                 MediaStore.Audio.Albums.ALBUM + "=?",
                new String[] {MediaStore.UNKNOWN_STRING, MediaStore.UNKNOWN_STRING},
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
                    }
                    catch (SecurityException e) {
                        Log.e(TAG, "SecurityException in delete unknown tracks " + e);
                    }
                } while (c.moveToNext());
            }
            finally {
                c.close();
                c = null;
            }
        }
    }
}
