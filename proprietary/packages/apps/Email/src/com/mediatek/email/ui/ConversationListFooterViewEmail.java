package com.mediatek.email.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

import com.android.email.R;
import com.android.mail.ConversationListContext;
import com.android.mail.browse.ConversationCursor;
import com.android.mail.browse.ConversationListFooterView;
import com.android.mail.providers.Account;
import com.android.mail.providers.Folder;
import com.android.mail.providers.UIProvider;
import com.android.mail.providers.UIProvider.AccountCapabilities;
import com.android.mail.ui.ActivityController;
import com.android.mail.ui.ControllableActivity;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;

/**
 * M: Override ConversationListFooterView, add remote search view.
 */
public class ConversationListFooterViewEmail extends ConversationListFooterView {
    /// M: add for local search feature.
    private View mRemoteSearch;

    public ConversationListFooterViewEmail(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mRemoteSearch = (View) findViewById(R.id.remote_search);
        mRemoteSearch.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        super.onClick(v);
        final int id = v.getId();
        final Folder f = (Folder) v.getTag();
        if (id == R.id.remote_search) {
            mClickListener.onFooterViewRemoteSearchClick(f);
        }
    }

    /**
     * Update the view to reflect the new folder status.
     */
    @Override
    public boolean updateStatus(final ConversationCursor cursor, final boolean currentFooterShown) {
        Folder folder = (Folder) mLoadMore.getTag();

        /// M: disable update footer view (loadmore/loading/network error),
        ///    since current mailbox is not syncable. @{
        if (folder != null && !folder.isSyncable()) {
            LogUtils.d(LogTag.getLogTag(),
                    "updateStatus return false, for unSyncable mailbox [%s]", folder);
            return false;
        }
        /// @}

        ControllableActivity activity = (ControllableActivity) mClickListener;
        ActivityController activityController = (ActivityController) activity.getAccountController();
        ConversationListContext listContext = activityController.getCurrentListContext();
        /** M: Add condition to avoid NPE.@{ */
        if (listContext == null) {
            return false;
        }
        /** @} */
        /// M: get the current account.
        Account account = listContext.account;

        // check if this folder allow remote search.
        /// M: show search on server for syncable folder except outbox.
        if (listContext.isLocalSearchExecuted() && folder != null
                && folder.isSyncable() && !folder.isType(UIProvider.FolderType.OUTBOX)) {
            mLoading.setVisibility(View.GONE);
            mLoadMore.setVisibility(View.GONE);
            /// M: pop account do not support remote search. @{
            boolean showRemoteSearch = account != null ? account.supportsCapability(
                    AccountCapabilities.FOLDER_SERVER_SEARCH) : false;
            mRemoteSearch.setVisibility(showRemoteSearch ? View.VISIBLE : View.GONE);
            return showRemoteSearch;
            /// @}
        } else {
            mRemoteSearch.setVisibility(View.GONE);
        }
        return super.updateStatus(cursor, currentFooterShown);
    }
}
