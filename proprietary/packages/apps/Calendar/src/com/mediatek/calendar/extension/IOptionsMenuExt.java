package com.mediatek.calendar.extension;

import android.view.Menu;

/**
 * M: this interface describes the way to extend AllInOneActivity's Options menu
 * A plug-in who implements this interface can be seem as an extension to Options menu
 * or action bar.
 */
public interface IOptionsMenuExt {
    /**
     * M: Plug-in should configure itself via this method
     * @param menu menu
     */
    void onCreateOptionsMenu(Menu menu);
    /**
     * M: Plug-in should tell the host whether it has been selected
     * that is to say, host will testify the selected id with plug-in.
     * @param itemId the item who is selected
     * @return true if the id can be recognized and already handled by plug-in
     * false otherwise.
     */
    boolean onOptionsItemSelected(int itemId);
}
