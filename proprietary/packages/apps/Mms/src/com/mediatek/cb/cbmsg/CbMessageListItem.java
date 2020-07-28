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

package com.mediatek.cb.cbmsg;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Paint;
import android.graphics.Paint.FontMetricsInt;
import android.graphics.Path;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.net.MailTo;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.method.HideReturnsTransformationMethod;
import android.text.style.ForegroundColorSpan;
import android.text.style.LineHeightSpan;
import android.text.style.StyleSpan;
import android.text.style.URLSpan;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.browser.provider.Browser;
import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.ui.MessageListItem;
import com.android.mms.ui.MessageUtils;

import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * M: This class provides view of a message in the messages list.
 */
public class CbMessageListItem extends LinearLayout {
    public static final String EXTRA_URLS = "com.android.mms.ExtraUrls";
    public static final int UPDATE_CHANNEL = 15;
    private static final String TAG = "MmsCB/CBMessageListItem";
    private static final StyleSpan STYLE_BOLD = new StyleSpan(Typeface.BOLD);

    private boolean mIsLastItemInList;
    private CbMessageItem mMessageItem;
    private Handler mHandler;
    private View mItemContainer;
    private TextView mBodyTextView;
    private TextView mSimStatus;
    private TextView mDateView;
    private Path mPath = new Path();
    private Paint mPaint = new Paint();
    private static Drawable sDefaultContactImage;
    // add for multi-delete
    private CheckBox mSelectedBox;

    private boolean mIsTel = false;
    private String mDefaultCountryIso;
    public CbMessageListItem(Context context) {
        super(context);
        if (sDefaultContactImage == null) {
            sDefaultContactImage = context.getResources()
                    .getDrawable(R.drawable.ic_contact_picture);
        }
        mDefaultCountryIso = MmsApp.getApplication().getCurrentCountryIso();
    }

    public CbMessageListItem(Context context, AttributeSet attrs) {
        super(context, attrs);
        int color = mContext.getResources().getColor(R.color.timestamp_color);
        mColorSpan = new ForegroundColorSpan(color);
        if (sDefaultContactImage == null) {
            sDefaultContactImage = context.getResources()
                    .getDrawable(R.drawable.ic_contact_picture);
        }
        mDefaultCountryIso = MmsApp.getApplication().getCurrentCountryIso();
    }

    @Override
    protected void onFinishInflate() {
        Log.d("MmsLog", "CBMessageListItem.onFinishInflate()");
        super.onFinishInflate();
        mBodyTextView = (TextView) findViewById(R.id.text_view);
        mDateView = (TextView) findViewById(R.id.date_view);
        mItemContainer = findViewById(R.id.mms_layout_view_parent);
        // mItemContainer.setLongClickable(true);
        mSimStatus = (TextView) findViewById(R.id.sim_status);
        // add for multi-delete
        mSelectedBox = (CheckBox) findViewById(R.id.select_check_box);

    }

