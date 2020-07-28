package com.android.calendar;

import android.content.Context;
import android.text.Editable;
import android.util.AttributeSet;

import com.android.mtkex.chips.MTKRecipientEditTextView;

public class ChipsAddressTextView extends MTKRecipientEditTextView {

    public ChipsAddressTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setThreshold(1);
    }

    /**
     * M: Not perform GAL searching if current selection ending with a completed address.
     * (ending with ", " or "; ").
     * By default, if current text is "xxx@126.com, t" and user delete the last char 't',
     * it will auto perform searching 'xxx@126.com, ', which will cause UI abnormal
     * and list popup window showing for a long time.
     * After fixed, it will not search 'xxx@126.com, ' when delete the last char 't'.
     */
    @Override
    public boolean enoughToFilter() {
        Editable s = getText();
        if (s != null) {
            int end = s.length();
            if (end > 2 && s.charAt(end - 1) == ' '
                    && (s.charAt(end - 2) == ',' || s.charAt(end - 2) == ';')) {
                return false;
            }
        }
        return super.enoughToFilter();
    }
}
