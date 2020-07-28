/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2006 The Android Open Source Project
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

package com.android.browser;

import com.android.browser.addbookmark.FolderSpinner;
import com.android.browser.addbookmark.FolderSpinnerAdapter;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.LoaderManager;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.AsyncTaskLoader;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.CursorLoader;
import android.content.DialogInterface;
import android.content.Loader;
import android.content.pm.ShortcutManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;
import android.net.ParseException;
import android.net.Uri;
import android.net.WebAddress;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import com.android.browser.provider.BrowserContract;
import com.android.browser.provider.BrowserContract.Accounts;
import android.text.InputFilter;
import android.text.Spanned;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CursorAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.android.browser.provider.BrowserProvider2;

/// M: Add for OP customization.
import com.mediatek.browser.ext.IBrowserBookmarkExt;

import java.net.URI;
import java.net.URISyntaxException;
import java.net.URLDecoder;
import java.net.URLEncoder;

public class AddBookmarkPage extends Activity
        implements View.OnClickListener, TextView.OnEditorActionListener,
        AdapterView.OnItemClickListener, LoaderManager.LoaderCallbacks<Cursor>,
        BreadCrumbView.Controller, FolderSpinner.OnSetSelectionListener,
        OnItemSelectedListener {

    public static final long DEFAULT_FOLDER_ID = -1;
    public static final String TOUCH_ICON_URL = "touch_icon_url";
    // Place on an edited bookmark to remove the saved thumbnail
    public static final String REMOVE_THUMBNAIL = "remove_thumbnail";
    public static final String USER_AGENT = "user_agent";
    public static final String CHECK_FOR_DUPE = "check_for_dupe";

    public static final String BOOKMARK_CURRENT_ID = "bookmark_current_id";

    /* package */ static final String EXTRA_EDIT_BOOKMARK = "bookmark";
    /* package */ static final String EXTRA_IS_FOLDER = "is_folder";

    private static final int MAX_CRUMBS_SHOWN = 2;

    private final String LOGTAG = "Bookmarks";
    private static final String XLOGTAG = "browser/AddBookmarkPage";

    // IDs for the CursorLoaders that are used.
    private final int LOADER_ID_ACCOUNTS = 0;
    private final int LOADER_ID_FOLDER_CONTENTS = 1;
    private final int LOADER_ID_EDIT_INFO = 2;

    private EditText    mTitle;
    private EditText    mAddress;
    private EditText    mEdit;
    private TextView    mButton;
    private View        mCancelButton;
    private boolean     mEditingExisting;
    private boolean     mEditingFolder;
    private Bundle      mMap;
    private String      mTouchIconUrl;
    private String      mOriginalUrl;
    private FolderSpinner mFolder;
    private View mDefaultView;
    private View mFolderSelector;
    private EditText mFolderNamer;
    private View mFolderCancel;
    private boolean mIsFolderNamerShowing;
    private View mFolderNamerHolder;
    private View mAddNewFolder;
    private View mAddSeparator;
    private long mCurrentFolder = -1;
    private FolderAdapter mAdapter;
    private BreadCrumbView mCrumbs;
    private TextView mFakeTitle;
    private View mCrumbHolder;
    private CustomListView mListView;
    private boolean mSaveToHomeScreen = false;
    private long mRootFolder;
    private TextView mTopLevelLabel;
    private Drawable mHeaderIcon;
    private View mRemoveLink;
    private View mFakeTitleHolder;
    private FolderSpinnerAdapter mFolderAdapter;
    private Spinner mAccountSpinner;
    private ArrayAdapter<BookmarkAccount> mAccountAdapter;
    private long mOverwriteBookmarkId = -1;
    private long mCurrentId;
    private static final int INIT_ID = -2;
    private int mRestoreFolder = INIT_ID;
    private boolean mSupportShortcut = true;

    /// M: Add for OP customization.
    private IBrowserBookmarkExt mBookmarkExt;

    private static class Folder {
        String Name;
        long Id;
        Folder(String name, long id) {
            Name = name;
            Id = id;
        }
    }

    // Message IDs
    private static final int SAVE_BOOKMARK = 100;
    private static final int TOUCH_ICON_DOWNLOADED = 101;
    private static final int BOOKMARK_DELETED = 102;

    private Handler mHandler;

    private AlertDialog mWarningDialog;

    private InputMethodManager getInputMethodManager() {
        return (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
    }

    private Uri getUriForFolder(long folder) {
        BookmarkAccount account =
                (BookmarkAccount) mAccountSpinner.getSelectedItem();
        if (folder == mRootFolder && account != null) {
            return BookmarksLoader.addAccount(
                    BrowserContract.Bookmarks.CONTENT_URI_DEFAULT_FOLDER,
                    account.accountType, account.accountName);
        }
        return BrowserContract.Bookmarks.buildFolderUri(folder);
    }

    /**
     * M: add for feature bookmarks folder.
     */
    public static long getIdFromData(Object data) {
        if (data == null) {
            return BrowserProvider2.FIXED_ID_ROOT;
        } else {
            Folder folder = (Folder) data;
            return folder.Id;
        }
    }

    @Override
    public void onTop(BreadCrumbView view, int level, Object data) {
        if (null == data) return;
        Folder folderData = (Folder) data;
        long folder = folderData.Id;
        LoaderManager manager = getLoaderManager();
        CursorLoader loader = (CursorLoader) ((Loader<?>) manager.getLoader(
                LOADER_ID_FOLDER_CONTENTS));
        loader.setUri(getUriForFolder(folder));
        loader.forceLoad();
        if (mIsFolderNamerShowing) {
            completeOrCancelFolderNaming(true);
        }
        setShowBookmarkIcon(level == 1);
    }

    /**
     * Show or hide the icon for bookmarks next to "Bookmarks" in the crumb view.
     * @param show True if the icon should visible, false otherwise.
     */
    private void setShowBookmarkIcon(boolean show) {
        Drawable drawable = show ? mHeaderIcon: null;
        mTopLevelLabel.setCompoundDrawablesWithIntrinsicBounds(drawable, null, null, null);
    }

    @Override
    public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
        if (v == mFolderNamer) {
            if (v.getText().length() > 0) {
                if (actionId == EditorInfo.IME_ACTION_DONE || actionId == EditorInfo.IME_NULL) {
                    hideSoftInput();
                }
            }
            // Steal the key press; otherwise a newline will be added
            return true;
        }
        return false;
    }

    /**
     * M: add for bug fix, rotation screen.
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // TODO Auto-generated method stub
        super.onConfigurationChanged(newConfig);

    }

    /**
     * M: hide IME.
     */
    private void hideSoftInput() {
        Log.d(XLOGTAG, "hideSoftInput");
        getInputMethodManager().hideSoftInputFromWindow(
                mListView.getWindowToken(), 0);
    }

    private void switchToDefaultView(boolean changedFolder) {
        mFolderSelector.setVisibility(View.GONE);
        mDefaultView.setVisibility(View.VISIBLE);
        mCrumbHolder.setVisibility(View.GONE);
        mFakeTitleHolder.setVisibility(View.VISIBLE);
        if (changedFolder) {
            Object data = mCrumbs.getTopData();
            if (data != null) {
                Folder folder = (Folder) data;
                mCurrentFolder = folder.Id;
                if (mCurrentFolder == mRootFolder) {
                    // The Spinner changed to show "Other folder ..."  Change
                    // it back to "Bookmarks", which is position 0 if we are
                    // editing a folder, 1 otherwise.
                    mFolder.setSelectionIgnoringSelectionChange(
                            mEditingFolder || !mSupportShortcut ? 0 : 1);
                } else {
                    mFolderAdapter.setOtherFolderDisplayText(folder.Name);
                }
            }
        } else {
            // The user canceled selecting a folder.  Revert back to the earlier
            // selection.
            if (mSaveToHomeScreen) {
                mFolder.setSelectionIgnoringSelectionChange(0);
            } else {
                if (mCurrentFolder == mRootFolder) {
                    mFolder.setSelectionIgnoringSelectionChange(
                            mEditingFolder || !mSupportShortcut ? 0 : 1);
                } else {
                    Object data = mCrumbs.getTopData();
                    if (data != null && ((Folder) data).Id == mCurrentFolder) {
                        // We are showing the correct folder hierarchy. The
                        // folder selector will say "Other folder..."  Change it
                        // to say the name of the folder once again.
                        mFolderAdapter.setOtherFolderDisplayText(((Folder) data).Name);
                    } else {
                        // We are not showing the correct folder hierarchy.
                        // Clear the Crumbs and find the proper folder
                        setupTopCrumb();
                        LoaderManager manager = getLoaderManager();
                        manager.restartLoader(LOADER_ID_FOLDER_CONTENTS, null, this);

                    }
                }
            }
        }
    }

    @Override
    public void onClick(View v) {
        if (v == mButton) {
            if (mFolderSelector.getVisibility() == View.VISIBLE) {
                // We are showing the folder selector.
                if (mIsFolderNamerShowing) {
                    completeOrCancelFolderNaming(false);
                } else {
                    // User has selected a folder.  Go back to the opening page
                    mSaveToHomeScreen = false;
                    switchToDefaultView(true);
                }
            } else {
                /// M: add for feature bookmarks folder
                mOverwriteBookmarkId = -1;
                if (save()) {
                    finish();
                }
            }
        } else if (v == mCancelButton) {
            if (mIsFolderNamerShowing) {
                completeOrCancelFolderNaming(true);
            } else if (mFolderSelector.getVisibility() == View.VISIBLE) {
                switchToDefaultView(false);
            } else {
                finish();
            }
        } else if (v == mFolderCancel) {
            completeOrCancelFolderNaming(true);
        } else if (v == mAddNewFolder) {
            setShowFolderNamer(true);
            mFolderNamer.setText(R.string.new_folder);
            mFolderNamer.setSelection(mFolderNamer.length());
            mFolderNamer.requestFocus();
            mAddNewFolder.setVisibility(View.GONE);
            mAddSeparator.setVisibility(View.GONE);
            InputMethodManager imm = getInputMethodManager();
            // Set the InputMethodManager to focus on the ListView so that it
            // can transfer the focus to mFolderNamer.
            imm.focusIn(mListView);
            imm.showSoftInput(mFolderNamer, InputMethodManager.SHOW_IMPLICIT);
        } else if (v == mRemoveLink) {
            if (!mEditingExisting) {
                throw new AssertionError("Remove button should not be shown for"
                        + " new bookmarks");
            }
            long id = mMap.getLong(BrowserContract.Bookmarks._ID);
            createHandler();
            Message msg = Message.obtain(mHandler, BOOKMARK_DELETED);
            /// M: for bookmark folder, delete bookmarks in this folder @ {
            if (mEditingFolder) {
                BookmarkUtils.displayRemoveFolderDialog(id,
                        mTitle.getText().toString(), this, msg);
            } else {
                BookmarkUtils.displayRemoveBookmarkDialog(id,
                        mTitle.getText().toString(), this, msg);
            }
            /// @ }
        }
    }

    /**
     * M: add for feature bookmarks folder.
     */
    private int haveToOverwriteBookmarkId(String title, String url, long parent) {
        if (!mSaveToHomeScreen && !mEditingFolder) {
            Log.d(XLOGTAG, "Add bookmark page haveToOverwriteBookmarkId mCurrentId:" + mCurrentId);
            return Bookmarks.getIdByNameOrUrl(getContentResolver(), title, url, parent, mCurrentId);
        } else {
            return -1;
        }
    }

    /**
     * M: if folder exist show toast.
     */
    private void displayToastForExistingFolder() {
        Toast.makeText(getApplicationContext(), R.string.duplicated_folder_warning,
                Toast.LENGTH_LONG).show();
    }

    /**
     * M: Does folder name exist in folder who's id is parentId.
     */
    private boolean isFolderExist(long parentId, String title) {
        android.util.Log.e(XLOGTAG, "BrowserProvider2.isValidAccountName parentId:"
                + parentId + " title:" + title);
        if (parentId <= 0 || title == null || title.length() == 0) {
            return false;
        }
        Uri uri = BrowserContract.Bookmarks.CONTENT_URI;
        Cursor cursor = null;
        boolean exist = false;
        try {
            cursor = getApplicationContext().getContentResolver().query(uri,
                    new String[] { BrowserContract.Bookmarks._ID },
                    BrowserContract.Bookmarks.PARENT + " = ? AND " +
                    BrowserContract.Bookmarks.IS_DELETED + " = ? AND " +
                    BrowserContract.Bookmarks.IS_FOLDER + " = ? AND " +
                    BrowserContract.Bookmarks.TITLE + " = ?",
                    new String[] { parentId + "", 0 + "", 1 + "", title },
                    null);
            exist = !(cursor == null || cursor.getCount() == 0);
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return exist;
    }

    /**
     * M: display alert dialog for existing bookmark.
     */
    private void displayAlertDialogForExistingBookmark() {
        // Put up a alert dialog to tell user the bookmark have name or url already exists.
        new AlertDialog.Builder(this)
                .setTitle(R.string.duplicated_bookmark)
                .setIcon(android.R.drawable.ic_dialog_alert)
                .setMessage(AddBookmarkPage.this
                               .getText(R.string.duplicated_bookmark_warning).toString())
                .setPositiveButton(R.string.ok, mAlertDlgOk)
                .setNegativeButton(R.string.cancel, null)
                .show();
    }

    /**
     * M: alert dialog click listener.
     */
    private DialogInterface.OnClickListener mAlertDlgOk = new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface v, int which) {
            if (save()) {
                finish();
            }
        }
    };

    // FolderSpinner.OnSetSelectionListener

    @Override
    public void onSetSelection(long id) {
        int intId = (int) id;
        switch (intId) {
            case FolderSpinnerAdapter.ROOT_FOLDER:
                mCurrentFolder = mRootFolder;
                mSaveToHomeScreen = false;
                break;
            case FolderSpinnerAdapter.HOME_SCREEN:
                // Create a short cut to the home screen
                mSaveToHomeScreen = true;
                break;
            case FolderSpinnerAdapter.OTHER_FOLDER:
                switchToFolderSelector();
                break;
            case FolderSpinnerAdapter.RECENT_FOLDER:
                mCurrentFolder = mFolderAdapter.recentFolderId();
                mSaveToHomeScreen = false;
                // In case the user decides to select OTHER_FOLDER
                // and choose a different one, so that we will start from
                // the correct place.
                LoaderManager manager = getLoaderManager();
                manager.restartLoader(LOADER_ID_FOLDER_CONTENTS, null, this);
                break;
            default:
                break;
        }
    }

    /**
     * Finish naming a folder, and close the IME
     * @param cancel If true, the new folder is not created.  If false, the new
     *      folder is created and the user is taken inside it.
     */
    private void completeOrCancelFolderNaming(boolean cancel) {
        getInputMethodManager().hideSoftInputFromWindow(
            mListView.getWindowToken(), 0);
        if (!cancel && !TextUtils.isEmpty(mFolderNamer.getText())
                && !TextUtils.isEmpty(mFolderNamer.getText().toString().trim())) {
            String name = mFolderNamer.getText().toString();
            long id = addFolderToCurrent(mFolderNamer.getText().toString());
            descendInto(name, id);
        }
        setShowFolderNamer(false);
        /// M: Modify for OP customization. @{
        // mAddNewFolder.setVisibility(View.VISIBLE);
        // mAddSeparator.setVisibility(View.VISIBLE);
        mBookmarkExt.showCustomizedEditFolderNewFolderView(
                mAddNewFolder, mAddSeparator, mMap);
        /// @}
    }

    private long addFolderToCurrent(String name) {
        // Add the folder to the database
        ContentValues values = new ContentValues();
        values.put(BrowserContract.Bookmarks.TITLE,
                name);
        values.put(BrowserContract.Bookmarks.IS_FOLDER, 1);
        long currentFolder;
        Object data = null;
        if (null != mCrumbs) {
            data = mCrumbs.getTopData();
        }
        if (data != null) {
            currentFolder = ((Folder) data).Id;
        } else {
            currentFolder = mRootFolder;
        }
        values.put(BrowserContract.Bookmarks.PARENT, currentFolder);
        Uri uri = getContentResolver().insert(
                BrowserContract.Bookmarks.CONTENT_URI, values);
        if (uri != null) {
            return ContentUris.parseId(uri);
        } else {
            return -1;
        }
    }

    /**
     * M: add for feature bookmarks foler, add foler to root.
     */
    public static long addFolderToRoot(Context context, String name) {
        // Add the folder whose parent is root to the database
        ContentValues values = new ContentValues();
        values.put(BrowserContract.Bookmarks.TITLE,
                name);
        values.put(BrowserContract.Bookmarks.IS_FOLDER, 1);
        values.put(BrowserContract.Bookmarks.PARENT, BrowserProvider2.FIXED_ID_ROOT);
        Uri uri = context.getContentResolver().insert(
                BrowserContract.Bookmarks.CONTENT_URI, values);
        if (uri != null) {
            return ContentUris.parseId(uri);
        } else {
            // folder already exist, return folder id.
            return getIdFromName(context, name);
        }
    }

    /**
     * M: if folder already exist, return folder id.
     */
    private static long getIdFromName(Context context, String name) {
        long id = -1;
        Cursor cursor = null;
        try {
            cursor = context.getContentResolver().query(
                    BrowserContract.Bookmarks.CONTENT_URI,
                    new String[] {
                        BrowserContract.Bookmarks._ID
                    },
                    BrowserContract.Bookmarks.TITLE + " = ? AND "
                            + BrowserContract.Bookmarks.IS_DELETED + " = ? AND "
                            + BrowserContract.Bookmarks.IS_FOLDER + " = ? AND "
                            + BrowserContract.Bookmarks.PARENT + " = ?", new String[] {
                            name, 0 + "", 1 + "", BrowserProvider2.FIXED_ID_ROOT + ""
                    }, null);
            if (cursor != null && cursor.getCount() != 0) {
                while (cursor.moveToNext()) {
                    id = cursor.getLong(0);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return id;
    }

    private void switchToFolderSelector() {
        // Set the list to the top in case it is scrolled.
        mListView.setSelection(0);
        mDefaultView.setVisibility(View.GONE);
        mFolderSelector.setVisibility(View.VISIBLE);
        mCrumbHolder.setVisibility(View.VISIBLE);
        mFakeTitleHolder.setVisibility(View.GONE);
        /// M: Modify for OP customization. @{
        // mAddNewFolder.setVisibility(View.VISIBLE);
        // mAddSeparator.setVisibility(View.VISIBLE);
        mBookmarkExt.showCustomizedEditFolderNewFolderView(
                mAddNewFolder, mAddSeparator, mMap);
        /// @}
        getInputMethodManager().hideSoftInputFromWindow(
                mListView.getWindowToken(), 0);
    }

    private void descendInto(String foldername, long id) {
        if (id != DEFAULT_FOLDER_ID) {
            mCrumbs.pushView(foldername, new Folder(foldername, id));
            mCrumbs.notifyController();
        } else {
            /// M: show toast when the folder has been exist.
            Toast.makeText(getApplicationContext(),
                R.string.duplicated_folder_warning, Toast.LENGTH_LONG).show();
        }
    }

    private LoaderCallbacks<EditBookmarkInfo> mEditInfoLoaderCallbacks =
            new LoaderCallbacks<EditBookmarkInfo>() {

        @Override
        public void onLoaderReset(Loader<EditBookmarkInfo> loader) {
            // Don't care
        }

        @Override
        public void onLoadFinished(Loader<EditBookmarkInfo> loader,
                EditBookmarkInfo info) {
            boolean setAccount = false;
            if (info.id != -1) {
                mEditingExisting = true;
                showRemoveButton();
                if (mEditingFolder) {
                    mFakeTitle.setText(R.string.edit_folder);
                } else {
                    mFakeTitle.setText(R.string.edit_bookmark);
                }
                /// M: Not set title again.
                //mTitle.setText(info.title);
                mFolderAdapter.setOtherFolderDisplayText(info.parentTitle);
                mMap.putLong(BrowserContract.Bookmarks._ID, info.id);
                setAccount = true;
                setAccount(info.accountName, info.accountType);
                mCurrentFolder = info.parentId;
                onCurrentFolderFound();
                if (mRestoreFolder >= 0) {
                    mFolder.setSelectionIgnoringSelectionChange(mRestoreFolder);
                    mRestoreFolder = INIT_ID;
                }
            }
            // TODO: Detect if lastUsedId is a subfolder of info.id in the
            // editing folder case. For now, just don't show the last used
            // folder at all to prevent any chance of the user adding a parent
            // folder to a child folder
            if (info.lastUsedId != -1 && info.lastUsedId != info.id
                    && !mEditingFolder) {
                if (setAccount && info.lastUsedId != mRootFolder
                        && TextUtils.equals(info.lastUsedAccountName, info.accountName)
                        && TextUtils.equals(info.lastUsedAccountType, info.accountType)) {
                    mFolderAdapter.addRecentFolder(info.lastUsedId, info.lastUsedTitle);
                } else if (!setAccount) {
                    setAccount = true;
                    setAccount(info.lastUsedAccountName, info.lastUsedAccountType);
                    if (info.lastUsedId != mRootFolder) {
                        mFolderAdapter.addRecentFolder(info.lastUsedId,
                                info.lastUsedTitle);
                    }
                    if (mRestoreFolder >= 0) {
                        mFolder.setSelectionIgnoringSelectionChange(mRestoreFolder);
                        mRestoreFolder = INIT_ID;
                    }

                }
            }
            if (!setAccount) {
                mAccountSpinner.setSelection(0);
            }
        }

        @Override
        public Loader<EditBookmarkInfo> onCreateLoader(int id, Bundle args) {
            return new EditBookmarkInfoLoader(AddBookmarkPage.this, mMap);
        }
    };

    void setAccount(String accountName, String accountType) {
        for (int i = 0; i < mAccountAdapter.getCount(); i++) {
            BookmarkAccount account = mAccountAdapter.getItem(i);
            if (TextUtils.equals(account.accountName, accountName)
                    && TextUtils.equals(account.accountType, accountType)) {
                mAccountSpinner.setSelection(i);
                onRootFolderFound(account.rootFolderId);
                return;
            }
        }
    }

    @Override
    public Loader<Cursor> onCreateLoader(int id, Bundle args) {
        String[] projection;
        switch (id) {
            case LOADER_ID_ACCOUNTS:
                return new AccountsLoader(this);
            case LOADER_ID_FOLDER_CONTENTS:
                projection = new String[] {
                        BrowserContract.Bookmarks._ID,
                        BrowserContract.Bookmarks.TITLE,
                        BrowserContract.Bookmarks.IS_FOLDER
                };
                String where = BrowserContract.Bookmarks.IS_FOLDER + " != 0";
                String whereArgs[] = null;
                if (mEditingFolder) {
                    where += " AND " + BrowserContract.Bookmarks._ID + " != ?";
                    whereArgs = new String[] { Long.toString(mMap.getLong(
                            BrowserContract.Bookmarks._ID)) };
                }
                long currentFolder;
                Object data = mCrumbs.getTopData();
                if (data != null) {
                    currentFolder = ((Folder) data).Id;
                } else {
                    currentFolder = mRootFolder;
                }
                return new CursorLoader(this,
                        getUriForFolder(currentFolder),
                        projection,
                        where,
                        whereArgs,
                        BrowserContract.Bookmarks._ID + " ASC");
            default:
                throw new AssertionError("Asking for nonexistant loader!");
        }
    }

    @Override
    public void onLoadFinished(Loader<Cursor> loader, Cursor cursor) {
        switch (loader.getId()) {
            case LOADER_ID_ACCOUNTS:
                mAccountAdapter.clear();
                while (cursor.moveToNext()) {
                    mAccountAdapter.add(new BookmarkAccount(this, cursor));
                }
                getLoaderManager().destroyLoader(LOADER_ID_ACCOUNTS);
                getLoaderManager().restartLoader(LOADER_ID_EDIT_INFO, null,
                        mEditInfoLoaderCallbacks);
                break;
            case LOADER_ID_FOLDER_CONTENTS:
                mAdapter.changeCursor(cursor);
                break;
        }
    }

    public void onLoaderReset(Loader<Cursor> loader) {
        switch (loader.getId()) {
            case LOADER_ID_FOLDER_CONTENTS:
                mAdapter.changeCursor(null);
                break;
        }
    }

    /**
     * Move cursor to the position that has folderToFind as its "_id".
     * @param cursor Cursor containing folders in the bookmarks database
     * @param folderToFind "_id" of the folder to move to.
     * @param idIndex Index in cursor of "_id"
     * @throws AssertionError if cursor is empty or there is no row with folderToFind
     *      as its "_id".
     */
    void moveCursorToFolder(Cursor cursor, long folderToFind, int idIndex)
            throws AssertionError {
        if (!cursor.moveToFirst()) {
            throw new AssertionError("No folders in the database!");
        }
        long folder;
        do {
            folder = cursor.getLong(idIndex);
        } while (folder != folderToFind && cursor.moveToNext());
        if (cursor.isAfterLast()) {
            throw new AssertionError("Folder(id=" + folderToFind
                    + ") holding this bookmark does not exist!");
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        TextView tv = (TextView) view.findViewById(android.R.id.text1);
        // Switch to the folder that was clicked on.
        descendInto(tv.getText().toString(), id);
    }

    private void setShowFolderNamer(boolean show) {
        if (show != mIsFolderNamerShowing) {
            mIsFolderNamerShowing = show;
            if (show) {
                // Set the selection to the folder namer so it will be in
                // view.
                mListView.addFooterView(mFolderNamerHolder);
            } else {
                mListView.removeFooterView(mFolderNamerHolder);
            }
            // Refresh the list.
            mListView.setAdapter(mAdapter);
            if (show) {
                mListView.setSelection(mListView.getCount() - 1);
            }
        }
    }

    /**
     * Shows a list of names of folders.
     */
    private class FolderAdapter extends CursorAdapter {
        public FolderAdapter(Context context) {
            super(context, null);
        }

        @Override
        public void bindView(View view, Context context, Cursor cursor) {
            ((TextView) view.findViewById(android.R.id.text1)).setText(
                    cursor.getString(cursor.getColumnIndexOrThrow(
                    BrowserContract.Bookmarks.TITLE)));
        }

        @Override
        public View newView(Context context, Cursor cursor, ViewGroup parent) {
            View view = LayoutInflater.from(context).inflate(
                    R.layout.folder_list_item, null);
            return view;
        }

        @Override
        public boolean isEmpty() {
            // Do not show the empty view if the user is creating a new folder.
            return super.isEmpty() && !mIsFolderNamerShowing;
        }
    }
    /// M: reset EditText error message @ {
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        if (mTitle != null && mTitle.getError() != null) {
            outState.putBoolean("titleHasError", true);
        }
        if (mAddress != null && mAddress.getError() != null) {
            outState.putBoolean("addrHasError", true);
        }

    }
    @Override
    protected void onRestoreInstanceState(Bundle inState) {
        super.onRestoreInstanceState(inState);
        Resources r = getResources();
        if (inState != null && inState.getBoolean("titleHasError") &&
                mTitle != null && mTitle.getText().toString().trim().length() == 0) {
            if (mEditingFolder) {
                mTitle.setError(r.getText(R.string.folder_needs_title));
            } else {
                mTitle.setError(r.getText(R.string.bookmark_needs_title));
            }
        }
        if (inState != null && inState.getBoolean("addrHasError") &&
                mAddress != null && mAddress.getText().toString().trim().length() == 0) {
            mAddress.setError(r.getText(R.string.bookmark_needs_url));
        }

    }
    /// @ }
    @Override
    protected void onPause() {
        super.onPause();
        mRestoreFolder = mFolder.getSelectedItemPosition();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    private InputFilter[] generateInputFilter(final int nLimit) {
        InputFilter[] contentFilters = new InputFilter[1];
        contentFilters[0] = new InputFilter.LengthFilter(nLimit) {
            public CharSequence filter(CharSequence source, int start, int end,
                    Spanned dest, int dstart, int dend) {

                int keep = nLimit - (dest.length() - (dend - dstart));
                if (keep <= 0) {
                    showWarningDialog();
                    return "";
                } else if (keep >= end - start) {
                    return null;
                } else {
                      if (keep < source.length()) {
                          showWarningDialog();
                      }
                      return source.subSequence(start, start + keep);
                }
            }
        };
        return contentFilters;
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        mMap = getIntent().getExtras();

        setContentView(R.layout.browser_add_bookmark);

        Window window = getWindow();

        /// M: fix tablet bug ALPS00440419 @{
        if (BrowserActivity.isTablet(this)) {
            window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        }
        /// @}

        /// M: Add for OP customization.
        mBookmarkExt = Extensions.getBookmarkPlugin(getApplicationContext());
        ShortcutManager shortcutMgr = getSystemService(ShortcutManager.class);
        mSupportShortcut = shortcutMgr.isRequestPinShortcutSupported();

        String title = null;
        String url = null;

        mFakeTitle = (TextView) findViewById(R.id.fake_title);

        if (mMap != null) {
            Bundle b = mMap.getBundle(EXTRA_EDIT_BOOKMARK);
            if (b != null) {
                mEditingFolder = mMap.getBoolean(EXTRA_IS_FOLDER, false);
                mMap = b;
                /// M: Modify for OP customization. @{
                // mEditingExisting = true;
                mEditingExisting = mBookmarkExt.customizeEditExistingFolderState(mMap, true);
                /// @}
                /// M: get the current bookmark ID.
                mCurrentId = mMap.getLong(BrowserContract.Bookmarks._ID, -1);
                Log.d(XLOGTAG, "Add bookmark page onCreate mCurrentId:" + mCurrentId);
                if (mEditingFolder) {
                    /// M: Modify for OP customization. @{
                    // mFakeTitle.setText(R.string.edit_folder);
                    String titleString = mBookmarkExt.getCustomizedEditFolderFakeTitleString(
                        mMap, getString(R.string.edit_folder));
                    mFakeTitle.setText(titleString);
                    /// @}
                    findViewById(R.id.row_address).setVisibility(View.GONE);
                } else {
                    mFakeTitle.setText(R.string.edit_bookmark);
                    showRemoveButton();
                }
            } else {
                int gravity = mMap.getInt("gravity", -1);
                if (gravity != -1) {
                    WindowManager.LayoutParams l = window.getAttributes();
                    l.gravity = gravity;
                    window.setAttributes(l);
                }
            }
            title = mMap.getString(BrowserContract.Bookmarks.TITLE);
            url = mOriginalUrl = mMap.getString(BrowserContract.Bookmarks.URL);
            mTouchIconUrl = mMap.getString(TOUCH_ICON_URL);
            /// M: For CR: 368648 and CR:384089. change the folder to the current folder been opened
            mCurrentFolder = mMap.getLong(BrowserContract.Bookmarks.PARENT, DEFAULT_FOLDER_ID);
            Log.i(LOGTAG, "CurrentFolderId: " + mCurrentFolder);
        }

        mWarningDialog = new AlertDialog.Builder(this).create();

        mTitle = (EditText) findViewById(R.id.title);
        /// M: fix bug ALPS00249872 @{
        final int nLimit = this.getResources().getInteger(R.integer.bookmark_title_maxlength);
        mTitle.setFilters(generateInputFilter(nLimit));
        /// @}
        mTitle.setText(title);
        if (title != null) {
            /// M: Fix bug ALPS02344748 @{
            mTitle.setSelection(mTitle.getText().length());
            ///@}
        }
        mAddress = (EditText) findViewById(R.id.address);
        /// M: Add for OP customization. @{
        Context context = getApplicationContext();
        InputFilter[] addressFilters =
            Extensions.getUrlPlugin(context).checkUrlLengthLimit(context);
        if (addressFilters != null) {
            mAddress.setFilters(addressFilters);
        }
        /// @}
        mAddress.setText(url);

        mButton = (TextView) findViewById(R.id.OK);
        mButton.setOnClickListener(this);

        mCancelButton = findViewById(R.id.cancel);
        mCancelButton.setOnClickListener(this);

        mFolder = (FolderSpinner) findViewById(R.id.folder);
        mFolderAdapter = new FolderSpinnerAdapter(this, !mEditingFolder && mSupportShortcut);
        mFolder.setAdapter(mFolderAdapter);
        mFolder.setOnSetSelectionListener(this);
        /// M: Set the original position. @{
        if (mCurrentFolder != -1 && mCurrentFolder != 1) {
            mFolder.setSelectionIgnoringSelectionChange(
                    mEditingFolder || !mSupportShortcut ? 1 : 2);
            mFolderAdapter.setOtherFolderDisplayText(getNameFromId(mCurrentFolder));
        }
        else {
            mFolder.setSelectionIgnoringSelectionChange(
                    mEditingFolder || !mSupportShortcut ? 0 : 1);
        }
        /// @}

        mDefaultView = findViewById(R.id.default_view);
        mFolderSelector = findViewById(R.id.folder_selector);

        mFolderNamerHolder = getLayoutInflater().inflate(R.layout.new_folder_layout, null);
        mFolderNamer = (EditText) mFolderNamerHolder.findViewById(R.id.folder_namer);
        /// M: fix bug ALPS00267462 @{
        final int limit = this.getResources().getInteger(R.integer.bookmark_title_maxlength);
        mFolderNamer.setFilters(generateInputFilter(limit));
        /// @}

        mFolderNamer.setOnEditorActionListener(this);
        mFolderCancel = mFolderNamerHolder.findViewById(R.id.close);
        mFolderCancel.setOnClickListener(this);

        mAddNewFolder = findViewById(R.id.add_new_folder);
        mAddNewFolder.setOnClickListener(this);
        mAddSeparator = findViewById(R.id.add_divider);

        /// M: Add for OP customization. @{
        mBookmarkExt.showCustomizedEditFolderNewFolderView(
                mAddNewFolder, mAddSeparator, mMap);
        /// @}

        mCrumbs = (BreadCrumbView) findViewById(R.id.crumbs);
        mCrumbs.setUseBackButton(true);
        mCrumbs.setController(this);
        mHeaderIcon = getResources().getDrawable(R.drawable.ic_folder_holo_dark);
        mCrumbHolder = findViewById(R.id.crumb_holder);
        mCrumbs.setMaxVisible(MAX_CRUMBS_SHOWN);

        mAdapter = new FolderAdapter(this);
        mListView = (CustomListView) findViewById(R.id.list);
        View empty = findViewById(R.id.empty);
        mListView.setEmptyView(empty);
        mListView.setAdapter(mAdapter);
        mListView.setOnItemClickListener(this);
        mListView.addEditText(mFolderNamer);

        mAccountAdapter = new ArrayAdapter<BookmarkAccount>(this,
                android.R.layout.simple_spinner_item);
        mAccountAdapter.setDropDownViewResource(
                android.R.layout.simple_spinner_dropdown_item);
        mAccountSpinner = (Spinner) findViewById(R.id.accounts);
        mAccountSpinner.setAdapter(mAccountAdapter);
        mAccountSpinner.setOnItemSelectedListener(this);


        mFakeTitleHolder = findViewById(R.id.title_holder);

        if (!window.getDecorView().isInTouchMode()) {
            mButton.requestFocus();
        }

        getLoaderManager().restartLoader(LOADER_ID_ACCOUNTS, null, this);
    }

    /// M: Get folder title from folder id. @{
    private String getNameFromId(long mCurrentFolder2) {
        String title = "";
        Cursor cursor = null;
        try {
            cursor = getApplicationContext().getContentResolver().query(
                        BrowserContract.Bookmarks.CONTENT_URI,
                        new String[] {
                            BrowserContract.Bookmarks.TITLE
                        },
                        BrowserContract.Bookmarks._ID + " = ? AND "
                            + BrowserContract.Bookmarks.IS_DELETED + " = ? AND "
                            + BrowserContract.Bookmarks.IS_FOLDER + " = ? ",
                        new String[] {
                            String.valueOf(mCurrentFolder2), 0 + "", 1 + ""
                        }, null);
            if (cursor != null && cursor.moveToNext()) {
                title = cursor.getString(0);
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        Log.d(XLOGTAG, "title :" + title);
        return title;
    }
    /// @}

    /**
     * M: show warning dialog for max input
     */
    private void showWarningDialog() {
        if (mWarningDialog != null && !mWarningDialog.isShowing()) {
            mWarningDialog.setTitle(R.string.max_input_browser_search_title);
            mWarningDialog.setMessage(getString(R.string.max_input_browser_search));
            mWarningDialog.setButton(getString(R.string.max_input_browser_search_button),
                new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    return;
                }
                });
            mWarningDialog.show();
        }
    }

    private void showRemoveButton() {
        findViewById(R.id.remove_divider).setVisibility(View.VISIBLE);
        mRemoveLink = findViewById(R.id.remove);
        mRemoveLink.setVisibility(View.VISIBLE);
        mRemoveLink.setOnClickListener(this);
    }

    // Called once we have determined which folder is the root folder
    private void onRootFolderFound(long root) {
        mRootFolder = root;
        /// M: For CR: 368648 and CR:384089. change the folder to the current folder been opened
        if (mCurrentFolder == -1 || mEditingExisting == true) {
            mCurrentFolder = mRootFolder;
        }
        setupTopCrumb();
        onCurrentFolderFound();
    }

    private void setupTopCrumb() {
        mCrumbs.clear();
        String name = getString(R.string.bookmarks);
        mTopLevelLabel = (TextView) mCrumbs.pushView(name, false,
                new Folder(name, mRootFolder));
        // To better match the other folders.
        mTopLevelLabel.setCompoundDrawablePadding(6);
    }

    private void onCurrentFolderFound() {
        LoaderManager manager = getLoaderManager();
        /// M: If homescreen has been selected, don't set selection again. @{
        if (!mSaveToHomeScreen) {
            /// M: For CR: 368648 and CR:384089. change the folder to the current folder been opened
            if (mCurrentFolder != -1 && mCurrentFolder != mRootFolder) {
                // Since we're not in the root folder, change the selection to other
                // folder now.  The text will get changed once we select the correct
                // folder.
                mFolder.setSelectionIgnoringSelectionChange(
                        mEditingFolder || !mSupportShortcut ? 1 : 2);
            } else {
                setShowBookmarkIcon(true);
                /// M: Modify for OP customization. @{
                // if (!mEditingFolder) {
                if (mBookmarkExt.shouldSetCustomizedEditFolderSelection(mMap, !mEditingFolder)) {
                    // Initially the "Bookmarks" folder should be showing, rather than
                    // the home screen.  In the editing folder case, home screen is not
                    // an option, so "Bookmarks" folder is already at the top.
                    // mFolder.setSelectionIgnoringSelectionChange(
                    //         FolderSpinnerAdapter.ROOT_FOLDER);
                    mFolder.setSelectionIgnoringSelectionChange(
                            mEditingFolder || !mSupportShortcut ? 0 : 1);
                }
                /// @}
            }
        }
        /// @}
        // Find the contents of the current folder
        manager.restartLoader(LOADER_ID_FOLDER_CONTENTS, null, this);
    }

    /**
     * Runnable to save a bookmark, so it can be performed in its own thread.
     */
    private class SaveBookmarkRunnable implements Runnable {
        // FIXME: This should be an async task.
        private Message mMessage;
        private Context mContext;
        public SaveBookmarkRunnable(Context ctx, Message msg) {
            mContext = ctx.getApplicationContext();
            mMessage = msg;
        }
        public void run() {
            // Unbundle bookmark data.
            Bundle bundle = mMessage.getData();
            String title = bundle.getString(BrowserContract.Bookmarks.TITLE);
            String url = bundle.getString(BrowserContract.Bookmarks.URL);
            boolean invalidateThumbnail = bundle.getBoolean(REMOVE_THUMBNAIL);
            Bitmap thumbnail = invalidateThumbnail ? null
                    : (Bitmap) bundle.getParcelable(BrowserContract.Bookmarks.THUMBNAIL);
            String touchIconUrl = bundle.getString(TOUCH_ICON_URL);

            // Save to the bookmarks DB.
            try {
                final ContentResolver cr = getContentResolver();
                Log.i(LOGTAG, "mCurrentFolder: " + mCurrentFolder);
                Bookmarks.addBookmark(AddBookmarkPage.this, false, url,
                        title, thumbnail, mCurrentFolder);
                if (touchIconUrl != null) {
                    new DownloadTouchIcon(mContext, cr, url).execute(mTouchIconUrl);
                }
                mMessage.arg1 = 1;
            } catch (IllegalStateException e) {
                mMessage.arg1 = 0;
            }
            mMessage.sendToTarget();
        }
    }

    private static class UpdateBookmarkTask extends AsyncTask<ContentValues, Void, Void> {
        Context mContext;
        Long mId;
        long mBookmarkCurrentId;


        public UpdateBookmarkTask(Context context, long id) {
            mContext = context.getApplicationContext();
            mId = id;
        }

        @Override
        protected Void doInBackground(ContentValues... params) {
            if (params.length < 1) {
                throw new IllegalArgumentException("No ContentValues provided!");
            }
            Uri uri = ContentUris.withAppendedId(BookmarkUtils.getBookmarksUri(mContext), mId);
            mContext.getContentResolver().update(
                    uri,
                    params[0], null, null);
            Log.d(XLOGTAG, "UpdateBookmarkTask doInBackground:");
            if (params.length > 1) {
                mBookmarkCurrentId = params[1].getAsLong(BOOKMARK_CURRENT_ID);
            } else {
                mBookmarkCurrentId = -1;
            }
            return null;
        }

        /**
         *  M: Is editing a exist bookmark, so delete current bookmark.
         */
        @Override
        protected void onPostExecute(Void o) {
            Log.d(XLOGTAG, "UpdateBookmarkTask onPostExecute mBookmarkCurrentId:"
                           + mBookmarkCurrentId);
            if (mBookmarkCurrentId > 0) {
                mContext.getContentResolver().delete(BrowserContract.Bookmarks.CONTENT_URI,
                        BrowserContract.Bookmarks._ID + " = ?",
                        new String[] {String.valueOf(mBookmarkCurrentId)});
            }
        }
    }

    private void createHandler() {
        if (mHandler == null) {
            mHandler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case SAVE_BOOKMARK:
                            if (1 == msg.arg1) {
                                Toast.makeText(AddBookmarkPage.this, R.string.bookmark_saved,
                                        Toast.LENGTH_LONG).show();
                            } else {
                                Toast.makeText(AddBookmarkPage.this, R.string.bookmark_not_saved,
                                        Toast.LENGTH_LONG).show();
                            }
                            break;
                        case TOUCH_ICON_DOWNLOADED:
                            Bundle b = msg.getData();
                            BookmarkUtils.createShortcutToHome(
                                    AddBookmarkPage.this,
                                    b.getString(BrowserContract.Bookmarks.URL),
                                    b.getString(BrowserContract.Bookmarks.TITLE),
                                    (Bitmap) b.getParcelable(BrowserContract.Bookmarks.TOUCH_ICON),
                                    (Bitmap) b.getParcelable(BrowserContract.Bookmarks.FAVICON));
                            break;
                        case BOOKMARK_DELETED:
                            finish();
                            break;
                    }
                }
            };
        }
    }

    /**
     * Parse the data entered in the dialog and post a message to update the bookmarks database.
     */
    boolean save() {
        createHandler();

        String title = mTitle.getText().toString().trim();
        String unfilteredUrl;
        unfilteredUrl = UrlUtils.fixUrl(mAddress.getText().toString());

        boolean emptyTitle = title.length() == 0;
        boolean emptyUrl = unfilteredUrl.trim().length() == 0;
        Resources r = getResources();
        if (emptyTitle || (emptyUrl && !mEditingFolder)) {
            if (emptyTitle) {
                if (mEditingFolder) {
                    mTitle.setError(r.getText(R.string.folder_needs_title));
                } else {
                    mTitle.setError(r.getText(R.string.bookmark_needs_title));
                }
            }
            if (emptyUrl) {
                mAddress.setError(r.getText(R.string.bookmark_needs_url));
            }
            return false;

        }

        /// M: Add for OP customization. @{
        Boolean result = mBookmarkExt.saveCustomizedEditFolder(
                getApplicationContext(), title, mCurrentFolder, mMap,
                getString(R.string.duplicated_folder_warning));
        if (result != null) {
            if (result.booleanValue()) {
                setResult(RESULT_OK);
            }
            return result.booleanValue();
        }
        /// @}

        String url = unfilteredUrl.trim();
        if (!mEditingFolder) {
            try {
                // We allow bookmarks with a javascript: scheme, but these will in most cases
                // fail URI parsing, so don't try it if that's the kind of bookmark we have.

                if (!url.toLowerCase().startsWith("javascript:")) {
                    url = URLEncoder.encode(url);
                    URI uriObj = new URI(url);
                    String scheme = uriObj.getScheme();
                    if (!Bookmarks.urlHasAcceptableScheme(unfilteredUrl.trim())) {
                        // If the scheme was non-null, let the user know that we
                        // can't save their bookmark. If it was null, we'll assume
                        // they meant http when we parse it in the WebAddress class.
                        if (scheme != null) {
                            mAddress.setError(r.getText(R.string.bookmark_cannot_save_url));
                            return false;
                        }
                        WebAddress address;
                        try {
                            address = new WebAddress(unfilteredUrl);
                        } catch (ParseException e) {
                            throw new URISyntaxException("", "");
                        }
                        if (address.getHost().length() == 0) {
                            throw new URISyntaxException("", "");
                        }
                        url = address.toString();
                    }
                    url = URLDecoder.decode(url); // url for origin to save as bookmark.
                }
            } catch (URISyntaxException e) {
                mAddress.setError(r.getText(R.string.bookmark_url_not_valid));
                return false;
            } catch (IllegalArgumentException ex) {
                mAddress.setError(r.getText(R.string.bookmark_url_not_valid));
                return false;
            }
        }

        /// M: Is editing a exist bookmark, set flag to delete current bookmark. @{
        boolean urlUnmodified = url.equals(mOriginalUrl);

        if (mOverwriteBookmarkId > 0) {
            ContentValues valuesDeleteItem = new ContentValues();
            valuesDeleteItem.put(BOOKMARK_CURRENT_ID, mCurrentId);

            ContentValues values = new ContentValues();
            values.put(BrowserContract.Bookmarks.TITLE, title);
            values.put(BrowserContract.Bookmarks.PARENT, mCurrentFolder);
            values.put(BrowserContract.Bookmarks.URL, url);
            if (!urlUnmodified) {
                values.putNull(BrowserContract.Bookmarks.THUMBNAIL);
            }
            if (values.size() > 0) {
                new UpdateBookmarkTask(getApplicationContext(),
                                       mOverwriteBookmarkId).execute(values, valuesDeleteItem);
            }
            mOverwriteBookmarkId = -1;
            setResult(RESULT_OK);
            return true;
        } else {
            mOverwriteBookmarkId = haveToOverwriteBookmarkId(title, url, mCurrentFolder);
            if (mOverwriteBookmarkId > 0) {
                displayAlertDialogForExistingBookmark();
                return false;
            }

            if (mEditingExisting && mEditingFolder) {
                Log.d(XLOGTAG, "editing folder save");
                // if edit exist folder, make sure does it changed, if not
                // changed, do not show toast .
                long editId = mMap.getLong(BrowserContract.Bookmarks._ID, -1);
                long beforeParentId = mMap.getLong(BrowserContract.Bookmarks.PARENT,
                        DEFAULT_FOLDER_ID);
                long currentParentId = mCurrentFolder;

                if (beforeParentId == -1) {
                    beforeParentId = mRootFolder;
                }
                if (currentParentId == -1) {
                    currentParentId = mRootFolder;
                }
                if (beforeParentId == currentParentId) {
                    // parent does not changed
                    String beforeEditTitle = getTitleFromId(editId);
                    if (beforeEditTitle != null && beforeEditTitle.equals(title)) {
                        // Does not change title, do not update
                        Log.d(XLOGTAG, "edit folder save, does not change anything");
                        return true;
                    }
                }
            }
            if (mEditingFolder) {
                // if title folder exists, show save failed.
                boolean isExist = isFolderExist(mCurrentFolder, title);
                if (isExist) {
                    displayToastForExistingFolder();
                    return false;
                }
            }
        }
        /// @}

        if (mSaveToHomeScreen) {
            mEditingExisting = false;
        }

        if (mEditingExisting) {
            Long id = mMap.getLong(BrowserContract.Bookmarks._ID);
            ContentValues values = new ContentValues();
            values.put(BrowserContract.Bookmarks.TITLE, title);
            values.put(BrowserContract.Bookmarks.PARENT, mCurrentFolder);
            if (!mEditingFolder) {
                values.put(BrowserContract.Bookmarks.URL, url);
                if (!urlUnmodified) {
                    values.putNull(BrowserContract.Bookmarks.THUMBNAIL);
                }
            }
            if (values.size() > 0) {
                new UpdateBookmarkTask(getApplicationContext(), id)
                    .executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, values);
            }
            setResult(RESULT_OK);
        } else {
            Bitmap thumbnail;
            Bitmap favicon;
            if (urlUnmodified) {
                thumbnail = (Bitmap) mMap.getParcelable(
                        BrowserContract.Bookmarks.THUMBNAIL);
                favicon = (Bitmap) mMap.getParcelable(
                        BrowserContract.Bookmarks.FAVICON);
            } else {
                thumbnail = null;
                favicon = null;
            }

            Bundle bundle = new Bundle();
            bundle.putString(BrowserContract.Bookmarks.TITLE, title);
            bundle.putString(BrowserContract.Bookmarks.URL, url);
            bundle.putParcelable(BrowserContract.Bookmarks.FAVICON, favicon);

            if (mSaveToHomeScreen) {
                if (mTouchIconUrl != null && urlUnmodified) {
                    Message msg = Message.obtain(mHandler,
                            TOUCH_ICON_DOWNLOADED);
                    msg.setData(bundle);
                    DownloadTouchIcon icon = new DownloadTouchIcon(this, msg,
                            mMap.getString(USER_AGENT));
                    icon.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, mTouchIconUrl);
                } else {
                    BookmarkUtils.createShortcutToHome(this, url,
                            title, null /*touchIcon*/, favicon);
                }
            } else {
                bundle.putParcelable(BrowserContract.Bookmarks.THUMBNAIL, thumbnail);
                bundle.putBoolean(REMOVE_THUMBNAIL, !urlUnmodified);
                bundle.putString(TOUCH_ICON_URL, mTouchIconUrl);
                // Post a message to write to the DB.
                Message msg = Message.obtain(mHandler, SAVE_BOOKMARK);
                msg.setData(bundle);
                // Start a new thread so as to not slow down the UI
                Thread t = new Thread(new SaveBookmarkRunnable(getApplicationContext(), msg));
                t.start();
            }
            setResult(RESULT_OK);
            LogTag.logBookmarkAdded(url, "bookmarkview");
        }
        return true;
    }

    /**
     * M: get folder title from given id.
     * @param editId
     * @return title string
     */
    private String getTitleFromId(long editId) {
        Uri uri = BrowserContract.Bookmarks.CONTENT_URI;
        Cursor cursor = null;
        String title = null;
        try {
            cursor = getApplicationContext().getContentResolver().query(
                    uri,
                    new String[] {
                        BrowserContract.Bookmarks.TITLE
                    },
                    BrowserContract.Bookmarks._ID + " = ? AND "
                            + BrowserContract.Bookmarks.IS_DELETED + " = ? AND "
                            + BrowserContract.Bookmarks.IS_FOLDER + " = ?", new String[] {
                            editId + "", 0 + "", 1 + ""
                    }, null);
            if (cursor != null && cursor.getCount() != 0) {
                while (cursor.moveToNext()) {
                    title = cursor.getString(0);
                }
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        return title;
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position,
            long id) {
        if (mAccountSpinner == parent) {
            long root = mAccountAdapter.getItem(position).rootFolderId;
            if (root != mRootFolder) {
                mCurrentFolder = -1;
                onRootFolderFound(root);
                mFolderAdapter.clearRecentFolder();
            }
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // Don't care
    }

    /*
     * Class used as a proxy for the InputMethodManager to get to mFolderNamer
     */
    public static class CustomListView extends ListView {
        private EditText mEditText;

        public void addEditText(EditText editText) {
            mEditText = editText;
        }

        public CustomListView(Context context) {
            super(context);
        }

        public CustomListView(Context context, AttributeSet attrs) {
            super(context, attrs);
        }

        public CustomListView(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, defStyle);
        }

        @Override
        public boolean checkInputConnectionProxy(View view) {
            return view == mEditText;
        }
    }

    static class AccountsLoader extends CursorLoader {

        static final String[] PROJECTION = new String[] {
            Accounts.ACCOUNT_NAME,
            Accounts.ACCOUNT_TYPE,
            Accounts.ROOT_ID,
        };

        static final int COLUMN_INDEX_ACCOUNT_NAME = 0;
        static final int COLUMN_INDEX_ACCOUNT_TYPE = 1;
        static final int COLUMN_INDEX_ROOT_ID = 2;

        public AccountsLoader(Context context) {
            super(context, Accounts.CONTENT_URI, PROJECTION, null, null, null);
        }

    }

    public static class BookmarkAccount {

        private String mLabel;
        String accountName, accountType;
        public long rootFolderId;

        public BookmarkAccount(Context context, Cursor cursor) {
            accountName = cursor.getString(
                    AccountsLoader.COLUMN_INDEX_ACCOUNT_NAME);
            accountType = cursor.getString(
                    AccountsLoader.COLUMN_INDEX_ACCOUNT_TYPE);
            rootFolderId = cursor.getLong(
                    AccountsLoader.COLUMN_INDEX_ROOT_ID);
            mLabel = accountName;
            if (TextUtils.isEmpty(mLabel)) {
                mLabel = context.getString(R.string.local_bookmarks);
            }
        }

        @Override
        public String toString() {
            return mLabel;
        }
    }

    static class EditBookmarkInfo {
        long id = -1;
        long parentId = -1;
        String parentTitle;
        String title;
        String accountName;
        String accountType;

        long lastUsedId = -1;
        String lastUsedTitle;
        String lastUsedAccountName;
        String lastUsedAccountType;
    }

    static class EditBookmarkInfoLoader extends AsyncTaskLoader<EditBookmarkInfo> {

        private Context mContext;
        private Bundle mMap;

        public EditBookmarkInfoLoader(Context context, Bundle bundle) {
            super(context);
            mContext = context.getApplicationContext();
            mMap = bundle;
        }

        @Override
        public EditBookmarkInfo loadInBackground() {
            final ContentResolver cr = mContext.getContentResolver();
            EditBookmarkInfo info = new EditBookmarkInfo();
            Cursor c = null;

            try {
                // First, let's lookup the bookmark (check for dupes, get needed info)
                String url = mMap.getString(BrowserContract.Bookmarks.URL);
                info.id = mMap.getLong(BrowserContract.Bookmarks._ID, -1);
                boolean checkForDupe = mMap.getBoolean(CHECK_FOR_DUPE);
                if (checkForDupe && info.id == -1 && !TextUtils.isEmpty(url)) {
                    c = cr.query(BrowserContract.Bookmarks.CONTENT_URI,
                            new String[] { BrowserContract.Bookmarks._ID},
                            BrowserContract.Bookmarks.URL + "=?",
                            new String[] { url }, null);
                    if (c.getCount() == 1 && c.moveToFirst()) {
                        info.id = c.getLong(0);
                    }
                    c.close();
                }
                if (info.id != -1) {
                    c = cr.query(ContentUris.withAppendedId(
                            BrowserContract.Bookmarks.CONTENT_URI, info.id),
                            new String[] {
                            BrowserContract.Bookmarks.PARENT,
                            BrowserContract.Bookmarks.ACCOUNT_NAME,
                            BrowserContract.Bookmarks.ACCOUNT_TYPE,
                            BrowserContract.Bookmarks.TITLE},
                            null, null, null);
                    if (c.moveToFirst()) {
                        info.parentId = c.getLong(0);
                        info.accountName = c.getString(1);
                        info.accountType = c.getString(2);
                        info.title = c.getString(3);
                    }
                    c.close();
                    c = cr.query(ContentUris.withAppendedId(
                            BrowserContract.Bookmarks.CONTENT_URI, info.parentId),
                            new String[] {
                            BrowserContract.Bookmarks.TITLE,},
                            null, null, null);
                    if (c.moveToFirst()) {
                        info.parentTitle = c.getString(0);
                    }
                    c.close();
                }

                // Figure out the last used folder/account
                c = cr.query(BrowserContract.Bookmarks.CONTENT_URI,
                        new String[] {
                        BrowserContract.Bookmarks.PARENT,
                        }, null, null,
                        BrowserContract.Bookmarks.DATE_MODIFIED + " DESC LIMIT 1");
                if (c.moveToFirst()) {
                    long parent = c.getLong(0);
                    c.close();
                    c = cr.query(BrowserContract.Bookmarks.CONTENT_URI,
                            new String[] {
                            BrowserContract.Bookmarks.TITLE,
                            BrowserContract.Bookmarks.ACCOUNT_NAME,
                            BrowserContract.Bookmarks.ACCOUNT_TYPE},
                            BrowserContract.Bookmarks._ID + "=?", new String[] {
                            Long.toString(parent)}, null);
                    if (c.moveToFirst()) {
                        info.lastUsedId = parent;
                        info.lastUsedTitle = c.getString(0);
                        info.lastUsedAccountName = c.getString(1);
                        info.lastUsedAccountType = c.getString(2);
                    }
                    c.close();
                }
            } finally {
                if (c != null) {
                    c.close();
                }
            }

            return info;
        }

        @Override
        protected void onStartLoading() {
            forceLoad();
        }

    }

}
