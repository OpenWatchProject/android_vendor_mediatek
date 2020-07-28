/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
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
package com.android.emailcommon.utility;

import android.text.Annotation;
import android.text.Editable;
import android.text.Html;
import android.text.Spannable;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.xml.sax.XMLReader;

/**
 * M:Contains common routines to convert html to text and vice versa.
 */
public class HtmlConverter {
    /**
     * When generating previews, Spannable objects that can't be converted into
     * a String are represented as 0xfffc. When displayed, these show up as
     * undisplayed squares. These constants define the object character and the
     * replacement character.
     */
    private static final char PREVIEW_OBJECT_CHARACTER = (char) 0xfffc;
    private static final char PREVIEW_OBJECT_REPLACEMENT = (char) 0x20; // space

    /**
     * toHtml() converts non-breaking spaces into the UTF-8 non-breaking space,
     * which doesn't get rendered properly in some clients. Replace it with a
     * simple space.
     */
    private static final char NBSP_CHARACTER = (char) 0x00a0; // utf-8
                                                              // non-breaking
                                                              // space
    private static final char NBSP_REPLACEMENT = (char) 0x20; // space

    private static final Pattern PLAIN_TEXT_TO_ESCAPE = Pattern
            .compile("\r?\n\r?\n");

    /**
     * Escape some special character as HTML escape sequence.
     *
     * @param text
     *            Text to be displayed using WebView.
     * @return Text correctly escaped.
     */
    public static String escapeCharacterToDisplay(String text) {
        Pattern pattern = PLAIN_TEXT_TO_ESCAPE;
        Matcher match = pattern.matcher(text);
        if (match.find()) {
            StringBuilder out = new StringBuilder();
            int end = 0;
            do {
                int start = match.start();
                out.append(text.substring(end, start));
                end = match.end();
                int c = text.codePointAt(start);
                if (c == '\r' || c == '\n') {
                    out.append("\r\n");
                }
            } while (match.find());
            out.append(text.substring(end));
            text = out.toString();
        }
        return text;
    }

    /**
     * Convert an HTML string to a plain text string.
     *
     * @param html
     *            HTML string to convert.
     * @return Plain text result.
     */
    public static String htmlToText(final String html) {
        String text = Html.fromHtml(html, null, new HtmlToTextTagHandler())
                .toString()
                .replace(PREVIEW_OBJECT_CHARACTER, PREVIEW_OBJECT_REPLACEMENT)
                .replace(NBSP_CHARACTER, NBSP_REPLACEMENT);
        return escapeCharacterToDisplay(text);
    }

    /**
     * Custom tag handler to use when converting HTML messages to text. It
     * currently handles text representations of HTML tags that Android's
     * built-in parser doesn't understand and hides code contained in STYLE and
     * SCRIPT blocks.
     */
    private static class HtmlToTextTagHandler implements Html.TagHandler {
        // List of tags whose content should be ignored.
        private static final Set<String> TAGS_WITH_IGNORED_CONTENT = Collections
                .unmodifiableSet(new HashSet<String>() {
                    {
                        add("style");
                        add("script");
                        add("title");
                        add("!"); // comments
                    }
                });

        public void handleTag(boolean opening, String tag, Editable output,
                XMLReader xmlReader) {
            tag = tag.toLowerCase();
            if (tag.equals("hr") && opening) {
                // In the case of an <hr>, replace it with a bunch of
                // underscores. This is roughly
                // the behaviour of Outlook in Rich Text mode.
                // output.append("_____________________________________________\n");
            } else if (TAGS_WITH_IGNORED_CONTENT.contains(tag)) {
                handleIgnoredTag(opening, output);
            }
        }

        private static final String IGNORED_ANNOTATION_KEY = "K9_ANNOTATION";
        private static final String IGNORED_ANNOTATION_VALUE = "hiddenSpan";

        /**
         * When we come upon an ignored tag, we mark it with an Annotation
         * object with a specific key and value as above. We don't really need
         * to be checking these values since Html.fromHtml() doesn't use
         * Annotation spans, but we should do it now to be safe in case they do
         * start using it in the future.
         *
         * @param opening
         *            If this is an opening tag or not.
         * @param output
         *            Spannable string that we're working with.
         */
        private void handleIgnoredTag(boolean opening, Editable output) {
            int len = output.length();
            if (opening) {
                output.setSpan(new Annotation(IGNORED_ANNOTATION_KEY,
                        IGNORED_ANNOTATION_VALUE), len, len,
                        Spannable.SPAN_MARK_MARK);
            } else {
                Object start = getOpeningAnnotation(output);
                if (start != null) {
                    int where = output.getSpanStart(start);
                    // Remove the temporary Annotation span.
                    output.removeSpan(start);
                    // Delete everything between the start of the Annotation and
                    // the end of the string
                    // (what we've generated so far).
                    output.delete(where, len);
                }
            }
        }

        /**
         * Fetch the matching opening Annotation object and verify that it's the
         * one added by K9.
         *
         * @param output
         *            Spannable string we're working with.
         * @return Starting Annotation object.
         */
        private Object getOpeningAnnotation(Editable output) {
            Object[] objs = output.getSpans(0, output.length(),
                    Annotation.class);
            for (int i = objs.length - 1; i >= 0; i--) {
                Annotation span = (Annotation) objs[i];
                if (output.getSpanFlags(objs[i]) == Spannable.SPAN_MARK_MARK
                        && span.getKey().equals(IGNORED_ANNOTATION_KEY)
                        && span.getValue().equals(IGNORED_ANNOTATION_VALUE)) {
                    return objs[i];
                }
            }
            return null;
        }
    }
}
