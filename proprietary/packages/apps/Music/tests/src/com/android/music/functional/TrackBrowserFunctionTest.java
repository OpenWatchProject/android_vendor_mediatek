package com.android.music.tests.functional;

import java.io.File;
import java.lang.reflect.Field;


import com.android.music.*;
import com.jayway.android.robotium.solo.Solo;

import android.app.Activity;
import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.app.LocalActivityManager;
import android.content.ComponentName;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.database.Cursor;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.View;
import android.widget.ListView;
import android.widget.TextView;

public class TrackBrowserFunctionTest extends
        ActivityInstrumentationTestCase2<MusicBrowserActivity> implements ServiceConnection {

    private static final String TAG = "MusicFunctionalTest";
    private static final String TEST_PLAYLIST_NAME = "functiontest playlist";
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
    private TrackBrowserActivity mTrackBrowserActivity = null;
    private ActivityMonitor mActivityMonitor = null;
    private ListView mListView = null;
    private Solo mSolo = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;

    public TrackBrowserFunctionTest(Class<MusicBrowserActivity> activityClass) {
        super(activityClass);
    }

    public TrackBrowserFunctionTest() {
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
        assertNotNull(mInstrumentation);
        assertNotNull(mMusicBrowserActivity);
        // Bind service
        Intent iService = new
         Intent(mInstrumentation.getTargetContext(), MediaPlaybackService.class);
        Log.d(TAG, "bindservice>>>");
        mInstrumentation.getContext().bindService(iService, this, Context.BIND_AUTO_CREATE);
        Log.d(TAG, "bindservice<<<");
        if (!mIsMusicServiceConnected) {
            synchronized (mLock) {
                while (!mIsMusicServiceConnected) {
                    mLock.wait(WAIT_FOR_SERVICE_CONNECTED_TIME);
                }
            }
        }

        //waitForServiceConnected();
        // Assert all used to be not null

        assertNotNull(mContext);
        assertNotNull(mSolo);
        assertNotNull(mService);

        // Click song tab to test in the TrackBrowserActivity
        View songTab = (View) mMusicBrowserActivity.findViewById(R.id.songtab);
        mSolo.clickOnView(songTab);
        mInstrumentation.waitForIdleSync();
        // Get TrackBrowserActivity and it's listview
        mTrackBrowserActivity = (TrackBrowserActivity)
         getDeclaredLocalActivityManager("mActivityManager").getActivity("Song");
        mListView = mTrackBrowserActivity.getListView();
        assertNotNull(mTrackBrowserActivity);
        assertNotNull(mListView);
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
        mInstrumentation.waitForIdleSync();
        Thread.sleep(MusicTestUtils.WAIT_TEAR_DONW_FINISH_TIME);
        Log.d(TAG, "tearDown<<<");
        super.tearDown();
    }

    public void test01_PlayAll() throws Exception {
        Log.d(TAG, ">> test01_PlayAll");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // Stop playback first
        mService.stop();

        // Click option meun play all
        mSolo.clickOnMenuItem(mContext.getString(R.string.play_all));
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");

        // Check the play status
        assertTrue(isPlaying());
        Log.d(TAG, "<< test01_PlayAll");
    }

    public void test02_PartyShuffle() throws Exception {
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
        assertTrue(isPlaying());

        // Click option meun party shuffle off
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle_off));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NONE);
        assertTrue(isPlaying());
        Log.d(TAG, "<< test02_PartyShuffle");
    }

    public void test03_ShuffleAll() throws Exception {
        Log.d(TAG, ">> test03_ShuffleAll");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // stop playing
        mService.stop();

        // Get number of all tracks
        int trackNum = MusicUtils.getAllSongs(mContext).length;

        // Click option meun shuffle all
        mSolo.clickOnMenuItem(mContext.getString(R.string.shuffle_all));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Check service status
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NORMAL);
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");
        assertTrue(isPlaying());
        assertTrue(mService.getQueue().length == trackNum);

        Log.d(TAG, "<< test03_ShuffleAll");
    }

    public void test04_PlaySelection() throws Exception {
        Log.d(TAG, ">> test04_PlaySelection");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.stop();
        assertFalse(isPlaying());
        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context option meun play
        mSolo.clickOnText(mContext.getString(R.string.play_selection));
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");

        // Check the play status
        assertTrue(isPlaying());

        // Check if current playing track is the selected
        long selectedAudioId = getDeclaredLongValue(mTrackBrowserActivity, "mSelectedId");
        long currentAudioId = mService.getAudioId();
        assertEquals(selectedAudioId, currentAudioId);

        Log.d(TAG, "<< test04_PlaySelection");
    }

    public void test05_AddToCurrentPlaylist() throws Exception {
        Log.d(TAG, ">> test05_AddToCurrentPlaylist");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // Stop playback
        mService.stop();
        // Clear playing queue
        mService.removeTracks(0, Integer.MAX_VALUE);
        mInstrumentation.waitForIdleSync();

        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context menu add to current playlist
        mSolo.clickOnText(mContext.getString(R.string.add_to_playlist));
        mSolo.clickOnText(mContext.getString(R.string.queue));
        mInstrumentation.waitForIdleSync();

        // Check the play status
        assertTrue(isPlaying());

        // Check if current playing track is the selected
        long selectedAudioId = getDeclaredLongValue(mTrackBrowserActivity, "mSelectedId");
        long currentAudioId = mService.getAudioId();
        assertEquals(selectedAudioId, currentAudioId);

        Log.d(TAG, "<< test05_AddToCurrentPlaylist");
    }

    public void test06_AddToNewPlaylist() throws Throwable {
        Log.d(TAG, ">> test06_AddToNewPlaylist");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        deleteAllPlaylist();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context menu add to new playlist
        mSolo.clickOnText(mContext.getString(R.string.add_to_playlist));
        mSolo.clickOnText(mContext.getString(R.string.new_playlist));
        mSolo.waitForActivity("CreatePlaylist", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create CreatePlaylist failed!", "CreatePlaylist");
        // Clear default playlist name and enter test playlist name
        mSolo.clearEditText(0);
        mSolo.enterText(0, TEST_PLAYLIST_NAME);
        mSolo.clickOnButton(mContext.getString(R.string.create_playlist_create_text));
        mInstrumentation.waitForIdleSync();

        // Check for playlist insertion and track insertion
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID, MediaStore.Audio.Playlists.NAME },
                MediaStore.Audio.Playlists.NAME + " LIKE '%" + TEST_PLAYLIST_NAME + "%'",
                null,
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);

        int newListId = -1;
        if (c != null) {
            try {
                assertTrue(c.moveToFirst());
                newListId = c.getInt(0);
            } catch (Exception e) {
                fail("cannot get valid playlist ID from Media DB!");
            } finally {
                c.close();
                c = null;
            }
        }
        // Check the track insert in test playlist
        long selectedAudioId = getDeclaredLongValue(mTrackBrowserActivity, "mSelectedId");
        c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.Members.getContentUri("external", newListId),
                new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID },
                null,
                null,
                MediaStore.Audio.Playlists.Members.DEFAULT_SORT_ORDER);
        assertNotNull(c);
        if (c != null) {
            try {
                assertTrue(c.moveToFirst());
                assertEquals(c.getLong(0), selectedAudioId);
            } catch (Exception e) {
                fail("cannot get valid audio ID from Media DB!");
            } finally {
                c.close();
                c = null;
            }
        }

        Log.d(TAG, "<< test06_AddToNewPlaylist");
    }

    public void test07_addToExistPlaylist() throws Exception {
        Log.d(TAG, ">> test07_addToExistPlaylist");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Delete all playlist in DB
        deleteAllPlaylist();

        // Create a new playlist with name of TEST_PLAYLIST_NAME and get playlist ID
        ContentValues cv = new ContentValues();
        cv.put(MediaStore.Audio.Playlists.NAME, TEST_PLAYLIST_NAME);
        Uri newPlaylistUri = mMusicBrowserActivity.getContentResolver().insert(
                        MediaStore.Audio.Playlists.getContentUri("external"), cv);
        assertTrue(newPlaylistUri != null);
        String idString = newPlaylistUri.getLastPathSegment();
        long newPlaylistId = Long.parseLong(idString);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context menu add to test playlist
        mSolo.clickOnText(mContext.getString(R.string.add_to_playlist));
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Check the seleted audio whether exist in the test playlist
        long selectedAudioId = getDeclaredLongValue(mTrackBrowserActivity, "mSelectedId");
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.Members.getContentUri("external", newPlaylistId),
                new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID },
                null,
                null,
                MediaStore.Audio.Playlists.Members.DEFAULT_SORT_ORDER);
        assertNotNull(c);
        if (c != null) {
            try {
                assertTrue(c.moveToFirst());
                assertEquals(c.getLong(0), selectedAudioId);
            } catch (Exception e) {
                fail("cannot get valid audio ID from Media DB!");
            } finally {
                c.close();
                c = null;
            }
        }

        Log.d(TAG, "<< test07_addToExistPlaylist");
    }

    public void test08_UseAsRingtone() throws Exception {
        Log.d(TAG, ">> test08_UseAsRingtone");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context meun add to use as ringtone
        mSolo.clickOnText(mContext.getString(R.string.ringtone_menu));
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Wait for a while and check for settings DB again
        long setAudioId = getDeclaredLongValue(mTrackBrowserActivity, "mSelectedId");
        String uriString = Settings.System.getString(mMusicBrowserActivity.getContentResolver(),
                Settings.System.RINGTONE);
        assertNotNull(uriString);
        Uri uri = Uri.parse(uriString);
        String idString = uri.getLastPathSegment();
        long gotAudioId = Long.parseLong(idString);
        assertEquals(setAudioId, gotAudioId);

        Log.d(TAG, "<< test08_UseAsRingtone");
    }

    public void test09_DeleteItem() throws Exception {
        Log.d(TAG, ">> test08_DeleteItem");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context meun add to delete
        mSolo.clickOnText(mContext.getString(R.string.delete_item));

        long deletedId = getDeclaredLongValue(mTrackBrowserActivity, "mSelectedId");
        String deletedPath = null;
        // get the actual file path of the deleting item
        Cursor c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Media.DATA },
                MediaStore.Audio.Media._ID + "=" + deletedId,
                null,
                MediaStore.Audio.Media.DEFAULT_SORT_ORDER);
        if (c != null) {
            try {
                assertTrue(c.moveToFirst());
                deletedPath = c.getString(0);
            } catch (Exception e) {
                fail("cannot get valid path info from Media DB!");
            } finally {
                c.close();
                c = null;
            }
        }

        mSolo.waitForActivity("DeleteItems", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create DeleteItems failed!", "DeleteItems");
        mSolo.clickOnButton(mContext.getString(R.string.delete_confirm_button_text));
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Check Media DB for deleted item
        c = mMusicBrowserActivity.getContentResolver().query(
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Media._ID },
                MediaStore.Audio.Media._ID + "=" + deletedId,
                null,
                MediaStore.Audio.Media.DEFAULT_SORT_ORDER);
        try {
            assertTrue(c == null || c.getCount() == 0);
        } catch (Exception e) {
            fail("delete item: check DB failed!");
        } finally {
            if (c != null) {
                c.close();
            }
        }

        // Check current playlist for deleted item
        if (mService != null) {
            long[] playingList = mService.getQueue();
            boolean findInQueue = false;
            for (long audioId : playingList) {
                if (audioId == deletedId) {
                    findInQueue = true;
                    break;
                }
            }
            assertFalse(findInQueue);
        }

        // check file system for deleted item
        if (deletedPath != null) {
            File f = new File(deletedPath);
            assertFalse(f.exists());
        }

        Log.d(TAG, "<< test09_DeleteItem");
    }

    public void test10_Search() throws Exception {
        /*Log.d(TAG, ">> test10_Search");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context option add to search
        mSolo.clickOnText(mContext.getString(R.string.search_title));
        // Click Music to search in music app
        mSolo.clickOnText(mContext.getString(R.string.musicbrowserlabel));
        mActivityMonitor =
            new ActivityMonitor("com.android.music.QueryBrowserActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        QueryBrowserActivity queryActivity = (QueryBrowserActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        StorageManager storageManager =
         (StorageManager) mContext.getSystemService(Context.STORAGE_SERVICE);
        try {
            assertNotNull(queryActivity);
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
            // 1. Check the search result
            int searchCount = getListViewCount();
            assertTrue(searchCount > 0);
            if (MusicFeatureOption.IS_SUPPORT_SHARED_SDCARD == false) {
                // 2. QueryBrowserActivity error of sdcard unmounted
                View errorView = queryActivity.findViewById(R.id.sd_icon);
                // a.Unmounted sdcard
                storageManager.enableUsbMassStorage();
                mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
                assertTrue(MusicTestUtils.checkErrorViewState(errorView, View.VISIBLE));
                // b.Mounted sdcard
                storageManager.disableUsbMassStorage();
                assertTrue(!MusicTestUtils.waitForScanFinishAfterDisableUsbMassStorage(mContext));
                mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
                assertTrue(MusicTestUtils.checkErrorViewState(errorView, View.GONE));
                assertEquals(0, getListViewCount());
            }
        } finally {
            if (queryActivity != null) {
                queryActivity.finish();
                queryActivity = null;
            }
            // finally we need make sure sdcard has been mounted
            if (!MusicTestUtils.hasMountedSDcard(mContext)) {
                storageManager.disableUsbMassStorage();
                assertTrue(!MusicTestUtils.waitForScanFinishAfterDisableUsbMassStorage(mContext));
                mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
            }
        }
        Log.d(TAG, "<< test10_Search");*/
    }

    public void test11_ClickListviewItemToPlay() throws Exception {
        Log.d(TAG, ">> test11_ClickListviewItemToPlay");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.stop();
        assertFalse(isPlaying());
        View view = mListView.getChildAt(0);
        assertNotNull(view);
        // Get album name from listview
        TextView albumTextView = (TextView) view.findViewById(R.id.line1);
        String selectTrackName = albumTextView.getText().toString();
        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Cick a listview item to open context menu
        mSolo.clickOnView(view);
        Log.d(TAG, ">> ClickListviewItemToPlay start sleep");
        //mSolo.sleep(TIME_OUT);
        Log.d(TAG, "<< ClickListviewItemToPlay end sleep");
        MediaPlaybackActivity playbackActivity = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        try {
            assertNotNull(playbackActivity);
            // Check music is still playing
            assertTrue(isPlaying());
            // Check if current playing track's album is same as the selected one
            assertEquals(selectTrackName, mService.getTrackName());
            mSolo.sleep(TIME_OUT);
        }
        finally {
            if (null != playbackActivity) {
                playbackActivity.finish();
                playbackActivity = null;
            }
        }

        Log.d(TAG, "<< test11_ClickListviewItemToPlay");
    }
    public void test12_ClickNowPlayingToPlay() throws Exception {
        /*Log.d(TAG, ">> test12_ClickNowPlayingToPlay");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // 1.test click nowplaying
        // Click option meun play all
        mSolo.clickOnMenuItem(mContext.getString(R.string.play_all));
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_BACK);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // Check the play status
        assertTrue(isPlaying());
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // Check NowPlayig is visibility
        View nowPlayingView =
        (View) mMusicBrowserActivity.findViewById(R.id.nowplaying).getParent();
        assertNotNull(nowPlayingView);
        assertTrue(MusicTestUtils.checkValueStatus(nowPlayingView.getVisibility(),View.VISIBLE));
        // Get track name from nowplaying
        TextView TrackNameTextView = (TextView) nowPlayingView.findViewById(R.id.title);
        String nowPlayingTrackName = TrackNameTextView.getText().toString();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mActivityMonitor =
                new ActivityMonitor("com.android.music.MediaPlaybackActivity", null, false);
        mInstrumentation.addMonitor(mActivityMonitor);
        // Click nowplaying to got to playback activity
        mSolo.clickOnView(nowPlayingView);
        mSolo.sleep(TIME_OUT);
        MediaPlaybackActivity playbackActivity = (MediaPlaybackActivity) (mInstrumentation.
                waitForMonitorWithTimeout(mActivityMonitor, WAIT_FOR_ACTIVITY_CREATED_TIME));
        assertNotNull(playbackActivity);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
                "MediaPlaybackActivity");
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME*2);
        // Check whether current playing track's album is same as the selected one or different
        assertEquals(nowPlayingTrackName, mService.getTrackName());
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // Clear the current playlist, check the now playing whether gone and Music has stopped

        View buttonQueue = (View) playbackActivity.findViewById(R.id.curplaylist);
        mSolo.clickOnView(buttonQueue);
        // Wait for TrackBrowserActivity and check the song number
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!", "TrackBrowserActivity");

        // 2.Test drag track
        int initialQueuePos = mService.getQueuePosition();
        // get remove from item location
        View view = (View) mListView.getChildAt(0).findViewById(R.id.icon);
        int[] FromItemLocation = new int[2];
        view.getLocationInWindow(FromItemLocation);
        view.getLocationOnScreen(FromItemLocation);
        // get move to item location
        view = (View) mListView.getChildAt(1).findViewById(R.id.icon);
        int[] ToItemlocation = new int[2];
        view.getLocationInWindow(ToItemlocation);
        view.getLocationOnScreen(ToItemlocation);

        mSolo.drag(FromItemLocation[0] + 10, FromItemLocation[0] + 10,
                FromItemLocation[1] + 30, ToItemlocation[1] + 30, 1);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getQueuePosition() == (initialQueuePos + 1));

        mSolo.clickOnMenuItem(mContext.getString(R.string.clear_playlist));
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        long audioId = mService.getAudioId();
        // no song is playing
        assertEquals(-1, audioId);
        assertFalse(isPlaying());
        Log.d(TAG, "<< test12_ClickNowPlayingToPlay");*/
    }

    public void test13_SwitchLandscapeAndPortrait() throws Exception {
        Log.d(TAG, ">> test13_SwitchLandscapeAndPortrait");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.play();
        assertTrue(isPlaying());

        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        waitForViewDisppear(mMusicBrowserActivity, R.id.nowplaying, View.GONE);
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
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 2);
        mSolo.clickOnMenuItem(mContext.getString(R.string.clear_playlist));
        // Check nowplaying can not press
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        nowPlayingTab = (View) mMusicBrowserActivity.findViewById(R.id.nowplayingtab);
        assertEquals(nowPlayingTab.isEnabled(), false);

        // 2.Switch to portrait
        mMusicBrowserActivity.setRequestedOrientation(Configuration.ORIENTATION_PORTRAIT);
        // mSolo.setActivityOrientation(Solo.PORTRAIT);
        mInstrumentation.waitForIdleSync();
        // mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        Log.d(TAG, "<< test13_SwitchLandscapeAndPortrait");
    }

    public void test14_ScrollUpAndDown() throws Exception {
        Log.d(TAG, ">> test14_ScrollUpAndDown");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        int listCount = mListView.getCount();
        if (listCount < 8) {
            return;
        }

        mService.play();
        assertTrue(isPlaying());
        // Get track position from listview before scroll
        int oldPosition = mListView.getFirstVisiblePosition();

        // scroll down and check the position change
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.scrollDownList(0);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // Get track position from listview after scroll
        int newPosition = mListView.getFirstVisiblePosition();
        assertFalse(newPosition == oldPosition);

        // Scroll up and check position revise
        mSolo.scrollUpList(0);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        newPosition = mListView.getFirstVisiblePosition();
        Log.d(TAG, "test14_MediaButtonIntent,oldPosition:" +
         oldPosition + ",newPosition:" + newPosition);
        MusicTestUtils.checkValueStatus(newPosition, oldPosition);
        assertTrue(isPlaying());
        Log.d(TAG, "<< test14_ScrollUpAndDown");
    }

    public void test15_SdcardStateChange() throws Exception {
        /*if (MusicFeatureOption.IS_SUPPORT_SHARED_SDCARD) {
            Log.d(TAG, "IS_SUPPORT_SHARED_SDCARD is true,skip this testcase");
            return;
        }
        Log.d(TAG, ">> test15_SdcardStateChange");
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.play();
        mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
        assertTrue(MusicTestUtils.isPlaying(mService));

        StorageManager storageManager =
         (StorageManager) mContext.getSystemService(Context.STORAGE_SERVICE);
        View errorView = mTrackBrowserActivity.findViewById(R.id.sd_icon);
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

        Log.d(TAG, "<< test15_SdcardStateChange");*/
    }

    public void test16_RemoveFromNowplaying() throws Exception {
        Log.d(TAG, ">> test16_RemoveFromNowplaying");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // 1.test click nowplaying
        // Click option meun play all
        mSolo.clickOnMenuItem(mContext.getString(R.string.play_all));
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());

        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME * 2);

        // click nowplaying list
        View buttonQueue = (View) mSolo.getCurrentActivity().findViewById(R.id.curplaylist);
        mSolo.clickOnView(buttonQueue);
        // Wait for TrackBrowserActivity and check the song number
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!", "TrackBrowserActivity");
        // remove from Nowplaylist and check the audio auto switch to next songs.
        long oldAudioId = MusicUtils.getCurrentAudioId();
        mSolo.clickLongInList(0, 0, LONG_CLICK_TIME);
        mSolo.clickOnText(mContext.getString(R.string.remove_from_playlist));
        mInstrumentation.waitForIdleSync();
        long currentAudioId = MusicUtils.getCurrentAudioId();
        // assertTrue(oldAudioId != currentAudioId);
        assertTrue(isPlaying());
        Log.d(TAG, "<< test16_RemoveFromNowplaying");
    }


    public void test17_RemoveFromPlaylist() throws Exception {
        Log.d(TAG, ">> test17_RemoveFromPlaylist");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Delete all playlist in DB
        deleteAllPlaylist();

        // 1.Create a new playlist with name of TEST_PLAYLIST_NAME and get playlist ID
        ContentValues cv = new ContentValues();
        cv.put(MediaStore.Audio.Playlists.NAME, TEST_PLAYLIST_NAME);
        Uri newPlaylistUri = mMusicBrowserActivity.getContentResolver().insert(
                        MediaStore.Audio.Playlists.getContentUri("external"), cv);
        assertTrue(newPlaylistUri != null);
        String idString = newPlaylistUri.getLastPathSegment();
        long newPlaylistId = Long.parseLong(idString);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Long click a listview item to open context menu
        mSolo.clickLongOnView(mListView.getChildAt(0), LONG_CLICK_TIME);
        // Click context menu add to test playlist
        mSolo.clickOnText(mContext.getString(R.string.add_to_playlist));
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // 2.Goto PlayListBrowserActivity
        View PlaylistTab = mSolo.getView(R.id.playlisttab);
        mSolo.clickOnView(PlaylistTab);
        mInstrumentation.waitForIdleSync();
        // 3.Enter NewPlaylist,then remove song from current palylist.
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);
        mSolo.clickLongInList(0, 0, LONG_CLICK_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 2);
        mSolo.clickLongOnText(mContext.getString(R.string.remove_from_playlist));
        mInstrumentation.waitForIdleSync();
        // 4.check the list item count is 0 and show "no music" in screen.
        assertTrue(mSolo.getCurrentViews(ListView.class).get(0).getCount() == 0);
        assertTrue(mSolo.searchText(mContext.getString(R.string.no_music_title)));
        Log.d(TAG, ">> test17_RemoveFromPlaylist");
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
     * @param long
     *            The member declared in activity
     * @return The long value of member declared in activity
     */
    public long getDeclaredLongValue(Activity activity, String string) {
        try {
            Field field = activity.getClass().getDeclaredField(string.toString());
            field.setAccessible(true);
            return ((Long) field.get(activity)).intValue();
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
     * Get local activity manager by string which is declared in MusicBrowserActivity.
     *
     * @param long
     *            The member declared in activity
     * @return The local activity manager declared in MusicBrowserActivity
     */
    public LocalActivityManager getDeclaredLocalActivityManager(String string) {
        try {
            Field field = mMusicBrowserActivity.getClass().getDeclaredField(string.toString());
            field.setAccessible(true);
            return ((LocalActivityManager) field.get(mMusicBrowserActivity));
        } catch (NoSuchFieldException ex) {
            Log.e(TAG, "No such field in runTestOnUiThread:", ex);
            return null;
        } catch (IllegalAccessException ex) {
            Log.e(TAG, "Illegal access exception in runTestOnUiThread:", ex);
            return null;
        }
    }

    public IMediaPlaybackService getDeclaredService(String string) {
        try {
            Field field = mMusicBrowserActivity.getClass().getDeclaredField(string.toString());
            field.setAccessible(true);
            return ((IMediaPlaybackService) field.get(mMusicBrowserActivity));
        } catch (NoSuchFieldException ex) {
            Log.e(TAG, "No such field in runTestOnUiThread:", ex);
            return null;
        } catch (IllegalAccessException ex) {
            Log.e(TAG, "Illegal access exception in runTestOnUiThread:", ex);
            return null;
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
     * Get the current listview count(such as using to get the search counts),we
     * shoule requery the songs number again for 6 times to makesure there are
     * really no songs in Music. If after 6 times requery there are no songs but
     * we sure there are some songs in sdcard, maybe the phone performance is
     * too bad.
     *
     * @return The count of search result
     */
    public int getListViewCount() {
        ListView currentListView = mSolo.getCurrentViews(ListView.class).get(0);
        if (null != currentListView) {
            int count = currentListView.getCount();
            // Requery the songs number again for 6 times to makesure there are
            // really no songs in Music.
            // If after 6 times requery there are no songs but we sure there are
            // some songs in sdcard, maybe
            // the phone performance is too bad.
            for (int reQueryTimes = 6, i = 1; count <= 0; i++) {
                mSolo.sleep(WAIT_FOR_RESPOND_TIME);
                count = currentListView.getCount();
                Log.d(TAG, "getListViewCount: query times = " + i + ", song count = " + count);
                if (i > reQueryTimes) {
                    break;
                }
            }
            return count;
        }
        return -1;
    }

    /**
     * wait for Veiw Dissapear
     */
    private void waitForViewDisppear(Activity mActivity, int viewId, int visibleState)
            throws InterruptedException {
        Log.d(TAG, "waitForViewDisppear() entry with type is " + viewId);
        View view = (View) mActivity.findViewById(viewId).getParent();
        assertNotNull(view);
        long startTime = System.currentTimeMillis();
        Log.d(TAG, "before enter loop,view.getVisibility()== " + view.getVisibility());
        while (view.getVisibility() != visibleState) {
            Log.d(TAG, "view.getVisibility()== " + view.getVisibility());
            if (System.currentTimeMillis() - startTime > TIME_OUT) {
                fail("waitForViewDisppear() timeout");
            }
            Thread.sleep(SLEEP_TIME);
            view = (View) mActivity.findViewById(viewId).getParent();
        }
        Log.d(TAG, "waitForViewDisppear() exit with view is " + view);
    }

    /**
     * Check whether music is playing in 5000 ms.
     *
     * @return If music is playing,return true,else return false
     */
    public boolean waitForServiceConnected() throws InterruptedException {
        long startTime = System.currentTimeMillis();
        long waitTime = 5000;
        boolean isTimeOut = false;

        try {
            while (mService == null) {
                Thread.sleep(WAIT_FOR_RESPOND_TIME);
                mService = getDeclaredService("mService");
                if (System.currentTimeMillis() - startTime > waitTime) {
                    isTimeOut = true;
                    break;
                }
            }

        } catch (InterruptedException e) {
            Log.e(TAG, "InterruptedException " + e);
        }

        return !isTimeOut;
    }
}
