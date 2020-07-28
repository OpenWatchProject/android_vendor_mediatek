/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.cb.cbmsg;

import android.content.Context;
import android.database.Cursor;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.CursorAdapter;

import com.android.mms.R;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

/**
 * M: The back-end data adapter for CMMessageList.
 */
public class CbMessageListAdapter extends CursorAdapter implements AbsListView.RecyclerListener {
    private static final String TAG = "MmsCB/CBMessageListAdapter";
    private static final boolean LOCAL_LOGV = false;

    private final LayoutInflater mFactory;
    private OnContentChangedListener mOnContentChangedListener;
    // add for multi-delete
    public boolean mIsDeleteMode = false;
    private Map<Long, Boolean> mListItem;
    private Handler mMsgListItemHandler;

    public CbMessageListAdapter(Context context, Cursor cursor) {
        super(context, cursor, false /* auto-requery */);
        mFactory = LayoutInflater.from(context);
        mListItem = new HashMap<Long, Boolean>();
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        Log.d("MmsLog", "CBMessageListAdapter.binview");
        if (!(view instanceof CbMessageListItem)) {
            Log.e(TAG, "Unexpected bound view: " + view);
            return;
        }
        CbMessageListItem itemView = (CbMessageListItem) view;
        CbMessage message = CbMessage.from(context, cursor);
        CbMessageItem ch = new CbMessageItem(context, message);
        // for multi-delete
        if (mIsDeleteMode) {
            if (mListItem.get(message.getMessageId()) == null) {
                mListItem.put(message.getMessageId(), false);
            } else {
                ch.setSelectedState(mListItem.get(message.getMessageId()));
            }
        } else {
            ch.setSelectedState(false);
        }

        if (null != mMsgListItemHandler) {
            Message msg = Message.obtain(mMsgListItemHandler, CbMessageListItem.UPDATE_CHANNEL);
            msg.arg1 = (int) message.getChannelId();
            msg.arg2 = (int) message.getSubId();
            msg.sendToTarget();
        }

        itemView.setMsgListItemHandler(mMsgListItemHandler);
        itemView.bind(ch, cursor.getPosition() == cursor.getCount() - 1, mIsDeleteMode);
    }

    public void onMovedToScrapHeap(View view) {
        CbMessageListItem headerView = (CbMessageListItem) view;
        headerView.unbind();
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        Log.d("MmsLog", "CBMessageListAdapter.newView");
        return mFactory.inflate(R.layout.cbmessage_list_item, parent, false);
    }

    public interface OnContentChangedListener {
        void onContentChanged(CbMessageListAdapter adapter);
    }

    public void setOnContentChangedListener(OnContentChangedListener l) {
        mOnContentChangedListener = l;
    }

    // add for multi-delete
    public void changeSelectedState(long listId) {
        mListItem.put(listId, !mListItem.get(listId));
    }

    public Map<Long, Boolean> getItemList() {
        return mListItem;
    }

    public void initListMap(Cursor cursor) {
        if (cursor != null) {
            long itemId = 0;
            while (cursor.moveToNext()) {
                itemId = cursor.getLong(0);
                if (mListItem.get(itemId) == null) {
                    mListItem.put(itemId, false);
                }
            }
        }
    }

    public void setItemsValue(boolean value, long[] keyArray) {
        Iterator iterator = mListItem.entrySet().iterator();
        // keyArray = null means set the all item
        if (keyArray != null) {
            for (int i = 0; i < keyArray.length; i++) {
                mListItem.put(keyArray[i], value);
            }
        } else {
            while (iterator.hasNext()) {
                @SuppressWarnings("unchecked")
                Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iterator.next();
                entry.setValue(value);
            }
        }
    }

    public void clearList() {
        setItemsValue(false, null);
    }

    public int getSelectedNumber() {
        Iterator iter = mListItem.entrySet().iterator();
        int number = 0;
        while (iter.hasNext()) {
            @SuppressWarnings("unchecked")
            Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
            if (entry.getValue()) {
                number++;
            }
        }
        return number;
    }

    @Override
    protected void onContentChanged() {
        Log.i(TAG, "onContentChanged called !!!");
        if (getCursor() != null && !getCursor().isClosed()) {
            if (mOnContentChangedListener != null) {
                mOnContentChangedListener.onContentChanged(this);
            }
        }
    }

    public void setMsgListItemHandler(Handler handler) {
        mMsgListItemHandler = handler;
    }
}