    public void bind(CbMessageItem msgItem, boolean isLastItem, boolean isDeleteMode) {
        mMessageItem = msgItem;
        mIsLastItemInList = isLastItem;
        setSelectedBackGroud(false);
        if (isDeleteMode) {
            mSelectedBox.setVisibility(View.VISIBLE);
            if (msgItem.isSelected()) {
                setSelectedBackGroud(true);
            }
        } else {
            mSelectedBox.setVisibility(View.GONE);
        }
        setLongClickable(false);
        mItemContainer.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                onMessageListItemClick();
            }
        });
        bindCommonMessage(msgItem);
    }

    public void onMessageListItemClick() {
        if (mSelectedBox != null && mSelectedBox.getVisibility() == View.VISIBLE) {
            if (!mSelectedBox.isChecked()) {
                setSelectedBackGroud(true);
            } else {
                setSelectedBackGroud(false);
            }
            if (null != mHandler) {
                Message msg = Message.obtain(mHandler, MessageListItem.ITEM_CLICK);
                msg.arg1 = (int) mMessageItem.getMessageId();
                msg.sendToTarget();
            }
            return;
        }

        final URLSpan[] spans = mBodyTextView.getUrls();
        final java.util.ArrayList<String> urls = MessageUtils.extractUris(spans);
        final String telPrefix = "tel:";
        String url = "";
        for (int i = 0; i < urls.size(); i++) {
            url = urls.get(i);
            if (url.startsWith(telPrefix)) {
                mIsTel = true;
                urls.add("smsto:" + url.substring(telPrefix.length()));
            }
        }
        if (spans.length == 0) {
            // do nothing
        } else if (spans.length == 1 && !mIsTel) {
            OpMessageUtils.getOpMessagePlugin().getOpMessageListItemExt()
                    .openUrl(mContext, spans[0].getURL());
            /// @}
        } else {
            ArrayAdapter<String> adapter =
                new ArrayAdapter<String>(mContext, android.R.layout.select_dialog_item, urls) {
                @Override
                public View getView(int position, View convertView, ViewGroup parent) {
                    View v = super.getView(position, convertView, parent);
                    TextView tv = (TextView) v;
                        String url = getItem(position).toString();
                        Uri uri = Uri.parse(url);

                        final String telPrefix = "tel:";
                        Drawable d = null;
                        try {
                            d = mContext.getPackageManager().getActivityIcon(
                                    new Intent(Intent.ACTION_VIEW, uri));
                        } catch (android.content.pm.PackageManager.NameNotFoundException ex) {
                            // go on, it is ok.
                        }
                        if (d != null) {
                            d.setBounds(0, 0, d.getIntrinsicHeight(), d.getIntrinsicHeight());
                            tv.setCompoundDrawablePadding(10);
                            tv.setCompoundDrawables(d, null, null, null);
                        } else {
                            if (url.startsWith(telPrefix)) {
                                d = mContext.getResources()
                                        .getDrawable(R.drawable.ic_launcher_phone);
                                d.setBounds(0, 0, d.getIntrinsicHeight(), d.getIntrinsicHeight());
                                tv.setCompoundDrawablePadding(10);
                                tv.setCompoundDrawables(d, null, null, null);
                            } else {
                                tv.setCompoundDrawables(null, null, null, null);
                            }
                        }

                        final String smsPrefix = "smsto:";
                        final String mailPrefix = "mailto";
                        if (url.startsWith(telPrefix)) {
                            url = PhoneNumberUtils.formatNumber(
                                            url.substring(telPrefix.length()), mDefaultCountryIso);
                            if (url == null) {
                                Log.d(TAG, "url turn to null after calling " +
                                        "PhoneNumberUtils.formatNumber");
                                url = getItem(position).toString().substring(telPrefix.length());
                            }
                        } else if (url.startsWith(smsPrefix)) {
                            url = PhoneNumberUtils.formatNumber(
                                            url.substring(smsPrefix.length()), mDefaultCountryIso);
                            if (url == null) {
                                Log.d(TAG, "url turn to null after " +
                                        "calling PhoneNumberUtils.formatNumber");
                                url = getItem(position).toString().substring(smsPrefix.length());
                            }
                        } else if (url.startsWith(mailPrefix)) {
                            MailTo mt = MailTo.parse(url);
                            url = mt.getTo();
                        }
                        tv.setText(url);
                    return v;
                }
            };

            AlertDialog.Builder b = new AlertDialog.Builder(mContext);
            DialogInterface.OnClickListener click = new DialogInterface.OnClickListener() {
                @Override
                public final void onClick(DialogInterface dialog, int which) {
                    if (which >= 0) {
                        Uri uri = Uri.parse(urls.get(which));
                        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                        intent.putExtra(Browser.EXTRA_APPLICATION_ID, mContext.getPackageName());
                        if (urls.get(which).startsWith("smsto:")) {
                            intent.setClassName(mContext,
                                    "com.android.mms.ui.SendMessageToActivity");
                        }
                        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                        mContext.startActivity(intent);
                        if (urls.get(which).startsWith("smsto:")) {
                            intent.setClassName(mContext,
                                    "com.android.mms.ui.SendMessageToActivity");
                        }
                    }
                    dialog.dismiss();
                }
            };

            b.setTitle(R.string.select_link_title);
            b.setCancelable(true);
            b.setAdapter(adapter, click);
            b.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                @Override
                public final void onClick(DialogInterface dialog, int which) {
                    dialog.dismiss();
                }
            });
            b.show();
        }
    }

    public void unbind() {
        // do nothing
    }

    public CbMessageItem getMessageItem() {
        return mMessageItem;
    }

    public void setMsgListItemHandler(Handler handler) {
        mHandler = handler;
    }

    private void bindCommonMessage(final CbMessageItem msgItem) {
        // Since the message text should be concatenated with the sender's
        // address(or name), I have to display it here instead of
        // displaying it by the Presenter.
        mBodyTextView.setTransformationMethod(HideReturnsTransformationMethod.getInstance());

        // Get and/or lazily set the formatted message from/on the
        // MessageItem. Because the MessageItem instances come from a
        // cache (currently of size ~50), the hit rate on avoiding the
        // expensive formatMessage() call is very high.
        CharSequence formattedMessage = msgItem.getCachedFormattedMessage();
        if (formattedMessage == null) {
            formattedMessage = formatMessage(msgItem.getSubject(), msgItem.getDate(), null);
            msgItem.setCachedFormattedMessage(formattedMessage);
        }
        mBodyTextView.setText(formattedMessage);

        CharSequence formattedTimestamp = formatTimestamp(msgItem, msgItem.getDate());
        mDateView.setText(formattedTimestamp);

        SubscriptionInfo subInfo = SubscriptionManager.from(MmsApp.getApplication())
                .getActiveSubscriptionInfo(mMessageItem.mSubId);
        if (subInfo != null) {
            mSimStatus.setText(subInfo.getDisplayName().toString());
            mSimStatus.setVisibility(View.VISIBLE);
            mSimStatus.setTextColor(subInfo.getIconTint());
        } else {
            mSimStatus.setVisibility(View.GONE);
        }
        requestLayout();
    }

    private LineHeightSpan mSpan = new LineHeightSpan() {
        public void chooseHeight(CharSequence text, int start, int end, int spanstartv, int v,
                FontMetricsInt fm) {
            fm.ascent -= 10;
        }
    };

