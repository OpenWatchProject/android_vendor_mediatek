package com.android.music.tests.functional;

import java.lang.reflect.Method;

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
import android.net.Uri;
import android.os.IBinder;
import android.provider.MediaStore;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.android.music.*;
import com.jayway.android.robotium.solo.Solo;

public class PlaylistBrowserFunctionTest extends
        ActivityInstrumentationTestCase2<MusicBrowserActivity> implements ServiceConnection {

    private static final String TAG = "MusicFunctionalTest";
    private static final String TEST_PLAYLIST_NAME = "functiontest playlist";
    private static final String TEST_PLAYLIST_NEW_NAME = "functiontest new playlist";
    private static final String SONGS_LESS_THAN_TRHEE = "There are less than three songs or the"
    + " system performance is bad, please push more or reboot phone to make music pass test";
    private static final int WAIT_FOR_ACTIVITY_CREATED_TIME = 10000;
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final int WAIT_FOR_DELETED_TIME = 1500;
    private static final int WAIT_FOR_SERVICE_CONNECTED_TIME = 5000;
    private static final int MINIMUM_SONGS_COUNT_IN_MUSIC = 3;
    private static final int LONG_CLICK_TIME = 5000;
    private static final int TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;

    private Instrumentation mInstrumentation = null;
    private MusicBrowserActivity mMusicBrowserActivity = null;
    private Solo mSolo = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;
    private ActivityMonitor mActivityMonitor = null;

    public PlaylistBrowserFunctionTest(String pkg, Class<MusicBrowserActivity> activityClass) {
        super(pkg, activityClass);
    }

    public PlaylistBrowserFunctionTest() {
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

        // Click song tab to test in the TrackBrowserActivity
        View songTab = (View) mMusicBrowserActivity.findViewById(R.id.playlisttab);
        mSolo.clickOnView(songTab);
        mInstrumentation.waitForIdleSync();
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
    public void onServiceDisconnected(ComponentName arg0) {
        Log.d(TAG, "onServiceDisconnected");
        mService = null;
        mIsMusicServiceConnected = false;
    }

    public void test01_SetAndPlayRecentlyAdded() throws Exception {
        Log.d(TAG, ">> test01_SetAndPlayRecentlyAdded");
        // Stop previous playbacks
        mService.stop();
        // Set the recent weeks to 1 in advance
        Method setIntPref = MusicUtils.class.getDeclaredMethod(
                "setIntPref", new Class[] { Context.class, String.class, int.class });
        setIntPref.setAccessible(true);
        Method getIntPref = MusicUtils.class.getDeclaredMethod(
                "getIntPref", new Class[] { Context.class, String.class, int.class });
        getIntPref.setAccessible(true);
        setIntPref.invoke(null, mInstrumentation.getTargetContext(), "numweeks", 1);

        // Long click Recently added to click context meun edit
        mSolo.clickLongOnText(mContext.getString(R.string.recentlyadded), 1, LONG_CLICK_TIME);
        mSolo.clickOnText(mContext.getString(R.string.edit_playlist_menu));
        mSolo.sendKey(Solo.DOWN);
        mSolo.clickOnButton(mContext.getString(R.string.weekpicker_set));
        mInstrumentation.waitForIdleSync();

        // check for 2 weeks recently added
        int X = ((Integer) getIntPref.invoke(mMusicBrowserActivity,
         mInstrumentation.getTargetContext(),
                "numweeks", 2)).intValue() * (3600 * 24 * 7);
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Media._ID },
                MediaStore.MediaColumns.DATE_ADDED + ">" + (System.currentTimeMillis() / 1000 - X),
                null,
                MediaStore.Audio.Media.DEFAULT_SORT_ORDER);

        int trackCount = 0;
        if (c != null) {
            try {
                trackCount = c.getCount();
            } finally {
                c.close();
                c = null;
            }

        }
        assertTrue(trackCount >= 0);

        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Play recently added
        // Long click Recently added to click context meun play
        mSolo.clickLongOnText(mContext.getString(R.string.recentlyadded), 1, LONG_CLICK_TIME);
        mSolo.clickOnText(mContext.getString(R.string.play_selection));
        // Wait for MediaPlaybackActivity
        MediaPlaybackActivity mediaPlaybackActivity  = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(mediaPlaybackActivity);
        // Check the play status
        assertTrue(MusicTestUtils.isPlaying(mService));
        // Check for now-playing list length
        assertEquals(trackCount, mService.getQueue().length);

        Log.d(TAG, "<< test01_SetAndPlayRecentlyAdded");
    }

    public void test02_RenamePlaylist() throws Exception, Throwable {
        Log.d(TAG, ">> test02_RenamePlaylist");
        // Stop playback first
        mService.stop();
        createTestPlaylist();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        mActivityMonitor =
                new ActivityMonitor("com.android.music.RenamePlaylist", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Long click test playlist to click context meun rename
        mSolo.clickLongOnText(TEST_PLAYLIST_NAME, 1, LONG_CLICK_TIME);
        mSolo.clickOnText(mContext.getString(R.string.rename_playlist_menu));
        // Wait for RenameActivity
        RenamePlaylist renamePlaylist  = (RenamePlaylist) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(renamePlaylist);
        // Clear old test name and enter new test playlist name
        mSolo.clearEditText(0);
        mSolo.enterText(0, TEST_PLAYLIST_NEW_NAME);
        mSolo.clickOnButton(mContext.getString(R.string.create_playlist_create_text));
        mInstrumentation.waitForIdleSync();
        // Check the old playlist name rename to be new name
        assertTrue(mSolo.searchText(TEST_PLAYLIST_NEW_NAME));
        assertFalse(mSolo.searchText(TEST_PLAYLIST_NAME));

        // Check for rename result
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID },
                MediaStore.Audio.Playlists.NAME + " LIKE '%" + TEST_PLAYLIST_NEW_NAME + "%'",
                null,
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);

        try {
            assertTrue(c != null && c.getCount() > 0);
        }
        catch (Exception e) {
        }
        finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }
        Log.d(TAG, "<< test02_RenamePlaylist");
    }

    public void test03_DeletePlaylist() throws Exception {
        Log.d(TAG, ">> test03_DeletePlaylist");
        // stop unnecessary playback
        mService.stop();
        createTestPlaylist();

        // Long click test playlist to click context meun delete
        mSolo.clickLongOnText(TEST_PLAYLIST_NAME, 1, LONG_CLICK_TIME);
        mSolo.clickOnText(mContext.getString(R.string.delete_playlist_menu));
        mInstrumentation.waitForIdleSync();
        // Check test playlist has been deleted
        assertFalse(mSolo.searchText(TEST_PLAYLIST_NAME));

        // Check test playlist has been deleted in DB
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID },
                MediaStore.Audio.Playlists.NAME + " LIKE '%" + TEST_PLAYLIST_NAME + "%'",
                null,
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);

        try {
            assertTrue(c == null || c.getCount() == 0);
        }
        catch (Exception e) {
        }
        finally {
            if (c != null) {
                c.close();
                c = null;
            }
        }
        Log.d(TAG, "<< test03_DeletePlaylist");
    }

    public void test04_PartyShuffle() throws Exception {
        Log.d(TAG, ">> test02_PartyShuffle");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // stop playing
        mService.stop();

        // quit party shuffle if we're already in this mode
        mService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);

        // Click option meun party shuffle
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_AUTO);
        assertTrue(MusicTestUtils.isPlaying(mService));

        // Click option meun party shuffle off
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle_off));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NONE);
        assertTrue(MusicTestUtils.isPlaying(mService));
        Log.d(TAG, "<< test02_PartyShuffle");
    }

    public void test05_SaveAsPlaylist() throws Exception {
        Log.d(TAG, ">> test05_SaveAsPlaylist");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Create a playlist with test playlist name
        createTestPlaylist();
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Click the test playlist
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        // Wait for TrackBrowserActivity
        TrackBrowserActivity trackBrowserActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackBrowserActivity);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 2);
        // Click option menu save as playlist
        mSolo.clickOnMenuItem(mContext.getString(R.string.save_as_playlist));
        // Wait for CreatePlaylist
        mSolo.waitForActivity("CreatePlaylist", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create CreatePlaylist failed!", "CreatePlaylist");
        // Clear default playlist name and enter new test playlist name
        mSolo.clearEditText(0);
        mSolo.enterText(0, TEST_PLAYLIST_NEW_NAME);
        mSolo.clickOnButton(mContext.getString(R.string.create_playlist_create_text));
        mInstrumentation.waitForIdleSync();

        // Check the save playlist exist in playlistactivity
        mSolo.goBack();
        mInstrumentation.waitForIdleSync();
        assertTrue(mSolo.searchText(TEST_PLAYLIST_NEW_NAME));
        // Check the save playlist track count
        assertEquals(getPlaylistTrackCounts(TEST_PLAYLIST_NEW_NAME),
         MusicUtils.getAllSongs(mContext).length);
        Log.d(TAG, "<< test05_SaveAsPlaylist");
    }

    public void test06_ShuffleAll() throws Exception {
        Log.d(TAG, ">> test06_ShuffleAll");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        //stop playing
        mService.stop();

        // Get number of all tracks
        int trackNum = MusicUtils.getAllSongs(mContext).length;

        // Click the test playlist
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 2);
        // Click option meun shuffle all
        mSolo.clickOnMenuItem(mContext.getString(R.string.shuffle_all));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Check service status
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NORMAL);
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");
        assertTrue(MusicTestUtils.isPlaying(mService));
        assertTrue(mService.getQueue().length == trackNum);


        Log.d(TAG, "<< test06_ShuffleAll");
    }

    public void test07_RemoveFromPlaylist() throws Exception {
        Log.d(TAG, ">> test07_RemoveFromPlaylist");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Click the test playlist
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        // Wait for TrackBrowserActivity
        TrackBrowserActivity trackBrowserActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackBrowserActivity);
        // Get number of before remove
        int trackNumBefore = MusicTestUtils.getListViewCount(trackBrowserActivity);
        // Click the test playlist
        mSolo.clickLongInList(0, 0, LONG_CLICK_TIME);
        // Click remove context option menu
        mSolo.clickOnText(mContext.getString(R.string.remove_from_playlist));
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        int trackNumAfter = MusicTestUtils.getListViewCount(trackBrowserActivity);
        assertEquals(trackNumBefore - 1, trackNumAfter);

        Log.d(TAG, "<< test07_RemoveFromPlaylist");
    }

    public void test08_UseAsRingtone() throws Exception {
        Log.d(TAG, ">> test08_UseAsRingtone");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Click the test playlist
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        // Wait for TrackBrowserActivity
        TrackBrowserActivity trackBrowserActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackBrowserActivity);
        // Click the test playlist
        mSolo.clickLongInList(0, 0, LONG_CLICK_TIME);
        // Click use as ringtone context option menu
        mSolo.clickOnText(mContext.getString(R.string.ringtone_menu));
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Wait for a while and check for settings DB again
        long setAudioId = MusicTestUtils.getDeclaredLongValue(mSolo.getCurrentActivity(),
         "mSelectedId");
        String uriString = Settings.System.getString(mMusicBrowserActivity.getContentResolver(),
                Settings.System.RINGTONE);
        assertNotNull(uriString);
        Uri uri = Uri.parse(uriString);
        String idString = uri.getLastPathSegment();
        long gotAudioId = Long.parseLong(idString);
        assertEquals(setAudioId, gotAudioId);

        Log.d(TAG, "<< test08_UseAsRingtone");
    }

    public void test09_ClickNowPlayingToPlay() throws Exception {
        Log.d(TAG, ">> test09_ClickNowPlayingToPlay");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        mService.play();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(MusicTestUtils.isPlaying(mService));
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // Check NowPlayig is visibility
        View nowPlayingView =
         (View) mMusicBrowserActivity.findViewById(R.id.nowplaying).getParent();
        assertNotNull(nowPlayingView);
        assertTrue(MusicTestUtils.checkValueStatus(nowPlayingView.getVisibility(), View.VISIBLE));
        // Get track name from nowplaying
        TextView TrackNameTextView = (TextView) nowPlayingView.findViewById(R.id.title);
        String nowPlayingTrackName = TrackNameTextView.getText().toString();
        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Click nowplaying to got to playback activity
        mSolo.clickOnView(nowPlayingView);
        // Wait for MediaPlaybackActivity
        MediaPlaybackActivity mediaPlaybackActivity  = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(mediaPlaybackActivity);
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME * 2);
        // Check whether current playing track's album is same as the selected one or different
        assertEquals(nowPlayingTrackName, mService.getTrackName());
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Clear the current playlist, check the now playing whether gone and Music has stopped
        View buttonQueue = (View) mSolo.getCurrentActivity().findViewById(R.id.curplaylist);
        mSolo.clickOnView(buttonQueue);
        // Wait for TrackBrowserActivity and check the nowplaying disappear, music will stop
        // Wait for MediaPlaybackActivity
        TrackBrowserActivity trackBrowserActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackBrowserActivity);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);
        mInstrumentation.waitForIdleSync();
        mSolo.clickOnMenuItem(mContext.getString(R.string.clear_playlist));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        long audioId = mService.getAudioId();

        assertEquals(-1, audioId);
        // assertTrue(MusicTestUtils.CheckViewStatus(nowPlayingView,
        // View.GONE));
        waitForViewDisppear(mMusicBrowserActivity, R.id.nowplaying, View.GONE);
        assertTrue(MusicTestUtils.isStopping(mService));
        Log.d(TAG, "<< test09_ClickNowPlayingToPlay");
    }

    public void test10_SwitchLandscapeAndPortrait() throws Exception {
        Log.d(TAG, ">> test10_SwitchLandscapeAndPortrait");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.play();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        View nowPlayingView = (View) mMusicBrowserActivity.findViewById(R.id.nowplaying)
                .getParent();
        assertNotNull(nowPlayingView);
        assertTrue(MusicTestUtils.CheckViewStatus(nowPlayingView, View.GONE));
        // Click the now playing tab
        View nowPlayingTab = (View) mMusicBrowserActivity.findViewById(R.id.nowplayingtab);
        mSolo.clickOnView(nowPlayingTab);
        // Wait for MediaPlaybackActivity
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");
        // Clear the current playlist, check the now playing whether gone and Music has stopped
        mInstrumentation.invokeMenuActionSync
        (mSolo.getCurrentActivity(), R.id.current_playlist_menu_item, 0);
        // Wait for TrackBrowserActivity and check the song number
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!", "TrackBrowserActivity");
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);
        mInstrumentation.waitForIdleSync();
        mSolo.clickOnMenuItem(mContext.getString(R.string.clear_playlist));
        // Check nowplaying can not press
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        nowPlayingTab = (View) mMusicBrowserActivity.findViewById(R.id.nowplayingtab);
        assertEquals(nowPlayingTab.isEnabled(), false);

        // 2.Switch to portrait
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        waitForViewDisppear(mMusicBrowserActivity, R.id.nowplaying, View.GONE);
        Log.d(TAG, "<< test10_SwitchLandscapeAndPortrait");
    }

    public void test11_SdcardStateChange() throws Exception {
        /*if (MusicFeatureOption.IS_SUPPORT_SHARED_SDCARD) {
            Log.d(TAG, "IS_SUPPORT_SHARED_SDCARD is true,skip this testcase");
            return;
        }
        Log.d(TAG, "<< test11_SdcardStateChange");
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.play();
        mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
        assertTrue(MusicTestUtils.isPlaying(mService));

        StorageManager storageManager =
         (StorageManager) mContext.getSystemService(Context.STORAGE_SERVICE);
        View errorView = mMusicBrowserActivity.findViewById(R.id.sd_icon);
        try {
            // 1.Unmounted sdcard
            storageManager.enableUsbMassStorage();
            assertTrue(MusicTestUtils.checkErrorViewState(errorView, View.VISIBLE));
            assertTrue(MusicTestUtils.isStopping(mService));
            mInstrumentation.waitForIdleSync();
            mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);

            // 2.Mounted sdcard
            storageManager.disableUsbMassStorage();
            assertTrue(!MusicTestUtils.waitForScanFinishAfterDisableUsbMassStorage(mContext));
            mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
            assertTrue(MusicTestUtils.checkErrorViewState(errorView, View.GONE));
            assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
            assertTrue(MusicTestUtils.isStopping(mService));
            // finally we need make sure sdcard has been mounted
        } finally {
            if (!MusicTestUtils.hasMountedSDcard(mContext)) {
                storageManager.disableUsbMassStorage();
                assertTrue(!MusicTestUtils.waitForScanFinishAfterDisableUsbMassStorage(mContext));
                mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
            }
        }

        Log.d(TAG, ">> test11_SdcardStateChange");*/
    }

    public void test12_Podcasts() throws Exception {
        Log.d(TAG, ">> test12_Podcasts");
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mSolo.clickOnText(mContext.getString(R.string.podcasts_title));
        TrackBrowserActivity trackActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackActivity);
        //mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        int listLength = MusicTestUtils.getListViewCount(trackActivity);
        //ListView trackList = trackActivity.getListView();
        //int listLength = trackList.getCount();
        assertTrue(listLength >= 3);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.clickInList(0);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
                                    "MediaPlaybackActivity");
        assertTrue(MusicTestUtils.isPlaying(mService));
        //set bookmark
        int[] xy = new int[2];
        ProgressBar mProgress = mSolo.getCurrentViews(ProgressBar.class).get(0);
        mProgress.getLocationOnScreen(xy);
        final int viewWidth = mProgress.getWidth();
        final int viewHeight = mProgress.getHeight();
        float x = xy[0] + (viewWidth / (float) 2.0) - 10;
        float y = xy[1] + (viewHeight / 2.0f);
        mSolo.clickOnScreen(x, y);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        //goto next
        View nextView = mSolo.getView(R.id.next);
        mSolo.clickOnView(nextView);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        //comeback check play from bookmark
        View preView = mSolo.getView(R.id.prev);
        mSolo.clickOnView(preView);
        assertTrue(MusicTestUtils.isPlaying(mService));
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        Log.d(TAG, "<< test12_Podcasts");
    }

    public void test13_DispatchKeyEvent() throws Exception, Throwable {
        /*Log.d(TAG, ">> test13_DispatchKeyEvent");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // First: test in normal playlist
        // Click the test playlist
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        // Wait for TrackBrowserActivity
        final TrackBrowserActivity trackBrowserActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackBrowserActivity);
        int oldListviewCount = MusicTestUtils.getListViewCount(trackBrowserActivity);
        Log.d(TAG, " oldListviewCount:" + oldListviewCount);
        // assertTrue(listView.requestFocus());
        sendKeys("3*DPAD_DOWN");

        // 1.KEYCODE_DPAD_UP
        KeyEvent keyEvent = new KeyEvent(0, 0, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_UP, 0,
                KeyEvent.META_ALT_ON);
        mInstrumentation.sendKeySync(keyEvent);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // 2.KEYCODE_DPAD_DOWN
        keyEvent = new KeyEvent(0, 0, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_DOWN, 0,
                KeyEvent.META_ALT_ON);
        mInstrumentation.sendKeySync(keyEvent);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // 3.KEYCODE_DEL
        final Method removeItemMethod = MusicTestUtils.getPrivateMethod(TrackBrowserActivity.class,
                "removeItem");
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                try {
                    removeItemMethod.invoke(trackBrowserActivity);
                } catch (Exception e) {
                    // TODO: handle exception
                    fail();
                }

            }
        });

        waitForListItemDeleted(oldListviewCount, trackBrowserActivity);
        ListView listView = trackBrowserActivity.getListView();
        Log.d(TAG,
                " listView.isDirty():" + listView.isDirty() + ",listView.getCount():"
                        + listView.getCount());
        Log.d(TAG, " oldListviewCount:" + oldListviewCount);
        assertEquals(oldListviewCount - 1, listView.getCount());

        // Second: test in nowplaying
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mSolo.clickInList(0);
        // Wait for MediaPlaybackActivity and goto nowplaying
        MediaPlaybackActivity mediaPlaybackActivity  = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(mediaPlaybackActivity);
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME*2);
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mSolo.clickOnView(mSolo.getView(R.id.curplaylist));
        final TrackBrowserActivity secTrackBrowserActivity  =
         (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(secTrackBrowserActivity);
        oldListviewCount = MusicTestUtils.getListViewCount(secTrackBrowserActivity);

        // 1.KEYCODE_DPAD_UP
        keyEvent = new KeyEvent(0, 0, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_UP, 0,
                KeyEvent.META_ALT_ON);
        mInstrumentation.sendKeySync(keyEvent);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // 2.KEYCODE_DPAD_DOWN
        keyEvent = new KeyEvent(0, 0, KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DPAD_DOWN, 0,
                KeyEvent.META_ALT_ON);
        mInstrumentation.sendKeySync(keyEvent);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // 3.KEYCODE_DEL
        runTestOnUiThread(new Runnable() {

            @Override
            public void run() {
                // TODO Auto-generated method stub
                try {
                    removeItemMethod.invoke(secTrackBrowserActivity);
                } catch (Exception e) {
                    // TODO: handle exception
                    fail();
                }

            }
        });

        waitForListItemDeleted(oldListviewCount, secTrackBrowserActivity);
        assertEquals(oldListviewCount - 1,
         MusicTestUtils.getListViewCount(secTrackBrowserActivity));
        Log.d(TAG, "<< test13_DispatchKeyEvent");*/
    }

    public void test14_AddFileToPlaylist() throws Exception {
        /*Log.d(TAG, ">> test14_AddFileToPlaylist");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // First: test in normal playlist
        // Click the test playlist
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        // Wait for TrackBrowserActivity
        TrackBrowserActivity trackBrowserActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackBrowserActivity);
        int oldListviewCount = MusicTestUtils.getListViewCount(trackBrowserActivity);
        String data = null;
        Cursor cursor = trackBrowserActivity.getContentResolver().query(
                ContentUris.withAppendedId(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                 MusicUtils.getCurrentAudioId()),
                new String[] { MediaStore.Audio.Media.DATA },
                null,
                null,
                null);
        try {
            if (null != cursor && cursor.moveToFirst()) {
                data = cursor.getString(0);
            }
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }
        if (data == null) {
            return;
        }
        Intent intent = new Intent();
        intent.setData(Uri.parse(data));
        MusicTestUtils.invokeDeclaredMethod(trackBrowserActivity, "onActivityResult",
                new Class[] {int.class, int.class, Intent.class},
                new Object[] {MusicUtils.Defs.CHILD_MENU_BASE + 7, Activity.RESULT_OK, intent});
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);
        assertEquals(oldListviewCount + 1, MusicTestUtils.getListViewCount(trackBrowserActivity));
        Log.d(TAG, "<< test14_AddFileToPlaylist");*/
    }

    public void test14_ClearPlaylist() throws Exception {
        Log.d(TAG, ">> test14_ClearPlaylist");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));

        MusicTestUtils.createTestPlaylist(mMusicBrowserActivity);
        mActivityMonitor =
                new ActivityMonitor("com.android.music.TrackBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Click the test playlist
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        // Wait for TrackBrowserActivity
        TrackBrowserActivity trackBrowserActivity  = (TrackBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(trackBrowserActivity);

        String playlist = MusicTestUtils.getDeclaredStringValue(trackBrowserActivity, "mPlaylist");
        MusicUtils.clearPlaylist(trackBrowserActivity, Integer.parseInt(playlist));
        assertTrue(mSolo.searchText(mContext.getString(R.string.no_music_title)));
        Log.d(TAG, "<< test14_ClearPlaylist");
    }

    public void test15_PlayOnePlaylist() throws Exception {
        /*Log.d(TAG, ">> test15_PlayOnePlaylist");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, MusicTestUtils.isMoreThanThreeSongs(mContext));
        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // 1. play recently added playlist
        mSolo.clickLongOnText(mContext.getString(R.string.recentlyadded), 0, LONG_CLICK_TIME);
        mSolo.clickOnText(mContext.getString(R.string.play_selection));
        MediaPlaybackActivity mediaPlaybackActivity  = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(mediaPlaybackActivity);
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 2. play podcasts playlist
        mService.pause();
        mSolo.goBack();
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 8);
        mSolo.clickLongOnText(mContext.getString(R.string.podcasts_listitem), 0, LONG_CLICK_TIME);
        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mSolo.clickOnText(mContext.getString(R.string.play_selection));
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mediaPlaybackActivity  = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(mediaPlaybackActivity);
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 4. play normal playlist
        mService.pause();
        mSolo.goBack();
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 8);
        MusicTestUtils.createTestPlaylist(mMusicBrowserActivity);
        mSolo.sleep(TIME_OUT);
        mInstrumentation.waitForIdleSync();
        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        mSolo.clickLongOnText(TEST_PLAYLIST_NAME, 0, LONG_CLICK_TIME);
        mSolo.clickOnText(mContext.getString(R.string.play_selection));
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mediaPlaybackActivity  = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(mediaPlaybackActivity);
        assertTrue(MusicTestUtils.isPlaying(mService));*/
    }

    /**
     * Makesure there are three songs in music list.
     *
     * @return If more than three song,return true,else return false
     */
    public boolean isMoreThanThreeSongs() {
        int count = -1;
        count = MusicUtils.getAllSongs(mContext).length;
        // Requery the songs number again for 6 times
        // to makesure there are really no songs in Music.
        // If after 6 times requery there are no songs but
        // we sure there are some songs in sdcard, maybe
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
     * wait for Veiw Dissapear
     */
    private void waitForViewDisppear(Activity mActivity, int viewId, int visibleState)
            throws InterruptedException {
        Log.d(TAG, "waitForViewDisppear() entry with type is " + viewId);
        View view = (View) mActivity.findViewById(viewId);
        assertNotNull(view);
        long startTime = System.currentTimeMillis();
        Log.d(TAG, "before enter loop,view.getVisibility()== " + view.getVisibility());
        while (view.getVisibility() != visibleState) {
            Log.d(TAG, "view.getVisibility()== " + view.getVisibility());
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForViewDisppear() timeout");
            }
            Thread.sleep(SLEEP_TIME);
            view = (View) mActivity.findViewById(viewId);
        }
        Log.d(TAG, "waitForViewDisppear() exit with view is " + view);
    }

    /**
     * wait for list item is deleted
     */
    private void waitForListItemDeleted(int oldCount, TrackBrowserActivity mActivity)
            throws InterruptedException {
        Log.d(TAG, "waitForListItemDeleted() entry,with oldCount:" + oldCount);
        long startTime = System.currentTimeMillis();
        ListView listView = mActivity.getListView();
        int newCount = listView.getCount();
        while (oldCount - 1 != newCount) {
            Log.d(TAG, "newCount== " + newCount);
            if (System.currentTimeMillis() - startTime > 10000) {
                fail("waitForListItemDeleted() timeout");
            }
            Thread.sleep(SLEEP_TIME);
            listView = mActivity.getListView();
            newCount = listView.getCount();
        }
        Log.d(TAG, "waitForListItemDeleted() exit");
    }

}
