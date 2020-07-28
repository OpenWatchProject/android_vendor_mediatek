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

import com.android.mms.R;
import com.android.mms.ui.MessageUtils;

/**
 * M: A holder class for a conversation header.
 */
public class CbMessageItem {
    private CbMessage mMessage;
    int mSubId;
    private int mChannelId;

    private long mId;
    private String mSubject;
    private String mDate;
    private String mFrom;
    // add for multi-delete
    boolean mItemSelected = false;

    // the presence icon resource id displayed for the conversation thread.
    private int mPresenceResId;

    CharSequence mCachedFormattedMessage;

    public CbMessageItem(Context context, CbMessage message) {
        mMessage = message;
        mId = message.getMessageId();
        mPresenceResId = 0;
        mSubject = message.getBody();
        mFrom = message.getDisplayName().toString();
        mDate = String.format(context.getString(R.string.received_on), MessageUtils
                .formatTimeStampString(context, message.getDate()));
        mChannelId = message.getChannelId();
        mSubId = message.getSubId();
    }

    /**
     * @return Returns the ID of the thread.
     */
    public long getMessageId() {
        return mId;
    }

    /**
     * @return Returns the date.
     */
    public String getDate() {
        return mDate;
    }

    public int getChannelId() {
        return mChannelId;
    }

    /**
     * @return Returns the from. (formatted for display)
     */
    public String getFrom() {
        return mFrom;
    }

    public int getPresenceResourceId() {
        return mPresenceResId;
    }

    /**
     * @return Returns the subject.
     */
    public String getSubject() {
        return mSubject;
    }

    public void setCachedFormattedMessage(CharSequence formattedMessage) {
        mCachedFormattedMessage = formattedMessage;
    }

    public CharSequence getCachedFormattedMessage() {
        return mCachedFormattedMessage;
    }

    // add for multi-delete
    public boolean isSelected() {
        return mItemSelected;
    }

    public void setSelectedState(boolean isSelected) {
        mItemSelected = isSelected;
    }

    @Override
    public String toString() {
        return "[CBMessageHeader from:" + getFrom() + " subject:" + getSubject() + "]";
    }

    public int getSubId() {
        return mSubId;
    }

}