//    TextAppearanceSpan mTextSmallSpan = new TextAppearanceSpan(mContext,
//            android.R.style.TextAppearance_Small);

//    AlignmentSpan.Standard mAlignRight = new AlignmentSpan.Standard(Alignment.ALIGN_OPPOSITE);

    ForegroundColorSpan mColorSpan = null; // set in ctor

//    private ClickableSpan mLinkSpan = new ClickableSpan() {
//        public void onClick(View widget) {
//        }
//    };

    private CharSequence formatMessage(String body, String timestamp, Pattern highlight) {
        SpannableStringBuilder buf = new SpannableStringBuilder();

        if (!TextUtils.isEmpty(body)) {
            buf.append(body);
        }

        if (highlight != null) {
            Matcher m = highlight.matcher(buf.toString());
            while (m.find()) {
                buf.setSpan(new StyleSpan(Typeface.BOLD), m.start(), m.end(), 0);
            }
        }

        return buf;
    }

    private CharSequence formatTimestamp(CbMessageItem msgItem, String timestamp) {
        SpannableStringBuilder buf = new SpannableStringBuilder();
        buf.append(TextUtils.isEmpty(timestamp) ? " " : timestamp);
        buf.setSpan(mSpan, 1, buf.length(), 0);

        // buf.setSpan(mTextSmallSpan, 0, buf.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        // Make the timestamp text not as dark
        buf.setSpan(mColorSpan, 0, buf.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        return buf;
    }

    public void setSelectedBackGroud(boolean selected) {
        if (selected) {
            mSelectedBox.setChecked(true);
            mSelectedBox.setBackgroundDrawable(null);
            setBackgroundResource(R.drawable.list_selected_holo_light);
        } else {
            setBackgroundDrawable(null);
            mSelectedBox.setChecked(false);
            mSelectedBox.setBackgroundResource(R.drawable.listitem_background);
        }
    }
}
