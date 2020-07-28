package com.android.music.tests.functional;

import java.io.File;

import android.app.Instrumentation;
import android.content.ComponentName;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.database.Cursor;
import android.media.AudioManager;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.provider.Settings;
import android.test.ActivityInstrumentationTestCase2;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.ListView;


import com.android.music.*;
import com.jayway.android.robotium.solo.Solo;

public class PlaybackFunctionTest extends ActivityInstrumentationTestCase2<MediaPlaybackActivity>
        implements ServiceConnection {

    private static final String TAG = "MusicFunctionalTest";
    private static final String TEST_PLAYLIST_NAME = "functiontest playlist";
    private static final String SONGS_LESS_THAN_TRHEE = "There are less than three songs or" +
     " the system performance is bad, please push more or reboot phone to make music pass test";
    private static final int WAIT_FOR_ACTIVITY_CREATED_TIME = 10000;
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final int WAIT_FOR_DELETED_TIME = 1500;
    private static final int WAIT_FOR_SERVICE_CONNECTED_TIME = 5000;
    private static final int WAIT_FOR_JUMPING_TO_PREV_TIME = 3000;
    private static final int MINIMUM_SONGS_COUNT_IN_MUSIC = 3;
    private static final int LONG_CLICK_TIME = 5000;

    private Instrumentation mInstrumentation = null;
    private MediaPlaybackActivity mMediaPlaybackActivity = null;
    private ListView mListView = null;
    private Solo mSolo = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;

    public PlaybackFunctionTest() {
        super(MediaPlaybackActivity.class);
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

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.d(TAG, "setUp>>>");
        setActivityInitialTouchMode(true);
        mInstrumentation = getInstrumentation();

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


        // ready the service for playback and other functions
        if (mService != null) {
            long[] audioIds = MusicUtils.getAllSongs(mInstrumentation.getContext());
            mService.open(audioIds, 0);
        }
        mInstrumentation.waitForIdleSync();
        // start activity until the service has started
        // so that the activity will not be finished once it starts at updateTrackInfo.
        mMediaPlaybackActivity = getActivity();
        mContext = mMediaPlaybackActivity.getApplicationContext();
        mSolo = new Solo(mInstrumentation, mMediaPlaybackActivity);

        // Assert all used to be not null
        assertNotNull(mInstrumentation);
        assertNotNull(mMediaPlaybackActivity);
        assertNotNull(mContext);
        assertNotNull(mSolo);
        assertNotNull(mService);
        Log.d(TAG, "setUp<<<");
    }

    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "tearDown>>>");
        mSolo.finishOpenedActivities();
        try {
            mSolo.finalize();
        } catch (Throwable t) {
            t.printStackTrace();
        }
        Thread.sleep(MusicTestUtils.WAIT_TEAR_DONW_FINISH_TIME);
        Log.d(TAG, "tearDown<<<");
        super.tearDown();
    }

    public void test01_PlayPauseSwitching() throws Throwable {
        Log.d(TAG, ">> test01_PlayPauseSwitching");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

         // Pause unnecessary playback
        mService.pause();
        mInstrumentation.waitForIdleSync();

        // Click play button and check the playback status
        final View buttonPlayPause = (View) mMediaPlaybackActivity.findViewById(R.id.pause);
        mSolo.clickOnView(buttonPlayPause);
        assertTrue(isPlaying());

        // Click pause again to pause playback and check the playback status
        mSolo.clickOnView(buttonPlayPause);
        assertTrue(isStop());

        Log.d(TAG, "<< test01_PlayPauseSwitching");
    }

    public void test02_PrevNext() throws Throwable {
        Log.d(TAG, ">> test02_PrevNext");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // 1.Test prev and next button
        // Make sure music is playing and in SHUFFLE_NONE, REPEAT_NONE modes.
        mService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);
        mService.setRepeatMode(MediaPlaybackService.REPEAT_NONE);
        mService.play();
        mInstrumentation.waitForIdleSync();
        assertTrue(isPlaying());
        int initialQueuePos = mService.getQueuePosition();
        final View buttonPrev = mMediaPlaybackActivity.findViewById(R.id.prev);
        final View buttonNext = mMediaPlaybackActivity.findViewById(R.id.next);

        // Test switching to next track
        mSolo.clickOnView(buttonNext);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // check if we are at track 2
        Log.d(TAG, "queue position=" + mService.getQueuePosition());
        assertTrue(mService.getQueuePosition() == (initialQueuePos + 1));

        mService.pause();
        mService.seek(0);
        mInstrumentation.waitForIdleSync();

        // Test switching to previous track;
        mSolo.clickOnView(buttonPrev);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());
        assertTrue(mService.getQueuePosition() == initialQueuePos);

        // 2.test forward and rewind button, the track duration must bigger than 100000ms
        if (mService.duration() < 100000) {
            return;
        }
        final int normalSpeedTime = 3000; // 10X speed
        final int fastSpeedTime = 6000;  // 40X speed
        final int positionForSpeedFast = 5000; // 10X speed below 5000ms and 40X more than 5000ms
        mService.seek(10000);
        mInstrumentation.waitForIdleSync();
        // Normal speed for forward and rewind
        long startPosition = mService.position();
        mSolo.clickLongOnView(buttonNext, normalSpeedTime);
        mInstrumentation.waitForIdleSync();
        long endPosition = mService.position();
        assertTrue(endPosition > startPosition);
        Log.d(TAG, "test02_PrevNext:Normal forward duration " + (endPosition - startPosition));

        startPosition = mService.position();
        mSolo.clickLongOnView(buttonPrev, normalSpeedTime);
        mInstrumentation.waitForIdleSync();
        endPosition = mService.position();
        assertTrue(endPosition < startPosition);
        Log.d(TAG, "test02_PrevNext:Normal rewind duration " + (startPosition - endPosition));

        // Fast speed for forward and rewind
        mService.seek(10000);
        startPosition = mService.position();
        mSolo.clickLongOnView(buttonNext, fastSpeedTime);
        mInstrumentation.waitForIdleSync();
        endPosition = mService.position();
        Log.d(TAG, "test02_PrevNext:Fast forward duration " +
         (endPosition - startPosition) + ",startPosition:" + startPosition +
          ",endPosition" + endPosition) ;
        assertTrue((endPosition - startPosition) > (positionForSpeedFast * 10));
        startPosition = mService.position();
        mSolo.clickLongOnView(buttonPrev, fastSpeedTime);
        mInstrumentation.waitForIdleSync();
        endPosition = mService.position();
        Log.d(TAG, "test02_PrevNext:Fast rewind duration " +
         (startPosition - endPosition) + ",startPosition:" + startPosition +
          ",endPosition" + endPosition);
        assertTrue((startPosition - endPosition) > (positionForSpeedFast * 10));
        Log.d(TAG, "<< test02_PrevNext");
    }

    public void test03_RepeatShuffle() throws Throwable {
        Log.d(TAG, ">> test03_RepeatShuffle");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME * 2);

        final View buttonShuffle = mMediaPlaybackActivity.findViewById(R.id.shuffle);
        final View buttonRepeat = mMediaPlaybackActivity.findViewById(R.id.repeat);

        // clear all shuffle/repeat status
        mService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);
        mService.setRepeatMode(MediaPlaybackService.REPEAT_NONE);
        // step 1: click shuffle
        mSolo.clickOnView(buttonShuffle);
        mInstrumentation.waitForIdleSync();
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NORMAL);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_NONE);

        // step 2: click repeat
        mSolo.clickOnView(buttonRepeat);
        mInstrumentation.waitForIdleSync();
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NORMAL);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_ALL);

        // step 3: click repeat again
        mSolo.clickOnView(buttonRepeat);
        mInstrumentation.waitForIdleSync();
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NONE);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_CURRENT);

        // step 4: clear shuffle/repeat status and click repeat twice
        mService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);
        mService.setRepeatMode(MediaPlaybackService.REPEAT_NONE);
        mSolo.clickOnView(buttonRepeat);
        mInstrumentation.waitForIdleSync();
        mSolo.clickOnView(buttonRepeat);
        mInstrumentation.waitForIdleSync();
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_CURRENT);

        // step 5: click shuffle once
        mSolo.clickOnView(buttonShuffle);
        mInstrumentation.waitForIdleSync();
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NORMAL);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_ALL);

        Log.d(TAG, "<< test03_RepeatShuffle");
    }

    public void test04_Queue() throws Throwable {
        Log.d(TAG, ">> test04_Queue");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME * 2);
        View buttonQueue = (View) mMediaPlaybackActivity.findViewById(R.id.curplaylist);
        mSolo.clickOnView(buttonQueue);
        // Wait for TrackBrowserActivity and check the song number
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!", "TrackBrowserActivity");
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        assertTrue(mSolo.getCurrentViews(ListView.class).get(0).getCount() > 0);

        Log.d(TAG, "<< test04_Queue");
    }

    public void test05_SearchByLongClick() throws Throwable {
        /*Log.d(TAG, ">> test05_SearchByLongClick");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        final View artistName =
        (View) mMediaPlaybackActivity.findViewById(R.id.artistname).getParent();
        final View albumName =
        (View) mMediaPlaybackActivity.findViewById(R.id.albumname).getParent();
        final View trackName =
        (View) mMediaPlaybackActivity.findViewById(R.id.trackname).getParent();

        // 1. search by artist name
        // Long click artist name to search
        mSolo.clickLongOnView(artistName, LONG_CLICK_TIME);
        // Click Music to search in music app
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // Check the search result
        int searchCount = getListViewCount();;
        assertTrue(searchCount > 0);

        // 2. search by album name
        // Go back to playback activity
        mSolo.goBack();
        mSolo.goBack();
        mInstrumentation.waitForIdleSync();
        // Long click artist name to search
        mSolo.clickLongOnView(albumName, LONG_CLICK_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // Click Music to search in music app
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // Check the search result
        searchCount = getListViewCount();
        assertTrue(searchCount > 0);

        // 3. search by track name
        // Go back to playback activity
        mSolo.goBack();
        mSolo.goBack();
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // Long click artist name to search
        mSolo.clickLongOnView(trackName, LONG_CLICK_TIME);
        // Click Music to search in music app
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // Check the search result
        searchCount = getListViewCount();;
        assertTrue(searchCount > 0);
        Log.d(TAG, "<< test05_SearchByLongClick");
        */
    }

    public void test06_PartyShuffle() throws Exception {
        Log.d(TAG, ">> test02_PartyShuffle");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // Can't stop playing, reason to be find TODO
        // mService.stop();
        mInstrumentation.waitForIdleSync();
        // Quit party shuffle if we're already in this mode
        mService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);

        // 1.test in mediaaplayback
        // Click option meun party shuffle
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_AUTO);
        assertTrue(isPlaying());
        Thread.sleep(LONG_CLICK_TIME);
        // Click option meun party shuffle off
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle_off));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NONE);
        assertTrue(isPlaying());

        // 2.test in queun
        // Stop playing
        mService.stop();
        mInstrumentation.waitForIdleSync();
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME * 2);
        View buttonQueue = (View) mMediaPlaybackActivity.findViewById(R.id.curplaylist);
        mSolo.clickOnView(buttonQueue);
        // Wait for TrackBrowserActivity and check the song number
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!", "TrackBrowserActivity");
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);
        // Click option meun party shuffle
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_AUTO);
        assertTrue(isPlaying());
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);
        // Click option meun party shuffle off
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle_off));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NONE);
        assertTrue(isPlaying());
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME * 4);
        Log.d(TAG, "<< test02_PartyShuffle");
    }

    public void test07_AddToNewPlaylist() throws Throwable {
        Log.d(TAG, ">> test07_AddToNewPlaylist");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // Delete all playlist in DB
        deleteAllPlaylist();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // Click context option add to new playlist and wait for CreatePlaylist
        mSolo.clickOnMenuItem(mContext.getString(R.string.add_to_playlist));
        mSolo.clickOnText(mContext.getString(R.string.new_playlist));
        mSolo.waitForActivity("CreatePlaylist", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create CreatePlaylist failed!", "CreatePlaylist");
        // Clear default playlist name and enter test playlist name
        mSolo.clearEditText(0);
        mSolo.enterText(0, TEST_PLAYLIST_NAME);
        mSolo.clickOnButton(mContext.getString(R.string.create_playlist_create_text));
        mInstrumentation.waitForIdleSync();

        // Check for playlist insertion and track insertion
        Cursor c = mMediaPlaybackActivity.getContentResolver().query(
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
        long selectedAudioId = MusicUtils.getCurrentAudioId();
        c = mMediaPlaybackActivity.getContentResolver().query(
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

        Log.d(TAG, "<< test07_AddToNewPlaylist");
    }

    public void test08_addToExistPlaylist() throws Exception {
        Log.d(TAG, ">> test08_addToExistPlaylist");

        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Delete all playlist in DB
        deleteAllPlaylist();

        // Create a new playlist with name of TEST_PLAYLIST_NAME
        ContentValues cv = new ContentValues();
        cv.put(MediaStore.Audio.Playlists.NAME, TEST_PLAYLIST_NAME);
        Uri newPlaylistUri = mMediaPlaybackActivity.getContentResolver().insert(
                        MediaStore.Audio.Playlists.getContentUri("external"), cv);
        assertTrue(newPlaylistUri != null);
        String idString = newPlaylistUri.getLastPathSegment();
        long testListId = Long.parseLong(idString);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // invalid option menu to refresh the new insert test playlist.
        mMediaPlaybackActivity.invalidateOptionsMenu();

        // Click option meun to add to test playlist
        mSolo.clickOnMenuItem(mContext.getString(R.string.add_to_playlist));
        //TODO why we call this can not add sucess!mSolo.clickOnMenuItem
        //(mContext.getString(R.string.add_to_playlist));
        mSolo.clickOnText(TEST_PLAYLIST_NAME);
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Check the seleted audio whether exist in the test playlist
        long selectedAudioId = MusicUtils.getCurrentAudioId();
        Cursor c = mMediaPlaybackActivity.getContentResolver().query(
                MediaStore.Audio.Playlists.Members.getContentUri("external", testListId),
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

        Log.d(TAG, "<< test08_addToExistPlaylist");
    }

    public void test09_UseAsRingtone() throws Exception {
        Log.d(TAG, ">> test09_UseAsRingtone");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Click option menu to use as ringtone
        mSolo.clickOnMenuItem(mContext.getString(R.string.ringtone_menu_short));
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Wait for a while and check for settings DB again
        long setAudioId = MusicUtils.getCurrentAudioId();
        String uriString = Settings.System.getString(mMediaPlaybackActivity.getContentResolver(),
                Settings.System.RINGTONE);
        assertNotNull(uriString);
        Uri uri = Uri.parse(uriString);
        String idString = uri.getLastPathSegment();
        long gotAudioId = Long.parseLong(idString);
        assertEquals(setAudioId, gotAudioId);

        Log.d(TAG, "<< test09_UseAsRingtone");
    }

    public void test10_DeleteItem() throws Exception {
        Log.d(TAG, ">> test10_DeleteItem");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());

        // Get the delete track ID
        long deletedId = MusicUtils.getCurrentAudioId();
        String deletedPath = null;
        // Get the actual file path of the deleting item
        Cursor c = mMediaPlaybackActivity.getContentResolver().query(
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
        // Click option option add to new playlist
        mSolo.clickOnMenuItem(mContext.getString(R.string.delete_item));
        mSolo.waitForActivity("DeleteItems", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create DeleteItems failed!", "DeleteItems");
        mSolo.clickOnButton(mContext.getString(R.string.delete_confirm_button_text));
        mInstrumentation.waitForIdleSync();
        Thread.sleep(WAIT_FOR_RESPOND_TIME);

        // Check Media DB for deleted item by 10 times in 5 second.
        int checkTimes = 10;
        boolean isDeleted = false;
        int i = 0;
        while (!isDeleted) {
            Thread.sleep(WAIT_FOR_RESPOND_TIME);
            File f = new File(deletedPath);
            isDeleted = !f.exists();
            i++;
            Log.d(TAG, "Delete: check times = " + i + ", isDeleted = " + isDeleted);
            if (i > checkTimes - 1) {
                break;
            }
        }

        c = mMediaPlaybackActivity.getContentResolver().query(
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

        // Check file system for deleted item
        if (deletedPath != null) {
            File f = new File(deletedPath);
            assertFalse(f.exists());
        }

        Log.d(TAG, "<< test10_DeleteItem");
    }

    public void test11_SwitchLandscapeAndPortrait() throws Exception {
        Log.d(TAG, ">> test11_SwitchLandscapeAndPortrait");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.play();
        assertTrue(isPlaying());

        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);


        // 2.Switch to portrait
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());
        Log.d(TAG, "<< test11_SwitchLandscapeAndPortrait");
    }

    public void test12_ClickQueryResult() throws Exception {
        /*Log.d(TAG, ">> test12_ClickQueryResult");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        //mService.stop();
        //assertTrue(MusicTestUtils.isStopping(mService));

        // Search by artist to query out artist, album and track
        final View artistName =
         (View) mMediaPlaybackActivity.findViewById(R.id.artistname).getParent();
        final View albumName =
         (View) mMediaPlaybackActivity.findViewById(R.id.albumname).getParent();
        final View trackName =
         (View) mMediaPlaybackActivity.findViewById(R.id.trackname).getParent();
        // Long click artist name to search
        longClickForSearch(artistName);

        // 1.Click artist
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.clickInList(0);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.waitForActivity("AlbumBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create AlbumBrowserActivity failed!", "AlbumBrowserActivity");
        assertEquals(mService.getArtistName(), mSolo.getCurrentActivity().getTitle().toString());

        // 2.Click album
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.clickInList(0);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!",
         "TrackBrowserActivity");
        // 3.Click track
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.clickInList(0);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");


        // Long click album name to search
        longClickForSearch(albumName);
        // 1.Click album
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.clickInList(0);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!", "TrackBrowserActivity");
        String album = mSolo.getCurrentActivity().getTitle().toString();
        // 2.Click track
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.clickInList(0);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mInstrumentation.waitForIdleSync();
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");
        assertEquals(mService.getAlbumName(), album);

        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // Long click track name to search
        longClickForSearch(trackName);
        // Click track
        if (getListViewCount() == 1) {
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
            mSolo.clickInList(0);
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
            mInstrumentation.waitForIdleSync();
        } else if (getListViewCount() == 2) {
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
            mSolo.clickInList(2);
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
            mInstrumentation.waitForIdleSync();
        } else {
            return;
        }
        mInstrumentation.waitForIdleSync();
        mSolo.waitForActivity("MediaPlaybackActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create MediaPlaybackActivity failed!",
         "MediaPlaybackActivity");
        assertEquals(mService.getAudioId(), MusicUtils.getCurrentAudioId());
        assertTrue(MusicTestUtils.isPlaying(mService));
        // go to querybrowseractivity
        mSolo.goBack();
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);

        // Switch landscape and portrait
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        assertTrue(MusicTestUtils.isPlaying(mService));

        Log.d(TAG, "<< test12_ClickQueryResult");*/
    }

    public void test13_AudioFocusChange() throws Exception {
        Log.d(TAG, ">> test13_AudioFocusChange");
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 1.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK
        AudioManager audioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        audioManager.requestAudioFocus(null, AudioManager.STREAM_RING,
         AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK);
        assertTrue(MusicTestUtils.isPlaying(mService));
        //TODO mCurrentVolume<1.0f
        audioManager.abandonAudioFocus(null);
        //TODO mCurrentVolume>0.2f
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 2.AUDIOFOCUS_LOSS_TRANSIENT
        audioManager.requestAudioFocus(null, AudioManager.STREAM_RING,
         AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        assertTrue(MusicTestUtils.isStopping(mService));
        audioManager.abandonAudioFocus(null);
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 3.AUDIOFOCUS_LOSS
        audioManager.requestAudioFocus(null, AudioManager.STREAM_RING,
         AudioManager.AUDIOFOCUS_GAIN);
        assertTrue(MusicTestUtils.isStopping(mService));
        audioManager.abandonAudioFocus(null);
        assertTrue(MusicTestUtils.isStopping(mService));

        // 4. Focus changed in music has stopped
        audioManager.requestAudioFocus(null, AudioManager.STREAM_RING,
        AudioManager.AUDIOFOCUS_GAIN);
        assertTrue(MusicTestUtils.isStopping(mService));
        audioManager.abandonAudioFocus(null);
        assertTrue(MusicTestUtils.isStopping(mService));

        audioManager.requestAudioFocus(null, AudioManager.STREAM_RING,
         AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK);
        assertTrue(MusicTestUtils.isStopping(mService));
        //TODO mCurrentVolume<1.0f
        audioManager.abandonAudioFocus(null);
        //TODO mCurrentVolume>0.2f
        assertTrue(MusicTestUtils.isStopping(mService));

        audioManager.requestAudioFocus(null, AudioManager.STREAM_RING,
         AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
        assertTrue(MusicTestUtils.isStopping(mService));
        audioManager.abandonAudioFocus(null);
        assertTrue(MusicTestUtils.isStopping(mService));

    }

    public void test14_MediaButtonIntent() throws Throwable {
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // 1.Noisy intent(Music stop)
        //Intent intent = new Intent();
        //intent.setAction(AudioManager.ACTION_AUDIO_BECOMING_NOISY);
        //mMediaPlaybackActivity.sendBroadcast(intent);
        //assertTrue(MusicTestUtils.isStopping(mService));

        // 2.Next media button
        int initialQueuePos = mService.getQueuePosition();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_NEXT);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        Log.d(TAG, "test14_MediaButtonIntent,initialQueuePos:" +
         initialQueuePos + ",currentPosition:" + mService.getQueuePosition());
        MusicTestUtils.checkValueStatus(mService.getQueuePosition(), initialQueuePos + 1);
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 3.Prev media button
        mService.pause();
        assertTrue(MusicTestUtils.isStopping(mService));
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PREVIOUS);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertEquals(initialQueuePos, mService.getQueuePosition());
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 4.Pause media button(Music stop)
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PAUSE);
        assertTrue(MusicTestUtils.isStopping(mService));

        // 5.Play media button(Music play)
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PLAY);
        assertTrue(MusicTestUtils.isPlaying(mService));

        // 6.Stop media button(Music stop)
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_STOP);
        assertTrue(MusicTestUtils.isStopping(mService));

        // 7.PlayPause media button(If music stop, music will play, contrariwise)
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE);
        assertTrue(MusicTestUtils.isPlaying(mService));
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE);
        assertTrue(MusicTestUtils.isStopping(mService));

        // 7.Forward and rewind media button(If music stop, music will play, contrariwise)
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_FAST_FORWARD);
        mInstrumentation.waitForIdleSync();
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_MEDIA_REWIND);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(MusicTestUtils.isStopping(mService));
    }

    public void test15_SdcardStateChange() throws Exception {
        /*if (MusicFeatureOption.IS_SUPPORT_SHARED_SDCARD) {
            Log.d(TAG, "IS_SUPPORT_SHARED_SDCARD is true,skip this testcase");
            return;
        }
        Log.d(TAG, "<< test15_SdcardStateChange");
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.play();
        mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
        assertTrue(MusicTestUtils.isPlaying(mService));
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.PORTRAIT);
        Thread.sleep(WAIT_FOR_RESPOND_TIME*2);
        View buttonQueue = (View) mMediaPlaybackActivity.findViewById(R.id.curplaylist);
        mSolo.clickOnView(buttonQueue);
        // Wait for TrackBrowserActivity and check the song number
        mSolo.waitForActivity("TrackBrowserActivity", WAIT_FOR_ACTIVITY_CREATED_TIME);
        mSolo.assertCurrentActivity("create TrackBrowserActivity failed!",
         "TrackBrowserActivity");

        StorageManager storageManager =
         (StorageManager) mContext.getSystemService(Context.STORAGE_SERVICE);
        try {
            // 1.Unmounted sdcard
            storageManager.enableUsbMassStorage();
            assertTrue(MusicTestUtils.isStopping(mService));
            mInstrumentation.waitForIdleSync();
            mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
            assertTrue(mMediaPlaybackActivity.isFinishing());
            // 2.Mounted sdcard
            storageManager.disableUsbMassStorage();
            assertTrue(!MusicTestUtils.waitForScanFinishAfterDisableUsbMassStorage(mContext));
            mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
            assertTrue(MusicTestUtils.isStopping(mService));
            // finally we need make sure sdcard has been mounted
        } finally {
            if (!MusicTestUtils.hasMountedSDcard(mContext)) {
                storageManager.disableUsbMassStorage();
                assertTrue(!MusicTestUtils.waitForScanFinishAfterDisableUsbMassStorage(mContext));
                mSolo.sleep(WAIT_FOR_SERVICE_CONNECTED_TIME);
            }
        }

        Log.d(TAG, ">> test15_SdcardStateChange");*/
    }

    public void tset16_BackUp() throws Exception {
        Log.d(TAG, ">> tset16_BackUp");
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));

        mSolo.goBack();
        mInstrumentation.waitForIdleSync();

        assertTrue(MusicTestUtils.isPlaying(mService));
    }

    public void test17_ActionBar_home() throws Exception {
        /*Log.d(TAG, ">> test17_ActionBar");
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));
//        View home_Actionbar = (View)mMediaPlaybackActivity.findViewById(android.R.id.home);
        View home_Actionbar = mSolo.getView(android.R.id.home);
        mSolo.clickOnView(home_Actionbar);
        mInstrumentation.waitForIdleSync();
        assertTrue(MusicTestUtils.isPlaying(mService));*/
    }

    public void test18_ActionBar_nowplay() throws Exception {
        Log.d(TAG, ">> test18_ActionBar_nowplay");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        mService.play();
        assertTrue(isPlaying());

        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);

        View nowplay_ActionBar = mSolo.getView(R.id.current_playlist_menu_item);
        mSolo.clickOnView(nowplay_ActionBar);
        mSolo.sleep(WAIT_FOR_ACTIVITY_CREATED_TIME);
        assertTrue(isPlaying());
        Log.d(TAG, "<< test18_ActionBar_nowplay");
    }

    public void test19_ActionBar_shuffle() throws Exception {
        Log.d(TAG, ">> test19_ActionBar_shuffle");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // Can't stop playing, reason to be find TODO
        // mService.stop();
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));
        mInstrumentation.waitForIdleSync();
        // Quit party shuffle if we're already in this mode
        mService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);
        mInstrumentation.waitForIdleSync();
        // 1.test in mediaaplayback
        // Click option meun party shuffle
        mSolo.clickOnMenuItem(mContext.getString(R.string.party_shuffle));
        Thread.sleep(WAIT_FOR_RESPOND_TIME);
        // No activity, just check service change in shuffle mode
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_AUTO);
        assertTrue(isPlaying());
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertEquals(Configuration.ORIENTATION_LANDSCAPE,
                mMediaPlaybackActivity.getResources().getConfiguration().orientation);

        View shuffle_ActionBar = mSolo.getView(R.id.shuffle_menu_item);
        mSolo.clickOnView(shuffle_ActionBar);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NONE);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());

        mSolo.clickOnView(shuffle_ActionBar);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NORMAL);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());

        mSolo.clickOnView(shuffle_ActionBar);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getShuffleMode() == MediaPlaybackService.SHUFFLE_NONE);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());
        Log.d(TAG, "<< test19_ActionBar_shuffle");
    }

    public void test20_ActionBar_repeate() throws Exception {
        /*Log.d(TAG, ">> test20_ActionBar_repeate");
        // Makesure there are more than three songs in music to pass auto test
        assertTrue(SONGS_LESS_THAN_TRHEE, isMoreThanThreeSongs());
        // Can't stop playing, reason to be find TODO
        // mService.stop();
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));
        mInstrumentation.waitForIdleSync();
        // Quit party shuffle if we're already in this mode
        mService.setRepeatMode(MediaPlaybackService.REPEAT_NONE);
        mInstrumentation.waitForIdleSync();

        // No activity, just check service change in shuffle mode
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_NONE);
        assertTrue(isPlaying());
        // 1.Switch to landscape
        mSolo.setActivityOrientation(Solo.LANDSCAPE);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);

        View repeate_ActionBar = mSolo.getView(R.id.repeat_menu_item);
        mSolo.clickOnView(repeate_ActionBar);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_ALL);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());

        mSolo.clickOnView(repeate_ActionBar);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_CURRENT);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(isPlaying());

        mSolo.clickOnView(repeate_ActionBar);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_NONE);

        mSolo.clickOnView(repeate_ActionBar);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        assertTrue(mService.getRepeatMode() == MediaPlaybackService.REPEAT_ALL);
        Log.d(TAG, "<< test20_ActionBar_repeate");*/
    }

    public void test21_progressbarSeek() throws Exception {
        /*Log.d(TAG, ">> test20_progressbarSeek");
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));
        int[] xy = new int[2];
        ProgressBar mProgress = mSolo.getCurrentViews(ProgressBar.class).get(0);
        mProgress.getLocationOnScreen(xy);
        final int viewWidth = mProgress.getWidth();
        final int viewHeight = mProgress.getHeight();
        float x = 3.0f;
        for (int i =3 ; i >= 1; i--){
            x = xy[0] + (viewWidth / (float)i)-10;
            float y = xy[1] + (viewHeight / 2.0f);
            mSolo.clickOnScreen(x,y);
            mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        }
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        Log.d(TAG, "<< test21_progressbarSeek");*/
    }

    public void test22_KeycodeHandle() throws Exception {
        /*Log.d(TAG, ">> test22_KeycodeHandle");
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));
        int keyboard[] =
                { KeyEvent.KEYCODE_Q, KeyEvent.KEYCODE_W, KeyEvent.KEYCODE_E,
                        KeyEvent.KEYCODE_R, KeyEvent.KEYCODE_T,
                        KeyEvent.KEYCODE_Y, KeyEvent.KEYCODE_U,
                        KeyEvent.KEYCODE_I, KeyEvent.KEYCODE_O, KeyEvent.KEYCODE_P,
                        KeyEvent.KEYCODE_A, KeyEvent.KEYCODE_S,
                        KeyEvent.KEYCODE_D, KeyEvent.KEYCODE_F, KeyEvent.KEYCODE_G,
                        KeyEvent.KEYCODE_H, KeyEvent.KEYCODE_J,
                        KeyEvent.KEYCODE_K, KeyEvent.KEYCODE_L,
                        KeyEvent.KEYCODE_DEL, KeyEvent.KEYCODE_Z,
                        KeyEvent.KEYCODE_X, KeyEvent.KEYCODE_C,
                        KeyEvent.KEYCODE_V, KeyEvent.KEYCODE_B,
                        KeyEvent.KEYCODE_N, KeyEvent.KEYCODE_M,
                        KeyEvent.KEYCODE_COMMA, KeyEvent.KEYCODE_PERIOD, KeyEvent.KEYCODE_ENTER };
        assertTrue(MusicTestUtils.isPlaying(mService));
        for (int i = 0; i < 10; i++) {
            mInstrumentation.sendKeyDownUpSync(keyboard[i]);
        }
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_ENTER);
        assertTrue(MusicTestUtils.isPlaying(mService));
        int length = keyboard.length;
        for (int i = 0; i < length; i++) {
            MusicTestUtils.invokeDeclaredMethod(mMediaPlaybackActivity,
             "seekMethod1", new Class[] {int.class},
                    new Object[] {keyboard[i]});
        }
        MusicTestUtils.invokeDeclaredMethod(mMediaPlaybackActivity,
         "seekMethod1", new Class[] {int.class},
                new Object[] {-1});
        assertTrue(MusicTestUtils.isPlaying(mService));
        Log.d(TAG, "<< test22_KeycodeHandle");*/
    }

    public void test23_MusicMethodsTest() throws Exception {
        Log.d(TAG, ">> test23_ServiceMethodTest");
        mService.play();
        assertTrue(MusicTestUtils.isPlaying(mService));
        // 1. getArtwork
        MusicUtils.getArtwork(mContext, -1, -1);

        // 2. setQueuePosition
        MusicUtils.sService.setQueuePosition(0);
        assertEquals(0, mService.getQueuePosition());

        Log.d(TAG, ">> test23_ServiceMethodTest");
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
     * Check whether music is stop in 5000 ms.
     *
     * @return If music is stop,return true,else return false
     */
    public boolean isStop() {
        long startTime = System.currentTimeMillis();
        long waitTime = 5000;
        boolean isTimeOut = false;
        try {
            while (mService.isPlaying()) {
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
        // Requery the songs number again for 6 times to makesure there are really
        // no songs in Music.
        // If after 6 times requery there are no songs but we sure there are some
        // songs in sdcard, maybe
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
        Uri newPlaylistUri = mMediaPlaybackActivity.getContentResolver().insert(
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
        Cursor c = mMediaPlaybackActivity.getContentResolver().query(
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
                    mMediaPlaybackActivity.getContentResolver().delete(deleteUri, null, null);
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
        Cursor c = mMediaPlaybackActivity.getContentResolver().query(
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
        c = mMediaPlaybackActivity.getContentResolver().query(
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
    public int getListViewCount() {
        ListView currentListView = mSolo.getCurrentViews(ListView.class).get(0);
        if (null != currentListView) {
            int count = currentListView.getCount();
            // Requery the songs number again for 6 times to makesure there
            // are really no songs in Music.
            // If after 6 times requery there are no songs but we sure there are some
            // songs in sdcard, maybe
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

    public void longClickForSearch(View clickView) {
        // Long click artist name to search
        mSolo.clickLongOnView(clickView, LONG_CLICK_TIME);
        // Click Music to search in music app
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_DOWN);
        mInstrumentation.sendKeyDownUpSync(KeyEvent.KEYCODE_DPAD_CENTER);
        mInstrumentation.waitForIdleSync();
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        mSolo.sleep(WAIT_FOR_RESPOND_TIME);
        // Check the search result
        int searchCount = getListViewCount();
        assertTrue(searchCount > 0);
    }
}
