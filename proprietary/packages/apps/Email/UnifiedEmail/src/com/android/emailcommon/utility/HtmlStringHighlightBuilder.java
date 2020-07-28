package com.android.emailcommon.utility;

import com.android.mail.utils.LogUtils;

import android.text.TextUtils;

/**
 * M: A html string highlight builder, which use string and query string as input, and build highlighted
 * char sequences.
 */
public class HtmlStringHighlightBuilder extends StringHighlightBuilder {
    private static final String HTML_HIGHTLIGHT_START = "<span style=\"background-color: "
            + TextUtilities.HIGHLIGHT_COLOR_STRING + "\">";
    private static final String HTML_HIGHTLIGHT_END = "</span>";
    private static int HTML_HIGHLIGHT_START_LEN = HTML_HIGHTLIGHT_START.length();
    private static int HTML_HIGHLIGHT_END_LEN = HTML_HIGHTLIGHT_END.length();

    public HtmlStringHighlightBuilder(String text, String query) {
        super(text, query);
    }

    @Override
    protected boolean init() {
        // Handle null and empty string
        if (TextUtils.isEmpty(mStrToHighlight) || TextUtils.isEmpty(mQueryStr)) {
            LogUtils.d(LOG_TAG, "Highlight builder with empty string [%s] or empty query string [%s]", mStrToHighlight, mQueryStr);
            // Just use the input as output in this case.
            mSpanStrBuilder.append((mStrToHighlight == null) ? "" : mStrToHighlight);
            return false;
        }

        /// M: skipCount is an array of a single int; that int is set inside stripHtmlEntity and is
        /// used to determine how many characters can be "skipped" due to the transformation of the
        /// entity to a single character.  When Java allows multiple return values, we can make this
        /// much cleaner :-)
        int[] skipCount = new int[1];
        char entityChar;
        final int length = mStrToHighlight.length();

        // Indicates whether we're in the middle of an HTML tag
        boolean inTag = false;

        // Walk through the text until we're done with the input
        // Just copy any HTML tags directly into the output; search for terms in the remaining text
        for (int i = 0; i < length; i++) {
            char chr = mStrToHighlight.charAt(i);
            if (!inTag && (chr == '<')) {
                // Find tags; they will begin with <! or !- or </ or <letter
                if (i < (length - 1)) {
                    char peek = mStrToHighlight.charAt(i + 1);
                    if (peek == '!' || peek == '-' || peek == '/' || Character.isLetter(peek)) {
                        inTag = true;
                        // Skip content of title, script, style and applet tags
                        if (i < (length - (TextUtilities.MAX_STRIP_TAG_LENGTH + 2))) {
                            String tag = mStrToHighlight.substring(i + 1, i + TextUtilities.MAX_STRIP_TAG_LENGTH + 1);
                            String tagLowerCase = tag.toLowerCase();
                            boolean stripContent = false;
                            for (String stripTag: TextUtilities.STRIP_TAGS) {
                                if (tagLowerCase.startsWith(stripTag)) {
                                    stripContent = true;
                                    tag = tag.substring(0, stripTag.length());
                                    break;
                                }
                            }
                            if (stripContent) {
                                // Look for the end of this tag
                                int endTagPosition = TextUtilities.findTagEnd(mStrToHighlight, tag, i);
                                if (endTagPosition < 0) {
                                    mSpanStrBuilder.append(mStrToHighlight.substring(i));
                                    break;
                                } else {
                                    mSpanStrBuilder.append(mStrToHighlight.substring(i, endTagPosition - 1));
                                    i = endTagPosition - 1;
                                    chr = mStrToHighlight.charAt(i);
                                }
                            }
                        }
                    }
                }
            } else if (inTag && (chr == '>')) {
                inTag = false;
                ///M: the last tag ,should we append it
                mSpanStrBuilder.append(chr);
                continue;
            }

            if (inTag) {
                mSpanStrBuilder.append(chr);
                continue;
            } else if (chr == '&') {
                /// M: Handle a possible HTML entity here
                /// We always get back a character to use; we also get back a "skip count",
                /// indicating how many characters were eaten from the entity
                entityChar = TextUtilities.stripHtmlEntity(mStrToHighlight, i, skipCount);
                if (entityChar != '&') {
                    // Find HTML entity, we should restore it
                    /// M: Get the correct entity(original:the last character will be missed,eg: ";" of "&nbsp;")
                    mSpanStrBuilder.append(mStrToHighlight.subSequence(i, i + skipCount[0] + 1));
                    i += skipCount[0];
                    continue;
                }
            }
            // After all that, we've got some "body" text
            mSpanStrBuilder.append(chr);
            // Collect all body text in first pass
            mBodyCharInfors.add(new BodyCharInfor(chr, mSpanStrBuilder.length() - 1));
        }
        mBodyStr = getBodyString();
        return true;
    }

