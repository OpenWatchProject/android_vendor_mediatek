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
package com.mediatek.email.ui;

import android.content.Context;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;

import com.android.emailcommon.Configuration;
import com.android.mail.R;
import com.android.mail.compose.QuotedTextView;
import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;
import com.android.mail.utils.Utils;

/*
 * M: TruncatableQuotedTextView.
 *    truncate the quoted text showned in the webview,
 *    to avoid ANR if refmessge body is too large, e.g 500k.
 *  - The quoted message showned in UI truncated, but original will not change.
 *    This means still reply/forward whole refmessge if user not click edit quoted.
 *  - After user click edit quoted, the truncated quoted text will append to text view.
 */
class TruncatableQuotedTextView extends QuotedTextView {

    public TruncatableQuotedTextView(Context context, AttributeSet attrs) {
        super(context, attrs, -1);
    }


    /**
     * truncate the quoted text showned in the webview,
     */
    private CharSequence mTruncatedViewQuotedText;

    /**
     * truncate quoted text if length is larger than the limited length.
     * if need truncated, return truncated result.
     * if no need truncated, return null.
     * Note: make sure the html end tag is completed.
     * @param quoteText
     * @return truncated result.
     */
    private CharSequence getTruncatedText(CharSequence quoteText, int limited) {
        LogUtils.d(LogTag.getLogTag(),
                "getTruncatedText, original length [%d] , target length [%d]",
                quoteText == null ? 0 : quoteText.length(), limited);
        if (TextUtils.isEmpty(quoteText) ||
                quoteText.length() <= limited) {
            // No need truncated quote text.
            return null;
        }

        // BLOCKQUOTE_END/QUOTE_END should never removed.
        CharSequence truncatedResult = null;
        String htmlText = quoteText.toString();
        int quoteTextlength = htmlText.length();
        int quoteTextEndTagLength = BLOCKQUOTE_END.length() + QUOTE_END.length();

        if (htmlText.endsWith(BLOCKQUOTE_END + QUOTE_END)) {
            // This is a formated quoted text, with end html tag.
            StringBuilder quoteTextBuilder = new StringBuilder();
            quoteTextBuilder.append(htmlText.substring(0,
                                    quoteTextlength >= (limited + quoteTextEndTagLength) ?
                                            limited - 1 : quoteTextlength - 1));
            quoteTextBuilder.append(BLOCKQUOTE_END);
            quoteTextBuilder.append(QUOTE_END);
            truncatedResult = quoteTextBuilder.toString();
            LogUtils.d(LogTag.getLogTag(), "getTruncatedText, truncated quoted text length.");
        } else {
            truncatedResult = htmlText.substring(0, limited - 1);
        }
        return truncatedResult;
    }

    @Override
    public void onClick(View v) {
        final int id = v.getId();

        if (id == R.id.respond_inline_button) {
            // truncated quoted text.
            if (mRespondInlineListener != null && mQuotedText != null) {
                boolean sanityCheckPass = mRespondInlineListener
                        .onRespondInlineSanityCheck(mQuotedText.toString());
                if (sanityCheckPass) {
                    // no need truncated it.
                    respondInline(false);
                }
            }
        } else {
            super.onClick(v);
        }
    }

    @Override
    public void respondInline(boolean truncated) {
        /*   truncate quoted text for large message:
         * - Get truncated text form  mTruncatedViewQuotedText fistly.
         * - If not enought, default convert from original body.*/
        String plainText = "";
        if (truncated && mTruncatedViewQuotedText != null) {
            plainText = Utils.convertHtmlToPlainText(mTruncatedViewQuotedText.toString());
            LogUtils.d(LogTag.getLogTag(),
                    "+++ respondInline convertHtmlToPlainText from truncated[%d]", plainText.length());
            if (plainText.length() < Configuration.MAX_EDIT_QUOTETEXT_LENGTH) {
                plainText = Utils.convertHtmlToPlainText(getQuotedText().toString());
            }
        } else {
            plainText = Utils.convertHtmlToPlainText(getQuotedText().toString());
        }
        if (mRespondInlineListener != null) {
            if (truncated && plainText.length() > Configuration.MAX_EDIT_QUOTETEXT_LENGTH) {
                plainText = plainText.substring(0, Configuration.MAX_EDIT_QUOTETEXT_LENGTH - 1);
                LogUtils.d(LogTag.getLogTag(),
                        "+++ respondInline truncated result [%d]", plainText.length());
            }
            mRespondInlineListener.onRespondInline("\n" + plainText);
        }

        // Set quoted text to unchecked and not visible.
        updateCheckedState(false);
        mRespondInlineButton.setVisibility(View.GONE);
        // Hide everything to do with quoted text.
        View quotedTextView = findViewById(R.id.quoted_text_area);
        if (quotedTextView != null) {
            quotedTextView.setVisibility(View.GONE);
        }
    }

    @Override
    protected void populateData() {
        String fontColor = getContext().getResources().getString(
                R.string.quoted_text_font_color_string);
        String html = "<head><style type=\"text/css\">* body { color: " +
                fontColor + "; }</style></head>"
                /* for truncated message, load truncated quoted text.*/
                + (mTruncatedViewQuotedText != null ? mTruncatedViewQuotedText
                        .toString() : mQuotedText.toString());
        mQuotedTextWebView.loadDataWithBaseURL(null, html, "text/html", "utf-8", null);
    }

    @Override
    protected void setQuotedText(CharSequence quotedText) {
        mQuotedText = quotedText;
        // truacated quoted text if need.
        mTruncatedViewQuotedText = getTruncatedText(quotedText,
                Configuration.MAX_VIEW_QUOTETEXT_LENGTH);
        populateData();
        if (mRespondInlineButton != null) {
            if (!TextUtils.isEmpty(quotedText)) {
                mRespondInlineButton.setVisibility(View.VISIBLE);
                mRespondInlineButton.setEnabled(true);
                mRespondInlineButton.setOnClickListener(this);
            } else {
                // No text to copy; disable the respond inline button.
                mRespondInlineButton.setVisibility(View.GONE);
                mRespondInlineButton.setEnabled(false);
            }
        }
    }
}
