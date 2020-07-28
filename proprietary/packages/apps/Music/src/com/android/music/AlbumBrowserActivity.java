/*
 * Copyright (C) 2016 MediaTek Inc.
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

import android.app.ActionBar;
import android.app.Activity;
//import com.android.music.MusicUtils.ServiceToken;

import android.app.ListActivity;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.SearchManager;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.provider.MediaStore;
import android.text.TextUtils;
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
//import android.widget.CursorAdapter;
//import android.widget.ExpandableListView;
import android.widget.ImageView;
//import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.SectionIndexer;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
//import android.widget.AdapterView.AdapterContextMenuInfo;

import com.android.music.AlbumBrowserActivity.AlbumListAdapter.ViewHolder;
import com.android.music.MusicUtils.ServiceToken;

public class AlbumBrowserActivity extends ListActivity
    implements View.OnCreateContextMenuListener, MusicUtils.Defs, ServiceConnection
{
    private static final String TAG = "Album";
    private static final String DIALOG_TAG_CREATE = "Create";
    private String mCurrentAlbumId;
    private String mCurrentAlbumName;
    private String mCurrentArtistNameForAlbum;
    boolean mIsUnknownArtist;
    boolean mIsUnknownAlbum;
    private AlbumListAdapter mAdapter;
    private boolean mAdapterSent;
    private final static int SEARCH = CHILD_MENU_BASE;
    private static int sLastListPosCourse = -1;
    private static int sLastListPosFine = -1;
    private ServiceToken mToken;

    /// M: Is the sdcard mounted
    private boolean mIsMounted = true;

    /// M: get the instance of IMediaPlaybackService,when bind service could call
    /// functions that included IMediaPlaybackService @{
    private IMediaPlaybackService mService = null;
    private boolean mWithtabs = false;
    private int mOrientation;
    /// @}

    /// M: Whether the activity is in background or not.
    private boolean mIsInBackgroud = false;

    /// M: Rescan delay time(ms)
    private static final long RESCAN_DELAY_TIME = 1000;

    /// M: We need to close submenu when sdcard is unmounted
    private SubMenu mSubMenu = null;

    /// M: Add for lazy loading album art
    private Context mContext;
    private ListView mListView;
    private int mVisibleItemsCount = 20;
    /// M: save the input of SearchView
    private CharSequence mQueryText;
    /// M: Add search button in actionbar when nowplaying not exist
    MenuItem mSearchItem;

    /// M: Album cursor columns
    private final String[] mCursorCols = new String[] {
            MediaStore.Audio.Albums._ID,
            MediaStore.Audio.Albums.ARTIST,
            MediaStore.Audio.Albums.ALBUM,
            MediaStore.Audio.Albums.ALBUM_ART,
            /// M: add for chinese sorting
    };

    private boolean mActvityResumAfterCreate = true;
    /// M: It's the delay time(ms) for show emptyview.
    private static final int EMPTYSHOW_SPEND = 200;

    public AlbumBrowserActivity()
    {
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        /// M: get intent @{
        MusicLogUtils.d(TAG, "onCreate");
        Intent intent = getIntent();
        mContext = getApplicationContext();
        /// @}
        if (icicle != null) {
            mCurrentAlbumId = icicle.getString("selectedalbum");
            mArtistId = icicle.getString("artist");
        } else {
            /// M: ALPS00122166
            mCurrentAlbumId = intent.getStringExtra("selectedalbum");
            mArtistId = intent.getStringExtra("artist");
        }
        /// M: get the value of "withtabs",if no value of "withtabs",get false
        mWithtabs = intent.getBooleanExtra("withtabs", false);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        /// M: when start from tab mode
        if (mWithtabs) {
            requestWindowFeature(Window.FEATURE_NO_TITLE);
        } else {
            /// M: Set the action bar on the right to be up navigation @{
            ActionBar actionBar = getActionBar();
            actionBar.setDisplayHomeAsUpEnabled(true);
            actionBar.setHomeButtonEnabled(true);
            /// @}
        }

        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        /// M: Call this only when this is without tab.
        //mToken = MusicUtils.bindToService(this, this);

        IntentFilter f = new IntentFilter();
        f.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        f.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        f.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        /// M: add action of intent @{
        f.addAction(Intent.ACTION_MEDIA_MOUNTED);
        /// @}
        f.addDataScheme("file");
        registerReceiver(mScanListener, f);
        /// M: register HDMI observer @{
        //IntentFilter hdmiIntentFilter = new IntentFilter();
        //hdmiIntentFilter.addAction(Intent.ACTION_HDMI_PLUG);
        //registerReceiver(mHdmiChangedReceiver, hdmiIntentFilter);
        /// @}
        if (mWithtabs) {
            /// M: when start from tab mode, don't need bind service
            /// and MusicBrowser will update Nowplaying
            setContentView(R.layout.media_picker_activity);
        } else {
            /// M: if not with tab enter current Acivity,set layout that include nowplaying
            ///Item adn bind to service @{
            setContentView(R.layout.media_picker_activity_nowplaying);
            mToken = MusicUtils.bindToService(this, this);
            mOrientation = getResources().getConfiguration().orientation;
            /// @}
        }
        mListView = getListView();
        mListView.setOnCreateContextMenuListener(this);
        mListView.setTextFilterEnabled(true);
        mAdapter = (AlbumListAdapter) getLastNonConfigurationInstance();
        if (mAdapter == null) {
            MusicLogUtils.d(TAG, "starting query");
            mAdapter = new AlbumListAdapter(
                    getApplication(),
                    this,
                    R.layout.track_list_item,
                    mAlbumCursor,
                    new String[] {},
                    new int[] {});
            setListAdapter(mAdapter);
            setTitle(R.string.working_albums);
            getAlbumCursor(mAdapter.getQueryHandler(), null);
        } else {
            mAdapter.setActivity(this);
            /// M: when switch language ,reload String
            mAdapter.reloadStringOnLocaleChanges();
            setListAdapter(mAdapter);
            mAlbumCursor = mAdapter.getCursor();
            if (mAlbumCursor != null) {
                init(mAlbumCursor);
            } else {
                getAlbumCursor(mAdapter.getQueryHandler(), null);
            }
        }
    }

    @Override
    public Object onRetainNonConfigurationInstance() {
        mAdapterSent = true;
        return mAdapter;
    }

    @Override
    public void onSaveInstanceState(Bundle outcicle) {
        // need to store the selected item so we don't lose it in case
        // of an orientation switch. Otherwise we could lose it while
        // in the middle of specifying a playlist to add the item to.
        outcicle.putString("selectedalbum", mCurrentAlbumId);
        outcicle.putString("artist", mArtistId);
        super.onSaveInstanceState(outcicle);
    }

    @Override
    public void onDestroy() {
        MusicLogUtils.d(TAG, "onDestroy");
        ListView lv = getListView();
        if (lv != null) {
            sLastListPosCourse = lv.getFirstVisiblePosition();
            View cv = lv.getChildAt(0);
            if (cv != null) {
                sLastListPosFine = cv.getTop();
            }
        }
        /// M: Determine whether mToken is valid.
        if (mToken != null) {
            MusicUtils.unbindFromService(mToken);
            /// M:make the object of Service be null,to avoid memory leaks.
            mService = null;
        }

        // If we have an adapter and didn't send it off to another activity yet, we should
        // close its cursor, which we do by assigning a null cursor to it. Doing this
        // instead of closing the cursor directly keeps the framework from accessing
        // the closed cursor later.
        if (!mAdapterSent && mAdapter != null) {
            mAdapter.quitLazyLoadingThread();
            mAdapter.changeCursor(null);
        }
        // Because we pass the adapter to the next activity, we need to make
        // sure it doesn't keep a reference to this activity. We can do this
        // by clearing its DatasetObservers, which setListAdapter(null) does.
        setListAdapter(null);
        mAdapter = null;
        unregisterReceiver(mScanListener);
        //unregisterReceiver(mHdmiChangedReceiver);
        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        MusicLogUtils.d(TAG, "onResume:");
        IntentFilter f = new IntentFilter();
        f.addAction(MediaPlaybackService.META_CHANGED);
        f.addAction(MediaPlaybackService.QUEUE_CHANGED);
        registerReceiver(mTrackListListener, f);
        mTrackListListener.onReceive(null, null);

        MusicUtils.setSpinnerState(this);
        mIsInBackgroud = false;
        /// M: update the nowplaying item @{
        if (mService != null) {
            MusicUtils.updateNowPlaying(this, mOrientation);
        }
        /// @}
        /// M: if deleted files in background that will not do query,so
        if (!mActvityResumAfterCreate) {
            refreshEmptyView();
        }
        MusicLogUtils.d(TAG, "onResume<<<");
    }

    private BroadcastReceiver mTrackListListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            getListView().invalidateViews();
            /// M: if mService exist,do update NowPlaying Item @{
            if (mService != null) {
                MusicUtils.updateNowPlaying(AlbumBrowserActivity.this, mOrientation);
            /// @}
            }
        }
    };

    private BroadcastReceiver mScanListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            /// M: get the action and current Storage State @{
            String action = intent.getAction();
            String status = Environment.getExternalStorageState();
            /// @}
            MusicLogUtils.d(TAG, "mScanListener.onReceive:" + action + ", status = " + status);
            /// M: Determine whether the action that get from intent match with
            /// ACTION_MEDIA_SCANNER_STARTED or ACTION_MEDIA_SCANNER_FINISHED @{
            if (Intent.ACTION_MEDIA_SCANNER_STARTED.equals(action) ||
                    Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
            /// @}
                MusicUtils.setSpinnerState(AlbumBrowserActivity.this);
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
            } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                mIsMounted = false;
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
                closeContextMenu();
                closeOptionsMenu();
                if (mSubMenu != null) {
                    mSubMenu.close();
                }
                mReScanHandler.sendEmptyMessageDelayed(0, RESCAN_DELAY_TIME);
            } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
                mIsMounted = true;
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
            }
            /// @}
        }
    };

    private Handler mReScanHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (mAdapter != null) {
                getAlbumCursor(mAdapter.getQueryHandler(), null);
            }
        }
    };

    @Override
    public void onPause() {
        MusicLogUtils.d(TAG, "onPause");
        mActvityResumAfterCreate = false;
        if (mSubMenu != null) {
            mSubMenu.close();
        }
        unregisterReceiver(mTrackListListener);
        mReScanHandler.removeCallbacksAndMessages(null);
        mIsInBackgroud = true;
        super.onPause();
    }

    /**
     * Init the list.
     *
     * @param c the cursor is used to init the list adapter
     */
    public void init(Cursor c) {

        if (mAdapter == null) {
            return;
        }
        mAdapter.changeCursor(c); // also sets mAlbumCursor

        if (mAlbumCursor == null) {
            MusicLogUtils.d(TAG, "mAlbumCursor is null");
            /// M: if mAlbumCursor is invalid ,show DatabaseError layout
            MusicUtils.displayDatabaseError(this, mIsMounted);
            closeContextMenu();
            mReScanHandler.sendEmptyMessageDelayed(0, RESCAN_DELAY_TIME);
            return;
        }
        /// M: when Scanning media file show scanning progressbar ,if files exist
        //   show file list,or show no music @{
            MusicUtils.emptyShow(getListView(), AlbumBrowserActivity.this);
        /// @}
        // restore previous position
        if (sLastListPosCourse >= 0) {
            getListView().setSelectionFromTop(sLastListPosCourse, sLastListPosFine);
            sLastListPosCourse = -1;
        }

        MusicUtils.hideDatabaseError(this);
        //Not required
        //MusicUtils.updateButtonBar(this, R.id.albumtab);
        setTitle();
    }

    private void setTitle() {
        CharSequence fancyName = "";
        if (mAlbumCursor != null && mAlbumCursor.getCount() > 0) {
            mAlbumCursor.moveToFirst();
            fancyName = mAlbumCursor.getString(
                    mAlbumCursor.getColumnIndex(MediaStore.Audio.Albums.ARTIST));
            if (fancyName == null || fancyName.equals(MediaStore.UNKNOWN_STRING)) {
                fancyName = getText(R.string.unknown_artist_name);
            }
        }

        if (mArtistId != null && fancyName != null) {
            setTitle(fancyName);
        } else {
            setTitle(R.string.albums_title);
        }
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View view, ContextMenuInfo menuInfoIn) {
        menu.add(0, PLAY_SELECTION, 0, R.string.play_selection);
        mSubMenu = menu.addSubMenu(0, ADD_TO_PLAYLIST, 0, R.string.add_to_playlist);
        MusicUtils.makePlaylistMenu(this, mSubMenu);
        menu.add(0, DELETE_ITEM, 0, R.string.delete_item);

        AdapterContextMenuInfo mi = (AdapterContextMenuInfo) menuInfoIn;
        mAlbumCursor.moveToPosition(mi.position);
        mCurrentAlbumId = mAlbumCursor.getString(
                mAlbumCursor.getColumnIndexOrThrow(MediaStore.Audio.Albums._ID));
        mCurrentAlbumName = mAlbumCursor.getString(
                mAlbumCursor.getColumnIndexOrThrow(MediaStore.Audio.Albums.ALBUM));
        mCurrentArtistNameForAlbum = mAlbumCursor.getString(
                mAlbumCursor.getColumnIndexOrThrow(MediaStore.Audio.Albums.ARTIST));
        mIsUnknownArtist = mCurrentArtistNameForAlbum == null ||
                mCurrentArtistNameForAlbum.equals(MediaStore.UNKNOWN_STRING);
        mIsUnknownAlbum = mCurrentAlbumName == null ||
                mCurrentAlbumName.equals(MediaStore.UNKNOWN_STRING);
        if (mIsUnknownAlbum) {
            menu.setHeaderTitle(getString(R.string.unknown_album_name));
        } else {
            menu.setHeaderTitle(mCurrentAlbumName);
        }
        if (!mIsUnknownAlbum || !mIsUnknownArtist) {
            menu.add(0, SEARCH, 0, R.string.search_title);
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case PLAY_SELECTION: {
                // play the selected album
                long [] list = MusicUtils.getSongListForAlbum(
                        this, Long.parseLong(mCurrentAlbumId));
                MusicUtils.playAll(this, list, 0);
                return true;
            }

            case QUEUE: {
                long [] list = MusicUtils.getSongListForAlbum(
                        this, Long.parseLong(mCurrentAlbumId));
                MusicUtils.addToCurrentPlaylist(this, list);
                return true;
            }

            case NEW_PLAYLIST: {
                /// M: get the object of ListView.
                Activity parent = getParent();
                //Intent intent = new Intent();
                //intent.setClass(this, CreatePlaylist.class);
                /// M: Restore mCurrentAlbumId in CreatePlaylist, so that
                ///when onActivityResult called, it will return
                /// the restored value to avoid mCurrentAlbumId become null when
                /// activity has been restarted, such as
                /// switch a language.
                //intent.putExtra(MusicUtils.ADD_TO_PLAYLIST_ITEM_ID, mCurrentAlbumId);
                /// M: Add to indicate the save_as_playlist and new_playlist
                //intent.putExtra(MusicUtils.SAVE_PLAYLIST_FLAG, MusicUtils.NEW_PLAYLIST);
                //if (parent == null) {
                   // startActivityForResult(intent, NEW_PLAYLIST);
                /// M: use the object of parent to new Playlist @{
               // } else {
                    /// M: Restore start activity tab so that parent MusicBrowserActivity
                    ///can return the activity result
                    /// to this start activity.
                //   intent.putExtra(MusicUtils.START_ACTIVITY_TAB_ID,
                //                    MusicBrowserActivity.ALBUM_INDEX);
                //   parent.startActivityForResult(intent, NEW_PLAYLIST);
                //}
                /// @}

                    showCreateDialog(String.valueOf(mCurrentAlbumId),
                                     MusicBrowserActivity.ALBUM_INDEX,
                                     MusicUtils.NEW_PLAYLIST);

                return true;
            }

            case PLAYLIST_SELECTED: {
                long [] list = MusicUtils.getSongListForAlbum(
                        this, Long.parseLong(mCurrentAlbumId));
                long playlist = item.getIntent().getLongExtra("playlist", 0);
                MusicUtils.addToPlaylist(this, list, playlist);
                return true;
            }
            case DELETE_ITEM: {
                long [] list = MusicUtils.getSongListForAlbum(
                        this, Long.parseLong(mCurrentAlbumId));
                //NOt required
                Bundle b = new Bundle();
                /// M: Get string in DeleteItems Activity to get current language string. @{
                //String f;
                //if (android.os.Environment.isExternalStorageRemovable()) {
                //    f = getString(R.string.delete_album_desc);
                //} else {
                //   f = getString(R.string.delete_album_desc_nosdcard);
                //}
                //String desc = String.format(f, mCurrentAlbumName);
                //b.putString("description", desc);
                b.putInt(MusicUtils.DELETE_DESC_STRING_ID, R.string.delete_album_desc);
                b.putString(MusicUtils.DELETE_DESC_TRACK_INFO, mCurrentAlbumName);
                /// @}
                b.putLongArray("items", list);
                Intent intent = new Intent();
                intent.setClass(this, DeleteItems.class);
                intent.putExtras(b);
                startActivityForResult(intent, -1);
                return true;
            }
            case SEARCH:
                doSearch();
                return true;

        }
        return super.onContextItemSelected(item);
    }

 private void showCreateDialog(String plstItemId,
                                   int tabId,
                                   String plstFlag) {
        MusicLogUtils.d(TAG, "showEditDialog>>");
        removeOldFragmentByTag(DIALOG_TAG_CREATE);
        FragmentManager fragmentManager = getFragmentManager();
        MusicLogUtils.d(TAG, "<showDeleteDialog> fragmentManager = " + fragmentManager);
        CreatePlaylist newFragment = CreatePlaylist.newInstance(plstItemId,
                                                                tabId,
                                                                plstFlag,
                                                                null);
        //((WeekSelector) newFragment).setOnClickListener(mDeleteDialogListener);
        newFragment.show(fragmentManager, DIALOG_TAG_CREATE);
        fragmentManager.executePendingTransactions();
    }

   private void removeOldFragmentByTag(String tag) {
        MusicLogUtils.d(TAG, "<removeOldFragmentByTag> tag = " + tag);
        FragmentManager fragmentManager = getFragmentManager();
        MusicLogUtils.d(TAG, "<removeOldFragmentByTag> fragmentManager = " + fragmentManager);
        DialogFragment oldFragment = (DialogFragment) fragmentManager.findFragmentByTag(tag);
        MusicLogUtils.d(TAG, "<removeOldFragmentByTag> oldFragment = " + oldFragment);
        if (null != oldFragment) {
            oldFragment.dismissAllowingStateLoss();
        }
    }



    void doSearch() {
        CharSequence title = null;
        String query = "";

        Intent i = new Intent();
        i.setAction(MediaStore.INTENT_ACTION_MEDIA_SEARCH);
        i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

        title = "";
        if (!mIsUnknownAlbum) {
            query = mCurrentAlbumName;
            i.putExtra(MediaStore.EXTRA_MEDIA_ALBUM, mCurrentAlbumName);
            title = mCurrentAlbumName;
        }
        if (!mIsUnknownArtist) {
            query = query + " " + mCurrentArtistNameForAlbum;
            i.putExtra(MediaStore.EXTRA_MEDIA_ARTIST, mCurrentArtistNameForAlbum);
            title = title + " " + mCurrentArtistNameForAlbum;
        }
        // Since we hide the 'search' menu item when both album and artist are
        // unknown, the query and title strings will have at least one of those.
        i.putExtra(MediaStore.EXTRA_MEDIA_FOCUS, MediaStore.Audio.Albums.ENTRY_CONTENT_TYPE);
        title = getString(R.string.mediasearch, title);
        i.putExtra(SearchManager.QUERY, query);

        startActivity(Intent.createChooser(i, title));
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        switch (requestCode) {
            case SCAN_DONE:
                if (resultCode == RESULT_CANCELED) {
                    finish();
                } else {
                    getAlbumCursor(mAdapter.getQueryHandler(), null);
                }
                break;

            case NEW_PLAYLIST:
                if (resultCode == RESULT_OK) {
                    Uri uri = intent.getData();
                    /// M: Get selected album id need add to new playlist from intent to
                    ///avoid mCurrentAlbumId to be null
                    /// when activity restart causing by configaration change. @{
                    String selectAlbumId =
                                   intent.getStringExtra(MusicUtils.ADD_TO_PLAYLIST_ITEM_ID);
                    MusicLogUtils.d(TAG, "onActivityResult: selectAlbumId = " + selectAlbumId);
                    if (uri != null && selectAlbumId != null) {
                        long [] list = MusicUtils.getSongListForAlbum(this,
                                              Long.parseLong(selectAlbumId));
                        MusicUtils.addToPlaylist(this, list,
                                                 Long.parseLong(uri.getLastPathSegment()));
                    }
                    /// @}
                }
                break;
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        Intent intent = new Intent(Intent.ACTION_PICK);
        intent.setDataAndType(Uri.EMPTY, "vnd.android.cursor.dir/track");
        intent.putExtra("album", Long.valueOf(id).toString());
        intent.putExtra("artist", mArtistId);
        startActivity(intent);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        /// M: mWithtabs marks enter the Activity with the tab switch.
        if (mWithtabs) {
            return super.onCreateOptionsMenu(menu);
        }
        menu.add(0, PARTY_SHUFFLE, 0, R.string.party_shuffle);
        // icon will be set in onPrepareOptionsMenu()
        menu.add(0, SHUFFLE_ALL, 0, R.string.shuffle_all).setIcon(R.drawable.ic_menu_shuffle);
        /// M: add Audio effets menu.
        menu.add(0, EFFECTS_PANEL, 0, R.string.effects_list_title).setIcon(R.drawable.ic_menu_eq);
        /// M: Add search view
        mSearchItem = MusicUtils.addSearchView(this, menu, mQueryTextListener, null);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        /// M: mWithtabs marks enter the Activity with the tab switch.
        if (mWithtabs) {
            return super.onPrepareOptionsMenu(menu);
        }
        /// M: When album cursor is null, it mean database not ready and need not show menu.
        if (mAlbumCursor == null) {
            MusicLogUtils.d(TAG, "Album cursor is null, need not show option menu!");
            return false;
        }
        MusicUtils.setPartyShuffleMenuIcon(menu);
        /// M: when the AudioEffect is exist , the effects_panel menu could Visible,
        /// otherwise ,set invisible.@{
        MusicUtils.setEffectPanelMenu(mContext, menu);
        /// M: restore the input of SearchView when config change @{
        if (mQueryText != null && !mQueryText.toString().equals("")) {
            MusicLogUtils.d(TAG, "setQueryText:" + mQueryText);
            SearchView searchView = (SearchView) mSearchItem.getActionView();
            searchView.setQuery(mQueryText, false);
            mQueryText = null;
        }
        //
        /// @}
        return true;
        /// @}
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        /// M:  mWithtabs marks enter the Activity with the tab switch @{
        if (mWithtabs) {
            return super.onOptionsItemSelected(item);
        }
        /// @}
        Cursor cursor;
        switch (item.getItemId()) {
            case PARTY_SHUFFLE:
                MusicUtils.togglePartyShuffle();
                break;

            case SHUFFLE_ALL:
                cursor = MusicUtils.query(this, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                        new String [] { MediaStore.Audio.Media._ID},
                        MediaStore.Audio.Media.IS_MUSIC + "=1", null,
                        /// M: add for chinese sorting
                        MediaStore.Audio.Media.DEFAULT_SORT_ORDER);
                if (cursor != null) {
                    MusicUtils.shuffleAll(this, cursor);
                    cursor.close();
                }
                return true;

                /// M: when select the item of effects_panel, jump to AudioEffect Activity @{
            case EFFECTS_PANEL:
                return MusicUtils.startEffectPanel(this);
                /// @}

                /// M:  Navigation button press back @{
            case android.R.id.home:
                if (!mIsInBackgroud) {
                    onBackPressed();
                }
                break;
                /// @}

                /// M: when selected the search Item,start to search @{
            case R.id.search:
                onSearchRequested();
                return true;
                /// @}

        }
        return super.onOptionsItemSelected(item);
    }

    private Cursor getAlbumCursor(AsyncQueryHandler async, String filter) {
        /// M: add for chinese sorting
        String sortOrder = MediaStore.Audio.Albums.DEFAULT_SORT_ORDER;
        Cursor ret = null;
        if (mArtistId != null) {
            Uri uri = MediaStore.Audio.Artists.Albums.getContentUri("external",
                    Long.valueOf(mArtistId));
            if (!TextUtils.isEmpty(filter)) {
                uri = uri.buildUpon().appendQueryParameter("filter", Uri.encode(filter)).build();
            }
            if (async != null) {
                async.startQuery(0, null, uri,
                        mCursorCols, null, null, sortOrder);
            } else {
                ret = MusicUtils.query(this, uri,
                        mCursorCols, null, null, sortOrder);
            }
        } else {
            Uri uri = MediaStore.Audio.Albums.EXTERNAL_CONTENT_URI;
            if (!TextUtils.isEmpty(filter)) {
                uri = uri.buildUpon().appendQueryParameter("filter", Uri.encode(filter)).build();
            }
            if (async != null) {
                async.startQuery(0, null,
                        uri,
                        mCursorCols, null, null, sortOrder);
            } else {
                ret = MusicUtils.query(this, uri,
                        mCursorCols, null, null, sortOrder);
            }
        }
        return ret;
    }

    static class AlbumListAdapter extends SimpleCursorAdapter implements SectionIndexer {

        //not required
        private final BitmapDrawable mDefaultAlbumIcon;
        private int mAlbumIdx;
        private int mArtistIdx;
        private int mAlbumArtIndex;
        private final Resources mResources;
        private final StringBuilder mStringBuilder = new StringBuilder();
        private String mUnknownAlbum;
        private String mUnknownArtist;
        private final String mAlbumSongSeparator;
        private final Object[] mFormatArgs = new Object[1];
        private AlphabetIndexer mIndexer;
        private AlbumBrowserActivity mActivity;
        private AsyncQueryHandler mQueryHandler;
        private String mConstraint = null;
        private boolean mConstraintIsValid = false;
        /// M: add for chinese sorting
        private int mAlbumPinyinIdx;
        /// M: add for lazy loading album art
        private static final int REFRESH_ALBUM_ART_DELAY_TIME = 100;
        private final BitmapDrawable mBackgroundAlbumIcon;
        private HandlerThread mLazyLoadingThread;
        private Handler mLazyLoaingHandler;
        private int mWhat = 0;

        static class ViewHolder {
            TextView mLine1;
            TextView mLine2;
            ImageView mPlayIndicator;
            ImageView mIcon;
        }

        class QueryHandler extends AsyncQueryHandler {
            QueryHandler(ContentResolver res) {
                super(res);
            }

            @Override
            protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
                /// M: Log out query result
                MusicLogUtils.d(TAG, "query complete: "
                        + (cursor == null ? null : cursor.getCount()));
                mActivity.init(cursor);
            }
        }

        AlbumListAdapter(Context context, AlbumBrowserActivity currentactivity,
                int layout, Cursor cursor, String[] from, int[] to) {
            super(context, layout, cursor, from, to);

            mActivity = currentactivity;
            mQueryHandler = new QueryHandler(context.getContentResolver());

            mUnknownAlbum = context.getString(R.string.unknown_album_name);
            mUnknownArtist = context.getString(R.string.unknown_artist_name);
            mAlbumSongSeparator = context.getString(R.string.albumsongseparator);

            Resources r = context.getResources();
            //not required
            //mNowPlayingOverlay = r.getDrawable(R.drawable.indicator_ic_mp_playing_list);

            Bitmap b = BitmapFactory.decodeResource(r, R.drawable.albumart_mp_unknown_list);
            mDefaultAlbumIcon = new BitmapDrawable(context.getResources(), b);
            // no filter or dither, it's a lot faster and we can't tell the difference
            mDefaultAlbumIcon.setFilterBitmap(false);
            mDefaultAlbumIcon.setDither(false);
            getColumnIndices(cursor);
            mResources = context.getResources();

            /// M: Create background default album icon
            Bitmap backgroud = Bitmap.createBitmap(b.getWidth(),
                                                   b.getHeight(),
                                                   Bitmap.Config.ARGB_8888);
            mBackgroundAlbumIcon = new BitmapDrawable(context.getResources(), backgroud);
            mBackgroundAlbumIcon.setFilterBitmap(false);
            mBackgroundAlbumIcon.setDither(false);
            /// M: Lazy loading album art in lazyLoadingThread when need, arg1 is album id and arg2
            /// is refresh position. {@
            mLazyLoadingThread = new HandlerThread("LazyLoading");
            mLazyLoadingThread.start();
            mLazyLoaingHandler = new Handler(mLazyLoadingThread.getLooper()) {
                @Override
                public void handleMessage(Message msg) {
                    int albumId = msg.arg1;
                    if (albumId >= 0) {
                        MusicUtils.getCachedArtwork(mActivity, msg.arg1, mDefaultAlbumIcon);
                    }
                    Message message = mActivity.mRefreshAlbumArtHandler.obtainMessage(0,
                                                                                      albumId,
                                                                                      msg.arg2);
                    mActivity.mRefreshAlbumArtHandler.sendMessageDelayed(message,
                                                                REFRESH_ALBUM_ART_DELAY_TIME);
                }
            };
            /// @}
        }

        private void getColumnIndices(Cursor cursor) {
            if (cursor != null) {
                mAlbumIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Albums.ALBUM);
                /// M: add for chinese sorting
               // mAlbumPinyinIdx =
                    //  cursor.getColumnIndexOrThrow(MediaStore.Audio.Albums.DEFAULT_SORT_ORDER);
                mArtistIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Albums.ARTIST);
                mAlbumArtIndex = cursor.getColumnIndexOrThrow(MediaStore.Audio.Albums.ALBUM_ART);

                if (mIndexer != null) {
                    mIndexer.setCursor(cursor);
                } else {
                    /// M: add for chinese sorting
                    mIndexer = new MusicAlphabetIndexer(cursor,
                                                        mAlbumIdx,
                                                        mResources.getString(
                            R.string.fast_scroll_alphabet));
                }
            }
        }

        public void setActivity(AlbumBrowserActivity newactivity) {
            mActivity = newactivity;
        }

        public AsyncQueryHandler getQueryHandler() {
            return mQueryHandler;
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
           View v = super.newView(context, cursor, parent);
           ViewHolder vh = new ViewHolder();
           vh.mLine1 = (TextView) v.findViewById(R.id.line1);
           vh.mLine2 = (TextView) v.findViewById(R.id.line2);
           vh.mPlayIndicator = (ImageView) v.findViewById(R.id.play_indicator);
           vh.mIcon = (ImageView) v.findViewById(R.id.icon);
           /// M: Use default black album icon to be background drawable.
           vh.mIcon.setBackgroundDrawable(mBackgroundAlbumIcon);
           vh.mIcon.setPadding(0, 0, 1, 0);
           v.setTag(vh);
           return v;
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {

            ViewHolder vh = (ViewHolder) view.getTag();

            String name = cursor.getString(mAlbumIdx);
            String displayname = name;
            boolean unknown = name == null || name.equals(MediaStore.UNKNOWN_STRING);
            if (unknown) {
                displayname = mUnknownAlbum;
            }
            vh.mLine1.setText(displayname);

            name = cursor.getString(mArtistIdx);
            displayname = name;
            if (name == null || name.equals(MediaStore.UNKNOWN_STRING)) {
                displayname = mUnknownArtist;
            }
            vh.mLine2.setText(displayname);

            //ImageView iv = vh.mIcon;
            // We don't actually need the path to the thumbnail file,
            // we just use it to see if there is album art or not
            String art = cursor.getString(mAlbumArtIndex);
            long aid = cursor.getLong(0);
            /*if (unknown || art == null || art.length() == 0) {
                iv.setImageDrawable(null);
            } else {
                Drawable d = MusicUtils.getCachedArtwork(context, aid, mDefaultAlbumIcon);
                iv.setImageDrawable(d);
            }*/

            long currentalbumid = MusicUtils.getCurrentAlbumId();
            ImageView iv = vh.mPlayIndicator;
            if (currentalbumid == aid) {
                iv.setVisibility(View.VISIBLE);
            } else {
                iv.setVisibility(View.GONE);
            }

            /// M: Use lazy loading to refresh album art. {@
            iv = vh.mIcon;
            if (unknown || art == null || art.length() == 0) {
                aid = -1;
            }
            iv.setImageDrawable(null);
            if (mLazyLoaingHandler.hasMessages(mWhat)) {
                mLazyLoaingHandler.removeMessages(mWhat);
            }
            Message message = mLazyLoaingHandler.obtainMessage(mWhat,
                                                              (int) aid,
                                                              cursor.getPosition());
            mLazyLoaingHandler.sendMessage(message);
            mWhat++;
            mWhat = mWhat % mActivity.mVisibleItemsCount;
            /// @}
        }

        @Override
        public void changeCursor(Cursor cursor) {
            if (mActivity.isFinishing() && cursor != null) {
                cursor.close();
                cursor = null;
            }
            if (cursor != mActivity.mAlbumCursor) {
                mActivity.mAlbumCursor = cursor;
                getColumnIndices(cursor);
                super.changeCursor(cursor);
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
            Cursor c = mActivity.getAlbumCursor(null, s);
            mConstraint = s;
            mConstraintIsValid = true;
            return c;
        }

        public Object[] getSections() {
            return mIndexer.getSections();
        }

        public int getPositionForSection(int section) {
        /// M: if the mIndexer is valid,we could get the Position.
            if (mIndexer != null) {
                return mIndexer.getPositionForSection(section);
            }
            /// M: default return back 0.
            return 0;
        }

        public int getSectionForPosition(int position) {
            /// M: if the mIndexer is valid,we could get the section.
            if (mIndexer != null) {
                return mIndexer.getSectionForPosition(position);
            }
            /// M: default return back 0.
            return 0;
        }

        /**
         * M: when switch language ,the Activity will be killed by the system,then
         * enter the activity,should reload the unknown_artist_name and unknown_album_name.
         */
        public void reloadStringOnLocaleChanges() {
            String unknownArtist = mActivity.getString(R.string.unknown_artist_name);
            String unknownAlbum = mActivity.getString(R.string.unknown_album_name);
            if (mUnknownArtist != null && !mUnknownArtist.equals(unknownArtist)) {
                mUnknownArtist = unknownArtist;
            }
            if (mUnknownAlbum != null && !mUnknownAlbum.equals(unknownAlbum)) {
                mUnknownAlbum = unknownAlbum;
            }
        }

        /**
         * M: Need quit the lazy loading thread when activity will be destroy.
         */
        void quitLazyLoadingThread() {
            boolean isQuitSuccuss = false;
            if (mLazyLoadingThread != null) {
                isQuitSuccuss = mLazyLoadingThread.quit();
            }
            MusicLogUtils.d(TAG,
            "Quit lazy loading thread when activity ondestroy: isQuitSuccuss = " + isQuitSuccuss);
        }
    }

    private Cursor mAlbumCursor;
    private String mArtistId;

    @Override
    public void onServiceConnected(ComponentName name, IBinder service) {
        /// M: get the object of IMediaPlaybackService.
        mService = IMediaPlaybackService.Stub.asInterface(service);
        /// M: when service connected , update the information of NowPlaying
        //layout same as the MediaPlaybackService.
        MusicUtils.updateNowPlaying(this, mOrientation);
    }

    @Override
    public void onServiceDisconnected(ComponentName name) {
        mService = null;
        finish();
    }

    /*
     *  M: in this Activity ,Configuration changed include (orientation|screenSize).
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mOrientation = newConfig.orientation;
        /// M: If showing database error UI, need not to refresh UI.
        if ((findViewById(R.id.sd_icon).getVisibility()) == View.VISIBLE) {
            MusicLogUtils.d(TAG, "Configuration Changed at database error, return!");
            return;
        }
        if (mService != null) {
            MusicUtils.updateNowPlaying(this, mOrientation);
        }
        /// M: Before invalidateOptionsMenu,save the input of SearchView @{
        if (mSearchItem != null) {
            SearchView searchView = (SearchView) mSearchItem.getActionView();
            mQueryText = searchView.getQuery();
            MusicLogUtils.d(TAG, "searchText:" + mQueryText);
        }
        /// @}
        invalidateOptionsMenu();
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
             intent.setClass(AlbumBrowserActivity.this, QueryBrowserActivity.class);
             intent.putExtra(SearchManager.QUERY, query);
             startActivity(intent);
             mSearchItem.collapseActionView();
             return true;
         }
         /// M: false if the SearchView should perform the default action of showing
         //any suggestions if available.
         public boolean onQueryTextChange(String newText) {
             return false;
         }
     };

    /**
     * M: Refresh Album art if need after finishing load.
     */
    private final Handler mRefreshAlbumArtHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int albumId = msg.arg1;
            int refreshPosition = msg.arg2;
            int firstVisiblePosition = mListView.getFirstVisiblePosition();
            int relativePosition = refreshPosition - firstVisiblePosition;
            int currentVisibleItemCount =
                                 mListView.getLastVisiblePosition() - firstVisiblePosition + 1;
            mVisibleItemsCount = mVisibleItemsCount > currentVisibleItemCount ?
                                 mVisibleItemsCount : currentVisibleItemCount;
            /// Only when the refresh position is visible, we need to update album art.
            if (relativePosition >= 0 && relativePosition <= mVisibleItemsCount) {
                View view = mListView.getChildAt(relativePosition);
                if (view != null) {
                    ViewHolder vh = (ViewHolder) view.getTag();
                    /// When album id not available, use default album art
                    if (albumId < 0) {
                        vh.mIcon.setImageDrawable(mAdapter.mDefaultAlbumIcon);
                    } else {
                        Drawable d = MusicUtils.getCachedArtwork(mContext,
                                                                 albumId,
                                                                 mAdapter.mDefaultAlbumIcon);
                        vh.mIcon.setImageDrawable(d);
                    }
                }
            }
        }
    };

    /**
     * When input the keyword of search ,can't let the optionmenu showed
     */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (mSearchItem != null && event.getKeyCode() == KeyEvent.KEYCODE_MENU) {
            if (mSearchItem.isActionViewExpanded()) {
                return true;
            }
        }
        return super.dispatchKeyEvent(event);
    }
    /// M: listener to the hdmi unplug,cause the different resoultion of screen,
    ///wee need to clear the cache img @{
    private final BroadcastReceiver mHdmiChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            MusicUtils.clearAlbumArtCache();
            getListView().invalidateViews();
        }
    };
    /// @}

    /**
     *  M: show the emptyview when the list's count be 0.
     */
    public void refreshEmptyView() {
        if (getListView().getAdapter() != null
                && getListView().getAdapter().getCount() == 0
                && getListView().getCount() != 0) {
            MusicLogUtils.d(TAG, "getExpandableListView().getCount() = "
                    + getListView().getCount());
            mListHandler.sendEmptyMessageDelayed(0, EMPTYSHOW_SPEND);
        } else {
            MusicUtils.emptyShow(getListView(), AlbumBrowserActivity.this);
        }
    }
    /**
     *  M: show emptyview when get the right list'count.
     */
    public Handler mListHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            refreshEmptyView();
            }
    };
}
