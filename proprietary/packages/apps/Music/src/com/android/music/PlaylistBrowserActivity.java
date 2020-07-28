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
import java.text.Collator;
import java.util.ArrayList;

import android.app.DialogFragment;
import android.app.FragmentManager;
import android.app.ListActivity;
import android.app.SearchManager;
import android.app.SearchableInfo;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.MergeCursor;
import android.database.sqlite.SQLiteException;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.MediaStore;
//Not Required
//import android.util.Log;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.ContextMenu.ContextMenuInfo;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.SimpleCursorAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.AdapterContextMenuInfo;


import com.android.music.MusicUtils.ServiceToken;


public class PlaylistBrowserActivity extends ListActivity
    implements View.OnCreateContextMenuListener, MusicUtils.Defs
{
    private static final String TAG = "PlaylistBrowser";
    private static boolean isMusicPermissionGrant = true;
    private static final String DIALOG_TAG_DELETE = "Delete";
    private static final String DIALOG_TAG_EDIT = "Edit";
    private static final String DIALOG_TAG_RENAME = "Rename";
    private static final int DELETE_PLAYLIST = CHILD_MENU_BASE + 1;
    private static final int EDIT_PLAYLIST = CHILD_MENU_BASE + 2;
    private static final int RENAME_PLAYLIST = CHILD_MENU_BASE + 3;
    private static final int CHANGE_WEEKS = CHILD_MENU_BASE + 4;
    private static final long RECENTLY_ADDED_PLAYLIST = -1;
    private static final long ALL_SONGS_PLAYLIST = -2;
    private static final long PODCASTS_PLAYLIST = -3;
    private PlaylistListAdapter mAdapter;
    boolean mAdapterSent;
    private static int mLastListPosCourse = -1;
    private static int mLastListPosFine = -1;

    private boolean mCreateShortcut;
    private ServiceToken mToken;

    /// M: add folder as a playlist
    public static final int ADD_FOLDER_AS_PLAYLIST = CHILD_MENU_BASE + 5;
    /// M: mediatek add variable @{
    private boolean mIsMounted = true;
    private boolean mWithtabs = false;
    private int mOrientaiton;
    private Toast mToast = null;
    /// M: Add search view
    private SearchView mSearchView;
    private MenuItem mSearchItem;

    private IMediaPlaybackService mService = null;
    /// @}


    private final DialogInterface.OnClickListener mDeleteDialogListener =
                                       new DialogInterface.OnClickListener() {
        @Override
        public void onClick(DialogInterface arg0, int arg1) {
            MusicLogUtils.v(TAG, "<mDeleteDialogListener onClick>");
            //deleteItems();
            arg0.dismiss();
        }
    };


    public PlaylistBrowserActivity()
    {
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle)
    {
        super.onCreate(icicle);
        MusicLogUtils.v(TAG, "onCreate");
    if (getApplicationContext()
          .checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
          != PackageManager.PERMISSION_GRANTED) {
            MusicLogUtils.v(TAG, "onCreate Permissions not granted");
            isMusicPermissionGrant = false;
            Toast.makeText(this, R.string.music_storage_permission_deny,
                Toast.LENGTH_SHORT).show();
        } else {
            MusicLogUtils.v(TAG, "onCreate Permissions granted");
            isMusicPermissionGrant = true;
        final Intent intent = getIntent();
        final String action = intent.getAction();
        if (Intent.ACTION_CREATE_SHORTCUT.equals(action)) {
            mCreateShortcut = true;
            MusicUtils.resetSdStatus();
        }
        /// M: get the value of mWithtabs(true|false) from intent
        mWithtabs = intent.getBooleanExtra("withtabs", false);

        //Not Required
        //requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        //requestWindowFeature(Window.FEATURE_NO_TITLE);
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        /// M: if enter with tab mode,set media_picker_activity layout
        if (mWithtabs) {
            requestWindowFeature(Window.FEATURE_NO_TITLE);
            requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
            setContentView(R.layout.media_picker_activity);
        } else {
            /// M: else enter with other mode,set media_picker_activity_nowplaying layout @{
            setContentView(R.layout.media_picker_activity_nowplaying);
            mOrientaiton = getResources().getConfiguration().orientation;
            /// @}
        mToken = MusicUtils.bindToService(this, new ServiceConnection() {
            public void onServiceConnected(ComponentName classname, IBinder obj) {
            /// M: instantiate the IMediaPlaybackService object
                mService = IMediaPlaybackService.Stub.asInterface(obj);
                if (Intent.ACTION_VIEW.equals(action)) {
                    Bundle b = intent.getExtras();
                    if (b == null) {
                        MusicLogUtils.v(TAG, "Unexpected:getExtras() returns null.");
                    } else {
                        try {
                            long id = Long.parseLong(b.getString("playlist"));
                            if (id == RECENTLY_ADDED_PLAYLIST) {
                                playRecentlyAdded();
                            } else if (id == PODCASTS_PLAYLIST) {
                                playPodcasts();
                            } else if (id == ALL_SONGS_PLAYLIST) {
                                long[] list = MusicUtils.getAllSongs(PlaylistBrowserActivity.this);
                                if (list != null) {
                                    MusicUtils.playAll(PlaylistBrowserActivity.this, list, 0);
                                    } else  {
                                        /// M: show USB in Using Status Message on Toast.
                                        showUSBInUsingMsg();
                                    }
                            } else {
                                MusicUtils.playPlaylist(PlaylistBrowserActivity.this, id);
                            }
                        } catch (NumberFormatException e) {
                            MusicLogUtils.v(TAG, "Playlist id missing or broken");
                        }
                    }
                    finish();
                    return;
                }
                //Not Required
                //MusicUtils.updateNowPlaying(PlaylistBrowserActivity.this);
            }

            public void onServiceDisconnected(ComponentName classname) {
                /// M: set the mService be null to avoid memory leak.
                mService = null;
                finish();
            }

        });
        }
        IntentFilter f = new IntentFilter();
        f.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        f.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        f.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        /// M: add the ScanListener Action @{
        f.addAction(Intent.ACTION_MEDIA_MOUNTED);
        /// @}
        f.addDataScheme("file");
        registerReceiver(mScanListener, f);

        //Not Required
        //setContentView(R.layout.media_picker_activity);
        //MusicUtils.updateButtonBar(this, R.id.playlisttab);
        ListView lv = getListView();
        lv.setOnCreateContextMenuListener(this);
        lv.setTextFilterEnabled(true);

        mAdapter = (PlaylistListAdapter) getLastNonConfigurationInstance();
        if (mAdapter == null) {
            MusicLogUtils.v(TAG, "starting query");
            mAdapter = new PlaylistListAdapter(
                    getApplication(),
                    this,
                    R.layout.track_list_item,
                    mPlaylistCursor,
                    new String[] { MediaStore.Audio.Playlists.NAME},
                    new int[] { android.R.id.text1 });
            setListAdapter(mAdapter);
            setTitle(R.string.working_playlists);
            getPlaylistCursor(mAdapter.getQueryHandler(), null);
        } else {
            mAdapter.setActivity(this);
            setListAdapter(mAdapter);
            mPlaylistCursor = mAdapter.getCursor();
            /// M: Determine whether the language/country changed @{
            String oldRecentString = mAdapter.retrieveRecentString();
            if (mPlaylistCursor != null && oldRecentString != null &&
                    !oldRecentString.equals(getString(R.string.recentlyadded))) {
                // System language/country has changed...
                MusicLogUtils.v(TAG, "old playlist cursor needs to be changed!");
                mPlaylistCursor.close();
                mPlaylistCursor = null;
            }
            /// @}
            // If mPlaylistCursor is null, this can be because it doesn't have
            // a cursor yet (because the initial query that sets its cursor
            // is still in progress), or because the query failed.
            // In order to not flash the error dialog at the user for the
            // first case, simply retry the query when the cursor is null.
            // Worst case, we end up doing the same query twice.
            if (mPlaylistCursor != null) {
                init(mPlaylistCursor);
            } else {
                setTitle(R.string.working_playlists);
                getPlaylistCursor(mAdapter.getQueryHandler(), null);
            }
        }
        mAdapter.storeRecentString();
    }
  }

    @Override
    public Object onRetainNonConfigurationInstance() {
        PlaylistListAdapter a = mAdapter;
        mAdapterSent = true;
        return a;
    }

    @Override
    public void onDestroy() {
        MusicLogUtils.v(TAG, "onDestroy");

        if (isMusicPermissionGrant == true) {
        ListView lv = getListView();
        if (lv != null) {
            mLastListPosCourse = lv.getFirstVisiblePosition();
            View cv = lv.getChildAt(0);
            if (cv != null) {
                mLastListPosFine = cv.getTop();
            }
        }
        /// M: Determine whether the value of mToken is valid.
        if (mToken != null) {
            MusicUtils.unbindFromService(mToken);
            /// M: set mService be null,avoid memory leak.
            mService = null;
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
        unregisterReceiver(mScanListener);
        }
        super.onDestroy();
    }


     private void showRenameDialog(long id) {
        MusicLogUtils.v(TAG, "showEditDialog>>");
        removeOldFragmentByTag(DIALOG_TAG_RENAME);
        FragmentManager fragmentManager = getFragmentManager();
        MusicLogUtils.v(TAG, "<showRenameDialog> fragmentManager = " + fragmentManager);
        RenamePlaylist newFragment = RenamePlaylist.newInstance(true);
        //((WeekSelector) newFragment).setOnClickListener(mDeleteDialogListener);
        newFragment.show(fragmentManager, DIALOG_TAG_RENAME);
        fragmentManager.executePendingTransactions();
    }


   private void showEditDialog() {
        MusicLogUtils.v(TAG, "showEditDialog>>");
        removeOldFragmentByTag(DIALOG_TAG_EDIT);
        FragmentManager fragmentManager = getFragmentManager();
        MusicLogUtils.v(TAG, "<showEditDialog> fragmentManager = " + fragmentManager);
        WeekSelector newFragment = WeekSelector.newInstance(true);
        //((WeekSelector) newFragment).setOnClickListener(mDeleteDialogListener);
        newFragment.show(fragmentManager, DIALOG_TAG_EDIT);
        fragmentManager.executePendingTransactions();
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
    }
    @Override
    public void onResume() {
        super.onResume();
        /// M: registerReceiver of mTrackListListener,valid action are META_CHANGED
         // and QUEUE_CHANGED @{
        if (isMusicPermissionGrant == true) {
        MusicLogUtils.v(TAG, "onResume>>");
        IntentFilter f = new IntentFilter();
        f.addAction(MediaPlaybackService.META_CHANGED);
        f.addAction(MediaPlaybackService.QUEUE_CHANGED);
        registerReceiver(mTrackListListener, f);
        mTrackListListener.onReceive(null, null);
        /// @}

        MusicUtils.setSpinnerState(this);
        MusicLogUtils.v(TAG, "onResume>>>");
        } else {
            finish();
        //android.os.Process.killProcess(android.os.Process.myPid());
        return;
        }
    }
    @Override
    public void onPause() {
        MusicLogUtils.v(TAG, "onPause");
        /// M: unregister mTrackListListener,then in other condition,
        // we needn't update nowPlaying.

        if (isMusicPermissionGrant == true) {
        unregisterReceiver(mTrackListListener);
        mReScanHandler.removeCallbacksAndMessages(null);
        }
        super.onPause();
    }
//Not Required

    private Handler mReScanHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (mAdapter != null) {
                getPlaylistCursor(mAdapter.getQueryHandler(), null);
            }
        }
    };
    public void init(Cursor cursor) {

        if (mAdapter == null) {
            return;
        }
        mAdapter.changeCursor(cursor);

        if (mPlaylistCursor == null) {
            MusicUtils.displayDatabaseError(this, mIsMounted);
            closeContextMenu();
            mReScanHandler.sendEmptyMessageDelayed(0, 1000);
            return;
        }

        // restore previous position
        if (mLastListPosCourse >= 0) {
            getListView().setSelectionFromTop(mLastListPosCourse, mLastListPosFine);
            mLastListPosCourse = -1;
        }
        MusicUtils.hideDatabaseError(this);
        //Not Required
        //MusicUtils.updateButtonBar(this, R.id.playlisttab);
        setTitle();
    }

    private void setTitle() {
        setTitle(R.string.playlists_title);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        /// M: mWithtabs marks enter the Activity with the tab switch.
        if (mWithtabs) {
            return super.onCreateOptionsMenu(menu);
        }
        if (!mCreateShortcut) {
            menu.add(0, PARTY_SHUFFLE, 0, R.string.party_shuffle);
            // icon will be set in onPrepareOptionsMenu()
            /// M: Add search view  @{
            getMenuInflater().inflate(R.menu.music_search_menu, menu);
            mSearchItem = menu.findItem(R.id.search);
            mSearchView = (SearchView) mSearchItem.getActionView();

            mSearchView.setOnQueryTextListener(mQueryTextListener);
            mSearchView.setQueryHint(getString(R.string.search_hint));
            mSearchView.setIconifiedByDefault(true);
            SearchManager searchManager = (SearchManager) getSystemService(Context.SEARCH_SERVICE);

            if (searchManager != null) {
                SearchableInfo info = searchManager.getSearchableInfo(this.getComponentName());
                mSearchView.setSearchableInfo(info);
            }
            /// @}
        }
        /// M: add Audio effets menu.
        menu.add(0, EFFECTS_PANEL, 0, R.string.effects_list_title).setIcon(R.drawable.ic_menu_eq);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (isMusicPermissionGrant == true) {
        /// M: mWithtabs marks enter the Activity with the tab switch.
        if (mWithtabs) {
            return super.onPrepareOptionsMenu(menu);
        }
        /// M: Playlist cursor is null, need not show option menu @{
        if (mPlaylistCursor == null) {
            mPlaylistCursor = mAdapter.getCursor();
            MusicLogUtils.v(TAG, "Playlist cursor is null, need not show option menu!");
            //return false;
        }
        /// M: when the AudioEffect is exist , the effects_panel menu could Visible,
        // otherwise ,set invisible. @{
        MusicUtils.setEffectPanelMenu(getApplicationContext(), menu);
        /// @}

        MusicUtils.setPartyShuffleMenuIcon(menu);
        //if (!Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {


        return true;
        }
        return true;
        /// @}
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        /// M: mWithtabs marks enter the Activity with the tab switch.
        if (mWithtabs) {
            return super.onOptionsItemSelected(item);
        }
        Intent intent;
        switch (item.getItemId()) {
            case PARTY_SHUFFLE:
                MusicUtils.togglePartyShuffle();
                break;
                /// M: when select the item of effects_panel, jump to AudioEffect Activity @{
            case EFFECTS_PANEL:
                return MusicUtils.startEffectPanel(this);
                /// @}
                /// M: when selected the search Item,start to search @{
            case R.id.search:
                if (!mCreateShortcut) {
                    onSearchRequested();
                }
                return true;
                /// @}
        }
        return super.onOptionsItemSelected(item);
    }

    public void onCreateContextMenu(ContextMenu menu, View view, ContextMenuInfo menuInfoIn) {

        if (isMusicPermissionGrant == true) {
        if (mCreateShortcut) {
            return;
        }

        AdapterContextMenuInfo mi = (AdapterContextMenuInfo) menuInfoIn;

        menu.add(0, PLAY_SELECTION, 0, R.string.play_selection);

        if (mi.id >= 0 /*|| mi.id == PODCASTS_PLAYLIST*/) {
            menu.add(0, DELETE_PLAYLIST, 0, R.string.delete_playlist_menu);
        }

        if (mi.id == RECENTLY_ADDED_PLAYLIST) {
            menu.add(0, EDIT_PLAYLIST, 0, R.string.edit_playlist_menu);
        }

        if (mi.id >= 0) {
            menu.add(0, RENAME_PLAYLIST, 0, R.string.rename_playlist_menu);
        }

        mPlaylistCursor.moveToPosition(mi.position);
        menu.setHeaderTitle(mPlaylistCursor.getString(mPlaylistCursor.getColumnIndexOrThrow(
                MediaStore.Audio.Playlists.NAME)));
    }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        AdapterContextMenuInfo mi = (AdapterContextMenuInfo) item.getMenuInfo();
        switch (item.getItemId()) {
            case PLAY_SELECTION:
                if (mi.id == RECENTLY_ADDED_PLAYLIST) {
                    playRecentlyAdded();
                } else if (mi.id == PODCASTS_PLAYLIST) {
                    playPodcasts();
                } else {
                    MusicUtils.playPlaylist(this, mi.id);
                }
                break;
            case DELETE_PLAYLIST:
                Uri uri = ContentUris.withAppendedId(
                        MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI, mi.id);
                getContentResolver().delete(uri, null, null);
                Toast.makeText(this, R.string.playlist_deleted_message, Toast.LENGTH_SHORT).show();
                if (mPlaylistCursor.getCount() == 0) {
                    setTitle(R.string.no_playlists_title);
                }
                break;
            case EDIT_PLAYLIST:
                if (mi.id == RECENTLY_ADDED_PLAYLIST) {

                    showEditDialog();
                } else {
                    MusicLogUtils.v(TAG, "should not be here");
                }
                break;
            case RENAME_PLAYLIST:
            /*
                Intent intent = new Intent();
                intent.setClass(this, RenamePlaylist.class);
                intent.putExtra("rename", mi.id);
                startActivityForResult(intent, RENAME_PLAYLIST);
                break;
                */
                MusicUtils.setLongPref(getApplicationContext(), "rename", mi.id);
                showRenameDialog(mi.id);

        }
        return true;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent intent) {
        switch (requestCode) {
            case SCAN_DONE:
                if (resultCode == RESULT_CANCELED) {
                    finish();
                } else if (mAdapter != null) {
                    getPlaylistCursor(mAdapter.getQueryHandler(), null);
                }
                break;

            default:
                Bundle extra = new Bundle();
                /// M: handling the result from filemanager when plugin need.{@
              /*  IMusicTrackBrowser mMusicPlugin =
                                         Extensions.getPluginObject(getApplicationContext());
                mMusicPlugin.onActivityResult(requestCode, resultCode, intent,
                        getApplicationContext(), mMusicListenerForPlugin,
                        ACTIVITY_NAME, this, extra);*/
                /// @}
                break;
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id)
    {
        if (mCreateShortcut) {
            final Intent shortcut = new Intent();
            shortcut.setAction(Intent.ACTION_VIEW);
            shortcut.setDataAndType(Uri.EMPTY, "vnd.android.cursor.dir/playlist");
            shortcut.putExtra("playlist", String.valueOf(id));
            shortcut.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);

            final Intent intent = new Intent();
            intent.putExtra(Intent.EXTRA_SHORTCUT_INTENT, shortcut);
            intent.putExtra(Intent.EXTRA_SHORTCUT_NAME, (
                (TextView) v.findViewById(R.id.line1)).getText());
            intent.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE,
                Intent.ShortcutIconResource.fromContext(
                    this, R.drawable.ic_launcher_shortcut_music_playlist));

            setResult(RESULT_OK, intent);
            finish();
            return;
        }
        if (id == RECENTLY_ADDED_PLAYLIST) {
            Intent intent = new Intent(Intent.ACTION_PICK);
            intent.setDataAndType(Uri.EMPTY, "vnd.android.cursor.dir/track");
            intent.putExtra("playlist", "recentlyadded");
            startActivity(intent);
        } else if (id == PODCASTS_PLAYLIST) {
            Intent intent = new Intent(Intent.ACTION_PICK);
            intent.setDataAndType(Uri.EMPTY, "vnd.android.cursor.dir/track");
            intent.putExtra("playlist", "podcasts");
            startActivity(intent);
        } else {
            Intent intent = new Intent(Intent.ACTION_EDIT);
            intent.setDataAndType(Uri.EMPTY, "vnd.android.cursor.dir/track");
            intent.putExtra("playlist", Long.valueOf(id).toString());
            startActivity(intent);
        }
    }

    private void playRecentlyAdded() {
        // do a query for all songs added in the last X weeks.
        int X = MusicUtils.getIntPref(this, "numweeks", 2) * (3600 * 24 * 7);
        final String[] ccols = new String[] { MediaStore.Audio.Media._ID};
        String where = MediaStore.MediaColumns.DATE_ADDED + ">" +
            (System.currentTimeMillis() / 1000 - X);
        Cursor cursor = MusicUtils.query(this, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, where, null,
                /// M: add for chinese sorting
                MediaStore.Audio.Media.DEFAULT_SORT_ORDER);

        if (cursor == null) {
            // Todo: show a message
            /// M: show USB in Using Status Message on Toast.
            showUSBInUsingMsg();
            return;
        }
        try {
            int len = cursor.getCount();
            long [] list = new long[len];
            for (int i = 0; i < len; i++) {
                cursor.moveToNext();
                list[i] = cursor.getLong(0);
            }
            MusicUtils.playAll(this, list, 0);
        } catch (SQLiteException ex) {
        } finally {
            cursor.close();
        }
    }

    private void playPodcasts() {
        // do a query for all files that are podcasts
        final String[] ccols = new String[] { MediaStore.Audio.Media._ID};
        Cursor cursor = MusicUtils.query(this, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, MediaStore.Audio.Media.IS_PODCAST + "=1",
                null,
                /// M: add for chinese sorting
                MediaStore.Audio.Media.DEFAULT_SORT_ORDER);

        if (cursor == null) {
            // Todo: show a message
            /// M: show USB in Using Status Message on Toast.
            showUSBInUsingMsg();
            return;
        }
        try {
            int len = cursor.getCount();
            long [] list = new long[len];
            for (int i = 0; i < len; i++) {
                cursor.moveToNext();
                list[i] = cursor.getLong(0);
            }
            MusicUtils.playAll(this, list, 0);
        } catch (SQLiteException ex) {
        } finally {
            cursor.close();
        }
    }


    String[] mCols = new String[] {
            MediaStore.Audio.Playlists._ID,
            MediaStore.Audio.Playlists.NAME
    };

    private Cursor getPlaylistCursor(AsyncQueryHandler async, String filterstring) {

        StringBuilder where = new StringBuilder();
        where.append(MediaStore.Audio.Playlists.NAME + " != ''");

        // Add in the filtering constraints
        String [] keywords = null;
        /// M: add for chinese sorting
        String sortOrder = MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER;
        if (filterstring != null) {
            String [] searchWords = filterstring.split(" ");
            keywords = new String[searchWords.length];
            Collator col = Collator.getInstance();
            col.setStrength(Collator.PRIMARY);
            for (int i = 0; i < searchWords.length; i++) {
                keywords[i] = '%' + searchWords[i] + '%';
            }
            for (int i = 0; i < searchWords.length; i++) {
                where.append(" AND ");
                where.append(MediaStore.Audio.Playlists.NAME + " LIKE ?");
            }
        }

        String whereclause = where.toString();


        if (async != null) {
            async.startQuery(0, null, MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                    mCols, whereclause, keywords, sortOrder);
            return null;
        }
        Cursor c = null;
        c = MusicUtils.query(this, MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                mCols, whereclause, keywords, sortOrder);
        return mergedCursor(c);
    }

    private Cursor mergedCursor(Cursor c) {
        if (c == null) {
            return null;
        }
        if (c instanceof MergeCursor) {
            // this shouldn't happen, but fail gracefully
            MusicLogUtils.v(TAG, "Already wrapped");
            return c;
        }
        MatrixCursor autoplaylistscursor = new MatrixCursor(mCols);
        if (mCreateShortcut) {
            ArrayList<Object> all = new ArrayList<Object>(2);
            all.add(ALL_SONGS_PLAYLIST);
            all.add(getString(R.string.play_all));
            autoplaylistscursor.addRow(all);
        }
        ArrayList<Object> recent = new ArrayList<Object>(2);
        recent.add(RECENTLY_ADDED_PLAYLIST);
        recent.add(getString(R.string.recentlyadded));
        autoplaylistscursor.addRow(recent);

        // check if there are any podcasts
        Cursor counter = MusicUtils.query(this, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                new String[] {"count(*)"}, "is_podcast=1", null, null);
        if (counter != null) {
            counter.moveToFirst();
            int numpodcasts = counter.getInt(0);
            counter.close();
            if (numpodcasts > 0) {
                ArrayList<Object> podcasts = new ArrayList<Object>(2);
                podcasts.add(PODCASTS_PLAYLIST);
                podcasts.add(getString(R.string.podcasts_listitem));
                autoplaylistscursor.addRow(podcasts);
            }
        }

        Cursor cc = new MergeCursor(new Cursor [] {autoplaylistscursor, c});
        return cc;
    }

    static class PlaylistListAdapter extends SimpleCursorAdapter {
        int mTitleIdx;
        int mIdIdx;
        private PlaylistBrowserActivity mActivity = null;
        private AsyncQueryHandler mQueryHandler;
        private String mConstraint = null;
        private boolean mConstraintIsValid = false;
        /// M: store current recently added string.
        private String mRecentString = null;

        class QueryHandler extends AsyncQueryHandler {
            QueryHandler(ContentResolver res) {
                super(res);
            }

            @Override
            protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
                /// M: just make mediatek LOG  @{
                boolean shouldFinish = false;
                if (null != cursor) {
                    MusicLogUtils.v(TAG, "query complete: " + cursor.getCount());
                } else {
                    MusicLogUtils.v(TAG, "query complete: cursor is null");
                }
                /// @}
                if (cursor != null) {
                    try {
                    cursor = mActivity.mergedCursor(cursor);
                    /// M:  @{
                    } catch (SQLiteException ex) {
                        MusicLogUtils.v(TAG, "---------Exception: " + ex.toString());
                        shouldFinish = true;
                }
                    /// @}
                }
                /// M: Determine whether should finish current activity @{
                if (shouldFinish) {
                    mActivity.finish();
                } else {
                /// @}
                mActivity.init(cursor);
            }
        }
    }

        PlaylistListAdapter(Context context, PlaylistBrowserActivity currentactivity,
                int layout, Cursor cursor, String[] from, int[] to) {
            super(context, layout, cursor, from, to);
            mActivity = currentactivity;
            getColumnIndices(cursor);
            mQueryHandler = new QueryHandler(context.getContentResolver());
        }
        private void getColumnIndices(Cursor cursor) {
            if (cursor != null) {
                mTitleIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Playlists.NAME);
                mIdIdx = cursor.getColumnIndexOrThrow(MediaStore.Audio.Playlists._ID);
            }
        }

        public void setActivity(PlaylistBrowserActivity newactivity) {
            mActivity = newactivity;
        }

        public AsyncQueryHandler getQueryHandler() {
            return mQueryHandler;
        }

        /*
         * M: store the current recentlyadded string.
         */
        public void storeRecentString() {
            if (mActivity != null) {
                mRecentString = mActivity.getString(R.string.recentlyadded);
            }
        }

        /*
         * M: get the value of recentlyadded String.
         */
        public String retrieveRecentString() {
            return mRecentString;
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {

            TextView tv = (TextView) view.findViewById(R.id.line1);

            String name = cursor.getString(mTitleIdx);
            tv.setText(name);

            long id = cursor.getLong(mIdIdx);

            ImageView iv = (ImageView) view.findViewById(R.id.icon);
            if (id == RECENTLY_ADDED_PLAYLIST) {
                iv.setImageResource(R.drawable.ic_mp_playlist_recently_added_list);
            } else {
                iv.setImageResource(R.drawable.ic_mp_playlist_list);
            }
            ViewGroup.LayoutParams p = iv.getLayoutParams();
            p.width = ViewGroup.LayoutParams.WRAP_CONTENT;
            p.height = ViewGroup.LayoutParams.WRAP_CONTENT;

            iv = (ImageView) view.findViewById(R.id.play_indicator);
            iv.setVisibility(View.GONE);

            view.findViewById(R.id.line2).setVisibility(View.GONE);
        }

        @Override
        public void changeCursor(Cursor cursor) {
            if (mActivity.isFinishing() && cursor != null) {
                cursor.close();
                cursor = null;
            }
            if (cursor != mActivity.mPlaylistCursor) {
                mActivity.mPlaylistCursor = cursor;
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
            Cursor c = mActivity.getPlaylistCursor(null, s);
            mConstraint = s;
            mConstraintIsValid = true;
            return c;
        }
    }

    private Cursor mPlaylistCursor;

    /*
     * M: listen current play files,and update nowplaying.
     */
    private BroadcastReceiver mTrackListListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            getListView().invalidateViews();
            if (mService != null) {
                MusicUtils.updateNowPlaying(PlaylistBrowserActivity.this, mOrientaiton);
            }
        }
    };
    /*
     * M: This listener gets called when the media scanner starts up or finishes, and
     * when the sd card is unmounted.
     */
    private BroadcastReceiver mScanListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            /// M: get the action and current Storage State @{
            String action = intent.getAction();
            String status = Environment.getExternalStorageState();
            /// @}
            MusicLogUtils.v(TAG, "mScanListener.onReceive:" + action + ", status = " + status);
            /// M: Determine whether the action that get from intent match with @{
            // ACTION_MEDIA_SCANNER_STARTED or ACTION_MEDIA_SCANNER_FINISHED
            if (Intent.ACTION_MEDIA_SCANNER_STARTED.equals(action) ||
                    Intent.ACTION_MEDIA_SCANNER_FINISHED.equals(action)) {
                MusicUtils.setSpinnerState(PlaylistBrowserActivity.this);
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
            } else if (Intent.ACTION_MEDIA_UNMOUNTED.equals(action)) {
                mIsMounted = false;
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
                closeContextMenu();
                closeOptionsMenu();
                mReScanHandler.sendEmptyMessageDelayed(0, 1000);
            } else if (Intent.ACTION_MEDIA_MOUNTED.equals(action)) {
              /// M: if SD Card is Mounted,send Message to rescan database @{
                mIsMounted = true;
                mReScanHandler.removeCallbacksAndMessages(null);
                mReScanHandler.sendEmptyMessage(0);
            }
             /// @}
        }
    };

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

    /*
     * M: when edit Text ,do query follow the message of the query.
     */
    SearchView.OnQueryTextListener mQueryTextListener = new SearchView.OnQueryTextListener() {
        public boolean onQueryTextSubmit(String query) {
            Intent intent = new Intent();
            intent.setClass(PlaylistBrowserActivity.this, QueryBrowserActivity.class);
            intent.putExtra(SearchManager.QUERY, query);
            startActivity(intent);
            return true;
        }
        /// M: false if the SearchView should perform the default action of showing
        //any suggestions if available.
        public boolean onQueryTextChange(String newText) {
            return false;
        }
    };

    /*
     * in this Activity ,Configuration changed include (orientation|screenSize).
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mOrientaiton = newConfig.orientation;
        // If showing database error UI, need not to refresh UI.
        if ((findViewById(R.id.sd_icon).getVisibility()) == View.VISIBLE) {
            MusicLogUtils.v(TAG, "Configuration Changed at database error, return!");
            return;
        }
        if (mService != null) {
            MusicUtils.updateNowPlaying(this, mOrientaiton);
        }
    }

    /*
     * M: if cursor or list is null in current moment,show the USB in using Toast
     * make User know current status.
     */
    private void showUSBInUsingMsg() {
        if (null == mToast) {
            final Context context = getApplicationContext();
            final int duration = Toast.LENGTH_SHORT;
            mToast = Toast.makeText(context, getString(R.string.usb_in_using), duration);
        }
        mToast.show();
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

