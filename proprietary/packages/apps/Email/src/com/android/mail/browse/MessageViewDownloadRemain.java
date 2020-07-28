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

import android.app.FragmentManager;
import android.content.AsyncQueryHandler;
import android.content.ContentValues;
import android.content.Context;
import android.net.Uri;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.android.email.R;
import com.android.mail.providers.Message;
import com.android.mail.providers.UIProvider;
import com.android.mail.providers.UIProvider.MessageColumns;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.StorageLowState;
import com.mediatek.mail.ui.utils.UiUtilities;
import com.mediatek.mail.utils.Utility;

/**
 * M: View for a partial downloaded conversation view. Shows download status and allows launching
 * intents to act on a completely fetching.
 *
 */
public class MessageViewDownloadRemain extends RelativeLayout implements OnClickListener {

    private TextView mRemainText;
    private TextView mLoadingText;
    private ProgressBar mProgress;
    /// M: Fragment Manager @{
    private FragmentManager mFragmentManager;
    /// @}

    private int mUiState = UIProvider.MessageState.NONE;
    private FetchCommandHandler mCommandHandler = null;
    private Message mConvMessage;
    private boolean mStarted = false;

    private static final String LOG_TAG = LogTag.getLogTag();


    public MessageViewDownloadRemain(Context context) {
        this(context, null);
    }

    public MessageViewDownloadRemain(Context context, AttributeSet attrs) {
        super(context, attrs);
        mCommandHandler = new FetchCommandHandler(context);
    }

    public static MessageViewDownloadRemain inflate(LayoutInflater inflater, ViewGroup parent) {
        MessageViewDownloadRemain view = (MessageViewDownloadRemain) inflater.inflate(
                R.layout.mtk_conversation_message_footer_ramin_btn, parent, false);
        return view;
    }

    public void render(Message message, boolean show) {
        mConvMessage = message;
        mUiState = message.state;
        if (show) {
            LogUtils.d(LOG_TAG, "Show download remain button");
            updateButtonAsState(mUiState);
        } else {
            LogUtils.d(LOG_TAG, "Gone download remain button");
            updateViewState(mRemainText, false);
            updateViewState(mProgress, false);
            updateViewState(mLoadingText, false);
        }
        updateViewState(this, show);
    }

    private void updateViewState(View view, boolean show) {
        view.setVisibility(show ? View.VISIBLE : View.GONE);
    }

    private void notifyLoadResult(int uiState) {
        if (uiState == UIProvider.MessageState.FAILED) {
            // TODO: Shall we give some notify to user about fetching failed
            // Toast or something ?
            resetFetchMessage(mConvMessage);
        } else if (uiState == UIProvider.MessageState.LOADING) {
            if (!mStarted) {
                // Re-try fetch when state loading
                startFetchMessage(mConvMessage);
            }
        }
        // Do nothing for other status
    }

    private void updateButtonAsState(int uiState) {
        switch (uiState) {
        case UIProvider.MessageState.LOADING:
            updateViewState(mRemainText, false);
            updateViewState(mProgress, true);
            updateViewState(mLoadingText, true);
            break;
        case UIProvider.MessageState.SUCCESS:
            updateViewState(mRemainText, false);
            updateViewState(mProgress, false);
            updateViewState(mLoadingText, false);
            break;
        case UIProvider.MessageState.NONE:
        case UIProvider.MessageState.FAILED:
            updateViewState(mRemainText, true);
            updateViewState(mProgress, false);
            updateViewState(mLoadingText, false);
            break;
        default:
            break;
        }
        notifyLoadResult(uiState);
        LogUtils.d(LOG_TAG, "Show download remain button as state: " + uiState);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mRemainText = (TextView) findViewById(R.id.msg_remain_btn);
        mProgress = (ProgressBar) findViewById(R.id.msg_remain_loading_progress);
        mLoadingText = (TextView) findViewById(R.id.msg_remain_loading_text);
        setOnClickListener(this);
        mRemainText.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        LogUtils.d(LOG_TAG, "Click button as state: " + mUiState);
        if (v.getId() == R.id.msg_remain_btn) {
            /// M: We Can't download remain in low storage state @{
            if (StorageLowState.checkIfStorageLow(mContext)) {
                LogUtils.e(LOG_TAG, "Can't download remain due to low storage");
                return;
            }
            /// @}
            /** M: We can not refresh in network disconnection state.@{ */
            if (!Utility.hasConnectivity(getContext())) {
                UiUtilities.showConnectionAlertDialog(mFragmentManager);
                return;
            }
            /** @} */
            // Just doing fetch once
            if (mUiState != UIProvider.MessageState.LOADING) {
                startFetchMessage(mConvMessage);
                mConvMessage.state = UIProvider.MessageState.LOADING;
                updateButtonAsState(mConvMessage.state);
            }
        }
    }

    private void startFetchMessage(Message message) {
        final ContentValues params = new ContentValues(1);
        params.put(MessageColumns.STATE, UIProvider.MessageState.LOADING);

        mCommandHandler.sendCommand(message.uri, params);
        mStarted = true;
        LogUtils.d(LOG_TAG, "Start to trigger fetching message: %s: ", message.uri);
    }

    private void resetFetchMessage(Message message) {
        final ContentValues params = new ContentValues(1);
        params.put(MessageColumns.STATE, UIProvider.MessageState.NONE);

        mCommandHandler.sendCommand(message.uri, params);
        LogUtils.d(LOG_TAG, "Reset state of message: %s", message.uri);
    }

    class FetchCommandHandler extends AsyncQueryHandler {
        public FetchCommandHandler(Context context) {
            super(context.getContentResolver());
        }

        /**
         * Asynchronously begin an update() on a ContentProvider.
         *
         */
        public void sendCommand(Uri uri, ContentValues params) {
            /** M: Check is the uri null. @{ */
            if (uri == null) {
                LogUtils.w(LOG_TAG, "FetchCommandHandler sendCommand but uri is null !!!");
                return;
            }
            /** @} */
            startUpdate(0, null, uri, params, null, null);
        }
    }

    /**
     * M: Initialize the fragment manager
     * @param fragMagr
     */
    public void initialize(FragmentManager fragMagr) {
        mFragmentManager = fragMagr;
    }
}
