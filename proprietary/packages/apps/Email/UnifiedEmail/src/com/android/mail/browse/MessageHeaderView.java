/**
 * Copyright (c) 2011, Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mail.browse;

import android.annotation.SuppressLint;
import android.app.FragmentManager;
import android.content.AsyncQueryHandler;
import android.content.Context;
import android.content.res.Resources;
import android.database.DataSetObserver;
import android.graphics.Bitmap;
import android.os.Handler;
import androidx.core.text.BidiFormatter;
import android.text.Html;
import android.text.Spannable;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.method.LinkMovementMethod;
import android.text.style.DynamicDrawableSpan;
import android.text.style.ImageSpan;
import android.text.style.URLSpan;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnDismissListener;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.QuickContactBadge;
import android.widget.TextView;
import android.widget.Toast;

import com.android.emailcommon.mail.Address;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.android.mail.ContactInfo;
import com.android.mail.ContactInfoSource;
import com.android.mail.R;
import com.android.mail.analytics.Analytics;
import com.android.mail.browse.ConversationViewAdapter.MessageHeaderItem;
import com.android.mail.compose.ComposeActivity;
import com.android.mail.perf.Timer;
import com.android.mail.photomanager.LetterTileProvider;
import com.android.mail.print.PrintUtils;
import com.android.mail.providers.Account;
import com.android.mail.providers.Conversation;
import com.android.mail.providers.Message;
import com.android.mail.providers.Settings;
import com.android.mail.providers.UIProvider;
import com.android.mail.text.EmailAddressSpan;
import com.android.mail.ui.AbstractConversationViewFragment;
import com.android.mail.ui.ImageCanvas;
import com.android.mail.utils.BitmapUtil;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.StyleUtils;
import com.android.mail.utils.Throttle;
import com.android.mail.utils.Utils;
import com.android.mail.utils.VeiledAddressMatcher;
import com.google.common.annotations.VisibleForTesting;
import com.mediatek.mail.utils.ThrottleContentObserver;
import com.mediatek.mail.vip.VipMember;
import com.mediatek.mail.vip.VipMemberCache;

import java.io.IOException;
import java.io.StringReader;
import java.util.Map;

public class MessageHeaderView extends SnapHeader implements OnClickListener,
        OnMenuItemClickListener, ConversationContainer.DetachListener, /*M:*/ OnDismissListener {

    /**
     * Cap very long recipient lists during summary construction for efficiency.
     */
    private static final int SUMMARY_MAX_RECIPIENTS = 50;

    private static final int MAX_SNIPPET_LENGTH = 100;

    private static final int SHOW_IMAGE_PROMPT_ONCE = 1;
    private static final int SHOW_IMAGE_PROMPT_ALWAYS = 2;

    private static final String HEADER_RENDER_TAG = "message header render";
    private static final String LAYOUT_TAG = "message header layout";
    private static final String MEASURE_TAG = "message header measure";

    private static final String LOG_TAG = LogTag.getLogTag();

    // This is a debug only feature
    public static final boolean ENABLE_REPORT_RENDERING_PROBLEM = false;

    private MessageHeaderViewCallbacks mCallbacks;

    private View mBorderView;
    private ViewGroup mUpperHeaderView;
    private View mTitleContainer;
    private View mSnapHeaderBottomBorder;
    private TextView mSenderNameView;
    private TextView mRecipientSummary;
    private TextView mDateView;
    private View mHideDetailsView;
    private TextView mSnippetView;
    private MessageHeaderContactBadge mPhotoView;
    private ViewGroup mExtraContentView;
    private ViewGroup mExpandedDetailsView;
    private SpamWarningView mSpamWarningView;
    private TextView mImagePromptView;
    private MessageInviteView mInviteView;
    private View mForwardButton;
    private View mOverflowButton;
    private View mDraftIcon;
    private View mEditDraftButton;
    private TextView mUpperDateView;
    private View mReplyButton;
    private View mReplyAllButton;
    private View mAttachmentIcon;
    private final EmailCopyContextMenu mEmailCopyMenu;
    boolean mIsFromVIP = false;
    public static final String LIST_DELIMITER_EMAIL = "\1";

    // temporary fields to reference raw data between initial render and details
    // expansion
    private String[] mFrom;
    private String[] mTo;
    private String[] mCc;
    private String[] mBcc;
    private String[] mReplyTo;

    private boolean mIsDraft = false;

    private int mSendingState;

    private String mSnippet;

    private Address mSender;

    private ContactInfoSource mContactInfoSource;

    private boolean mPreMeasuring;

    private ConversationAccountController mAccountController;

    private Map<String, Address> mAddressCache;

    private boolean mShowImagePrompt;

    private PopupMenu mPopup;

    private MessageHeaderItem mMessageHeaderItem;
    private ConversationMessage mMessage;

    private boolean mRecipientSummaryValid;
    private boolean mExpandedDetailsValid;

    private final LayoutInflater mInflater;

    private AsyncQueryHandler mQueryHandler;

    private boolean mObservingContactInfo;

    /**
     * What I call myself? "me" in English, and internationalized correctly.
     */
    private final String mMyName;

    private final DataSetObserver mContactInfoObserver = new DataSetObserver() {
        @Override
        public void onChanged() {
            updateContactInfo();
        }
    };

    private boolean mExpandable = true;

    private VeiledAddressMatcher mVeiledMatcher;

    private boolean mIsViewOnlyMode = false;
    /// M: Using this flag to let the external decide if the header should be view only.
    private boolean mIsViewOnlyModeFromOutside = false;

    private LetterTileProvider mLetterTileProvider;
    private final int mContactPhotoWidth;
    private final int mContactPhotoHeight;
    private final int mTitleContainerMarginEnd;

    /**
     * The snappy header has special visibility rules (i.e. no details header,
     * even though it has an expanded appearance)
     */
    private boolean mIsSnappy;

    private BidiFormatter mBidiFormatter;


    public interface MessageHeaderViewCallbacks {
        void setMessageSpacerHeight(MessageHeaderItem item, int newSpacerHeight);

        void setMessageExpanded(MessageHeaderItem item, int newSpacerHeight);

        void setMessageDetailsExpanded(MessageHeaderItem messageHeaderItem, boolean expanded,
                int previousMessageHeaderItemHeight);

        void showExternalResources(Message msg);

        void showExternalResources(String senderRawAddress);

        boolean supportsMessageTransforms();

        String getMessageTransforms(Message msg);

        FragmentManager getFragmentManager();

        /**
         * @return <tt>true</tt> if this header is contained within a SecureConversationViewFragment
         * and cannot assume the content is <strong>not</strong> malicious
         */
        boolean isSecure();
    }

    public MessageHeaderView(Context context) {
        this(context, null);
    }

    public MessageHeaderView(Context context, AttributeSet attrs) {
        this(context, attrs, -1);
    }

    public MessageHeaderView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);

        mIsSnappy = false;
        mEmailCopyMenu = new EmailCopyContextMenu(getContext());
        mInflater = LayoutInflater.from(context);
        mMyName = context.getString(R.string.me_object_pronoun);

        final Resources res = getResources();
        mContactPhotoWidth = res.getDimensionPixelSize(R.dimen.contact_image_width);
        mContactPhotoHeight = res.getDimensionPixelSize(R.dimen.contact_image_height);
        mTitleContainerMarginEnd = res.getDimensionPixelSize(R.dimen.conversation_view_margin_side);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();
        mBorderView = findViewById(R.id.message_header_border);
        mUpperHeaderView = (ViewGroup) findViewById(R.id.upper_header);
        mTitleContainer = findViewById(R.id.title_container);
        mSnapHeaderBottomBorder = findViewById(R.id.snap_header_bottom_border);
        mSenderNameView = (TextView) findViewById(R.id.sender_name);
        mRecipientSummary = (TextView) findViewById(R.id.recipient_summary);
        mDateView = (TextView) findViewById(R.id.send_date);
        mHideDetailsView = findViewById(R.id.hide_details);
        mSnippetView = (TextView) findViewById(R.id.email_snippet);
        mPhotoView = (MessageHeaderContactBadge) findViewById(R.id.photo);
        mPhotoView.setQuickContactBadge(
                (QuickContactBadge) findViewById(R.id.invisible_quick_contact));
        mReplyButton = findViewById(R.id.reply);
        mReplyAllButton = findViewById(R.id.reply_all);
        mForwardButton = findViewById(R.id.forward);
        mOverflowButton = findViewById(R.id.overflow);
        mDraftIcon = findViewById(R.id.draft);
        mEditDraftButton = findViewById(R.id.edit_draft);
        mUpperDateView = (TextView) findViewById(R.id.upper_date);
        mAttachmentIcon = findViewById(R.id.attachment);
        mExtraContentView = (ViewGroup) findViewById(R.id.header_extra_content);

        setExpanded(true);

        registerMessageClickTargets(mReplyButton, mReplyAllButton, mForwardButton,
                mEditDraftButton, mOverflowButton, mUpperHeaderView, mDateView, mHideDetailsView);

        mUpperHeaderView.setOnCreateContextMenuListener(mEmailCopyMenu);
    }

    private void registerMessageClickTargets(View... views) {
        for (View v : views) {
            if (v != null) {
                v.setOnClickListener(this);
            }
        }
    }

    @Override
    public void initialize(ConversationAccountController accountController,
            Map<String, Address> addressCache, MessageHeaderViewCallbacks callbacks,
            ContactInfoSource contactInfoSource, VeiledAddressMatcher veiledAddressMatcher) {
        initialize(accountController, addressCache);
        setCallbacks(callbacks);
        setContactInfoSource(contactInfoSource);
        setVeiledMatcher(veiledAddressMatcher);
    }

    /**
     * Associate the header with a contact info source for later contact
     * presence/photo lookup.
     */
    public void setContactInfoSource(ContactInfoSource contactInfoSource) {
        mContactInfoSource = contactInfoSource;
    }

    public void setCallbacks(MessageHeaderViewCallbacks callbacks) {
        mCallbacks = callbacks;
    }

    public void setVeiledMatcher(VeiledAddressMatcher matcher) {
        mVeiledMatcher = matcher;
    }

    public boolean isExpanded() {
        // (let's just arbitrarily say that unbound views are expanded by default)
        return mMessageHeaderItem == null || mMessageHeaderItem.isExpanded();
    }

    @Override
    public void onDetachedFromParent() {
        unbind();
    }

    /**
     * Headers that are unbound will not match any rendered header (matches()
     * will return false). Unbinding is not guaranteed to *hide* the view's old
     * data, though. To re-bind this header to message data, call render() or
     * renderUpperHeaderFrom().
     */
    @Override
    public void unbind() {
        mMessageHeaderItem = null;
        mMessage = null;

        if (mObservingContactInfo) {
            mContactInfoSource.unregisterObserver(mContactInfoObserver);
            mObservingContactInfo = false;
        }
        /// M: Add for VIP to unregister VIP observer.
        unregisterVipObserver();
        /// M: Pop up menu to be dismissed when entry/exit multiwindow
        if(mPopup != null)
            mPopup.dismiss();
    }

    public void initialize(ConversationAccountController accountController,
            Map<String, Address> addressCache) {
        mAccountController = accountController;
        mAddressCache = addressCache;
    }

    private Account getAccount() {
        return mAccountController != null ? mAccountController.getAccount() : null;
    }

    public void bind(MessageHeaderItem headerItem, boolean measureOnly) {
        if (mMessageHeaderItem != null && mMessageHeaderItem == headerItem) {
            return;
        }

        mMessageHeaderItem = headerItem;
        render(measureOnly);
        ///M: Add for VIP to register VIP observer.
        registerVipObserver();
    }

    /**
     * Rebinds the view to its data. This will only update the view
     * if the {@link MessageHeaderItem} sent as a parameter is the
     * same as the view's current {@link MessageHeaderItem} and the
     * view's expanded state differs from the item's expanded state.
     */
    public void rebind(MessageHeaderItem headerItem) {
        if (mMessageHeaderItem == null || mMessageHeaderItem != headerItem ||
                isActivated() == isExpanded()) {
            return;
        }

        render(false /* measureOnly */);
    }

    @Override
    public void refresh() {
        render(false);
    }

    private BidiFormatter getBidiFormatter() {
        if (mBidiFormatter == null) {
            final ConversationViewAdapter adapter = mMessageHeaderItem != null
                    ? mMessageHeaderItem.getAdapter() : null;
            if (adapter == null) {
                mBidiFormatter = BidiFormatter.getInstance();
            } else {
                mBidiFormatter = adapter.getBidiFormatter();
            }
        }
        return mBidiFormatter;
    }

    private void render(boolean measureOnly) {
        if (mMessageHeaderItem == null) {
            return;
        }

        Timer t = new Timer();
        t.start(HEADER_RENDER_TAG);

        mRecipientSummaryValid = false;
        mExpandedDetailsValid = false;

        mMessage = mMessageHeaderItem.getMessage();

        final Account account = getAccount();
        final boolean alwaysShowImagesForAccount = (account != null) &&
                (account.settings.showImages == Settings.ShowImages.ALWAYS);

        final boolean alwaysShowImagesForMessage = mMessage.shouldShowImagePrompt();

        if (!alwaysShowImagesForMessage) {
            // we don't need the "Show picture" prompt if the user allows images for this message
            mShowImagePrompt = false;
        } else if (mCallbacks.isSecure()) {
            // in a secure view we always display the "Show picture" prompt
            mShowImagePrompt = true;
        } else {
            // otherwise honor the account setting for automatically showing pictures
            mShowImagePrompt = !alwaysShowImagesForAccount;
        }
        LogUtils.d(LOG_TAG, "MessageHeaderView render %s", mMessageHeaderItem.isExpanded());
        setExpanded(mMessageHeaderItem.isExpanded());

        mFrom = mMessage.getFromAddresses();
        mTo = mMessage.getToAddresses();
        mCc = mMessage.getCcAddresses();
        mBcc = mMessage.getBccAddresses();
        mReplyTo = mMessage.getReplyToAddresses();

        /**
         * Turns draft mode on or off. Draft mode hides message operations other
         * than "edit", hides contact photo, hides presence, and changes the
         * sender name to "Draft".
         */
        mIsDraft = mMessage.draftType != UIProvider.DraftType.NOT_A_DRAFT;
        mSendingState = mMessage.sendingState;
        /**
         * M: Update view only mode for header view.For two case:
         * 1. View eml attachment. set by {@link EmlMessageViewFragment.isViewOnlyMode}
         * 2. View OutBox mail. set by mMessage.isSending.
         * set ViewOnlyMode true, if meet any case.
         * @{ */
        mIsViewOnlyMode = mIsViewOnlyModeFromOutside ||
                (mSendingState == UIProvider.ConversationSendingState.SENDING);
        /**@}*/

        // If this was a sent message AND:
        // 1. the account has a custom from, the cursor will populate the
        // selected custom from as the fromAddress when a message is sent but
        // not yet synced.
        // 2. the account has no custom froms, fromAddress will be empty, and we
        // can safely fall back and show the account name as sender since it's
        // the only possible fromAddress.
        /** M:  3. Before set sender to account name, we could check whether
         * Reply-To is available, set sender to Reply-To if possible could make
         * Sender more sense @{ */
        String from = mMessage.getFrom();
        if (TextUtils.isEmpty(from)) {
            if (mReplyTo != null && mReplyTo.length > 0) {
                from = mReplyTo[0];
            } else {
                /**
                 * M: It shouldn't use the account display name to instead of
                 * the mail sender, if the from in message is empty, otherwise,
                 * it would import a time issue, sometimes the sender name
                 * shows, and sometime the sender name showed nothing.
                 */
//                from = getAccount() != null ? getAccount().getDisplayName() : "";
                LogUtils.d(LOG_TAG, "There is no from field, account is %s",
                        getAccount());
            }
        }
        mSender = getAddress(from);
        /** M: @} */

        updateChildVisibility();

        final String snippet;
        if (mIsDraft || mSendingState != UIProvider.ConversationSendingState.OTHER) {
            snippet = makeSnippet(mMessage.snippet);
        } else {
            snippet = mMessage.snippet;
        }
        mSnippet = snippet == null ? null : getBidiFormatter().unicodeWrap(snippet);

        mSenderNameView.setText(getHeaderTitle());
        setRecipientSummary();
        setDateText();
        mSnippetView.setText(mSnippet);
        setAddressOnContextMenu();

        if (mUpperDateView != null) {
            mUpperDateView.setText(mMessageHeaderItem.getTimestampShort());
        }

        /** M: Calculate VIP sender @{ */
        updateSenderVipIcon(!mIsDraft && mSendingState
                != UIProvider.ConversationSendingState.SENDING && mSender != null
                && VipMemberCache.isVIP(mSender.getAddress()));
        /** @} */

        if (measureOnly) {
            // avoid leaving any state around that would interfere with future regular bind() calls
            unbind();
        } else {
            updateContactInfo();
            if (!mObservingContactInfo) {
                mContactInfoSource.registerObserver(mContactInfoObserver);
                mObservingContactInfo = true;
            }
        }

        t.pause(HEADER_RENDER_TAG);
    }

    /**
     * Update context menu's address field for when the user long presses
     * on the message header and attempts to copy/send email.
     */
    private void setAddressOnContextMenu() {
        if (mSender != null) {
            mEmailCopyMenu.setAddress(mSender.getAddress());
        }
    }

    @Override
    public boolean isBoundTo(ConversationOverlayItem item) {
        return item == mMessageHeaderItem;
    }

    public Address getAddress(String emailStr) {
        return Utils.getAddress(mAddressCache, emailStr);
    }

    private void updateSpacerHeight() {
        final int h = measureHeight();

        mMessageHeaderItem.setHeight(h);
        if (mCallbacks != null) {
            mCallbacks.setMessageSpacerHeight(mMessageHeaderItem, h);
        }
    }

    private int measureHeight() {
        ViewGroup parent = (ViewGroup) getParent();
        /// M: When we measure this header view, sometimes it isn't attached to any parent view,
        // ie. when we do pre-measure here to get its height to set message's spacer height. @{
        if (parent == null) {
            parent = mMessageHeaderItem.getParentView();
        }
        /// @}
        if (parent == null) {
            LogUtils.e(LOG_TAG, new Error(), "Unable to measure height of detached header");
            return getHeight();
        }
        mPreMeasuring = true;
        final int h = Utils.measureViewHeight(this, parent);
        mPreMeasuring = false;
        return h;
    }

    private CharSequence getHeaderTitle() {
        CharSequence title;
        switch (mSendingState) {
            case UIProvider.ConversationSendingState.QUEUED:
            case UIProvider.ConversationSendingState.SENDING:
            case UIProvider.ConversationSendingState.RETRYING:
                title = getResources().getString(R.string.sending);
                break;
            case UIProvider.ConversationSendingState.SEND_ERROR:
                title = getResources().getString(R.string.message_failed);
                break;
            default:
                if (mIsDraft) {
                    title = SendersView.getSingularDraftString(getContext());
                } else {
                    title = getBidiFormatter().unicodeWrap(
                            getSenderName(mSender));
                }
        }

        return title;
    }

    private void setRecipientSummary() {
        if (!mRecipientSummaryValid) {
            if (mMessageHeaderItem.recipientSummaryText == null) {
                final Account account = getAccount();
                final String meEmailAddress = (account != null) ? account.getEmailAddress() : "";
                mMessageHeaderItem.recipientSummaryText = getRecipientSummaryText(getContext(),
                        meEmailAddress, mMyName, mTo, mCc, mBcc, mAddressCache, mVeiledMatcher,
                        getBidiFormatter());
            }
            mRecipientSummary.setText(mMessageHeaderItem.recipientSummaryText);
            mRecipientSummaryValid = true;
        }
    }

    private void setDateText() {
        if (mIsSnappy) {
            mDateView.setText(mMessageHeaderItem.getTimestampLong());
            mDateView.setOnClickListener(null);
        } else {
            mDateView.setMovementMethod(LinkMovementMethod.getInstance());
            mDateView.setText(Html.fromHtml(getResources().getString(
                    R.string.date_and_view_details, mMessageHeaderItem.getTimestampLong())));
            StyleUtils.stripUnderlinesAndUrl(mDateView);
        }
    }

    /**
     * Return the name, if known, or just the address.
     */
    private static String getSenderName(Address sender) {
        if (sender == null) {
            return "";
        }
        final String displayName = sender.getPersonal();
        return TextUtils.isEmpty(displayName) ? sender.getAddress() : displayName;
    }

    private static void setChildVisibility(int visibility, View... children) {
        for (View v : children) {
            if (v != null) {
                v.setVisibility(visibility);
            }
        }
    }

    private void setExpanded(final boolean expanded) {
        // use View's 'activated' flag to store expanded state
        // child view state lists can use this to toggle drawables
        setActivated(expanded);
        if (mMessageHeaderItem != null) {
            mMessageHeaderItem.setExpanded(expanded);
        }
    }

    /**
     * Update the visibility of the many child views based on expanded/collapsed
     * and draft/normal state.
     */
    private void updateChildVisibility() {
        // Too bad this can't be done with an XML state list...

        LogUtils.d(LOG_TAG, "MessageHeaderView updateChildVisisbility ViewOnly:%s, Expanded:%s",
                mIsViewOnlyMode, isExpanded());
        if (mIsViewOnlyMode) {
            /** M: In view only mode
             * Above all: hide all the view which can make the user to modify the information.
             * When meeting the above conditions, then Showing the appropriate views
             * in current layout.
             *  1. Layout is collapsed: Just to show mSnippetView, mUpperDateView.
             *  2. Layout is expanded: Just show the message information will be fine.
             *  @{ */
            setChildVisibility(GONE, mSnapHeaderBottomBorder);
            setChildVisibility(GONE, mReplyButton, mReplyAllButton, mForwardButton,
                    mOverflowButton, mEditDraftButton);

            if (mIsDraft) {
                setChildVisibility(VISIBLE, mDraftIcon);
                setChildVisibility(GONE, mPhotoView);
            } else {
                setChildVisibility(GONE, mDraftIcon);
                setChildVisibility(VISIBLE, mPhotoView);
            }

            if (isExpanded()) {
                setMessageDetailsVisibility(VISIBLE);
                setChildVisibility(VISIBLE, mRecipientSummary);
                setChildVisibility(GONE, mAttachmentIcon, mUpperDateView, mSnippetView);

                setChildMarginEnd(mTitleContainer, 0);
            } else {
                setMessageDetailsVisibility(GONE);
                setChildVisibility(GONE, mRecipientSummary, mDateView, mHideDetailsView);
                setChildVisibility(VISIBLE, mSnippetView, mUpperDateView);
                setChildVisibility(mMessage.hasAttachments ? VISIBLE : GONE, mAttachmentIcon);

                setChildMarginEnd(mTitleContainer, mTitleContainerMarginEnd);
            }
            /** @} */
        } else if (isExpanded()) {
            int normalVis;
            int draftVis;

            final boolean isSnappy = isSnappy();
            setMessageDetailsVisibility((isSnappy) ? GONE : VISIBLE);
            setChildVisibility(isSnappy ? VISIBLE : GONE, mSnapHeaderBottomBorder);

            if (mIsDraft) {
                normalVis = GONE;
                draftVis = VISIBLE;
            } else {
                normalVis = VISIBLE;
                draftVis = GONE;
            }

            setReplyOrReplyAllVisible();
            /// M: VIP show as normal case
            setChildVisibility(normalVis, mPhotoView, mForwardButton, mOverflowButton);
            setChildVisibility(draftVis, mDraftIcon, mEditDraftButton);
            setChildVisibility(VISIBLE, mRecipientSummary);
            setChildVisibility(GONE, mAttachmentIcon, mUpperDateView, mSnippetView);

            setChildMarginEnd(mTitleContainer, 0);
        } else {
            setMessageDetailsVisibility(GONE);
            setChildVisibility(GONE, mSnapHeaderBottomBorder);
            setChildVisibility(VISIBLE, mSnippetView, mUpperDateView);

            setChildVisibility(GONE, mEditDraftButton, mReplyButton, mReplyAllButton,
                    mForwardButton, mOverflowButton, mRecipientSummary,
                    mDateView, mHideDetailsView);

            setChildVisibility(mMessage.hasAttachments ? VISIBLE : GONE,
                    mAttachmentIcon);

            if (mIsDraft) {
                setChildVisibility(VISIBLE, mDraftIcon);
                setChildVisibility(GONE, mPhotoView);
            } else {
                setChildVisibility(GONE, mDraftIcon);
                setChildVisibility(VISIBLE, mPhotoView);
            }

            setChildMarginEnd(mTitleContainer, mTitleContainerMarginEnd);
        }

        final ConversationViewAdapter adapter = mMessageHeaderItem.getAdapter();
        if (adapter != null) {
            mBorderView.setVisibility(
                    adapter.isPreviousItemSuperCollapsed(mMessageHeaderItem) ? GONE : VISIBLE);
        } else {
            mBorderView.setVisibility(VISIBLE);
        }
        /// M: request layout to force layout the MesasgeHeaderView after child view gemoetry
        // changed, cause this maybe called outside of a layout pass, eg. message loaded completely
        // -->render-->update() @{
        if (!isInLayout()) {
            requestLayout();
        }
        /// @}
    }

    /**
     * If an overflow menu is present in this header's layout, set the
     * visibility of "Reply" and "Reply All" actions based on a user preference.
     * Only one of those actions will be visible when an overflow is present. If
     * no overflow is present (e.g. big phone or tablet), it's assumed we have
     * plenty of screen real estate and can show both.
     */
    private void setReplyOrReplyAllVisible() {
        if (mIsDraft) {
            setChildVisibility(GONE, mReplyButton, mReplyAllButton);
            return;
        } else if (mOverflowButton == null) {
            setChildVisibility(VISIBLE, mReplyButton, mReplyAllButton);
            return;
        }

        final Account account = getAccount();
        final boolean defaultReplyAll = (account != null) ? account.settings.replyBehavior
                == UIProvider.DefaultReplyBehavior.REPLY_ALL : false;
        /// M: Add log for debugging reply UI
        LogUtils.d(LOG_TAG, "setReplyOrReplyAllVisible defaultReplyAll = " + defaultReplyAll
            + ", mReplyButton = " + mReplyButton + ", mReplyAllButton = " + mReplyAllButton);
        setChildVisibility(defaultReplyAll ? GONE : VISIBLE, mReplyButton);
        setChildVisibility(defaultReplyAll ? VISIBLE : GONE, mReplyAllButton);
    }

    @SuppressLint("NewApi")
    private static void setChildMarginEnd(View childView, int marginEnd) {
        MarginLayoutParams mlp = (MarginLayoutParams) childView.getLayoutParams();
        if (Utils.isRunningJBMR1OrLater()) {
            mlp.setMarginEnd(marginEnd);
        } else {
            mlp.rightMargin = marginEnd;
        }
        childView.setLayoutParams(mlp);
    }



    @VisibleForTesting
    static CharSequence getRecipientSummaryText(Context context, String meEmailAddress,
            String myName, String[] to, String[] cc, String[] bcc,
            Map<String, Address> addressCache, VeiledAddressMatcher matcher,
            BidiFormatter bidiFormatter) {

        final RecipientListsBuilder builder = new RecipientListsBuilder(
                context, meEmailAddress, myName, addressCache, matcher, bidiFormatter);

        builder.append(to);
        builder.append(cc);
        builder.appendBcc(bcc);

        return builder.build();
    }

    /**
     * Utility class to build a list of recipient lists.
     */
    private static class RecipientListsBuilder {
        private final Context mContext;
        private final String mMeEmailAddress;
        private final String mMyName;
        private final StringBuilder mBuilder = new StringBuilder();
        private final CharSequence mComma;
        private final Map<String, Address> mAddressCache;
        private final VeiledAddressMatcher mMatcher;
        private final BidiFormatter mBidiFormatter;

        int mRecipientCount = 0;
        boolean mSkipComma = true;

        public RecipientListsBuilder(Context context, String meEmailAddress, String myName,
                Map<String, Address> addressCache, VeiledAddressMatcher matcher,
                BidiFormatter bidiFormatter) {
            mContext = context;
            mMeEmailAddress = meEmailAddress;
            mMyName = myName;
            mComma = mContext.getText(R.string.enumeration_comma);
            mAddressCache = addressCache;
            mMatcher = matcher;
            mBidiFormatter = bidiFormatter;
        }

        public void append(String[] recipients) {
            final int addLimit = SUMMARY_MAX_RECIPIENTS - mRecipientCount;
            final boolean hasRecipients = appendRecipients(recipients, addLimit);
            if (hasRecipients) {
                mRecipientCount += Math.min(addLimit, recipients.length);
            }
        }

        public void appendBcc(String[] recipients) {
            final int addLimit = SUMMARY_MAX_RECIPIENTS - mRecipientCount;
            if (shouldAppendRecipients(recipients, addLimit)) {
                // add the comma before the bcc header
                // and then reset mSkipComma so we don't add a comma after "bcc: "
                if (!mSkipComma) {
                    mBuilder.append(mComma);
                    mSkipComma = true;
                }
                mBuilder.append(mContext.getString(R.string.bcc_header_for_recipient_summary));
            }
            append(recipients);
        }

        /**
         * Appends formatted recipients of the message to the recipient list,
         * as long as there are recipients left to append and the maximum number
         * of addresses limit has not been reached.
         * @param rawAddrs The addresses to append.
         * @param maxToCopy The maximum number of addresses to append.
         * @return {@code true} if a recipient has been appended. {@code false}, otherwise.
         */
        private boolean appendRecipients(String[] rawAddrs, int maxToCopy) {
            if (!shouldAppendRecipients(rawAddrs, maxToCopy)) {
                return false;
            }

            final int len = Math.min(maxToCopy, rawAddrs.length);
            for (int i = 0; i < len; i++) {
                final Address email = Utils.getAddress(mAddressCache, rawAddrs[i]);
                final String emailAddress = email.getAddress();
                final String name;
                if (mMatcher != null && mMatcher.isVeiledAddress(emailAddress)) {
                    if (TextUtils.isEmpty(email.getPersonal())) {
                        // Let's write something more readable.
                        name = mContext.getString(VeiledAddressMatcher.VEILED_SUMMARY_UNKNOWN);
                    } else {
                        name = email.getSimplifiedName();
                    }
                } else {
                    // Not a veiled address, show first part of email, or "me".
                    name = mMeEmailAddress.equals(emailAddress) ?
                            mMyName : email.getSimplifiedName();
                }

                // duplicate TextUtils.join() logic to minimize temporary allocations
                if (mSkipComma) {
                    mSkipComma = false;
                } else {
                    mBuilder.append(mComma);
                }
                mBuilder.append(mBidiFormatter.unicodeWrap(name));
            }

            return true;
        }

        /**
         * @param rawAddrs The addresses to append.
         * @param maxToCopy The maximum number of addresses to append.
         * @return {@code true} if a recipient should be appended. {@code false}, otherwise.
         */
        private boolean shouldAppendRecipients(String[] rawAddrs, int maxToCopy) {
            return rawAddrs != null && rawAddrs.length != 0 && maxToCopy != 0;
        }

        public CharSequence build() {
            return mContext.getString(R.string.to_message_header, mBuilder);
        }
    }

    private void updateContactInfo() {
        if (mContactInfoSource == null || mSender == null) {
            mPhotoView.setImageToDefault();
            mPhotoView.setContentDescription(getResources().getString(
                    R.string.contact_info_string_default));
            return;
        }

        // Set the photo to either a found Bitmap or the default
        // and ensure either the contact URI or email is set so the click
        // handling works
        String contentDesc = getResources().getString(R.string.contact_info_string,
                !TextUtils.isEmpty(mSender.getPersonal())
                        ? mSender.getPersonal()
                        : mSender.getAddress());
        mPhotoView.setContentDescription(contentDesc);
        boolean photoSet = false;
        final String email = mSender.getAddress();
        final ContactInfo info = mContactInfoSource.getContactInfo(email);
        if (info != null) {
            if (info.contactUri != null) {
                mPhotoView.assignContactUri(info.contactUri);
            } else {
                mPhotoView.assignContactFromEmail(email, true /* lazyLookup */);
            }

            if (info.photo != null) {
                mPhotoView.setImageBitmap(BitmapUtil.frameBitmapInCircle(info.photo));
                photoSet = true;
            }
        } else {
            mPhotoView.assignContactFromEmail(email, true /* lazyLookup */);
        }

        if (!photoSet) {
            mPhotoView.setImageBitmap(
                    BitmapUtil.frameBitmapInCircle(makeLetterTile(mSender.getPersonal(), email)));
        }
    }

    private Bitmap makeLetterTile(
            String displayName, String senderAddress) {
        if (mLetterTileProvider == null) {
            mLetterTileProvider = new LetterTileProvider(getContext().getResources());
        }

        final ImageCanvas.Dimensions dimensions = new ImageCanvas.Dimensions(
                mContactPhotoWidth, mContactPhotoHeight, ImageCanvas.Dimensions.SCALE_ONE);
        return mLetterTileProvider.getLetterTile(dimensions, displayName, senderAddress);
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        /// M: NP check for mPopup before dismiss the popup dialog
        if(mPopup != null){
        mPopup.dismiss();
        }
        return onClick(null, item.getItemId());
    }

    @Override
    public void onClick(View v) {
        onClick(v, v.getId());
    }

    /**
     * Handles clicks on either views or menu items. View parameter can be null
     * for menu item clicks.
     */
    public boolean onClick(final View v, final int id) {
        if (mMessage == null) {
            LogUtils.i(LOG_TAG, "ignoring message header tap on unbound view");
            return false;
        }

        boolean handled = true;

        if (id == R.id.reply) {
            ComposeActivity.reply(getContext(), getAccount(), mMessage);
        } else if (id == R.id.reply_all) {
            ComposeActivity.replyAll(getContext(), getAccount(), mMessage);
        } else if (id == R.id.forward) {
            ComposeActivity.forward(getContext(), getAccount(), mMessage);
        /// M: Remove add/remove start menu @{
        /*} else if (id == R.id.add_star) {
            mMessage.star(true);
        } else if (id == R.id.remove_star) {
            mMessage.star(false);*/
        // @}
        } else if (id == R.id.print_message) {
            printMessage();
        } else if (id == R.id.report_rendering_problem) {
            final String text = getContext().getString(R.string.report_rendering_problem_desc);
            ComposeActivity.reportRenderingFeedback(getContext(), getAccount(), mMessage,
                    text + "\n\n" + mCallbacks.getMessageTransforms(mMessage));
        } else if (id == R.id.report_rendering_improvement) {
            final String text = getContext().getString(R.string.report_rendering_improvement_desc);
            ComposeActivity.reportRenderingFeedback(getContext(), getAccount(), mMessage,
                    text + "\n\n" + mCallbacks.getMessageTransforms(mMessage));
        } else if (id == R.id.edit_draft) {
            ComposeActivity.editDraft(getContext(), getAccount(), mMessage);
        } else if (id == R.id.overflow) {
            if (mPopup == null) {
                mPopup = new PopupMenu(getContext(), v);
                mPopup.getMenuInflater().inflate(R.menu.message_header_overflow_menu,
                        mPopup.getMenu());
                mPopup.setOnMenuItemClickListener(this);
                mPopup.setOnDismissListener(this);
            } else {
                /// M: if mPopup != null, it indicated that the mPopup has been shown.
                /// And we no need create duplicate menu.
                return true;
            }
            final boolean defaultReplyAll = getAccount().settings.replyBehavior
                    == UIProvider.DefaultReplyBehavior.REPLY_ALL;
            final Menu m = mPopup.getMenu();
            m.findItem(R.id.reply).setVisible(defaultReplyAll);
            m.findItem(R.id.reply_all).setVisible(!defaultReplyAll);
            m.findItem(R.id.print_message).setVisible(Utils.isRunningKitkatOrLater());

            /// M: Remove add/remove star menu for overflow menu @{
            /*final boolean isStarred = mMessage.starred;
            boolean showStar = true;
            final Conversation conversation = mMessage.getConversation();
            if (conversation != null) {
                showStar = !conversation.isInTrash();
            }
            m.findItem(R.id.add_star).setVisible(showStar && !isStarred);
            m.findItem(R.id.remove_star).setVisible(showStar && isStarred);*/
            // @}

            final boolean reportRendering = ENABLE_REPORT_RENDERING_PROBLEM
                && mCallbacks.supportsMessageTransforms();
            m.findItem(R.id.report_rendering_improvement).setVisible(reportRendering);
            m.findItem(R.id.report_rendering_problem).setVisible(reportRendering);

            /** M: Adjust the UI layout and logic to support VIP new feature @{ */
            MenuItem item = m.findItem(R.id.vip_switcher_menu);
            if (mIsFromVIP) {
                item.setTitle(R.string.vip_remove_title);
            } else {
                item.setTitle(R.string.vip_choose_contact);
            }
            /** @} */

            mPopup.show();
        } else if (id == R.id.send_date || id == R.id.hide_details ||
                id == R.id.details_expanded_content) {
            toggleMessageDetails();
        } else if (id == R.id.upper_header) {
            toggleExpanded();
        } else if (id == R.id.show_pictures_text) {
            handleShowImagePromptClick(v);
        /** M: Adjust the UI layout and logic to support VIP new feature @{ */
        } else if (id == R.id.vip_switcher_menu) {
            onVipSwitcherClick();
        /** @} */
        } else {
            LogUtils.i(LOG_TAG, "unrecognized header tap: %d", id);
            handled = false;
        }

        if (handled && id != R.id.overflow) {
            Analytics.getInstance().sendMenuItemEvent(Analytics.EVENT_CATEGORY_MENU_ITEM, id,
                    "message_header", 0);
        }

        return handled;
    }

    private void printMessage() {
        // Secure conversation view does not use a conversation view adapter
        // so it's safe to test for existence as a signal to use javascript or not.
        final boolean useJavascript = mMessageHeaderItem.getAdapter() != null;
        final Account account = getAccount();
        final Conversation conversation = mMessage.getConversation();
        final String baseUri =
                AbstractConversationViewFragment.buildBaseUri(getContext(), account, conversation);
        PrintUtils.printMessage(getContext(), mMessage, conversation.subject,
                mAddressCache, conversation.getBaseUri(baseUri), useJavascript);
    }

    /**
     * Set to true if the user should not be able to perform message actions
     * on the message such as reply/reply all/forward/star/etc.
     *
     * Default is false.
     */
    public void setViewOnlyMode(boolean isViewOnlyMode) {
        /// M: Record the external demand
//        mIsViewOnlyMode = isViewOnlyMode;
        mIsViewOnlyModeFromOutside = isViewOnlyMode;
    }

    public void setExpandable(boolean expandable) {
        mExpandable = expandable;
    }

    public void toggleExpanded() {
        if (!mExpandable) {
            return;
        }
        LogUtils.d(LOG_TAG, "MessageHeaderView toggleExpanded Expanded:%s, Snappy:%s",
                isExpanded(), isSnappy());
        setExpanded(!isExpanded());

        // The snappy header will disappear; no reason to update text.
        if (!isSnappy()) {
            mSenderNameView.setText(getHeaderTitle());
            setRecipientSummary();
            setDateText();
            mSnippetView.setText(mSnippet);
        }

        updateChildVisibility();

        // Force-measure the new header height so we can set the spacer size and
        // reveal the message div in one pass. Force-measuring makes it unnecessary to set
        // mSizeChanged.
        int h = measureHeight();
        mMessageHeaderItem.setHeight(h);
        if (mCallbacks != null) {
            mCallbacks.setMessageExpanded(mMessageHeaderItem, h);
        }
    }

    private static boolean isValidPosition(int position, int size) {
        return position >= 0 && position < size;
    }

    @Override
    public void setSnappy() {
        mIsSnappy = true;
        hideMessageDetails();
    }

    private boolean isSnappy() {
        return mIsSnappy;
    }

    private void toggleMessageDetails() {
        int heightBefore = measureHeight();
        final boolean expand =
                (mExpandedDetailsView == null || mExpandedDetailsView.getVisibility() == GONE);
        setMessageDetailsExpanded(expand);
        if (mCallbacks != null) {
            mCallbacks.setMessageDetailsExpanded(mMessageHeaderItem, expand, heightBefore);
        }
    }

    private void setMessageDetailsExpanded(boolean expand) {
        LogUtils.d(LOG_TAG, "MessageHeaderView setMessageDetailsExpanded %s", expand);
        if (expand) {
            showExpandedDetails();
        } else {
            hideExpandedDetails();
        }

        if (mMessageHeaderItem != null) {
            mMessageHeaderItem.detailsExpanded = expand;
        }
        /// M: update space height here that to avoid some overlay problem
        updateSpacerHeight();
    }

    public void setMessageDetailsVisibility(int vis) {
        LogUtils.d(LOG_TAG, "MessageHeaderView setMessageDetailsVisibility %d", vis);
        if (vis == GONE) {
            hideExpandedDetails();
            hideSpamWarning();
            hideShowImagePrompt();
            hideInvite();
            mUpperHeaderView.setOnCreateContextMenuListener(null);
        } else {
            setMessageDetailsExpanded(mMessageHeaderItem.detailsExpanded);
            if (mMessage.spamWarningString == null) {
                hideSpamWarning();
            } else {
                showSpamWarning();
            }
            if (mShowImagePrompt) {
                if (mMessageHeaderItem.getShowImages()) {
                    showImagePromptAlways(true);
                } else {
                    showImagePromptOnce();
                }
            } else {
                hideShowImagePrompt();
            }
            if (mMessage.isFlaggedCalendarInvite()) {
                showInvite();
            } else {
                hideInvite();
            }
            mUpperHeaderView.setOnCreateContextMenuListener(mEmailCopyMenu);
        }
    }

    private void hideMessageDetails() {
        setMessageDetailsVisibility(GONE);
    }

    private void hideExpandedDetails() {
        if (mExpandedDetailsView != null) {
            mExpandedDetailsView.setVisibility(GONE);
        }
        mDateView.setVisibility(VISIBLE);
        mHideDetailsView.setVisibility(GONE);
    }

    private void hideInvite() {
        if (mInviteView != null) {
            mInviteView.setVisibility(GONE);
        }
    }

    private void showInvite() {
        if (mInviteView == null) {
            mInviteView = (MessageInviteView) mInflater.inflate(
                    R.layout.conversation_message_invite, this, false);
            mExtraContentView.addView(mInviteView);
        }
        mInviteView.bind(mMessage);
        mInviteView.setVisibility(VISIBLE);
    }

    private void hideShowImagePrompt() {
        if (mImagePromptView != null) {
            mImagePromptView.setVisibility(GONE);
        }
    }

    private void showImagePromptOnce() {
        if (mImagePromptView == null) {
            mImagePromptView = (TextView) mInflater.inflate(
                    R.layout.conversation_message_show_pics, this, false);
            mExtraContentView.addView(mImagePromptView);
            mImagePromptView.setOnClickListener(this);
        }
        mImagePromptView.setVisibility(VISIBLE);
        mImagePromptView.setText(R.string.show_images);
        mImagePromptView.setTag(SHOW_IMAGE_PROMPT_ONCE);
    }

    /**
     * Shows the "Always show pictures" message
     *
     * @param initialShowing <code>true</code> if this is the first time we are showing the prompt
     *        for "show images", <code>false</code> if we are transitioning from "Show pictures"
     */
    private void showImagePromptAlways(final boolean initialShowing) {
        if (initialShowing) {
            // Initialize the view
            showImagePromptOnce();
        }

        mImagePromptView.setText(R.string.always_show_images);
        mImagePromptView.setTag(SHOW_IMAGE_PROMPT_ALWAYS);

        if (!initialShowing) {
            // the new text's line count may differ, so update the spacer height
            updateSpacerHeight();
        }
    }

    private void hideSpamWarning() {
        if (mSpamWarningView != null) {
            mSpamWarningView.setVisibility(GONE);
        }
    }

    private void showSpamWarning() {
        if (mSpamWarningView == null) {
            mSpamWarningView = (SpamWarningView)
                    mInflater.inflate(R.layout.conversation_message_spam_warning, this, false);
            mExtraContentView.addView(mSpamWarningView);
        }

        mSpamWarningView.showSpamWarning(mMessage, mSender);
    }

    private void handleShowImagePromptClick(View v) {
        Integer state = (Integer) v.getTag();
        if (state == null) {
            return;
        }
        switch (state) {
            case SHOW_IMAGE_PROMPT_ONCE:
                if (mCallbacks != null) {
                    mCallbacks.showExternalResources(mMessage);
                }
                if (mMessageHeaderItem != null) {
                    mMessageHeaderItem.setShowImages(true);
                }
                if (mIsViewOnlyMode) {
                    hideShowImagePrompt();
                } else {
                    showImagePromptAlways(false);
                }
                break;
            case SHOW_IMAGE_PROMPT_ALWAYS:
                mMessage.markAlwaysShowImages(getQueryHandler(), 0 /* token */, null /* cookie */);

                if (mCallbacks != null) {
                    mCallbacks.showExternalResources(mMessage.getFrom());
                }

                mShowImagePrompt = false;
                v.setTag(null);
                v.setVisibility(GONE);
                updateSpacerHeight();
                Toast.makeText(getContext(), R.string.always_show_images_toast, Toast.LENGTH_SHORT)
                        .show();
                break;
        }
    }

    private AsyncQueryHandler getQueryHandler() {
        if (mQueryHandler == null) {
            mQueryHandler = new AsyncQueryHandler(getContext().getContentResolver()) { };
        }
        return mQueryHandler;
    }

    /**
     * Makes expanded details visible. If necessary, will inflate expanded
     * details layout and render using saved-off state (senders, timestamp,
     * etc).
     */
    private void showExpandedDetails() {
        // lazily create expanded details view
        final boolean expandedViewCreated = ensureExpandedDetailsView();
        if (expandedViewCreated) {
            mExtraContentView.addView(mExpandedDetailsView, 0);
        }
        mExpandedDetailsView.setVisibility(VISIBLE);
        mDateView.setVisibility(GONE);
        mHideDetailsView.setVisibility(VISIBLE);
    }

    private boolean ensureExpandedDetailsView() {
        boolean viewCreated = false;
        if (mExpandedDetailsView == null) {
            View v = inflateExpandedDetails(mInflater);
            v.setOnClickListener(this);

            mExpandedDetailsView = (ViewGroup) v;
            viewCreated = true;
        }
        if (!mExpandedDetailsValid) {
            renderExpandedDetails(getResources(), mExpandedDetailsView, mMessage.viaDomain,
                    mAddressCache, getAccount(), mVeiledMatcher, mFrom, mReplyTo, mTo, mCc, mBcc,
                    mMessageHeaderItem.getTimestampFull(),
                    getBidiFormatter());

            mExpandedDetailsValid = true;
        }
        return viewCreated;
    }

    public static View inflateExpandedDetails(LayoutInflater inflater) {
        return inflater.inflate(R.layout.conversation_message_header_details, null, false);
    }

    public static void renderExpandedDetails(Resources res, View detailsView,
            String viaDomain, Map<String, Address> addressCache, Account account,
            VeiledAddressMatcher veiledMatcher, String[] from, String[] replyTo,
            String[] to, String[] cc, String[] bcc, CharSequence receivedTimestamp,
            BidiFormatter bidiFormatter) {
        renderEmailList(res, R.id.from_heading, R.id.from_details, from, viaDomain,
                detailsView, addressCache, account, veiledMatcher, bidiFormatter);
        renderEmailList(res, R.id.replyto_heading, R.id.replyto_details, replyTo, viaDomain,
                detailsView, addressCache, account, veiledMatcher, bidiFormatter);
        renderEmailList(res, R.id.to_heading, R.id.to_details, to, viaDomain,
                detailsView, addressCache, account, veiledMatcher, bidiFormatter);
        renderEmailList(res, R.id.cc_heading, R.id.cc_details, cc, viaDomain,
                detailsView, addressCache, account, veiledMatcher, bidiFormatter);
        renderEmailList(res, R.id.bcc_heading, R.id.bcc_details, bcc, viaDomain,
                detailsView, addressCache, account, veiledMatcher, bidiFormatter);

        // Render date
        detailsView.findViewById(R.id.date_heading).setVisibility(VISIBLE);
        final TextView date = (TextView) detailsView.findViewById(R.id.date_details);
        date.setText(receivedTimestamp);
        date.setVisibility(VISIBLE);
    }

    /**
     * Render an email list for the expanded message details view.
     */
    private static void renderEmailList(Resources res, int headerId, int detailsId,
            String[] emails, String viaDomain, View rootView,
            Map<String, Address> addressCache, Account account,
            VeiledAddressMatcher veiledMatcher, BidiFormatter bidiFormatter) {
        final TextView detailsText = (TextView) rootView.findViewById(detailsId);
        if (emails == null || emails.length == 0) {
            /** M: When address is null, need update the view's display.@{ */
            if (rootView.findViewById(headerId).getVisibility() == View.VISIBLE) {
                detailsText.setText(null);
                detailsText.setVisibility(GONE);
                rootView.findViewById(headerId).setVisibility(GONE);
            }
            /** @} */
            return;
        }
        final String[] formattedEmails = new String[emails.length];
        for (int i = 0; i < emails.length; i++) {
            final Address email = Utils.getAddress(addressCache, emails[i]);
            String name = email.getPersonal();
            final String address = email.getAddress();
            // Check if the address here is a veiled address.  If it is, we need to display an
            // alternate layout
            final boolean isVeiledAddress = veiledMatcher != null &&
                    veiledMatcher.isVeiledAddress(address);
            final String addressShown;
            if (isVeiledAddress) {
                // Add the warning at the end of the name, and remove the address.  The alternate
                // text cannot be put in the address part, because the address is made into a link,
                // and the alternate human-readable text is not a link.
                addressShown = "";
                if (TextUtils.isEmpty(name)) {
                    // Empty name and we will block out the address. Let's write something more
                    // readable.
                    name = res.getString(VeiledAddressMatcher.VEILED_ALTERNATE_TEXT_UNKNOWN_PERSON);
                } else {
                    name = name + res.getString(VeiledAddressMatcher.VEILED_ALTERNATE_TEXT);
                }
            } else {
                /// M: insert a space between name and address so that text view will auto change line
                /// when no space, it will make UI more clear.
                /// Ignore case of recipients without name, only address.@{
                if (!TextUtils.isEmpty(name)) {
                    addressShown = " " + address;
                } else {
                    addressShown = address;
                }
                /// @}
            }
            if (name == null || name.length() == 0 || name.equalsIgnoreCase(addressShown)) {
                formattedEmails[i] = bidiFormatter.unicodeWrap(addressShown);
            } else {
                // The one downside to having the showViaDomain here is that
                // if the sender does not have a name, it will not show the via info
                if (viaDomain != null) {
                    formattedEmails[i] = res.getString(
                            R.string.address_display_format_with_via_domain,
                            bidiFormatter.unicodeWrap(name),
                            bidiFormatter.unicodeWrap(addressShown),
                            bidiFormatter.unicodeWrap(viaDomain));
                } else {
                    formattedEmails[i] = res.getString(R.string.address_display_format,
                            bidiFormatter.unicodeWrap(name),
                            bidiFormatter.unicodeWrap(addressShown));
                }
            }

            /// M: Add a space for VIP Icon space
            if (VipMemberCache.isVIP(addressShown)) {
                formattedEmails[i] = LIST_DELIMITER_EMAIL + formattedEmails[i];
            }
        }

        rootView.findViewById(headerId).setVisibility(VISIBLE);
        detailsText.setText(TextUtils.join("\n", formattedEmails));
        stripUnderlines(detailsText, account);
        /// M: Add a space for VIP Icon
        addVipIcon(detailsText);
        detailsText.setVisibility(VISIBLE);
    }

    private static void stripUnderlines(TextView textView, Account account) {
        final Spannable spannable = (Spannable) textView.getText();
        final URLSpan[] urls = textView.getUrls();

        for (URLSpan span : urls) {
            final int start = spannable.getSpanStart(span);
            final int end = spannable.getSpanEnd(span);
            spannable.removeSpan(span);
            span = new EmailAddressSpan(account, span.getURL().substring(7));
            spannable.setSpan(span, start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
    }

    /**
     * Returns a short plaintext snippet generated from the given HTML message
     * body. Collapses whitespace, ignores '&lt;' and '&gt;' characters and
     * everything in between, and truncates the snippet to no more than 100
     * characters.
     *
     * @return Short plaintext snippet
     */
    @VisibleForTesting
    static String makeSnippet(final String messageBody) {
        if (TextUtils.isEmpty(messageBody)) {
            return null;
        }

        final StringBuilder snippet = new StringBuilder(MAX_SNIPPET_LENGTH);

        final StringReader reader = new StringReader(messageBody);
        try {
            int c;
            while ((c = reader.read()) != -1 && snippet.length() < MAX_SNIPPET_LENGTH) {
                // Collapse whitespace.
                if (Character.isWhitespace(c)) {
                    snippet.append(' ');
                    do {
                        c = reader.read();
                    } while (Character.isWhitespace(c));
                    if (c == -1) {
                        break;
                    }
                }

                if (c == '<') {
                    // Ignore everything up to and including the next '>'
                    // character.
                    while ((c = reader.read()) != -1) {
                        if (c == '>') {
                            break;
                        }
                    }

                    // If we reached the end of the message body, exit.
                    if (c == -1) {
                        break;
                    }
                } else if (c == '&') {
                    // Read HTML entity.
                    StringBuilder sb = new StringBuilder();

                    while ((c = reader.read()) != -1) {
                        if (c == ';') {
                            break;
                        }
                        sb.append((char) c);
                    }

                    String entity = sb.toString();
                    if ("nbsp".equals(entity)) {
                        snippet.append(' ');
                    } else if ("lt".equals(entity)) {
                        snippet.append('<');
                    } else if ("gt".equals(entity)) {
                        snippet.append('>');
                    } else if ("amp".equals(entity)) {
                        snippet.append('&');
                    } else if ("quot".equals(entity)) {
                        snippet.append('"');
                    } else if ("apos".equals(entity) || "#39".equals(entity)) {
                        snippet.append('\'');
                    } else {
                        // Unknown entity; just append the literal string.
                        snippet.append('&').append(entity);
                        if (c == ';') {
                            snippet.append(';');
                        }
                    }

                    // If we reached the end of the message body, exit.
                    if (c == -1) {
                        break;
                    }
                } else {
                    // The current character is a non-whitespace character that
                    // isn't inside some
                    // HTML tag and is not part of an HTML entity.
                    snippet.append((char) c);
                }
            }
        } catch (IOException e) {
            LogUtils.wtf(LOG_TAG, e, "Really? IOException while reading a freaking string?!? ");
        }

        return snippet.toString();
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        Timer perf = new Timer();
        perf.start(LAYOUT_TAG);
        super.onLayout(changed, l, t, r, b);
        perf.pause(LAYOUT_TAG);
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        Timer t = new Timer();
        if (Timer.ENABLE_TIMER && !mPreMeasuring) {
            t.count("header measure id=" + mMessage.id);
            t.start(MEASURE_TAG);
        }
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        if (!mPreMeasuring) {
            t.pause(MEASURE_TAG);
        }
    }

    /// M: Observing VIP info changes
    private ThrottleContentObserver mVipObserver;

    /**
     * M: VIP feature: Update the sender vip icon on the sub header.
     * @param isVip Whether this sender is vip now.
     */
    public void updateSenderVipIcon(boolean isVip) {
        if (isVip) {
            mIsFromVIP = true;
            mSenderNameView.setCompoundDrawablesWithIntrinsicBounds(
                    R.drawable.ic_email_vip, 0, 0, 0);
        } else {
            mIsFromVIP = false;
            mSenderNameView.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, 0);
        }
    }

    /**
     * M: Add the VIP Icon in the detailsText.
     * @param textView the detailsText
     */
    private static void addVipIcon(TextView textView) {
        final Spannable spannable = (Spannable) textView.getText();
        String mails = spannable.toString();
        int start = -1;
        do {
            start = mails.indexOf(LIST_DELIMITER_EMAIL, start + 1);
            if (start >= 0) {
                ImageSpan imageSpan = new ImageSpan(textView.getContext(), R.drawable.ic_email_vip,
                        DynamicDrawableSpan.ALIGN_BASELINE);
                spannable.setSpan(imageSpan, start, start + 1, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
        } while (start >= 0);
    }

    /**
     * M: The action of clicking the vip switcher button
     */
    private void onVipSwitcherClick() {
        if (mSender == null) {
            return;
        }
        if (VipMemberCache.isVIP(mSender.getAddress())) {
            removeVip(mSender);
        } else {
            addVip(mSender);
        }
    }

    /**
     * M: Remove the address from vips
     * @param address  the address that contains email address and display name
     */
    private void removeVip(Address address) {
        new EmailAsyncTask<Address, Void, Boolean>(null) {

            @Override
            protected Boolean doInBackground(Address... addresses) {
                if (addresses == null || addresses.length == 0) {
                    return false;
                }
                String mail = addresses[0].getAddress();
                mail = com.android.emailcommon.mail.Address.getFirstMailAddress(mail);
                boolean result = VipMember.deleteVipMembers(getContext(),
                        mail) >= 0;
                VipMemberCache.updateVipMemberCache();
                return result;
            }

            @Override
            protected void onSuccess(Boolean result) {
                if (result) {
                    updateSenderVipIcon(false);
                }
            };

        }.executeParallel(address);
    }

    /**
     * M: Add the address as VIP
     * @param address the address that contains email address and display name
     */
    private void addVip(Address address) {
        new EmailAsyncTask<Address, Void, Boolean>(null) {
            private boolean mIsDuplicateVip = false;

            @Override
            protected Boolean doInBackground(Address... addresses) {
                String mail = addresses[0].getAddress();
                mail = com.android.emailcommon.mail.Address.getFirstMailAddress(mail);
                com.android.emailcommon.mail.Address addr =
                        new com.android.emailcommon.mail.Address(
                                mail, addresses[0].getSimplifiedName());
                boolean result = VipMember.addVIP(getContext(), addr,
                        new VipMember.AddVipsCallback() {
                    @Override
                    public void tryToAddDuplicateVip() {
                        Utility.showToast(getContext(), R.string.not_add_duplicate_vip);
                        mIsDuplicateVip = true;
                    }
                    @Override
                    public void addVipOverMax() {
                        Utility.showToast(getContext(), R.string.can_not_add_vip_over_99);
                    }
                });
                VipMemberCache.updateVipMemberCache();
                return result;
            }

            @Override
            protected void onSuccess(Boolean result) {
                updateSenderVipIcon(result || mIsDuplicateVip);
            };

        }.executeParallel(address);
    }

    /// M: Register the VIP member observer
    private void registerVipObserver() {
        if (mVipObserver == null) {
            Handler handler = new Handler();
            Throttle throttle = new Throttle("VipMembersObserver-Throttle", new Runnable() {
                @Override
                public void run() {
                    if (!mIsDraft) {
                        refresh();
                    }
                }
            }, handler, 500, Throttle.DEFAULT_MAX_TIMEOUT);

            ThrottleContentObserver observer = new ThrottleContentObserver(handler,
                    getContext(), "VipMembersObserver", throttle);
            observer.register(VipMember.UIPROVIDER_VIPMEMBER_NOTIFIER);
            mVipObserver = observer;
            LogUtils.d(LOG_TAG, "MessageHeaderView registerContentObserver for VIP member >>>>>");
        }
    }
    /// @}

    /// M: Unregister the VIP member observer
    private void unregisterVipObserver() {
        if (mVipObserver != null) {
            mVipObserver.unregister();
            mVipObserver = null;
            LogUtils.d(LOG_TAG, "MessageHeaderView unregisterContentObserver for VIP member <<<<<");
        }
    }
    /// @}

    /* M: add for tracking the popup menu's visibility.
     * (non-Javadoc)
     * @see android.widget.PopupMenu.OnDismissListener#onDismiss(android.widget.PopupMenu)
     */
    @Override
    public void onDismiss(PopupMenu menu) {
        mPopup = null;
    }

    /*
     * M: add for unbinding the observers from the view when the view detached from Window.
     */
    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        unbind();
    }
}
