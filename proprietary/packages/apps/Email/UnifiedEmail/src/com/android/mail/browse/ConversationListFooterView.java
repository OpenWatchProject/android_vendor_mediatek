/*
 * Copyright (C) 2012 Google Inc.
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

package com.android.mail.browse;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.util.AttributeSet;
import android.view.View;
import android.widget.LinearLayout;

import com.android.mail.R;
import com.android.mail.providers.Folder;
import com.android.mail.providers.UIProvider;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;

/**
 * M:
 *
 */
public /*final*/ class ConversationListFooterView extends LinearLayout implements View.OnClickListener {

    public interface FooterViewClickListener {
        void onFooterViewLoadMoreClick(Folder folder);
        /// M: add for local search(start remote search in footer view not by IME key).
        void onFooterViewRemoteSearchClick(Folder folder);
    }

    /// M: status for footer view
    private static final int STATUS_NONE     = 0;
    private static final int STATUS_LOADING  = 1;
    private static final int STATUS_LOADMORE = 2;
    private static final int STATUS_NETWORK  = 3;

    /// M: Make inheritable
    protected View mLoading;
    protected View mLoadMore;
    protected Uri mLoadMoreUri;
    protected FooterViewClickListener mClickListener;
    /// M:To recorder the last cursor state
    private int mCursorState;

    public ConversationListFooterView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mLoading = findViewById(R.id.loading);
        mLoadMore = findViewById(R.id.load_more);
        mLoadMore.setOnClickListener(this);
    }

    public void setClickListener(FooterViewClickListener listener) {
        mClickListener = listener;
    }

    @Override
    public void onClick(View v) {
        final int id = v.getId();
        final Folder f = (Folder) v.getTag();
        if (id == R.id.load_more) {
            LogUtils.d(LogTag.getLogTag(), "LoadMore triggered folder [%s]",
                    f != null ? f.loadMoreUri : "null");
            mClickListener.onFooterViewLoadMoreClick(f);
        }
    }

    /// M: update UI status, for Loading/LoadMore/NetworkError.
    public void updateLoadingStatus(boolean start) {
        LogUtils.d(LogTag.getLogTag(), "updateLoadingStatus show loading progress dialog ? [%s]", start);
        if (start) {
            mLoading.setVisibility(View.VISIBLE);
            mLoadMore.setVisibility(View.GONE);
        } else {
            mLoading.setVisibility(View.GONE);
            mLoadMore.setVisibility(View.VISIBLE);
        }
    }

    /**
     * M: update footer view status with loading, loadmore, network and none
     * @param status status to set
     */
    public void updateFooterStatus(final int status) {
        switch (status) {
            case STATUS_LOADING:
                mLoading.setVisibility(View.VISIBLE);
                mLoadMore.setVisibility(View.GONE);
                break;
            case STATUS_LOADMORE:
                mLoading.setVisibility(View.GONE);
                mLoadMore.setVisibility(View.VISIBLE);
                break;
            case STATUS_NETWORK:
                mLoading.setVisibility(View.GONE);
                mLoadMore.setVisibility(View.GONE);
                break;
            case STATUS_NONE:
            default:
                LogUtils.d(LogTag.getLogTag(), "updateFooterStatus with unknown status: %d", status);
                mLoading.setVisibility(View.GONE);
                mLoadMore.setVisibility(View.GONE);
                break;
        }
    }

    public void setFolder(Folder folder) {
        mLoadMore.setTag(folder);
        mLoadMoreUri = folder.loadMoreUri;
    }

    /**
     * M: Update the view to reflect the new folder status.
     * @param ConversationCursor the cursor
     * @param currentFooterShown the current footer view shown state
     * @return boolean the new footer view shown state
     */
    public boolean updateStatus(final ConversationCursor cursor, final boolean currentFooterShown) {
        if (cursor == null) {
            // reset status of footer view
            updateFooterStatus(STATUS_NONE);
            return false;
        }
        boolean showFooter = true;
        final Bundle extras = cursor.getExtras();
        final int cursorStatus = extras.getInt(UIProvider.CursorExtraKeys.EXTRA_STATUS);
        final int totalCount = extras.getInt(UIProvider.CursorExtraKeys.EXTRA_TOTAL_COUNT);
        /// M: Get the value from extra
        final boolean allMessagesLoadFinish = extras.getBoolean(
                UIProvider.CursorExtraKeys.EXTRA_MESSAGES_LOAD_FINISH, false);

        if (UIProvider.CursorStatus.isWaitingForResults(cursorStatus)) {
            if (cursor.getCount() != 0) {
                // When loading more, show the spinner in the footer.
                updateFooterStatus(STATUS_LOADING);
            } else {
                // We're currently loading, but we have no messages at all. We don't need to show
                // the footer, because we should be displaying the loading state on the
                // conversation list itself.
                showFooter = false;
            }

        } else if (mLoadMoreUri != null && cursor.getCount() < totalCount
                && !allMessagesLoadFinish) {
            // We know that there are more messages on the server than we have locally, so we
            // need to show the footer with the "load more" button.
            updateFooterStatus(STATUS_LOADMORE);
        } else {
            // TODO: what happens to here ?
            // Maybe showFooter as what it was is better.
            showFooter = false;
            updateFooterStatus(STATUS_NONE);
            LogUtils.d(LogTag.getLogTag(), "Enter the case and footer view's shown state was : %s",
                    currentFooterShown);
        }
        return showFooter;
    }
}
