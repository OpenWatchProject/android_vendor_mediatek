package com.android.music.tests.functional;


import com.android.music.*;

import android.app.Instrumentation;
import android.app.Instrumentation.ActivityMonitor;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.provider.MediaStore;
import android.test.InstrumentationTestCase;
import android.util.Log;

import com.android.music.tests.annotation.*;

public class MusicFunctionalAutoTestByAnnotation extends InstrumentationTestCase implements
        ServiceConnection {

    private static final String TAG = "MusicFunctionalAutoTestByAnnotation";
    private static final String TEST_PLAYLIST_NAME = "functiontest playlist";
    private static final String SONGS_LESS_THAN_TRHEE = "There are less than three songs or the" +
     " system performance is bad, please push more or reboot phone to make music pass test";
    private static final int WAIT_FOR_ACTIVITY_CREATED_TIME = 10000;
    private static final int WAIT_FOR_RESPOND_TIME = 500;
    private static final int WAIT_FOR_DELETED_TIME = 1500;
    private static final int WAIT_FOR_SERVICE_CONNECTED_TIME = 5000;
    private static final int WAIT_FOR_NEXT_PLAYER_READY = 1000;
    private static final int MINIMUM_SONGS_COUNT_IN_MUSIC = 3;
    private static final int LONG_CLICK_TIME = 5000;
    private static final long TIME_OUT = 5000;
    private static final int SLEEP_TIME = 200;

    private Instrumentation mInstrumentation = null;
    private ActivityMonitor mActivityMonitor = null;
    private Context mContext = null;
    private IMediaPlaybackService mService = null;
    private Object mLock = new Object();
    private boolean mIsMusicServiceConnected = false;
    private String[] mCursorCols;
    private final static long[] sEmptyList = new long[0];
    private static final int PLAY_MP3 = 0;
    private static final int PLAY_WAV = 1;
    private static final int PLAY_IMY = 2;
    private static final int PLAY_MIDI = 3;
    private static final int PLAY_FLAC = 4;
    private static final int PLAY_WMA = 5;
    private static final int PLAY_AAC = 6;
    private static final int PLAY_AMR = 7;
    private static final int PLAY_3GPP = 8;

    private long mCurrentSongId = -1;
    private long mNextSongId = -1;

    // / M: means 16X maybe 10X 32X
    private static final int SPEED_NORMAL = 16;
    // / M: means 40X
    private static final int SPEED_FAST = 40;
    private static final int POSITION_FOR_SPEED_FAST = 5000;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        Log.d(TAG, "setUp>>>");
        mInstrumentation = getInstrumentation();
        mContext = mInstrumentation.getTargetContext();
        // Bind service
        Intent iService = new Intent(mContext, MediaPlaybackService.class);
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
        assertNotNull(mContext);
        assertNotNull(mService);
        Log.d(TAG, "setUp<<<");
    }

    @FwkAnnotation
    public void testcase001_playPauseSeekMP3() {
        Log.d(TAG, "testcase001_playPauseSeekMp3<<<");
        String format = ".mp3";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase001_playPauseSeekMP3>>>");
    }

    @FwkAnnotation
    public void testcase002_playPauseSeekWAV() {
        Log.d(TAG, "testcase002_playPauseSeekWAV<<<");
        String format = ".wav";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase002_playPauseSeekWAV>>>");
    }

    @FwkAnnotation
    public void testcase003_playPauseSeekIMY() {
        Log.d(TAG, "testcase003_playPauseSeekIMY<<<");
        String format = ".imy";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase003_playPauseSeekIMY>>>");
    }

    @FwkAnnotation
    public void testcase004_playPauseSeekMIDI() {
        Log.d(TAG, "testcase004_playPauseSeekMIDI<<<");
        String format = ".mid";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase004_playPauseSeekMIDI>>>");
    }

    @FwkAnnotation
    public void testcase005_playPauseSeekFLAC() {
        Log.d(TAG, "testcase005_playPauseSeekFLAC<<<");
        String format = ".flac";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase005_playPauseSeekFLAC>>>");
    }

    @FwkAnnotation
    public void testcase006_playPauseSeekWMA() {
        Log.d(TAG, "testcase006_playPauseSeekWMA<<<");
        String format = ".wma";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase006_playPauseSeekWMA>>>");
    }

    @FwkAnnotation
    public void testcase007_playPauseSeekAAC() {
        Log.d(TAG, "testcase007_playPauseSeekAAC<<<");
        String format = ".aac";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase007_playPauseSeekAAC>>>");
    }

    @FwkAnnotation
    public void testcase008_playPauseSeekAMR() {
        Log.d(TAG, "testcase008_playPauseSeekAMR<<<");
        String format = ".amr";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase008_playPauseSeekAMR>>>");
    }

    @FwkAnnotation
    public void testcase009_playPauseSeek3GPP() {
        Log.d(TAG, "testcase009_playPauseSeek3GPP<<<");
        String format = ".3gpp";
        playPauseSeekAllKindsOfSongs(format);
        Log.d(TAG, "testcase009_playPauseSeek3GPP>>>");
    }

    @FwkAnnotation
    public void testcase010_switchSong() {
        Log.d(TAG, "testcase002_switchToPreSong<<<");
        mCursorCols = new String[] {
                MediaStore.Audio.Media._ID, MediaStore.Audio.Media.TITLE,
                MediaStore.Audio.Media.DATA, MediaStore.Audio.Media.ALBUM,
                MediaStore.Audio.Media.ARTIST, MediaStore.Audio.Media.ARTIST_ID,
                MediaStore.Audio.Media.DURATION,
                // / M: add cursor drm columns
                MediaStore.Audio.Media.IS_DRM, MediaStore.Audio.Media.DRM_METHOD,
                // / M: add for chinese sorting
                MediaStore.Audio.Media.DEFAULT_SORT_ORDER
        };
        Cursor c = query(mContext, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, mCursorCols,
                MediaStore.Audio.Media.IS_MUSIC + "=1", null,
                MediaStore.Audio.Genres.Members.DEFAULT_SORT_ORDER);
        playAll(mContext, c);
        c.close();

        try {
            Log.d(TAG, "before swtich next song,id:" + mService.getAudioId());
            Thread.sleep(WAIT_FOR_NEXT_PLAYER_READY);
            mService.next();
            MusicTestUtils.isPlaying(mService);
            Thread.sleep(WAIT_FOR_NEXT_PLAYER_READY);
            long nextSongId = mService.getAudioId();
            Log.d(TAG, "switch to next song,id:" + nextSongId + ",mNextSongId:" + mNextSongId);
            if (mNextSongId == -1 || nextSongId != mNextSongId) {
                String errrorMsg = "switch to next song,unequal value,mNextSongId:" + mNextSongId
                        + "nextSongId:" + nextSongId;
                fail(errrorMsg);
            }
            Log.d(TAG, "before swtich prev song,id:" + mService.getAudioId());
            mService.prev();
            assertTrue(MusicTestUtils.isPlaying(mService));
            long currentSongId = mService.getAudioId();
            Log.d(TAG, "switch to prev song,id:" + currentSongId + ",mCurrentSongId:"
                    + mCurrentSongId);
            if (currentSongId == -1 || currentSongId != mCurrentSongId) {
                String errrorMsg = "switch to prev song,unequal value,currentSongId:"
                        + currentSongId + "mCurrentSongId:" + mCurrentSongId;
                fail(errrorMsg);
            }
        } catch (Exception e) {
            e.printStackTrace();
            fail();
        }
        Log.d(TAG, "testcase002_switchToPreSong>>>");
    }

    @InternalApiAnnotation
    public void testcase011_testInternalSample() {

    }

    @ExternalApiAnnotation
    public void testcase012_testExternalSample() {

    }

    @Override
    protected void tearDown() throws Exception {
        Log.d(TAG, "tearDown>>>");
        super.tearDown();
        Log.d(TAG, "tearDown<<<");
    }

    public Cursor query(Context context, Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder, int limit) {
        try {
            ContentResolver resolver = context.getContentResolver();
            if (resolver == null) {
                return null;
            }
            if (limit > 0) {
                uri = uri.buildUpon().appendQueryParameter("limit", "" + limit).build();
            }
            return resolver.query(uri, projection, selection, selectionArgs, sortOrder);
        } catch (UnsupportedOperationException ex) {
            return null;
        }

    }

    public Cursor query(Context context, Uri uri, String[] projection, String selection,
            String[] selectionArgs, String sortOrder) {
        return query(context, uri, projection, selection, selectionArgs, sortOrder, 0);
    }

    public long[] getSongListForCursor(Cursor cursor) {
        if (cursor == null) {
            return sEmptyList;
        }
        int len = cursor.getCount();
        long[] list = new long[len];
        cursor.moveToFirst();
        int colidx = -1;
        try {
            colidx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Playlists.Members.AUDIO_ID);
        } catch (IllegalArgumentException ex) {
            colidx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID);
        }
        for (int i = 0; i < len; i++) {
            list[i] = cursor.getLong(colidx);
            cursor.moveToNext();
        }
        return list;
    }

    public void playAll(Context context, Cursor cursor) {
        playAll(context, cursor, 0, false);
    }

    private void playAll(Context context, Cursor cursor, int position, boolean force_shuffle) {

        long[] list = getSongListForCursor(cursor);
        Log.d(TAG, "list length:" + list.length + ",positon:" + position);
        mCurrentSongId = list[position];
        if (list.length > 1) {
            mNextSongId = list[position + 1];
        }
        Log.d(TAG, "mCurrentSongId:" + mCurrentSongId + ",mNextSongId:" + mNextSongId);
        playAll(context, list, position, force_shuffle);
    }

    private void playAll(Context context, long[] list, int position, boolean force_shuffle) {
        if (list.length == 0 || mService == null) {
            Log.d(TAG, "attempt to play empty song list");
            return;
        }
        try {
            if (position < 0) {
                position = 0;
            }
            mService.open(list, force_shuffle ? -1 : position);
            mService.play();
        } catch (RemoteException ex) {
        } finally {
            // / M: launch assigned activity
            /*
             * Intent intent = new Intent().setClass(context,
             * MediaPlaybackActivity.class)
             * .setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
             * context.startActivity(intent);
             */
        }
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

    public void playPauseSeekAllKindsOfSongs(String format) {
        Log.d(TAG, "playPauseSeekAllKindsOfSongs(" + format + ")<<<");
        mCursorCols = new String[] {
                MediaStore.Audio.Media._ID, MediaStore.Audio.Media.TITLE,
                MediaStore.Audio.Media.DATA, MediaStore.Audio.Media.ALBUM,
                MediaStore.Audio.Media.ARTIST, MediaStore.Audio.Media.ARTIST_ID,
                MediaStore.Audio.Media.DURATION,
                // / M: add cursor drm columns
                MediaStore.Audio.Media.IS_DRM, MediaStore.Audio.Media.DRM_METHOD,
                // / M: add for chinese sorting
                MediaStore.Audio.Media.DEFAULT_SORT_ORDER
        };
        if (!format.equals("")) {
            Cursor c = query(mContext, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, mCursorCols,
                    MediaStore.Audio.Media.DATA + " LIKE '%" + format + "'", null,
                    MediaStore.Audio.Genres.Members.DEFAULT_SORT_ORDER);
            String failMessage = "the song with format:" + format + " is not existed in db";
            String successMessage = "";
            assertTrue(failMessage, c != null);
            assertTrue(failMessage, c.getCount() > 0);
            playAll(mContext, c);
            c.close();
            if (!MusicTestUtils.isPlaying(mService)) {
                failMessage = "play " + format + " failed";
                fail(failMessage);
            } else {
                successMessage = "play " + format + " success,now do pause";
                Log.d(TAG, successMessage);
                try {
                    mService.pause();
                } catch (Exception e) {
                    Log.e(TAG, "pause exception");
                }
                if (!MusicTestUtils.isStopping(mService)) {
                    failMessage = "pause " + format + " failed";
                    fail(failMessage);
                } else {
                    Log.d(TAG, "success pause " + format + ",now do seek");
                    try {
                        if (mService.seek(1000) != 0 && mService.position() == 1000) {
                            Log.d(TAG, "success seek " + format);
                        } else {
                            failMessage = "seek " + format + " failed";
                            fail(failMessage);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        fail();
                    }
                }
            }
        }
        Log.d(TAG, "playPauseSeekAllKindsOfSongs(" + format + ")>>>");
    }

}
