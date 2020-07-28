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
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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

package com.mediatek.wappush.ui;

import android.content.Context;
import android.graphics.Paint;
import android.graphics.Paint.FontMetricsInt;
import android.graphics.Path;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.provider.Telephony.Mms;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.method.HideReturnsTransformationMethod;
import android.text.style.ClickableSpan;
import android.text.style.ForegroundColorSpan;
import android.text.style.LineHeightSpan;
import android.text.style.StyleSpan;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.QuickContactBadge;
import android.widget.TextView;

import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.util.MmsLog;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;


/** M:
 * WPMessageListItem
 */
public class WPMessageListItem extends LinearLayout implements Contact.UpdateListener {
    public static final String EXTRA_URLS = "com.android.mms.ExtraUrls";

    private static final StyleSpan STYLE_BOLD = new StyleSpan(Typeface.BOLD);

    private View mItemContainer;
    private TextView mBodyTextView;
    private TextView mDateView;
    private Handler mHandler;
    private WPMessageItem mMessageItem;
    private String mDefaultCountryIso;
    private Path mPath = new Path();
    private Paint mPaint = new Paint();
    private boolean mIsLastItemInList;
    private static Drawable sDefaultContactImage;
    private TextView mFromView;
    private QuickContactBadge mAvatarView;
    private ImageView mExpirationIndicator;
    private static final int DEFAULT_ICON_INDENT = 5;

    private static final String WP_TAG = "Mms/WapPush";

    public WPMessageListItem(Context context) {
        super(context);
        mDefaultCountryIso = MmsApp.getApplication().getCurrentCountryIso();

        if (sDefaultContactImage == null) {
            sDefaultContactImage
                    = context.getResources().getDrawable(R.drawable.ic_contact_picture);
        }
    }

    public WPMessageListItem(Context context, AttributeSet attrs) {
        super(context, attrs);

        int color = mContext.getResources().getColor(R.color.timestamp_color);
        mColorSpan = new ForegroundColorSpan(color);
        mDefaultCountryIso = MmsApp.getApplication().getCurrentCountryIso();

        if (sDefaultContactImage == null) {
            sDefaultContactImage
                    = context.getResources().getDrawable(R.drawable.ic_contact_picture);
        }
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        mFromView = (TextView) findViewById(R.id.from);
        mBodyTextView = (TextView) findViewById(R.id.subject);
        mDateView = (TextView) findViewById(R.id.date);
        mAvatarView = (QuickContactBadge) findViewById(R.id.avatar);
        mExpirationIndicator = (ImageView) findViewById(R.id.expiration_indicator);
    }

    public void bind(WPMessageItem msgItem, boolean isLastItem) {
        MmsLog.i(WP_TAG, "bind msgItem");
        mMessageItem = msgItem;
        mIsLastItemInList = isLastItem;
        setLongClickable(false);
        setClickable(false);
        updateBackground();
        bindCommonMessage(msgItem);
        updateAvatarView();
    }

    public void unbind() {
        // Clear all references to the message item, which can contain attachments and other
        // memory-intensive objects
        mMessageItem = null;
        Contact.removeListener(this);
    }

    public WPMessageItem getMessageItem() {
        return mMessageItem;
    }

    public View getItemContainer() {
        return mItemContainer;
    }

    public void setMsgListItemHandler(Handler handler) {
        mHandler = handler;
    }

    private void bindCommonMessage(final WPMessageItem msgItem) {
        // Since the message text should be concatenated with the sender's
        // address(or name), I have to display it here instead of
        // displaying it by the Presenter.
        mBodyTextView.setTransformationMethod(HideReturnsTransformationMethod.getInstance());

        CharSequence formattedMessage = formatMessage(msgItem, msgItem.mContact, msgItem.mText,
                msgItem.mURL, msgItem.mTimestamp, msgItem.mExpiration, msgItem.mHighlight);
        mBodyTextView.setText(formattedMessage);

        CharSequence timestamp = msgItem.mTimestamp;

        mDateView.setText(timestamp);

        // From.
        mFromView.setVisibility(VISIBLE);
        mFromView.setText(formatMessage());
        Contact.addListener(this);

        //Expiration icon
        if (msgItem.mIsExpired == 1) {
            mExpirationIndicator.setVisibility(View.VISIBLE);
        } else {
            mExpirationIndicator.setVisibility(View.GONE);
        }
        requestLayout();
    }

