package com.android.emailcommon.utility;

import java.util.ArrayList;
import java.util.StringTokenizer;

import com.android.mail.utils.LogUtils;

import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.style.BackgroundColorSpan;

/**
 * M: A plain string highlight builder, which use string and query string as input, and build highlighted
 * char sequences.
 */
public class StringHighlightBuilder {
    protected static final String LOG_TAG = LogUtils.TAG;

    protected String mStrToHighlight; // The string/html to be highlighted
    protected String mQueryStr; // The string to highlight
    protected String mBodyStr; // The real body string
    protected ArrayList<BodyCharInfor> mBodyCharInfors; // The char information for real body text
    protected SpannableStringBuilder mSpanStrBuilder; // The output string for plain text input

    public StringHighlightBuilder(String text, String query) {
        mStrToHighlight = text;
        mQueryStr = query;
        mBodyCharInfors = new ArrayList<BodyCharInfor>();
        mSpanStrBuilder = new SpannableStringBuilder();
    }

    /**
     * Init the string highlight builder, return whether we need to do highlight operations.
     * @return true if we need to do highlight operation and vice versa.
     */
    protected boolean init() {
        // Handle null and empty string
        if (TextUtils.isEmpty(mStrToHighlight) || TextUtils.isEmpty(mQueryStr)) {
            // Just use the input as output in this case.
            mSpanStrBuilder.append((mStrToHighlight == null) ? "" : mStrToHighlight);
            return false;
        }

        final int length = mStrToHighlight.length();

        // Walk through the text until we're done with the input
        for (int i = 0; i < length; i++) {
            char chr = mStrToHighlight.charAt(i);
            // After all that, we've got some "body" text
            mSpanStrBuilder.append(chr);
            // Collect all body text in first pass
            mBodyCharInfors.add(new BodyCharInfor(chr, i));
        }
        mBodyStr = getBodyString();
        return true;
    }

    /**
     * Mark all matched query strings.
     */
    private void markAllQueryString() {
        // Break up the query into search strings and mark one by one
        if (mQueryStr != null) {
            StringTokenizer st = new StringTokenizer(mQueryStr);
            while (st.hasMoreTokens()) {
                markQueryString(0, st.nextToken());
            }
        }
    }

    /**
     * Mark corresponding character inforamtion as need highlight for all matched search string, since
     * position start, inclusively.
     * Use recursive method.
     * @param start
     * @param qStr
     */
    private void markQueryStringRecursive(int start, String qStr) {
        int index = markFirstQueryString(start, qStr);
        if (index >= start) {
            markQueryStringRecursive(index + 1, qStr);
        } else {
            LogUtils.d(LOG_TAG, "No more sub string found");
        }
    }

    /**
     * Mark corresponding character inforamtion as need highlight for all matched search string, since
     * position start, inclusively.
     * Use loop method.
     * @param start
     * @param qStr
     */
    private void markQueryString(int start, String qStr) {
        int index = start;
        int nextStart = start;
        do {
            index = markFirstQueryString(nextStart, qStr);
            nextStart = index + 1;
        } while (index >= start);
    }

    /**
     * Mark corresponding character information as need highlight for first matched search string no
     * case-sensitivity, since position start, inclusively.
     */
    private int markFirstQueryString(int start, String qStr) {
        if (qStr == null || mBodyStr == null) {
            LogUtils.e(LOG_TAG, "QueryString [%s], BodyString [%s]", qStr, mBodyStr);
            return -1;
        }
        if (start < 0 || start + qStr.length() > mBodyStr.length()) {
            return -1;
        }
        int index = mBodyStr.toLowerCase().indexOf(qStr.toLowerCase(), start);
        if (index >= start) {
            markHighlight(index, qStr.length() + index - 1);
        }
        return index;
    }

    public void addSigChar(char ch, int pos) {
        addSigChar(new BodyCharInfor(ch, pos));
    }

    public void addSigChar(BodyCharInfor sc) {
        mBodyCharInfors.add(sc);
    }

    private void markHighlight(int start, int end) {
        if (start < 0 || end >= mBodyCharInfors.size()) {
            LogUtils.e(LOG_TAG, "Wrong start or end!");
            return;
        }
        for (int i = start; i <= end; ++i) {
            mBodyCharInfors.get(i).mNeedHighlight = true;
        }
    }

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Characters before a ! means it should be highlighted!");
        for (BodyCharInfor sc : mBodyCharInfors) {
            sb.append(sc.mChar);
            sb.append(sc.mNeedHighlight ? '!' : '~');
        }
        return sb.toString();
    }

    /**
     * Get real body string
     * @return
     */
    protected String getBodyString() {
        StringBuilder sb = new StringBuilder();
        for (BodyCharInfor sc : mBodyCharInfors) {
            sb.append(sc.mChar);
        }
        return sb.toString();
    }

    /**
     * Do the real highlight operation
     */
    protected void performHighlight() {
        boolean inHighlight = false;
        BodyCharInfor highlightStart = null;
        BodyCharInfor highlightEnd = null;
        for (BodyCharInfor sc: mBodyCharInfors) {
            if (sc.mNeedHighlight) {
                if (!inHighlight) {
                    inHighlight = true;
                    highlightStart = sc;
                }
            } else {
                if (inHighlight) {
                    inHighlight = false;
                    highlightEnd = sc;
                    mSpanStrBuilder.setSpan(new BackgroundColorSpan(TextUtilities.HIGHLIGHT_COLOR_INT), highlightStart.mOriginalPos,
                            highlightEnd.mOriginalPos, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
                }
            }
        }
        if (inHighlight) {
            inHighlight = false;
            highlightEnd = mBodyCharInfors.get(mBodyCharInfors.size() - 1);
            mSpanStrBuilder.setSpan(new BackgroundColorSpan(TextUtilities.HIGHLIGHT_COLOR_INT), highlightStart.mOriginalPos,
                    highlightEnd.mOriginalPos + 1, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }
    }

    public CharSequence build() {
        if (init()) {
            markAllQueryString();
            performHighlight();
        }
        return mSpanStrBuilder;
    }

    /**
     * M: This class is used to collect real body text chars' information, ie. char/original position
     * in original char sequence, and whether it need to be highlighted. Only used to collect body
     * text chars, while html tag chars will be ignored.
     */
    class BodyCharInfor {
        char mChar; // The character
        int mOriginalPos; // The position of this body character in oringinal string
        boolean mNeedHighlight; // Whether need to be highlighted

        public BodyCharInfor() {
            mChar = '\0';
            mOriginalPos = -1;
            mNeedHighlight = false;
        }

        public BodyCharInfor(char c, int pos) {
            mChar = c;
            mOriginalPos = pos;
            mNeedHighlight = false;
        }
    }
}
