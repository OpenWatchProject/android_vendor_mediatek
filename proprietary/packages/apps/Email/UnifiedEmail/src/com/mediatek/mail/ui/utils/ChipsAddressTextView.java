/*
 * Copyright (C) 2011 The Android Open Source Project
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

package com.mediatek.mail.ui.utils;

import android.content.Context;
import android.content.res.Configuration;
import android.text.InputFilter;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.util.AttributeSet;
import android.widget.Filter;

import com.android.mtkex.chips.MTKRecipientEditTextView;

/**
 * This is a MultiAutoCompleteTextView which has a custom validator.
 */
public class ChipsAddressTextView extends MTKRecipientEditTextView {
    /**
     *  Set the search address threshold value as 1
     *  The default threshold length is 2.
     */
    public static final int AUTO_SEARCH_THRESHOLD_LENGTH = 1;
    private static final long DELETE_KEY_POST_DELAY = 500L;
    private static final long ADD_POST_DELAY = 300L;

    /**
     * M: An {@link InputFilter} that implements special address cleanup rules.
     * The first space key entry following an "@" symbol that is followed by any combination
     * of letters and symbols, including one+ dots and zero commas, should insert an extra
     * comma (followed by the space).
     */
    public static final InputFilter RECIPIENT_FILTER = new InputFilter() {
        @Override
        public CharSequence filter(CharSequence source, int start, int end, Spanned dest,
                int dstart, int dend) {

            // Quick check - did they enter a single space?
            if (end - start != 1 || source.charAt(start) != ' ') {
                return null;
            }

            // determine if the characters before the new space fit the pattern
            // follow backwards and see if we find a comma, dot, or @
            int scanBack = dstart;
            boolean dotFound = false;
            while (scanBack > 0) {
                char c = dest.charAt(--scanBack);
                switch (c) {
                    case '.':
                        dotFound = true;    // one or more dots are req'd
                        break;
                    case ',':
                        return null;
                    case '@':
                        if (!dotFound) {
                            return null;
                        }
                        // we have found a comma-insert case.  now just do it
                        // in the least expensive way we can.
                        if (source instanceof Spanned) {
                            SpannableStringBuilder sb = new SpannableStringBuilder(",");
                            sb.append(source);
                            return sb;
                        } else {
                            return ", ";
                        }
                    default:
                        // just keep going
                }
            }
            // no termination cases were found, so don't edit the input
            return null;
        }
    };

    /** A noop validator that does not munge invalid texts. */
    private static class ForwardValidator implements Validator {
        private Validator mValidator = null;

        public CharSequence fixText(CharSequence invalidText) {
            //M: use the mValidator fixText to fix the text, such as append the domain
            return mValidator != null ? mValidator.fixText(invalidText) : invalidText;
        }

        public boolean isValid(CharSequence text) {
            return mValidator != null ? mValidator.isValid(text) : true;
        }

        public void setValidator(Validator validator) {
            mValidator = validator;
        }
    }

    private final ForwardValidator mInternalValidator = new ForwardValidator();

    public ChipsAddressTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        super.setValidator(mInternalValidator);
        //set search address threshold length as 1
        setThreshold(AUTO_SEARCH_THRESHOLD_LENGTH);
    }

    @Override
    public void setValidator(Validator validator) {
        mInternalValidator.setValidator(validator);
    }

    public void setGalSearchDelayer() {
        Filter filter = getFilter();
        /** M: MTK Dependence @{ */
        if (filter != null) {
            filter.setDelayer(new Filter.Delayer() {

                private int mPreviousLength = 0;

                public long getPostingDelay(CharSequence constraint) {
                    if (constraint == null) {
                        return 0;
                    }

                    long delay = constraint.length() < mPreviousLength
                            ? DELETE_KEY_POST_DELAY : ADD_POST_DELAY;
                    mPreviousLength = constraint.length();
                    return delay;
                }
            });
        }
        /** @} */
    }

    /**
     * M: Clear recipientsView's popup view.
     * @param newConfig
     */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (isFocused() && isPopupShowing()) {
            dismissDropDown();
        }
    }
}
