/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.music;

import android.Manifest;
import android.app.ActionBar;
import android.app.Activity;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.ListActivity;
import android.app.SearchManager;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.database.AbstractCursor;
import android.database.CharArrayBuffer;
import android.database.Cursor;
import android.drm.DrmManagerClient;
import android.drm.DrmStore;
import android.graphics.Bitmap;
import android.media.AudioManager;
import android.media.audiofx.AudioEffect;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;
import android.provider.MediaStore;
import android.provider.MediaStore.Audio.Playlists;
import android.text.TextUtils;
//Not Required
//import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.AdapterView.AdapterContextMenuInfo;
import android.widget.AlphabetIndexer;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.SectionIndexer;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;


import com.android.music.MusicUtils.ServiceToken;
//import com.mediatek.drm.OmaDrmClient;
import com.mediatek.omadrm.OmaDrmUtils;
//import com.mediatek.drm.OmaDrmUiUtils;
import com.mediatek.media.MtkMediaStore;



//Not Required
//import java.text.Collator;
import java.util.Arrays;
import android.app.NotificationManager;
import android.app.NotificationChannel;


public class TrackBrowserActivity extends ListActivity
        implements View.OnCreateContextMenuListener, MusicUtils.Defs,
         ServiceConnection, DialogInterface.OnClickListener
{
    private static final int Q_SELECTED = CHILD_MENU_BASE;
    private static final int Q_ALL = CHILD_MENU_BASE + 1;
    private static final int SAVE_AS_PLAYLIST = CHILD_MENU_BASE + 2;
    private static final int PLAY_ALL = CHILD_MENU_BASE + 3;
    private static final int CLEAR_PLAYLIST = CHILD_MENU_BASE + 4;
    private static final int REMOVE = CHILD_MENU_BASE + 5;
    private static final int SEARCH = CHILD_MENU_BASE + 6;
    private static final String DIALOG_TAG_CREATE = "Create";
    private static final String DIALOG_TAG_DELETE = "Delete";
	private static final String PLAYLIST_NAME = "playlistname";
	private static final String PLAYLIST_LEN = "playlistlen";
    /// M: It's the delay time(ms) for show emptyview.
    private int EMPTYSHOW_SPEND = 200;

    private static final String TAG = "TrackBrowser";
    private String[] mCursorCols;
    private String[] mPlaylistMemberCols;
    private boolean mDeletedOneRow = false;
    private boolean mEditMode = false;
    private String mCurrentTrackName;
    private String mCurrentAlbumName;
    private String mCurrentArtistNameForAlbum;
    private ListView mTrackList;
    private Cursor mTrackCursor;
    private TrackListAdapter mAdapter;
    private boolean mAdapterSent = false;
    private String mAlbumId;
    private String mArtistId;
    private String mPlaylist;
    private String mGenre;
    private String mSortOrder;
    private int mSelectedPosition;
    private long mSelectedId;
    private static int mLastListPosCourse = -1;
    private static int mLastListPosFine = -1;
    //Not Required
    //private boolean mUseLastListPos = false;
    private ServiceToken mToken;

    /// M: store the bitmap image decode from file to show it when switch screen.
    private Bitmap mAlbumArtBitmap = null;

    /// M: async task for getting album art.
    private AlbumArtFetcher mAsyncAlbumArtFetcher = null;

    /// M: reset SD card status to refresh database error UI.
    private boolean mResetSdStatus = false;

    /// M: get the instance of IMediaPlaybackService,when bind service could call
    /// functions that included IMediaPlaybackService.
    private IMediaPlaybackService mService = null;

    /// M: drm client for drm right checking.
    private DrmManagerClient mDrmClient = null;

    /// M: track position which is selected by user.
    private int mCurTrackPos = -1;

    /// M: record mount status to avoid meaningless mount action.
    private boolean mIsMounted = true;

    /// M: enter this Activity with tabs or other ways.
    private boolean mWithtabs = false;
    /// M: record the screen orientation for nowplaying update.
    private int mOrientation;

    /// M: aviod Navigation button respond JE if Activity is background.
    private boolean mIsInBackgroud = false;

    /// M: We need to close submenu when sdcard is unmounted
    private SubMenu mSubMenu = null;
    /// M: We need to manage menu item when sdcards umounted
    private Menu mOptionMenu = null;
    /// M: Add search button in actionbar when nowplaying not exist
    MenuItem mSearchItem;
    /// M: Use member variable to show toast to avoid show the toast
    // on screen for a long time if user click many time.
    private Toast mToast;
    /// M: save the input of SearchView
    private CharSequence mQueryText;
    private String mPlayListName;

    private boolean mActvityResumAfterCreate = true;
    public static Long DeletedItemId = -1L;
    private Long SelectedDelId = -1L;
    public TrackBrowserActivity()
    {
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle)
    {
        super.onCreate(icicle);
        MusicLogUtils.v(TAG, "onCreate");
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        Intent intent = getIntent();
        if (intent != null) {
            /// M: if enter this Activity is with tabs, disable action bar,
            // otherwise,set the action bar on the right to be up navigation @{
            mWithtabs = intent.getBooleanExtra("withtabs", false);
            if (mWithtabs) {
                requestWindowFeature(Window.FEATURE_NO_TITLE);
            } else {
                ActionBar actionBar = getActionBar();
                actionBar.setDisplayHomeAsUpEnabled(true);
            /// @}
            }
        }

        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        if (icicle != null) {
            mSelectedId = icicle.getLong("selectedtrack");
            mAlbumId = icicle.getString("album");
            mArtistId = icicle.getString("artist");
            mPlaylist = icicle.getString("playlist");
            mGenre = icicle.getString("genre");
            mEditMode = icicle.getBoolean("editmode", false);
            /// M: get the current track positionfrom bundle
            mCurTrackPos = icicle.getInt("curtrackpos");
        } else {
            /// M: ALPS00122166.
            mSelectedId = intent.getLongExtra("selectedtrack", 0);

            mAlbumId = intent.getStringExtra("album");
            // If we have an album, show everything on the album, not just stuff
            // by a particular artist.
            mArtistId = intent.getStringExtra("artist");
            mPlaylist = intent.getStringExtra("playlist");
            mGenre = intent.getStringExtra("genre");
            mEditMode = intent.getAction().equals(Intent.ACTION_EDIT);
        }

        mCursorCols = new String[] {
                MediaStore.Audio.Media._ID,
                MediaStore.Audio.Media.TITLE,
                MediaStore.Audio.Media.DATA,
                MediaStore.Audio.Media.ALBUM,
                MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.ARTIST_ID,
                MediaStore.Audio.Media.DURATION,
                /// M: add cursor drm columns
                MediaStore.Audio.Media.IS_DRM,

        };
        mPlaylistMemberCols = new String[] {
                MediaStore.Audio.Playlists.Members._ID,
                MediaStore.Audio.Media.TITLE,
                MediaStore.Audio.Media.DATA,
                MediaStore.Audio.Media.ALBUM,
                MediaStore.Audio.Media.ARTIST,
                MediaStore.Audio.Media.ARTIST_ID,
                MediaStore.Audio.Media.DURATION,
                MediaStore.Audio.Playlists.Members.PLAY_ORDER,
                MediaStore.Audio.Playlists.Members.AUDIO_ID,
                MediaStore.Audio.Media.IS_MUSIC,
                /// M: add cursor drm columns
                MediaStore.Audio.Media.IS_DRM,

        };
        /// M: if enter current activity with tab mode or edit mode,
        // set layout without nowplaying in the bottom. @{
        if (!mWithtabs){
			NotificationManager mgr = (NotificationManager)this.
              getSystemService(Context.NOTIFICATION_SERVICE);
              mgr.createNotificationChannel(new NotificationChannel(
              MusicBrowserActivity.MUSIC_NOTIFICATION_CHANNEL, 
              "MUSIC", NotificationManager.IMPORTANCE_LOW));
		}
        if (mWithtabs || mEditMode) {
            setContentView(R.layout.media_picker_activity);
        } else {
            setContentView(R.layout.media_picker_activity_nowplaying);
            mOrientation = getResources().getConfiguration().orientation;
        }
        /// @}
        /// M: it dosen't need to bind service in tab mode, the relative thing
        /// move to MusicBrowser @{
        if (!mWithtabs) {
            mToken = MusicUtils.bindToService(this, this);
        }
        /// @}
        mTrackList = getListView();
        mTrackList.setOnCreateContextMenuListener(this);
        mTrackList.setCacheColorHint(0);
        if (mEditMode) {
            ((TouchInterceptor) mTrackList).setDropListener(mDropListener);
            ((TouchInterceptor) mTrackList).setRemoveListener(mRemoveListener);
            /// M: Register the observer to TouchInterceptor.
            ((TouchInterceptor) mTrackList).registerContentObserver(getApplicationContext());
            /// M: Marked for ICS style.
            //mTrackList.setDivider(null);
            //mTrackList.setSelector(R.drawable.list_selector_background);
        } else {
            mTrackList.setTextFilterEnabled(true);
        }
        //Not Required
        //mAdapter = (TrackListAdapter) getLastNonConfigurationInstance();

        /// M: When in below mode(such as show album or artist list),
        // we need show album art backgound and set
        /// a listener to upgrade it at on size change finish.
        if (getApplicationContext()
          .checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
          == PackageManager.PERMISSION_GRANTED) {
           MusicLogUtils.d(TAG, "onCreate AlbumArt Permission Granted");
           if (!mWithtabs && !mEditMode) {
            /// M: When use trackbrowser to show album or artist, set album art upgrade listener.
            ((TouchInterceptor) mTrackList).setUpgradeAlbumArtListener(mUpgradeAlbumArtListener);
            /// M: use async task to get the Album art for background;
            setAlbumArtBackground();
           }
        }
        //Not required
        //mToken = MusicUtils.bindToService(this, this);

        /// M: Creates a DrmManagerClient @{
        if (OmaDrmUtils.isOmaDrmEnabled()) {
            mDrmClient = new DrmManagerClient(this);
        }
        /// @}

        /// M: initialize adapter;
        initAdapter();

        /// M: the below statements move form onServiceConnected(), becase the
        // activity may not connect service in tab mode.
        IntentFilter f = new IntentFilter();
        f.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        f.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        f.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        /// M: add action for Scanning sdcard @{
        f.addAction(Intent.ACTION_MEDIA_MOUNTED);
        /// @}
        f.addDataScheme("file");
        registerReceiver(mScanListener, f);
//Not required
    }

    public void onServiceConnected(ComponentName name, IBinder service)
    {
        /// M: get the object of IMediaPlaybackService and update the
        // nowplaying item.
        mService = IMediaPlaybackService.Stub.asInterface(service);
        MusicUtils.updateNowPlaying(this, mOrientation);
    }

    public void onServiceDisconnected(ComponentName name) {
        // we can't really function without the service, so don't
        /// M: clear service reference.
        mService = null;
        finish();
    }

    @Override
    public Object onRetainNonConfigurationInstance() {
        TrackListAdapter a = mAdapter;
        mAdapterSent = true;
        return a;
    }

    @Override
    public void onDestroy() {
        MusicLogUtils.v(TAG, "onDestroy");
        /// M: cancel the async task firstly.
        if (mAsyncAlbumArtFetcher != null && !mAsyncAlbumArtFetcher.isCancelled()) {
            mAsyncAlbumArtFetcher.cancel(true);
        }
        ListView lv = getListView();
        if (lv != null) {
            /// M: use mWithtabs instead of mUseLastListPos.
            if (mWithtabs) {
                mLastListPosCourse = lv.getFirstVisiblePosition();
                View cv = lv.getChildAt(0);
                if (cv != null) {
                    mLastListPosFine = cv.getTop();
                }
            }
            if (mEditMode) {
                // clear the listeners so we won't get any more callbacks
                ((TouchInterceptor) lv).setDropListener(null);
                ((TouchInterceptor) lv).setRemoveListener(null);
                /// M: unregister the observer in TouchInterceptor when activity ondestroy
                ((TouchInterceptor) lv).unregisterContentObserver(getApplicationContext());
            }
        }
        MusicUtils.unbindFromService(mToken);
        /// M: clear service reference.
        mService = null;
        try {
            if ("nowplaying".equals(mPlaylist)) {
                unregisterReceiverSafe(mNowPlayingListener);
            } else {
                unregisterReceiverSafe(mTrackListListener);
            }
        } catch (IllegalArgumentException ex) {
            // we end up here in case we never registered the listeners
        }

        // If we have an adapter and didn't send it off to another activity yet, we should
        // close its cursor, which we do by assigning a null cursor to it. Doing this
        // instead of closing the cursor directly keeps the framework from accessing
        // the closed cursor later.
        if (!mAdapterSent && mAdapter != null) {
            mAdapter.changeCursor(null);
        }
        // Because we pass the adapter to the next activity, we need to make
        // sure it doesn't keep a reference to this activity. We can do this
        // by clearing its DatasetObservers, which setListAdapter(null) does.
        setListAdapter(null);
        mAdapter = null;
        unregisterReceiverSafe(mScanListener);
        if (mDrmClient != null) {
            mDrmClient.release();
            mDrmClient = null;
        }
        super.onDestroy();
    }

    /**
     * Unregister a receiver, but eat the exception that is thrown if the
     * receiver was never registered to begin with. This is a little easier
     * than keeping track of whether the receivers have actually been
     * registered by the time onDestroy() is called.
     */
    private void unregisterReceiverSafe(BroadcastReceiver receiver) {
        try {
            unregisterReceiver(receiver);
        } catch (IllegalArgumentException e) {
            // ignore
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        MusicLogUtils.v(TAG, "onResume>>>");
        if (mTrackCursor != null) {
            /// M: Fix in TouchInterceptor page can't update track name
            // when database change @{
            if (mEditMode || (mPlaylist != null && mPlaylist.equals("recentlyadded"))) {
                MusicLogUtils.v(TAG, "need to requery data!");
                getTrackCursor(mAdapter.getQueryHandler(), null, true);
            }
            /// @}
            getListView().invalidateViews();
        }
        MusicUtils.setSpinnerState(this);
        /// M: aviod Navigation button respond JE if Activity is background
        mIsInBackgroud = false;
        /// M: update the nowplaying item @{
        if (mService != null) {
            MusicUtils.updateNowPlaying(this, mOrientation);
        }
        /// @}
        /// M: when the listview's count be 0,show the emptyview.
        if (!mActvityResumAfterCreate) {
            refreshEmptyView();
        }
        MusicLogUtils.v(TAG, "onResume<<<");
    }

    /**
     *  M: show the emptyview when the list's count be 0.
     */
    public void refreshEmptyView() {
        if (getListView().getAdapter() != null
                && getListView().getAdapter().getCount() == 0
                && getListView().getCount() != 0) {
            MusicLogUtils.v(TAG, "getExpandableListView().getCount() = "
                    + getListView().getCount());
            mListHandler.sendEmptyMessageDelayed(0, EMPTYSHOW_SPEND);
        } else {
            MusicUtils.emptyShow(getListView(), TrackBrowserActivity.this);
        }
    }

    @Override
    public void onPause() {
        MusicLogUtils.v(TAG, "onPause");
        mActvityResumAfterCreate = false;
        if (mSubMenu != null) {
            mSubMenu.close();
        }
        mReScanHandler.removeCallbacksAndMessages(null);
        /// M: ALPS00122166, save the current item information @{
        Intent i = new Intent(getIntent());
        i.putExtra("selectedtrack", mSelectedId);
        i.putExtra("curtrackpos", mCurTrackPos);
        setIntent(i);
        /// @}
        /// M: aviod Navigation button respond JE if Activity is background
        mIsInBackgroud = true;
        super.onPause();
    }

    /*
     * This listener gets called when the media scanner starts up or finishes, and
     * when the sd card is unmounted.
     * M: receive more action:MEDIA_UNMOUNTED and MEDIA_EJECT for SD card
     * hotplug.
     */
    private BroadcastReceiver mScanListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            /// M: get the action and current Storage State.
            String status = Environment.getExternalStorageState();
            MusicLogUtils.v(TAG, "mScanListener.onReceive:" + action + ", status = " + status);
            if (Intent.ACTION_MEDIA_SCANNER_STARTED.equals(action) ||
                    Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
            } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                /// M: set SAVE_AS_PLAYLIST invisible when all sdcards eject
                if (!MusicUtils.hasMountedSDcard(getApplicationContext()) && mOptionMenu != null
                        && mOptionMenu.findItem(SAVE_AS_PLAYLIST) != null) {
                    mOptionMenu.findItem(SAVE_AS_PLAYLIST).setVisible(false);
                }
                mIsMounted = false;
                mResetSdStatus = true;
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
                closeContextMenu();
                closeOptionsMenu();
                if (mSubMenu != null) {
                    mSubMenu.close();
                }
                mReScanHandler.sendEmptyMessageDelayed(0, 1000);
            } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                /// M: set SAVE_AS_PLAYLIST visible when sdcard mounted
                if (mOptionMenu != null && mOptionMenu.findItem(SAVE_AS_PLAYLIST) != null) {
                    mOptionMenu.findItem(SAVE_AS_PLAYLIST).setVisible(true);
                }
                /// M: if SD Card is Mounted,send Message to rescan database.
                mIsMounted = true;
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
            }
            //Not required
            //mReScanHandler.sendEmptyMessage(0);
        }
    };

    private Handler mReScanHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (mAdapter != null) {
                getTrackCursor(mAdapter.getQueryHandler(), null, true);
            }
            // if the query results in a null cursor, onQueryComplete() will
            // call init(), which will post a delayed message to this handler
            // in order to try again.
        }
    };

    /**
     *  M: show emptyview when get the right list'count.
     */
    public Handler mListHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            refreshEmptyView();
            }
    };

    public void onSaveInstanceState(Bundle outcicle) {
        // need to store the selected item so we don't lose it in case
        // of an orientation switch. Otherwise we could lose it while
        // in the middle of specifying a playlist to add the item to.
        outcicle.putLong("selectedtrack", mSelectedId);
        outcicle.putString("artist", mArtistId);
        outcicle.putString("album", mAlbumId);
        outcicle.putString("playlist", mPlaylist);
        outcicle.putString("genre", mGenre);
        outcicle.putBoolean("editmode", mEditMode);
        /// M: save the track position.
        outcicle.putInt("curtrackpos", mCurTrackPos);
        super.onSaveInstanceState(outcicle);
    }

    public void init(Cursor newCursor, boolean isLimited) {

        if (mAdapter == null) {
            return;
        }
        mAdapter.changeCursor(newCursor); // also sets mTrackCursor

        if (mTrackCursor == null) {
            /// M: reset SD card status to refresh database error UI. @{
            if (mResetSdStatus) {
                MusicUtils.resetSdStatus();
                mResetSdStatus = false;
            }
            /// @}
            /// M: add sdcard mount status to avoid meaningless mount action.
            MusicUtils.displayDatabaseError(this, mIsMounted);
            closeContextMenu();
            mReScanHandler.sendEmptyMessageDelayed(0, 1000);
            return;
        }
        /// M: when Scanning media file show scanning progressbar ,if files exist
        //   show file list,or show no music
        MusicUtils.emptyShow(getListView(), TrackBrowserActivity.this);
        MusicUtils.hideDatabaseError(this);
        /// M: move buttone bar to Musicbrowser.
        //mUseLastListPos = MusicUtils.updateButtonBar(this, R.id.songtab);
        setTitle();

        // Restore previous position
        /// M: use mWithtabs isstead of mUseLastListPos.
        if (mLastListPosCourse >= 0 && mWithtabs) {
            ListView lv = getListView();
            // this hack is needed because otherwise the position doesn't change
            // for the 2nd (non-limited) cursor
            lv.setAdapter(lv.getAdapter());
            lv.setSelectionFromTop(mLastListPosCourse, mLastListPosFine);
            if (!isLimited) {
                mLastListPosCourse = -1;
            }
        }

        // When showing the queue, position the selection on the currently playing track
        // Otherwise, position the selection on the first matching artist, if any
        IntentFilter f = new IntentFilter();
        f.addAction(MediaPlaybackService.META_CHANGED);
        f.addAction(MediaPlaybackService.QUEUE_CHANGED);
        if ("nowplaying".equals(mPlaylist)) {
            try {
                f.addAction(MediaPlaybackService.QUIT_PLAYBACK);
                int cur = MusicUtils.sService.getQueuePosition();
                setSelection(cur);
                registerReceiver(mNowPlayingListener, new IntentFilter(f));
                mNowPlayingListener.onReceive(this, new Intent(MediaPlaybackService.META_CHANGED));
            } catch (RemoteException ex) {
            }
        } else {
            String key = getIntent().getStringExtra("artist");
            if (key != null) {
                int keyidx = mTrackCursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ARTIST_ID);
                mTrackCursor.moveToFirst();
                while (! mTrackCursor.isAfterLast()) {
                    String artist = mTrackCursor.getString(keyidx);
                    if (artist.equals(key)) {
                        setSelection(mTrackCursor.getPosition());
                        break;
                    }
                    mTrackCursor.moveToNext();
                }
            }
            registerReceiver(mTrackListListener, new IntentFilter(f));
            mTrackListListener.onReceive(this, new Intent(MediaPlaybackService.META_CHANGED));
        }
    }

    /*
     * M: set album art for the listviewbackground by async task.
     */
    private void setAlbumArtBackground() {
        //if (!mEditMode) {
        //    try {
        //        long albumid = Long.valueOf(mAlbumId);
        //        Bitmap bm = MusicUtils.getArtwork(TrackBrowserActivity.this, -1, albumid, false);
        //        if (bm != null) {
        //            MusicUtils.setBackground(mTrackList, bm);
        //            mTrackList.setCacheColorHint(0);
        //            return;
        //        }
        //    } catch (Exception ex) {
        //    }
        //}
        //mTrackList.setBackgroundColor(0xff000000);
        //mTrackList.setCacheColorHint(0);

        MusicLogUtils.v(TAG, "setAlbumArtBackground: mAlbumId = "
         + mAlbumId + ", mAlbumArtBitmap = " + mAlbumArtBitmap);
        /// M: Cancel the old one and execute new task to set background.
        if (mAlbumId == null) {
            return;
        }
        /// M: If album art has existed, just need to set it
        /// to background when acitity is in forcegournd.
        if (mAlbumArtBitmap != null) {
            MusicUtils.setBackground((View) mTrackList.getParent().getParent(), mAlbumArtBitmap);
            mTrackList.setCacheColorHint(0);
            return;
        }
        if (mAsyncAlbumArtFetcher != null && !mAsyncAlbumArtFetcher.isCancelled()) {
            mAsyncAlbumArtFetcher.cancel(true);
        }
        mAsyncAlbumArtFetcher = new AlbumArtFetcher();
        try {
            mAsyncAlbumArtFetcher.execute(Long.valueOf(mAlbumId));
        } catch (IllegalStateException ex) {
            MusicLogUtils.v(TAG, "Exception while fetching album art!!", ex);
        }
        //Not Required
        //mTrackList.setBackgroundColor(0xff000000);
        //mTrackList.setCacheColorHint(0);
    }

    private void setTitle() {

        CharSequence fancyName = null;
        if (mAlbumId != null) {
            int numresults = mTrackCursor != null ? mTrackCursor.getCount() : 0;
            if (numresults > 0) {
                mTrackCursor.moveToFirst();
                int idx = mTrackCursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ALBUM);
                fancyName = mTrackCursor.getString(idx);
                // For compilation albums show only the album title,
                // but for regular albums show "artist - album".
                // To determine whether something is a compilation
                // album, do a query for the artist + album of the
                // first item, and see if it returns the same number
                // of results as the album query.
                /// M: Adjust the condition to get cursor. @{
                String where = MediaStore.Audio.Media.ALBUM_ID + "=?  AND "
                + MediaStore.Audio.Media.ARTIST_ID + "=? ";
                String[] whereArgs = new String[]{
                        String.valueOf(mAlbumId),
                        String.valueOf(mTrackCursor.getLong(mTrackCursor.getColumnIndexOrThrow(
                                MediaStore.Audio.Media.ARTIST_ID)))};
                Cursor cursor = MusicUtils.query(this, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                    new String[] {MediaStore.Audio.Media.ALBUM}, where, whereArgs, null);
                /// @}
                if (cursor != null) {
                    if (cursor.getCount() != numresults) {
                        // compilation album
                        fancyName = mTrackCursor.getString(idx);
                    }
                    /// M: close cursor instead of deactive.
                    cursor.close();
                }
                if (fancyName == null || fancyName.equals(MediaStore.UNKNOWN_STRING)) {
                    fancyName = getString(R.string.unknown_album_name);
                }
            }
        } else if (mPlaylist != null) {
            if (mPlaylist.equals("nowplaying")) {
                if (MusicUtils.getCurrentShuffleMode() == MediaPlaybackService.SHUFFLE_AUTO) {
                    fancyName = getText(R.string.partyshuffle_title);
                } else {
                    fancyName = getText(R.string.nowplaying_title);
                }
            } else if (mPlaylist.equals("podcasts")) {
                fancyName = getText(R.string.podcasts_title);
            } else if (mPlaylist.equals("recentlyadded")) {
                fancyName = getText(R.string.recentlyadded_title);
            } else {
                String [] cols = new String [] {
                MediaStore.Audio.Playlists.NAME
                };
                Cursor cursor = MusicUtils.query(this,
                        ContentUris.withAppendedId(Playlists.EXTERNAL_CONTENT_URI,
                        Long.valueOf(mPlaylist)),
                        cols, null, null, null);
                if (cursor != null) {
                    if (cursor.getCount() != 0) {
                        cursor.moveToFirst();
                        fancyName = cursor.getString(0);
                    }
                    /// M: close cursor instead of deactive.
                    cursor.close();
                }
            }
            /// M: store playlist name
            if (fancyName != null) {
                mPlayListName = fancyName.toString();
            }
        } else if (mGenre != null) {
            String [] cols = new String [] {
            MediaStore.Audio.Genres.NAME
            };
            Cursor cursor = MusicUtils.query(this,
                    ContentUris.withAppendedId(MediaStore.Audio.Genres.EXTERNAL_CONTENT_URI,
                    Long.valueOf(mGenre)),
                    cols, null, null, null);
            if (cursor != null) {
                if (cursor.getCount() != 0) {
                    cursor.moveToFirst();
                    fancyName = cursor.getString(0);
                }
                /// M: close cursor instead of deactive.
                cursor.close();
            }
        }
        /// M: in tab mode which has not title, so no need to set.
        if (!mWithtabs) {
            if (fancyName != null) {
                setTitle(fancyName);
            } else {
                setTitle(R.string.tracks_title);
            }
        }
    }

    private TouchInterceptor.DropListener mDropListener =
        new TouchInterceptor.DropListener() {
        public void drop(int from, int to) {
            if (mTrackCursor instanceof NowPlayingCursor) {
                // update the currently playing list
                NowPlayingCursor c = (NowPlayingCursor) mTrackCursor;
                c.moveItem(from, to);
                ((TrackListAdapter) getListAdapter()).notifyDataSetChanged();
                /// M: if drag nowplaying playlist, need not stop draw the view,
                /// so reset status to pre-draw again.
                ((TouchInterceptor) mTrackList).resetPredrawStatus();
                getListView().invalidateViews();
                mDeletedOneRow = true;
            } else {
                // update a saved playlist, success
                boolean isSuccesss = MediaStore.Audio.Playlists.Members.moveItem
                (getContentResolver(), Long.valueOf(mPlaylist), from, to);
                /// M: if move item fail, we need reset predraw status.
                if (!isSuccesss) {
                    ((TouchInterceptor) mTrackList).resetPredrawStatus();
                }
                MusicLogUtils.v(TAG, "drop: from = " + from + ", to = " + to +
                ", isSuccesss = " + isSuccesss);
                /// @}
            }
        }
    };

    private TouchInterceptor.RemoveListener mRemoveListener =
        new TouchInterceptor.RemoveListener() {
        public void remove(int which) {
            removePlaylistItem(which);
        }
    };

    private void removePlaylistItem(int which) {
        View v = mTrackList.getChildAt(which - mTrackList.getFirstVisiblePosition());
        if (v == null) {
            MusicLogUtils.v(TAG, "No view when removing playlist item " + which);
            return;
        }
        try {
            if (MusicUtils.sService != null
                    && which != MusicUtils.sService.getQueuePosition()) {
                mDeletedOneRow = true;
            }
        } catch (RemoteException e) {
            // Service died, so nothing playing.
            mDeletedOneRow = true;
        }
        v.setVisibility(View.GONE);
        mTrackList.invalidateViews();
        if (mTrackCursor instanceof NowPlayingCursor) {
            ((NowPlayingCursor) mTrackCursor).removeItem(which);
            ((TrackListAdapter) getListAdapter()).notifyDataSetChanged();
        } else {
            int colidx = mTrackCursor.getColumnIndexOrThrow(
                    MediaStore.Audio.Playlists.Members._ID);
            mTrackCursor.moveToPosition(which);
            long id = mTrackCursor.getLong(colidx);
            Uri uri = MediaStore.Audio.Playlists.Members.getContentUri("external",
                    Long.valueOf(mPlaylist));
            getContentResolver().delete(
                    ContentUris.withAppendedId(uri, id), null, null);
        }
        v.setVisibility(View.VISIBLE);
        mTrackList.invalidateViews();
    }

    private BroadcastReceiver mTrackListListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            getListView().invalidateViews();
            /// M: only update when service is available and param for oriention. @{
            if (mService != null) {
                MusicUtils.updateNowPlaying(TrackBrowserActivity.this, mOrientation);
            }
            /// @}
        }
    };

    private BroadcastReceiver mNowPlayingListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals(MediaPlaybackService.META_CHANGED)) {
                getListView().invalidateViews();
            } else if (intent.getAction().equals(MediaPlaybackService.QUEUE_CHANGED)) {
                if (mDeletedOneRow) {
                    // This is the notification for a single row that was
                    // deleted previously, which is already reflected in
                    // the UI.
                    mDeletedOneRow = false;
                    return;
                }
                // The service could disappear while the broadcast was in flight,
                // so check to see if it's still valid
                if (MusicUtils.sService == null) {
                    finish();
                    return;
                }
                if (mAdapter != null) {
                    Cursor c = new NowPlayingCursor(MusicUtils.sService, mCursorCols);
                    if (c.getCount() == 0) {
                        finish();
                        return;
                    }
                    mAdapter.changeCursor(c);
                }
            } else if (intent.getAction().equals(MediaPlaybackService.QUIT_PLAYBACK)) {
                finish();
            }
        }
    };

    // Cursor should be positioned on the entry to be checked
    // Returns false if the entry matches the naming pattern used for recordings,
    // or if it is marked as not music in the database.
    private boolean isMusic(Cursor c) {
        int titleidx = c.getColumnIndex(MediaStore.Audio.Media.TITLE);
        int albumidx = c.getColumnIndex(MediaStore.Audio.Media.ALBUM);
        int artistidx = c.getColumnIndex(MediaStore.Audio.Media.ARTIST);

        String title = c.getString(titleidx);
        String album = c.getString(albumidx);
        String artist = c.getString(artistidx);
        if (MediaStore.UNKNOWN_STRING.equals(album) &&
                MediaStore.UNKNOWN_STRING.equals(artist) &&
                title != null &&
                title.startsWith("recording")) {
            // not music
            return false;
        }

        int ismusic_idx = c.getColumnIndex(MediaStore.Audio.Media.IS_MUSIC);
        boolean ismusic = true;
        if (ismusic_idx >= 0) {
            ismusic = mTrackCursor.getInt(ismusic_idx) != 0;
        }
        return ismusic;
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View view, ContextMenuInfo menuInfoIn) {
        menu.add(0, PLAY_SELECTION, 0, R.string.play_selection);
        mSubMenu = menu.addSubMenu(0, ADD_TO_PLAYLIST, 0, R.string.add_to_playlist);
        MusicUtils.makePlaylistMenu(this, mSubMenu);
        if (mEditMode) {
            menu.add(0, REMOVE, 0, R.string.remove_from_playlist);
        }
        //Design changed
        //menu.add(0, USE_AS_RINGTONE, 0, R.string.ringtone_menu);
        //menu.add(0, DELETE_ITEM, 0, R.string.delete_item);
        AdapterContextMenuInfo mi = (AdapterContextMenuInfo) menuInfoIn;
        mSelectedPosition =  mi.position;
        mTrackCursor.moveToPosition(mSelectedPosition);
        try {
            int id_idx = mTrackCursor.getColumnIndexOrThrow(
                    MediaStore.Audio.Playlists.Members.AUDIO_ID);
            mSelectedId = mTrackCursor.getLong(id_idx);
        } catch (IllegalArgumentException ex) {
            mSelectedId = mi.id;
        }
        /**M: disable the setRingTone menu item while current user is not owner user.@{**/
        int isDrm = 0;
        isDrm = mTrackCursor.getInt(mTrackCursor.getColumnIndexOrThrow
                                    (MediaStore.Audio.Media.IS_DRM));
        boolean isVoiceCapable = MusicUtils.isVoiceCapable(this);
        if (UserHandle.myUserId() == UserHandle.USER_OWNER) {
            if (OmaDrmUtils.isOmaDrmEnabled()) {
                int drmMethod = 0;//mTrackCursor.getInt(mTrackCursor.getColumnIndexOrThrow
//                (MtkMediaStore.MediaColumns.DRM_METHOD));
                if (isVoiceCapable && canDispalyRingtone(isDrm, drmMethod)) {
                    menu.add(0, USE_AS_RINGTONE, 0, R.string.ringtone_menu);
                }
            } else {
                if (isVoiceCapable) {
                    menu.add(0, USE_AS_RINGTONE, 0, R.string.ringtone_menu);
                }
            }
        }

        /**@}**/
        /// M: move DELETE_ITEM to here, before use as
        // ringtone, need check the drm right if the song is protect by drm. @{
        menu.add(0, DELETE_ITEM, 0, R.string.delete_item);
        /// @}
        // only add the 'search' menu if the selected item is music
        if (isMusic(mTrackCursor)) {
            menu.add(0, SEARCH, 0, R.string.search_title);
        }
        mCurrentAlbumName = mTrackCursor.getString(mTrackCursor.getColumnIndexOrThrow(
                MediaStore.Audio.Media.ALBUM));
        mCurrentArtistNameForAlbum = mTrackCursor.getString(mTrackCursor.getColumnIndexOrThrow(
                MediaStore.Audio.Media.ARTIST));
        mCurrentTrackName = mTrackCursor.getString(mTrackCursor.getColumnIndexOrThrow(
                MediaStore.Audio.Media.TITLE));
        menu.setHeaderTitle(mCurrentTrackName);
        /// M: if the song is protect by drm, need add menu to display drm
        // information. @{
        if (OmaDrmUtils.isOmaDrmEnabled() && isDrm == 1) {
            menu.add(0, DRM_INFO, 0, com.mediatek.internal.R.string.drm_protectioninfo_title);
        }
        /// @}
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case PLAY_SELECTION: {
                // play the track
                int position = mSelectedPosition;
                /// M: check drm rights firstly when the song to be played.
                if (checkDrmRightsForPlay(mTrackCursor, position, false, true)) {
                    MusicUtils.playAll(this, mTrackCursor, position);
                }
                return true;
            }

            case QUEUE: {
                long [] list = new long[] { mSelectedId };
                MusicUtils.addToCurrentPlaylist(this, list);
                return true;
            }

            case NEW_PLAYLIST: {
                /// M: if this acitivity is started by LocalActivityManager, it wants
                // to start activity and get result, it must use parent to start activty
                // which is the limitation of  LocalActivityManager. @{
                Activity parent = getParent();
               /* Intent intent = new Intent();
                intent.setClass(this, CreatePlaylist.class);
                /// M: Restore mSelectedId as a string in CreatePlaylist,
                /// so that when onActivityResult called, it will
                /// return the restored value to avoid mCurrentAlbumId
                /// become null when activity has been restarted, such as
                /// switch a language.
                intent.putExtra(MusicUtils.ADD_TO_PLAYLIST_ITEM_ID, String.valueOf(mSelectedId));
                /// M: Add to indicate the save_as_playlist and new_playlist
                intent.putExtra(MusicUtils.SAVE_PLAYLIST_FLAG, MusicUtils.NEW_PLAYLIST);
                if (parent == null) {
                    startActivityForResult(intent, NEW_PLAYLIST);
                } else {
                    /// M: Restore start activity tab
                    ///so that parent MusicBrowserActivity can return the activity result
                    /// to this start activity.
                    intent.putExtra(MusicUtils.START_ACTIVITY_TAB_ID,
                                    MusicBrowserActivity.SONG_INDEX);
                    parent.startActivityForResult(intent, NEW_PLAYLIST);
                }
                return true;
                */

                showCreateDialog(String.valueOf(mSelectedId),
                                 MusicBrowserActivity.SONG_INDEX,
                                 MusicUtils.NEW_PLAYLIST,
                                 null);

                return true;
                /// @}
            }

            case PLAYLIST_SELECTED: {
                long [] list = new long[] { mSelectedId };
                long playlist = item.getIntent().getLongExtra("playlist", 0);
                MusicUtils.addToPlaylist(this, list, playlist);
                return true;
            }

            case USE_AS_RINGTONE:
                // Set the system setting to make this the current ringtone
                MusicUtils.setRingtone(this, mSelectedId);
                return true;

            case DELETE_ITEM: {
                //long [] list = new long[1];
                //list[0] = (int) mSelectedId;
                //Bundle b = new Bundle();
                /// M: Get string in DeleteItems Activity to get current language string. @{
                //String f;
                //if (android.os.Environment.isExternalStorageRemovable()) {
                //f = getString(R.string.delete_song_desc);
                //} else {
                //    f = getString(R.string.delete_song_desc_nosdcard);
                //}
                //String desc = String.format(f, mCurrentTrackName);
                //b.putString("description", desc);
               // b.putInt(MusicUtils.DELETE_DESC_STRING_ID, R.string.delete_song_desc);
                //b.putString(MusicUtils.DELETE_DESC_TRACK_INFO, mCurrentTrackName);
                /// @}
                //b.putLongArray("items", list);
                //Intent intent = new Intent();
               // intent.setClass(this, DeleteItems.class);
               // intent.putExtras(b);
                showDeleteDialog(mSelectedId,
                                 R.string.delete_song_desc,
                                 mCurrentTrackName);
              //  startActivityForResult(intent, -1);
                return true;
            }

            case REMOVE:
                removePlaylistItem(mSelectedPosition);
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
                return true;

            case SEARCH:
                doSearch();
                return true;

            /// M: if the song is protect by drm, display drm information. @{
            case DRM_INFO:
                if (OmaDrmUtils.isOmaDrmEnabled()) {
                    OmaDrmUtils.showProtectionInfoDialog(this, mDrmClient,
                           ContentUris.withAppendedId(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                           mSelectedId));
                }
                return true;

            default:
                return super.onContextItemSelected(item);
            /// @}
        }
        //Not required
        //return super.onContextItemSelected(item);
    }

     private void showCreateDialog(String plst_item_id,
                                   int tab_id,
                                   String plst_flag,
                                   long [] list1) {
        MusicLogUtils.v(TAG, "showCreateDialog>>");
        removeOldFragmentByTag(DIALOG_TAG_CREATE);
        FragmentManager fragmentManager = getFragmentManager();
        MusicLogUtils.v(TAG, "<showCreateDialog> fragmentManager = " + fragmentManager);
        CreatePlaylist newFragment = CreatePlaylist.newInstance(plst_item_id,
                                                                tab_id,
                                                                plst_flag,
                                                                null);
        newFragment.setCursor(list1);
        //((WeekSelector) newFragment).setOnClickListener(mDeleteDialogListener);
        newFragment.show(fragmentManager, DIALOG_TAG_CREATE);
        fragmentManager.executePendingTransactions();
    }
    private void showDeleteDialog(Long track_id, int str_id, String track_name) {
        MusicLogUtils.v(TAG, "showDeleteDialog>>");
        removeOldFragmentByTag(DIALOG_TAG_DELETE);
        FragmentManager fragmentManager = getFragmentManager();
        MusicLogUtils.v(TAG, "<showDeleteDialog> fragmentManager = " + fragmentManager);
        DeleteDialogFragment newFragment =
                          DeleteDialogFragment.newInstance(true, track_id, str_id, track_name);
        newFragment.show(fragmentManager, DIALOG_TAG_DELETE);
        fragmentManager.executePendingTransactions();
        SelectedDelId = track_id;
    }

   private void removeOldFragmentByTag(String tag) {
        MusicLogUtils.v(TAG, "<removeOldFragmentByTag> tag = " + tag);
        FragmentManager fragmentManager = getFragmentManager();
        MusicLogUtils.v(TAG, "<removeOldFragmentByTag> fragmentManager = " + fragmentManager);
        DialogFragment oldFragment = (DialogFragment) fragmentManager.findFragmentByTag(tag);
        MusicLogUtils.v(TAG, "<removeOldFragmentByTag> oldFragment = " + oldFragment);
        if (null != oldFragment) {
            oldFragment.dismissAllowingStateLoss();
        }
        SelectedDelId = -1L;
    }


    void doSearch() {
        CharSequence title = null;
        String query = null;

        Intent i = new Intent();
        i.setAction(MediaStore.INTENT_ACTION_MEDIA_SEARCH);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        title = mCurrentTrackName;
        if (MediaStore.UNKNOWN_STRING.equals(mCurrentArtistNameForAlbum)) {
            query = mCurrentTrackName;
        } else {
            query = mCurrentArtistNameForAlbum + " " + mCurrentTrackName;
            i.putExtra(MediaStore.EXTRA_MEDIA_ARTIST, mCurrentArtistNameForAlbum);
        }
        if (MediaStore.UNKNOWN_STRING.equals(mCurrentAlbumName)) {
            i.putExtra(MediaStore.EXTRA_MEDIA_ALBUM, mCurrentAlbumName);
        }
        i.putExtra(MediaStore.EXTRA_MEDIA_FOCUS, "audio/*");
        title = getString(R.string.mediasearch, title);
        i.putExtra(SearchManager.QUERY, query);

        startActivity(Intent.createChooser(i, title));
    }

    // In order to use alt-up/down as a shortcut for moving the selected item
    // in the list, we need to override dispatchKeyEvent, not onKeyDown.
    // (onKeyDown never sees these events, since they are handled by the list)
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        int curpos = mTrackList.getSelectedItemPosition();
        if (mPlaylist != null && !mPlaylist.equals("recentlyadded") && curpos >= 0 &&
                event.getMetaState() != 0 && event.getAction() == KeyEvent.ACTION_DOWN) {
            switch (event.getKeyCode()) {
                case KeyEvent.KEYCODE_DPAD_UP:
                    moveItem(true);
                    return true;
                case KeyEvent.KEYCODE_DPAD_DOWN:
                    moveItem(false);
                    return true;
                case KeyEvent.KEYCODE_DEL:
                    removeItem();
                    return true;
            }
        }
        if (mSearchItem != null && event.getKeyCode() == KeyEvent.KEYCODE_MENU) {
            if (mSearchItem.isActionViewExpanded()) {
                return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }

    private void removeItem() {
        int curcount = mTrackCursor.getCount();
        int curpos = mTrackList.getSelectedItemPosition();
        if (curcount == 0 || curpos < 0) {
            return;
        }

        if ("nowplaying".equals(mPlaylist)) {
            // remove track from queue

            // Work around bug 902971. To get quick visual feedback
            // of the deletion of the item, hide the selected view.
            try {
                if (curpos != MusicUtils.sService.getQueuePosition()) {
                    mDeletedOneRow = true;
                }
            } catch (RemoteException ex) {
            }
            View v = mTrackList.getSelectedView();
            v.setVisibility(View.GONE);
            mTrackList.invalidateViews();
            ((NowPlayingCursor) mTrackCursor).removeItem(curpos);
            ((TrackListAdapter) getListAdapter()).notifyDataSetChanged();
            v.setVisibility(View.VISIBLE);
            mTrackList.invalidateViews();
        } else {
            // remove track from playlist
            int colidx = mTrackCursor.getColumnIndexOrThrow(
                    MediaStore.Audio.Playlists.Members._ID);
            mTrackCursor.moveToPosition(curpos);
            long id = mTrackCursor.getLong(colidx);
            Uri uri = MediaStore.Audio.Playlists.Members.getContentUri("external",
                    Long.valueOf(mPlaylist));
            getContentResolver().delete(
                    ContentUris.withAppendedId(uri, id), null, null);
            curcount--;
            if (curcount == 0) {
                finish();
            } else {
                mTrackList.setSelection(curpos < curcount ? curpos : curcount);
            }
        }
    }

    private void moveItem(boolean up) {
        int curcount = mTrackCursor.getCount();
        int curpos = mTrackList.getSelectedItemPosition();
        if ((up && curpos < 1) || (!up  && curpos >= curcount - 1)) {
            return;
        }

        if (mTrackCursor instanceof NowPlayingCursor) {
            NowPlayingCursor c = (NowPlayingCursor) mTrackCursor;
            c.moveItem(curpos, up ? curpos - 1 : curpos + 1);
            ((TrackListAdapter) getListAdapter()).notifyDataSetChanged();
            getListView().invalidateViews();
            mDeletedOneRow = true;
            if (up) {
                mTrackList.setSelection(curpos - 1);
            } else {
                mTrackList.setSelection(curpos + 1);
            }
        } else {
            int colidx = mTrackCursor.getColumnIndexOrThrow(
                    MediaStore.Audio.Playlists.Members.PLAY_ORDER);
            mTrackCursor.moveToPosition(curpos);
            int currentplayidx = mTrackCursor.getInt(colidx);
            Uri baseUri = MediaStore.Audio.Playlists.Members.getContentUri("external",
                    Long.valueOf(mPlaylist));
            ContentValues values = new ContentValues();
            String where = MediaStore.Audio.Playlists.Members._ID + "=?";
            String [] wherearg = new String[1];
            ContentResolver res = getContentResolver();
            if (up) {
                values.put(MediaStore.Audio.Playlists.Members.PLAY_ORDER, currentplayidx - 1);
                wherearg[0] = mTrackCursor.getString(0);
                res.update(baseUri, values, where, wherearg);
                mTrackCursor.moveToPrevious();
            } else {
                values.put(MediaStore.Audio.Playlists.Members.PLAY_ORDER, currentplayidx + 1);
                wherearg[0] = mTrackCursor.getString(0);
                res.update(baseUri, values, where, wherearg);
                mTrackCursor.moveToNext();
            }
            values.put(MediaStore.Audio.Playlists.Members.PLAY_ORDER, currentplayidx);
            wherearg[0] = mTrackCursor.getString(0);
            res.update(baseUri, values, where, wherearg);
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id)
    {
        MusicLogUtils.v("MusicPerformanceTest", "[Performance test][Music] play song start ["
            + System.currentTimeMillis() + "]");
        MusicLogUtils.v("MusicPerformanceTest", "[CMCC Performance test][Music] play song start ["
            + System.currentTimeMillis() + "]");
        if (mTrackCursor.getCount() == 0) {
        	MusicLogUtils.v(TAG,"return count 0");
            return;
        }
        // When selecting a track from the queue, just jump there instead of
        // reloading the queue. This is both faster, and prevents accidentally
        // dropping out of party shuffle.
        if (mTrackCursor instanceof NowPlayingCursor) {
            if (MusicUtils.sService != null) {
                try {
                    /// M: check drm rights firstly when the song to be played.
                    if (checkDrmRightsForPlay(mTrackCursor, position, true, true)) {
                        MusicUtils.sService.setQueuePosition(position);
                    }
                    return;
                } catch (RemoteException ex) {
                }
            }
        }
        /// M: check drm rights firstly when the song to be played.
        if (checkDrmRightsForPlay(mTrackCursor, position, false, true)) {
            MusicUtils.playAll(this, mTrackCursor, position);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        /* This activity is used for a number of different browsing modes, and the menu can
         * be different for each of them:
         * - all tracks, optionally restricted to an album, artist or playlist
         * - the list of currently playing songs
         */
        /// M: if this acitivity is in tab mode, use parent's option menu. @{
        boolean ret = super.onCreateOptionsMenu(menu);
        if (mWithtabs) {
            return ret;
        }
        /// @}
        if (mPlaylist == null) {
            menu.add(0, PLAY_ALL, 0, R.string.play_all).setIcon(R.drawable.ic_menu_play_clip);
        }
        menu.add(0, PARTY_SHUFFLE, 0, R.string.party_shuffle);
        // icon will be set in onPrepareOptionsMenu()
        menu.add(0, SHUFFLE_ALL, 0, R.string.shuffle_all).setIcon(R.drawable.ic_menu_shuffle);
        if (mPlaylist != null) {
            menu.add(0, SAVE_AS_PLAYLIST, 0, R.string.save_as_playlist)
            .setIcon(R.drawable.ic_menu_save);

            /// M: Add clear playlist and add file to option menu to
            ///playlist except "Recently added" and "Podcasts".
            /// M: Use plugin to enable need feature. {@
            if (mPlaylist.equals("nowplaying")) {
                menu.add(0, CLEAR_PLAYLIST, 0, R.string.clear_playlist)
                    .setIcon(R.drawable.ic_menu_clear_playlist);
            }
        }
       // Bundle options = new Bundle();
       //options.putString(PluginUtils.PLAYLIST_NAME, mPlaylist);
       // options.putInt(PluginUtils.PLAYLIST_LEN,
       //(mTrackList == null) ? 0 : mTrackList.getCount());

       // mMusicPlugin.onCreateOptionsMenu(menu, ACTIVITY_NAME, options);
        /// M: Add effect panel
        menu.add(0, EFFECTS_PANEL, 0, R.string.effects_list_title).setIcon(R.drawable.ic_menu_eq);
        /// M: Add search view
        mSearchItem = MusicUtils.addSearchView(this, menu, mQueryTextListener, null);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        /// M: if this acitivity is in tab mode, use parent's option menu. @{
        boolean ret = super.onPrepareOptionsMenu(menu);
        if (mWithtabs) {
            return ret;
        }
        /// @}
        MusicUtils.setPartyShuffleMenuIcon(menu);

        if (mPlaylist != null && menu.findItem(CLEAR_PLAYLIST) != null) {
            if (mTrackList != null && mTrackList.getCount() <= 0) {
                /// M: Disable clear playlist option menu when playlist is empty.
                menu.findItem(CLEAR_PLAYLIST).setVisible(false);
            }
        }

        Bundle options = new Bundle();
        options.putString(PLAYLIST_NAME, mPlaylist);
        options.putInt(PLAYLIST_LEN, (mTrackList == null) ? 0 : mTrackList.getCount());


        /// M: the clear playlist is dependen on plugin app @{
     //   mMusicPlugin.onPrepareOptionsMenu(menu, ACTIVITY_NAME, options);
        /// @}
        /// M: set party shufll,shuffle all,save as playlist
        /// invisble at SoundRecord,FM,Phone Recording playlist @{
        if (mPlayListName != null
                && !mPlayListName.equals("")
                && (mPlayListName.equals(getText(R.string.soundrecord_playlist_name))
                        || mPlayListName.equals(getText(R.string.fmrecord_playlist_name))
                        || mPlayListName
                            .equals(getText(R.string.phonerecord_playlist_name)))) {
            menu.findItem(PARTY_SHUFFLE).setVisible(false);
            menu.findItem(SHUFFLE_ALL).setVisible(false);
            menu.findItem(SAVE_AS_PLAYLIST).setVisible(false);
        }
        //@}

        /// M: get effect panel status to set the menu item. @{
        boolean isEffectMenuVisible = true;
        Intent i = new Intent(AudioEffect.ACTION_DISPLAY_AUDIO_EFFECT_CONTROL_PANEL);
        if (getPackageManager().resolveActivity(i, 0) == null) {
            isEffectMenuVisible = false;
        }
        menu.findItem(EFFECTS_PANEL).setVisible(isEffectMenuVisible);
        /// @}
        mOptionMenu = menu;
        /// M: resore the input of SearchView when config change @{
        if (mQueryText != null && !mQueryText.toString().equals("")) {
            MusicLogUtils.v(TAG, "setQueryText:" + mQueryText);
            SearchView searchView = (SearchView) mSearchItem.getActionView();
            searchView.setQuery(mQueryText, false);
            mQueryText = null;
        }
        /// @}
        /// M: if there is no song, the option menu is disable. @{
        if ((findViewById(R.id.sd_icon).getVisibility()) == View.VISIBLE) {
            MusicLogUtils.v(TAG, "SDcard not ready, disable option menu!");
            return false;
        }
        /// @}
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        /// M: if this acitivity is in tab mode, use parent's option menu. @{
        boolean ret = super.onOptionsItemSelected(item);
        if (mWithtabs) {
            return ret;
        }
        Intent intent;
        Cursor cursor;
        switch (item.getItemId()) {
            case PLAY_ALL: {
                MusicUtils.playAll(this, mTrackCursor);
                return true;
            }

            case PARTY_SHUFFLE:
                MusicUtils.togglePartyShuffle();
                setTitle();
                break;

            case SHUFFLE_ALL:
                // Should 'shuffle all' shuffle ALL, or only the tracks shown?
                cursor = MusicUtils.query(this, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                        new String [] { MediaStore.Audio.Media._ID},
                        MediaStore.Audio.Media.IS_MUSIC + "=1", null,
                        /// M: add for chinese sorting
                        null);
                if (cursor != null) {
                    MusicUtils.shuffleAll(this, cursor);
                    cursor.close();
                }
                return true;

            case SAVE_AS_PLAYLIST:
                //intent = new Intent();
                //intent.setClass(this, CreatePlaylist.class);
                //intent.putExtra(MusicUtils.SAVE_PLAYLIST_FLAG, MusicUtils.SAVE_AS_PLAYLIST);
                //intent.putExtra(MusicUtils.PLAYLIST_NAME, mPlayListName);
                //startActivityForResult(intent, SAVE_AS_PLAYLIST);
                long [] list1 = MusicUtils.getSongListForCursor(mTrackCursor);
               // MusicLogUtils.v(TAG,
               // "SAVE_AS_PLAYLIST list" + Arrays.toString(list1[0]));
                showCreateDialog(null,
                                 MusicBrowserActivity.SONG_INDEX,
                                 MusicUtils.SAVE_AS_PLAYLIST,
                                 list1);
                MusicLogUtils.v(TAG,
                "SAVE_AS_PLAYLIST list" + Arrays.toString(list1));
                return true;

            case CLEAR_PLAYLIST:
                /// M: clear the current playlist, or the specify playlist and
                /// finish current acitivity. Add song to playlist @{
                if (mPlaylist.equals("nowplaying")) {
                    MusicUtils.clearQueue();
                }
                return true;
                /// @}

                /// M: handle effect, back press, local search and defalt case. @{
            case EFFECTS_PANEL:
                return MusicUtils.startEffectPanel(this);

            case android.R.id.home:
                /// M: Navigation button press back
                if (!mIsInBackgroud) {
                    onBackPressed();
                }
                return true;

            case R.id.search:
                onSearchRequested();
                return true;

            default:
                Bundle options = new Bundle();
                options.putString(PLAYLIST_NAME, mPlaylist);
                options.putInt(PLAYLIST_LEN, (mTrackList == null) ?
                 0 : mTrackList.getCount());
               /* if (mMusicPlugin.onOptionsItemSelected(getApplicationContext(),
                                                       item,
                                                       ACTIVITY_NAME,
                                                       this,
                                                       mMusicListenerForPlugin,
                                                       options)) {
                    return true;
                }*/
                return false;
        }
        return ret;
        /// @}
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent intent) {
        switch (requestCode) {
            case SCAN_DONE:
                if (resultCode == RESULT_CANCELED) {
                    finish();
                } else {
                    getTrackCursor(mAdapter.getQueryHandler(), null, true);
                }
                break;

            case NEW_PLAYLIST:
                /*if (resultCode == RESULT_OK) {
                    Uri uri = intent.getData();
                    /// M: Get selected album id need add to new playlist from
                    /// intent to avoid mCurrentAlbumId to be null
                    /// when activity restart causing by configaration change. @{
                    String selectAudioId =
                     intent.getStringExtra(MusicUtils.ADD_TO_PLAYLIST_ITEM_ID);
                    MusicLogUtils.v(TAG, "onActivityResult: selectAudioId = " + selectAudioId);
                    if (uri != null && selectAudioId != null) {
                        long [] list = new long[] { Long.parseLong(selectAudioId) };
                        MusicUtils.addToPlaylist(this, list, Integer.valueOf
                        (uri.getLastPathSegment()));
                    }
                    /// @}
                }

               */

                break;
            case SAVE_AS_PLAYLIST:
                if (resultCode == RESULT_OK) {
                    Uri uri = intent.getData();
                    /// M: Return SAVE_PLAYLIST_FLAG only
                    ///when overwrite playlist by save as playlist
                    String flag = intent.getStringExtra(MusicUtils.SAVE_PLAYLIST_FLAG);
                    if (flag == null) {
                        flag = "";
                    }
                    long [] list = MusicUtils.getSongListForCursor(mTrackCursor);
                    int listLength = list.length;
                    if (uri != null) {
                        //Not Required
                        //long [] list = MusicUtils.getSongListForCursor(mTrackCursor);
                        int plid = Integer.parseInt(uri.getLastPathSegment());
                        MusicUtils.addToPlaylist(this, list, plid);
                    } else if (flag.equals(MusicUtils.SAVE_AS_PLAYLIST)) {
                        String message = getApplicationContext().getResources().getQuantityString(
                                R.plurals.NNNtrackstoplaylist, listLength, listLength);
                        Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
                    }
                }
                break;

            default:
                Bundle options = new Bundle();
                options.putString(PLAYLIST_NAME, mPlaylist);
                options.putInt(PLAYLIST_LEN, (mTrackList == null) ?
                 0 : mTrackList.getCount());

                /// M: handling the result from filemanager when plugin need.{@
              /*  mMusicPlugin.onActivityResult(requestCode, resultCode, intent,
                getApplicationContext(), mMusicListenerForPlugin, ACTIVITY_NAME, this, options);*/
                /// @}
                break;
        }
    }

    private Cursor getTrackCursor(TrackListAdapter.TrackQueryHandler queryhandler, String filter,
            boolean async) {

        if (queryhandler == null) {
            throw new IllegalArgumentException();
        }

        Cursor ret = null;
        /// M: add for chinese sorting
        mSortOrder = null;
        StringBuilder where = new StringBuilder();
        where.append(MediaStore.Audio.Media.TITLE + " != ''");

        if (mGenre != null) {
            Uri uri = MediaStore.Audio.Genres.Members.getContentUri("external",
                    Integer.valueOf(mGenre));
            if (!TextUtils.isEmpty(filter)) {
                uri = uri.buildUpon().appendQueryParameter("filter", Uri.encode(filter)).build();
            }
            mSortOrder = MediaStore.Audio.Genres.Members.DEFAULT_SORT_ORDER;
            ret = queryhandler.doQuery(uri,
                    mCursorCols, where.toString(), null, null, async);
        } else if (mPlaylist != null) {
            if (mPlaylist.equals("nowplaying")) {
                if (MusicUtils.sService != null) {
                    ret = new NowPlayingCursor(MusicUtils.sService, mCursorCols);
                    if (ret.getCount() == 0) {
                        finish();
                    }
                } else {
                    // Nothing is playing.
                }
            } else if (mPlaylist.equals("podcasts")) {
                where.append(" AND " + MediaStore.Audio.Media.IS_PODCAST + "=1");
                Uri uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                if (!TextUtils.isEmpty(filter)) {
                    uri = uri.buildUpon().appendQueryParameter("filter",
                        Uri.encode(filter)).build();
                }
                ret = queryhandler.doQuery(uri,
                        mCursorCols, where.toString(), null,
                        /// M: add for chinese sorting
                        null, async);
            } else if (mPlaylist.equals("recentlyadded")) {
                // do a query for all songs added in the last X weeks
                Uri uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                if (!TextUtils.isEmpty(filter)) {
                    uri = uri.buildUpon().appendQueryParameter("filter",
                        Uri.encode(filter)).build();
                }
                int X = MusicUtils.getIntPref(this, "numweeks", 2) * (3600 * 24 * 7);
                where.append(" AND " + MediaStore.MediaColumns.DATE_ADDED + ">");
                where.append(System.currentTimeMillis() / 1000 - X);
                ret = queryhandler.doQuery(uri,
                        mCursorCols, where.toString(), null,
                        /// M: add for chinese sorting
                        null, async);
            } else {
                Uri uri = MediaStore.Audio.Playlists.Members.getContentUri("external",
                        Long.valueOf(mPlaylist));
                if (!TextUtils.isEmpty(filter)) {
                    uri = uri.buildUpon().appendQueryParameter("filter",
                        Uri.encode(filter)).build();
                }
                mSortOrder = MediaStore.Audio.Playlists.Members.DEFAULT_SORT_ORDER;
                ret = queryhandler.doQuery(uri, mPlaylistMemberCols,
                        where.toString(), null, mSortOrder, async);
            }
        } else {
            if (mAlbumId != null) {
                where.append(" AND " + MediaStore.Audio.Media.ALBUM_ID + "=" + mAlbumId);
                mSortOrder = MediaStore.Audio.Media.TRACK + ", " + mSortOrder;
            }
            if (mArtistId != null) {
                where.append(" AND " + MediaStore.Audio.Media.ARTIST_ID + "=" + mArtistId);
            }
            where.append(" AND " + MediaStore.Audio.Media.IS_MUSIC + "=1");
            Uri uri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
            if (!TextUtils.isEmpty(filter)) {
                uri = uri.buildUpon().appendQueryParameter("filter", Uri.encode(filter)).build();
            }
            ret = queryhandler.doQuery(uri,
                    mCursorCols, where.toString() , null, mSortOrder, async);
        }

        // This special case is for the "nowplaying" cursor, which cannot be handled
        // asynchronously using AsyncQueryHandler, so we do some extra initialization here.
        if (ret != null && async) {
            init(ret, false);
            setTitle();
        }
        return ret;
    }

    private class NowPlayingCursor extends AbstractCursor
    {
        public NowPlayingCursor(IMediaPlaybackService service, String [] cols)
        {
            mCols = cols;
            mService  = service;
            makeNowPlayingCursor();
        }
        private void makeNowPlayingCursor() {
            /// M: if CurrentPlaylistCursor is invalid ,do nothing ,just close cursor @{
            if (mCurrentPlaylistCursor != null) {
                mCurrentPlaylistCursor.close();
            }
            /// @}
            mCurrentPlaylistCursor = null;
            try {
                mNowPlaying = mService.getQueue();
            } catch (RemoteException ex) {
                mNowPlaying = new long[0];
            }
            mSize = mNowPlaying.length;
            if (mSize == 0) {
                return;
            }

            StringBuilder where = new StringBuilder();
            where.append(MediaStore.Audio.Media._ID + " IN (");
            for (int i = 0; i < mSize; i++) {
                where.append(mNowPlaying[i]);
                if (i < mSize - 1) {
                    where.append(",");
                }
            }
            where.append(")");

            mCurrentPlaylistCursor = MusicUtils.query(TrackBrowserActivity.this,
                    MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                    mCols, where.toString(), null, MediaStore.Audio.Media._ID);

            if (mCurrentPlaylistCursor == null) {
                mSize = 0;
                return;
            }

            int size = mCurrentPlaylistCursor.getCount();
            mCursorIdxs = new long[size];
            mCurrentPlaylistCursor.moveToFirst();
            int colidx = mCurrentPlaylistCursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID);
            for (int i = 0; i < size; i++) {
                mCursorIdxs[i] = mCurrentPlaylistCursor.getLong(colidx);
                mCurrentPlaylistCursor.moveToNext();
            }
            mCurrentPlaylistCursor.moveToFirst();
            mCurPos = -1;

            // At this point we can verify the 'now playing' list we got
            // earlier to make sure that all the items in there still exist
            // in the database, and remove those that aren't. This way we
            // don't get any blank items in the list.
            try {
                int removed = 0;
                for (int i = mNowPlaying.length - 1; i >= 0; i--) {
                    long trackid = mNowPlaying[i];
                    int crsridx = Arrays.binarySearch(mCursorIdxs, trackid);
                    if (crsridx < 0) {
                        //Log.i("@@@@@", "item no longer exists in db: " + trackid);
                        removed += mService.removeTrack(trackid);
                    }
                }
                if (removed > 0) {
                    mNowPlaying = mService.getQueue();
                    mSize = mNowPlaying.length;
                    if (mSize == 0) {
                        mCursorIdxs = null;
                        return;
                    }
                }
            } catch (RemoteException ex) {
                mNowPlaying = new long[0];
            }
        }

        @Override
        public int getCount()
        {
            return mSize;
        }

        @Override
        public boolean onMove(int oldPosition, int newPosition)
        {
            if (oldPosition == newPosition)
                return true;

            if (mNowPlaying == null || mCursorIdxs == null || newPosition >= mNowPlaying.length) {
                return false;
            }

            // The cursor doesn't have any duplicates in it, and is not ordered
            // in queue-order, so we need to figure out where in the cursor we
            // should be.

            long newid = mNowPlaying[newPosition];
            int crsridx = Arrays.binarySearch(mCursorIdxs, newid);
            mCurrentPlaylistCursor.moveToPosition(crsridx);
            mCurPos = newPosition;

            return true;
        }

        public boolean removeItem(int which)
        {
            try {
                if (mService.removeTracks(which, which) == 0) {
                    return false; // delete failed
                }
                int i = (int) which;
                mSize--;
                while (i < mSize) {
                    mNowPlaying[i] = mNowPlaying[i + 1];
                    i++;
                }
                onMove(-1, (int) mCurPos);
            } catch (RemoteException ex) {
            }
            return true;
        }

        public void moveItem(int from, int to) {
            try {
                long[] serviceQueue = mService.getQueue();
                if (serviceQueue.length == mNowPlaying.length) {
                    mService.moveQueueItem(from, to);
                    mNowPlaying = mService.getQueue();
                    onMove(-1, mCurPos); // update the underlying cursor
                } else {
                    if (from < 0 ||
                        to < 0 ||
                        from >= mNowPlaying.length ||
                        to >= mNowPlaying.length ||
                        from == to) {
                        return;
                    }
                    long tmp = mNowPlaying[from];
                    if (from < to) {
                        for (int i = from; i < to; i++) {
                            mNowPlaying[i] = mNowPlaying[i + 1];
                        }
                    } else if (to < from) {
                         for (int i = from; i > to; i--) {
                            mNowPlaying[i] = mNowPlaying[i - 1];
                        }
                    }
                    mNowPlaying[to] = tmp;
                    onMove(-1, mCurPos);
                }
            } catch (RemoteException ex) {
            }
        }

        private void dump() {
            String where = "(";
            for (int i = 0; i < mSize; i++) {
                where += mNowPlaying[i];
                if (i < mSize - 1) {
                    where += ",";
                }
            }
            where += ")";
            MusicLogUtils.v(TAG, where);
        }

        @Override
        public String getString(int column)
        {
            try {
                return mCurrentPlaylistCursor.getString(column);
            } catch (Exception ex) {
                onChange(true);
                return "";
            }
        }

        @Override
        public short getShort(int column)
        {
            return mCurrentPlaylistCursor.getShort(column);
        }

        @Override
        public int getInt(int column)
        {
            try {
                return mCurrentPlaylistCursor.getInt(column);
            } catch (Exception ex) {
                onChange(true);
                return 0;
            }
        }

        @Override
        public long getLong(int column)
        {
            try {
                return mCurrentPlaylistCursor.getLong(column);
            } catch (Exception ex) {
                onChange(true);
                return 0;
            }
        }

        @Override
        public float getFloat(int column)
        {
            return mCurrentPlaylistCursor.getFloat(column);
        }

        @Override
        public double getDouble(int column)
        {
            return mCurrentPlaylistCursor.getDouble(column);
        }

        @Override
        public int getType(int column) {
            return mCurrentPlaylistCursor.getType(column);
        }

        @Override
        public boolean isNull(int column)
        {
            return mCurrentPlaylistCursor.isNull(column);
        }

        @Override
        public String[] getColumnNames()
        {
            return mCols;
        }

        @Override
        public void deactivate()
        {
            if (mCurrentPlaylistCursor != null)
                mCurrentPlaylistCursor.deactivate();
        }

        @Override
        public boolean requery()
        {
            makeNowPlayingCursor();
            return true;
        }

        /*
         * M: close CurrentPlaylistCursor if needed
         */
        @Override
        public void close() {
            super.close();
            if (mCurrentPlaylistCursor != null) {
                mCurrentPlaylistCursor.close();
                mCurrentPlaylistCursor = null;
            }
        }

        private String [] mCols;
        private Cursor mCurrentPlaylistCursor;     // updated in onMove
        private int mSize;          // size of the queue
        private long[] mNowPlaying;
        private long[] mCursorIdxs;
        private int mCurPos;
        private IMediaPlaybackService mService;
    }

    static class TrackListAdapter extends SimpleCursorAdapter implements SectionIndexer {
        boolean mIsNowPlaying;
        boolean mDisableNowPlayingIndicator;

        int mTitleIdx;
        int mArtistIdx;
        int mDurationIdx;
        int mAudioIdIdx;

        private final StringBuilder mBuilder = new StringBuilder();
        /// M: remove final for Locale change, the value will chane at
        // reloadStringOnLocaleChanges().
        private String mUnknownArtist;
        private String mUnknownAlbum;

        private AlphabetIndexer mIndexer;

        private TrackBrowserActivity mActivity = null;
        private TrackQueryHandler mQueryHandler;
        private String mConstraint = null;
        private boolean mConstraintIsValid = false;

        /// M: DRM cursor index.
        int mIsDrmIdx = -1;
        int mDrmMethodIdx = -1;
        /// M: add for chinese sorting, title pinyin index.
        int mTitlePinyinIdx;

        static class ViewHolder {
            TextView line1;
            TextView line2;
            TextView duration;
            ImageView play_indicator;
            CharArrayBuffer buffer1;
            char [] buffer2;
            /// M: add the ViewHolder members, DRM icon and Edit icon @{
            ImageView drmLock;
            ImageView editIcon;
            /// @}
        }

        class TrackQueryHandler extends AsyncQueryHandler {

            class QueryArgs {
                public Uri uri;
                public String [] projection;
                public String selection;
                public String [] selectionArgs;
                public String orderBy;
            }

            TrackQueryHandler(ContentResolver res) {
                super(res);
            }

            public Cursor doQuery(Uri uri, String[] projection,
                    String selection, String[] selectionArgs,
                    String orderBy, boolean async) {
                if (async) {
                    // Get 100 results first, which is enough to allow the user to start scrolling,
                    // while still being very fast.
                    Uri limituri = uri.buildUpon().appendQueryParameter("limit", "100").build();
                    QueryArgs args = new QueryArgs();
                    args.uri = uri;
                    args.projection = projection;
                    args.selection = selection;
                    args.selectionArgs = selectionArgs;
                    args.orderBy = orderBy;

                    startQuery(0, args, limituri, projection, selection, selectionArgs, orderBy);
                    return null;
                }
                return MusicUtils.query(mActivity,
                        uri, projection, selection, selectionArgs, orderBy);
            }

            @Override
            protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
                MusicLogUtils.v(TAG, "query complete: count is "
                 + (cursor == null ? null : cursor.getCount())
                        + "   " + mActivity);
                mActivity.init(cursor, cookie != null);
                /// M: Cursor may be close when activity has been finishing, so we need check it.
                if (token == 0 && cookie != null && cursor != null
                 && !cursor.isClosed() && cursor.getCount() >= 100) {
                    QueryArgs args = (QueryArgs) cookie;
                    startQuery(1, null, args.uri, args.projection, args.selection,
                            args.selectionArgs, args.orderBy);
                }
            }
        }

        TrackListAdapter(Context context, TrackBrowserActivity currentactivity,
                int layout, Cursor cursor, String[] from, int[] to,
                boolean isnowplaying, boolean disablenowplayingindicator) {
            super(context, layout, cursor, from, to);
            mActivity = currentactivity;
            getColumnIndices(cursor);
            mIsNowPlaying = isnowplaying;
            mDisableNowPlayingIndicator = disablenowplayingindicator;
            mUnknownArtist = context.getString(R.string.unknown_artist_name);
            mUnknownAlbum = context.getString(R.string.unknown_album_name);

            mQueryHandler = new TrackQueryHandler(context.getContentResolver());
        }

        public void setActivity(TrackBrowserActivity newactivity) {
            mActivity = newactivity;
        }

        public TrackQueryHandler getQueryHandler() {
            return mQueryHandler;
        }

        private void getColumnIndices(Cursor cursor) {
            if (cursor != null) {
                mTitleIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.TITLE);
                mArtistIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.ARTIST);
                mDurationIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.DURATION);
                try {
                    mAudioIdIdx = cursor.getColumnIndexOrThrow(
                            MediaStore.Audio.Playlists.Members.AUDIO_ID);
                } catch (IllegalArgumentException ex) {
                    mAudioIdIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID);
                }

                /// M: add for chinese sorting
               
                /// M: if DRM feature is on,get mIsDrmIdx and mDrmMethodIdx from current cursor @{
                if (OmaDrmUtils.isOmaDrmEnabled()) {
                    mIsDrmIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.IS_DRM);
                   // mDrmMethodIdx = cursor.getColumnIndexOrThrow(MtkMediaStore.MediaColumns.DRM_METHOD);
                }
                /// @}
                if (mIndexer != null) {
                    mIndexer.setCursor(cursor);
                } else if (!mActivity.mEditMode && mActivity.mAlbumId == null) {
                    String alpha = mActivity.getString(R.string.fast_scroll_alphabet);
                    /// M: add for chinese sorting
                    mIndexer = new MusicAlphabetIndexer(cursor, mTitleIdx, alpha);
                } else {
                    /// M: disable the FastScroll.
                    mActivity.mTrackList.setFastScrollEnabled(false);
                }
            }
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            View v = super.newView(context, cursor, parent);
            /// M: set the edit icon visibibility at bindview(), don't set here.
            //ImageView iv = (ImageView) v.findViewById(R.id.icon);
            //iv.setVisibility(View.GONE);

            ViewHolder vh = new ViewHolder();
            vh.line1 = (TextView) v.findViewById(R.id.line1);
            vh.line2 = (TextView) v.findViewById(R.id.line2);
            vh.duration = (TextView) v.findViewById(R.id.duration);
            vh.play_indicator = (ImageView) v.findViewById(R.id.play_indicator);
            vh.buffer1 = new CharArrayBuffer(100);
            vh.buffer2 = new char[200];
            /// M: get the ImageView of drm_lcok and play_indicator.
            vh.drmLock = (ImageView) v.findViewById(R.id.drm_lock);
            /// M: when on Edit mode,get the ImagerView of edit icon @{
            if (mActivity.mEditMode) {
                vh.editIcon = (ImageView) v.findViewById(R.id.icon);
            }
            /// @}
            v.setTag(vh);
            return v;
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {

            ViewHolder vh = (ViewHolder) view.getTag();

            cursor.copyStringToBuffer(mTitleIdx, vh.buffer1);
            vh.line1.setText(vh.buffer1.data, 0, vh.buffer1.sizeCopied);

            //MusicLogUtils.v(TAG, "BindView:" + view + "!!" );
            //MusicLogUtils.v(TAG, "BindView:Song Name: " + cursor.getString(mTitleIdx) + "!!" );
            int secs = cursor.getInt(mDurationIdx) / 1000;
            // M: mark for show "0:00" when the duration is 0 @{
            //if (secs == 0) {
            //    vh.duration.setText(" ");
            //} else */{
            vh.duration.setText(MusicUtils.makeTimeString(context, secs));
            //}
            /// @}
            final StringBuilder builder = mBuilder;
            builder.delete(0, builder.length());

            String name = cursor.getString(mArtistIdx);
            if (name == null || name.equals(MediaStore.UNKNOWN_STRING)) {
                builder.append(mUnknownArtist);
            } else {
                builder.append(name);
            }
            //MusicLogUtils.v(TAG, "Artist Name: " + name + "!!" );
            int len = builder.length();
            if (vh.buffer2.length < len) {
                vh.buffer2 = new char[len];
            }
            builder.getChars(0, len, vh.buffer2, 0);
            vh.line2.setText(vh.buffer2, 0, len);
            /// M: when in edit mode,add the Image resource and make it visible @{
            if (mActivity.mEditMode) {
                ImageView ivEdit = vh.editIcon;
                ivEdit.setImageResource(R.drawable.ic_playlist_move);
                ivEdit.setVisibility(View.VISIBLE);
            }
            /// @}

            ImageView iv = vh.play_indicator;
            long id = -1;
            if (MusicUtils.sService != null) {
                // TODO: IPC call on each bind??
                try {
                    if (mIsNowPlaying) {
                        id = MusicUtils.sService.getQueuePosition();
                    } else {
                        id = MusicUtils.sService.getAudioId();
                    }
                } catch (RemoteException ex) {
                }
            }

            // Determining whether and where to show the "now playing indicator
            // is tricky, because we don't actually keep track of where the songs
            // in the current playlist came from after they've started playing.
            //
            // If the "current playlists" is shown, then we can simply match by position,
            // otherwise, we need to match by id. Match-by-id gets a little weird if
            // a song appears in a playlist more than once, and you're in edit-playlist
            // mode. In that case, both items will have the "now playing" indicator.
            // For this reason, we don't show the play indicator at all when in edit
            // playlist mode (except when you're viewing the "current playlist",
            // which is not really a playlist)
            if ((mIsNowPlaying && cursor.getPosition() == id) ||
                 (!mIsNowPlaying && !mDisableNowPlayingIndicator &&
                 cursor.getLong(mAudioIdIdx) == id)) {
                //iv.setImageResource(R.drawable.indicator_ic_mp_playing_list);
                iv.setVisibility(View.VISIBLE);
            } else {
                iv.setVisibility(View.GONE);
            }
            /// M: update DRM lock icon.
            updateDrmLockIcon(vh.drmLock, cursor);
        }

        @Override
        public void changeCursor(Cursor cursor) {
            if (mActivity.isFinishing() && cursor != null) {
                cursor.close();
                cursor = null;
            }
            if (cursor != mActivity.mTrackCursor) {
                mActivity.mTrackCursor = cursor;
                super.changeCursor(cursor);
                getColumnIndices(cursor);
            }
        }

        @Override
        public Cursor runQueryOnBackgroundThread(CharSequence constraint) {
            String s = constraint.toString();
            if (mConstraintIsValid && (
                    (s == null && mConstraint == null) ||
                    (s != null && s.equals(mConstraint)))) {
                return getCursor();
            }
            Cursor c = mActivity.getTrackCursor(mQueryHandler, s, false);
            mConstraint = s;
            mConstraintIsValid = true;
            return c;
        }

        // SectionIndexer methods

        public Object[] getSections() {
            if (mIndexer != null) {
                return mIndexer.getSections();
            } else {
                return new String [] { " " };
            }
        }

        public int getPositionForSection(int section) {
            if (mIndexer != null) {
                return mIndexer.getPositionForSection(section);
            }
            return 0;
        }

        public int getSectionForPosition(int position) {
            /// M: call Indexer's getSectionForPosition() due to ICS
            // FastScroll change. @{
            if (mIndexer != null) {
                return mIndexer.getSectionForPosition(position);
            }
            /// @}
            return 0;
        }

        /**
         * M: update the drm lock icon.
         *
         * @param drmLock the drm lock image view for updating.
         * @param cursor the cursor for getting drm information.
         */
        private void updateDrmLockIcon(ImageView drmLock, Cursor cursor) {
            int drmLockVisible = View.GONE;
            if (OmaDrmUtils.isOmaDrmEnabled()) {
                int isDrm = cursor.getInt(mIsDrmIdx);
               // int drmMethod = cursor.getInt(mDrmMethodIdx);
                if (isDrm == 1) {
                    int rightsStatus = mActivity.getDrmRightsStatus(cursor);
                    if (rightsStatus < 0) {
                        drmLockVisible = View.GONE;
                    } else if (rightsStatus == DrmStore.RightsStatus.RIGHTS_VALID) {
                        drmLock.setImageResource(com.mediatek.internal.R.drawable.drm_green_lock);
                        drmLockVisible = View.VISIBLE;
                    } else {
                        drmLock.setImageResource(com.mediatek.internal.R.drawable.drm_red_lock);
                        drmLockVisible = View.VISIBLE;
                    }
                }
            }
            drmLock.setVisibility(drmLockVisible);
        }

        /*
         * M: when switch language ,the Activity will be killed by the system,then
         * enter the activity,should reload the unknown_artist_name and unknown_album_name.
         */
        public void reloadStringOnLocaleChanges() {
            String sUnknownArtist = mActivity.getString(R.string.unknown_artist_name);
            String sUnknownAlbum = mActivity.getString(R.string.unknown_album_name);
            if (mUnknownArtist != null && !mUnknownArtist.equals(sUnknownArtist)) {
                mUnknownArtist = sUnknownArtist;
            }
            if (mUnknownAlbum != null && !mUnknownAlbum.equals(sUnknownAlbum)) {
                mUnknownAlbum = sUnknownAlbum;
            }
        }
    }

    /*
     * M: initialize adapter for listview, which is moved from
     *  onServiceConnected().
     */
    private void initAdapter() {
        mAdapter = (TrackListAdapter) getLastNonConfigurationInstance();
        if (mAdapter == null) {
            MusicLogUtils.v(TAG, "starting query");
            mAdapter = new TrackListAdapter(
                    getApplication(), // need to use application context to avoid leaks
                    this,
                    mEditMode ? R.layout.edit_track_list_item : R.layout.track_list_item,
                    null, // cursor
                    new String[] {},
                    new int[] {},
                    "nowplaying".equals(mPlaylist),
                    mPlaylist != null &&
                    !(mPlaylist.equals("podcasts") || mPlaylist.equals("recentlyadded")));
            setListAdapter(mAdapter);
            /// M: in tab mode which has not title, so no need to set.
            if (!mWithtabs) {
                setTitle(R.string.working_songs);
            }
            getTrackCursor(mAdapter.getQueryHandler(), null, true);
        } else {
            mAdapter.setActivity(this);
            /// M: when switch language ,reload String.
            mAdapter.reloadStringOnLocaleChanges();
            setListAdapter(mAdapter);
            mTrackCursor = mAdapter.getCursor();
            // If mTrackCursor is null, this can be because it doesn't have
            // a cursor yet (because the initial query that sets its cursor
            // is still in progress), or because the query failed.
            // In order to not flash the error dialog at the user for the
            // first case, simply retry the query when the cursor is null.
            // Worst case, we end up doing the same query twice.
            if (mTrackCursor != null) {
                init(mTrackCursor, false);
            } else {
                /// M: in tab mode which has not title, so no need to set.
                if (!mWithtabs) {
                    setTitle(R.string.working_songs);
                }
                getTrackCursor(mAdapter.getQueryHandler(), null, true);
            }
        }
    }

    /**
     * M: Call when search request and expand search action view.
     */
    @Override
    public boolean onSearchRequested() {
        if (mSearchItem != null) {
            mSearchItem.expandActionView();
        }
        return true;
    }

    /**
     * M: when edit Text ,do query follow the message of the query.
     */
    SearchView.OnQueryTextListener mQueryTextListener = new SearchView.OnQueryTextListener() {
        public boolean onQueryTextSubmit(String query) {
            Intent intent = new Intent();
            intent.setClass(TrackBrowserActivity.this, QueryBrowserActivity.class);
            intent.putExtra(SearchManager.QUERY, query);
            startActivity(intent);
            mSearchItem.collapseActionView();
            return true;
        }

        public boolean onQueryTextChange(String newText) {
            return false;
        }
    };

    /**
     * M: in this Activity ,Configuration changed include (orientation|screenSize).
     *
     * @param newConfig The new device configuration.
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mOrientation = newConfig.orientation;

        /// M: If showing database error UI, need not to refresh UI.
        if ((findViewById(R.id.sd_icon).getVisibility()) == View.VISIBLE) {
            MusicLogUtils.v(TAG, "Configuration Changed at database error, return!");
            return;
        }

        /// M: When turn off screen at landscape,
        /// it will change to portrait when unlock screen, and the
        /// album art background will be abnormal, we need clear it and load new one.
        if (mOrientation == Configuration.ORIENTATION_PORTRAIT && mIsInBackgroud) {
            MusicUtils.setBackground((View) mTrackList.getParent().getParent(), null);
            MusicLogUtils.v(TAG,
             "onConfigurationChanged clear background album art when in background.");
        }

        if (mService != null) {
            MusicUtils.updateNowPlaying(this, mOrientation);
        }
        /// M: Before invalidateOptionsMenu,save the input of SearchView @{
        if (mSearchItem != null) {
            SearchView searchView = (SearchView) mSearchItem.getActionView();
            mQueryText = searchView.getQuery();
            MusicLogUtils.v(TAG, "searchText:" + mQueryText);
        }
        /// @}
        invalidateOptionsMenu();
    }

    /**
     * M: set Album art background bitmap in AsyncTask mode.
     */
    private class AlbumArtFetcher extends AsyncTask<Long, Void, Bitmap> {
        protected Bitmap doInBackground(Long... albumId) {

            if (mAlbumArtBitmap == null) {
                long id = albumId[0].longValue();
                mAlbumArtBitmap = MusicUtils.getArtwork(TrackBrowserActivity.this, -1, id, false);
                MusicLogUtils.v(TAG, "AlbumArtFetcher: getArtwork returns " + mAlbumArtBitmap);

            }
            /// M: wait the new view finish measure and then set the bitmap to be background.
            ((TouchInterceptor) mTrackList).waitMeasureFinished
            ((getResources().getConfiguration().orientation
                    == Configuration.ORIENTATION_LANDSCAPE));
            return mAlbumArtBitmap;
        }

        protected void onPostExecute(Bitmap bm) {
            /// M: Only set background album art when activity is
            // foreground and show it at main layout so that
            /// it will show with listview and nowplaying height,
            // when nowplaying no exsit, the ablum art will
            /// not excute.
            if (bm != null) {
                MusicUtils.setBackground((View) mTrackList.getParent().getParent(), bm);
                mTrackList.setCacheColorHint(0);
                return;
            }
        }
    }

    /**
     * M: Upgrade album art
     */
    private TouchInterceptor.UpgradeAlbumArtListener mUpgradeAlbumArtListener =
        new TouchInterceptor.UpgradeAlbumArtListener() {
        @Override
        public void UpgradeAlbumArt() {
            setAlbumArtBackground();
        }
    };

    /**
     * M: when click showed Dialog Button,call This method will be invoked.
     *
     * @param dialog The dialog that received the click.
     * @param which The button that was clicked or the position
     *            of the item clicked.
     */
    public void onClick(DialogInterface dialog, int which) {
        if (which == DialogInterface.BUTTON_POSITIVE) {
            if (mTrackCursor instanceof NowPlayingCursor) {
                if (MusicUtils.sService != null) {
                    try {
                        MusicUtils.sService.setQueuePosition(mCurTrackPos);
                        return;
                    } catch (RemoteException ex) {
                        MusicLogUtils.v(TAG, "RemoteException when setQueuePosition: ", ex);
                    }
                }
            } else {
                MusicUtils.playAll(this, mTrackCursor, mCurTrackPos);
            }
        }
    }

    /**
     * M: get the uri of the current cursor position.
     *
     * @param cursor the cursor for getting uri.
     *
     * @return Returns the uri getting from cursor.
     */
    private Uri getUri(Cursor cursor) {
        int colIdx = -1;
        try {
            colIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Playlists.Members.AUDIO_ID);
        } catch (IllegalArgumentException ex) {
            colIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID);
        }
        return ContentUris.withAppendedId(
                MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, cursor.getLong(colIdx));
    }

    /**
     * M: get the DRM rights status.
     *
     * @param cursor the cursor for getting DRM rights status;
     *
     * @return Returns the DRM rights status.
     */
    private int getDrmRightsStatus(Cursor cursor) {
        Uri uri = null;
        int rightsStatus = -1;
        uri = getUri(cursor);
        /// M: We should catch the IllegalArgumentException
        try {
            rightsStatus = mDrmClient.checkRightsStatus(uri, DrmStore.Action.PLAY);
        } catch (IllegalArgumentException e) {
            MusicLogUtils.v(TAG, "getDrmRightsStatus throw IllegalArgumentException " + e);
        }

        MusicLogUtils.v(TAG, "getDrmRightsStatus: rightsStatus=" + rightsStatus);
        return rightsStatus;
    }

    /**
     * M: check the DRM rights status, if it can be set as ringtone, returns true.
     *
     * @param cursor the cursor for checking DRM rights status;
     * @param position the song position needed to be set as ringtone;
     *
     * @return Returns true if the file can be played.
     */
    private boolean checkDrmRightsForRingtone(Cursor cursor, int position) {
        if (!OmaDrmUtils.isOmaDrmEnabled()) {
            return true;
        }
        int oldPos = cursor.getPosition();
        cursor.moveToPosition(position);
        int isDrm = cursor.getInt(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.IS_DRM));
        MusicLogUtils.d(TAG, "checkDrmRightsForRingtone: isDrm=" + isDrm);
        if (isDrm == 0) {
            return true;
        }
        Uri uri = null;
        int rightsStatus = -1;
        uri = getUri(cursor);
        try {
            rightsStatus = mDrmClient.checkRightsStatus(uri, DrmStore.Action.RINGTONE);
            return (rightsStatus == DrmStore.RightsStatus.RIGHTS_VALID);
        } catch (IllegalArgumentException e) {
            MusicLogUtils.v(TAG, "checkDrmRightsForRingtone throw IllegalArgumentException " + e);
            return (rightsStatus == DrmStore.RightsStatus.RIGHTS_VALID);
        } finally {
        MusicLogUtils.v(TAG, "checkDrmRightsForRingtone: rightsStatus=" + rightsStatus);
            cursor.moveToPosition(oldPos);
        }

    }
    /**
     * M: check the DRM rights status, if it can be played, returns true.
     *
     * @param cursor the cursor for checking DRM rights status;
     * @param position the song position needed to play;
     * @param isNowplaying whether the cursor is a instance of Newplaying, if
     *              not the play queue and current position will be check;
     * @param showFlInvalidToast indicates whether need to show toast when FL is invalid
     *
     * @return Returns true if the file can be played.
     */
    private boolean checkDrmRightsForPlay(Cursor cursor,
     int position, boolean isNowplaying, boolean showFlInvalidToast) {
        if (!OmaDrmUtils.isOmaDrmEnabled()) {
            return true;
        }
        mCurTrackPos = position;
        cursor.moveToPosition(position);
        int isDrm = cursor.getInt(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.IS_DRM));
        MusicLogUtils.d(TAG, "checkDrmRightsForPlay: isDrm=" + isDrm);
        if (isDrm == 0) {
            return true;
        }
        OmaDrmUtils.showConsumerDialog(this, mDrmClient, getUri(cursor), this);
        return false;

