/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
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
import android.text.ClipboardManager;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.widget.ListView;

public final class MessageListView extends ListView {
    private static final String TAG = "MessageListView";
    private OnSizeChangedListener mOnSizeChangedListener;
    private VelocityTracker mVelocityTracker = null;
    // M:just for getting scroll velocity
    private static float mVelocity = 2;
    public MessageListView(Context context) {
        super(context);
    }

    public MessageListView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        super.onTouchEvent(event);
        int action = event.getAction();
        switch (action) {
        case MotionEvent.ACTION_DOWN:
            if (mVelocityTracker == null) {
                mVelocityTracker = VelocityTracker.obtain();
            } else {
                mVelocityTracker.clear();
            }
            mVelocityTracker.addMovement(event);
            break;
        case MotionEvent.ACTION_MOVE:
            if (mVelocityTracker == null) {
                mVelocityTracker = VelocityTracker.obtain();
            }
            mVelocityTracker.addMovement(event);
            mVelocityTracker.computeCurrentVelocity(1000);

            mVelocity = mVelocityTracker.getYVelocity();
            MyScrollListener.setScrollVelocity(mVelocity);
            break;
        case MotionEvent.ACTION_UP:
            break;
        case MotionEvent.ACTION_CANCEL:
            if (mVelocityTracker != null) {
                ///M: Modify for fix alps01259991. when action_cancel,
                /// clear tracker and set it to null.
                mVelocityTracker.clear();
                mVelocityTracker.recycle();
                mVelocityTracker = null;
                /// @}
            }
            break;
        }

        return true;
    }
    @Override
    public boolean onKeyShortcut(int keyCode, KeyEvent event) {
        switch (keyCode) {
        case KeyEvent.KEYCODE_C:
            MessageListItem view = (MessageListItem) getSelectedView();
            if (view == null) {
                break;
            }
            MessageItem item = view.getMessageItem();
            if (item != null && item.isSms()) {
                ClipboardManager clip =
                    (ClipboardManager) getContext().getSystemService(Context.CLIPBOARD_SERVICE);
                clip.setText(item.mBody);
                return true;
            }
            break;
        }

        return super.onKeyShortcut(keyCode, event);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        if (mOnSizeChangedListener != null) {
            mOnSizeChangedListener.onSizeChanged(w, h, oldw, oldh);
        }
    }

    @Override
    protected void layoutChildren() {
        try {
            super.layoutChildren();
        } catch (IllegalStateException e) {
            Log.e(TAG, "layoutChildren, Adapter.getCount()=" + getAdapter().getCount()
                  + ", ListView.getChildCount()=" + getChildCount());
            throw new IllegalStateException("IllegalStateException = " + e);
        }
    }

    /**
     * Set the listener which will be triggered when the size of
     * the view is changed.
     */
    void setOnSizeChangedListener(OnSizeChangedListener l) {
        mOnSizeChangedListener = l;
    }

    public interface OnSizeChangedListener {
        void onSizeChanged(int width, int height, int oldWidth, int oldHeight);
    }
}

