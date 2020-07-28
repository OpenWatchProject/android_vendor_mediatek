package com.android.mail.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.FrameLayout;

import com.android.mail.utils.LogTag;
import com.android.mail.utils.LogUtils;

/**
 * M: Abstract interface SwipeableItemView to supply default implementation
 * for {@link dismiss()} on ConversationSpecialView which should be had focus
 * cleared before detaching from ListView
 */
public abstract class ConversationSpecialItemAbstractView extends FrameLayout
    implements ConversationSpecialItemView, SwipeableItemView {

    private static final String LOG_TAG = LogTag.getLogTag();

    public ConversationSpecialItemAbstractView(Context context) {
        super(context);
    }

    public ConversationSpecialItemAbstractView(Context context,
            AttributeSet attrs) {
        super(context, attrs);
    }

    public ConversationSpecialItemAbstractView(Context context,
            AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    public void dismiss() {
        if (hasFocus()) {
            LogUtils.d(LOG_TAG, "Oops! ConverationSpecialItem dismiss with Focus %s but isFocusable is %s."
                    , hasFocus(), isFocusable());
            clearFocus();
        }
    }
}
