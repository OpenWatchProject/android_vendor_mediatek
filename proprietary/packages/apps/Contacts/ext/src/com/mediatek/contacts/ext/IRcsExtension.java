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

/**
 * For Operator RCSE
 */
public interface IRcsExtension {
    // ---------------------for list-------------------//
    /**
     * RCS will filter phone number in list. let the op01 get intent data from
     * host set.
     *
     * @param intent
     *            data from caller
     * @param fragment
     *            Fragment
     * @internal
     */
    void getIntentData(Intent intent, Fragment fragment);

    /**
     * RCS will filter phone number in list. set the exist numbers for
     * filter,not show in list.
     *
     * @param selection
     *            Query selection
     * @param context
     *            Context
     * @internal
     */
    void setListFilter(StringBuilder selection, Context context);

    /**
     * RCS will configure phone list uri. remove duplicate number in list.
     *
     * @param query
     *            uri
     * @internal
     */
    Uri configListUri(Uri uri);

    /**
     * RCS will add menu item in list, eg:group. mark item for selected group in
     * list.
     *
     * @param context
     *            Context
     * @param menu
     *            Menu
     * @param item
     *            MenuItem
     * @param fragment
     *            Fragment
     * @internal
     */
    void addListMenuOptions(Context context, Menu menu, MenuItem item, Fragment fragment);

    /**
     * RCS will mark item for selected group in phone numbers list. get data ids
     * from selected group and mark item in list.
     *
     * @param list
     *            fragment and data ids
     * @internal
     */
    void getGroupListResult(Fragment fragment, long[] ids);

    // -----------------------------for group------------------//
    /**
     * RCS will add group menu item custom group menu and show in action bar.
     *
     * @param menu
     *            : group menu
     * @param context
     *            : group activity context
     * @internal
     */
    void addGroupMenuOptions(Menu menu, Context context);

    /**
     * OP01 RCS will add group detail menu item
     * custom group menu and show in action bar.
     * @param menu  group detail menu.
     * @param groupId  group id.
     * @param context  contact host context.
     * @param groupSize  group member size.
     * @internal
     */
    void addGroupDetailMenuOptions(Menu menu, long groupId, Context context, int groupSize);

    // -----------------------------for editor------------------//
    /**
     * RCS will listen phone number text change. if query same phone number,
     * show popup list view for choose.
     *
     * @param state
     *            RawContactDelta
     * @param view
     *            EditText
     * @param inputType
     *            input type
     * @param number
     *            input number
     * @internal
     */
    void setTextChangedListener(RawContactDelta state, EditText view, int inputType, String number);

    /**
     * RCS will close listener of phone number text change. close popup list and
     * destroy search engine.
     *
     * @param quit
     *            search engine or only close popup list
     * @internal
     */
    void closeTextChangedListener(boolean quit);

    /**
     * RCS will set current editor fragment and it's manager.
     *
     * @param fragment
     *            Fragment
     * @param manager
     *            FragmentManager
     * @internal
     */
    void setEditorFragment(Fragment fragment, FragmentManager manager);

    /**
     * OP01 RCS will add editor menu item
     * custom editor menu and show in action bar.
     * @param fragment  Fragment
     * @param menu  Editor Menu
     * @param isInsert  insert or edit
     * @internal
     */
    void addEditorMenuOptions(Fragment fragment, Menu menu, boolean isInsert);

    // --------------for PeopleActivity----------------//
    /**
     * RCS will add menu item in PeopleActivity
     *
     * @param people
     *            menu
     * @internal
     */
    public void addPeopleMenuOptions(Menu menu);

    /**
     * RCS will go to personal profile activity when click profile item.
     *
     * @param uri
     *            contact lookup uri.
     * @param isEmpty
     *            If current profile is empty.
     * @return success or failure
     * @internal
     */
    boolean addRcsProfileEntryListener(Uri uri, boolean isEmpty);

