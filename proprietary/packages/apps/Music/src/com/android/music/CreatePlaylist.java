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

import android.Manifest;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
//Not required anymore
//import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class CreatePlaylist extends DialogFragment {
    /// M: add for debug log
    private static final String TAG = "CreatePlaylist";
    /// M: add dialog key
    private static final int ALERT_DIALOG_KEY = 0;

    private EditText mPlaylist;
    //used dialogue instead
    //private TextView mPrompt;
    private Button mSaveButton;
    private static long [] sTempCursor;
    /// M: add for dialog

    private Dialog mDialog;
    /// @}
     public static CreatePlaylist newInstance(String itemId,
                                              int tabId,
                                              String flag,
                                              String name) {
        CreatePlaylist frag = new CreatePlaylist();
        Bundle args = new Bundle();
        args.putString(MusicUtils.ADD_TO_PLAYLIST_ITEM_ID, itemId);
        args.putInt(MusicUtils.START_ACTIVITY_TAB_ID, tabId);
        args.putString(MusicUtils.SAVE_PLAYLIST_FLAG, flag);
        args.putString(MusicUtils.PLAYLIST_NAME, name);
        frag.setArguments(args);
        return frag;
    }


public static void setCursor(long [] obj) {
       sTempCursor = obj;
    }

    private String mPrompt;

    /** M: Add to restore selected item id, such as album id, artist id and audio id. */
    private String mSelectItemId = null;
    /** M: Add to restore start activity tab, so that result can return to right activity. */
    private int mStartActivityTab = -1;
    private Intent mIntent = null;
    /// M: Add to indicate the  save_as_playlist and new_playlsit
    private String mPlaylistFlag = "";

    private String mPlaylistName = null;

    @Override
 public Dialog onCreateDialog(Bundle icicle) {


        MusicLogUtils.d(TAG, "onCreate");
        mDialog = new Dialog(getActivity());

        mDialog.setContentView(R.layout.rename_playlist);
        mPlaylist = (EditText) mDialog.findViewById(R.id.playlist);
        Button tv = (Button) mDialog.findViewById(R.id.rename_cancel);
        tv.setText(getResources().getString(R.string.cancel));
        tv.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View viewIn) {

                if (mDialog != null) {
                        mDialog.dismiss();
                        mDialog = null;
                    }
                }
        });
        tv = (Button) mDialog.findViewById(R.id.rename_done);
        tv.setText(getResources().getString(R.string.delete_confirm_button_text));
        tv.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View viewIn) {
                savePlaylist();
                if (mDialog != null) {
                        mDialog.dismiss();
                        mDialog = null;
                    }
                }
        });
        String defaultname = icicle != null ? icicle.getString("defaultname")
                : MusicUtils.makePlaylistName(getActivity().getApplicationContext(),
                                              getString(R.string.new_playlist_name_template));
        if (defaultname == null) {
            String toastShow = getString(R.string.save_playlist_error);
            Toast.makeText(getActivity().getApplicationContext(),
                           toastShow,
                           Toast.LENGTH_SHORT).show();
            if (mDialog != null) {
               mDialog.dismiss();
               mDialog = null;
            }
            return mDialog;
        }
        String promptformat = getString(R.string.create_playlist_create_text_prompt);
        /// M: get string for display in dialog
        mPrompt = String.format(promptformat, defaultname);
        mPlaylist.setText(defaultname);
        mPlaylist.setSelection(defaultname.length());
        mPlaylist.addTextChangedListener(mTextWatcher);
        /// M: register receiver about scanning sdcard @{
        IntentFilter f = new IntentFilter();
        f.addAction(Intent.ACTION_MEDIA_EJECT);
        f.addDataScheme("file");
        mIntent = getActivity().registerReceiver(mScanListener, f);
        /// M: Restore ADD_TO_PLAYLIST_ITEM_ID in mSelectItemId to return back.

            mSelectItemId = getArguments().getString(MusicUtils.ADD_TO_PLAYLIST_ITEM_ID);
            mStartActivityTab = getArguments().getInt(MusicUtils.START_ACTIVITY_TAB_ID);
            mPlaylistFlag = getArguments().getString(MusicUtils.SAVE_PLAYLIST_FLAG);
            mPlaylistName = getArguments().getString(MusicUtils.PLAYLIST_NAME);

     mDialog.setTitle(mPrompt);
     return mDialog;
    }

    TextWatcher mTextWatcher = new TextWatcher() {
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            // don't care about this one
        }
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            /// M: move the original code to setSaveButton
            setSaveButton();
        };
        public void afterTextChanged(Editable s) {
            // don't care about this one
        }
    };

    //Not required

    @Override
    public void onSaveInstanceState(Bundle outcicle) {
        outcicle.putString("defaultname", mPlaylist.getText().toString());
    }

    @Override
    public void onResume() {
        super.onResume();
        /// M: update the save button @{
        setSaveButton();
        /// @}
    }

    /**
     * M: override onDestroy() for unregister scan sdcard recever
     */
    @Override
    public void onDestroy() {
        getActivity().unregisterReceiver(mScanListener);
        super.onDestroy();
    }

    /**
     * M: update save button when the edit text is changing.
     */
    private void setSaveButton() {
        String newText = mPlaylist.getText().toString();
        if (mDialog == null) {
            MusicLogUtils.v(TAG, "setSaveButton with dialog is null return!");
            return;
        }
        if (mSaveButton == null) {
            mSaveButton = (Button) mDialog.findViewById(R.id.rename_done);
        }
        if (mSaveButton != null) {
            if (newText.trim().length() == 0) {
                mSaveButton.setEnabled(false);
            } else {
                mSaveButton.setEnabled(true);
                /// check if playlist with current name exists already, and warn the user if need.
                if (MusicUtils.idForplaylist(getActivity().getApplicationContext(),
                                             newText.trim()) >= 0) {
                    mSaveButton.setText(R.string.create_playlist_overwrite_text);
                } else {
                    mSaveButton.setText(R.string.create_playlist_create_text);
                }
                //Not required
                //c.moveToNext();
            }
        }
        MusicLogUtils.d(TAG, "setSaveButton " + mSaveButton);
    }

    /**
     * M: a button click listener for dialog; positive for save playlist;
     * neutral for quit directly.
     */

        public void savePlaylist() {
            if (getActivity().getApplicationContext()
                .checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
                Toast.makeText(getActivity().getApplicationContext(),
                    R.string.music_storage_permission_deny, Toast.LENGTH_SHORT).show();
                return  ;
            }
                String name = mPlaylist.getText().toString();
                if (name != null && name.length() > 0) {
                    name = name.trim();
                    Intent intent = new Intent();
                    ContentResolver resolver = getActivity().getContentResolver();
                    int id = MusicUtils.idForplaylist(getActivity().getApplicationContext(), name);
                    Uri uri = null;
                    /// M: Don't need to clearPlaylist when SAVE_AS_PLAYLIST @{
                    if (id >= 0 && (mPlaylistFlag.equals(MusicUtils.NEW_PLAYLIST))) {
                        uri = ContentUris.withAppendedId(
                                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI, id);
                        if (MusicUtils.clearPlaylist(getActivity().getApplicationContext(),
                                                     id) == -1) {
                            if (mDialog != null) {
                                mDialog.dismiss();
                                mDialog = null;
                            }
                            return;
                        }
                        ContentValues values = new ContentValues(1);
                        values.put(MediaStore.Audio.Playlists.NAME, name);
                        resolver.update(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                                values, MediaStore.Audio.Playlists._ID + "=?", new String[] {
                                    Long.valueOf(id).toString()
                                });
                    } else if (id >= 0 && mPlaylistFlag.equals(MusicUtils.SAVE_AS_PLAYLIST)) {
                        if (!name.equals(mPlaylistName) && name.equalsIgnoreCase(mPlaylistName)) {
                            ContentValues values = new ContentValues(1);
                            values.put(MediaStore.Audio.Playlists.NAME, name);
                            resolver.update(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                                    values, MediaStore.Audio.Playlists._ID + "=?", new String[] {
                                        Long.valueOf(id).toString()
                                    });
                        } else if (!name.equals(mPlaylistName) &&
                         !name.equalsIgnoreCase(mPlaylistName)) {
                            //ContentValues values = new ContentValues(1);
                            //values.put(MediaStore.Audio.Playlists.NAME, name);
                            try {
                                int newPid =
                                MusicUtils.idForplaylist(getActivity().getApplicationContext(),
                                                         name);
                                if (newPid >= 0) {
                                    uri = ContentUris
                                            .withAppendedId(
                                                    MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                                                    newPid);
                                    if (MusicUtils.clearPlaylist(
                                                   getActivity().getApplicationContext(),
                                                   newPid) == -1) {
                                         if (mDialog != null) {
                                             mDialog.dismiss();
                                             mDialog = null;
                                          }
                                        return;
                                    }
                                    ContentValues valuesNew = new ContentValues(1);
                                    valuesNew.put(MediaStore.Audio.Playlists.NAME, name);
                                    resolver.update(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                                            valuesNew, MediaStore.Audio.Playlists._ID + "=?",
                                            new String[] {
                                                Long.valueOf(newPid).toString()
                                            });
                                }
                                //resolver.insert(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                                        //values);
                            } catch (UnsupportedOperationException ex) {
                                MusicLogUtils.d(TAG,
                                  "OnClickListener() with UnsupportedOperationException:" + ex);
                                 if (mDialog != null) {
                                     mDialog.dismiss();
                                     mDialog = null;
                                 }
                                return;
                            }
                        }
                        intent.putExtra(MusicUtils.SAVE_PLAYLIST_FLAG, MusicUtils.SAVE_AS_PLAYLIST);
                    } else {
                        ContentValues values = new ContentValues(1);
                        values.put(MediaStore.Audio.Playlists.NAME, name);
                        try {
                            uri = resolver.insert(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                                    values);
                        } catch (UnsupportedOperationException ex) {
                            MusicLogUtils.d(TAG,
                              "OnClickListener() with UnsupportedOperationException:" + ex);
                             if (mDialog != null) {
                                 mDialog.dismiss();
                                 mDialog = null;
                             }
                            return;
                        }
                    }
                    if (mPlaylistFlag.equals(MusicUtils.NEW_PLAYLIST)) {
                          if (mStartActivityTab == MusicBrowserActivity.SONG_INDEX) {
                           long [] list = new long[] { Long.parseLong(mSelectItemId) };
                           MusicUtils.addToPlaylist(getActivity().getApplicationContext(),
                                                    list,
                                                    Integer.valueOf(uri.getLastPathSegment()));
                        } else if (mStartActivityTab == MusicBrowserActivity.ALBUM_INDEX) {
                            if (uri != null) {
                                  long [] list = MusicUtils.getSongListForAlbum(
                                              getActivity().getApplicationContext(),
                                              Long.parseLong(mSelectItemId));
                                MusicUtils.addToPlaylist(
                                             getActivity().getApplicationContext(),
                                             list,
                                             Long.parseLong(uri.getLastPathSegment()));
                            }
                        } else if (mStartActivityTab == MusicBrowserActivity.ARTIST_INDEX) {
                                      if (uri != null && mSelectItemId != null) {
                                        long [] list = null;
                                        String selectItemId =
                                        mSelectItemId.substring(mSelectItemId.lastIndexOf("_") + 1);

                                        if (mSelectItemId.startsWith("selectedartist")) {
                                            list = MusicUtils.getSongListForArtist(
                                                        getActivity().getApplicationContext(),
                                                        Long.parseLong(selectItemId));
                                        } else if (mSelectItemId.startsWith("selectedalbum")) {
                                            list = MusicUtils.getSongListForAlbum(
                                                           getActivity().getApplicationContext(),
                                                           Long.parseLong(selectItemId));
                                        }
                                        MusicUtils.addToPlaylist(
                                                   getActivity().getApplicationContext(),
                                                   list,
                                                   Long.parseLong(uri.getLastPathSegment()));
                                    }
                        } else {
                            long [] list = new long[] { Long.parseLong(mSelectItemId) };
                          MusicUtils.addToPlaylist(getActivity().getApplicationContext(),
                                                    list,
                                                    Integer.valueOf(uri.getLastPathSegment()));
                        }
                    } else if (mPlaylistFlag.equals(MusicUtils.SAVE_AS_PLAYLIST)) {
                    long [] list = sTempCursor;
                    int listLength = list.length;
                    if (uri != null) {

                        int plid = Integer.parseInt(uri.getLastPathSegment());
                        MusicUtils.addToPlaylist(getActivity().getApplicationContext(), list, plid);
                    } else if (mPlaylistFlag.equals(MusicUtils.SAVE_AS_PLAYLIST)) {
                        MusicUtils.showCreatePlaylistToast(listLength,
                                            getActivity().getApplicationContext());
                    }
                }

            }

        }


    /**
     * M: Finish create playlist activity when sdcard has been unmounted.
     */
    private BroadcastReceiver mScanListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            /// M: When SD card is unmounted, finish the create playlist activity
             if (mDialog != null) {
                        mDialog.dismiss();
                        mDialog = null;
                    }
            MusicLogUtils.d(TAG, "SD card is ejected, finish CreatePlaylist activity!");
        }
    };

}
