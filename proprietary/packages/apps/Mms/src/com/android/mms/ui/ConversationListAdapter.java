/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
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
import android.database.Cursor;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;

import com.android.mms.R;
import com.android.mms.data.Conversation;
import com.android.mms.util.MmsLog;

import java.util.HashSet;

/**
 * The back-end data adapter for ConversationList.
 */
//TODO: This should be public class ConversationListAdapter extends ArrayAdapter<Conversation>
public class ConversationListAdapter extends MessageCursorAdapter
        implements AbsListView.RecyclerListener {
    private static final String TAG = "ConversationListAdapter";
    private static final boolean LOCAL_LOGV = false;

    private final LayoutInflater mFactory;
    private OnContentChangedListener mOnContentChangedListener;

    private static HashSet<Long> sSelectedTheadsId;

    public ConversationListAdapter(Context context, Cursor cursor) {
        super(context, cursor, false /* auto-requery */);
        mFactory = LayoutInflater.from(context);
        sSelectedTheadsId = new HashSet<Long>();
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        if (!(view instanceof ConversationListItem)) {
            Log.e(TAG, "Unexpected bound view: " + view);
            return;
        }
        ConversationListItem headerView = (ConversationListItem) view;

        /// M: Code analyze 027, For bug ALPS00331731, set conversation cache . @{
        Conversation conv;
        if (!mIsScrolling) {
            Conversation.setNeedCacheConv(false);
            conv = Conversation.from(context, cursor);
            Conversation.setNeedCacheConv(true);
            if (conv != null) {
                conv.setIsChecked(sSelectedTheadsId.contains(conv.getThreadId()));
            }
            headerView.bind(context, conv);
        } else {
            conv = Conversation.getConvFromCache(context, cursor);
            if (conv != null) {
                conv.setIsChecked(sSelectedTheadsId.contains(conv.getThreadId()));
            }
            headerView.bindDefault(conv);
        }
        /// @}
    }

    public void onMovedToScrapHeap(View view) {
        ConversationListItem headerView = (ConversationListItem) view;
        headerView.unbind();
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        if (LOCAL_LOGV) {
            Log.v(TAG, "inflating new view");
        }
        return mFactory.inflate(R.layout.conversation_list_item, parent, false);
    }

    public interface OnContentChangedListener {
        void onContentChanged(ConversationListAdapter adapter);
    }

    public void setOnContentChangedListener(OnContentChangedListener l) {
        mOnContentChangedListener = l;
    }

    @Override
    protected void onContentChanged() {
        if (mCursor != null && !mCursor.isClosed()) {
            if (mOnContentChangedListener != null) {
                mOnContentChangedListener.onContentChanged(this);
            }
        }
    }

    /// M: Code analyze 026, personal use, caculate time . @{
     @Override
    public void notifyDataSetChanged() {
        super.notifyDataSetChanged();
    }
     /// @}

    /// M: For ConversationList to check listener @{
    public OnContentChangedListener getOnContentChangedListener() {
        return mOnContentChangedListener;
    }
    /// @}

    public void setSelectedState(long threadid) {
        if (sSelectedTheadsId != null) {
            sSelectedTheadsId.add(threadid);
        }
    }

    public static void removeSelectedState(long threadid) {
        if (sSelectedTheadsId != null) {
            sSelectedTheadsId.remove(threadid);
        }
    }

    public boolean isContainThreadId(long threadid) {
        if (sSelectedTheadsId != null) {
            return sSelectedTheadsId.contains(threadid);
        }
        return false;
    }

    public void clearstate() {
        if (sSelectedTheadsId != null) {
            sSelectedTheadsId.clear();
        }
    }

    public HashSet<Long> getSelectedThreadsList() {
            return sSelectedTheadsId;
    }
}
