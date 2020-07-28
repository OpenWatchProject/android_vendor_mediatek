/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

/*
 * Copyright (C) 2008 The Android Open Source Project
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
import java.io.File;
import java.io.FileDescriptor;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.Formatter;
import java.util.HashMap;
import java.util.Locale;

import android.app.Activity;
import android.app.SearchManager;
import android.app.SearchableInfo;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.media.audiofx.AudioEffect;
import android.net.Uri;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.os.storage.StorageManager;
import android.provider.MediaStore;
import android.provider.Settings;
import android.telephony.TelephonyManager;
//Not Required
//import android.text.TextUtils;
import android.text.format.Time;

//Not Required
//import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.TextView;
import android.widget.Toast;


public class MusicUtils {

    private static final String TAG = "MusicUtils";

    /// M: add for SDCard @{
    public static final String SDCARD_STATUS_UPDATE = "com.android.music.sdcardstatusupdate";
    public static final String SDCARD_STATUS_MESSAGE = "message";
    public static final String SDCARD_STATUS_ONOFF = "onoff";
    /// @}
    /// M: Add for delete tracks @{
    public static final String DELETE_DESC_STRING_ID = "delete_desc_string_id";
    public static final String DELETE_DESC_TRACK_INFO = "delete_desc_track_info";
    /// @}
    /// M: Add to restore the selected to add to new playlist item info,
    /// like album id, artist id and audio id. @{
    /** M: Restore selected to add to new playlist item info,
    * like album id, artist id and audio id */
    public static final String ADD_TO_PLAYLIST_ITEM_ID = "add_to_playlist_item_id";
    /** M: Restore the start activity tab index,
    * so when onActivityResult can return to right activity */
    public static final String START_ACTIVITY_TAB_ID = "start_activity_tab_id";
    /// @}
    /// M: Add to indicate the save_as_playlist and new_playlist @{
    public static final String SAVE_PLAYLIST_FLAG = "SAVE_PLAYLIST_FLAG";
    public static final String SAVE_AS_PLAYLIST = "save_as_playlist";
    public static final String NEW_PLAYLIST = "new_playlist";
    /// @}

    /// M: keep playlist name
    public static final String PLAYLIST_NAME = "playlist_name";

    public final static int NOT_DELETE = -1;

    public interface Defs {
        public final static int OPEN_URL = 0;
        public final static int ADD_TO_PLAYLIST = 1;
        public final static int USE_AS_RINGTONE = 2;
        public final static int PLAYLIST_SELECTED = 3;
        public final static int NEW_PLAYLIST = 4;
        public final static int PLAY_SELECTION = 5;
        public final static int GOTO_START = 6;
        public final static int GOTO_PLAYBACK = 7;
        public final static int PARTY_SHUFFLE = 8;
        public final static int SHUFFLE_ALL = 9;
        public final static int DELETE_ITEM = 10;
        public final static int SCAN_DONE = 11;
        public final static int QUEUE = 12;
        public final static int EFFECTS_PANEL = 13;
        /// M: add for send fm transmitter
        public final static int FM_TRANSMITTER = 14;
        /// M: add for drm
        public final static int DRM_INFO = 15;
        public final static int CHILD_MENU_BASE = 16; // this should be the last item
        /**M: Add Hotknot menu.@{**/
        public final static int HOTKNOT = CHILD_MENU_BASE + 10;
        /**@}**/
    }

    public static String makeAlbumsLabel(Context context, int numalbums,
     int numsongs, boolean isUnknown) {
        // There are two formats for the albums/songs information:
        // "N Song(s)"  - used for unknown artist/album
        // "N Album(s)" - used for known albums

        StringBuilder songs_albums = new StringBuilder();

        Resources r = context.getResources();
        if (isUnknown) {
            if (numsongs == 1) {
                songs_albums.append(context.getString(R.string.onesong));
            } else {
                String f = r.getQuantityText(R.plurals.Nsongs, numsongs).toString();
                sFormatBuilder.setLength(0);
                /// M: use local format
                sFormatter.format(Locale.getDefault(), f, Integer.valueOf(numsongs));
                songs_albums.append(sFormatBuilder);
            }
        } else {
            String f = r.getQuantityText(R.plurals.Nalbums, numalbums).toString();
            sFormatBuilder.setLength(0);
            /// M: use local format
            sFormatter.format(Locale.getDefault(), f, Integer.valueOf(numalbums));
            songs_albums.append(sFormatBuilder);
            songs_albums.append(context.getString(R.string.albumsongseparator));
        }
        return songs_albums.toString();
    }

    /**
     * This is now only used for the query screen
     */
    public static String makeAlbumsSongsLabel(Context context, int numalbums,
     int numsongs, boolean isUnknown) {
        // There are several formats for the albums/songs information:
        // "1 Song"   - used if there is only 1 song
        // "N Songs" - used for the "unknown artist" item
        // "1 Album"/"N Songs"
        // "N Album"/"M Songs"
        // Depending on locale, these may need to be further subdivided

        StringBuilder songs_albums = new StringBuilder();

        if (numsongs == 1) {
            songs_albums.append(context.getString(R.string.onesong));
        } else {
            Resources r = context.getResources();
            if (! isUnknown) {
                String f = r.getQuantityText(R.plurals.Nalbums, numalbums).toString();
                sFormatBuilder.setLength(0);
                /// M: use local format
                sFormatter.format(Locale.getDefault(), f, Integer.valueOf(numalbums));
                songs_albums.append(sFormatBuilder);
                songs_albums.append(context.getString(R.string.albumsongseparator));
            }
            String f = r.getQuantityText(R.plurals.Nsongs, numsongs).toString();
            sFormatBuilder.setLength(0);
            /// M: use local format
            sFormatter.format(Locale.getDefault(), f, Integer.valueOf(numsongs));
            songs_albums.append(sFormatBuilder);
        }
        return songs_albums.toString();
    }

    public static IMediaPlaybackService sService = null;
    private static HashMap<Context, ServiceBinder> sConnectionMap =
     new HashMap<Context, ServiceBinder>();

    public static class ServiceToken {
        ContextWrapper mWrappedContext;
        ServiceToken(ContextWrapper context) {
            mWrappedContext = context;
        }
    }

    public static ServiceToken bindToService(Activity context) {
        return bindToService(context, null);
    }

    public static ServiceToken bindToService(Activity context, ServiceConnection callback) {
        Activity realActivity = context.getParent();
        if (realActivity == null) {
            realActivity = context;
        }
        MusicLogUtils.v(TAG, "bindToService: activity=" + context.toString());
        ContextWrapper cw = new ContextWrapper(realActivity);
        cw.startService(new Intent(cw, MediaPlaybackService.class));
        ServiceBinder sb = new ServiceBinder(callback);
        if (cw.bindService((new Intent()).setClass(cw, MediaPlaybackService.class), sb, 0)) {
            sConnectionMap.put(cw, sb);
            return new ServiceToken(cw);
        }
        MusicLogUtils.v(TAG, "Failed to bind to service");
        return null;
    }

    public static void unbindFromService(ServiceToken token) {
        /// M: set mLastSdStatus is null when unbind service
        sLastSdStatus = null;
        MusicLogUtils.v(TAG, "Reset mLastSdStatus to be null");
        if (token == null) {
            MusicLogUtils.v(TAG, "Trying to unbind with null token");
            return;
        }
        ContextWrapper cw = token.mWrappedContext;
        ServiceBinder sb = sConnectionMap.remove(cw);
        if (sb == null) {
            MusicLogUtils.v(TAG, "Trying to unbind for unknown Context");
            return;
        }
        cw.unbindService(sb);
        if (sConnectionMap.isEmpty()) {
            // presumably there is nobody interested in the service at this point,
            // so don't hang on to the ServiceConnection
            sService = null;
        }
    }

    private static class ServiceBinder implements ServiceConnection {
        ServiceConnection mCallback;
        ServiceBinder(ServiceConnection callback) {
            mCallback = callback;
        }

        public void onServiceConnected(ComponentName className, android.os.IBinder service) {
            sService = IMediaPlaybackService.Stub.asInterface(service);
            initAlbumArtCache();
            if (mCallback != null) {
                mCallback.onServiceConnected(className, service);
            }
        }

        public void onServiceDisconnected(ComponentName className) {
            if (mCallback != null) {
                mCallback.onServiceDisconnected(className);
            }
            sService = null;
        }
    }

    public static long getCurrentAlbumId() {
        if (sService != null) {
            try {
                return sService.getAlbumId();
            } catch (RemoteException ex) {
            }
        }
        return -1;
    }

    public static long getCurrentArtistId() {
        if (MusicUtils.sService != null) {
            try {
                return sService.getArtistId();
            } catch (RemoteException ex) {
            }
        }
        return -1;
    }

    public static long getCurrentAudioId() {
        if (MusicUtils.sService != null) {
            try {
                return sService.getAudioId();
            } catch (RemoteException ex) {
            }
        }
        return -1;
    }

    public static int getCurrentShuffleMode() {
        int mode = MediaPlaybackService.SHUFFLE_NONE;
        if (sService != null) {
            try {
                mode = sService.getShuffleMode();
            } catch (RemoteException ex) {
            }
        }
        return mode;
    }

    public static void togglePartyShuffle() {
        if (sService != null) {
            int shuffle = getCurrentShuffleMode();
            try {
                if (shuffle == MediaPlaybackService.SHUFFLE_AUTO) {
                    sService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);
                } else {
                    sService.setShuffleMode(MediaPlaybackService.SHUFFLE_AUTO);
                }
            } catch (RemoteException ex) {
            }
        }
    }

    public static void setPartyShuffleMenuIcon(Menu menu) {
        MenuItem item = menu.findItem(Defs.PARTY_SHUFFLE);
        if (item != null) {
            int shuffle = MusicUtils.getCurrentShuffleMode();
            if (shuffle == MediaPlaybackService.SHUFFLE_AUTO) {
                item.setIcon(R.drawable.ic_menu_party_shuffle);
                item.setTitle(R.string.party_shuffle_off);
            } else {
                item.setIcon(R.drawable.ic_menu_party_shuffle);
                item.setTitle(R.string.party_shuffle);
            }
        }
    }

    /*
     * Returns true if a file is currently opened for playback (regardless
     * of whether it's playing or paused).
     */
    public static boolean isMusicLoaded() {
        if (MusicUtils.sService != null) {
            try {
                return sService.getPath() != null;
            } catch (RemoteException ex) {
            }
        }
        return false;
    }

    private final static long [] sEmptyList = new long[0];

    public static long [] getSongListForCursor(Cursor cursor) {
        if (cursor == null) {
            return sEmptyList;
        }
        int len = cursor.getCount();
        long [] list = new long[len];
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

    public static long [] getSongListForArtist(Context context, long id) {
        final String[] ccols = new String[] { MediaStore.Audio.Media._ID };
        /// M: use selectionArgs replace set query value in where @{
        String where = MediaStore.Audio.Media.ARTIST_ID + "=?  AND " +
        MediaStore.Audio.Media.IS_MUSIC + "=1";
        String[] whereArgs = new String[]{String.valueOf(id)};
        Cursor cursor = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, where, whereArgs,
                MediaStore.Audio.Media.ALBUM_KEY + ","  + MediaStore.Audio.Media.TRACK);
        /// @}
        if (cursor != null) {
            long [] list = getSongListForCursor(cursor);
            cursor.close();
            return list;
        }
        return sEmptyList;
    }

    public static long [] getSongListForAlbum(Context context, long id) {
        final String[] ccols = new String[] { MediaStore.Audio.Media._ID };
        /// M: use selectionArgs replace set query value in where @{
        String where = MediaStore.Audio.Media.ALBUM_ID + "=? AND " +
                MediaStore.Audio.Media.IS_MUSIC + "=1";
        String[] whereArgs = new String[]{String.valueOf(id)};
        Cursor cursor = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                ccols, where, whereArgs, MediaStore.Audio.Media.TRACK);
        /// @}
        if (cursor != null) {
            long [] list = getSongListForCursor(cursor);
            cursor.close();
            return list;
        }
        return sEmptyList;
    }

    public static long [] getSongListForPlaylist(Context context, long plid) {
        final String[] ccols = new String[] { MediaStore.Audio.Playlists.Members.AUDIO_ID };
        Cursor cursor = query(context, MediaStore.Audio.Playlists.Members.
        getContentUri("external", plid),
                ccols, null, null, MediaStore.Audio.Playlists.Members.DEFAULT_SORT_ORDER);

        if (cursor != null) {
            long [] list = getSongListForCursor(cursor);
            cursor.close();
            return list;
        }
        return sEmptyList;
    }

    public static void playPlaylist(Context context, long plid) {
        long [] list = getSongListForPlaylist(context, plid);
        if (list != null) {
            playAll(context, list, -1, false);
        }
    }

    public static long [] getAllSongs(Context context) {
        Cursor c = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                new String[] {MediaStore.Audio.Media._ID}, MediaStore.Audio.Media.IS_MUSIC + "=1",
                null, MediaStore.Audio.Media.DEFAULT_SORT_ORDER);
        try {
            /// M: if count is 0,return array of length 0
            if (c == null) {
                return null;
            }
            int len = c.getCount();
            long [] list = new long[len];
            for (int i = 0; i < len; i++) {
                c.moveToNext();
                list[i] = c.getLong(0);
            }

            return list;
        } finally {
            if (c != null) {
                c.close();
            }
        }
    }

    /**
     * Fills out the given submenu with items for "new playlist" and
     * any existing playlists. When the user selects an item, the
     * application will receive PLAYLIST_SELECTED with the Uri of
     * the selected playlist, NEW_PLAYLIST if a new playlist
     * should be created, and QUEUE if the "current playlist" was
     * selected.
     * @param context The context to use for creating the menu items
     * @param sub The submenu to add the items to.
     */
    public static void makePlaylistMenu(Context context, SubMenu sub) {
        String[] cols = new String[] {
                MediaStore.Audio.Playlists._ID,
                MediaStore.Audio.Playlists.NAME
        };
        ContentResolver resolver = context.getContentResolver();
        if (resolver == null) {
            System.out.println("resolver = null");
        } else {
            String whereclause = MediaStore.Audio.Playlists.NAME + " != ''";
            /// M: add for chinese sorting
            Cursor cur = resolver.query(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                cols, whereclause, null,
                MediaStore.Audio.Playlists.DEFAULT_SORT_ORDER);
            sub.clear();
            sub.add(1, Defs.QUEUE, 0, R.string.queue);
            sub.add(1, Defs.NEW_PLAYLIST, 0, R.string.new_playlist);
            if (cur != null && cur.moveToFirst()) {
                //sub.addSeparator(1, 0);
                while (! cur.isAfterLast()) {
                    Intent intent = new Intent();
                    intent.putExtra("playlist", cur.getLong(0));
//                    if (cur.getInt(0) == mLastPlaylistSelected) {
//                        sub.add(0, MusicBaseActivity.PLAYLIST_SELECTED,
//                          cur.getString(1)).setIntent(intent);
//                    } else {
                        sub.add(1, Defs.PLAYLIST_SELECTED, 0, cur.getString(1)).setIntent(intent);
//                    }
                    cur.moveToNext();
                }
            }
            if (cur != null) {
                cur.close();
            }
        }
    }

    public static int clearPlaylist(Context context, int plid) {
        int deleteItem = 0;
        try {
            Uri uri = MediaStore.Audio.Playlists.Members.getContentUri("external", plid);
            deleteItem = context.getContentResolver().delete(uri, null, null);
        } catch (UnsupportedOperationException ex) {
            MusicLogUtils.v(TAG, "clearPlaylist() with UnsupportedOperationException:" + ex);
            deleteItem = -1;
        } finally {
            return deleteItem;
        }
    }

    public static void removeTracks(Context context, long [] list) {
        if (sService == null) {
            MusicLogUtils.v(TAG, "removeTracks(),sService is null");
            return;
        }
        if (!hasMountedSDcard(context)) {
            return;
        }
        try {
            for (long id : list) {
                sService.removeTrack(id);
            }
        } catch (RemoteException rx) {
            MusicLogUtils.v(TAG, "removeTracks with RemoteException", rx);
        }
    }

    public static int deleteTracks(Context context, long [] list) {
        MusicLogUtils.v(TAG, ">> deleteTracks");
        int deleteTrackNum = 0;
        if (context
            .checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
             != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(context, R.string.music_storage_permission_deny
                                           , Toast.LENGTH_SHORT).show();
            return NOT_DELETE ;
        }
        if (sService == null) {
            MusicLogUtils.v(TAG, "deleteTracks(),sService is null");
            return deleteTrackNum;
        }
        String [] cols = new String [] { MediaStore.Audio.Media._ID,
                MediaStore.Audio.Media.DATA, MediaStore.Audio.Media.ALBUM_ID };
        StringBuilder where = new StringBuilder();
        where.append(MediaStore.Audio.Media._ID + " IN (");
        for (int i = 0; i < list.length; i++) {
            where.append(list[i]);
            if (i < list.length - 1) {
                where.append(",");
            }
        }
        where.append(")");
        /// M: Make sure database exist when deleting and make sure we have latest news @{
        if (!hasMountedSDcard(context)) {
            return deleteTrackNum;
        }
        /// @}
        Cursor c = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, cols,
                where.toString(), null, null);

        if (c != null) {
            if (c.getCount() == 0) {
                MusicLogUtils.v(TAG, "c.getCount()==0");
                c.close();
                return -1;
            }

            /// M: move this to removeTracksFromPlaylist, which need call in main thread
            /// to avoid do rmoveTrack in sub thread which will cause nextPlayer
            /// not sync. {@
            // step 1: remove selected tracks from the current playlist, as well
            // as from the album art cache
            /*try {
                c.moveToFirst();
                while (!c.isAfterLast()) {
                    // remove from current playlist
                    long id = c.getLong(0);
                    sService.removeTrack(id);
                    // remove from album art cache
                    long artIndex = c.getLong(2);
                    synchronized (sArtCache) {
                        sArtCache.remove(artIndex);
                    }
                    c.moveToNext();
                }
            } catch (RemoteException ex) {
                c.close();
                return deleteTrackNum;
            }*/
            /// remove from album art cache
            c.moveToFirst();
            while (!c.isAfterLast()) {
                long artIndex = c.getLong(2);
                synchronized (sArtCache) {
                    sArtCache.remove(artIndex);
                }
                c.moveToNext();
            }
            /// @}

            /// M: step 2: remove selected tracks from the database @{
            if (!hasMountedSDcard(context)) {
                c.close();
                return deleteTrackNum;
            }
            try {
                deleteTrackNum = context.getContentResolver().
                delete(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, where.toString(), null);
            } catch (UnsupportedOperationException ex) {
                // Just in case
                c.close();
                return deleteTrackNum;
            }
            /// @}

            // step 3: remove files from card
            c.moveToFirst();
            /// M: add filter (SDCard is mounted)
            while (!c.isAfterLast() && hasMountedSDcard(context)) {
                String name = c.getString(1);
                File f = new File(name);
                try {  // File.delete can throw a security exception
                    if (!f.delete()) {
                        // I'm not sure if we'd ever get here (deletion would
                        // have to fail, but no exception thrown)
                        MusicLogUtils.v(TAG, "Failed to delete file " + name);
                    }
                    c.moveToNext();
                } catch (SecurityException ex) {
                    c.moveToNext();
                }
            }
            c.close();
        }

        // We deleted a number of tracks, which could affect any number of things
        // in the media content domain, so update everything.
        context.getContentResolver().notifyChange(Uri.parse("content://media"), null);
        MusicLogUtils.v(TAG, "<< deleteTracks: num = " + deleteTrackNum);
        return deleteTrackNum;
    }

    public static void addToCurrentPlaylist(Context context, long [] list) {
        if (sService == null) {
            return;
        }
        try {
            sService.enqueue(list, MediaPlaybackService.LAST);
            String message = context.getResources().getQuantityString(
                    R.plurals.NNNtrackstoplaylist, list.length, Integer.valueOf(list.length));
            Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
        } catch (RemoteException ex) {
        }
    }

    private static ContentValues[] sContentValuesCache = null;

    /**
     * @param ids The source array containing all the ids to be added to the playlist
     * @param offset Where in the 'ids' array we start reading
     * @param len How many items to copy during this pass
     * @param base The play order offset to use for this pass
     */
    private static void makeInsertItems(long[] ids, int offset, int len, int base) {
        // adjust 'len' if would extend beyond the end of the source array
        if (offset + len > ids.length) {
            len = ids.length - offset;
        }
        // allocate the ContentValues array, or reallocate if it is the wrong size
        if (sContentValuesCache == null || sContentValuesCache.length != len) {
            sContentValuesCache = new ContentValues[len];
        }
        // fill in the ContentValues array with the right values for this pass
        for (int i = 0; i < len; i++) {
            if (sContentValuesCache[i] == null) {
                sContentValuesCache[i] = new ContentValues();
            }

            sContentValuesCache[i].put(MediaStore.Audio.Playlists.Members.PLAY_ORDER,
             base + offset + i);
            sContentValuesCache[i].put(MediaStore.Audio.Playlists.Members.AUDIO_ID,
             ids[offset + i]);
        }
    }

    public static void addToPlaylist(Context context, long [] ids, long playlistid) {
        if (ids == null) {
            // this shouldn't happen (the menuitems shouldn't be visible
            // unless the selected item represents something playable
            MusicLogUtils.v(TAG, "ListSelection null");
        } else {
            int size = ids.length;
            ContentResolver resolver = context.getContentResolver();
            // need to determine the number of items currently in the playlist,
            // so the play_order field can be maintained.
            /// M: use last play order as base to avoid same order which may happen when remove
            /// audio from playlist and add a new one. {@
            String[] cols = new String[] {
                    MediaStore.Audio.Playlists.Members.PLAY_ORDER
            };
            Cursor cur = null;
            try {
                Uri uri = MediaStore.Audio.Playlists.Members.getContentUri("external", playlistid);
                cur = resolver.query(uri, cols, null, null,
                        MediaStore.Audio.Playlists.Members.PLAY_ORDER + " desc");
                int base = 0;
                int numinserted = 0;
                if (cur != null && cur.moveToFirst()) {
                    base = cur.getInt(0) + 1;
                }
                MusicLogUtils.v(TAG, "addToPlaylist: base = " + base);
                for (int i = 0; i < size; i += 1000) {
                    makeInsertItems(ids, i, 1000, base);
                    numinserted += resolver.bulkInsert(uri, sContentValuesCache);
                }
                String message = context.getResources().getQuantityString(
                        R.plurals.NNNtrackstoplaylist, numinserted, numinserted);
                Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
            } catch (UnsupportedOperationException ex) {
                MusicLogUtils.v(TAG, "addToPlaylist() with UnsupportedOperationException:" + ex);
            } finally {
                if (cur != null) {
                    cur.close();
                    cur = null;
                }
            }
            /// @}
            //mLastPlaylistSelected = playlistid;
        }
    }

    public static Cursor query(Context context, Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder, int limit) {
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
    public static Cursor query(Context context, Uri uri, String[] projection,
            String selection, String[] selectionArgs, String sortOrder) {
        return query(context, uri, projection, selection, selectionArgs, sortOrder, 0);
    }

    public static boolean isMediaScannerScanning(Context context) {
        boolean result = false;
        Cursor cursor = query(context, MediaStore.getMediaScannerUri(),
                new String [] { MediaStore.MEDIA_SCANNER_VOLUME }, null, null, null);
        if (cursor != null) {
            result = cursor.getCount() > 0;
            cursor.close();
        }

        return result;
    }

    public static void setSpinnerState(Activity a) {
        if (isMediaScannerScanning(a)) {
            /// M: start the progress spinner
            a.setProgressBarIndeterminateVisibility(true);
        } else {
            /// M: stop the progress spinner
            a.setProgressBarIndeterminateVisibility(false);
        }
    }

    /// M: set default value
    private static String sLastSdStatus = null;

    /// M: add filter SDCard status to display database error @{
    public static void displayDatabaseError(Activity a, boolean isMounted) {
        if (a.isFinishing()) {
            // When switching tabs really fast, we can end up with a null
            // cursor (not sure why), which will bring us here.
            // Don't bother showing an error message in that case.
            return;
        }

        String status = Environment.getExternalStorageState();
        int title;
        int message;
        if ((null != sLastSdStatus) && (sLastSdStatus.equals(status))) {
            MusicLogUtils.v(TAG, "displayDatabaseError: SD status is not change");
            return;
        }
        MusicLogUtils.v(TAG, "displayDatabaseError: SD status=" + status);
        sLastSdStatus = status;

        //if (android.os.Environment.isExternalStorageRemovable()) {
        //    title = R.string.sdcard_error_title;
        //    message = R.string.sdcard_error_message;
        //} else {
        //    title = R.string.sdcard_error_title_nosdcard;
        //    message = R.string.sdcard_error_message_nosdcard;
        //}
        title = R.string.sdcard_busy_title;
        message = R.string.sdcard_busy_message;

        if (status.equals(Environment.MEDIA_SHARED) ||
                status.equals(Environment.MEDIA_UNMOUNTED)) {
            //if (android.os.Environment.isExternalStorageRemovable()) {
                title = R.string.sdcard_busy_title;
                message = R.string.sdcard_busy_message;
            //} else {
            //    title = R.string.sdcard_busy_title_nosdcard;
            //    message = R.string.sdcard_busy_message_nosdcard;
            //}
        } else if (status.equals(Environment.MEDIA_REMOVED)) {
            //if (android.os.Environment.isExternalStorageRemovable()) {
                title = R.string.sdcard_missing_title;
                message = R.string.sdcard_missing_message;
            //} else {
            //    title = R.string.sdcard_missing_title_nosdcard;
            //    message = R.string.sdcard_missing_message_nosdcard;
            //}
        } else if (status.equals(Environment.MEDIA_MOUNTED) && isMounted) {
            // The card is mounted, but we didn't get a valid cursor.
            // This probably means the mediascanner hasn't started scanning the
            // card yet (there is a small window of time during boot where this
            // will happen).
            a.setTitle("");
            Intent intent = new Intent();
            intent.setClass(a, ScanningProgress.class);

            Activity parent = a.getParent();
            if (parent != null) {
                parent.startActivityForResult(intent, Defs.SCAN_DONE);
            } else {
                a.startActivityForResult(intent, Defs.SCAN_DONE);
            }
        }

        a.setTitle(title);
        /// M: Show sdcard error. {@
        View v = a.findViewById(R.id.sd_error);
        if (v != null) {
            v.setVisibility(View.VISIBLE);
        }
        /// @}
        v = a.findViewById(R.id.sd_message);
        if (v != null) {
            v.setVisibility(View.VISIBLE);
        }
        v = a.findViewById(R.id.sd_icon);
        if (v != null) {
            v.setVisibility(View.VISIBLE);
        }
        v = a.findViewById(android.R.id.list);
        if (v != null) {
            v.setVisibility(View.GONE);
        }
        v = a.findViewById(R.id.nowplaying);
        if (v != null) {
            View parent = (View) v.getParent();
            parent.setVisibility(View.GONE);
        }
        /// M: disappear empty view. @{
        v = a.findViewById(R.id.scan);
        if (v != null) {
            v.setVisibility(View.GONE);
        }
        /// @}
        TextView tv = (TextView) a.findViewById(R.id.sd_message);
        tv.setText(message);

        Intent i = new Intent(SDCARD_STATUS_UPDATE);
        i.putExtra(SDCARD_STATUS_MESSAGE, message);
        i.putExtra(SDCARD_STATUS_ONOFF, false);
        a.sendBroadcast(i);
    }
    /// @}

    public static void hideDatabaseError(Activity a) {
        /// M: Disappear sdcard error. {@
        View v = a.findViewById(R.id.sd_error);
        if (v != null) {
            v.setVisibility(View.GONE);
        }
        /// @}
        v = a.findViewById(R.id.sd_message);
        if (v != null) {
            v.setVisibility(View.GONE);
        }
        v = a.findViewById(R.id.sd_icon);
        if (v != null) {
            v.setVisibility(View.GONE);
        }
        v = a.findViewById(android.R.id.list);
        if (v != null) {
            v.setVisibility(View.VISIBLE);
        }
        /// M: update nowplaying and send broadcast about SDCard status @{
        Intent i = new Intent(SDCARD_STATUS_UPDATE);
        i.putExtra(SDCARD_STATUS_ONOFF, true);
        a.sendBroadcast(i);
        /// @}
        MusicLogUtils.v(TAG, "hideDatabaseError when sdcard mounted!");
    }

    static protected Uri getContentURIForPath(String path) {
        return Uri.fromFile(new File(path));
    }


    /*  Try to use String.format() as little as possible, because it creates a
     *  new Formatter every time you call it, which is very inefficient.
     *  Reusing an existing Formatter more than tripled the speed of
     *  makeTimeString().
     *  This Formatter/StringBuilder are also used by makeAlbumSongsLabel()
     */
    private static StringBuilder sFormatBuilder = new StringBuilder();
    private static Formatter sFormatter = new Formatter(sFormatBuilder, Locale.getDefault());
    private static final Object[] sTimeArgs = new Object[5];

    public static String makeTimeString(Context context, long secs) {
        String durationformat = context.getString(
                secs < 3600 ? R.string.durationformatshort : R.string.durationformatlong);

        /* Provide multiple arguments so the format can be changed easily
         * by modifying the xml.
         */
        sFormatBuilder.setLength(0);

        final Object[] timeArgs = sTimeArgs;
        timeArgs[0] = secs / 3600;
        timeArgs[1] = secs / 60;
        timeArgs[2] = (secs / 60) % 60;
        timeArgs[3] = secs;
        timeArgs[4] = secs % 60;
        /// M: use local format
        return sFormatter.format(Locale.getDefault(), durationformat, timeArgs).toString();
    }

    public static void shuffleAll(Context context, Cursor cursor) {
        /// M: modify position from 0 to -1 for divide shuffle and no-shuffle
        playAll(context, cursor, -1, true);
    }

    public static void playAll(Context context, Cursor cursor) {
        playAll(context, cursor, 0, false);
    }

    public static void playAll(Context context, Cursor cursor, int position) {
        playAll(context, cursor, position, false);
    }

    public static void playAll(Context context, long [] list, int position) {
        playAll(context, list, position, false);
    }

    private static void playAll(Context context, Cursor cursor,
     int position, boolean force_shuffle) {

        long [] list = getSongListForCursor(cursor);
        playAll(context, list, position, force_shuffle);
    }

    private static void playAll(Context context, long [] list,
     int position, boolean force_shuffle) {
     MusicLogUtils.v(TAG, "Play<<" +list.length+", service = "+sService);
        if (list.length == 0 || sService == null) {
            MusicLogUtils.v(TAG, "attempt to play empty song list");
            // Don't try to play empty playlists. Nothing good will come of it.
            String message = context.getString(R.string.emptyplaylist, list.length);
            Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
            return;
        }
        try {
            /// M: start playback first.
            MusicLogUtils.v(TAG, "Play 1");
            Intent intent = new Intent().setClass(context, MediaPlaybackActivity.class);
            context.startActivity(intent);

            if (force_shuffle) {
                sService.setShuffleMode(MediaPlaybackService.SHUFFLE_NORMAL);
            }
            long curid = sService.getAudioId();
            int curpos = sService.getQueuePosition();
			MusicLogUtils.v(TAG, "position = "+position+", currid = "+curid+", curpos = "+curpos);
            if (position != -1 && curpos == position && curid == list[position]) {
                // The selected file is the file that's currently playing;
                // figure out if we need to restart with a new playlist,
                // or just launch the playback activity.
                long [] playlist = sService.getQueue();
                if (Arrays.equals(list, playlist)) {
                    // we don't need to set a new list, but we should resume playback if needed
                    MusicLogUtils.v(TAG, "playAll: same playlist!");
                    sService.play();
                    return; // the 'finally' block will still run
                }
            }
            if (position < 0) {
                position = 0;
            }
            sService.open(list, force_shuffle ? -1 : position);
            /// M: We use async prepare in service, so it will auto playback after finish
            /// prepare, need not call play to trigger it.
            //sService.play();
        } catch (RemoteException ex) {
        } finally {
            /// M: launch assigned activity
            /*Intent intent = new Intent().setClass(context, MediaPlaybackActivity.class)
                .setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            context.startActivity(intent);*/
        }
		MusicLogUtils.v(TAG, "Play>>");
    }

    public static void clearQueue() {
        try {
            sService.removeTracks(0, Integer.MAX_VALUE);
            /// M: If clear queue with party shuffle mode, turn off party shuffle.
            if (sService.getShuffleMode() == MediaPlaybackService.SHUFFLE_AUTO) {
                sService.setShuffleMode(MediaPlaybackService.SHUFFLE_NONE);
            }
        } catch (RemoteException ex) {
        }
    }

    // A really simple BitmapDrawable-like class, that doesn't do
    // scaling, dithering or filtering.
    private static class FastBitmapDrawable extends Drawable {
        private Bitmap mBitmap;
        public FastBitmapDrawable(Bitmap b) {
            mBitmap = b;
        }
        @Override
        public void draw(Canvas canvas) {
            canvas.drawBitmap(mBitmap, 0, 0, null);
        }
        @Override
        public int getOpacity() {
            return PixelFormat.OPAQUE;
        }
        @Override
        public void setAlpha(int alpha) {
        }
        @Override
        public void setColorFilter(ColorFilter cf) {
        }
    }

    private static int sArtId = -2;
    private static Bitmap mCachedBit = null;
    private static final BitmapFactory.Options sBitmapOptionsCache = new BitmapFactory.Options();
    private static final BitmapFactory.Options sBitmapOptions = new BitmapFactory.Options();
    private static final Uri sArtworkUri = Uri.parse("content://media/external/audio/albumart");
    private static final HashMap<Long, Drawable> sArtCache = new HashMap<Long, Drawable>();
    private static int sArtCacheId = -1;

    static {
        //for the cache,
        //565 is faster to decode and display
        //and we don't want to dither here because the image will be scaled down later
        sBitmapOptionsCache.inPreferredConfig = Bitmap.Config.RGB_565;
        sBitmapOptionsCache.inDither = false;

        /// M:Modify for Contour issue
        sBitmapOptions.inPreferredConfig = Bitmap.Config.ARGB_8888;
        sBitmapOptions.inDither = false;
    }

    public static void initAlbumArtCache() {
        try {
            int id = sService.getMediaMountedCount();
            if (id != sArtCacheId) {
                clearAlbumArtCache();
                sArtCacheId = id;
            }
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public static void clearAlbumArtCache() {
        synchronized (sArtCache) {
            sArtCache.clear();
        }
    }

    public static Drawable getCachedArtwork(Context context,
     long artIndex, BitmapDrawable defaultArtwork) {
        Drawable d = null;
        synchronized (sArtCache) {
            d = sArtCache.get(artIndex);
        }
        if (d == null) {
            d = defaultArtwork;
            final Bitmap icon = defaultArtwork.getBitmap();
            int w = icon.getWidth();
            int h = icon.getHeight();
            /// M: call getArtwork to get album art bitmap
            Bitmap b = MusicUtils.getArtwork(context, -1, artIndex, false);
            if (b != null) {
                /// M: scaled from an existing bitmap to assigned size
                b = Bitmap.createScaledBitmap(b, w, h, true);
                d = new FastBitmapDrawable(b);
                synchronized (sArtCache) {
                    // the cache may have changed since we checked
                    Drawable value = sArtCache.get(artIndex);
                    if (value == null) {
                        sArtCache.put(artIndex, d);
                    } else {
                        d = value;
                    }
                }
            }
        }
        return d;
    }

    // Get album art for specified album. This method will not try to
    // fall back to getting artwork directly from the file, nor will
    // it attempt to repair the database.
    /*private static Bitmap getArtworkQuick(Context context, long album_id, int w, int h) {
        // NOTE: There is in fact a 1 pixel border on the right side in the ImageView
        // used to display this drawable. Take it into account now, so we don't have to
        // scale later.
        w -= 1;
        ContentResolver res = context.getContentResolver();
        Uri uri = ContentUris.withAppendedId(sArtworkUri, album_id);
        if (uri != null) {
            ParcelFileDescriptor fd = null;
            try {
                fd = res.openFileDescriptor(uri, "r");
                int sampleSize = 1;

                // Compute the closest power-of-two scale factor
                // and pass that to sBitmapOptionsCache.inSampleSize, which will
                // result in faster decoding and better quality
                sBitmapOptionsCache.inJustDecodeBounds = true;
                BitmapFactory.decodeFileDescriptor(
                        fd.getFileDescriptor(), null, sBitmapOptionsCache);
                int nextWidth = sBitmapOptionsCache.outWidth >> 1;
                int nextHeight = sBitmapOptionsCache.outHeight >> 1;
                while (nextWidth>w && nextHeight>h) {
                    sampleSize <<= 1;
                    nextWidth >>= 1;
                    nextHeight >>= 1;
                }

                sBitmapOptionsCache.inSampleSize = sampleSize;
                sBitmapOptionsCache.inJustDecodeBounds = false;
                Bitmap b = BitmapFactory.decodeFileDescriptor(
                        fd.getFileDescriptor(), null, sBitmapOptionsCache);

                if (b != null) {
                    // finally rescale to exactly the size we need
                    if (sBitmapOptionsCache.outWidth != w || sBitmapOptionsCache.outHeight != h) {
                        Bitmap tmp = Bitmap.createScaledBitmap(b, w, h, true);
                        // Bitmap.createScaledBitmap() can return the same bitmap
                        if (tmp != b) b.recycle();
                        b = tmp;
                    }
                }

                return b;
            } catch (FileNotFoundException e) {
            } finally {
                try {
                    if (fd != null)
                        fd.close();
                } catch (IOException e) {
                }
            }
        }
        return null;
    }*/

    /** Get album art for specified album. You should not pass in the album id
     * for the "unknown" album here (use -1 instead)
     * This method always returns the default album art icon when no album art is found.
     */
    public static Bitmap getArtwork(Context context, long song_id, long album_id) {
        return getArtwork(context, song_id, album_id, true);
    }

    /** Get album art for specified album. You should not pass in the album id
     * for the "unknown" album here (use -1 instead)
     */
    public static Bitmap getArtwork(Context context, long song_id, long album_id,
            boolean allowdefault) {
        MusicLogUtils.v(TAG, ">> getArtWork, song_id=" + song_id + ", album_id=" + album_id);
        if (album_id < 0) {
            // This is something that is not in the database, so get the album art directly
            // from the file.
            if (song_id >= 0) {
                Bitmap bm = getArtworkFromFile(context, song_id, -1);
                if (bm != null) {
                    return bm;
                }
            }
            if (allowdefault) {
                return getDefaultArtwork(context);
            }
            return null;
        }

        ContentResolver res = context.getContentResolver();
        Uri uri = ContentUris.withAppendedId(sArtworkUri, album_id);
        if (uri != null) {
            InputStream in = null;
            try {
                in = res.openInputStream(uri);
                return BitmapFactory.decodeStream(in, null, sBitmapOptions);
            } catch (FileNotFoundException ex) {
                // The album art thumbnail does not actually exist. Maybe the user deleted it, or
                // maybe it never existed to begin with.
                MusicLogUtils.d(TAG, "getArtWork: open " + uri.toString() +
                 " failed, try getArtworkFromFile");
                Bitmap bm = getArtworkFromFile(context, song_id, album_id);
                if (bm != null) {
                    if (bm.getConfig() == null) {
                        bm = bm.copy(Bitmap.Config.RGB_565, false);
                        if (bm == null && allowdefault) {
                            return getDefaultArtwork(context);
                        }
                    }
                } else if (allowdefault) {
                    bm = getDefaultArtwork(context);
                }
                return bm;
            } finally {
                try {
                    if (in != null) {
                        in.close();
                    }
                } catch (IOException ex) {
                }
            }
        }

        return null;
    }

    // get album art for specified file
    private static final String sExternalMediaUri =
     MediaStore.Audio.Media.EXTERNAL_CONTENT_URI.toString();
    private static Bitmap getArtworkFromFile(Context context, long songid, long albumid) {
        MusicLogUtils.v(TAG, ">> getArtworkFromFile, songid=" + songid + ", albumid=" + albumid);
        Bitmap bm = null;
        byte [] art = null;
        String path = null;

        if (albumid < 0 && songid < 0) {
            throw new IllegalArgumentException("Must specify an album or a song id");
        }
        ParcelFileDescriptor pfd = null;
        try {
            if (albumid < 0) {
                Uri uri = Uri.parse("content://media/external/audio/media/" + songid + "/albumart");
                pfd = context.getContentResolver().openFileDescriptor(uri, "r");
                MusicLogUtils.v(TAG, "getArtworkFromFile: pFD=" + pfd);
                if (pfd != null) {
                    FileDescriptor fd = pfd.getFileDescriptor();
                    if (fd != null) {
                        bm = BitmapFactory.decodeFileDescriptor(fd);
                    } else {
                        MusicLogUtils.v(TAG, "getArtworkFromFile: fd is null");
                    }
                }
            } else {
                Uri uri = ContentUris.withAppendedId(sArtworkUri, albumid);
                pfd = context.getContentResolver().openFileDescriptor(uri, "r");
                MusicLogUtils.v(TAG, "getArtworkFromFile: pFD=" + pfd);
                if (pfd != null) {
                    FileDescriptor fd = pfd.getFileDescriptor();
                    if (fd != null) {
                        bm = BitmapFactory.decodeFileDescriptor(fd);
                    } else {
                        MusicLogUtils.v(TAG, "getArtworkFromFile: fd is null");
                    }
                }
            }
        } catch (IllegalStateException ex) {
        } catch (FileNotFoundException ex) {
            MusicLogUtils.v(TAG, "getArtworkFromFile: FileNotFoundException!");
        } finally {
            try {
                if (pfd != null) {
                    pfd.close();
                }
            } catch (IOException e) {
                MusicLogUtils.v(TAG, "fd.close: IOException!");
            }
        }
        if (bm != null) {
            mCachedBit = bm;
        }
        MusicLogUtils.v(TAG, "<< getArtworkFromFile: " + bm);
        return bm;
    }

    static Bitmap getDefaultArtwork(Context context) {
        MusicLogUtils.v(TAG, "getDefaultArtwork");
        BitmapFactory.Options opts = new BitmapFactory.Options();
        opts.inPreferredConfig = Bitmap.Config.ARGB_8888;
        return BitmapFactory.decodeStream(
                context.getResources().openRawResource(R.drawable.albumart_mp_unknown), null, opts);
    }
    static long getLongPref(Context context, String name, long def) {
           SharedPreferences prefs =
               context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE);
           return prefs.getLong(name, def);
       }

       static void setLongPref(Context context, String name, long value) {
           SharedPreferences prefs =
               context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE);
           Editor ed = prefs.edit();
           ed.putLong(name, value);
           SharedPreferencesCompat.apply(ed);
       }

    static int getIntPref(Context context, String name, int def) {
        SharedPreferences prefs =
            context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE);
        return prefs.getInt(name, def);
    }

    static void setIntPref(Context context, String name, int value) {
        SharedPreferences prefs =
            context.getSharedPreferences(context.getPackageName(), Context.MODE_PRIVATE);
        Editor ed = prefs.edit();
        ed.putInt(name, value);
        SharedPreferencesCompat.apply(ed);
    }

    static void showCreatePlaylistToast(int listLength, Context context) {
        String message = context.getResources().getQuantityString(
                                R.plurals.NNNtrackstoplaylist, listLength, listLength);
                        Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
    }

    static void showDeleteToast(int deleteNum, Context context) {
        String message = context.getResources().getQuantityString(R.plurals.NNNtracksdeleted,
                                deleteNum, Integer.valueOf(deleteNum));
                        Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
    }

    static void setRingtone(Context context, long id) {
        ContentResolver resolver = context.getContentResolver();
        // Set the flag in the database to mark this as a ringtone
        if (context.checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED || context
                    .checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
                    != PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(context, R.string.music_storage_permission_deny
                                   , Toast.LENGTH_SHORT).show();
                    return;
                }
        Uri ringUri = ContentUris.withAppendedId(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, id);
        try {
            ContentValues values = new ContentValues(2);
            values.put(MediaStore.Audio.Media.IS_RINGTONE, "1");
            values.put(MediaStore.Audio.Media.IS_ALARM, "1");
            resolver.update(ringUri, values, null, null);
        } catch (UnsupportedOperationException ex) {
            // most likely the card just got unmounted
            MusicLogUtils.v(TAG, "couldn't set ringtone flag for id " + id);
            return;
        }

        String[] cols = new String[] {
                MediaStore.Audio.Media._ID,
                MediaStore.Audio.Media.DATA,
                MediaStore.Audio.Media.TITLE
        };
        /// M: use selectionArgs replace set query value in where @{
        String where = MediaStore.Audio.Media._ID + "=?";
        String[] whereArgs = new String[]{String.valueOf(id)};
        Cursor cursor = query(context, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                cols, where , whereArgs, null);
        /// @}
        try {
            if (cursor != null && cursor.getCount() == 1) {
                // Set the system setting to make this the current ringtone
                cursor.moveToFirst();
                Settings.System.putString(resolver, Settings.System.RINGTONE, ringUri.toString());
                String message = context.getString(R.string.ringtone_set, cursor.getString(2));
                Toast.makeText(context, message, Toast.LENGTH_SHORT).show();
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    /**
     * M: Update nowplaying view.
     * Add a new parameter orientation, because only need show nowplaying bottom bar in portrait.
     * @param activity
     * @param orientation The orientation, when in landscape needn't show nowplaying
     */
    static void updateNowPlaying(Activity activity, int orientation) {
        /// M: When cannot find nowplaying or current is
        /// landscape or sd error shown, need not show nowplaying. {@
        View nowPlayingView = activity.findViewById(R.id.nowplaying);
        if (nowPlayingView == null) {
            return;
        }
        MusicLogUtils.v(TAG, "updateNowPlaying: activity = "
         + activity + ", orientaiton = " + orientation);
        View nowPlayingParent = (View) nowPlayingView.getParent();
        if ((orientation == Configuration.ORIENTATION_LANDSCAPE)
                || (activity.findViewById(R.id.sd_icon).getVisibility() == View.VISIBLE)) {
            nowPlayingParent.setVisibility(View.GONE);
            return;
        }
        /// @}

        /// M: Get the blank view between search button and overflow button,
        /// enable it when only search button and
        /// overflow button show on nowplaying bottom bar to make
        /// search button at left side of nowplaying bottom
        /// bar and overflow button at right side.
        View blankView = activity.findViewById(R.id.blank_between_search_and_overflow);

        /// M: Show nowplaying view when exist a prepared audio
        /// and show the track name and artist name in the view.
        try {
            if (MusicUtils.sService != null && MusicUtils.sService.getAudioId() != -1) {
                TextView title = (TextView) nowPlayingView.findViewById(R.id.title);
                TextView artist = (TextView) nowPlayingView.findViewById(R.id.artist);
                title.setText(MusicUtils.sService.getTrackName());
                /// M: Set marquee for track name to show total track name to user.
                title.setSelected(true);
                /// M: change the text color along theme change @{
                /*if (MusicFeatureOption.IS_SUPPORT_THEMEMANAGER) {
                    int textColor = activity.getResources().getThemeMainColor();
                    if (textColor != 0) {
                        title.setTextColor(textColor);
                    }
                }*/
                /// @}
                String artistName = MusicUtils.sService.getArtistName();
                if (MediaStore.UNKNOWN_STRING.equals(artistName)) {
                    artistName = activity.getString(R.string.unknown_artist_name);
                }
                artist.setText(artistName);
                //mNowPlayingView.setOnFocusChangeListener(mFocuser);
                //mNowPlayingView.setOnClickListener(this);
                nowPlayingView.setVisibility(View.VISIBLE);
                nowPlayingView.setOnClickListener(new View.OnClickListener() {

                public void onClick(View v) {
                    Context c = v.getContext();
                    c.startActivity(new Intent(c, MediaPlaybackActivity.class));
                } });

                /// M: Enable nowplaying parent view and disable the blank view.
                nowPlayingParent.setVisibility(View.VISIBLE);
                blankView.setVisibility(View.GONE);
                MusicLogUtils.v(TAG, "updateNowPlaying with id = " + sService.getAudioId()
                        + ", track name = " + sService.getTrackName());
            } else {
                nowPlayingParent.setVisibility(View.GONE);
                nowPlayingView.setVisibility(View.GONE);
            }
        } catch (RemoteException ex) {
            nowPlayingView.setVisibility(View.GONE);
            MusicLogUtils.v(TAG, "updateNowPlaying with RemoteException: " + ex);
        }

        /// M: Only when there is not action bar,
        /// we need show search button on nowplaying bottom bar, so if the
        /// update nowplaying activity is MusicBrowserActivity,
        /// set nowplaying search button to be visibility,
        /// otherwise set it to be gone. {@
        if (MusicBrowserActivity.class.equals(activity.getClass())) {
            /// M: Show search button in MusicBrowserActivity
            View overflowButton = activity.findViewById(R.id.overflow_menu_nowplaying);
            nowPlayingParent.setVisibility(View.VISIBLE);
            /// M: When nowplaying view not show, overflow button and search button show at
            /// nowplaying bottom bar
            /// enable blank view to make these two button distribute in both end of bottom bar.
            View searchButton = activity.findViewById(R.id.search_menu_nowplaying);
            if (nowPlayingView.getVisibility() != View.VISIBLE
                    && overflowButton.getVisibility() == View.VISIBLE
                    && searchButton.getVisibility() == View.VISIBLE) {
                blankView.setVisibility(View.VISIBLE);
            }
        } else {
            View searchButton = activity.findViewById(R.id.search_menu_nowplaying);
            searchButton.setVisibility(View.GONE);
        }
        /// @}

    }
    /// @}

    static void setBackground(View v, Bitmap bm) {

        if (bm == null) {
            v.setBackgroundResource(0);
            return;
        }

        int vwidth = v.getWidth();
        int vheight = v.getHeight();
        int bwidth = bm.getWidth();
        int bheight = bm.getHeight();
        float scalex = (float) vwidth / bwidth;
        float scaley = (float) vheight / bheight;
        float scale = Math.max(scalex, scaley) * 1.3f;
        if (vwidth == 0 || vheight == 0) {
            v.setBackgroundResource(0);
            return;
        }

        Bitmap.Config config = Bitmap.Config.ARGB_8888;
        Bitmap bg = Bitmap.createBitmap(vwidth, vheight, config);
        Canvas c = new Canvas(bg);
        Paint paint = new Paint();
        paint.setAntiAlias(true);
        paint.setFilterBitmap(true);
        ColorMatrix greymatrix = new ColorMatrix();
        greymatrix.setSaturation(0);
        ColorMatrix darkmatrix = new ColorMatrix();
        darkmatrix.setScale(.3f, .3f, .3f, 1.0f);
        greymatrix.postConcat(darkmatrix);
        ColorFilter filter = new ColorMatrixColorFilter(greymatrix);
        paint.setColorFilter(filter);
        Matrix matrix = new Matrix();
        matrix.setTranslate(-bwidth / 2, -bheight / 2); // move bitmap center to origin
        matrix.postRotate(10);
        matrix.postScale(scale, scale);
        matrix.postTranslate(vwidth / 2, vheight / 2);  // Move bitmap center to view center
        c.drawBitmap(bm, matrix, paint);
        //v.setBackgroundDrawable(new BitmapDrawable(bg));
        v.setBackground(new BitmapDrawable(v.getResources(), bg));
    }

    static int getCardId(Context context) {
        ContentResolver res = context.getContentResolver();
        Cursor c = res.query(Uri.parse("content://media/external/fs_id"), null, null, null, null);
        int id = -1;
        if (c != null) {
            c.moveToFirst();
            id = c.getInt(0);
            c.close();
        } else {
            MusicLogUtils.v(TAG, "getCardId: cursor=null");
        }
        return id;
    }

    static class LogEntry {
        Object item;
        long time;

        LogEntry(Object o) {
            item = o;
            time = System.currentTimeMillis();
        }

        void dump(PrintWriter out) {
            sTime.set(time);
            out.print(sTime.toString() + " : ");
            if (item instanceof Exception) {
                ((Exception) item).printStackTrace(out);
            } else {
                out.println(item);
            }
        }
    }

    private static LogEntry[] sMusicLog = new LogEntry[100];
    private static int sLogPtr = 0;
    private static Time sTime = new Time();

    static void debugLog(Object o) {

        sMusicLog[sLogPtr] = new LogEntry(o);
        sLogPtr++;
        if (sLogPtr >= sMusicLog.length) {
            sLogPtr = 0;
        }
    }

    static void debugDump(PrintWriter out) {
        for (int i = 0; i < sMusicLog.length; i++) {
            int idx = (sLogPtr + i);
            if (idx >= sMusicLog.length) {
                idx -= sMusicLog.length;
            }
            LogEntry entry = sMusicLog[idx];
            if (entry != null) {
                entry.dump(out);
            }
        }
    }

    /**
     * Reset mLastSdStatus to be null, so that can refresh database error UI.
     *
     */
    public static void resetSdStatus() {
        sLastSdStatus = null;
        MusicLogUtils.v(TAG, "Reset mLastSdStatus to be null to refresh database error UI!");
    }

    /**
     * M: Whether the device has a mounded sdcard or not.
     *
     * @param context a context.
     * @return If one of the sdcard is mounted, return true, otherwise return false.
     */
    static boolean hasMountedSDcard(Context context) {
        StorageManager storageManager = (StorageManager) context.getSystemService(
                Context.STORAGE_SERVICE);
        boolean hasMountedSDcard = false;
        if (storageManager != null) {
            String[] volumePath = storageManager.getVolumePaths();
            if (volumePath != null) {
                String status = null;
                int length = volumePath.length;
                for (int i = 0; i < length; i++) {
                    status = storageManager.getVolumeState(volumePath[i]);
                    MusicLogUtils.v(TAG, "hasMountedSDcard: path = " + volumePath[i] + ",status = "
                            + status);
                    if (Environment.MEDIA_MOUNTED.equals(status)) {
                        hasMountedSDcard = true;
                    }
                }
            }
        }
        MusicLogUtils.v(TAG, "hasMountedSDcard = " + hasMountedSDcard);
        return hasMountedSDcard;
    }

    /**
     * M: Add search view to activity option menu.
     *
     * @param activity The activity
     * @param menu The activity menu
     * @param queryTextListener The query text listener for the search view
     * @param suggestionListener The suggestion listener for the search view
     * @return Search view
     */
    static MenuItem addSearchView(Activity activity, Menu menu,
            SearchView.OnQueryTextListener queryTextListener,
            SearchView.OnSuggestionListener suggestionListener) {
        activity.getMenuInflater().inflate(R.menu.music_search_menu, menu);
        MenuItem searchItem = menu.findItem(R.id.search);
        SearchView searchView = (SearchView) searchItem.getActionView();
        if (queryTextListener != null) {
            searchView.setOnQueryTextListener(queryTextListener);
        }
        if (suggestionListener != null) {
            searchView.setOnSuggestionListener(suggestionListener);
        }
        searchView.setQueryHint(activity.getString(R.string.search_hint));
        searchView.setIconifiedByDefault(true);
        SearchManager searchManager = (SearchManager) activity
                .getSystemService(Context.SEARCH_SERVICE);

        if (searchManager != null) {
            SearchableInfo info = searchManager.getSearchableInfo(activity.getComponentName());
            searchView.setSearchableInfo(info);
        }
        return searchItem;
    }

    /**
     *  M: when listview empty,show emptyview,and
     *  if when Sdcard is not ready,show scanning view.
     * @param list This listview will show emptyview.
     * @param a the Activity the view will be showed.
     */
    static void emptyShow(ListView list, Activity a) {
        if (list.getCount() == 0) {
            View scanView = a.findViewById(R.id.scan);
            if(scanView != null){
                scanView.setVisibility(View.VISIBLE);
                TextView text = (TextView) a.findViewById(R.id.message);
                MusicLogUtils.v(TAG, "empty show");
                View spinnerView =  scanView.findViewById(R.id.spinner);
                View messageView = scanView.findViewById(R.id.message);
                if(spinnerView != null && messageView != null){
                    if (MusicUtils.isMediaScannerScanning(a)) {
                    a.setProgressBarIndeterminateVisibility(false);
                    text.setText(R.string.scanning);
                    spinnerView.setVisibility(View.VISIBLE);
                    messageView.setVisibility(View.VISIBLE);
                } else if (hasMountedSDcard(a.getApplicationContext())) {
                    text.setText(R.string.no_music_title);
                    spinnerView.setVisibility(View.GONE);
                    messageView.setVisibility(View.VISIBLE);
                } else {
                    MusicLogUtils.v(TAG, "empty show gone");
                    text.setText(R.string.no_music_title);
                    spinnerView.setVisibility(View.GONE);
                    messageView.setVisibility(View.GONE);
                    }
                }
            }
            list.setEmptyView(scanView);
        } else {
            if (isMediaScannerScanning(a)) {
                a.setProgressBarIndeterminateVisibility(true);
            } else {
                a.setProgressBarIndeterminateVisibility(false);
            }
        }
    }

    /**
     * M: Start AudioEffect panel to adjust sound effect.
     *
     * @param activity The activity to start effect panel
     * @return If start success, return true, otherwise false
     */
    static boolean startEffectPanel(Activity activity) {
        if (sService == null) {
            return false;
        }
        try {
            int audioSessionId = sService.getAudioSessionId();
            Intent intent = new Intent(AudioEffect.ACTION_DISPLAY_AUDIO_EFFECT_CONTROL_PANEL);
            intent.putExtra(AudioEffect.EXTRA_AUDIO_SESSION, audioSessionId);
            /// M: must Use startActivityForResult() because That is the requirement of
            /// MusicEffect get the ActivityPackage.
            activity.startActivityForResult(intent, Defs.EFFECTS_PANEL);
            return true;
        } catch (RemoteException re) {
            MusicLogUtils.v(TAG, "RemoteException in start effect  " + re);
            return false;
        }
    }

    /**
     * M: set AudioEffect panel option menu to be enable only when AudioEffect class is exist.
     *
     * @param context Application context
     * @param menu The option menu to show Effect
     */
    static void setEffectPanelMenu(Context context, Menu menu) {
        /// M: Only show effect menu when effect class is exist.
        Intent i = new Intent(AudioEffect.ACTION_DISPLAY_AUDIO_EFFECT_CONTROL_PANEL);
        menu.findItem(Defs.EFFECTS_PANEL)
                .setVisible(context.getPackageManager().resolveActivity(i, 0) != null);
    }

    /**
     * M: Reset the static service instance to be null after Music Service has been destroyed.
     */
    static void resetStaticService() {
        sConnectionMap.clear();
        sService = null;
        MusicLogUtils.v(TAG, "resetStaticService when service onDestroy!");
    }

    /**
     * M: Check whether there is a client connect to service.
     *
     * @return If there is a client connect to service, return ture, otherwise false.
     */
    static boolean hasBoundClient() {
        if (sConnectionMap == null || sConnectionMap.isEmpty()) {
            return false;
        }
        return true;
    }

    /**
     * M: get a song list with given cursor and eliminate these songs in sub folder
     *
     * @param cursor
     * @param folderPath
     * @return The song list
     */
    static long [] getSongListForCursorExceptSubFolder(Cursor cursor, String folderPath) {
        if (cursor == null || folderPath == null) {
            return sEmptyList;
        }
        int len = cursor.getCount();
        long [] listAll = new long[len];
        cursor.moveToFirst();
        int columnId = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media._ID);
        int columnData = cursor.getColumnIndexOrThrow(MediaStore.Audio.Media.DATA);
        String data;
        String path;
        int songNum = 0;
        for (int i = 0; i < len; i++) {
            data = cursor.getString(columnData);
            path = data.substring(0, data.lastIndexOf("/"));
            /// Only put these songs in select folder to list
            MusicLogUtils.v(
                TAG,
                "getSongListForCursorExceptSub: path = " + path + ", folderPath = " + folderPath);
            if (folderPath.equals(path)) {
                listAll[songNum++] = cursor.getLong(columnId);
            }
            cursor.moveToNext();
        }
        /// If there are no audio in select folder except sub folder, return a empty list.
        if (songNum == 0) {
            MusicLogUtils.v(
                TAG, "getSongListForCursorExceptSubFolder: select folder has no music!");
            return sEmptyList;
        }
        /// Copy these audio in select folder to a new list
        len = songNum;
        long [] listExceptSubFolder = new long[len];
        for (int i = 0; i < len; i++) {
            listExceptSubFolder[i] = listAll[i];
        }
        return listExceptSubFolder;
    }

    /**
     * M: Get the playlist id with given name.
     *
     * @param context context
     * @param name playlist name
     * @return playlist id with given name if exist, otherwise -1.
     */
    static int idForplaylist(Context context, String name) {
        Cursor c = MusicUtils.query(context, MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists._ID, MediaStore.Audio.Playlists.NAME},
                /*MediaStore.Audio.Playlists.NAME + "=?"*/null,
                /*new String[] { name }*/null,
                MediaStore.Audio.Playlists.NAME);
        int id = -1;
        if (c == null) {
            return id;
        }
        c.moveToFirst();
        while (! c.isAfterLast()) {
            String playlistname = c.getString(1);
            if (playlistname != null && playlistname.compareToIgnoreCase(name) == 0) {
                id = c.getInt(0);
                break;
            }
            c.moveToNext();
        }
        c.close();
        return id;
    }

    /**
     * M: make playlist name not exsit in database with given template
     *
     * @param context Context
     * @param template A template to format the playlist name.
     * @return
     */
    static String makePlaylistName(Context context, String template) {
        int num = 1;

        String[] cols = new String[] {
                MediaStore.Audio.Playlists.NAME
        };
        ContentResolver resolver = context.getContentResolver();
        String whereclause = MediaStore.Audio.Playlists.NAME + " != ''";
        Cursor c = resolver.query(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
            cols, whereclause, null,
            MediaStore.Audio.Playlists.NAME);

        if (c == null) {
            return null;
        }

        String suggestedname;
        suggestedname = String.format(template, num++);

        // Need to loop until we've made 1 full pass through without finding a match.
        // Looping more than once shouldn't happen very often, but will happen if
        // you have playlists named "New Playlist 1"/10/2/3/4/5/6/7/8/9, where
        // making only one pass would result in "New Playlist 10" being erroneously
        // picked for the new name.
        boolean done = false;
        while (!done) {
            done = true;
            c.moveToFirst();
            while (! c.isAfterLast()) {
                String playlistname = c.getString(0);
                if (playlistname.compareToIgnoreCase(suggestedname) == 0) {
                    suggestedname = String.format(template, num++);
                    done = false;
                }
                c.moveToNext();
            }
        }
        c.close();
        return suggestedname;
    }

    /**
     * M: add method to judge has mounted the external sdcard
     * @param context
     * @return
     */
    static boolean hasMountedExternalSDcard(Context context) {
        StorageManager storageManager = (StorageManager) context
                .getSystemService(Context.STORAGE_SERVICE);
        boolean hasMountedExternalSDcard = false;
        if (storageManager != null) {
            String[] volumePath = storageManager.getVolumePaths();
            if (volumePath != null) {
                String status = null;
                int length = volumePath.length;
                if (length >= 2) {
                    for (int i = 1; i < length; i++) {
                        status = storageManager.getVolumeState(volumePath[1]);
                        MusicLogUtils.v(TAG, "hasMountedExternalSDcard: path = " + volumePath[i]
                                + ",status = " + status);
                        if (Environment.MEDIA_MOUNTED.equals(status)) {
                            hasMountedExternalSDcard = true;
                        }
                    }
                }

            }
        }
        MusicLogUtils.v(TAG, "hasMountedExternalSDcard = " + hasMountedExternalSDcard);
        return hasMountedExternalSDcard;
    }


    public static boolean isVoiceCapable(Context context) {
        TelephonyManager telephony =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);
        return telephony != null && telephony.isVoiceCapable();
    }
}
