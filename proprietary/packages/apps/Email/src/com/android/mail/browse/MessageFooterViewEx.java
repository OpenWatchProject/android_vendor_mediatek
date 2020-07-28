/*
 * Copyright (C) 2012 Google Inc.
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

package com.android.mail.browse;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;

import com.android.email.R;
import com.android.mail.browse.ConversationViewAdapter.MessageHeaderItem;
import com.android.mail.providers.UIProvider;
import com.android.mail.utils.LogUtils;

public class MessageFooterViewEx extends MessageFooterView {

    private static final String TAG = LogUtils.TAG;
    private MessageViewDownloadRemain mMsgRemain;
    private boolean mPartialLoaded = false;
    public MessageFooterViewEx(Context context) {
        super(context);
    }

    public MessageFooterViewEx(Context context, AttributeSet attrs) {
        super(context, attrs);
        mInflater = LayoutInflater.from(context);
    }

    @Override
    public void bind(MessageHeaderItem headerItem,
            boolean measureOnly, boolean hasAttachment) {
        LogUtils.d(TAG, "_________________ MessageFooterViewEx bind hasAttachment: " + hasAttachment);
        /// M: Call super {@MessageFooterView} for unified behavior
        super.bind(headerItem, measureOnly, hasAttachment);
        mPartialLoaded = (headerItem.getMessage().flagLoaded == UIProvider.MessageFlagLoaded.FLAG_LOADED_PARTIAL);
        LogUtils.d(TAG, "_________________ MessageFooterViewEx bind mPartialLoaded: " + mPartialLoaded);
        if (!mPartialLoaded && !hasAttachment) {
            /// M: Gone the footer view as no attachments either
            setVisibility(View.GONE);
        } else {
            setVisibility(View.VISIBLE);
        }
        bindMessageRemainButton(headerItem.getMessage());
    }

    private void bindMessageRemainButton(ConversationMessage conversationMessage) {
        mMsgRemain = (MessageViewDownloadRemain) findViewById(R.id.msg_remain);
        if (mMsgRemain == null) {
            LogUtils.d(TAG, "_________________ MessageFooterViewEx no remain view found");
            mMsgRemain = MessageViewDownloadRemain.inflate(mInflater, this);
        }
        /// M: init fragment manager @{
        mMsgRemain.initialize(mFragmentManager);
        /// @}
        LogUtils.d(TAG, "_________________ MessageFooterViewEx render id:%d state:%d",
                conversationMessage.id, conversationMessage.state);
        mMsgRemain.render(conversationMessage, mPartialLoaded);
    }
}
