/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.mms.ui;

import android.content.Context;
import android.provider.Telephony.Mms;
import android.telephony.PhoneNumberUtils;
import android.text.Annotation;
import android.text.Editable;
import android.text.InputType;
import android.text.Layout;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.TextWatcher;
import android.text.util.Rfc822Token;
import android.text.util.Rfc822Tokenizer;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Patterns;
import android.view.MotionEvent;
import android.view.View;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.widget.AdapterView;
import android.widget.Filter;
import android.widget.MultiAutoCompleteTextView;

import com.android.mms.MmsConfig;
import com.android.mms.data.Contact;
import com.android.mms.data.ContactList;
import com.android.mms.R;
import com.android.mms.util.MmsLog;
import com.android.mtkex.chips.MTKRecipient;
import com.android.mtkex.chips.MTKRecipientEditTextView;
import com.android.mtkex.chips.MTKRecipientList;
import com.android.mtkex.chips.RecipientEntry;

import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.callback.IRecipientsEditorCallback;
import com.mediatek.mms.ext.IOpRecipientsEditorExt;
import com.mediatek.mms.ipmessage.IIpRecipientsEditorExt;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;

/**
 * Provide UI for editing the recipients of multi-media messages.
 */
public class RecipientsEditor extends MTKRecipientEditTextView
        implements IRecipientsEditorCallback {
    private int mLongPressedPosition = -1;
    private final RecipientsEditorTokenizer mTokenizer;
    private char mLastSeparator = ',';
    private Runnable mOnSelectChipRunnable;
    private final AddressValidator mInternalValidator;
    private boolean mIsPointInChip = false;

    private IOpRecipientsEditorExt mOpRecipientsEditorExt;
    private IIpRecipientsEditorExt mIpRecipientsEditorExt;

    private static final long DELETE_KEY_POST_DELAY = 500L;
    private static final long ADD_POST_DELAY = 300L;
    /// M:
    private static final String TAG = "Mms/RecipientsEditor";

        /** A noop validator that does not munge invalid texts and claims any address is valid */
    private class AddressValidator implements Validator {
        public CharSequence fixText(CharSequence invalidText) {
            return invalidText;
        }

        public boolean isValid(CharSequence text) {
            return true;
        }
    }

    public void setGalSearchDelayer() {
        Filter filter = getFilter();
        if (filter != null) {
            MmsLog.i(TAG, "setGalSearchDelayer");
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
    }

    public RecipientsEditor(Context context, AttributeSet attrs) {
        super(context, attrs);
 /// M:       setDropDownWidth(LayoutParams.MATCH_PARENT);
        mTokenizer = new RecipientsEditorTokenizer();
        setTokenizer(mTokenizer);

        mInternalValidator = new AddressValidator();
        super.setValidator(mInternalValidator);

        // For the focus to move to the message body when soft Next is pressed
        setImeOptions(EditorInfo.IME_ACTION_NEXT);

        setThreshold(1);    // pop-up the list after a single char is typed

        /// M:
        setInputType(getInputType() | InputType.TYPE_TEXT_FLAG_MULTI_LINE);

        mOpRecipientsEditorExt = OpMessageUtils.getOpMessagePlugin()
                .getOpRecipientsEditorExt();
        mIpRecipientsEditorExt = IpMessageUtils.getIpMessagePlugin(context)
                .getIpRecipientsEditorExt();
        if (!getResources().getBoolean(R.bool.isWVGAScreen)) {
            setMaxLines(3);
        } else {
            setMaxLines(1);
        }
        mOpRecipientsEditorExt.init(this, getResources().getBoolean(R.bool.isWVGAScreen));

        /*
         * The point of this TextWatcher is that when the user chooses
         * an address completion from the AutoCompleteTextView menu, it
         * is marked up with Annotation objects to tie it back to the
         * address book entry that it came from.  If the user then goes
         * back and edits that part of the text, it no longer corresponds
         * to that address book entry and needs to have the Annotations
         * claiming that it does removed.
         */
        addTextChangedListener(new TextWatcher() {
            private Annotation[] mAffected;

            @Override
            public void beforeTextChanged(CharSequence s, int start,
                    int count, int after) {
                mAffected = ((Spanned) s).getSpans(start, start + count,
                        Annotation.class);
            }

            @Override
            public void onTextChanged(CharSequence s, int start,
                    int before, int after) {
                if (before == 0 && after == 1) {    // inserting a character
                    char c = s.charAt(start);
                    if (c == ',' || c == ';') {
                        // Remember the delimiter the user typed to end this recipient. We'll
                        // need it shortly in terminateToken().
                        mLastSeparator = c;
                    }
                }
            }

            @Override
            public void afterTextChanged(Editable s) {
                if (mAffected != null) {
                    for (Annotation a : mAffected) {
                        s.removeSpan(a);
                    }
                }
                mAffected = null;
            }
        });
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        super.onItemClick(parent, view, position, id);

        if (mOnSelectChipRunnable != null) {
            mOnSelectChipRunnable.run();
        }
    }

    public void setOnSelectChipRunnable(Runnable onSelectChipRunnable) {
        mOnSelectChipRunnable = onSelectChipRunnable;
    }

//    @Override
//    public boolean enoughToFilter() {
//        if (!super.enoughToFilter()) {
//            return false;
//        }
//        // If the user is in the middle of editing an existing recipient, don't offer the
//        // auto-complete menu. Without this, when the user selects an auto-complete menu item,
//        // it will get added to the list of recipients so we end up with the old before-editing
//        // recipient and the new post-editing recipient. As a precedent, gmail does not show
//        // the auto-complete menu when editing an existing recipient.
//        int end = getSelectionEnd();
//        int len = getText().length();
//
//        return end == len;
//
//    }

    public int getRecipientCount() {
        return mTokenizer.getNumbers().size();
    }

    public List<String> getNumbers() {
        return mTokenizer.getNumbers();
    }

    public ContactList constructContactsFromInput(boolean blocking) {
        List<String> numbers = mTokenizer.getNumbers();
        ContactList list = new ContactList();
        for (String number : numbers) {
            Contact contact = new Contact(number);
            list.add(contact);
        }
        return list;
    }

    private boolean isValidAddress(String number, boolean isMms) {
        if (isMms) {
            return MessageUtils.isValidMmsAddress(number);
        } else {
            // TODO: PhoneNumberUtils.isWellFormedSmsAddress() only check if the number is a valid
            // GSM SMS address. If the address contains a dialable char, it considers it a well
            // formed SMS addr. CDMA doesn't work that way and has a different parser for SMS
            // address (see CdmaSmsAddress.parse(String address)). We should definitely fix this!!!
            boolean commonValidValue =
                MessageUtils.isWellFormedSmsAddress(number.replaceAll(" |-", ""))
                || Mms.isEmailAddress(number);

            if (mOpRecipientsEditorExt.isValidAddress(commonValidValue, number)) {
                return false;
            }
            if (mIpRecipientsEditorExt.isValidAddress(commonValidValue, number)) {
                return true;
            }
            return commonValidValue;
        }
    }

    public boolean hasValidRecipient(boolean isMms) {
        for (String number : mTokenizer.getNumbers()) {
            if (isValidAddress(number, isMms))
                return true;
        }
        return false;
    }

    public boolean hasInvalidRecipient(boolean isMms) {
        for (String number : mTokenizer.getNumbers()) {
            if (!isValidAddress(number, isMms)) {
                if (MmsConfig.getEmailGateway() == null) {
                    return true;
                } else if (!MessageUtils.isAlias(number)) {
                    return true;
                }
            }
        }
        return false;
    }

    public String formatInvalidNumbers(boolean isMms) {
        StringBuilder sb = new StringBuilder();
        for (String number : mTokenizer.getNumbers()) {
            if (!isValidAddress(number, isMms)) {
                if (sb.length() != 0) {
                    sb.append(", ");
                }
                sb.append(number);
            }
        }
        return sb.toString();
    }

    public boolean containsEmail() {
        if (TextUtils.indexOf(getText(), '@') == -1)
            return false;

        List<String> numbers = mTokenizer.getNumbers();
        for (String number : numbers) {
            if (Mms.isEmailAddress(number))
                return true;
        }
        return false;
    }

    /*public static CharSequence contactToToken(Contact c) {
        SpannableString s = new SpannableString(c.getNameAndNumber());
        int len = s.length();

        if (len == 0) {
            return s;
        }

        s.setSpan(new Annotation("number", c.getNumber()), 0, len,
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        return s;
    }*/

    public void populate(ContactList list) {
        // Very tricky bug. In the recipient editor, we always leave a trailing
        // comma to make it easy for users to add additional recipients. When a
        // user types (or chooses from the dropdown) a new contact Mms has never
        // seen before, the contact gets the correct trailing comma. But when the
        // contact gets added to the mms's contacts table, contacts sends out an
        // onUpdate to CMA. CMA would recompute the recipients and since the
        // recipient editor was still visible, call mRecipientsEditor.populate(recipients).
        // This would replace the recipient that had a comma with a recipient
        // without a comma. When a user manually added a new comma to add another
        // recipient, this would eliminate the span inside the text. The span contains the
        // number part of "Fred Flinstone <123-1231>". Hence, the whole
        // "Fred Flinstone <123-1231>" would be considered the number of
        // the first recipient and get entered into the canonical_addresses table.
        // The fix for this particular problem is very easy. All recipients have commas.
        // TODO: However, the root problem remains. If a user enters the recipients editor
        // and deletes chars into an address chosen from the suggestions, it'll cause
        // the number annotation to get deleted and the whole address (name + number) will
        // be used as the number.
        MmsLog.i(TAG, "RecipientsEditor:populate, list.size = " + list.size());
        if (list.size() == 0) {
            // The base class RecipientEditTextView will ignore empty text. That's why we need
            // this special case.
            setText(null);
        } else {
            /// M: redesign append mechanism. @{
            MTKRecipientList recipientList = new MTKRecipientList();
            for (Contact c : list) {
                // Calling setText to set the recipients won't create chips,
                // but calling append() will.
                String displayName = c.getName();
                if (!TextUtils.isEmpty(displayName) && displayName.equals(c.getNumber())) {
                    displayName = "";
                }
                MTKRecipient recipient = new MTKRecipient(displayName, c.getNumberProtosomatic());
                recipientList.addRecipient(recipient);
            }
            appendList(recipientList);
            /// @}
        }
    }

    private int pointToPosition(int x, int y) {
        Layout layout = getLayout();
        if (layout == null) {
            return -1;
        }
        x -= getCompoundPaddingLeft();
        y -= getExtendedPaddingTop();

        x += getScrollX();
        y += getScrollY();

        int line = layout.getLineForVertical(y);
        int off = layout.getOffsetForHorizontal(line, x);

        return off;
    }

    private boolean mIsTouchable = true;
    public void setIsTouchable(boolean isTouchable) {
        mIsTouchable = isTouchable;
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        final int action = ev.getAction();
        final int x = (int) ev.getX();
        final int y = (int) ev.getY();
        /// M: remember the valid position, for updating the chip in recipient control later. @{
        mIsPointInChip = isTouchPointInChip(x, y);
        if (mIsPointInChip) {
            mChipX = (float) ev.getX();
            mChipY = (float) ev.getY();
        }
        /// @}
        if (action == MotionEvent.ACTION_DOWN) {
            mLongPressedPosition = pointToPosition(x, y);
        }
        if (mIsTouchable) {
            return super.onTouchEvent(ev);
        } else {
            return true;
        }
    }

    @Override
    protected ContextMenuInfo getContextMenuInfo() {
        if ((mLongPressedPosition >= 0)) {
            Spanned text = getText();
            if (mLongPressedPosition <= text.length() && mIsPointInChip) {
                /// M: get correct token (end with ", ") while long press a chip.
                /// i.e. if text is "123456, 891-7823, ", the second token is "891-7823, ". @{
                int start = mTokenizer.findTokenStart(text, mLongPressedPosition);
                int end = Math.min(mTokenizer.findTokenEnd(text, start) + 1, text.length());
                boolean isPressedContact = false;
                if (end != start &&
                        (mLongPressedPosition <= end && mLongPressedPosition >= start)) {
                    isPressedContact = true;
                } else if (mLongPressedPosition < start || mLongPressedPosition > end) {
                    end = Math.min(mTokenizer.findTokenEnd(text,
                            mLongPressedPosition) + 1, text.length());
                    start = mTokenizer.findTokenStart(text, end);
                    if (end != start && (mLongPressedPosition <= end
                            && mLongPressedPosition >= start)) {
                        isPressedContact = true;
                    }
                }
                /// @}
                if (isPressedContact) {
                    String number = getNumberAt(getText(), start, end, getContext());
                    /// M: Recipient Control refactory, get contact info from framework. @{
                    RecipientEntry recipient = getRecipientEntry(mChipX, mChipY);
                    Contact c = new Contact(number);
                    if (recipient != null) {
                        String name = recipient.getDisplayName().toString();
                        String reNumber = recipient.getDestination();
                        reNumber = PhoneNumberUtils.replaceUnicodeDigits(reNumber);
                        long personId = recipient.getContactId();
                        if ((personId < 0 &&
                                name != null && name.equals(reNumber)) || personId == -2) {
                            name = "";
                        }
                        byte[] data = recipient.getPhotoBytes();
                        c.setContactInfoFormChipWatcher(name, reNumber, personId, data);
                    } else {
                        c = Contact.get(number, true);
                    }
                    /// @}
                    setEnableDiscardNextActionUp(true);
                    return new RecipientContextMenuInfo(c);
                }
            }
        }
        return null;
    }

    private static String getNumberAt(Spanned sp, int start, int end, Context context) {
        String number = getFieldAt("number", sp, start, end, context);
        number = PhoneNumberUtils.replaceUnicodeDigits(number);
        if (!TextUtils.isEmpty(number)) {
            int pos = number.lastIndexOf('<');
            if ((pos >= 0 && pos < number.lastIndexOf('>'))
                    || number.endsWith(",") || number.endsWith(";")) {
                if (isPhoneNumber(number)) {
                    number = number.replaceAll("[()]", "");
                }
                // The number looks like an Rfc882 address, i.e. <fred flinstone> 891-7823
                Rfc822Token[] tokens = Rfc822Tokenizer.tokenize(number);
                if (tokens.length == 0) {
                    return number;
                }
                return tokens[0].getAddress();
            }
        }
        return number;
    }

    private static int getSpanLength(Spanned sp, int start, int end, Context context) {
        // TODO: there's a situation where the span can lose its annotations:
        //   - add an auto-complete contact
        //   - add another auto-complete contact
        //   - delete that second contact and keep deleting into the first
        //   - we lose the annotation and can no longer get the span.
        // Need to fix this case because it breaks auto-complete contacts with commas in the name.
        Annotation[] a = sp.getSpans(start, end, Annotation.class);
        if (a.length > 0) {
            return sp.getSpanEnd(a[0]);
        }
        return 0;
    }

    private static String getFieldAt(String field, Spanned sp, int start, int end,
            Context context) {
        Annotation[] a = sp.getSpans(start, end, Annotation.class);
        String fieldValue = getAnnotation(a, field);
        if (TextUtils.isEmpty(fieldValue)) {
            fieldValue = TextUtils.substring(sp, start, end);
            /** M: just return the substring is not so good,
             * its format is probably like this: lily <1234567>
             * as the comment in populate, there is some cases
             * that user can deliminate the annotation.
             * when this happened, we come here.
             * and the old strategy is return the whole string as the phone number
             * but the while string is as lily <1234567>
             * it is wrong. currently we can not find out all cases that deliminate the annotation.
             * but we can make a little more, try to filter out the right number and return it.
             * it's better than just return.
             */
            int lIndex = TextUtils.lastIndexOf(fieldValue, '<');
            if (lIndex >= 0) {
                int rIndex = TextUtils.lastIndexOf(fieldValue, '>');
                if (lIndex < rIndex) {
                    fieldValue = TextUtils.substring(fieldValue, lIndex + 1, rIndex);
                    //MmsLog.d(TAG, "annotation missing! filter right number:"+fieldValue);
                }
            }
        }
        return fieldValue;

    }

    private static String getAnnotation(Annotation[] a, String key) {
        for (int i = 0; i < a.length; i++) {
            if (a[i].getKey().equals(key)) {
                return a[i].getValue();
            }
        }

        return "";
    }

    private class RecipientsEditorTokenizer
            implements MultiAutoCompleteTextView.Tokenizer {

        public int findTokenStart(CharSequence text, int cursor) {
            int i = cursor > 0 ? cursor - 1 : cursor;
            char c;

            // If we're sitting at a delimiter, back up so we find the previous token
            if (i > 0 && ((c = text.charAt(i - 1)) == ',' || c == ';')) {
                --i;
            }
            // Now back up until the start or until we find the separator of the previous token
//            while (i > 0 && (c = text.charAt(i - 1)) != ',' && c != ';') {
//                i--;
//            }
            while (i > 0) {
                if ((c = text.charAt(i - 1)) != ',' && c != ';') {
                    if (c == '>') {
                        i--;
                        while (i > 0) {
                            c = text.charAt(i);
                            if (c == '<') {
                                i--;
                                break;
                            } else {
                                i--;
                            }
                        }
                    } else if (c == '"') {
                        i--;
                        while (i > 0) {
                            c = text.charAt(i);
                            if (c == '"') {
                                i--;
                                break;
                            } else {
                                i--;
                            }
                        }
                    } else {
                        i--;
                    }
                } else {
                    break;
                }
            }

            while (i < cursor && ((c = text.charAt(i)) == ' ' || c == ',' || c == ';')) {
                i++;
            }

            return i;
        }

        /**
         * Returns the end of the token (minus trailing punctuation)
         * that begins at offset <code>cursor</code> within <code>text</code>.
         * It is a method from the MultiAutoCompleteTextView.Tokenizer interface.
         */
        public int findTokenEnd(CharSequence text, int cursor) {
            int i = cursor;
            int len = text.length();
            char c;

            while (i >= 0 && i < len) {
                if ((c = text.charAt(i)) == ',' || c == ';') {
                    return i;
                    /// M: fix the bug if name/number contain comma. @
                } else if (c == '"') {
                    i++;
                    while (i < len) {
                        c = text.charAt(i);
                        if (c == '"') {
                            i++;
                            break;
                        } else {
                            i++;
                        }
                    }
                } else if (c == '<') {
                    i++;
                    while (i < len) {
                        c = text.charAt(i);
                        if (c == '>') {
                            i++;
                            break;
                        } else {
                            i++;
                        }
                    }
                    /// @}
                } else {
                    i++;
                }
            }

            return len;
        }

        /**
         * Returns <code>text</code>, modified, if necessary, to ensure that
         * it ends with a token terminator (for example a space or comma).
         * It is a method from the MultiAutoCompleteTextView.Tokenizer interface.
         */
        public CharSequence terminateToken(CharSequence text) {
            int i = text.length();

            while (i > 0 && text.charAt(i - 1) == ' ') {
                i--;
            }

            char c;
            if (i > 0 && ((c = text.charAt(i - 1)) == ',' || c == ';')) {
                return text;
            } else {
                // Use the same delimiter the user just typed.
                // This lets them have a mixture of commas and semicolons in their list.
                String separator = mLastSeparator + " ";
                if (text instanceof Spanned) {
                    SpannableString sp = new SpannableString(text + separator);
                    TextUtils.copySpansFrom((Spanned) text, 0, text.length(),
                                            Object.class, sp, 0);
                    return sp;
                } else {
                    return text + separator;
                }
            }
        }

        public List<String> getNumbers() {
            Spanned sp = RecipientsEditor.this.getText();
            int len = sp.length();
            List<String> list = new ArrayList<String>();

            int start = 0;
            int i = 0;
            while (i < len + 1) {
                char c;
                if ((i == len) || ((c = sp.charAt(i)) == ',') || (c == ';')) {
                    if (i > start) {
                        list.add(getNumberAt(sp, start, i, getContext()));

                        // calculate the recipients total length. This is so if the name contains
                        // commas or semis, we'll skip over the whole name to the next
                        // recipient, rather than parsing this single name into multiple
                        // recipients.
                        int spanLen = getSpanLength(sp, start, i, getContext());
                        if (spanLen > i) {
                            i = spanLen;
                        }
                    }

                    i++;

                    while ((i < len) && (sp.charAt(i) == ' ')) {
                        i++;
                    }

                    start = i;
                /// M: fix the bug if name/number contain comma. @
                } else if (c == '"') {
                    i++;
                    while (i < len) {
                        c = sp.charAt(i);
                        if (c == '"') {
                            i++;
                            break;
                        } else {
                            i++;
                        }
                    }
                } else if (c == '<') {
                    i++;
                    while (i < len) {
                        c = sp.charAt(i);
                        if (c == '>') {
                            i++;
                            break;
                        } else {
                            i++;
                        }
                    }
                /// @}
                } else {
                    i++;
                }
            }

            return list;
        }
    }

    static class RecipientContextMenuInfo implements ContextMenuInfo {
        final Contact recipient;

        RecipientContextMenuInfo(Contact r) {
            recipient = r;
        }
    }

    /// M:
    @Override
    public void onLongPress(MotionEvent event) {
    }

    /** M: this method is added for ComposeMessageActivity's updateTitle
     *  because the constructContactsFromInput method is time cost when contact number is 100.
     *  updateTitle don't need all really contact to show,
     *  only first limit(now use 20 on landscape/10 on portrait) is enough.
     */
    /*public ContactList constructContactsFromInputWithLimit(boolean blocking, int limit) {
        MmsLog.d(TAG, "begin constructContactsFromInputWithLimit");
        List<String> numbers = mTokenizer.getNumbers();
        mLastContact = null;
        /// M: this method is used to update title, if the last number in recipients editor is
        /// end without "," or ";", do not show this number on title. @{
        Spanned text = getText();
        int spaceCount = 0;
        int pos = 0;
        for (pos = text.length() - 1; pos >= 0; pos--) {
            char c = text.charAt(pos);
            if (c == ' ') {
                spaceCount++;
                continue;
            }
            if ((c == ',') || (c == ';')) {
                break;
            }
        }
        if (text.length() > (pos + spaceCount + 1)) {
            if (numbers.size() > 0) {
                String removedNumber = numbers.remove(numbers.size()-1);
                if (numbers.size() < 10) {
                    Contact contact = Contact.get(removedNumber, blocking);
                    contact.setNumber(removedNumber);
                    mLastContact = contact;
                }
            }
        }
        /// @}
        ContactList list = new ContactList();
        int count = 0;
        for (String number : numbers) {
            if (TextUtils.isEmpty(number) || "".equals(number.trim())) {
                continue;
            }
            if (count < limit) {
                Contact contact = Contact.get(number, blocking);
                contact.setNumber(number);
                list.add(contact);
            } else {
                /// M: fix ALPS01033728, new a temp contact object for saving time.
                Contact contact = new Contact(number, true);
                list.add(contact);
            }
            count++;
        }
        MmsLog.d(TAG, "end constructContactsFromInputWithLimit");
        return list;
    }*/

    private float mChipX;
    private float mChipY;

    /// M: fix ALPS00944194, numbers in brackets is considered
    /// to comments in RFC822Tokenizer. like "86723(131)23",
    /// (131) will be think of comments, will be cut. so use PHONE_EX to walk around. @{
    private static boolean isPhoneNumber(String number) {

        if (TextUtils.isEmpty(number)) {
            return false;
        }
        Matcher match = MTKRecipientEditTextView.PHONE_EX.matcher(number);
        boolean result = match.matches();
        return result;
    }
    /// @}

    /// M: ALPS00956607, not show modify button on recipients editor @{
    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        InputConnection inputConn = super.onCreateInputConnection(outAttrs);
        mOpRecipientsEditorExt.onCreateInputConnection(outAttrs);
        return inputConn;
    }
    /// @}

    /// M: Recipient Control refactory, get recipients from FW. @{
    public boolean containsEmailFromChipWatcher() {
        if (TextUtils.indexOf(getText(), '@') == -1)
            return false;

        List<String> numbers = getNumbersFromChipWatcher();
        for (String number : numbers) {
            if (Mms.isEmailAddress(number))
                return true;
        }
        return false;
    }

    public void parseRecipientsFromChipWatcher(ArrayList<RecipientEntry> recipientList,
            ArrayList<String> changedNumbers, String lastRecipient, int limit) {
        ContactList list = new ContactList();
        List<String> numbers = new ArrayList<String>();
        if ((changedNumbers == null || (changedNumbers != null && changedNumbers.size() == 0))
                && recipientList.size() == getContactsFromChipWatcher().size()) {
            for (int i = 0; i < recipientList.size(); i++) {
                RecipientEntry recipient = recipientList.get(i);
                String number = recipient.getDestination();
                number = PhoneNumberUtils.replaceUnicodeDigits(number);
                numbers.add(number);
            }
        } else {
            ArrayList<String> changed = new ArrayList<String>();
            for (String num : changedNumbers) {
                num = PhoneNumberUtils.replaceUnicodeDigits(num);
                changed.add(num);
            }
            for (int i = 0; i < recipientList.size(); i++) {
                RecipientEntry recipient = recipientList.get(i);
                String number = recipient.getDestination();
                number = PhoneNumberUtils.replaceUnicodeDigits(number);
                numbers.add(number);
                if (i < limit) {
                    Contact contact = Contact.get(number, false);
                    if (changed.contains(number)) {
                        String name = recipient.getDisplayName();
                        long personId = recipient.getContactId();
                        if (personId <= 0 && name != null && name.equals(number)) {
                            name = "";
                        }
                        byte[] data = recipient.getPhotoBytes();
                        synchronized (contact) {
                            contact.setContactInfoFormChipWatcher(name, number, personId, data);
                        }
                    }
                    list.add(contact);
                } else {
                    Contact contact = new Contact(number, true);
                    list.add(contact);
                }
            }
            setContactsFromChipWatcher(list);
        }

        // the last recipient, should be included in numbers for setWorkingRecipients
        if (!TextUtils.isEmpty(lastRecipient) && !"".equals(
                lastRecipient.replaceAll(";", "").replaceAll(",", "").trim())) {
            /// M: fix ALPS01561087, make contact in cache is new.
            Contact contact = Contact.get(lastRecipient, false);
            contact.setNumber(lastRecipient);
            numbers.add(lastRecipient);
        }
        // delete the contact from cache if delete some numbers
        if (recipientList != null && changedNumbers != null) {
            for (String changedNumber : changedNumbers) {
                boolean addNumber = false;
                for (RecipientEntry entry : recipientList) {
                    if (changedNumber.equals(entry.getDestination())) {
                        addNumber = true;
                    }
                }
                if (addNumber == false) {
                    Contact contact = new Contact(changedNumber);
                    contact.removeFromCache();
                }
            }
        }
        //MmsLog.dpi(TAG,
        //        "parseRecipientsFromChipWatcher end, numbers = " + numbers);
        setNumbersFromChipWatcher(numbers);
    }

    private ContactList mContactList = new ContactList();
    private List<String> mNumbers = new ArrayList<String>();
    private void setContactsFromChipWatcher(ContactList list) {
        mContactList = list;
    }

    public ContactList getContactsFromChipWatcher() {
        return mContactList != null ? mContactList : new ContactList();
    }

    private void setNumbersFromChipWatcher(List<String> numbers) {
        mNumbers = numbers;
    }

    public List<String> getNumbersFromChipWatcher() {
        return mNumbers != null ? mNumbers : new ArrayList<String>();
    }
    /// @}

    /// M: call back @{
    public boolean hasValidRecipientCallback(boolean isMms) {
        return hasValidRecipient(isMms);
    }
    /// @}
}
