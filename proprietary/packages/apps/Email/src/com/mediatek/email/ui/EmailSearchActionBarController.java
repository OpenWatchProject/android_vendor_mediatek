//package com.mediatek.email.ui;
//
//import android.content.Context;
//import android.support.v7.app.ActionBar;
//import android.text.TextUtils;
//import android.util.AttributeSet;
//import android.view.Menu;
//import android.view.MenuItem;
//import android.view.View;
//import android.view.ViewGroup;
//import android.widget.SearchView;
//import android.widget.TextView;
//
//import com.android.email.R;
//import com.android.mail.ConversationListContext;
//import com.android.mail.providers.Folder;
//import com.android.mail.ui.ActivityController;
//import com.android.mail.ui.ControllableActivity;
//import com.android.mail.ui.SearchActionBarController;
//import com.android.mail.ui.ViewMode;
//import com.android.mail.utils.LogTag;
//import com.android.mail.utils.LogUtils;
//import com.android.mail.utils.Utils;
//
///**
// * M: The EmailSearchMailActionBarView class is overlay the SearchMailActionBarView.
// * To handle the remote search on action bar and simplify the original design
// * Original:
// *      User could enter remote search and issue search with different text by searchView
// * New:
// *      User could only enter remote search by local search with query text of it,
// *      Which means remote search is a enhancement of local search, user could search mails
// *      on server when they did not find what he wants
// */
//public class EmailSearchActionBarController extends SearchActionBarController {
//    private TextView mSearchResultCountView;
//    private TextView mSearchTextView;
//    private int mRemoteSearchResult = 0;
//
//    public EmailSearchActionBarController(Context context) {
//        super(context);
//    }
//
//    @Override
//    public void initialize(ControllableActivity activity,
//            ActivityController callback, ActionBar actionBar) {
//        super.initialize(activity, callback, actionBar);
//        mSearchResultCountView = (TextView) findViewById(R.id.remote_result_count);
//        mSearchResultCountView.setText(String.valueOf(mRemoteSearchResult));
//        mSearchTextView = (TextView) findViewById(R.id.remote_search_title);
//        LogUtils.logFeature(LogTag.SEARCH_TAG, "EmailActionBarView initialize for remote search result");
//    }
//
//    @Override
//    public void onViewModeChanged(int newMode) {
//        LogUtils.logFeature(LogTag.SEARCH_TAG, "EmailSearchMailActionBarView newMode: %d", newMode);
//        super.onViewModeChanged(newMode);
//        switch (getMode()) {
//            case ViewMode.SEARCH_RESULTS_LIST:
//                final int current = mActionBar.getDisplayOptions();
//                final int mask = ActionBar.DISPLAY_SHOW_CUSTOM | current;
//                mActionBar.setDisplayOptions(mask, mask);
//                setTitleModeFlags(ActionBar.DISPLAY_SHOW_CUSTOM);
//                break;
//            default:
//                LogUtils.logFeature(LogTag.SEARCH_TAG, "EmailSearchMailActionBarView invalid mode: %d", newMode);
//                break;
//        }
//    }
//
//    @Override
//    public boolean onPrepareOptionsMenu(Menu menu) {
//        LogUtils.logFeature(LogTag.SEARCH_TAG, "EmailSearchActionBarView onPrepareOptionsMenu with viewMode: %d", getMode());
//        super.onPrepareOptionsMenu(menu);
//        if (ViewMode.isSearchMode(getMode())) {
//            setSearchQueryTerm();
//        }
//        return true;
//    }
//
//    @Override
//    public void onFolderUpdated(Folder folder) {
//        if (folder == null) {
//            LogUtils.logFeature(LogTag.SEARCH_TAG, "onFolderUpdated with invalid Folder");
//            return;
//        }
//        mFolder = folder;
//    }
//
//    /**
//     * call to update search result count.
//     */
//    public void updateSearchCount(int count) {
//        mRemoteSearchResult  = count;
//        mSearchResultCountView.setText(String.valueOf(mRemoteSearchResult));
//    }
//
//    /**
//     * Sets the query term in the text field, so the user can see what was searched for.
//     */
//    private void setSearchQueryTerm() {
//        final String query = mActivity.getIntent().getStringExtra(
//                ConversationListContext.EXTRA_SEARCH_QUERY);
//        final String searchTitle = mContext.getString(R.string.searching_on_server_title, query);
//        LogUtils.logFeature(LogTag.SEARCH_TAG, "EmailSearchMailActionBarView query Text [%s]", searchTitle);
//        if (!TextUtils.isEmpty(query)) {
//            mSearchTextView.setText(searchTitle);
//        }
//    }
//}
