/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/**
 * Copyright (c) 2013, Google Inc.
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

package com.android.email.activity;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.FragmentManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Parcelable;
import android.text.InputFilter;
import android.text.SpannableStringBuilder;
import android.text.util.Rfc822Token;
import android.text.util.Rfc822Tokenizer;
import android.view.Menu;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.MultiAutoCompleteTextView;

import com.android.email.R;
import com.android.emailcommon.provider.EmailContent;
import com.android.emailcommon.Configuration;
import com.android.emailcommon.utility.TextUtilities;
import com.android.emailcommon.utility.Utility;
import com.android.mail.analytics.Analytics;
import com.android.mail.compose.AttachmentsView.AttachmentFailureException;
import com.android.mail.compose.ComposeActivity;
import com.android.mail.providers.Attachment;
import com.android.mail.providers.Message;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.Utils;
import com.android.mtkex.chips.MTKRecipientList;
import com.google.common.collect.Sets;

import com.mediatek.email.attachment.AttachmentHelper;
import com.mediatek.email.attachment.AttachmentTypeSelectorAdapter;
import com.mediatek.email.ui.EditQuotedConfirmDialog;
import com.mediatek.mail.ui.utils.ChipsAddressTextView;
import com.mediatek.mail.utils.DrmClientUtility;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class ComposeActivityEmail extends ComposeActivity
        implements InsertQuickResponseDialog.Callback,
        EditQuotedConfirmDialog.Callback, LoadingAttachProgressDialog.Callback {
    static final String INSERTQUICKRESPONE_DIALOG_TAG = "insertQuickResponseDialog";
    /// M: support add attachment @{
    private static final String TAG = "ComposeActivityEmail";
    /// @}

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        getMenuInflater().inflate(R.menu.email_compose_menu_extras, menu);
        /// M: reset google default menu, hide attach photo, video.@{
        resetMenu(menu);
        /// @}
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        final int id = item.getItemId();
        if (id == R.id.insert_quick_response_menu_item) {
            InsertQuickResponseDialog dialog = InsertQuickResponseDialog
                    .newInstance(null, mReplyFromAccount.account);
            dialog.show(getFragmentManager(), INSERTQUICKRESPONE_DIALOG_TAG);
        /// M: support more kinds of attachment.
        } else if (id == R.id.add_attachment) {
            onAddAttachment();
        }
        return super.onOptionsItemSelected(item);
    }

    public void onQuickResponseSelected(CharSequence quickResponse) {
        final int selEnd = mBodyView.getSelectionEnd();
        final int selStart = mBodyView.getSelectionStart();

        if (selEnd >= 0 && selStart >= 0) {
            final SpannableStringBuilder messageBody =
                    new SpannableStringBuilder(mBodyView.getText());
            final int replaceStart = selStart < selEnd ? selStart : selEnd;
            final int replaceEnd = selStart < selEnd ? selEnd : selStart;
            messageBody.replace(replaceStart, replaceEnd, quickResponse);
            mBodyView.setText(messageBody);
            /** M: MessageBody may be exceed the size limited by Body, which may cause the Outofbound exception @{ */
            int textSize = mBodyView.getText().length();
            int cursorPos = replaceStart + quickResponse.length();
            if (messageBody.length() != textSize && (cursorPos > textSize)) {
                mBodyView.setSelection(textSize);
            } else {
                mBodyView.setSelection(cursorPos);
            }
            /** @} */
        } else {
            mBodyView.append(quickResponse);
            mBodyView.setSelection(mBodyView.getText().length());
        }
    }

    @Override
    protected String getEmailProviderAuthority() {
        return EmailContent.AUTHORITY;
    }

    @Override
    protected String getEmailAttachmentProviderAuthority() {
        return EmailContent.Attachment.ATTACHMENT_PROVIDER_AUTHORITY;
    }

    /// M: support add Image/Audio/Video/Contact/Calendar/File attachments. @{

    /**
     * Since have added a more power attachments interface, hide some default entrances.
     */
    public void resetMenu(Menu menu) {
        MenuItem photoAtt = menu.findItem(R.id.add_photo_attachment);
        if (photoAtt != null) {
            photoAtt.setVisible(false);
        }

        // hide some menu, since account not ready.
        MenuItem attachmentMemu = menu
                .findItem(R.id.insert_quick_response_menu_item);
        MenuItem quickResponseMenu = menu.findItem(R.id.add_attachment);
        boolean accountReady = mAccount != null && mAccount.isAccountReady();
        LogUtils.d(TAG, "reset attachment menu and quickResponse Menu, since account is read? %s", accountReady);
        if (attachmentMemu != null) {
            attachmentMemu.setVisible(accountReady);
        }
        if (quickResponseMenu != null) {
            quickResponseMenu.setVisible(accountReady);
        }
    }

    /**
     * M: set adding attachment state.
     * @param state
     */
    public void setAddingAttachment(boolean state) {
        mAddingAttachment = state;
    }

    /**
     * M: Dialog fragment for choose type of adding attachment.
     *
     */
    public static class ChooseAttachmentTypeDialog extends DialogFragment {
        public static final String TAG = "ChooseAttachmentTypeDialog";
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Context context = getActivity();

            AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(context);
            dialogBuilder.setTitle(R.string.choose_attachment_dialog_title);
            final AttachmentTypeSelectorAdapter adapter =
                    new AttachmentTypeSelectorAdapter(context);
            dialogBuilder.setAdapter(adapter, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    ((ComposeActivityEmail) getActivity()).setAddingAttachment(true);
                    AttachmentHelper.addAttachment(adapter.buttonToCommand(which), context);
                    dialog.dismiss();
                }
            });
            return dialogBuilder.create();
        }
    }

    /**
     * M: Kick off a dialog to choose types of attachments: image, music and video.
     */
    private void onAddAttachment() {
        FragmentManager fm = getFragmentManager();
        if (fm.findFragmentByTag(ChooseAttachmentTypeDialog.TAG) == null) {
            fm.beginTransaction()
            .add(new ChooseAttachmentTypeDialog(), ChooseAttachmentTypeDialog.TAG)
            .commitAllowingStateLoss();
            fm.executePendingTransactions();
        }
    }

    @Override
    protected final void onActivityResult(int request, int result, Intent data) {
        // let supper handle it firstly, make sure the request is unique.
        super.onActivityResult(request, result, data);
        LogUtils.e(TAG, "ComposeActivityEmail onActivityResult [%d] data: %s ", request,
                (data != null) ? data.toString() : "NULL");

        if (data == null) {
            return;
        }
        ///M:after add attachment, we just hide the IME in order to pass CMCC AUTO TEST
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);

        switch (request) {
        case AttachmentHelper.REQUEST_CODE_ATTACH_IMAGE:
            addAttachmentAndUpdateView(data);
            break;
        case AttachmentHelper.REQUEST_CODE_ATTACH_VIDEO:
            addAttachmentAndUpdateView(data.getData());
            break;
        case AttachmentHelper.REQUEST_CODE_ATTACH_SOUND:
            addAttachmentAndUpdateView(data.getData());
            break;
        case AttachmentHelper.REQUEST_CODE_ATTACH_CONTACT:
            Bundle extras = data.getExtras();
            if (extras != null) {
                Uri uri = (Uri) extras.get(AttachmentHelper.ITEXTRA_CONTACTS);
                if (uri != null) {
                    addAttachmentAndUpdateView(uri);
                }
            } else {
                LogUtils.e(TAG,
                        "Can not get extras data from the attaching contact");
            }
            break;
        case AttachmentHelper.REQUEST_CODE_ATTACH_CALENDAR:
            // handle calendar
            addAttachmentAndUpdateView(data.getData());
            break;
        case AttachmentHelper.REQUEST_CODE_ATTACH_FILE:
            addAttachmentAndUpdateView(data.getData());
            break;
        default:
            LogUtils.i(TAG, "Can not handle the requestCode [%s] in onActivityResult method", request);
        }
        mAddingAttachment = false;
    }
    /// Attachment enhancement @}

    /**
     * M: Override default to filter DRM protected attachment.
     * This is the entrance of forward attachment.
     */
    @Override
    protected void initAttachments(Message refMessage) {
        // M: filter DRM attachment.
        addAttachments(DrmClientUtility.filterDrmAttachments(
                this, refMessage.getAttachments()), false);
    }

    /**
     * M: Override default to add attachment in background, not block UI thread.
     * This is the entrance of choose one attachment invoked from Email.
     */
    @Override
    public void addAttachmentAndUpdateView(Uri contentUri) {
        LogUtils.d(TAG, "addAttachmentAndUpdateView uri: %s",
                (contentUri != null) ? (contentUri.toSafeString()) : "NULL");
        if (contentUri == null) {
            return;
        }
        ArrayList<Uri> uriArray = new ArrayList<Uri>();
        uriArray.add(contentUri);
        loadAttachmentsInBackground(uriArray);
    }

    /**
     * M: Override default to load attachment in background, not block UI thread.
     * This is the entrance of Share/Send attachment from other Apps.
     */
    @Override
    protected void initAttachmentsFromIntent(Intent intent) {
        Bundle extras = intent.getExtras();
        if (extras == null) {
            extras = Bundle.EMPTY;
        }
        final String action = intent.getAction();
        ArrayList<Uri> uriArray = new ArrayList<Uri>();
        if (!mAttachmentsChanged) {
            if (extras.containsKey(EXTRA_ATTACHMENTS)) {
                String[] uris = (String[]) extras.getSerializable(EXTRA_ATTACHMENTS);
                for (String uriString : uris) {
                    final Uri uri = Uri.parse(uriString);
                    uriArray.add(uri);
                }
            }
            if (extras.containsKey(Intent.EXTRA_STREAM)) {
                if (Intent.ACTION_SEND_MULTIPLE.equals(action)) {
                    ArrayList<Parcelable> uris = extras
                            .getParcelableArrayList(Intent.EXTRA_STREAM);
                    for (Parcelable uri : uris) {
                        Uri newUri = (Uri) uri;
                        uriArray.add(newUri);
                    }
                } else {
                    final Uri uri = (Uri) extras.getParcelable(Intent.EXTRA_STREAM);
                    if (uri != null) {
                        uriArray.add(uri);
                    }
                }
            }

            loadAttachmentsInBackground(uriArray);
        }
    }

    /**
     *  M: MTK load attachment in background EmailAsyncTask, and show Load
     *  Attachment ProgressDialog when attachments is loading. avoid user make
     *  UI operation cause ANR.
     */
    private void loadAttachmentsInBackground(final ArrayList<Uri> uris) {
        LogUtils.d(TAG, "loadAttachmentsInBackground: %s", uris.toString());
        if (uris.size() > 0) {
            // / M: if the activity is destroyed or null, do nothing.
            FragmentManager fm = getFragmentManager();
            if (fm.findFragmentByTag(LoadingAttachProgressDialog.TAG) == null) {
                LoadingAttachProgressDialog progressDialog = LoadingAttachProgressDialog
                        .newInstance(null, uris);
                fm.beginTransaction().add(progressDialog, LoadingAttachProgressDialog.TAG)
                        .commitAllowingStateLoss();
                fm.executePendingTransactions();
            }
        }
    }

    @Override
    public void onAttachmentsLoadingComplete(AttachmentFailureException exception,
            List<Attachment> attachments) {
        if (exception != null) {
            showErrorToast(getString(exception.getErrorRes()));
        }
        LoadingAttachProgressDialog progressDialog = (LoadingAttachProgressDialog)
                getFragmentManager().findFragmentByTag(LoadingAttachProgressDialog.TAG);
        if (progressDialog != null) {
            progressDialog.dismissAllowingStateLoss();
        }
        if (null == attachments || attachments.size() == 0) {
            return;
        }
        LogUtils.d(TAG, "onAttachmentsLoadingComplete: attachments count %s", attachments.size());
        // Add attachments list to UI
        long addedAttachments = addAttachments(attachments, true);
        int totalSize = getAttachments().size();
        if (totalSize > 0) {
            // /M: if no attachments has been added , keep mAttachmentsChanged from modification
            if (addedAttachments > 0) {
                mAttachmentsChanged = true;
            }
            updateSaveUi();

            Analytics.getInstance().sendEvent("send_intent_with_attachments",
                    Integer.toString(totalSize), null, totalSize);
        }
    }

    /**
     * M: Override and filter email address.
     */
    @Override
    protected void setupRecipients(ChipsAddressTextView view) {
        super.setupRecipients(view);
        InputFilter[] recipientFilters = new InputFilter[] {
                ChipsAddressTextView.RECIPIENT_FILTER };
        view.setFilters(recipientFilters);
        // set filter delayer to avoid too many Filter thread be created.
        ((ChipsAddressTextView) view).setGalSearchDelayer();
    }

    /**
     * M: For reply, replyall and edit draft safely add CcAddresses to cc view at one time.
     * and the limit max number for add in one time is 250 to avoid too many to cause ANR.
     */
    @Override
    protected void addCcAddressesToList(List<Rfc822Token[]> addresses,
            List<Rfc822Token[]> compareToList, MultiAutoCompleteTextView list) {
        Set<String> tokenAddresses = Sets.newHashSet();

        if (compareToList == null) {
            for (Rfc822Token[] tokens : addresses) {
                for (int i = 0; i < tokens.length; i++) {
                    if (!tokenAddresses.contains(tokens[i].toString())) {
                        tokenAddresses.add(tokens[i].toString());
                    }
                }
            }
        } else {
            HashSet<String> compareTo = convertToHashSet(compareToList);
            for (Rfc822Token[] tokens : addresses) {
                for (int i = 0; i < tokens.length; i++) {
                    // Check if this is a duplicate:
                    if (!compareTo.contains(tokens[i].getAddress())
                            && !tokenAddresses.contains(tokens[i].toString())) {
                        tokenAddresses.add(tokens[i].toString());
                    }
                }
            }
        }
        safeAddAddressesToView(tokenAddresses, list);
    }

    /**
     * M: For reply, replyall and edit draft safely add ToAddresses to To view at one time.
     * and the limit max number for add in one time is 250 to avoid too many to cause ANR.
     */
    @Override
    protected void addAddressesToList(Collection<String> addresses,
            MultiAutoCompleteTextView list) {
        Set<String> tokenAddresses = Sets.newHashSet();
        for (String address : addresses) {
            if (address == null || list == null) {
                return;
            }

            final Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(address);
            for (int i = 0; i < tokens.length; i++) {
                if (!tokenAddresses.contains(tokens[i].toString())) {
                    tokenAddresses.add(tokens[i].toString());
                }
            }
        }
        safeAddAddressesToView(tokenAddresses, list);
    }

    /**
     * M: add numbers of address to RecipientView and avoid can not get the add text by appendList.
     */
    private void safeAddAddressesToView(Collection<String> addresses,
            MultiAutoCompleteTextView list) {
        final MTKRecipientList recipientList = new MTKRecipientList();
        for (String address : addresses) {
            if (recipientList.getRecipientCount() >= Configuration.RECIPIENT_MAX_NUMBER) {
                Utility.showToast(this,
                        getString(R.string.not_add_more_recipients, Configuration.RECIPIENT_MAX_NUMBER));
                LogUtils.d(TAG, "Not add more recipient, added address length is: %d", addresses.size());
                break;
            }
            recipientList.addRecipient("", address);
        }
        ((ChipsAddressTextView) list).appendList(recipientList);
    }

    /**
     * M: support truncated quoted text.
     */
    @Override
    public boolean onRespondInlineSanityCheck(String text) {
        final String plainText = Utils.convertHtmlToPlainText(text);
        if (plainText.length() > Configuration.MAX_EDIT_QUOTETEXT_LENGTH) {
            FragmentManager fm = getFragmentManager();
            if (fm.findFragmentByTag(EditQuotedConfirmDialog.TAG) == null) {
                fm.beginTransaction()
                  .add(EditQuotedConfirmDialog.newInstance(),
                       EditQuotedConfirmDialog.TAG)
                  .commitAllowingStateLoss();
            }
            return false;
        } else {
            // SanityCheck pass, QuotedTextView will handle next action.
            return true;
        }
    }

    /**
     * M: call back from confim dialog of respondInline.
     */
    @Override
    public void onConfimRespondInline() {
        if (mQuotedTextView != null) {
            mQuotedTextView.respondInline(true);
        }
    }

    /**
     * M: Init edit view's length filter
     */
    @Override
    protected void initLenghtFilter() {
        super.initLenghtFilter();
        TextUtilities.setupLengthFilter(getSubjectEditText(), this,
                Configuration.EDITVIEW_MAX_LENGTH_1, getString(R.string.not_add_more_text));
        TextUtilities.setupLengthFilter(getBodyEditText(), this,
                Configuration.EDITVIEW_MAX_LENGTH_2, getString(R.string.not_add_more_text));
    }

}
