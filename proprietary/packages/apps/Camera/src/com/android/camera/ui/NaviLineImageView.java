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

package com.android.camera.ui;

import android.content.Context;
import android.util.AttributeSet;
import android.widget.ImageView;


/**
 * A @{code ImageView} used for naviline in panorama view.
 */
public class NaviLineImageView extends ImageView {
    // private static final String TAG = "NaviLineImageView";

    private int mLeft;
    private int mTop;
    private int mRight;
    private int mBottom;
    private boolean mFirstDraw;

    public NaviLineImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
    }

    public void layout(int l, int t, int r, int b) {
        if (!mFirstDraw || (mLeft == l && mTop == t && mRight == r && mBottom == b)) {
            super.layout(l, t, r, b);
            mFirstDraw = true;
        }
    }

    public void setLayoutPosition(int l, int t, int r, int b) {
        mLeft = l;
        mTop = t;
        mRight = r;
        mBottom = b;
        layout(l, t, r, b);
    }
}
