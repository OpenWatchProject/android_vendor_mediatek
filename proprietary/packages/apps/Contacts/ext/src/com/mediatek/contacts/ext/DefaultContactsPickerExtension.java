package com.mediatek.contacts.ext;

import android.app.Activity;
import android.content.Context;
import android.net.Uri;
import android.view.Menu;

public class DefaultContactsPickerExtension implements IContactsPickerExtension {
    /**
     * Add the search menu to contacts picker screen
     *
     * @param activity
     *            the Activity.
     * @param menu
     *            the Menu.
     */
    @Override
    public void addSearchMenu(Activity activity, Menu menu) {

    }

    /**
     * enable or disable the search icon to contacts picker screen
     *
     * @param isSearchEnabled
     *            the Seach active or not.
     * @param menu
     *            the Menu.
     * @return true true or false based upon handling
     */
    @Override
    public boolean enableDisableSearchMenu(boolean isSearchEnabled, Menu menu) {
        return true;
    }

    /**
     * open add profile instead of toast no contact.
     *
     * @param lookupUri
     *            searched Uri of contact.
     * @param context
     *            Context.
     * @return true or false based upon handling
     */
    @Override
    public boolean openAddProfileScreen(Uri lookupUri, Context context) {
        return false;
    }
}
