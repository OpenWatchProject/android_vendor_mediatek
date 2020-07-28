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

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.media.AudioManager;
import android.os.Bundle;
import android.provider.MediaStore;
import android.text.Editable;
import android.text.TextWatcher;
//Not required
//import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
//Not required
//import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class RenamePlaylist extends DialogFragment
{
    /// M: add for debug log
    private static final String TAG = "RenamePlaylist";
    /// M: add dialog key
    private static final int ALERT_DIALOG_KEY = 0;
    private EditText mPlaylist;
    //Not Required
    //private TextView mPrompt;
    private Button mSaveButton;
    private long mRenameId;
    private String mOriginalName;
    /// M: dialog, view, prompt and exist id  @{

    private long mExistingId;
    private String mPrompt;
    private Dialog mDialog;
    /// @}
     public static RenamePlaylist newInstance(Boolean single) {
        RenamePlaylist frag = new RenamePlaylist();
        Bundle args = new Bundle();
        frag.setArguments(args);
        return frag;
    }

    public static void setPlaylist(long id) {
        RenamePlaylist frag = new RenamePlaylist();
        Bundle args = new Bundle();
        frag.setArguments(args);

    }
    @Override
 public Dialog onCreateDialog(Bundle savedInstanceState) {

        MusicLogUtils.v(TAG, "onCreate");

        mDialog = new Dialog(getActivity());

        mDialog.setContentView(R.layout.rename_playlist);
        mPlaylist = (EditText) mDialog.findViewById(R.id.playlist);


           // mDialog.setPositiveButton(getResources()
           // .getString(R.string.delete_confirm_button_text));
           // mDialog.setNeutralButton(getResources().getString(R.string.cancel));
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
                String name = mPlaylist.getText().toString();
                if (name != null && name.length() > 0) {
                    name = name.trim();
                    ContentResolver resolver = getActivity().getContentResolver();
                    if (mExistingId >= 0) {
                        /// M: There is another playlist which has the same name with renamed one
                        // we should overwrite existing one, i.e. delete it from database
                        resolver.delete(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                                MediaStore.Audio.Playlists._ID + "=?",
                                new String[] {Long.valueOf(mExistingId).toString()});
                        MusicLogUtils.v(TAG, "to overwrite, delete the existing one");
                    }
                    String oldData = dataForId(mRenameId);
                    if (oldData == null) {
                        MusicLogUtils.v(TAG, "oldData is null");
                        return;
                    }
                    String newData = oldData.replace(
                            oldData.substring(oldData.lastIndexOf("/") + 1), name);
                    MusicLogUtils.v(TAG, "oldData:" + oldData + ",mRenameId:" + mRenameId
                            + ",name:" + name + ",newData:" + newData);
                    ContentValues values = new ContentValues(1);
                    values.put(MediaStore.Audio.Playlists.NAME, name);
                    values.put(MediaStore.Audio.Playlists.DATA, newData);
                    resolver.update(MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                            values,
                            MediaStore.Audio.Playlists._ID + "=?",
                            new String[] { Long.valueOf(mRenameId).toString()});
                    Toast.makeText(getActivity().getApplicationContext(),
                     R.string.playlist_renamed_message, Toast.LENGTH_SHORT).show();

                }
                if (mDialog != null) {
                        mDialog.dismiss();
                        mDialog = null;
                    }
                }
        });
            mDialog.setCanceledOnTouchOutside(true);
            mDialog.setCancelable(true);

            if (savedInstanceState == null)
            {
                mRenameId =
                 MusicUtils.getLongPref(getActivity().getApplicationContext(), "rename", 1);
            }
        /// M: get exist id to restore when activity is restart again

        mOriginalName = nameForId(mRenameId);
        String defaultname = savedInstanceState !=
                       null ? savedInstanceState.getString("defaultname") : mOriginalName;

        if (mRenameId < 0 || mOriginalName == null || defaultname == null) {
            MusicLogUtils.v(TAG, "Rename failed: " + mRenameId + "/" + defaultname);
            mPlaylist.setText(defaultname);
            mPlaylist.setSelection(defaultname.length());
            mPlaylist.addTextChangedListener(mTextWatcher);
            return mDialog;
        }

        String promptformat;
        if (mOriginalName.equals(defaultname)) {
            promptformat = getString(R.string.rename_playlist_same_prompt);
        } else {
            promptformat = getString(R.string.rename_playlist_diff_prompt);
        }
        /// M: get prompt string
        mPrompt = String.format(promptformat, mOriginalName, defaultname);
        mPlaylist.setText(defaultname);
        mPlaylist.setSelection(defaultname.length());
        mPlaylist.addTextChangedListener(mTextWatcher);
        mDialog.setTitle(mPrompt);
        return mDialog;
        /// @}


    }



    TextWatcher mTextWatcher = new TextWatcher() {
        public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            // don't care about this one
        }
        public void onTextChanged(CharSequence s, int start, int before, int count) {
            // check if playlist with current name exists already, and warn the user if so.
            setSaveButton();
        };
        public void afterTextChanged(Editable s) {
            // don't care about this one
        }
    };

    private void setSaveButton() {
        String typedname = mPlaylist.getText().toString().trim();
        MusicLogUtils.v(TAG, "setSaveButton " + mSaveButton);
        /// M: check and retrieve save button @{
        if (mSaveButton == null) {
            if (mDialog == null) {
                return;
            } else {
                mSaveButton = (Button) mDialog.findViewById(R.id.rename_done);
            }
        }
        /// @}
        /// M: check save button again
        if (mSaveButton != null) {
            if (typedname.length() == 0) {
                mSaveButton.setEnabled(false);
            } else {
                mSaveButton.setEnabled(true);
                /// M: record the exist id  @{
                final long id = MusicUtils.idForplaylist(getActivity().getApplicationContext(),
                                                         typedname);
                MusicLogUtils.v(TAG, "id " + id + ",mOriginalName:" +
                 mOriginalName + ",typedname:" + typedname);
                if (id >= 0 && !mOriginalName.equals(typedname)
                        && !mOriginalName.equalsIgnoreCase(typedname)) {
                    mSaveButton.setText(R.string.create_playlist_overwrite_text);
                    mExistingId = id;
                } else {
                    mSaveButton.setText(R.string.create_playlist_create_text);
                    mExistingId = -1;
                }
                /// @}
            }
        }

    }

    private String nameForId(long id) {
        Cursor c = MusicUtils.query(getActivity().getApplicationContext(),
                              MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists.NAME },
                MediaStore.Audio.Playlists._ID + "=?",
                new String[] { Long.valueOf(id).toString() },
                MediaStore.Audio.Playlists.NAME);
        String name = null;
        if (c != null) {
            c.moveToFirst();
            if (!c.isAfterLast()) {
                name = c.getString(0);
            }
            /// M: close cursor if the cursor is not null.
            c.close();
        }
        return name;
    }

    private String dataForId(long id) {
        Cursor c = MusicUtils.query(getActivity().getApplicationContext(),
                                MediaStore.Audio.Playlists.EXTERNAL_CONTENT_URI,
                new String[] { MediaStore.Audio.Playlists.DATA },
                MediaStore.Audio.Playlists._ID + "=?",
                new String[] { Long.valueOf(id).toString() },
                MediaStore.Audio.Playlists.DATA);
        String data = null;
        if (c != null) {
            c.moveToFirst();
            if (!c.isAfterLast()) {
                data = c.getString(0);
            }
            /// M: close cursor if the cursor is not null.
            c.close();
        }
        return data;
    }


    @Override
    public void onSaveInstanceState(Bundle outcicle) {
        outcicle.putString("defaultname", mPlaylist.getText().toString());
        outcicle.putLong("rename", mRenameId);
        /// M: store the exist id
        outcicle.putLong("existing", mExistingId);
    }

    @Override
    public void onResume() {
        super.onResume();
        /// M: Update save button.
        setSaveButton();
    }


    /**
     * M: Unregister a receiver, but eat the exception that is thrown if the
     * receiver was never registered to begin with.
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
       // unregisterReceiver(mScanListener);

    }


    /**
     * M: Finish rename playlist activity when sdcard has been unmounted.
     */
    final private BroadcastReceiver mScanListener = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            /// M: When SD card is unmounted, finish the rename playlist activity
            if (mDialog == null)
            {
                mDialog.dismiss();
              mDialog = null;
            }
            MusicLogUtils.v(TAG, "SD card is ejected, finish RenamePlaylist activity!");
        }
    };
}
