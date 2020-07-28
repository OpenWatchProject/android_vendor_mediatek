package com.mediatek.browser.ext;

import android.app.Activity;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

public interface IBrowserHistoryExt {

    /**
     * Create the history page option menu
     * @param menu the menu
     * @param inflater the menu inflater
     */
    void createHistoryPageOptionsMenu(Menu menu, MenuInflater inflater);

    /**
     * Prepare the history page option menu
     * @param menu the menu
     * @param isNull whether the adapter is null or not
     * @param isEmpty whether the adapter is empty or not
     */
    void prepareHistoryPageOptionsMenuItem(Menu menu, boolean isNull, boolean isEmpty);

    /**
     * Handle the history page option menu item selected operation
     * @param item the menu item
     * @param activity the history page activity
     * @return true to consume the menu handling already
     */
    boolean historyPageOptionsMenuItemSelected(MenuItem item, Activity activity);
}