    /**
     * RCS will add public account entry to people list.
     *
     * @param listView
     *            list view of contact info
     * @internal
     */
    void createPublicAccountEntryView(ListView listView);

    /**
     * RCS will add some entry to people list.
     *
     * @param listView
     *            list view of contact info
     * @param context
     *            Context.
     */
    void createEntryView(ListView listView, Context context);

    // --------------for QuickContact----------------//
    /**
     * RCS will go to contact detail activity, update photo from rcs server.
     *
     * @param lookupUri
     *            Contact lookup uri.
     * @param photo
     *            Contact photo view.
     * @param context
     *            Context.
     * @internal
     */
    void updateContactPhotoFromRcsServer(Uri lookupUri, ImageView photo, Context context);

    /**
     * RCS will configure contact account.
     *
     * @param rawContacts
     *            original rawContacts list.
     * @param isProfile
     *            if it is profile.
     * @return configured rawContacts list.
     * @internal
     */
    ImmutableList<RawContact> rcsConfigureRawContacts(ImmutableList<RawContact> rawContacts,
            boolean isProfile);

    /**
     * RCS will get photo from rcs server, and refresh thumbnail Photo.
     *
     * @param isLetterTile
     *            letter photo or thumbnail Photo.
     * @param hasThemeColor
     *            Theme Color has set or not.
     * @return true or false.
     * @internal
     */
    boolean needUpdateContactPhoto(boolean isLetterTile, boolean hasThemeColor);

    /**
     * OP01 RCS will add quick contact menu item.
     * @param menu  quick contact menu item.
     * @param uri  Contact uri.
     * @param context  contact host context.
     * @internal
     */
    void addQuickContactMenuOptions(Menu menu, Uri uri, Context context);

    /**
     * Used in RCS to custom ContactListItemView in People List.
     */
    public interface ContactListItemRcsView {
        /**
         * onMeasure.
         *
         * @param widthMeasureSpec
         *            width MeasureSpec
         * @param heightMeasureSpec
         *            height MeasureSpec
         * @internal
         */
        void onMeasure(int widthMeasureSpec, int heightMeasureSpec);

        /**
         * onLayout.
         *
         * @param changed
         *            changed
         * @param leftBound
         *            left Bound
         * @param topBound
         *            top Bound
         * @param rightBound
         *            right Bound
         * @param bottomBound
         *            bottom Bound
         * @internal
         */
        void onLayout(boolean changed, int leftBound, int topBound, int rightBound,
                int bottomBound);

         /**
         * addCustomView.
         * @param contactId contactId
         * @param viewGroup ViewGroup
         */
        void addCustomView(long contactId, ViewGroup viewGroup);

        /**
        * getWidthWithPadding.
        * @return padding width
        */
        int getWidthWithPadding();
    }

    /**
     * Used to add Joyn icon on the left top in QuickActivity.
     *
     */
    public interface QuickContactRcsScroller {
        /**
         * create Rcs Icon View.
         *
         * @param container
         *            A FrameLayout placed on the left top.
         * @param anchorView
         *            the NameView
         * @param lookupUri
         *            Contact uri
         * @return ImageView contains joyn icon
         * @internal
         */
        View createRcsIconView(View container, View anchorView, Uri lookupUri);

        /**
         * update Rcs contact.
         *
         * @param lookupUri
         *            Contact uri
         * @param isLoadFinished
         *            is load contact finished
         * @internal
         */
        void updateRcsContact(Uri lookupUri, boolean isLoadFinished);

        /**
         * update visibility of the ImageView when scroll the screen.
         * @internal
         */
        void updateRcsIconView();
    }

    /**
     *
     * @return get the object for custom ContactListItemView
     * @internal
     */
    ContactListItemRcsView getContactListItemRcsView();

    /**
     * @return get the object for custom MultiShrinkScroller
     * @internal
     */
    QuickContactRcsScroller getQuickContactRcsScroller();

    /**
     * check if RCS service is run.
     *
     * @return RCS service is availabe
     */
    boolean isRcsServiceAvailable();

}