/*            mCurTrackPos = position;
            int drmMethod =
             cursor.getInt(cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.DRM_METHOD));
            MusicLogUtils.v(TAG, "checkDrmRightsForPlay: drmMethod=" + drmMethod);
            /// when modify the suffix of drm file ,drmMedthod in db is -1 in JB edtion
            if (drmMethod == -1) {
                showToast(getString(R.string.playback_failed));
                return false;
            }

            int rightsStatus = getDrmRightsStatus(cursor);
            switch (rightsStatus) {
                case DrmStore.RightsStatus.RIGHTS_VALID:
                    /// M: if rigits is valid and drm method is FL play it directory @{
                    if (drmMethod == OmaDrmStore.Method.FL) {
                        // if FL, play directly
                        return true;
                    }
                    /// @}

                    // check for possibility of clicking an playing/opened file
                    // In now-playing list, always pop up dialog,
                    // since file will always be re-opened.
                    if (!isNowplaying && MusicUtils.sService != null) {
                        long[] list = MusicUtils.getSongListForCursor(cursor);
                        long[] curPlaylist = MusicUtils.sService.getQueue();
                        int curPos = MusicUtils.sService.getQueuePosition();
                        //MusicLogUtils.v(TAG, "track cursor list=" + list.toString());
                        //MusicLogUtils.v(TAG, "curPlaylist=" + curPlaylist.toString());
                        MusicLogUtils.v(TAG, "curPos=" + curPos + ", position=" + position);
                        if (Arrays.equals(list, curPlaylist) && position == curPos
                                && !MediaPlaybackService.mTrackCompleted) {
                            /// same playlist, same now-playing position
                            // do not need to show the consume dialog
                            // instead, play directly
                            return true;
                        }
                    }
                default:
                    break;

               case OmaDrmStore.RightsStatus.RIGHTS_INVALID:
                    MusicLogUtils.v(TAG, "OmaDrmStore.RightsStatus.RIGHTS_INVALID=");
                    /// M: if rigits is invalid and drm method is FL,toast @{
                    if (drmMethod == OmaDrmStore.DrmMethod.METHOD_FL) {
                        if (showFlInvalidToast == true) {
                            showToast(getString(R.string.fl_invalid));
                        }
                        return false;
                    }
                    /// @}
                    OmaDrmUiUtils.showRefreshLicenseDialog(mDrmClient, this,
                        getUri(cursor), null);
                    break;
                case OmaDrmStore.RightsStatus.SECURE_TIMER_INVALID:
                    OmaDrmUiUtils.showSecureTimerInvalidDialog(this, null, null);
                    break;
                default:
                    MusicLogUtils.v(TAG, "No such rights status for current DRM file!!");
                    break;
*/
    }

    /**
     * M: Show the given text to screen.
     *
     * @param toastText Need show text.
     */
    private void showToast(CharSequence toastText) {
        if (mToast == null) {
            mToast = Toast.makeText(getApplicationContext(), toastText, Toast.LENGTH_SHORT);
        }
        mToast.setText(toastText);
        mToast.show();
    }

    /**
     * M: judge whether show ringtone menu,make invalid drm file cann't be set as ringtone
     * @param isDrm
     * @param drmMethod
     * @return
     */
    private boolean canDispalyRingtone(int isDrm, int drmMethod) {
        if (isDrm != 1) {
            return true;
        } else {
            return checkDrmRightsForRingtone(mTrackCursor, mSelectedPosition);
        }
    }

  /*  private PluginUtils.IMusicListenter mMusicListenerForPlugin =
     new PluginUtils.IMusicListenter() {
        public void onCallPlay(Context context, long[] list, int position) {
            MusicUtils.playAll(context, list, position);
        }

        public void onCallAddToPlaylist(Context context, long[] ids, long playlistid) {
            MusicUtils.addToPlaylist(context, ids, playlistid);
        }

        public void onCallAddToCurrentPlaylist(Context context, long[] list) {
            MusicUtils.addToCurrentPlaylist(context, list);
        }

        public void onCallClearPlaylist(Context context, int playlistid) {
            MusicUtils.clearPlaylist(context, playlistid);
        }
    };*/

}

