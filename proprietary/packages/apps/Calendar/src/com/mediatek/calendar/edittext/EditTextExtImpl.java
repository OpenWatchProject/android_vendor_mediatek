package com.mediatek.calendar.edittext;

import android.content.Context;
import android.os.Vibrator;
import android.text.InputFilter;
import android.text.Spanned;
import android.widget.EditText;

import com.mediatek.calendar.LogUtil;

public class EditTextExtImpl implements IEditTextExt {
    private static final String TAG = "EditTextExtensionImpl";


    @Override
    public void setLengthInputFilter(EditText inputText, Context context,
            int maxLength) {
        InputFilter[] inputFilters = createInputFilter(inputText, context, maxLength);
        if (inputFilters != null) {
            inputText.setFilters(inputFilters);
        }
    }

    private InputFilter[] createInputFilter(EditText inputText, final Context context,
            final int maxLength) {
        InputFilter[] contentFilters = new InputFilter[1];

        contentFilters[0] = new InputFilter.LengthFilter(maxLength) {
            public CharSequence filter(CharSequence source, int start, int end,
                    Spanned dest, int dstart, int dend) {
                if (source != null && source.length() > 0
                        && (((dest == null ? 0 : dest.length()) + dstart - dend) == maxLength)) {
                    Vibrator vibrator = (Vibrator) context
                            .getSystemService(context.VIBRATOR_SERVICE);
                    boolean hasVibrator = vibrator.hasVibrator();
                    if (hasVibrator) {
                        vibrator.vibrate(new long[] { 100, 100 }, -1);
                    }
                    LogUtil.w(TAG, "input out of range,hasVibrator:" + hasVibrator);
                    return "";
                }
                if (dest != null && source != null) {
                return super.filter(source, start, end, dest, dstart, dend);
            }
                else
                    return "";
            }
        };

       return contentFilters;
    }
}
