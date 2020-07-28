/*
 * Copyright (C) 2014 Google Inc.
 * Licensed to The Android Open Source Project.
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

package com.android.mail.ui;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.os.AsyncTask;
import android.os.Bundle;
import android.speech.RecognizerIntent;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.MenuItem.OnMenuItemClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.ListAdapter;
import android.widget.ListPopupWindow;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

import com.android.emailcommon.service.SearchParams;
import com.android.emailcommon.utility.Utility;
import com.android.mail.ConversationListContext;
import com.android.mail.R;
import com.android.mail.providers.SearchRecentSuggestionsProvider;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.Throttle;
import com.android.mail.utils.ViewUtils;

import java.util.ArrayList;
import java.util.Locale;

/**
 * Controller for interactions between ActivityController and our custom search views.
 */
public class MaterialSearchViewController implements ViewMode.ModeChangeListener,
        TwoPaneLayout.ConversationListLayoutListener {
    private static final long FADE_IN_OUT_DURATION_MS = 150;

    // The controller is not in search mode. Both search action bar and the suggestion list
    // are not visible to the user.
    public static final int SEARCH_VIEW_STATE_GONE = 0;
    // The controller is actively in search (as in the action bar is focused and the user can type
    // into the search query). Both the search action bar and the suggestion list are visible.
    public static final int SEARCH_VIEW_STATE_VISIBLE = 1;
    // The controller is in a search ViewMode but not actively searching. This is relevant when
    // we have to show the search actionbar on top while the user is not interacting with it.
    public static final int SEARCH_VIEW_STATE_ONLY_ACTIONBAR = 2;

    private static final String EXTRA_CONTROLLER_STATE = "extraSearchViewControllerViewState";

    private MailActivity mActivity;
    private ActivityController mController;

    private SearchRecentSuggestionsProvider mSuggestionsProvider;

    private MaterialSearchActionView mSearchActionView;
    private MaterialSearchSuggestionsList mSearchSuggestionList;

    private int mViewMode;
    private int mControllerState;
    private int mEndXCoordForTabletLandscape;

    private boolean mSavePending;
    private boolean mDestroyProvider;

    /** M: Add local search feature. @{ */
    private TabListener mTabListener = new TabListener();
    private String mSearchField;
    private static final String[] SEARCH_FIELD_LIST = { SearchParams.SEARCH_FIELD_ALL,
            SearchParams.SEARCH_FIELD_FROM, SearchParams.SEARCH_FIELD_TO,
            SearchParams.SEARCH_FIELD_SUBJECT, SearchParams.SEARCH_FIELD_BODY };
    private static final int INITIAL_FIELD_INDEX = 1;

    private TextView mSearchResultCountView;
    private TextView mSearchFiledSpinner;
    private HorizontalScrollView mSearchFieldTabs;
    private MailSearchTabView mSelectedTab;
    private SearchFieldDropdownPopup mSearchFiledDropDown;
    // Backup the query string of the UI.
    private String mQueryTextString = null;

    /** @}*/

    public MaterialSearchViewController(MailActivity activity, ActivityController controller,
            Intent intent, Bundle savedInstanceState) {
        mActivity = activity;
        mController = controller;

        final Intent voiceIntent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        final boolean supportVoice =
                voiceIntent.resolveActivity(mActivity.getPackageManager()) != null;

        mSuggestionsProvider = mActivity.getSuggestionsProvider();
        mSearchSuggestionList = (MaterialSearchSuggestionsList) mActivity.findViewById(
                R.id.search_overlay_view);
        /// M: disable search suggestion due to conflict with local search.
        // mSearchSuggestionList.setController(this, mSuggestionsProvider);
        mSearchActionView = (MaterialSearchActionView) mActivity.findViewById(
                R.id.search_actionbar_view);
        mSearchActionView.setController(this, intent.getStringExtra(
                ConversationListContext.EXTRA_SEARCH_QUERY), supportVoice);

        if (savedInstanceState != null && savedInstanceState.containsKey(EXTRA_CONTROLLER_STATE)) {
            mControllerState = savedInstanceState.getInt(EXTRA_CONTROLLER_STATE);
        }

        mActivity.getViewMode().addListener(this);
        /// M: initialize of local search view. @{
        mSearchResultCountView = (TextView) mActivity.findViewById(R.id.result_count);
        mSearchResultCountView.setText(String.valueOf(0));
        mSearchFiledSpinner = (TextView) mActivity.findViewById(R.id.search_field);
        /// @}
    }

    /**
     * This controller should not be used after this is called.
     */
    public void onDestroy() {
        mDestroyProvider = mSavePending;
        if (!mSavePending) {
            mSuggestionsProvider.cleanup();
        }
        mActivity.getViewMode().removeListener(this);
        mActivity = null;
        mController = null;
        mSearchActionView = null;
        mSearchSuggestionList = null;
        /// M: when onDestroy ,dismiss the SearchFiledDropDown
        if(mSearchFiledDropDown != null) {
            mSearchFiledDropDown.dismiss();
        }
    }

    public void saveState(Bundle outState) {
        outState.putInt(EXTRA_CONTROLLER_STATE, mControllerState);
    }

    @Override
    public void onViewModeChanged(int newMode) {
        final int oldMode = mViewMode;
        mViewMode = newMode;
        // Never animate visibility changes that are caused by view state changes.
        if (mController.shouldShowSearchBarByDefault(mViewMode)) {
            showSearchActionBar(SEARCH_VIEW_STATE_ONLY_ACTIONBAR, false /* animate */);
        } else if (oldMode == ViewMode.UNKNOWN) {
            showSearchActionBar(mControllerState, false /* animate */);
        } else if (mViewMode == ViewMode.CONVERSATION_LIST
                && mController.getCurrentListContext() != null && mController.getCurrentListContext().isLocalSearch()) {
            /// M: add case for back from conversation view.
            showSearchActionBar(SEARCH_VIEW_STATE_VISIBLE, false);
        } else {
            showSearchActionBar(SEARCH_VIEW_STATE_GONE, false /* animate */);
        }
    }

    @Override
    public void onConversationListLayout(int xEnd, boolean drawerOpen) {
        // Only care about the first layout
        if (mEndXCoordForTabletLandscape != xEnd) {
            // This is called when we get into tablet landscape mode
            mEndXCoordForTabletLandscape = xEnd;
            if (ViewMode.isSearchMode(mViewMode)) {
                final int defaultVisibility = mController.shouldShowSearchBarByDefault(mViewMode) ?
                        View.VISIBLE : View.GONE;
                setViewVisibilityAndAlpha(mSearchActionView,
                        drawerOpen ? View.INVISIBLE : defaultVisibility);
            }
            adjustViewForTwoPaneLandscape();
        }
    }

    public boolean handleBackPress() {
        final boolean shouldShowSearchBar = mController.shouldShowSearchBarByDefault(mViewMode);
        if (shouldShowSearchBar && mSearchSuggestionList.isShown()) {
            showSearchActionBar(SEARCH_VIEW_STATE_ONLY_ACTIONBAR);
            return true;
        } else if (!shouldShowSearchBar && mSearchActionView.isShown()) {
            showSearchActionBar(SEARCH_VIEW_STATE_GONE);
            return true;
        }
        return false;
    }

    /**
     * Set the new visibility state of the search controller.
     * @param state the new view state, must be one of the following options:
     *   {@link MaterialSearchViewController#SEARCH_VIEW_STATE_ONLY_ACTIONBAR},
     *   {@link MaterialSearchViewController#SEARCH_VIEW_STATE_VISIBLE},
     *   {@link MaterialSearchViewController#SEARCH_VIEW_STATE_GONE},
     */
    public void showSearchActionBar(int state) {
        // By default animate the visibility changes
        showSearchActionBar(state, true /* animate */);
    }

    /**
     * @param animate if true, the search bar and suggestion list will fade in/out of view.
     */
    public void showSearchActionBar(int state, boolean animate) {
        mControllerState = state;
        /// M: record previous visibility.
        int lastSearchVisibility = mSearchActionView.getVisibility();

        // ACTIONBAR is only applicable in search mode
        final boolean onlyActionBar = state == SEARCH_VIEW_STATE_ONLY_ACTIONBAR &&
                mController.shouldShowSearchBarByDefault(mViewMode);
        final boolean isStateVisible = state == SEARCH_VIEW_STATE_VISIBLE;

        final boolean isSearchBarVisible = isStateVisible || onlyActionBar;

        final int searchBarVisibility = isSearchBarVisible ? View.VISIBLE : View.GONE;
        final int suggestionListVisibility = isStateVisible ? View.VISIBLE : View.GONE;
        if (animate) {
            fadeInOutView(mSearchActionView, searchBarVisibility);
            /// M: disable search suggestion.
            // fadeInOutView(mSearchSuggestionList, suggestionListVisibility);
        } else {
            setViewVisibilityAndAlpha(mSearchActionView, searchBarVisibility);
            /// M: disable search suggestion.
            // setViewVisibilityAndAlpha(mSearchSuggestionList, suggestionListVisibility);
        }
        mSearchActionView.focusSearchBar(isStateVisible);

        final boolean useDefaultColor = !isSearchBarVisible || shouldAlignWithTl();
        final int statusBarColor = useDefaultColor ? R.color.mail_activity_status_bar_color :
                R.color.search_status_bar_color;
        ViewUtils.setStatusBarColor(mActivity, statusBarColor);

        // Specific actions for each view state
        if (onlyActionBar) {
            adjustViewForTwoPaneLandscape();
        } else if (isStateVisible) {
            // Set to default layout/assets
            mSearchActionView.adjustViewForTwoPaneLandscape(false /* do not align */, 0);
        } else {
            // For non-search view mode, clear the query term for search
            /// M: don't clear query when enter conversation mode.
            if (!ViewMode.isSearchMode(mViewMode) && mViewMode != ViewMode.CONVERSATION) {
                mSearchActionView.clearSearchQuery();
            }
        }

        /// M: start processing local search part. @{
        if (lastSearchVisibility != searchBarVisibility
                && state != SEARCH_VIEW_STATE_ONLY_ACTIONBAR) {
            if (searchBarVisibility == View.VISIBLE) {
                ConversationListContext listContext = mController
                        .getCurrentListContext();
                enterLocalSearch(mQueryTextString,
                        listContext != null ? listContext.getSearchField()
                                : null);
            } else if (mViewMode == ViewMode.CONVERSATION) {
                removeLocalSearchView();
            } else {
                exitLocalSearch();
            }
        }
        /// @}
    }

    /**
     * Helper function to fade in/out the provided view by animating alpha.
     */
    private void fadeInOutView(final View v, final int visibility) {
        if (visibility == View.VISIBLE) {
            v.setVisibility(View.VISIBLE);
            v.animate()
                    .alpha(1f)
                    .setDuration(FADE_IN_OUT_DURATION_MS)
                    .setListener(null);
        } else {
            v.animate()
                    .alpha(0f)
                    .setDuration(FADE_IN_OUT_DURATION_MS)
                    .setListener(new AnimatorListenerAdapter() {
                        @Override
                        public void onAnimationEnd(Animator animation) {
                            v.setVisibility(visibility);
                        }
                    });
        }
    }

    /**
     * Sets the view's visibility and alpha so that we are guaranteed that alpha = 1 when the view
     * is visible, and alpha = 0 otherwise.
     */
    private void setViewVisibilityAndAlpha(View v, int visibility) {
        v.setVisibility(visibility);
        if (visibility == View.VISIBLE) {
            v.setAlpha(1f);
        } else {
            v.setAlpha(0f);
        }
    }

    private boolean shouldAlignWithTl() {
        return mController.isTwoPaneLandscape() &&
                mControllerState == SEARCH_VIEW_STATE_ONLY_ACTIONBAR &&
                ViewMode.isSearchMode(mViewMode);
    }

    private void adjustViewForTwoPaneLandscape() {
        // Try to adjust if the layout happened already
        if (mEndXCoordForTabletLandscape != 0) {
            mSearchActionView.adjustViewForTwoPaneLandscape(shouldAlignWithTl(),
                    mEndXCoordForTabletLandscape);
        }
    }

    public void onQueryTextChanged(String query) {
        /// M: execute local search. @{
        mQueryTextString = query;
        /**
         * Not start local search immediately, use Throttle control the query event.
         */
        LogUtils.logFeature(LogTag.SEARCH_TAG, "onQueryTextChange [%s]", query);
        mLocalSearchThrottle.onEvent();
        /// @}
    }

    public void onSearchCanceled() {
        // Special case search mode
        if (ViewMode.isSearchMode(mViewMode)) {
            mActivity.setResult(Activity.RESULT_OK);
            mActivity.finish();
        } else {
            mSearchActionView.clearSearchQuery();
            showSearchActionBar(SEARCH_VIEW_STATE_GONE);
        }
    }

    public void onSearchPerformed(String query) {
        query = query.trim();
        if (!TextUtils.isEmpty(query)) {
            /// M: set query words back to action view, support voice search.
            mSearchActionView.setSearchQuery(query);
            /// M: execute local search instead remote search.@{
            mLocalSearchThrottle.onEvent();
            // Follow JB, hide the input method, when press on the submit/search keyboard.
            if (mActivity != null) {
                final InputMethodManager imm = (InputMethodManager) mActivity
                        .getActivityContext().getSystemService(Context.INPUT_METHOD_SERVICE);
                if (imm != null && imm.isActive()) {
                    imm.hideSoftInputFromWindow(mSearchActionView.getWindowToken(), 0);
                }
            }
            /// @}
        }
    }

    public void onVoiceSearch() {
        final Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,
                RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE, Locale.getDefault().getLanguage());

        // Some devices do not support the voice-to-speech functionality.
        try {
            mActivity.startActivityForResult(intent,
                    AbstractActivityController.VOICE_SEARCH_REQUEST_CODE);
        } catch (ActivityNotFoundException e) {
            final String toast =
                    mActivity.getResources().getString(R.string.voice_search_not_supported);
            Toast.makeText(mActivity, toast, Toast.LENGTH_LONG).show();
        }
    }

    public void saveRecentQuery(String query) {
        /// M: disable search suggestion.
        // new SaveRecentQueryTask().execute(query);
    }

     // static asynctask to save the query in the background.
    private class SaveRecentQueryTask extends AsyncTask<String, Void, Void> {

        @Override
        protected void onPreExecute() {
            mSavePending = true;
        }

        @Override
        protected Void doInBackground(String... args) {
            mSuggestionsProvider.saveRecentQuery(args[0]);
            return null;
        }

        @Override
        protected void onPostExecute(Void aVoid) {
            if (mDestroyProvider) {
                mSuggestionsProvider.cleanup();
                mDestroyProvider = false;
            }
            mSavePending = false;
        }
    }

    /**
     * M: Add local search related source code here. @{
     * */
    private void enterLocalSearch(String query, String field) {
        // support expand search for specific field
        if (field != null) {
            mSearchField = field;
        } else {
            mSearchField = SEARCH_FIELD_LIST[INITIAL_FIELD_INDEX];
        }
        mController.enterLocalSearch(mSearchField);
        initLocalSearchView();
    }

    private void exitLocalSearch() {
        mController.exitLocalSearch();
        removeLocalSearchView();
        updateSearchCount(0);
    }

    public void setQueryTextString(String query) {
        mQueryTextString = query;
        mSearchActionView.setSearchQuery(query);
    }

    private boolean onPopupFieldsItemSelected(int itemPosition, View v) {
        String searchField = SEARCH_FIELD_LIST[itemPosition];
        mSearchFiledSpinner.setText(((TextView) v).getText());
        mSearchField = searchField;
        mController.enterLocalSearch(mSearchField);
        onQueryTextChanged(mQueryTextString);
        return true;
    }

    // Based on Spinner.DropdownPopup
    private class SearchFieldDropdownPopup extends ListPopupWindow {
        public SearchFieldDropdownPopup(Context context, View anchor) {
            super(context);
            setAnchorView(anchor);
            setModal(true);
            setPromptPosition(POSITION_PROMPT_ABOVE);
            setOnItemClickListener(new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                    onPopupFieldsItemSelected(position, v);
                    dismiss();
                }
            });
        }

        @Override
        public void show() {
            setWidth(mActivity.getResources().getDimensionPixelSize(R.dimen.search_fields_popup_width));
            setInputMethodMode(ListPopupWindow.INPUT_METHOD_NOT_NEEDED);
            super.show();
            // List view is instantiated in super.show(), so we need to do this after...
            getListView().setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        }
    }

    /**
     * M: An search field tab text view, which inherit system's actionbar tab style
     * and could be selected
     */
    private class MailSearchTabView extends TextView {

        private TabListener mCallback;
        private Object mTag;
        private Drawable mIcon;
        private CharSequence mText;
        private CharSequence mContentDesc;
        private int mPosition = -1;
        private View mCustomView;

        public MailSearchTabView(Context context) {
            super(context);
        }

        public MailSearchTabView(Context context, AttributeSet attrs, int defStyle) {
            super(context, attrs, android.R.attr.actionBarTabStyle);
        }

        public TabListener getCallback() {
            return mCallback;
        }

        public void setTabListener(TabListener callback) {
            mCallback = callback;
        }

        public int getPosition() {
            return mPosition;
        }

        public void setPosition(int position) {
            mPosition = position;
        }

        public void select() {
            selectTab(this);
        }
    }

    public void selectTab(MailSearchTabView tab) {
        if (mSelectedTab != tab) {
            if (mSelectedTab != null) {
                mSelectedTab.setSelected(false);
            }
            mSelectedTab = tab;
            if (mSelectedTab != null) {
                mSelectedTab.getCallback().onTabSelected(mSelectedTab);
            }
        }
    }

    /**
     * M: record current selected tab.
     */
    private class TabListener {
        /* The following are each of the ActionBar.TabListener callbacks */
        public void onTabSelected(MailSearchTabView tab) {
            mSearchField = (String) tab.getTag();
            mController.enterLocalSearch(mSearchField);
            String query = mQueryTextString;
            if (!TextUtils.isEmpty(query)) {
                onQueryTextChanged(query);
            }
        }
    }

    private class OnSearchItemClickListener implements OnMenuItemClickListener {
        @Override
        public boolean onMenuItemClick(MenuItem item) {
            mSearchField = SEARCH_FIELD_LIST[INITIAL_FIELD_INDEX];
            updateSearchCount(0);
            mController.enterLocalSearch(mSearchField);
            initLocalSearchView();
            return true;
        }
    }

    private void removeLocalSearchView() {
        if (useTabMode(mActivity.getApplicationContext())) {
            final LinearLayout listLayout = (LinearLayout) mActivity
                    .findViewById(R.id.list_content_view);
            listLayout.removeView(mSearchFieldTabs);
            mSelectedTab = null;
            mSearchFieldTabs = null;
        }
        mSearchFiledSpinner.setVisibility(View.GONE);
    }

    private void initLocalSearchView() {
        if (useTabMode(mActivity.getApplicationContext())) {
            initTabs();
        } else {
            initSpinner();
        }
    }

    private boolean useTabMode(Context context) {
        int orientation = context.getResources().getConfiguration().orientation;
        boolean isPortrait = orientation == Configuration.ORIENTATION_PORTRAIT;
        boolean isOnePane = mController.isDrawerEnabled();
        return isPortrait && isOnePane;
    }

    /**
     * M: call to update search result count.
     */
    public void updateSearchCount(int count) {
        mSearchResultCountView.setText(String.valueOf(count));
    }

    /**
     * M: initialize the tab-styled local search UI
     */
    private void initTabs() {
        if (mSearchFieldTabs != null) {
            return;
        }
        Context context = mActivity.getApplicationContext();
        mSearchFieldTabs = new HorizontalScrollView(context);
        mSearchFieldTabs.setLayoutParams(new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        mSearchFieldTabs.setBackgroundColor(context.getResources()
                .getColor(R.color.search_status_bar_color));
        mSearchFieldTabs.setVisibility(View.VISIBLE);
        // backup current field, cause addTab would change mSearchField;
        String currentField = mSearchField;

        // Clear old tabs before we add new tab.
        LogUtils.logFeature(LogTag.SEARCH_TAG,
                "Before initTabs remove old Tabs, current status: search field [%s]", mSearchField);
        mSearchFieldTabs.removeAllViews();
        LinearLayout layout = new LinearLayout(context, null, android.R.attr.actionBarTabBarStyle);
        layout.setOrientation(LinearLayout.HORIZONTAL);
        layout.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        String[] searchFieldList = context.getResources().getStringArray(R.array.search_field_list);
        TypedArray actionBarSizeTypedArray = mActivity.obtainStyledAttributes(
                                     new int[] { android.R.attr.actionBarSize });
        int actionbarSize = (int) actionBarSizeTypedArray.getDimension(0, 0f);
        for (int i = 0; i < searchFieldList.length; i++) {
            MailSearchTabView tab = new MailSearchTabView(context, null, 0);
            tab.setText(searchFieldList[i]);
            tab.setTabListener(mTabListener);
            tab.setTag(SEARCH_FIELD_LIST[i]);
            tab.setPosition(i);
            tab.setHeight(actionbarSize * 2 / 3);
            tab.setGravity(Gravity.CENTER);
            tab.setOnClickListener(new View.OnClickListener() {
                // just listening search fields tab view clicking, and do some reactions.
                @Override
                public void onClick(View v) {
                    MailSearchTabView tab = (MailSearchTabView) v;
                    tab.setSelected(true);
                    selectTab(tab);
                }
            });
            layout.addView(tab, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT));
            if (currentField.equals(SEARCH_FIELD_LIST[i])) {
                tab.setSelected(true);
                mSelectedTab = tab;
            }
        }
        mSearchFieldTabs.addView(layout, new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        mSearchFieldTabs.setHorizontalScrollBarEnabled(false);
        final LinearLayout listLayout = (LinearLayout) mActivity
                .findViewById(R.id.list_content_view);
        // adding tabs layout dynamic, we place it nearby toolbar which index was 1.
        listLayout.addView(mSearchFieldTabs, 1, new ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        mSearchField = currentField;
    }

    /**
     * M: initialize the dropdownlist-style local search UI(for tablet)
     */
    private void initSpinner() {
        ArrayList<String> items = new ArrayList<String>();
        Context context = mActivity.getApplicationContext();
        String[] searchFieldList = context.getResources().getStringArray(R.array.search_field_list);
        for (String field : searchFieldList) {
            items.add(field);
        }

        ListAdapter adapter = new ArrayAdapter<String>(context, R.layout.search_fields_spinner,
                items);

        // field dropdown
        mSearchFiledSpinner.setVisibility(View.VISIBLE);
        mSearchFiledDropDown = new SearchFieldDropdownPopup(mActivity, mSearchFiledSpinner);
        mSearchFiledDropDown.setAdapter(adapter);

        mSearchFiledSpinner.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mSearchFiledDropDown.show();
            }
        });
        for (int i = 0; i < SEARCH_FIELD_LIST.length; i++) {
            if (SEARCH_FIELD_LIST[i].equals(mSearchField)) {
                mSearchFiledSpinner.setText(items.get(i));
                break;
            }
        }
    }

    /**
     * M: Use throttle to avoid throw too many query, when user keep input or delete query.
     * same to a delay when query changed.
     */
    private static final int MIN_QUERY_INTERVAL = 200;
    private static final int MAX_QUERY_INTERVAL = 500;

    private final Throttle mLocalSearchThrottle = new Throttle("EmailActionBarView",
            new Runnable() {
                @Override public void run() {
                    if (null != mController && null != mQueryTextString) {
                        /// M: mSearchWidget.getQuery() may get nothing, so use the backup string
                        mController.executeLocalSearch(mQueryTextString);
                    }
                }
            }, Utility.getMainThreadHandler(),
            MIN_QUERY_INTERVAL, MAX_QUERY_INTERVAL);

    /** @}*/
}
