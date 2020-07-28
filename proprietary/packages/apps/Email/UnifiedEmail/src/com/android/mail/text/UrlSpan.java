/*
 * Copyright (C) 2014 Google Inc.
 * Licensed to The Android Open Source Project.
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

package com.android.mail.text;

import android.text.TextPaint;
import android.text.style.URLSpan;

/**
 * Extension to URLSpan that does not have underlines.
 * Stupid URLSpan.<p/>
 *
 * WARNING: this span will not work if the TextView it uses
 * saves and restores its text since TextView can only save
 * and restore {@link android.text.ParcelableSpan}s which
 * can only be implemented by framework Spans.
 */
public class UrlSpan extends URLSpan {
    public UrlSpan(String url) {
        super(url);
    }

    /**
     * Makes the text in the link color and not underlined.
     */
    @Override
    public void updateDrawState(TextPaint ds) {
        ds.setColor(ds.linkColor);
        ds.setUnderlineText(false);
    }
}
