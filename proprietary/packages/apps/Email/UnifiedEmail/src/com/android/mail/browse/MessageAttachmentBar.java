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

import android.app.AlertDialog;
import android.app.FragmentManager;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import androidx.core.text.BidiFormatter;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.android.emailcommon.utility.AttachmentUtilities;
import com.android.emailcommon.utility.Utility;
import com.android.mail.R;
import com.android.mail.analytics.Analytics;
import com.android.mail.providers.Account;
import com.android.mail.providers.Attachment;
import com.android.mail.providers.UIProvider.AttachmentDestination;
import com.android.mail.providers.UIProvider.AttachmentState;
import com.android.mail.ui.AccountFeedbackActivity;
import com.android.mail.utils.AttachmentUtils;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.MimeType;
import com.android.mail.utils.StorageLowState;
import com.android.mail.utils.Utils;

/**
 * View for a single attachment in conversation view. Shows download status and allows launching
 * intents to act on an attachment.
 *
 */
public class MessageAttachmentBar extends FrameLayout implements OnClickListener,
        OnMenuItemClickListener, AttachmentViewInterface {

    ///M: add title for 3rd App to show the right file name
    private static final String INTENT_ATTACHMENT_TITLE = "title";
    private Attachment mAttachment;
    private TextView mTitle;
    private TextView mSubTitle;
    private String mAttachmentSizeText;
    private String mDisplayType;
    private ProgressBar mProgress;
    private ImageButton mCancelButton;
    private PopupMenu mPopup;
    private ImageView mOverflowButton;

    private final AttachmentActionHandler mActionHandler;
    private boolean mSaveClicked;
    private Account mAccount;
    /// M: Is view only mode @{
    private boolean mIsViewOnlyMode;
    /// @}

    private final Runnable mUpdateRunnable = new Runnable() {
            @Override
        public void run() {
            updateActionsInternal();
        }
    };

    private static final String LOG_TAG = LogTag.getLogTag();

    /**
     * Boolean used to tell whether extra option 1 should always be hidden.
     * Currently makes sure that there is no conversation because that state
     * means that we're in the EML viewer.
     */
    private boolean mHideExtraOptionOne;


    public MessageAttachmentBar(Context context) {
        this(context, null);
    }

    public MessageAttachmentBar(Context context, AttributeSet attrs) {
        super(context, attrs);

        mActionHandler = new AttachmentActionHandler(context, this);
    }

    public void initialize(FragmentManager fragmentManager) {
        mActionHandler.initialize(fragmentManager);
    }

    /**
     * M: set view only mode
     * @param isViewOnlyMode
     */
    public void setViewOnlyMode(boolean isViewOnlyMode) {
        mIsViewOnlyMode = isViewOnlyMode;
    }

    public static MessageAttachmentBar inflate(LayoutInflater inflater, ViewGroup parent) {
        MessageAttachmentBar view = (MessageAttachmentBar) inflater.inflate(
                R.layout.conversation_message_attachment_bar, parent, false);
        return view;
    }

    /**
     * Render or update an attachment's view. This happens immediately upon instantiation, and
     * repeatedly as status updates stream in, so only properties with new or changed values will
     * cause sub-views to update.
     */
    public void render(Attachment attachment, Account account, ConversationMessage message,
            boolean loaderResult, BidiFormatter bidiFormatter) {
        // get account uri for potential eml viewer usage
        mAccount = account;

        final Attachment prevAttachment = mAttachment;
        mAttachment = attachment;
        if (mAccount != null) {
            mActionHandler.setAccount(mAccount.getEmailAddress());
        }
        mActionHandler.setMessage(message);
        mActionHandler.setAttachment(mAttachment);
        mHideExtraOptionOne = message.getConversation() == null;

        // reset mSaveClicked if we are not currently downloading
        // So if the download fails or the download completes, we stop
        // showing progress, etc
        /// M: there was a timing issue that user clicked save button, but the asynchronous
        /// updating not executed yet. If render running here, the mSaveClicked would be set false,
        /// and has no chance to revert to true. Then the UI state became wrong...@{
        mSaveClicked = attachment.isDownloading() ? true : false;

        LogUtils.d(LOG_TAG, "got attachment list row: name=%s state/dest=%d/%d dled=%d" +
                " contentUri=%s MIME=%s flags=%d", attachment.getName(), attachment.state,
                attachment.destination, attachment.downloadedSize, attachment.contentUri,
                attachment.getContentType(), attachment.flags);

        final String attachmentName = attachment.getName();
        if ((attachment.flags & Attachment.FLAG_DUMMY_ATTACHMENT) != 0) {
            mTitle.setText(R.string.load_attachment);
        } else if (prevAttachment == null
                || !TextUtils.equals(attachmentName, prevAttachment.getName())) {
            mTitle.setText(attachmentName);
        }

        if (prevAttachment == null || attachment.size != prevAttachment.size) {
            mAttachmentSizeText = bidiFormatter.unicodeWrap(
                    AttachmentUtils.convertToHumanReadableSize(getContext(), attachment.size));
            mDisplayType = bidiFormatter.unicodeWrap(
                    AttachmentUtils.getDisplayType(getContext(), attachment));
            updateSubtitleText();
        }

        updateActions();
        mActionHandler.updateStatus(loaderResult);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mTitle = (TextView) findViewById(R.id.attachment_title);
        mSubTitle = (TextView) findViewById(R.id.attachment_subtitle);
        mProgress = (ProgressBar) findViewById(R.id.attachment_progress);
        mOverflowButton = (ImageView) findViewById(R.id.overflow);
        mCancelButton = (ImageButton) findViewById(R.id.cancel_attachment);

        setOnClickListener(this);
        mOverflowButton.setOnClickListener(this);
        mCancelButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {
        onClick(v.getId(), v);
    }

    @Override
    public boolean onMenuItemClick(MenuItem item) {
        mPopup.dismiss();
        return onClick(item.getItemId(), null);
    }

    private boolean onClick(final int res, final View v) {
        if (res == R.id.preview_attachment) {
            previewAttachment();
        } else if (res == R.id.save_attachment) {
            /// M: We Can't load attachment in low storage state @{
            if (StorageLowState.checkIfStorageLow(mContext, StorageLowState.CHECK_FOR_DOWNLOAD_ATTACHMENT)) {
                LogUtils.e(LOG_TAG, "Can't download attachment due to low storage");
                return true;
            }
            /// @}
            /** M: When size is 0, alert user and do not need to down load the attachment.@{ */
            if (mAttachment.size <= 0) {
                alertDialog();
                return true;
            }
            /** @} */
            if (mAttachment.canSave()) {
                /** M: When SD card 's space is not enough. the attachment can not been saved. @{ */
                if (!AttachmentUtils.isSDCardSpaceEnough(getContext(), mAttachment.size, true)) {
                    LogUtils.e(LOG_TAG, "Can't download attachment due to SD card space is not enough");
                    return true;
                }
                /** @} */
                /// M: If the attachment file has existed, just copy it to external. no need download again.@{
                if (Utility.fileExists(mContext, mAttachment.contentUri)) {
                    AttachmentUtilities.copyAttachmentFromInternalToExternal(mContext, mAttachment.uri,
                            mAttachment.contentUri);
                } else {
                    /// M: Can't download when no network available, prompt a dialog @{
                    if (!AttachmentUtils.canDownloadAttachment(mContext, null)) {
                        mActionHandler.showConnectionAlertDialog();
                        return true;
                    }
                    /// @}
                    mActionHandler.startDownloadingAttachment(AttachmentDestination.EXTERNAL);
                }
                /// @}
                mSaveClicked = true;

                Analytics.getInstance().sendEvent(
                        "save_attachment", Utils.normalizeMimeType(mAttachment.getContentType()),
                        "attachment_bar", mAttachment.size);
            }
        } else if (res == R.id.download_again) {
            /// M: We Can't load attachment in low storage state @{
            if (StorageLowState.checkIfStorageLow(mContext, StorageLowState.CHECK_FOR_DOWNLOAD_ATTACHMENT)) {
                LogUtils.e(LOG_TAG, "Can't download attachment due to low storage");
                return true;
            }
            /// @}
            /** M: When size is 0, alert user and do not need to down load the attachment.@{ */
            if (mAttachment.size <= 0) {
                alertDialog();
                return true;
            }
            /** @} */
            /** M: When SD card 's space is not enough. the attachment can not been saved. @{ */
            if (!AttachmentUtils.isSDCardSpaceEnough(getContext(), mAttachment.size, true)) {
                LogUtils.e(LOG_TAG, "Can't download attachment due to SD card space is not enough");
                return true;
            }
            /** @} */
            /// M: download_again menu is shown when attachment saved/failed
            if (mAttachment.isDownloadFinishedOrFailed()) {
                /// M: Can't download when no network available, prompt a dialog @{
                if (!AttachmentUtils.canDownloadAttachment(mContext, null)) {
                    mActionHandler.showConnectionAlertDialog();
                    return true;
                }
                /// @}
                mActionHandler.showDownloadingDialog();
                mActionHandler.startRedownloadingAttachment(mAttachment);

                Analytics.getInstance().sendEvent("redownload_attachment",
                        Utils.normalizeMimeType(mAttachment.getContentType()), "attachment_bar",
                        mAttachment.size);
            }
        } else if (res == R.id.cancel_attachment) {
            mActionHandler.cancelAttachment();
            mSaveClicked = false;

            Analytics.getInstance().sendEvent(
                    "cancel_attachment", Utils.normalizeMimeType(mAttachment.getContentType()),
                    "attachment_bar", mAttachment.size);
        } else if (res == R.id.attachment_extra_option1) {
            mActionHandler.handleOption1();
        } else if (res == R.id.overflow) {
            // If no overflow items are visible, just bail out.
            // We shouldn't be able to get here anyhow since the overflow
            // button should be hidden.
            if (shouldShowOverflow()) {
                if (mPopup == null) {
                    mPopup = new PopupMenu(getContext(), v);
                    mPopup.getMenuInflater().inflate(R.menu.message_footer_overflow_menu,
                            mPopup.getMenu());
                    mPopup.setOnMenuItemClickListener(this);
                }

                final Menu menu = mPopup.getMenu();
                menu.findItem(R.id.preview_attachment).setVisible(shouldShowPreview());
                menu.findItem(R.id.save_attachment).setVisible(shouldShowSave());
                menu.findItem(R.id.download_again).setVisible(shouldShowDownloadAgain());
                menu.findItem(R.id.attachment_extra_option1).setVisible(shouldShowExtraOption1());

                mPopup.show();
            }
        } else {
            // Handles clicking the attachment
            // in any area that is not the overflow
            // button or cancel button or one of the
            // overflow items.
            /// M: We Can't load attachment in low storage state @{
            if (StorageLowState.checkIfStorageLow(mContext, StorageLowState.CHECK_FOR_VIEW_ATTACHMENT)) {
                LogUtils.e(LOG_TAG, "Can't view attachment due to low storage");
                return true;
            }
            /// @}
            /** M: When size is 0, alert user.@{ */
            if (mAttachment.size <= 0) {
                alertDialog();
                return true;
            }
            /** @} */
            final String mime = Utils.normalizeMimeType(mAttachment.getContentType());
            final String action;

            if ((mAttachment.flags & Attachment.FLAG_DUMMY_ATTACHMENT) != 0) {
                /// M: Can't download when no network available, prompt a dialog @{
                if (!AttachmentUtils.canDownloadAttachment(mContext, null)) {
                    mActionHandler.showConnectionAlertDialog();
                    return true;
                }
                /// @}
                // This is a dummy. We need to download it, but not attempt to open or preview.
                mActionHandler.showDownloadingDialog();
                mActionHandler.setViewOnFinish(false);
                mActionHandler.startDownloadingAttachment(AttachmentDestination.CACHE);

                action = null;
            }
            // If we can install, install.
            else if (MimeType.isInstallable(mAttachment.getContentType())) {
                // Save to external because the package manager only handles
                // file:// uris not content:// uris. We do the same
                // workaround in
                // UiProvider#getUiAttachmentsCursorForUIAttachments()
                /// M: If the attachment file has existed, just copy it to external. no need download again.@{
                if (mAttachment.destination == AttachmentDestination.CACHE
                        && Utility.fileExists(mContext, mAttachment.contentUri)) {
                    /// M: If we are not in draft or outbox, do whatever you want
                    if (!mIsViewOnlyMode) {
                        mActionHandler.showDownloadingDialog();
                        AttachmentUtilities.copyAttachmentFromInternalToExternal(
                                mContext, mAttachment.uri, mAttachment.contentUri);
                    } else {
                        /// M: For draft or outbox apk attachment, just copy the attachment to
                        // external but not delete the internal data.@{
                        AttachmentUtilities.copyAttachmentFromInternalToExternal(
                                mContext, mAttachment.uri, mAttachment.contentUri,
                                new AttachmentUtilities.CopyAttachmentCallback() {
                                    @Override
                                    public void onCopyCompleted(String uri) {
                                        if (null == uri) {
                                            LogUtils.e(LOG_TAG, "Copy attachment failed, return a null uri");
                                            return;
                                        }
                                        Uri currentUri = mAttachment.contentUri;
                                        mAttachment.contentUri = Uri.parse(uri);
                                        viewAttachment();
                                        mAttachment.contentUri = currentUri;
                                    }
                                });
                    }
                    /// @}

                } else {
                    mActionHandler
                            .showAttachment(AttachmentDestination.EXTERNAL);
                }

                action = "attachment_bar_install";
            }
            // If we can view or play with an on-device app,
            // view or play.
            /// M: using the enhanced viewable checking function.
            else if (MimeType.isViewable(getContext(), mAttachment)) {
                mActionHandler.showAttachment(AttachmentDestination.CACHE);

                action = "attachment_bar";
            }
            // If we can only preview the attachment, preview.
            else if (mAttachment.canPreview()) {
                previewAttachment();

                action = null;
            }
            // Otherwise, if we cannot do anything, show the info dialog.
            else {
                AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
                int dialogMessage = R.string.no_application_found;
                builder.setTitle(R.string.more_info_attachment)
                       .setMessage(dialogMessage)
                       .show();

                action = "attachment_bar_no_viewer";
            }

            if (action != null) {
                Analytics.getInstance()
                        .sendEvent("view_attachment", mime, action, mAttachment.size);
            }
        }

        return true;
    }

    /**
     * M: Use for attachment's size is 0.
     */
    private void alertDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
        int dialogMessage = R.string.no_application_found;
        builder.setTitle(R.string.more_info_attachment).setMessage(dialogMessage).show();
    }

    private boolean shouldShowPreview() {
        // state could be anything
        return mAttachment.canPreview();
    }

    private boolean shouldShowSave() {
        return mAttachment.canSave() && !mSaveClicked;
    }

    private boolean shouldShowDownloadAgain() {
        // implies state == SAVED || state == FAILED
        // and the attachment supports re-download
        return mAttachment.supportsDownloadAgain() && mAttachment.isDownloadFinishedOrFailed();
    }

    private boolean shouldShowExtraOption1() {
        return !mHideExtraOptionOne &&
                mActionHandler.shouldShowExtraOption1(mAccount.getType(),
                        mAttachment.getContentType());
    }

    /**
     * M: Don't show overflow when in view only mode
     * @return
     */
    private boolean shouldShowOverflow() {
        boolean result = (!mIsViewOnlyMode)
                && ((shouldShowPreview() || shouldShowSave() || shouldShowDownloadAgain()
                        || shouldShowExtraOption1()) && !shouldShowCancel());
        LogUtils.d(LOG_TAG,
                "shouldShowOverflow [%s], detail: shouldShowPreview(%s), shouldShowSave(%s),"
                        + "shouldShowDownloadAgain(%s), shouldShowCancel(%s)", result,
                shouldShowPreview(), shouldShowSave(), shouldShowDownloadAgain(),
                shouldShowCancel());
        return result;
    }

    /**
     * M: Don't show cancel when in view only mode
     * @return
     */
    private boolean shouldShowCancel() {
        return (!mIsViewOnlyMode) && (mAttachment.isDownloading() && mSaveClicked);
    }

    @Override
    public void viewAttachment() {
        if (mAttachment.contentUri == null) {
            LogUtils.e(LOG_TAG, "viewAttachment with null content uri");
            return;
        }

        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION
                | Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);

        final String contentType = mAttachment.getContentType();

        /// M: For opening video attachment in same task as Email
        if (contentType.startsWith("video/")) {
            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        }

        /**
         * M: Before viewAttachment, in isViewable check we may manual set the InferredContentType
         *  through setInferredContentType(), the behavior will cause conversationview UI update
         *  error. here default reset the InferredContentType to null, this behavior no negative
         *  effect with the InferredContentType set not in isViewable, so just ignore.
         */
        mAttachment.setInferredContentType(null);
        Utils.setIntentDataAndTypeAndNormalize(
                intent, mAttachment.contentUri, contentType);
        /// M: add title for view App show the right file name
        intent.putExtra(INTENT_ATTACHMENT_TITLE, mAttachment.getName());

        // For EML files, we want to open our dedicated
        // viewer rather than let any activity open it.
        if (MimeType.isEmlMimeType(contentType)) {
            intent.setPackage(getContext().getPackageName());
            intent.putExtra(AccountFeedbackActivity.EXTRA_ACCOUNT_URI,
                    mAccount != null ? mAccount.uri : null);
        }

        try {
            getContext().startActivity(intent);
        } catch (ActivityNotFoundException e) {
            // couldn't find activity for View intent
            LogUtils.e(LOG_TAG, e, "Couldn't find Activity for intent");
        }
    }

    private void previewAttachment() {
        if (mAttachment.canPreview()) {
            final Intent previewIntent =
                    new Intent(Intent.ACTION_VIEW, mAttachment.previewIntentUri);
            previewIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_DOCUMENT);
            getContext().startActivity(previewIntent);

            Analytics.getInstance().sendEvent(
                    "preview_attachment", Utils.normalizeMimeType(mAttachment.getContentType()),
                    null, mAttachment.size);
        }
    }

    private static void setButtonVisible(View button, boolean visible) {
        button.setVisibility(visible ? VISIBLE : GONE);
    }

    /**
     * Update all actions based on current downloading state.
     */
    private void updateActions() {
        removeCallbacks(mUpdateRunnable);
        post(mUpdateRunnable);
    }

    private void updateActionsInternal() {
        // If the progress dialog is visible, skip any of the updating
        if (mActionHandler.isProgressDialogVisible()) {
            return;
        }

        // To avoid visibility state transition bugs, every button's visibility should be touched
        // once by this routine.
        setButtonVisible(mCancelButton, shouldShowCancel());
        setButtonVisible(mOverflowButton, shouldShowOverflow());
    }

    @Override
    public void onUpdateStatus() {
        updateSubtitleText();
    }

    @Override
    public void updateProgress(boolean showProgress) {
        if (mAttachment.isDownloading()) {
            mProgress.setMax(mAttachment.size);
            mProgress.setProgress(mAttachment.downloadedSize);
            mProgress.setIndeterminate(!showProgress);
            mProgress.setVisibility(VISIBLE);
            mSubTitle.setVisibility(INVISIBLE);
        } else {
            mProgress.setVisibility(INVISIBLE);
            mSubTitle.setVisibility(VISIBLE);
        }
    }

    private void updateSubtitleText() {
        // TODO: make this a formatted resource when we have a UX design.
        // not worth translation right now.
        final StringBuilder sb = new StringBuilder();
        if (mAttachment.state == AttachmentState.FAILED) {
            sb.append(getResources().getString(R.string.download_failed));
        } else {
            if (mAttachment.isSavedToExternal()) {
                sb.append(getResources().getString(R.string.saved, mAttachmentSizeText));
            } else {
                sb.append(mAttachmentSizeText);
            }
            if (mDisplayType != null) {
                sb.append(' ');
                sb.append(mDisplayType);
            }
        }
        mSubTitle.setText(sb.toString());
    }

    @Override
    protected void onDetachedFromWindow() {
        /// M: if the view is detached form window, also dismiss the popup menu if needed @{
        if (null != mPopup) {
            mPopup.dismiss();
        }
        /// @}
        super.onDetachedFromWindow();
    }
}
