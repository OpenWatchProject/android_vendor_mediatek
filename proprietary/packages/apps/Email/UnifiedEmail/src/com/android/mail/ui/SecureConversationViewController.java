/*
 * Copyright (C) 2013 Google Inc.
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

import android.app.Fragment;
import android.app.FragmentManager;
import android.content.res.Resources;
import android.graphics.Rect;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebSettings;
import android.widget.Toast;


import com.android.emailcommon.utility.TextUtilities;
import com.android.mail.FormattedDateBuilder;
import com.android.mail.R;
import com.android.mail.browse.ConversationMessage;
import com.android.mail.browse.ConversationViewAdapter;
import com.android.mail.browse.ConversationViewAdapter.MessageHeaderItem;
import com.android.mail.browse.ConversationViewHeader;
import com.android.mail.browse.InlineAttachmentViewIntentBuilderCreator;
import com.android.mail.browse.InlineAttachmentViewIntentBuilderCreatorHolder;
import com.android.mail.browse.MessageFooterView;
import com.android.mail.browse.MessageHeaderView;
import com.android.mail.browse.MessageScrollView;
import com.android.mail.browse.MessageWebView;
import com.android.mail.browse.ScrollNotifier.ScrollListener;
import com.android.mail.browse.WebViewContextMenu;
import com.android.mail.print.PrintUtils;
import com.android.mail.providers.Conversation;
import com.android.mail.providers.Message;
import com.android.mail.utils.ConversationViewUtils;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;

/**
 * Controller to do most of the heavy lifting for
 * {@link SecureConversationViewFragment} and
 * {@link com.android.mail.browse.EmlMessageViewFragment}. Currently that work
 * is pretty much the rendering logic.
 */
