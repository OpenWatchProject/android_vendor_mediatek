package com.mediatek.op18.contacts;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.net.Uri;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Profile;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;

import com.mediatek.contacts.ext.DefaultContactsPickerExtension;

public class OP18ContactsPickerExt extends DefaultContactsPickerExtension {
    private static final String TAG = "OP18ContactsPickerExt";
    private static final String INTENT_EXTRA_NEW_LOCAL_PROFILE = "newLocalProfile";
    private Context mHostContext;
    private Resources mHostResources;
    String packageName = "com.android.contacts";
    String menuName = "mtk_list_multichoice";
    String searchMenu = "search_menu_item";
    String searchMenuTitle = "hint_findContacts";


    public OP18ContactsPickerExt(Context context) {
        Log.d(TAG, "addSearchMenu entry");
    }
    /**
     * Add the search menu to contacts picker screen.
     *
     * @param activity
     *            the Activity.
     * @param menu
     *            the Menu.
     */
    @Override
    public void addSearchMenu(Activity activity, Menu menu) {
        Log.d(TAG, "addSearchMenu entry");
        mHostContext = activity;
        mHostResources = mHostContext.getResources();
        MenuInflater inflater = activity.getMenuInflater();
        int menuRes = mHostResources.getIdentifier(menuName, "menu", packageName);
        inflater.inflate(menuRes, menu);

        int searchId = mHostResources.getIdentifier(searchMenu, "id", packageName);
        MenuItem optionItem = menu.findItem(searchId);

        int searchTitle = mHostResources.getIdentifier(searchMenuTitle, "string", packageName);
        optionItem.setTitle(searchTitle);
        Log.d(TAG, "addSearchMenu exit");
    }

    /**
     * enable or disable the search icon to contacts picker screen.
     *
     * @param isSearchMode
     *            the Seach active or not.
     * @param menu
     *            the Menu.
     * @return true or false based upon handling
     */
    @Override
    public boolean enableDisableSearchMenu(boolean isSearchMode, Menu menu) {
        Log.d(TAG, "enableDisableSearchMenu isSearchMode" + isSearchMode);
        int searchId = mHostResources.getIdentifier(searchMenu, "id", packageName);
        MenuItem menuItem = menu.findItem(searchId);
        if (isSearchMode) {
            menuItem.setVisible(false);
            return false;
        } else {
            menuItem.setVisible(true);
            return true;
        }
    }

    /**
     * open add profile instead of toast contact not found.
     *
     * @param lookupUri
     *            the contact uri of lookup contact.
     * @param context
     *            Context.
     * @return true or false based upon handling
     */
    @Override
    public boolean openAddProfileScreen(Uri lookupUri, Context context) {
        boolean isprofile = lookupUri.equals(Profile.CONTENT_URI);
        Log.d(TAG, "openAddProfileScreen isprofile" + isprofile);
        if (isprofile) {
            Intent intent = new Intent(Intent.ACTION_INSERT, Contacts.CONTENT_URI);
            intent.putExtra(INTENT_EXTRA_NEW_LOCAL_PROFILE, true);
            String packageName = context.getPackageName();
            intent.setPackage(packageName);
            context.startActivity(intent);
            Log.d(TAG, "openAddProfileScreen add profile intent" + intent);
            return true;
        } else {
            return false;
        }
    }

}
