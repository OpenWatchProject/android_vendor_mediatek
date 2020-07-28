package com.mediatek.browser.ext;

import android.app.Activity;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;

public class DefaultBrowserBookmarkExt implements IBrowserBookmarkExt {

    private static final String TAG = "DefaultBrowserBookmarkExt";

    @Override
    public int addDefaultBookmarksForCustomer(SQLiteDatabase db) {
        Log.i("@M_" + TAG, "Enter: " + "addDefaultBookmarksForCustomer" + " --default implement");
        return 0;
    }

    @Override
    public void createBookmarksPageOptionsMenu(Menu menu, MenuInflater inflater) {
        Log.i("@M_" + TAG, "Enter: " + "createBookmarksPageOptionsMenu" + " --default implement");
    }

    @Override
    public boolean bookmarksPageOptionsMenuItemSelected(MenuItem item,
                                                        Activity activity,
                                                        long folderId) {
        Log.i("@M_" + TAG, "Enter: " + "bookmarksPageOptionsMenuItemSelected" +
                " --default implement");
        return false;
    }

    @Override
    public boolean customizeEditExistingFolderState(Bundle bundle, boolean state) {
        Log.i(TAG, "Enter: " + "customizeEditExistingFolderState" + " --default implement");
        return state;
    }

    @Override
    public String getCustomizedEditFolderFakeTitleString(Bundle bundle, String title) {
        Log.i(TAG, "Enter: " + "getCustomizedEditFolderFakeTitleString" + " --default implement");
        return title;
    }

    @Override
    public void showCustomizedEditFolderNewFolderView(
            View newFolder, View divider, Bundle bundle) {
        Log.i(TAG, "Enter: " + "showCustomizedEditFolderNewFolderView" + " --default implement");
        if (newFolder != null) {
            newFolder.setVisibility(View.VISIBLE);
        }
        if (divider != null) {
            divider.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public boolean shouldSetCustomizedEditFolderSelection(Bundle bundle, boolean setValue) {
        Log.i(TAG, "Enter: " + "shouldSetCustomizedEditFolderSelection" + " --default implement");
        return setValue;
    }

    @Override
    public Boolean saveCustomizedEditFolder(Context context, String title,
            long currentFolder, Bundle bundle, String errorMessage) {
        Log.i(TAG, "Enter: " + "saveCustomizedEditFolder" + " --default implement");
        return null;
    }

}