public class SecureConversationViewController implements
        MessageHeaderView.MessageHeaderViewCallbacks, ScrollListener,
        MessageFooterView.MessageFooterCallbacks {
    private static final String BEGIN_HTML =
            "<body style=\"margin: 0 %spx;\"><div style=\"margin: 16px 0; font-size: 80%%\">";
    private static final String END_HTML = "</div></body>";
    private static final String LOG_TAG = LogTag.getLogTag();

    private final SecureConversationViewControllerCallbacks mCallbacks;

    private MessageWebView mWebView;
    private ConversationViewHeader mConversationHeaderView;
    private MessageHeaderView mMessageHeaderView;
    private MessageHeaderView mSnapHeaderView;
    private MessageFooterView mMessageFooterView;
    private ConversationMessage mMessage;
    private MessageScrollView mScrollView;
    /// M: Indicate the message have large body.
    // Used to control the toast show only once for the message
    private long mTooLargeMessageId = -1;

    private ConversationViewProgressController mProgressController;
    private FormattedDateBuilder mDateBuilder;

    private int mSideMarginInWebPx;

    public SecureConversationViewController(SecureConversationViewControllerCallbacks callbacks) {
        mCallbacks = callbacks;
    }

    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.secure_conversation_view, container, false);
        mScrollView = (MessageScrollView) rootView.findViewById(R.id.scroll_view);
        mConversationHeaderView = (ConversationViewHeader) rootView.findViewById(R.id.conv_header);
        mMessageHeaderView = (MessageHeaderView) rootView.findViewById(R.id.message_header);
        mSnapHeaderView = (MessageHeaderView) rootView.findViewById(R.id.snap_header);
        mMessageFooterView = (MessageFooterView) rootView.findViewById(R.id.message_footer);

        mScrollView.addScrollListener(this);

        // Add color backgrounds to the header and footer.
        // Otherwise the backgrounds are grey. They can't
        // be set in xml because that would add more overdraw
        // in ConversationViewFragment.
        final int color = rootView.getResources().getColor(
                R.color.message_header_background_color);
        mMessageHeaderView.setBackgroundColor(color);
        mSnapHeaderView.setBackgroundColor(color);
        mMessageFooterView.setBackgroundColor(color);

        mProgressController = new ConversationViewProgressController(
                mCallbacks.getFragment(), mCallbacks.getHandler());
        mProgressController.instantiateProgressIndicators(rootView);
        mWebView = (MessageWebView) rootView.findViewById(R.id.webview);
        mWebView.setOverScrollMode(View.OVER_SCROLL_NEVER);
        mWebView.setWebViewClient(mCallbacks.getWebViewClient());
        final InlineAttachmentViewIntentBuilderCreator creator =
                InlineAttachmentViewIntentBuilderCreatorHolder.
                        getInlineAttachmentViewIntentCreator();
        mWebView.setOnCreateContextMenuListener(new WebViewContextMenu(
                mCallbacks.getFragment().getActivity(),
                creator.createInlineAttachmentViewIntentBuilder(null, -1)));
        mWebView.setFocusable(false);
        final WebSettings settings = mWebView.getSettings();

        settings.setJavaScriptEnabled(false);
        settings.setLayoutAlgorithm(WebSettings.LayoutAlgorithm.NORMAL);

        ConversationViewUtils.setTextZoom(mCallbacks.getFragment().getResources(), settings);

        settings.setSupportZoom(true);
        settings.setBuiltInZoomControls(true);
        settings.setDisplayZoomControls(false);

        mScrollView.setInnerScrollableView(mWebView);

        return rootView;
    }

    public void onActivityCreated(Bundle savedInstanceState) {
        mCallbacks.setupConversationHeaderView(mConversationHeaderView);

        final Fragment fragment = mCallbacks.getFragment();

        mDateBuilder = new FormattedDateBuilder(fragment.getActivity());
        mMessageHeaderView.initialize(
                mCallbacks.getConversationAccountController(), mCallbacks.getAddressCache());
        mMessageHeaderView.setContactInfoSource(mCallbacks.getContactInfoSource());
        mMessageHeaderView.setCallbacks(this);
        mMessageHeaderView.setExpandable(false);
        mMessageHeaderView.setViewOnlyMode(mCallbacks.isViewOnlyMode());

        mSnapHeaderView.setSnappy();
        mSnapHeaderView.initialize(
                mCallbacks.getConversationAccountController(), mCallbacks.getAddressCache());
        mSnapHeaderView.setContactInfoSource(mCallbacks.getContactInfoSource());
        mSnapHeaderView.setCallbacks(this);
        mSnapHeaderView.setExpandable(false);
        mSnapHeaderView.setViewOnlyMode(mCallbacks.isViewOnlyMode());

        mCallbacks.setupMessageHeaderVeiledMatcher(mMessageHeaderView);
        mCallbacks.setupMessageHeaderVeiledMatcher(mSnapHeaderView);

        mMessageFooterView.initialize(fragment.getLoaderManager(), fragment.getFragmentManager(),
                mCallbacks.getConversationAccountController(), this);

        mCallbacks.startMessageLoader();

        mProgressController.showLoadingStatus(mCallbacks.isViewVisibleToUser());

        final Resources r = mCallbacks.getFragment().getResources();
        mSideMarginInWebPx = (int) (r.getDimensionPixelOffset(
                R.dimen.conversation_message_content_margin_side) / r.getDisplayMetrics().density);
    }

    /**
     * M: Add for highlight search keywords.
     */
    public void renderMessage(ConversationMessage message) {
        renderMessage(message, null);
    }

    public void onDestroyView() {
        /// M: @{
        mMessageHeaderView.unbind();
        // Release Webview to avoid leak
        if (mWebView != null) {
            mWebView.destroy();
            mWebView = null;
        }
        /// @}
    }

    @Override
    public void onNotifierScroll(final int y) {
        // We need to decide whether or not to display the snap header.
        // Get the location of the moveable message header inside the scroll view.
        Rect rect = new Rect();
        mScrollView.offsetDescendantRectToMyCoords(mMessageHeaderView, rect);

        // If we have scrolled further than the distance from the top of the scrollView to the top
        // of the message header, then the message header is at least partially ofscreen. As soon
        // as the message header goes partially offscreen we need to display the snap header.
        // TODO - re-enable when dogfooders howl
//        if (y > rect.top) {
//            mSnapHeaderView.setVisibility(View.VISIBLE);
//        } else {
            mSnapHeaderView.setVisibility(View.GONE);
//        }
    }

    /**
     * Populate the adapter with overlay views (message headers, super-collapsed
     * blocks, a conversation header), and return an HTML document with spacer
     * divs inserted for all overlays.
     */
    public void renderMessage(ConversationMessage message, String query) {
        mMessage = message;
        ///M: update the subject, in case it is change when back from edit draft
        setSubject(mMessage.subject);

        final boolean alwaysShowImages = mCallbacks.shouldAlwaysShowImages();
        mWebView.getSettings().setBlockNetworkImage(
                !alwaysShowImages && !mMessage.alwaysShowImages);

        // Add formatting to message body
        // At this point, only adds margins.
        StringBuilder dataBuilder = new StringBuilder(
                String.format(BEGIN_HTML, mSideMarginInWebPx));
        dataBuilder.append(mMessage.getBodyAsHtml());
        dataBuilder.append(END_HTML);

        /// M: Highlight the query terms, if we are opening an searched result message.
        String htmlToLoad = null;
        if (query != null) {
            htmlToLoad = TextUtilities.highlightTermsInHtml(dataBuilder.toString(), query);
        } else {
            htmlToLoad = dataBuilder.toString();
        }
        mWebView.loadDataWithBaseURL(mCallbacks.getBaseUri(), htmlToLoad,
                "text/html", "utf-8", null);
        final MessageHeaderItem item = ConversationViewAdapter.newMessageHeaderItem(
                null, mDateBuilder, mMessage, true, mMessage.alwaysShowImages);
        // Clear out the old info from the header before (re)binding
        mMessageHeaderView.unbind();
        mMessageHeaderView.bind(item, false);

        mSnapHeaderView.unbind();
        mSnapHeaderView.bind(item, false);

        /// M: Unify MessageFooterView for UEmail and Email
        mMessageFooterView.bind(item,
                !mMessage.hasAttachments, mMessage.hasAttachments);
//        if (mMessage.hasAttachments) {
//            mMessageFooterView.setVisibility(View.VISIBLE);
//            mMessageFooterView.bind(
//                    item, ConversationViewAdapter.newMessageFooterItem(null, item), !mMessage.hasAttachments, mMessage.hasAttachments);
//        }
        LogUtils.i(LOG_TAG, "_________________ load message finished, will renderMessage " + message);

        /** M: If the message body must be truncated show a toast. @{ */
        if (mCallbacks.isViewVisibleToUser()
                && mTooLargeMessageId != mMessage.id && mMessage.isFlaggedBodyTooLarge()) {
            mTooLargeMessageId = mMessage.id;
            LogUtils.d(LOG_TAG, "messageId: %d is too large and show a toast at Fragment: %s",
                    mMessage.id, mCallbacks.getFragment());
            Toast.makeText(mCallbacks.getFragment().getActivity(), R.string.message_body_too_large,
                    Toast.LENGTH_LONG).show();
        }
        /** @} */
    }

    public ConversationMessage getMessage() {
        return mMessage;
    }

    public ConversationViewHeader getConversationHeaderView() {
        return mConversationHeaderView;
    }

    public void dismissLoadingStatus() {
        mProgressController.dismissLoadingStatus();
    }

    public void setSubject(String subject) {
        mConversationHeaderView.setSubject(subject);
    }

    public void printMessage() {
        /**
         * M: Adjust null for the case that the eml message is in loading, then
         * print the message @{
         */
        if (mMessage != null) {
            final Conversation conversation = mMessage.getConversation();
            PrintUtils.printMessage(mCallbacks.getFragment().getActivity(), mMessage,
                    conversation != null ? conversation.subject : mMessage.subject,
                    mCallbacks.getAddressCache(), mCallbacks.getBaseUri(), false /* useJavascript */);
        } else {
            LogUtils.v(LogTag.getLogTag(),
                    "SecureConvViewController mMessage is null and is loading.");
        }
        /** @} */
    }

    // Start MessageHeaderViewCallbacks implementations

    @Override
    public void setMessageSpacerHeight(MessageHeaderItem item, int newSpacerHeight) {
        // Do nothing.
    }

    @Override
    public void setMessageExpanded(MessageHeaderItem item, int newSpacerHeight) {
        // Do nothing.
    }

    @Override
    public void setMessageDetailsExpanded(MessageHeaderItem i, boolean expanded, int heightBefore) {
        // Do nothing.
    }

    @Override
    public void showExternalResources(final Message msg) {
        mWebView.getSettings().setBlockNetworkImage(false);
    }

    @Override
    public void showExternalResources(final String rawSenderAddress) {
        mWebView.getSettings().setBlockNetworkImage(false);
    }

    @Override
    public boolean supportsMessageTransforms() {
        return false;
    }

    @Override
    public String getMessageTransforms(final Message msg) {
        return null;
    }

    @Override
    public boolean isSecure() {
        return true;
    }

    /** M:
     * @see com.android.mail.browse.MessageFooterView.MessageFooterCallbacks#onAttachmentsChanged()
     */
    @Override
    public void onAttachmentsLayoutChanged() {
        // nothing to do
    }

    @Override
    public FragmentManager getFragmentManager() {
        return mCallbacks.getFragment().getFragmentManager();
    }

    // End MessageHeaderViewCallbacks implementations
    /**
     * M: Dispatch keyevent to webView for Zoom in/out on SmartBook
     */
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (null != mWebView) {
            LogUtils.v(LogTag.getLogTag(), "SecureConvViewController dispatchKeyEvent event: %s", event);
            return mWebView.dispatchKeyEvent(event);
        }
        return false;
    }
}
