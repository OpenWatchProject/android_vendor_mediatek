package com.mediatek.calendar.edittext;

import android.content.Context;
import android.widget.EditText;

/**
 * M:This class is used to extension the EditText
 *
 */
public interface IEditTextExt {

    /**
     * the extension feature that set the inputText length input filter
     * @param inputText the EditText to set.
     * @param context
     * @param maxLength
     */
    void setLengthInputFilter(EditText inputText, final Context context, final int maxLength);
}
