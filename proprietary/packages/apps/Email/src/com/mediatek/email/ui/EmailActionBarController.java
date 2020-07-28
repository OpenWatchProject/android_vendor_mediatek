//package com.mediatek.email.ui;
//
//import android.app.FragmentTransaction;
//import android.app.SearchManager;
//import android.app.SearchableInfo;
//import android.content.Context;
//import android.content.res.Configuration;
//import android.content.res.Resources;
//import android.graphics.drawable.Drawable;
//import android.os.Bundle;
//import android.os.Parcelable;
//import android.support.v4.view.MenuItemCompat;
//import android.support.v7.app.ActionBar;
//import android.support.v7.widget.SearchView;
//import android.text.TextUtils;
//import android.util.AttributeSet;
//import android.view.ActionMode;
//import android.view.ActionMode.Callback;
//import android.view.Gravity;
//import android.view.Menu;
//import android.view.MenuItem;
//import android.view.View;
//import android.view.ViewGroup;
//import android.view.MenuItem.OnMenuItemClickListener;
//import android.view.WindowManager;
//import android.view.inputmethod.EditorInfo;
//import android.view.inputmethod.InputMethodManager;
//import android.widget.AdapterView;
//import android.widget.ArrayAdapter;
//import android.widget.HorizontalScrollView;
//import android.widget.LinearLayout;
//import android.widget.ListAdapter;
//import android.widget.ListPopupWindow;
//import android.widget.ListView;
//import android.widget.SpinnerAdapter;
//import android.widget.AdapterView.OnItemClickListener;
//import android.widget.TextView;
//
//import com.android.email.R;
//import com.android.emailcommon.service.SearchParams;
//import com.android.emailcommon.utility.Utility;
//import com.android.mail.ConversationListContext;
//import com.android.mail.providers.UIProvider.AccountCapabilities;
//import com.android.mail.ui.ActionBarController;
//import com.android.mail.ui.ViewMode;
//import com.android.mail.utils.LogTag;
//import com.android.mail.utils.LogUtils;
//import com.android.mail.utils.Throttle;
//import com.android.mail.utils.Utils;
//
//import java.lang.reflect.Field;
//import java.util.ArrayList;
//
///**
// * M: Base on MailActionBarView, add local search related feature.
// */
//public class EmailActionBarController extends ActionBarController implements View.OnClickListener {
//    //private static final String BUNDLE_KEY_ACTION_BAR_SELECTED_FIELD = "ActionBarController.ACTION_BAR_SELECTED_TAB";
//    private TabListener mTabListener = new TabListener();
//    private String mSearchField;
//    private static final String[] SEARCH_FIELD_LIST = { SearchParams.SEARCH_FIELD_ALL, SearchParams.SEARCH_FIELD_FROM,
//            SearchParams.SEARCH_FIELD_TO, SearchParams.SEARCH_FIELD_SUBJECT, SearchParams.SEARCH_FIELD_BODY };
//    private static final int INITIAL_FIELD_INDEX = 1;
//
//    private int mLocalSearchResult = 0;
//    private TextView mSearchResultCountView;
//    private TextView mSearchFiledSpinner;
//    private HorizontalScrollView mSearchFieldTabs;
//    private MailSearchTabView mSelectedTab;
//    private SearchFieldDropdownPopup mSearchFiledDropDown;
//    // Indicated user was opening searched conversation, we don't need exit local search mode.
//    private boolean mOpeningLocalSearchConversation = false;
//    // Indicated user was back from searched conversation, we need restore query text.
//    private boolean mBackingLocalSearchConversation = false;
//
//    /// M: expandSearch might be called eariler than onCreateOptionsMenu(),
//     // in this case, execute this expanding request after creating the search UI
//    private String mPendingQuery;
//    /// M: the flag is set to true if the localsearch execute but the search bar has not expand.
//    private boolean mHasPendingQuery = false;
//    /// M: Backup the query string of the UI, because mSearchWidget.getQuery() maybe get nothing.
//    protected String mQueryTextString;
//
//    private static final String SEARCHVIEW_QUERYTEXT_FIELD_NAME = "mQueryTextView";
//
//    public EmailActionBarController(Context context) {
//        super(context);
//    }
//
//    @Override
//    public boolean onCreateOptionsMenu(Menu menu) {
//        mEmptyTrashItem = menu.findItem(R.id.empty_trash);
//        mEmptySpamItem = menu.findItem(R.id.empty_spam);
//
//        // If the mode is valid, then set the initial menu
//        if (getMode() == ViewMode.UNKNOWN) {
//            return false;
//        }
//        mSearch = menu.findItem(R.id.search);
//        if (mSearch != null) {
//            mSearch.setActionView(R.layout.local_search_actionbar_view);
//            mSearchResultCountView = (TextView) MenuItemCompat.getActionView(mSearch)
//                    .findViewById(R.id.result_count);
//            mSearchResultCountView.setText(String.valueOf(0));
//            mSearchFiledSpinner = (TextView) MenuItemCompat.getActionView(mSearch)
//                    .findViewById(R.id.search_field);
//            mSearchWidget = (SearchView) MenuItemCompat.getActionView(mSearch)
//                    .findViewById(R.id.email_search_view);
//            MenuItemCompat.setOnActionExpandListener(mSearch, this);
//            mSearch.setOnMenuItemClickListener(new OnSearchItemClickListener());
//            if (mSearchWidget != null) {
//                mSearchWidget.setOnQueryTextListener(this);
//                mSearchWidget.setOnSuggestionListener(this);
//                mSearchWidget.setIconifiedByDefault(true);
//                mSearchWidget.setQueryHint(getContext().getResources()
//                        .getString(R.string.search_hint));
//                disableSearchViewActionMode(mSearchWidget);
//            }
//        }
//        return true;
//    }
//
//    /**
//     * This is a workaround of support.v7 SearchView's action mode issue.
//     * Now there is no better way to disable SearchView's action mode. We
//     * have to hack in to this object, get the mQueryTextView filed and turn it off.
//     * @param searchView SearchView to disable the action mode.
//     */
//    private void disableSearchViewActionMode(SearchView searchView) {
//        Field queryTextViewfield = null;
//        try{
//            queryTextViewfield = searchView.getClass()
//                    .getDeclaredField(SEARCHVIEW_QUERYTEXT_FIELD_NAME);
//        } catch (NoSuchFieldException e) {
//            LogUtils.e(LOG_TAG, " get SearchView mQueryTextView field failded " +
//                        "due to NoSuchFieldException");
//        }
//        if (queryTextViewfield != null) {
//            queryTextViewfield.setAccessible(true);
//            TextView queryTextView = null;
//            try {
//                queryTextView = (TextView) queryTextViewfield.get(searchView);
//            } catch (IllegalAccessException e) {
//                LogUtils.e(LOG_TAG, "SearchView mQueryTextView field return" +
//                        " IllegalAccessException");
//            } catch (IllegalArgumentException e) {
//                LogUtils.e(LOG_TAG, "SearchView mQueryTextView field return" +
//                        " IllegalArgumentException");
//            }
//            if (queryTextView != null) {
//                // replace the default action mode with new one.
//                queryTextView.setCustomSelectionActionModeCallback(new Callback() {
//                    @Override
//                    public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
//                        return false;
//                    }
//                    @Override
//                    public void onDestroyActionMode(ActionMode mode) {
//                    }
//                    @Override
//                    public boolean onCreateActionMode(ActionMode mode, Menu menu) {
//                        // return false to disable default action mode.
//                        return false;
//                    }
//                    @Override
//                    public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
//                        return false;
//                    }
//                });
//            }
//        }
//    }
//
//    @Override
//    public void validateVolatileMenuOptionVisibility() {
//        super.validateVolatileMenuOptionVisibility();
//        // disable empty trash/spam items in local search mode.
//        ConversationListContext currentListContext = mController.getCurrentListContext();
//        boolean isLocalSearch = currentListContext != null && currentListContext.isLocalSearch();
//        if (mEmptyTrashItem != null) {
//            mEmptyTrashItem.setVisible(mEmptyTrashItem.isVisible() && !isLocalSearch);
//        }
//        if (mEmptySpamItem != null) {
//            mEmptySpamItem.setVisible(mEmptySpamItem.isVisible() && !isLocalSearch);
//        }
//    }
//
//    @Override
//    public boolean onPrepareOptionsMenu(Menu menu) {
//        boolean result = super.onPrepareOptionsMenu(menu);
//        // always enable search menu for local searching.
//        if (!mController.shouldHideMenuItems()) {
//            Utils.setMenuItemVisibility(menu, R.id.search, true);
//        }
//        // M: if has been local searching(resume activity/pull out drawer), init view content.
//        // add mSearch null check, sometimes(in localsearch conversationview mode) the mSearch
//        // will be null if we change to landscape. @{
//        ConversationListContext currentListContext = mController.getCurrentListContext();
//        boolean isLocalSearch = currentListContext != null && currentListContext.isLocalSearch();
//        if (mSearch != null && isLocalSearch) {
//            mSearchResultCountView.setText(String.valueOf(mLocalSearchResult));
//            expandSearch(mQueryTextString, currentListContext.getSearchField());
//            if (!useTabMode(getContext())) {
//                initSpinner();
//            }
//        }
//        /// @}
//
//        /// M: restore local search state, it's a little strange, however, the menu item don't expand
//        /// when back form conversation view. @{
//        if (mBackingLocalSearchConversation) {
//            if (currentListContext != null) {
//                //Reset local search status.
//                LogUtils.logFeature(LogTag.SEARCH_TAG,
//                        "onPrepareOptionsMenu reset localsearch, currentListContext [%s]", currentListContext);
//                expandSearch(currentListContext.getSearchQuery(), currentListContext.getSearchField());
//            }
//            mBackingLocalSearchConversation = false;
//        }
//        /// @}
//
//        /// M: There's a pending expanding search request
//        if (mHasPendingQuery) {
//            expandSearch(mPendingQuery, mSearchField);
//            mPendingQuery = null;
//            mHasPendingQuery = false;
//        }
//        return result;
//    }
//
//    @Override
//    public boolean onQueryTextSubmit(String query) {
//        LogUtils.logFeature(LogTag.SEARCH_TAG, "onQueryTextSubmit [%s]", query);
//        if (!TextUtils.isEmpty(query)) {
//            mLocalSearchThrottle.onEvent();
//            // Follow JB, hide the input method, when press on the submit/search keyboard.
//            if (mActivity != null) {
//                final InputMethodManager imm = (InputMethodManager) mActivity
//                        .getActivityContext().getSystemService(Context.INPUT_METHOD_SERVICE);
//                if (imm != null && imm.isActive()) {
//                    imm.hideSoftInputFromWindow(mSearchWidget.getWindowToken(), 0);
//                }
//            }
//        }
//        return true;
//    }
//
//    @Override
//    public boolean onQueryTextChange(String newText) {
//        // if back from conversation view, don't re-query empty term in conversations list.
//        if (mBackingLocalSearchConversation
//                && TextUtils.isEmpty(newText)) {
//            return true;
//        }
//        /// M: backup the query string.
//        mQueryTextString = newText;
//        /**
//         * Not start local search immediately, use Throttle control the query event.
//         */
//        LogUtils.logFeature(LogTag.SEARCH_TAG, "onQueryTextChange [%s]", newText);
//        mLocalSearchThrottle.onEvent();
//        return true;
//    }
//
//    @Override
//    public boolean onMenuItemActionExpand(MenuItem item) {
//        boolean result = super.onMenuItemActionExpand(item);
//        String listContextQuery = null;
//        // backup the list context query, cause onActionViewExpanded would clear query text.
//        ConversationListContext listContext = mController.getCurrentListContext();
//        if (listContext != null && listContext.isLocalSearch()) {
//            listContextQuery = listContext.getSearchQuery();
//        }
//        mSearchWidget.onActionViewExpanded();
//        mSearchWidget.setImeOptions(EditorInfo.IME_FLAG_NO_FULLSCREEN | EditorInfo.IME_ACTION_SEARCH);
//        if (listContextQuery != null) {
//            mSearchWidget.setQuery(listContextQuery, false);
//        }
//        return result;
//    }
//
//    @Override
//    public boolean onMenuItemActionCollapse(MenuItem item) {
//        ConversationListContext listContext = mController.getCurrentListContext();
//        if (mOpeningLocalSearchConversation) {
//            removeLocalSearchView();
//            return super.onMenuItemActionCollapse(item);
//        } else if (listContext != null && listContext.isLocalSearch()) {
//            mController.exitLocalSearch();
//            // call SearchView's collapsed api to clear focus and query text.
//            mSearchWidget.onActionViewCollapsed();
//            // M: Manual clear the query text, make sure the query text cleared.
//            mSearchWidget.setQuery(null, false);
//            removeLocalSearchView();
//        }
//        return super.onMenuItemActionCollapse(item);
//    }
//
//    private int mLastViewMode = ViewMode.UNKNOWN;
//    /*
//     * M: Don't exit local search mode, if open message in local search results list.
//     * @see com.android.mail.ui.MailActionBarView#onViewModeChanged(int)
//     */
//    @Override
//    public void onViewModeChanged(int newMode) {
//        ConversationListContext listContext = mController.getCurrentListContext();
//        if (listContext != null && listContext.isLocalSearch()
//                && mLastViewMode == ViewMode.CONVERSATION_LIST && newMode == ViewMode.CONVERSATION) {
//            mOpeningLocalSearchConversation = true;
//            mBackingLocalSearchConversation = false;
//            clearSearchFocus();
//        }
//        if (listContext != null && listContext.isLocalSearch()
//                && mLastViewMode == ViewMode.CONVERSATION && newMode == ViewMode.CONVERSATION_LIST) {
//            mOpeningLocalSearchConversation = false;
//            mBackingLocalSearchConversation = true;
//        }
//        mLastViewMode = newMode;
//        super.onViewModeChanged(newMode);
//    }
//
//    /**
//     * Remove focus from the search field to avoid 1. The keyboard popping in and out. 2. The search suggestions shown
//     * up.
//     */
//    private void clearSearchFocus() {
//        // Remove focus from the search action menu in search results mode so
//        // the IME and the suggestions don't get in the way.
//        mSearchWidget.clearFocus();
//    }
//
//    /**
//     * Expand the local search UI and query the text
//     */
//    @Override
//    public void expandSearch(String query, String field) {
//        // M: support expand search for specific field
//        if (field != null) {
//            mSearchField = field;
//        } else {
//            mSearchField = SEARCH_FIELD_LIST[INITIAL_FIELD_INDEX];
//        }
//        if (mSearch != null) {
//            mController.enterLocalSearch(mSearchField);
//            initLocalSearchView();
//            super.expandSearch();
//            mSearchWidget.setQuery(query, false);
//        } else {
//            mPendingQuery = query;
//            ///M: after search actionbar expand, the pending search will be execute
//            // and then this flag will reset.
//            mHasPendingQuery = true;
//        }
//    }
//
//    /**
//     * M: initialize the tab-styled local search UI
//     */
//    private void initTabs() {
//        if (mSearchFieldTabs != null) {
//            return;
//        }
//        Context context = mActivity.getApplicationContext();
//        mSearchFieldTabs = new HorizontalScrollView(mContext);
//        mSearchFieldTabs.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
//                ViewGroup.LayoutParams.WRAP_CONTENT));
//        mSearchFieldTabs.setBackgroundColor(context.getResources().getColor(R.color.actionbar_color));
//        mSearchFieldTabs.setVisibility(View.VISIBLE);
//        // backup current field, cause addTab would change mSearchField;
//        String currentField = mSearchField;
//
//        // Clear old tabs before we add new tab.
//        LogUtils.logFeature(LogTag.SEARCH_TAG,
//                "Before initTabs remove old Tabs, current status: search field [%s]", mSearchField);
//        mSearchFieldTabs.removeAllViews();
//        LinearLayout layout = new LinearLayout(context, null, android.R.attr.actionBarTabBarStyle);
//        layout.setOrientation(LinearLayout.HORIZONTAL);
//        layout.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
//                    ViewGroup.LayoutParams.MATCH_PARENT));
//        String[] searchFieldList = context.getResources().getStringArray(R.array.search_field_list);
//        for (int i = 0; i < searchFieldList.length; i++) {
//            MailSearchTabView tab = new MailSearchTabView(context, null, 0);
//            tab.setText(searchFieldList[i]);
//            tab.setTabListener(mTabListener);
//            tab.setTag(SEARCH_FIELD_LIST[i]);
//            tab.setPosition(i);
//            tab.setHeight(mActivity.getSupportActionBar().getHeight() * 2 / 3);
//            tab.setGravity(Gravity.CENTER);
//            tab.setOnClickListener(this);
//            layout.addView(tab, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
//                    ViewGroup.LayoutParams.WRAP_CONTENT));
//            if (currentField.equals(SEARCH_FIELD_LIST[i])) {
//                tab.setSelected(true);
//                mSelectedTab = tab;
//            }
//        }
//        mSearchFieldTabs.addView(layout, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
//                ViewGroup.LayoutParams.WRAP_CONTENT));
//        mSearchFieldTabs.setHorizontalScrollBarEnabled(false);
//        final LinearLayout listLayout = (LinearLayout) mActivity
//                .findViewById(R.id.list_content_view);
//        // adding tabs layout dynamic, we place it nearby toolbar which index was 1.
//        listLayout.addView(mSearchFieldTabs, 1,
//                new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
//                        ViewGroup.LayoutParams.WRAP_CONTENT));
//        mSearchField = currentField;
//    }
//
//    /**
//     * M: initialize the dropdownlist-style local search UI(for tablet)
//     */
//    private void initSpinner() {
//        ArrayList<String> items = new ArrayList<String>();
//        Context context = mActivity.getApplicationContext();
//        String[] searchFieldList = context.getResources().getStringArray(R.array.search_field_list);
//        for (String field : searchFieldList) {
//            items.add(field);
//        }
//
//        ListAdapter adapter = new ArrayAdapter<String>(context, R.layout.search_fields_spinner, items);
//
//        // field dropdown
//        mSearchFiledSpinner.setVisibility(View.VISIBLE);
//        mSearchFiledDropDown = new SearchFieldDropdownPopup(getContext(), mSearchFiledSpinner);
//        mSearchFiledDropDown.setAdapter(adapter);
//
//        mSearchFiledSpinner.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                mSearchFiledDropDown.show();
//            }
//        });
//        for (int i = 0; i < SEARCH_FIELD_LIST.length; i++) {
//            if (SEARCH_FIELD_LIST[i].equals(mSearchField)) {
//                mSearchFiledSpinner.setText(items.get(i));
//                break;
//            }
//        }
//    }
//
//    /**
//     * M: record current selected tab.
//     */
//    public class TabListener {
//        /* The following are each of the ActionBar.TabListener callbacks */
//        public void onTabSelected(MailSearchTabView tab) {
//            mSearchField = (String) tab.getTag();
//            mController.enterLocalSearch(mSearchField);
//            String query = mSearchWidget.getQuery().toString();
//            if (!TextUtils.isEmpty(query)) {
//                onQueryTextChange(query);
//            }
//        }
//    }
//
//    private boolean onPopupFieldsItemSelected(int itemPosition, View v) {
//        String searchField = SEARCH_FIELD_LIST[itemPosition];
//        mSearchFiledSpinner.setText(((TextView) v).getText());
//        mSearchField = searchField;
//        mController.enterLocalSearch(mSearchField);
//        onQueryTextChange(mSearchWidget.getQuery().toString());
//        return true;
//    }
//
//    // Based on Spinner.DropdownPopup
//    private class SearchFieldDropdownPopup extends ListPopupWindow {
//        public SearchFieldDropdownPopup(Context context, View anchor) {
//            super(context);
//            setAnchorView(anchor);
//            setModal(true);
//            setPromptPosition(POSITION_PROMPT_ABOVE);
//            setOnItemClickListener(new OnItemClickListener() {
//                public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
//                    onPopupFieldsItemSelected(position, v);
//                    dismiss();
//                }
//            });
//        }
//
//        @Override
//        public void show() {
//            setWidth(getContext().getResources().getDimensionPixelSize(R.dimen.search_fields_popup_width));
//            setInputMethodMode(ListPopupWindow.INPUT_METHOD_NOT_NEEDED);
//            super.show();
//            // List view is instantiated in super.show(), so we need to do this after...
//            getListView().setChoiceMode(ListView.CHOICE_MODE_SINGLE);
//        }
//    }
//
//    /**
//     * M: call to update search result count.
//     */
//    public void updateSearchCount(int count) {
//        mLocalSearchResult = count;
//        mSearchResultCountView.setText(String.valueOf(mLocalSearchResult));
//    }
//
//    /**
//     * M: get the query term if current search field were "body" or "all", otherwise returns null
//     */
//    public String getQueryTermIfSearchBody() {
//        String selectedTab;
//        if (!TextUtils.isEmpty(mSearchField)) {
//            selectedTab = mSearchField;
//        } else {
//            return null;
//        }
//
//        return (selectedTab.equalsIgnoreCase(SearchParams.SEARCH_FIELD_BODY) || selectedTab
//                .equalsIgnoreCase(SearchParams.SEARCH_FIELD_ALL)) ? mSearchWidget.getQuery().toString() : null;
//    }
//
//    private boolean useTabMode(Context context) {
//        int orientation = context.getResources().getConfiguration().orientation;
//        boolean isPortrait = orientation == Configuration.ORIENTATION_PORTRAIT;
//        boolean isOnePane = mController.isDrawerEnabled();
//        return isPortrait && isOnePane;
//    }
//
//    private void initLocalSearchView() {
//        if (useTabMode(mActivity.getApplicationContext())) {
//            initTabs();
//        } else {
//            initSpinner();
//        }
//    }
//
//    private void removeLocalSearchView() {
//        if (useTabMode(mActivity.getApplicationContext())) {
//            final LinearLayout listLayout = (LinearLayout) mActivity
//                    .findViewById(R.id.list_content_view);
//            listLayout.removeView(mSearchFieldTabs);
//            mSelectedTab = null;
//            mSearchFieldTabs = null;
//        }
//        mSearchFiledSpinner.setVisibility(View.GONE);
//    }
//
//    private class OnSearchItemClickListener implements OnMenuItemClickListener {
//        @Override
//        public boolean onMenuItemClick(MenuItem item) {
//            mSearchField = SEARCH_FIELD_LIST[INITIAL_FIELD_INDEX];
//            updateSearchCount(0);
//            mController.enterLocalSearch(mSearchField);
//            initLocalSearchView();
//            return true;
//        }
//    }
//
//    public void selectTab(MailSearchTabView tab) {
//        if (mSelectedTab != tab) {
//            if (mSelectedTab != null) {
//                mSelectedTab.setSelected(false);
//            }
//            mSelectedTab = tab;
//            if (mSelectedTab != null) {
//                mSelectedTab.getCallback().onTabSelected(mSelectedTab);
//            }
//        }
//    }
//
//    /* M: just listening search fields tab view clicking, and do some reactions.
//     * @see android.view.View.OnClickListener#onClick(android.view.View)
//     */
//    @Override
//    public void onClick(View v) {
//        MailSearchTabView tab = (MailSearchTabView)v;
//        tab.setSelected(true);
//        selectTab(tab);
//    }
//
//    /**
//     * M: An search field tab text view, which inherit system's actionbar tab style
//     * and could be selected
//     * @author mtk54113
//     *
//     */
//    public class MailSearchTabView extends TextView {
//
//        private TabListener mCallback;
//        private Object mTag;
//        private Drawable mIcon;
//        private CharSequence mText;
//        private CharSequence mContentDesc;
//        private int mPosition = -1;
//        private View mCustomView;
//
//        public MailSearchTabView(Context context) {
//            super(context);
//        }
//
//        public MailSearchTabView(Context context, AttributeSet attrs, int defStyle) {
//            super(context, attrs, android.R.attr.actionBarTabStyle);
//        }
//
//        public TabListener getCallback() {
//            return mCallback;
//        }
//
//        public void setTabListener(TabListener callback) {
//            mCallback = callback;
//        }
//
//        public int getPosition() {
//            return mPosition;
//        }
//
//        public void setPosition(int position) {
//            mPosition = position;
//        }
//
//        public void select() {
//            selectTab(this);
//        }
//    }
//
//    /**
//     * Use throttle to avoid throw too many query, when user keep input or delete query.
//     * same to a delay when query changed.
//     */
//    private static final int MIN_QUERY_INTERVAL = 200;
//    private static final int MAX_QUERY_INTERVAL = 500;
//
//    private final Throttle mLocalSearchThrottle = new Throttle("EmailActionBarView",
//            new Runnable() {
//                @Override public void run() {
//                    if (null != mController && null != mSearchWidget
//                            && null != mQueryTextString) {
//                        /// M: mSearchWidget.getQuery() may get nothing, so use the backup string
//                        mController.executeLocalSearch(mQueryTextString);
//                    }
//                }
//            }, Utility.getMainThreadHandler(),
//            MIN_QUERY_INTERVAL, MAX_QUERY_INTERVAL);
//}