    @Override
    protected void performHighlight() {
        boolean inHighlight = false;
        BodyCharInfor highlightStart = null;
        BodyCharInfor highlightEnd = null;
        int posOffset = 0;
        int nextContHLOrigPos = -1; // Next continious highlight original position
        for (BodyCharInfor sc: mBodyCharInfors) {
            if (sc.mNeedHighlight) {
                if (!inHighlight) {
                    inHighlight = true;
                    highlightStart = sc;
                    mSpanStrBuilder.insert(posOffset + sc.mOriginalPos, HTML_HIGHTLIGHT_START);
                    posOffset += HTML_HIGHLIGHT_START_LEN;
                    nextContHLOrigPos = sc.mOriginalPos + posOffset + 1;
                } else if (nextContHLOrigPos != -1 && nextContHLOrigPos != sc.mOriginalPos + posOffset) {
                    // no more continious body text, which means we encounter some tag text here, make an end
                    inHighlight = false;
                    mSpanStrBuilder.insert(nextContHLOrigPos, HTML_HIGHTLIGHT_END);
                    posOffset += HTML_HIGHLIGHT_END_LEN;
                    nextContHLOrigPos = -1;
                    // and then, make an start right in front of sc
                    inHighlight = true;
                    highlightStart = sc;
                    mSpanStrBuilder.insert(posOffset + sc.mOriginalPos, HTML_HIGHTLIGHT_START);
                    posOffset += HTML_HIGHLIGHT_START_LEN;
                    nextContHLOrigPos = sc.mOriginalPos + posOffset + 1;
                } else {
                    nextContHLOrigPos++;
                }
            } else {
                if (inHighlight) {
                    inHighlight = false;
                    if (nextContHLOrigPos != -1 && nextContHLOrigPos != sc.mOriginalPos + posOffset) {
                        // no more continious body text, which means we encounter some tag text here, make an end
                        mSpanStrBuilder.insert(nextContHLOrigPos, HTML_HIGHTLIGHT_END);
                        posOffset += HTML_HIGHLIGHT_END_LEN;
                        nextContHLOrigPos = -1;
                    } else {
                        highlightEnd = sc;
                        mSpanStrBuilder.insert(posOffset + highlightEnd.mOriginalPos, HTML_HIGHTLIGHT_END);
                        posOffset += HTML_HIGHLIGHT_END_LEN;
                    }
                }
                nextContHLOrigPos = -1;
            }
        }
        if (inHighlight) {
            inHighlight = false;
            mSpanStrBuilder.insert(posOffset + highlightStart.mOriginalPos, HTML_HIGHTLIGHT_START);
            posOffset += HTML_HIGHLIGHT_START_LEN;
            mSpanStrBuilder.insert(posOffset + mBodyCharInfors.get(mBodyCharInfors.size() - 1).mOriginalPos + 1,
                    HTML_HIGHTLIGHT_END);
            posOffset += HTML_HIGHLIGHT_END_LEN;
        }
    }
}
