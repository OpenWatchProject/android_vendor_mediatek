package com.mediatek.contacts.ext;

import android.app.Fragment;
import android.app.FragmentManager;
import android.content.Intent;
import android.content.Context;
import android.net.Uri;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;

import com.android.contacts.model.RawContact;
import com.android.contacts.model.RawContactDelta;

import com.google.common.collect.ImmutableList;

public class DefaultRcsExtension implements IRcsExtension {
    //---------------------for list-------------------//
    /**
     * OP01 RCS will filter phone number in list.
     * let the op01 get intent data from host set.
     * @param intent data from caller
     * @param fragment Fragment
     */
    @Override
    public void getIntentData(Intent intent, Fragment fragment) {
        //do-nothing
    }

    /**
     * OP01 RCS will filter phone number in list.
     * set the exist numbers for filter,not show in list.
     * @param selection Query selection
     * @param context Context
     */
    @Override
    public void setListFilter(StringBuilder selection, Context context) {
        //do-nothing
    }

    /**
     * OP01 RCS will configure phone list uri.
     * remove duplicate number in list.
     * @param query uri
     */
    @Override
    public Uri configListUri(Uri uri) {
        return uri;
    }

    /**
     * OP01 RCS will add menu item in list, eg:group.
     * mark item for selected group in list.
     * @param context Context
     * @param menu Menu
     * @param item MenuItem
     * @param fragment Fragment
     */
    @Override
    public void addListMenuOptions(Context context, Menu menu,
            MenuItem item, Fragment fragment) {
        //do-nothing
    }

    /**
     * OP01 RCS will mark item for selected group in phone numbers list.
     * get data ids from selected group and mark item in list.
     * @param list fragment and data ids
     */
    @Override
    public void getGroupListResult(Fragment fragment, long[] ids) {
        //do-nothing
    }

    //-----------------------------for group------------------//
    /**
     * OP01 RCS will add group menu item
     * custom group menu and show in action bar.
     * @param menu: group menu
     * @param context: group activity context
     */
    @Override
    public void addGroupMenuOptions(Menu menu, Context context) {
        //do-nothing
    }

    /**
     * OP01 RCS will add group detail menu item
     * custom group menu and show in action bar.
     * @param menu  group detail menu.
     * @param groupId  group id.
     * @param context  contact host context.
     * @param groupSize  group member size.
     */
    @Override
    public void addGroupDetailMenuOptions(Menu menu, long groupId, Context context, int groupSize) {
        //do-nothing
    }

     //-----------------------------for editor------------------//
     /**
     * OP01 RCS will listen phone number text change.
     * if query same phone number, show popup list view for choose.
     * @param state RawContactDelta
     * @param view EditText
     * @param inputType input type
     * @param number input number
     */
    @Override
    public void setTextChangedListener(RawContactDelta state, EditText view, int inputType,
            String number) {
        //do-nothing
    }

     /**
     * OP01 RCS will close listener of phone number text change.
     * close popup list and destroy search engine.
     * @param quit search engine or only close popup list
     */
    @Override
    public void closeTextChangedListener(boolean quit) {
        //do-nothing
    }

    /**
     * OP01 RCS will set current editor fragment and it's manager.
     * @param fragment Fragment
     * @param manager FragmentManager
     */
    @Override
    public void setEditorFragment(Fragment fragment, FragmentManager manager) {
        //do-nothing
    }

    /**
     * OP01 RCS will add editor menu item
     * custom editor menu and show in action bar.
     * @param fragment  Fragment
     * @param menu  Editor Menu
     * @param isInsert  insert or edit
     */
    @Override
    public void addEditorMenuOptions(Fragment fragment, Menu menu, boolean isInsert) {
        //do-nothing
    }

    //--------------for PeopleActivity----------------//
    /**
     * OP01 RCS will add menu item in PeopleActivity
     * @param people menu
     */
    @Override
    public void addPeopleMenuOptions(Menu menu) {
        //do-nothing
    }

    /**
     * OP01 RCS will go to personal profile activity when click profile item.
     * @param uri contact lookup uri.
     * @param isEmpty If current profile is empty.
     * @return success or failure
     */
    @Override
    public boolean addRcsProfileEntryListener(Uri uri, boolean isEmpty) {
        return false;
    }

    /**
     * OP01 RCS will add public account entry to people list.
     * @param listView list view of contact info
     */
    @Override
    public void createPublicAccountEntryView(ListView listView) {
        //do-nothing
    }

    /**
     * OP01 RCS will add some entry to people list.
     * @param listView list view of contact info.
     * @param context Context.
     */
    @Override
    public void createEntryView(ListView listView, Context context) {
        //do-nothing
    }

    //--------------for QuickContact----------------//
    /**
     * OP01 RCS will go to contact detail activity, update photo from rcs server.
     * @param lookupUri Contact lookup uri.
     * @param photo Contact photo view.
     * @param context Context.
     */
    @Override
    public void updateContactPhotoFromRcsServer(Uri lookupUri, ImageView photo, Context context) {
         //do-nothing
    }

    /**
     * OP01 RCS will configure contact account.
     * @param rawContacts original rawContacts list.
     * @param isProfile if it is profile.
     * @return configured rawContacts list.
     */
    @Override
    public ImmutableList<RawContact> rcsConfigureRawContacts(
            ImmutableList<RawContact> rawContacts, boolean isProfile) {
        return rawContacts;
    }

    /**
     * OP01 RCS will get photo from rcs server, and refresh thumbnail Photo.
     * @param isLetterTile letter photo or thumbnail Photo.
     * @param hasThemeColor Theme Color has set or not.
     * @return true or false.
     */
    @Override
    public boolean needUpdateContactPhoto(boolean isLetterTile, boolean hasThemeColor) {
        //do-nothing, just return hasThemeColor default value.
        return hasThemeColor;
    }

    /**
     * OP01 RCS will add quick contact menu item.
     * @param menu  quick contact menu item.
     * @param uri  Contact uri.
     * @param context  contact host context.
     */
    public void addQuickContactMenuOptions(Menu menu, Uri uri, Context context) {
        //do-nothing
    }

    @Override
    public ContactListItemRcsView getContactListItemRcsView() {
        return mContactListItemRcsView;
    }

    @Override
    public QuickContactRcsScroller getQuickContactRcsScroller() {
        return mQuickContactScrollerCustom;
    }

    @Override
    public boolean isRcsServiceAvailable() {
        return false;
    }

    /////------------------below are the default implements------------------//
    private ContactListItemRcsView mContactListItemRcsView = new ContactListItemRcsView() {

        @Override
        public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
            // do-nothing
        }

        @Override
        public void onLayout(boolean changed, int leftBound, int topBound, int rightBound,
                int bottomBound) {
            // do-nothing
        }

        @Override
        public void addCustomView(long contactId, ViewGroup viewGroup) {
        }

        @Override
        public int getWidthWithPadding() {
            return 0;
        }
    };

    private QuickContactRcsScroller mQuickContactScrollerCustom = new QuickContactRcsScroller() {
        @Override
        public View createRcsIconView(View container, View anchorView, Uri lookupUri) {
            return null;
        }

        @Override
        public void updateRcsContact(Uri lookupUri, boolean isLoadFinished) {
            // do-nothing
        }

        @Override
        public void updateRcsIconView() {
            // do-nothing
        }
    };
}
