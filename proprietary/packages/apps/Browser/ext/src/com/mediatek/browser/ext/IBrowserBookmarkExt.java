package com.mediatek.browser.ext;

import android.app.Activity;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;

public interface IBrowserBookmarkExt {

    /**
     * Add the default bookmarks for customer
     * @param db the Browser database
     * @return the number of the bookmarks added
     */
    int addDefaultBookmarksForCustomer(SQLiteDatabase db);

    /**
     * Create the bookmark page option menu
     * @param menu the menu
     * @param inflater the menu inflater
     */
    void createBookmarksPageOptionsMenu(Menu menu, MenuInflater inflater);

    /**
     * Handle the bookmark page option menu item selected operation
     * @param item the menu item
     * @param activity the bookmark page activity
     * @param folderId the current folder id
     * @return true to consume the menu handling already
     */
    boolean bookmarksPageOptionsMenuItemSelected(MenuItem item, Activity activity, long folderId);

    /**
     * Customize the state of editing the existing folder.
     * @param bundle the edit folder bundle
     * @param state the default state of editing the existing folder
     * @return the customized state
     */
    boolean customizeEditExistingFolderState(Bundle bundle, boolean state);

    /**
     * Get the customized fake title string for editing folder.
     * @param bundle the edit folder bundle
     * @param title the default title string
     * @return the customized title string
     */
    String getCustomizedEditFolderFakeTitleString(Bundle bundle, String title);

    /**
     * Show the add new folder view for editing folder.
     * @param newFolder the add new folder view
     * @param divider the add divider
     * @param bundle the edit folder bundle
     */
    void showCustomizedEditFolderNewFolderView(View newFolder, View divider, Bundle bundle);

    /**
     * Should set the folder selection.
     * @param bundle the edit folder bundle
     * @param setValue the default value
     * @return the customized set value
     */
    boolean shouldSetCustomizedEditFolderSelection(Bundle bundle, boolean setValue);

    /**
     * Save the customized edit folder.
     * @param context the context
     * @param title the current title
     * @param currentFolder the current folder id
     * @param bundle the edit folder bundle
     * @param errorMessage the error message shown when saving failed
     * @return Boolean to consume the saving operation already
     */
    Boolean saveCustomizedEditFolder(Context context, String title, long currentFolder,
                Bundle bundle, String errorMessage);

}