    private LineHeightSpan mSpan = new LineHeightSpan() {
        public void chooseHeight(CharSequence text, int start,
                int end, int spanstartv, int v, FontMetricsInt fm) {
            fm.ascent -= 10;
        }
    };

    ForegroundColorSpan mColorSpan = null;  // set in ctor


    private ClickableSpan mLinkSpan = new ClickableSpan() {
        public void onClick(View widget) {
        }
    };

    private CharSequence formatMessage(WPMessageItem msgItem, String contact,
            String mText, String mURL, String timestamp, String expiration, Pattern highlight) {
        SpannableStringBuilder buf = new SpannableStringBuilder();

        if (!TextUtils.isEmpty(mText)) {
            buf.append(mText);
            buf.append("\n");
        }
        /*
         * Fix the bug that *.inc will be not be treated as URL
         */
        if (!TextUtils.isEmpty(mURL)) {
            int urlStart = buf.length();
            buf.append(mURL);
            //new URLSpan(mURL);//it doesn't work
            buf.setSpan(mLinkSpan, urlStart, buf.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }

        if (highlight != null) {
            Matcher m = highlight.matcher(buf.toString());
            while (m.find()) {
                buf.setSpan(new StyleSpan(Typeface.BOLD), m.start(), m.end(), 0);
            }
        }

        return buf;
    }

    /// M: add for adjust font size
    public void setTextSize(float size) {
        if (mBodyTextView != null) {
//            mBodyTextView.setTextSize(size);
        }
    }

    private CharSequence formatMessage() {
        final int color = android.R.styleable.Theme_textColorSecondary;
        String from = "";
        if (mMessageItem != null) {
            from = mMessageItem.mAddress;
        }

        if (TextUtils.isEmpty(from)) {
            from = mContext.getString(android.R.string.unknownName);
        } else {
            from = Contact.get(from, true).getName();
        }
        SpannableStringBuilder buf = new SpannableStringBuilder(from);
        return buf;
    }

    private void updateAvatarView() {

        if (mMessageItem == null) {
            return;
        }
        Drawable avatarDrawable = sDefaultContactImage;
        /// M:
        Contact contact = Contact.get(mMessageItem.mAddress, true);
        avatarDrawable = contact.getAvatar(mContext, sDefaultContactImage, -1);

        // / M: fix bug ALPS00400483, same as 319320, clear all data of
        // mAvatarView firstly.
        mAvatarView.assignContactUri(null);

        // / M: Code analyze 030, For new feature ALPS00241750, Add email
        // address
        // / to email part in contact . @{
        String number = contact.getNumber();
        if (Mms.isEmailAddress(number)) {
            mAvatarView.assignContactFromEmail(number, true);
        } else {
            if (contact.existsInDatabase()) {
                mAvatarView.assignContactUri(contact.getUri());
            } else {
                mAvatarView.assignContactFromPhone(number, true);
            }
            // / @}
        }
        mAvatarView.setImageDrawable(avatarDrawable);
        mAvatarView.setVisibility(View.VISIBLE);
    }

    protected void updateBackground() {
        int backgroundId;
        if (mMessageItem.isChecked()) {
            backgroundId = R.drawable.list_selected_holo_light;
        } else if (mMessageItem.isUnread()) {
            backgroundId = R.drawable.conversation_item_background_unread;
        } else {
            backgroundId = R.drawable.conversation_item_background_read;
        }
        Drawable background = mContext.getResources().getDrawable(backgroundId);

        setBackgroundDrawable(background);
    }

    /// M: fix bug ALPS00527739, update from number contact info after contact is changed. @{
    private Handler mUiHandler = new Handler();

    private void updateFromView() {
        mFromView.setText(formatMessage());
        updateAvatarView();
    }

    private Runnable mUpdateFromViewRunnable = new Runnable() {
        public void run() {
            updateFromView();
        }
    };

    public void onUpdate(Contact updated) {
        MmsLog.ipi(WP_TAG, "onUpdate: " + this + " contact: " + updated);
        if (updated == null || mMessageItem == null
                || !updated.getNumber().equals(mMessageItem.mAddress)) {
            return;
        }
        mUiHandler.removeCallbacks(mUpdateFromViewRunnable);
        mUiHandler.post(mUpdateFromViewRunnable);
    }
    /// @}
}
