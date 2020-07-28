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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Typeface;
import android.graphics.Paint.FontMetricsInt;
import android.graphics.drawable.Drawable;
import android.net.MailTo;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.Telephony;
import android.provider.ContactsContract.Profile;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.Html;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.text.method.HideReturnsTransformationMethod;
import android.text.style.ForegroundColorSpan;
import android.text.style.LeadingMarginSpan;
import android.text.style.LineHeightSpan;
import android.text.style.StyleSpan;
import android.text.style.TextAppearanceSpan;
import android.text.style.URLSpan;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.QuickContactBadge;
import android.widget.TextView;
import android.widget.Toast;

import com.android.browser.provider.Browser;
import com.android.internal.telephony.PhoneConstants;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.data.Contact;
import com.android.mms.data.WorkingMessage;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.R;
import com.android.mms.transaction.TransactionBundle;
import com.android.mms.util.DownloadManager;
import com.android.mms.util.ItemLoadedCallback;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.android.mms.util.ThumbnailManager.ImageLoaded;
import com.google.android.mms.pdu.PduPart;
import com.google.android.mms.util.SqliteWrapper;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.appserviceproxy.AppServiceProxy;
import com.mediatek.mms.callback.IMessageListItemCallback;
import com.mediatek.mms.ext.IOpMessageListItemExt;
import com.mediatek.mms.ipmessage.IIpMessageItemExt;
import com.mediatek.mms.ipmessage.IIpMessageListItemExt;
import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.mms.util.DrmUtilsEx;
import com.mediatek.mms.util.VCardUtils;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/// @}
/**
 * This class provides view of a message in the messages list.
 */
public class MessageListItem extends LinearLayout implements
 SlideViewInterface, OnClickListener, IMessageListItemCallback {
    public static final String EXTRA_URLS = "com.android.mms.ExtraUrls";

    private static final String TAG = "MessageListItem";
    private static final boolean DEBUG = false;
    private static final boolean DEBUG_DONT_LOAD_IMAGES = false;

    private static final String M_TAG = "Mms/MessageListItem";
    public static final String TAG_DIVIDER = "Mms/divider";

    private static final StyleSpan STYLE_BOLD = new StyleSpan(Typeface.BOLD);
    private static HashMap<Integer, String> mHashSub = new HashMap<Integer, String>();
    private String mDefualtSubName = "SUB01";

    static final int MSG_LIST_EDIT    = 1;
    static final int MSG_LIST_PLAY    = 2;
    static final int MSG_LIST_DETAILS = 3;

    static final int MSG_LIST_EDIT_MMS   = 1;
    static final int MSG_LIST_EDIT_SMS   = 2;
    private static final int PADDING_LEFT_THR = 3;
    private static final int PADDING_LEFT_TWE = 13;

    public static final int text_view = 1;
    public static final int date_view = 2;
    public static final int sim_status = 3;
//    public static final int account_icon = 4;
    public static final int locked_indicator = 5;
    public static final int delivered_indicator = 6;
 //   public static final int details_indicator = 7;
 //   public static final int avatar = 8;
    public static final int message_block = 9;
    public static final int select_check_box = 10;
    public static final int time_divider = 11;
    public static final int time_divider_str = 12;
    public static final int on_line_divider = 15;
    public static final int on_line_divider_str = 16;
    public static final int sim_divider = 17;
    public static final int text_expire = 18;
    public static final int sender_name = 19;
    public static final int sender_name_separator = 20;
    public static final int sender_photo = 21;
 //   public static final int send_time_txt = 22;
 //   public static final int double_time_layout = 23;
    public static final int mms_file_attachment_view_stub = 24;
    public static final int file_attachment_view = 25;
    public static final int file_attachment_thumbnail = 26;
    public static final int file_attachment_name_info = 27;
    public static final int file_attachment_name_info2 = 28;
    public static final int file_attachment_thumbnail2 = 29;
    public static final int file_attachment_size_info = 30;
    public static final int mms_view = 31;
    public static final int mms_layout_view_stub = 32;
    public static final int image_view = 33;
    public static final int play_slideshow_button = 34;
    public static final int mms_downloading_view_stub = 35;
    public static final int btn_download_msg = 36;
    public static final int label_downloading = 37;
    public static final int mms_download_controls = 38;
    public static final int status_panel = 39;


    private View mMmsView;
    /// M: add for vcard
    private View mFileAttachmentView;
    private ImageView mImageView;
    private ImageView mLockedIndicator;
    private ImageView mDeliveredIndicator;
    private ImageButton mSlideShowButton;
    private TextView mBodyTextView;
    private Button mDownloadButton;
    private TextView mDownloadingLabel;
    private Handler mHandler;
    private MessageItem mMessageItem;
    private String mDefaultCountryIso;
    private TextView mDateView;
//    public View mMessageBlock;
    private Path mPathRight;
    private Path mPathLeft;
    private Paint mPaint;
    private boolean mIsLastItemInList;
    static private Drawable sDefaultContactImage;
    private Presenter mPresenter;
    private int mPosition;      // for debugging
    private ImageLoadedCallback mImageLoadedCallback;

    /// M: google JB.MR1 patch, group mms
    private boolean mIsGroupMms;
    /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms
    private QuickContactBadge mSenderPhoto;
    private TextView mSenderName;
    private View mSendNameSeparator;

    private Toast mInvalidContactToast;
    private LeadingMarginSpan mLeadingMarginSpan;
    private long mHandlerTime = 0;
    /// M:
    private static  boolean sImageButtonCanClick = true; // this is a hack for quick click.

    /// M: add for time divider
    private View mTimeDivider; // time_divider
    private TextView mTimeDividerStr; // time_divider_str
    /// M: add for online divider
    private View mOnLineDivider; // on_line_divider
    private TextView mOnLineDividertextView; // on_line_divider_str
    /// M: add for sub message divider
    private View mSubDivider;
    private TextView mExpireText;

    private MessageListAdapter mMessageListAdapter;

    IIpMessageListItemExt mIpMessageListItem;

    IOpMessageListItemExt mOpMessageListItemExt;

    public MessageListItem(Context context, View view) {
        super(context);
        addView(view);
        mDefaultCountryIso = MmsApp.getApplication().getCurrentCountryIso();
        int color = context.getResources().getColor(R.color.timestamp_color);
        mColorSpan = new ForegroundColorSpan(color);
        if (sDefaultContactImage == null) {
            sDefaultContactImage =
                context.getResources().getDrawable(R.drawable.ic_contact_picture);
        }
        initPlugin(this.getContext());
        onFinishInflateView();
    }

    public View findView(int id) {
        return findView(id, null);
    }

    public View findView(int id, View view) {
        View parentView = this;
        if (view != null) {
            parentView = view;
        }
        // add for ipmessage
        View find = mIpMessageListItem.findIpView(id, parentView);
        if (find != null) {
            return find;
        }

        switch (id) {
            case text_view:
                return parentView.findViewById(R.id.text_view);
            case date_view:
                return parentView.findViewById(R.id.date_view);
            case sim_status:
                return parentView.findViewById(R.id.sim_status);
            case locked_indicator:
                return parentView.findViewById(R.id.locked_indicator);
            case delivered_indicator:
                return parentView.findViewById(R.id.delivered_indicator);
        //    case details_indicator:
        //        return parentView.findViewById(R.id.details_indicator);
       //     case avatar:
       //         return parentView.findViewById(R.id.avatar);
            case select_check_box:
                return parentView.findViewById(R.id.select_check_box);
            case time_divider:
                return parentView.findViewById(R.id.time_divider);
            case time_divider_str:
                return parentView.findViewById(R.id.time_divider_str);
            case on_line_divider:
                return parentView.findViewById(R.id.on_line_divider);
            case on_line_divider_str:
                return parentView.findViewById(R.id.on_line_divider_str);
            case sim_divider:
                return parentView.findViewById(R.id.sim_divider);
            case text_expire:
                return parentView.findViewById(R.id.text_expire);
            case sender_name:
                return parentView.findViewById(R.id.sender_name);
            case sender_name_separator:
                return parentView.findViewById(R.id.sender_name_separator);
            case sender_photo:
                return parentView.findViewById(R.id.sender_photo);
     //       case send_time_txt:
     //           return parentView.findViewById(R.id.send_time_txt);
      //      case double_time_layout:
      //          return parentView.findViewById(R.id.double_time_layout);
            case mms_file_attachment_view_stub:
                return parentView.findViewById(R.id.mms_file_attachment_view_stub);
            case file_attachment_view:
                return parentView.findViewById(R.id.file_attachment_view);
            case file_attachment_thumbnail:
                return parentView.findViewById(R.id.file_attachment_thumbnail);
            case file_attachment_name_info:
                return parentView.findViewById(R.id.file_attachment_name_info);
            case file_attachment_name_info2:
                return parentView.findViewById(R.id.file_attachment_name_info2);
            case file_attachment_thumbnail2:
                return parentView.findViewById(R.id.file_attachment_thumbnail2);
            case file_attachment_size_info:
                return parentView.findViewById(R.id.file_attachment_size_info);
            case mms_view:
                return parentView.findViewById(R.id.mms_view);
            case mms_layout_view_stub:
                return parentView.findViewById(R.id.mms_layout_view_stub);
            case image_view:
                return parentView.findViewById(R.id.image_view);
            case play_slideshow_button:
                return parentView.findViewById(R.id.play_slideshow_button);
            case mms_downloading_view_stub:
                return parentView.findViewById(R.id.mms_downloading_view_stub);
            case btn_download_msg:
                return parentView.findViewById(R.id.btn_download_msg);
            case label_downloading:
                return parentView.findViewById(R.id.label_downloading);
            case mms_download_controls:
                return parentView.findViewById(R.id.mms_download_controls);
            case status_panel:
                return parentView.findViewById(R.id.status_panel);
            default:
        }
        return null;
    }

    protected void onFinishInflateView() {
        super.onFinishInflate();

        // add for ipmessage
        mIpMessageListItem.onIpFinishInflate(mContext, mBodyTextView, this, mHandler, this);

        mBodyTextView = (TextView) findView(text_view);
        mDateView = (TextView) findView(date_view);
        /// M: @{
        mSubStatus = (TextView) findView(sim_status);
        /// @}
        mLockedIndicator = (ImageView) findView(locked_indicator);
        mDeliveredIndicator = (ImageView) findView(delivered_indicator);
        /// M: Remove Google default code
        // mMessageBlock = findView(message_block);
        /// M: @{
        //add for multi-delete
        mSelectedBox = (CheckBox) findView(select_check_box);
        /// @}

        /// M: add for time divider
        mTimeDivider = (View) findView(time_divider);
        if (null != mTimeDivider) {
            mTimeDividerStr = (TextView) findView(time_divider_str, mTimeDivider);
        }
        mOnLineDivider = (View) findView(on_line_divider);
        if (null != mOnLineDivider) {
            mOnLineDividertextView = (TextView) findView(on_line_divider_str, mOnLineDivider);
        }
        mSubDivider = (View) findView(sim_divider);
        mExpireText = (TextView) findView(text_expire);
        mOpMessageListItemExt.onFinishInflateView(mExpireText, mSelectedBox, mBodyTextView);
    }

    public void bind(MessageItem msgItem,
            boolean convGroupMms, int position, boolean isDeleteMode) {
        if (msgItem == null) {
            /// M: google jb.mr1 patch, group mms. isLastItem (useless) ? convHasMultiRecipients
            boolean isLastItem = convGroupMms;
            bindDefault(null, isLastItem);
            MmsLog.i(TAG, "bind: msgItem is null, position = " + position);
            return;
        }
        /// M: fix bug ALPS00383381 @{
        if (DEBUG) {
            MmsLog.ipi(TAG, "MessageListItem.bind() : msgItem.mSubId = " + msgItem.mSubId
                + ", position = " + position +
                ", uri = " + msgItem.mMessageUri);
        }
        MmsLog.ipi(TAG, "bind " + this);
        /// @}
        mMessageItem = msgItem;
        mIsGroupMms = convGroupMms;
        mPosition = position;
        /// fix bug ALPS00400536, set null text to avoiding reuse view @{
        mBodyTextView.setText("");
        /// @}

        /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms
        /// set Gone to avoiding reuse view (Visible)
        if (!mMessageItem.isMe()) {
            mSenderName = (TextView) findView(sender_name);
            mSendNameSeparator = findView(sender_name_separator);
            mSenderPhoto = (QuickContactBadge) findView(sender_photo);
            if (mSenderName != null && mSenderPhoto != null && mSendNameSeparator != null) {
                mSenderName.setVisibility(View.GONE);
                mSendNameSeparator.setVisibility(View.GONE);
                mSenderPhoto.setVisibility(View.GONE);
            }
        }

        /// M: @{
        if (isDeleteMode) {
            mSelectedBox.setVisibility(View.VISIBLE);
            if (msgItem.isSelected()) {
                setSelectedBackGroud(true);
            } else {
                setSelectedBackGroud(false);
            }
        } else {
            /// M: change for ALPS01899925, set the background as null if not in delete mode. @{
            if (msgItem.isSubMsg()) {
                setSelectedBackGroud(false);
            }
            /// @}
            mSelectedBox.setVisibility(View.GONE);
        }
        /// M: @{

        setLongClickable(false);
        //set item these two false can make listview always get click event.
        setFocusable(false);
        setClickable(false);

        mContext = msgItem.mContext;

        if (!msgItem.isSubMsg()) {
            bindDividers(msgItem, isDeleteMode);
            // add for ipmessage
            if (mIpMessageListItem.onIpBind(msgItem.mIpMessageItem, msgItem.mMsgId,
                    msgItem.mIpMessageId, isDeleteMode)) {
                return;
            }
        }

        if (mSubDivider != null) {
            if (msgItem.isSubMsg() && (position > 0)) {
                mSubDivider.setVisibility(View.VISIBLE);
            } else {
                mSubDivider.setVisibility(View.GONE);
            }
        }

        switch (msgItem.mMessageType) {
            case MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND:
                bindNotifInd();
                break;
            default:
                bindCommonMessage();
                break;
        }
       // setSubIconAndLabel();
    }

    public void unbind() {
        /* Mem opt*/
        MmsLog.d(TAG, "unbind " + this);
        ///M: fix bug ALPS00383381 @{
    //    MmsLog.ipi(TAG, "unbind() :  " + " position = " + mPosition + "uri = " +
    //            (mMessageItem == null ? "" : mMessageItem.mMessageUri));
        /// @}
        // Clear all references to the message item, which can contain attachments and other
        // memory-intensive objects
        mMessageItem = null;
        if (mContext != null) {
            mContext = mContext.getApplicationContext();
        }
        if (mImageView != null) {
            MmsLog.d(TAG, "unbind1");
            // Because #setOnClickListener may have set the listener to an object that has the
            // message item in its closure.
            mImageView.setOnClickListener(null);
            //recycleBitmap(mImageView);
            mImageView.setImageBitmap(null);
            mImageView = null;
        }
        if (mSlideShowButton != null) {
            // Because #drawPlaybackButton sets the tag to mMessageItem
            mSlideShowButton.setTag(null);
        }
        // leave the presenter in case it's needed when rebound to a different MessageItem.
        if (mPresenter != null) {
            mPresenter.cancelBackgroundLoading();
        }
        mIpMessageListItem.onIpUnbind();
    }

    public MessageItem getMessageItem() {
        return mMessageItem;
    }

    public void setMsgListItemHandler(Handler handler) {
        mHandler = handler;
    }

    private void bindNotifInd() {
        showMmsView(false);
        // add for vcard
        hideFileAttachmentViewIfNeeded();

        /// M: fix bug ALPS00423228, reuse last view when refresh ListView @{
        mDateView.setText("");
        /// @}

        String msgSizeText = mContext.getString(R.string.message_size_label)
                                + String.valueOf((mMessageItem.mMessageSize + 1023) / 1024)
                                + mContext.getString(R.string.kilobyte);

        mBodyTextView.setVisibility(View.VISIBLE);
        mBodyTextView.setText(formatMessage(mMessageItem, null,
                              mMessageItem.mSubject,
                              mMessageItem.mHighlight,
                              mMessageItem.mTextContentType));
        /// M:
        mExpireText.setText(msgSizeText + "\t\n" + mMessageItem.mTimestamp);
        mExpireText.setVisibility(View.VISIBLE);
        MessageUtils.setSubIconAndLabel(mMessageItem.mSubId, null, mSubStatus);
        if (DEBUG) {
            MmsLog.ipi(TAG, "bindNotifInd: uri = " + mMessageItem.mMessageUri +
                    ", position = " + mPosition + ", downloading Status ="
                    + mMessageItem.getMmsDownloadStatus());
        }
        switch (mMessageItem.getMmsDownloadStatus()) {
            case DownloadManager.STATE_DOWNLOADING:
                showDownloadingAttachment();
                /// M: @{
                findView(text_view).setVisibility(GONE);
                /// @}
                break;
            case DownloadManager.STATE_UNKNOWN:
            case DownloadManager.STATE_UNSTARTED:
                /** M: comment this code, this code bug fix is not perfect.
                 * there is other bigger bugs
                DownloadManager downloadManager = DownloadManager.getInstance();
                boolean autoDownload = downloadManager.isAuto();
                boolean dataSuspended = (MmsApp.getApplication().getTelephonyManager()
                        .getDataState() == TelephonyManager.DATA_SUSPENDED);

                // If we're going to automatically start downloading the mms attachment, then
                // don't bother showing the download button for an instant before the actual
                // download begins. Instead, show downloading as taking place.
                if (autoDownload && !dataSuspended) {
                    showDownloadingAttachment();
                    break;
                }
                */
            case DownloadManager.STATE_TRANSIENT_FAILURE:
            case DownloadManager.STATE_PERMANENT_FAILURE:
            default:
                setLongClickable(true);
                inflateDownloadControls();
                mDownloadingLabel.setVisibility(View.GONE);
                mDownloadButton.setVisibility(View.VISIBLE);
                /// M: @{
                findView(text_view).setVisibility(GONE);
                /// @}
                mDownloadButton.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        ///M: When it's not default SMS, do not download
                        if (!MmsConfig.isSmsEnabled(mContext)) {
                            Toast.makeText(mContext,
                                    R.string.download_disabled_toast, Toast.LENGTH_LONG).show();
                            return;
                        }

                        ///M: fix bug ALPS00383381 @{
                        // avoid mMessageItem is already setted null
                        if (mMessageItem == null || mMessageItem.mMessageUri == null) {
                            MmsLog.w(TAG,
                                    "downloadButton onClick, mMessageItem or mMessageUri is null");
                            return;
                        }
                        //@}

                        /// M: @{
                        //add for multi-delete
                        if (mSelectedBox != null && mSelectedBox.getVisibility() == View.VISIBLE) {
                            return;
                        }

                        if (getEnabledSubCount() == 0) {
                            Toast.makeText(mContext,
                                    R.string.no_sim_card, Toast.LENGTH_LONG).show();
                            return;
                        }

                        MmsLog.ipi(TAG, "bindNotifInd: download button onClick: uri = "
                                + mMessageItem.mMessageUri +
                                ", position = " + mPosition);

                        if (mOpMessageListItemExt.onDownloadButtonClick(
                                ComposeMessageActivity.getComposeContext(),
                                MmsConfig.getDeviceStorageFullStatus())) {
                            return;
                        }

                        // add for gemini
                        int subId = 0;
                        // get sub id by uri
                        Cursor cursor = SqliteWrapper.query(mMessageItem.mContext,
                                mMessageItem.mContext.getContentResolver(),
                                mMessageItem.mMessageUri, new String[] {
                                    Telephony.Mms.SUBSCRIPTION_ID
                                }, null, null, null);
                        if (cursor != null) {
                            try {
                                if ((cursor.getCount() == 1) && cursor.moveToFirst()) {
                                    subId = cursor.getInt(0);
                                }
                            } finally {
                                cursor.close();
                            }
                        }

                        /// M: fix bug ALPS00406912
                        mMessageItem.mMmsStatus = DownloadManager.STATE_DOWNLOADING;
                        DownloadManager.getInstance().markState(mMessageItem.mMessageUri,
                                DownloadManager.STATE_DOWNLOADING, subId);
                        if (mMessageListAdapter != null) {
                            mMessageListAdapter.saveNotifIndStatus(mMessageItem.getMessageId(),
                                    DownloadManager.STATE_DOWNLOADING);
                        }

                        mDownloadingLabel.setVisibility(View.VISIBLE);
                        mDownloadButton.setVisibility(View.GONE);
                        Intent intent = new Intent();
                        intent.setClassName(MmsConfig.MMS_APP_SERVICE_PACKAGE,
                                MmsConfig.TRANSACTION_SERVICE);
                        intent.putExtra(TransactionBundle.URI,
                                mMessageItem.mMessageUri.toString());
                        intent.putExtra(TransactionBundle.TRANSACTION_TYPE,
                                TransactionBundle.RETRIEVE_TRANSACTION);
                        // add for gemini
                        intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
                        MmsConfig.setMmsConfig(intent, subId);
                        mContext.startService(intent);
                        /*
                        new Thread(new Runnable() {
                            public void run() {
                                MmsLog.d(MmsApp.TXN_TAG, "MessageListItem.bindNotifInd->onClick: call startServiceByParam");
                                AppServiceProxy.getInstance(mContext).startServiceByParam(mContext, intent);
                            }
                        }).start();
                        */
                    }
                });
                //mtk81083 this is a google default bug. it has no this code!
                // When we show the mDownloadButton, this list item's onItemClickListener doesn't
                // get called. (It gets set in ComposeMessageActivity:
                // mMsgListView.setOnItemClickListener) Here we explicitly set the item's
                // onClickListener. It allows the item to respond to embedded html links and at the
                // same time, allows the button to work.
                setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        onMessageListItemClick();
                    }
                });
                break;
        }

        // Hide the indicators.
        /// M: @{
        //mLockedIndicator.setVisibility(View.GONE);
        if (mMessageItem.mLocked) {
            mLockedIndicator.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_lock_message_sms));
            mLockedIndicator.setVisibility(View.VISIBLE);
            mSubStatus.setPadding(PADDING_LEFT_THR, 0, 0, 0);
        } else {
            mLockedIndicator.setVisibility(View.GONE);
            mSubStatus.setPadding(PADDING_LEFT_TWE, 0, 0, 0);
        }
        /// @}
        mDeliveredIndicator.setVisibility(View.GONE);
        /// M: Remove Google default code
//        updateAvatarView(msgItem.mAddress, false);
        mOpMessageListItemExt.bindMessage(mMessageItem, mSubStatus, mDateView,
                mContext.getResources().getString(R.string.sending_message));

    }

    /// M: google JB.MR1 patch, group mms
    private String buildTimestampLine(String timestamp) {
        if (!mIsGroupMms || mMessageItem.isMe() || TextUtils.isEmpty(mMessageItem.mContact)) {
            // Never show "Me" for messages I sent.
            return timestamp;
        }

        /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms
        if (mSenderName != null && mSenderPhoto != null && mSendNameSeparator != null) {
            mSendNameSeparator.setVisibility(View.VISIBLE);
            mSenderName.setText(mMessageItem.mContact);
            mSenderName.setVisibility(View.VISIBLE);
            Drawable avatarDrawable;
            if (mMessageItem.mGroupContact != null) {
                avatarDrawable =
                    mMessageItem.mGroupContact.getAvatar(mContext, sDefaultContactImage, -1);
            } else {
                avatarDrawable = sDefaultContactImage;
            }
            mSenderPhoto.setImageDrawable(avatarDrawable);
            mSenderPhoto.setVisibility(View.VISIBLE);

            // mSenderPhoto.setClickable(false);
            if (mMessageItem.mGroupContact != null) {
                String number = mMessageItem.mGroupContact.getNumber();
                if (Mms.isEmailAddress(number)) {
                    mSenderPhoto.assignContactFromEmail(number, true);
                } else {
                    if (mMessageItem.mGroupContact.existsInDatabase()) {
                        mSenderPhoto.assignContactUri(mMessageItem.mGroupContact.getUri());
                    } else {
                        mSenderPhoto.assignContactFromPhone(number, true);
                    }
                }
            }
        }

        // This is a group conversation, show the sender's name on the same line as the timestamp.
        return timestamp;
    }

    private void showDownloadingAttachment() {
        inflateDownloadControls();
        mDownloadingLabel.setVisibility(View.VISIBLE);
        mDownloadButton.setVisibility(View.GONE);

        mOpMessageListItemExt.showDownloadingAttachment(mSelectedBox);
    }

    private void bindCommonMessage() {
        if (mDownloadButton != null) {
            mDownloadButton.setVisibility(View.GONE);
            mDownloadingLabel.setVisibility(View.GONE);
            /// M: @{
            mBodyTextView.setVisibility(View.VISIBLE);
            /// @}
        }

        /// M: only mms notifInd view will use and show this.
        if (mExpireText != null) {
            mExpireText.setVisibility(View.GONE);
        }

        // Since the message text should be concatenated with the sender's
        // address(or name), I have to display it here instead of
        // displaying it by the Presenter.
        mBodyTextView.setTransformationMethod(HideReturnsTransformationMethod.getInstance());

        // Get and/or lazily set the formatted message from/on the
        // MessageItem.  Because the MessageItem instances come from a
        // cache (currently of size ~50), the hit rate on avoiding the
        // expensive formatMessage() call is very high.
        CharSequence formattedMessage = mMessageItem.getCachedFormattedMessage();
        /// M: @{
        // CharSequence formattedTimestamp = mMessageItem.getCachedFormattedTimestamp();
//        CharSequence formattedSubStatus = mMessageItem.getCachedFormattedSubStatus();
        /// @}

        if (formattedMessage == null) {
            formattedMessage = formatMessage(mMessageItem,
                                             mMessageItem.mBody,
                                             mMessageItem.mSubject,
                                             mMessageItem.mHighlight,
                                             mMessageItem.mTextContentType);
            /// M: @{
            // formattedTimestamp = formatTimestamp(mMessageItem, mMessageItem.mTimestamp);
//            formattedSubStatus = formatSubStatus(mMessageItem);
//            mMessageItem.setCachedFormattedSubStatus(formattedSubStatus);
            /// @}
            mMessageItem.setCachedFormattedMessage(formattedMessage);
        }
        /// M:
        if (TextUtils.isEmpty(mMessageItem.mBody) && TextUtils.isEmpty(mMessageItem.mSubject)) {
            mBodyTextView.setVisibility(View.GONE);
        } else {
            mBodyTextView.setVisibility(View.VISIBLE);
            mBodyTextView.setText(formattedMessage);
        }

        // Debugging code to put the URI of the image attachment in the body of the list item.
        if (DEBUG) {
            String debugText = null;
            if (mMessageItem.mSlideshow == null) {
                debugText = "NULL slideshow";
            } else {
                SlideModel slide = ((SlideshowModel) mMessageItem.mSlideshow).get(0);
                if (slide == null) {
                    debugText = "NULL first slide";
                } else if (!slide.hasImage()) {
                    debugText = "Not an image";
                } else {
                    debugText = slide.getImage().getUri().toString();
                }
            }
            mBodyTextView.setText(mPosition + ": " + debugText);
        }

        // If we're in the process of sending a message (i.e. pending), then we show a "SENDING..."
        // string in place of the timestamp.
        /// M: @{
        /*mDateView.setText(msgItem.isSending() ?
                mContext.getResources().getString(R.string.sending_message) :
                    buildTimestampLine(msgItem.mTimestamp));
        */
        if (mMessageItem.isFailedMessage()
                || (!mMessageItem.isSending()
                        && TextUtils.isEmpty(mMessageItem.mTimestamp))) {
            mDateView.setVisibility(View.GONE);
        } else {
            mDateView.setVisibility(View.VISIBLE);
            /// M: google jb.mr1 patch, group mms
            String dateStr = mMessageItem.isSending()
                    ? mContext.getResources().getString(R.string.sending_message)
                    : buildTimestampLine(mMessageItem.mTimestamp);
            mDateView.setText(dateStr);
        }
        /// @}

        /// M: @{
        //if (!mMessageItem.isSubMsg() && !TextUtils.isEmpty(formattedSubStatus)) {
        if (!mMessageItem.isSubMsg()) {
            MessageUtils.setSubIconAndLabel(mMessageItem.mSubId, null, mSubStatus);
        } else {
            mSubStatus.setVisibility(View.GONE);
        }
        /// @}

        if (mMessageItem.isSms()) {
            showMmsView(false);
            mMessageItem.setOnPduLoaded(null);
            // add for vcard
            hideFileAttachmentViewIfNeeded();
        } else if (mMessageItem.isMms()) {
            if (DEBUG) {
                MmsLog.vpi(TAG, "bindCommonMessage for item: " + mPosition + " " +
                        mMessageItem.toString() +
                        " mMessageItem.mAttachmentType: " + mMessageItem.mAttachmentType);
            }
            boolean isShowFileAttachmentView = false;
            if (mMessageItem.mAttachmentType != WorkingMessage.TEXT &&
                    mMessageItem.mAttachmentType != MessageItem.ATTACHMENT_TYPE_NOT_LOADED) {
                if (mMessageItem.mAttachmentType == WorkingMessage.ATTACHMENT) {
                    isShowFileAttachmentView = true;
                    showMmsView(false);
                    // show file attachment view
                    showFileAttachmentView(mMessageItem.mSlideshow.getAttachFiles());
                } else {
                    /// M: OP01 add for VCard and VCanlendar
                    if (!mOpMessageListItemExt.showOrHideFileAttachmentView(
                            mMessageItem.mSlideshow.getAttachFiles())) {
                        hideFileAttachmentViewIfNeeded();
                    }
                    setImage(null, null);

                    drawPlaybackButton(mMessageItem);
                    if (mSlideShowButton.getVisibility() == View.GONE) {
                        setMediaOnClickListener(mMessageItem);
                    }
                }
            } else {
                showMmsView(false);
                /// M:  add for VCard and VCanlendar
                hideFileAttachmentViewIfNeeded();
            }
            if (mMessageItem.mSlideshow == null && !isShowFileAttachmentView) {
                mMessageItem.setOnPduLoaded(new MessageItem.PduLoadedCallback() {
                    public void onPduLoaded(MessageItem messageItem) {
                        if (DEBUG) {
                            MmsLog.vpi(TAG, "PduLoadedCallback in MessageListItem for item: "
                                    + mPosition +
                                    " " + (mMessageItem == null ?
                                            "NULL" : mMessageItem.toString()) +
                                    " passed in item: " +
                                    (messageItem == null ? "NULL" : messageItem.toString()));
                        }
                        if (messageItem != null && mMessageItem != null &&
                                messageItem.getMessageId() == mMessageItem.getMessageId()) {
                            mMessageItem.setCachedFormattedMessage(null);
                            bindCommonMessage();
                        }
                    }
                });
            } else {
                if (mPresenter == null) {
                    mPresenter = PresenterFactory.getPresenter(
                    "MmsThumbnailPresenter", mContext,
                    this, mMessageItem.mSlideshow);
                } else {
                    mPresenter.setModel(mMessageItem.mSlideshow);
                    mPresenter.setView(this);
                }
                if (mImageLoadedCallback == null) {
                    mImageLoadedCallback = new ImageLoadedCallback(this);
                } else {
                    mImageLoadedCallback.reset(this);
                }
                mPresenter.present(mImageLoadedCallback);
            }
        }
        drawRightStatusIndicator(mMessageItem);

        requestLayout();

        mOpMessageListItemExt.bindCommonMessage(mDownloadButton, mDateView);
        mOpMessageListItemExt.bindMessage(mMessageItem, mSubStatus, mDateView,
                mContext.getResources().getString(R.string.sending_message));

    }

    static private class ImageLoadedCallback implements ItemLoadedCallback<ImageLoaded> {
        private long mMessageId;
        private final MessageListItem mListItem;

        public ImageLoadedCallback(MessageListItem listItem) {
            mListItem = listItem;
            mMessageId = listItem.getMessageItem().getMessageId();
        }

        public void reset(MessageListItem listItem) {
            mMessageId = listItem.getMessageItem().getMessageId();
        }

        public void onItemLoaded(ImageLoaded imageLoaded, Throwable exception) {
            if (DEBUG_DONT_LOAD_IMAGES) {
                return;
            }
            // Make sure we're still pointing to the same message. The list item could have
            // been recycled.
            MessageItem msgItem = mListItem.mMessageItem;
            if (msgItem != null && msgItem.getMessageId() == mMessageId) {
                if (imageLoaded.mIsVideo) {
                    mListItem.setVideoThumbnail(null, imageLoaded.mBitmap);
                } else {
                    mListItem.setImage(null, imageLoaded.mBitmap);
                }
            }
        }
    }

    @Override
    public void startAudio() {
        // TODO Auto-generated method stub
    }

    @Override
    public void startVideo() {
        // TODO Auto-generated method stub
    }

    @Override
    public void setAudio(Uri audio, String name, Map<String, ?> extras) {
        // TODO Auto-generated method stub
    }

    @Override
    public void setImage(String name, Bitmap bitmap) {
        showMmsView(true);

        try {
            mImageView.setImageBitmap(bitmap);
            mImageView.setVisibility(VISIBLE);
            /// M:
            // add for ipmessage
            mIpMessageListItem.onIpSetMmsImage();
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "setImage: out of memory: ", e);
            MessageUtils.writeHprofDataToFile();
        }
    }

    private void showMmsView(boolean visible) {
        if (mMmsView == null) {
            mMmsView = findView(mms_view);
            // if mMmsView is still null here, that mean the mms section hasn't been inflated

            if (visible && mMmsView == null) {
                //inflate the mms view_stub
                View mmsStub = findView(mms_layout_view_stub);
                mmsStub.setVisibility(View.VISIBLE);
                mMmsView = findView(mms_view);
            }
        }
        if (mMmsView != null) {
            if (mImageView == null) {
                mImageView = (ImageView) findView(image_view);
            }
            if (mSlideShowButton == null) {
                mSlideShowButton = (ImageButton) findView(play_slideshow_button);
            }
            mMmsView.setVisibility(visible ? View.VISIBLE : View.GONE);
            mImageView.setVisibility(visible ? View.VISIBLE : View.GONE);
        }
    }

      private void inflateDownloadControls() {
        if (mDownloadButton == null) {
            //inflate the download controls
            findView(mms_downloading_view_stub).setVisibility(VISIBLE);
            mDownloadButton = (Button) findView(btn_download_msg);
            mDownloadingLabel = (TextView) findView(label_downloading);
            mOpMessageListItemExt.inflateDownloadControls(
                    ComposeMessageActivity.getComposeContext(),
                    (LinearLayout) findView(mms_download_controls),
                    (TextView) findView(text_expire));
        }
    }

    private LineHeightSpan mSpan = new LineHeightSpan() {
        @Override
        public void chooseHeight(CharSequence text, int start,
                int end, int spanstartv, int v, FontMetricsInt fm) {
            fm.ascent -= 10;
        }
    };

    TextAppearanceSpan mTextSmallSpan =
        new TextAppearanceSpan(mContext, android.R.style.TextAppearance_Small);

    ForegroundColorSpan mColorSpan = null;  // set in ctor

    private CharSequence formatMessage(MessageItem msgItem, String body,
                                       String subject, Pattern highlight,
                                       String contentType) {
        SpannableStringBuilder buf = new SpannableStringBuilder();

        boolean hasSubject = !TextUtils.isEmpty(subject);
        if (hasSubject) {
            buf.append(mContext.getResources().getString(R.string.inline_subject, subject));
        }

        if (!TextUtils.isEmpty(body)) {
            // Converts html to spannable if MmsContentType is "text/html".
            if (contentType != null && MmsContentType.TEXT_HTML.equals(contentType)) {
                buf.append("\n");
                buf.append(Html.fromHtml(body));
            } else {
                if (hasSubject) {
                    buf.append(" - ");
                }
                buf.append(body);
            }
        }

        // add for ipmessage
        buf = new SpannableStringBuilder(IpMessageUtils.formatIpMessage(body, true, buf));

        if (highlight != null) {
            Matcher m = highlight.matcher(buf.toString());
            while (m.find()) {
                buf.setSpan(new StyleSpan(Typeface.BOLD), m.start(), m.end(), 0);
            }
        }
        /// M: @{
        buf.setSpan(mLeadingMarginSpan, 0, buf.length(), 0);
        /// @}
        return buf;
    }

    private void drawPlaybackButton(MessageItem msgItem) {
        switch (msgItem.mAttachmentType) {
            case WorkingMessage.SLIDESHOW:
            case WorkingMessage.AUDIO:
            case WorkingMessage.VIDEO:
                updateSlideShowButton(msgItem);
                break;
            case WorkingMessage.IMAGE:
                if (msgItem.mSlideshow.get(0).hasText()) {
                    updateSlideShowButton(msgItem);
                } else {
                    mSlideShowButton.setVisibility(View.GONE);
                }
                break;
            default:
                mSlideShowButton.setVisibility(View.GONE);
                break;
        }
    }

    private void updateSlideShowButton(MessageItem msgItem) {
        // Show the 'Play' button and bind message info on it.
        mSlideShowButton.setTag(msgItem);
        /// M: @{
        mSlideShowButton.setVisibility(View.GONE);

        Bitmap drmBitmap = DrmUtilsEx.getDrmBitmapWithLockIcon(mContext,
                msgItem, R.drawable.mms_play_btn);
        if (drmBitmap != null) {
            mSlideShowButton.setImageBitmap(drmBitmap);
        } else {
            Bitmap bitmap = BitmapFactory.decodeResource(mContext.getResources(),
                                R.drawable.mms_play_btn);
            mSlideShowButton.setImageBitmap(bitmap);
        }
        /// @}

        // Set call-back for the 'Play' button.
        mSlideShowButton.setOnClickListener(this);
        mSlideShowButton.setVisibility(View.VISIBLE);
        setLongClickable(true);

        // When we show the mSlideShowButton, this list item's onItemClickListener doesn't
        // get called. (It gets set in ComposeMessageActivity:
        // mMsgListView.setOnItemClickListener) Here we explicitly set the item's
        // onClickListener. It allows the item to respond to embedded html links and at the
        // same time, allows the slide show play button to work.
        setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                onMessageListItemClick();
            }
        });
    }

    // OnClick Listener for the playback button
    @Override
    public void onClick(View v) {
        /// M: add for multi-delete @{
        if (mSelectedBox != null && mSelectedBox.getVisibility() == View.VISIBLE) {
            return;
        }
        if (!sImageButtonCanClick) {
            return;
        }
        sImageButtonCanClick = false;
        /// @}
        sendMessage(mMessageItem, MSG_LIST_PLAY);
        /// M:
        if (mHandler != null) {
            Runnable run = new Runnable() {
                public void run() {
                    sImageButtonCanClick = true;
                }
            };
            mHandler.postDelayed(run, 1000);
        }
    }

    private void sendMessage(MessageItem messageItem, int message) {
        if (mHandler != null) {
            Message msg = Message.obtain(mHandler, message);
            msg.obj = messageItem;
            msg.sendToTarget(); // See ComposeMessageActivity.mMessageListItemHandler.handleMessage
        }
    }

    public void onMessageListItemClick() {
        if (mMessageItem == null) {
            MmsLog.e(TAG, "onMessageListItemClick(): Message Item is null!");
            return;
        }
        if (mIpMessageListItem.onIpMessageListItemClick()) {
            return;
        }
        /// M: add for multi-delete @{
        if (mSelectedBox != null && mSelectedBox.getVisibility() == View.VISIBLE) {
            if (!mSelectedBox.isChecked()) {
                setSelectedBackGroud(true);
            } else {
                setSelectedBackGroud(false);
            }
            if (null != mHandler) {
                Message msg = Message.obtain(mHandler, ITEM_CLICK);
                msg.arg1 = (int) MessageListAdapter.getKey(mMessageItem.mType,
                        mMessageItem.mMsgId);
                msg.arg2 = mMessageItem.mLocked ?
                    MultiDeleteActivity.MESSAGE_STATUS_LOCK
                    : MultiDeleteActivity.MESSAGE_STATUS_NOT_LOCK;
                msg.obj = mMessageItem;
                msg.sendToTarget();
            }
            return;
        }
        /// @}
        if (mOpMessageListItemExt.onMessageListItemClick()) {
            return;
        }

        // If the message is a failed one, clicking it should reload it in the compose view,
        // regardless of whether it has links in it
        if (mMessageItem != null &&
                (mMessageItem.isOutgoingMessage() &&
                mMessageItem.isFailedMessage())) {
            // Assuming the current message is a failed one, reload it into the
            // compose view so
            // the user can resend it.
            sendMessage(mMessageItem, MSG_LIST_EDIT);
            return;
        }

        // Check for links. If none, do nothing; if 1, open it; if >1, ask user to pick one
        final URLSpan[] spans = mBodyTextView.getUrls();
        /// M: @{
        final java.util.ArrayList<String> urls = MessageUtils.extractUris(spans);
        final String telPrefix = "tel:";
        String url = "";
        boolean isTel = false;
        /// M: fix bug ALPS00367589, uri_size sync according to url after filter to unique array @{
        for (int i = 0; i < urls.size(); i++) {
            url = urls.get(i);
            if (url.startsWith(telPrefix)) {
                isTel = true;
                if (MmsConfig.isSmsEnabled(mContext)) {
                    urls.add("smsto:" + url.substring(telPrefix.length()));
                }
            }
        }
        /// @}
        if (spans.length == 0) {
            sendMessage(mMessageItem, MSG_LIST_DETAILS);    // show the message details dialog
        /// M: @{
        //} else if (spans.length == 1) {
        } else if (spans.length == 1 && !isTel) {
        /// @}
            /*
            Uri uri = Uri.parse(spans[0].getURL());
            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            intent.putExtra(Browser.EXTRA_APPLICATION_ID, mContext.getPackageName());
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
            mContext.startActivity(intent);
            */
            mOpMessageListItemExt.openUrl(mContext, spans[0].getURL());
        /** M: delete google default code @{
        } else if (spans.length == 1) {
            spans[0].onClick(mBodyTextView);
        */
        } else {
            /// M: modify google default Code.@{
            // ArrayAdapter<URLSpan> adapter =
            // new ArrayAdapter<URLSpan>(mContext, android.R.layout.select_dialog_item, spans) {

            ArrayAdapter<String> adapter =
                new ArrayAdapter<String>(mContext, android.R.layout.select_dialog_item, urls) {
              /// @}
                @Override
                public View getView(int position, View convertView, ViewGroup parent) {
                    View v = super.getView(position, convertView, parent);
                    TextView tv = (TextView) v;
                    /// M: move this try into the exact place
                    //try {
                        /// M: modify google default Code @{
                        // URLSpan span = getItem(position);
                        // String url = span.getURL();
                        String url = getItem(position).toString();
                        /// @}
                        Uri uri = Uri.parse(url);

                        final String telPrefix = "tel:";
                        /// M: use default icon to display
                        Drawable d = parseAppIcon(mContext, url);
                        if (d != null) {
                            d.setBounds(0, 0, d.getIntrinsicHeight(), d.getIntrinsicHeight());
                            tv.setCompoundDrawablePadding(10);
                            tv.setCompoundDrawables(d, null, null, null);
                        } else {
                            /// M: currently we only know this one
                            if (url.startsWith(telPrefix)) {
                                d = mContext.getResources().getDrawable(
                                        R.drawable.ic_launcher_phone);
                                d.setBounds(0, 0, d.getIntrinsicHeight(), d.getIntrinsicHeight());
                                tv.setCompoundDrawablePadding(10);
                                tv.setCompoundDrawables(d, null, null, null);
                            } else {
                                tv.setCompoundDrawables(null, null, null, null);
                            }
                        }

                        /// M: @{
                        final String smsPrefix = "smsto:";
                        final String mailPrefix = "mailto";
                        /// @}
                        if (url.startsWith(telPrefix)) {
                            url = PhoneNumberUtils.formatNumber(
                                            url.substring(telPrefix.length()), mDefaultCountryIso);
                            if (url == null) {
                                MmsLog.w(TAG, "url turn to null after calling" +
                                        " PhoneNumberUtils.formatNumber");
                                url = getItem(position).toString().substring(telPrefix.length());
                            }
                        } else if (url.startsWith(smsPrefix)) { /// M: @{
                            url = PhoneNumberUtils.formatNumber(
                                            url.substring(smsPrefix.length()), mDefaultCountryIso);
                            if (url == null) {
                                MmsLog.w(TAG, "url turn to null after" +
                                        " calling PhoneNumberUtils.formatNumber");
                                url = getItem(position).toString().substring(smsPrefix.length());
                            }
                        } else if (url.startsWith(mailPrefix)) {
                            String uu = url.substring(mailPrefix.length() + 1, url.length());
                            uu = Uri.encode(uu);
                            uu = mailPrefix + ":" + uu;
                            MailTo mt = MailTo.parse(uu);
                            url = mt.getTo();
                        }
                        /// @}
                        tv.setText(url);
                    /// M: move this catch to the exact place
                    //} catch (android.content.pm.PackageManager.NameNotFoundException ex) {
                        // it's ok if we're unable to set the drawable for this view - the user
                        // can still use it
                    //    tv.setCompoundDrawables(null, null, null, null);
                    //    return v;
                    //}
                    return v;
                }
            };

            AlertDialog.Builder b = new AlertDialog.Builder(mContext);

            DialogInterface.OnClickListener click = new DialogInterface.OnClickListener() {
                @Override
                public final void onClick(DialogInterface dialog, int which) {
                    if (which >= 0) {
                        /// M: change google default action to choose how to response click  @{
                        //spans[which].onClick(mBodyTextView);

                        Uri uri = Uri.parse(urls.get(which));
                        Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                        intent.putExtra(Browser.EXTRA_APPLICATION_ID, mContext.getPackageName());
                        if (urls.get(which).startsWith("smsto:")) {
                            intent.setClassName(mContext,
                                    "com.android.mms.ui.SendMessageToActivity");
                        }
                        if (!(mContext instanceof Activity)) {
                            MmsLog.d(TAG, "onMessageListItemClick()->uri with non activity");
                            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET|Intent.FLAG_ACTIVITY_NEW_TASK);
                        } else {
                            MmsLog.d(TAG, "onMessageListItemClick()->uri with activity");
                            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                        }
                        mContext.startActivity(intent);
                        /*
                        if (urls.get(which).startsWith("smsto:")) {
                            intent.setClassName(mContext,
                                    "com.android.mms.ui.SendMessageToActivity");
                        }
                        */
                        /// @}

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

    public void showMessageDetail() {
        if (mHandler != null) {
            Message msg = Message.obtain(mHandler, MSG_LIST_DETAILS);
            msg.obj = mMessageItem;
            msg.sendToTarget(); // See
                                // ComposeMessageActivity.mMessageListItemHandler.handleMessage
        }
    }

   private void setOnClickListener(final MessageItem msgItem) {
        switch(msgItem.mAttachmentType) {
            case WorkingMessage.IMAGE:
            case WorkingMessage.VIDEO:
                mImageView.setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        sendMessage(msgItem, MSG_LIST_PLAY);
                    }
                });
                mImageView.setOnLongClickListener(new OnLongClickListener() {
                    @Override
                    public boolean onLongClick(View v) {
                        return v.showContextMenu();
                    }
                });
                break;

            default:
                mImageView.setOnClickListener(null);
                break;
            }
    }

    private void setMediaOnClickListener(final MessageItem msgItem) {
        switch(msgItem.mAttachmentType) {
        case WorkingMessage.IMAGE:
        case WorkingMessage.VIDEO:
            mImageView.setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    /// M: @{
                    //add for multi-delete
                    if (mSelectedBox != null && mSelectedBox.getVisibility() == View.VISIBLE) {
                        mSelectedBox.setChecked(!mSelectedBox.isChecked());

                        if (mSelectedBox.isChecked()) {
                            setSelectedBackGroud(true);
                        } else {
                            setSelectedBackGroud(false);
                        }

                        if (null != mHandler) {
                            Message msg = Message.obtain(mHandler, ITEM_CLICK);
                            msg.arg1 = (int) (mMessageItem.mType.equals("mms") ?
                                    -mMessageItem.mMsgId : mMessageItem.mMsgId);
                            msg.arg2 = mMessageItem.mLocked ?
                                    MultiDeleteActivity.MESSAGE_STATUS_LOCK
                                        : MultiDeleteActivity.MESSAGE_STATUS_NOT_LOCK;
                            msg.obj = mMessageItem;
                            msg.sendToTarget();
                        }
                        return;
                    }
                    /// @}
                    if (!sImageButtonCanClick) {
                        return;
                    }
                    sImageButtonCanClick = false;
                    /// M: @{
                    if (msgItem.mAttachmentType == WorkingMessage.IMAGE
                            && msgItem.mSlideshow.get(0).hasText()) {
                        mImageView.setOnClickListener(null);
                    } else {
                        sendMessage(msgItem, MSG_LIST_PLAY);
                    }
                    if (mHandler != null) {
                        Runnable run = new Runnable() {
                            public void run() {
                                sImageButtonCanClick = true;
                            }
                        };
                        mHandler.postDelayed(run, 1000);
                    }
                    /// @}
                }
            });
            mImageView.setOnLongClickListener(new OnLongClickListener() {
                @Override
                public boolean onLongClick(View v) {
                    return v.showContextMenu();
                }
            });
            break;

        default:
            mImageView.setOnClickListener(null);
            break;
        }
    }

    private void drawRightStatusIndicator(MessageItem msgItem) {
        // Locked icon
        if (msgItem.mLocked) {
            mLockedIndicator.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_lock_message_sms));
            mLockedIndicator.setVisibility(View.VISIBLE);
            mSubStatus.setPadding(PADDING_LEFT_THR, 0, 0, 0);
        } else {
            mLockedIndicator.setVisibility(View.GONE);
            mSubStatus.setPadding(PADDING_LEFT_TWE, 0, 0, 0);
        }
        if (msgItem.isOutgoingMessage() && msgItem.isFailedMessage()) {
            mDeliveredIndicator.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.ic_list_alert_sms_failed));
            mDeliveredIndicator.setVisibility(View.VISIBLE);
        } else if (msgItem.isSms() &&
                msgItem.mDeliveryStatus == MessageItem.DeliveryStatus.RECEIVED) {
            /// M: @{
            mDeliveredIndicator.setClickable(false);
            /// @}
            ///M: modified for new icon
//            mDeliveredIndicator.setImageResource(R.drawable.ic_sms_mms_delivered);
            mDeliveredIndicator.setImageDrawable(mContext.getResources().getDrawable(
                    R.drawable.im_meg_status_reach));
            mDeliveredIndicator.setVisibility(View.VISIBLE);
        } else {
            /// M: Add new status icon for MMS or SMS. @{
            int resId = MessageUtils.getStatusResourceId(mContext, msgItem);
            if (resId > 0) {
                mDeliveredIndicator.setClickable(false);
                mDeliveredIndicator.setImageDrawable(mContext.getResources().getDrawable(resId));
//                mDeliveredIndicator.setImageResource(resId);
                mDeliveredIndicator.setVisibility(View.VISIBLE);
            } else {
                mDeliveredIndicator.setVisibility(View.GONE);
            }
            /// @}
        }
    }

    @Override
    public void setImageRegionFit(String fit) {
        // TODO Auto-generated method stub
    }

    @Override
    public void setImageVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    @Override
    public void setText(String name, String text) {
        // TODO Auto-generated method stub
    }

    @Override
    public void setTextVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    @Override
    public void setVideo(String name, Uri uri) {
    }

    @Override
    public void setVideoThumbnail(String name, Bitmap bitmap) {
        showMmsView(true);

        try {
            mImageView.setImageBitmap(bitmap);
            mImageView.setVisibility(VISIBLE);
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "setVideo: out of memory: ", e);
        }
    }

    @Override
    public void setVideoVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    @Override
    public void stopAudio() {
        // TODO Auto-generated method stub
    }

    @Override
    public void stopVideo() {
        // TODO Auto-generated method stub
    }

    @Override
    public void reset() {
    }

    @Override
    public void setVisibility(boolean visible) {
        // TODO Auto-generated method stub
    }

    @Override
    public void pauseAudio() {
        // TODO Auto-generated method stub

    }

    @Override
    public void pauseVideo() {
        // TODO Auto-generated method stub

    }

    @Override
    public void seekAudio(int seekTo) {
        // TODO Auto-generated method stub

    }

    @Override
    public void seekVideo(int seekTo) {
        // TODO Auto-generated method stub

    }

    /// M: @{
    public static final int ITEM_CLICK          = 5;
    static final int ITEM_MARGIN         = 50;
    private TextView mSubStatus;
    public CheckBox mSelectedBox;

    private CharSequence formatTimestamp(MessageItem msgItem, String timestamp) {
        SpannableStringBuilder buf = new SpannableStringBuilder();
        if (msgItem.isSending()) {
            timestamp = mContext.getResources().getString(R.string.sending_message);
        }

           buf.append(TextUtils.isEmpty(timestamp) ? " " : timestamp);
           buf.setSpan(mSpan, 1, buf.length(), 0);

        //buf.setSpan(mTextSmallSpan, 0, buf.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        // Make the timestamp text not as dark
        buf.setSpan(mColorSpan, 0, buf.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

        return buf;
    }

//    private CharSequence formatSubStatus(MessageItem msgItem) {
//        SpannableStringBuilder buffer = new SpannableStringBuilder();
//        // If we're in the process of sending a message, then we show a "Sending..."
//        // string in place of the timestamp.
//        //Add sub info
//        CharSequence subInfo = MessageUtils.getSubInfo(mContext, msgItem.mSubId);
//        Log.i(TAG,"formatSubStatus::subInfo="+subInfo);
//        if (subInfo.length() > 0) {
//            buffer.append(subInfo);
//        }
//        int subInfoStart = buffer.length();
//
//        //buffer.setSpan(mTextSmallSpan, 0, buffer.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
//        // Make the timestamp text not as dark
//        buffer.setSpan(mColorSpan, 0, subInfoStart, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
//
//        return buffer;
//    }

    public void setSelectedBackGroud(boolean selected) {
        if (selected) {
            mSelectedBox.setChecked(true);
//            mSelectedBox.setBackgroundDrawable(null);
//            mMessageBlock.setBackgroundDrawable(null);
//            mDateView.setBackgroundDrawable(null);
        } else {
              mSelectedBox.setChecked(false);
//            mSelectedBox.setBackgroundResource(R.drawable.listitem_background);
//            mMessageBlock.setBackgroundResource(R.drawable.listitem_background);
//            mDateView.setBackgroundResource(R.drawable.listitem_background);
        }
        setBackgroundDrawable(null);
    }

    public void bindDefault(MessageItem msgItem, boolean isLastItem) {
        MmsLog.d(M_TAG, "bindDefault()");
        mIsLastItemInList = isLastItem;
        mSelectedBox.setVisibility(View.GONE);
        setLongClickable(false);
        setFocusable(false);
        setClickable(false);
        String msgType = "";
        if (msgItem != null) {
            msgType = msgItem.mType;
        }
        if (mMmsView != null) {
            mMmsView.setVisibility(View.GONE);
        }
        if (mFileAttachmentView != null) {
            mFileAttachmentView.setVisibility(View.GONE);
        }
        /// M: fix bug ALPS00435619, show Refreshing string
        /// when we fail to reload item and put in cache during 500 ms
        mBodyTextView.setVisibility(View.VISIBLE);
        // M:for ALPS01065027,just for compose sms messagelist use in scrolling
        if ("sms".equals(msgType)) {
            CharSequence formattedMessage = formatMessage(msgItem, msgItem.mBody, msgItem.mSubject,
                    msgItem.mHighlight, msgItem.mTextContentType);
            msgItem.setCachedFormattedMessage(formattedMessage);

            if (TextUtils.isEmpty(msgItem.mBody) && TextUtils.isEmpty(msgItem.mSubject)) {
                mBodyTextView.setVisibility(View.GONE);
            } else {
                mBodyTextView.setText(formattedMessage);
            }
        } else {
            mBodyTextView.setText(this.getContext().getString(R.string.refreshing));
        }

        // if (null != mTimeDivider) {
        // mTimeDivider.setVisibility(View.GONE);
        // }
        //

        // mDateView.setVisibility(View.GONE);
        // mSimStatus.setVisibility(View.GONE);
        if (mDownloadButton != null) {
            mDownloadingLabel.setVisibility(View.GONE);
            mDownloadButton.setVisibility(View.GONE);
        }

        mOpMessageListItemExt.bindDefault(mDownloadButton);

        mLockedIndicator.setVisibility(View.GONE);
        mSubStatus.setPadding(PADDING_LEFT_TWE, 0, 0, 0);
//        CharSequence formattedSubStatus = "";
        if (msgItem != null) {
            if (DEBUG) {
                MmsLog.d(TAG, "message item is not null");
            }
//            formattedSubStatus = msgItem.getCachedFormattedSubStatus();
            //if (!msgItem.isSubMsg() && !TextUtils.isEmpty(formattedSubStatus)) {
            if (!msgItem.isSubMsg() && mMessageItem != null) {
                MessageUtils.setSubIconAndLabel(mMessageItem.mSubId, null, mSubStatus);
            }
        } else {
            if (DEBUG) {
                MmsLog.w(TAG, "message item is null");
            }
        }
        // mDeliveredIndicator.setVisibility(View.GONE);

        /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms
        /// set Gone in bindDefault()
        if (mMessageItem != null && !mMessageItem.isMe()) {
            mSenderName = (TextView) findView(sender_name);
            mSendNameSeparator = findView(sender_name_separator);
            mSenderPhoto = (QuickContactBadge) findView(sender_photo);
            if (mSenderName != null && mSenderPhoto != null && mSendNameSeparator != null) {
                mSenderName.setVisibility(View.GONE);
                mSendNameSeparator.setVisibility(View.GONE);
                mSenderPhoto.setVisibility(View.GONE);
            }
        }
        if (mOnLineDivider != null) {
            mOnLineDivider.setVisibility(View.GONE);
        }
        /// M: add for ALPS01918645, hide deliver status and time because they contains old data.
        if (mDeliveredIndicator != null) {
            mDeliveredIndicator.setVisibility(View.GONE);
        }
        if (mDateView != null) {
            mDateView.setVisibility(View.GONE);
        }
        /// @}
        // add for ipmessage
        //mIpMessageListItem.onIpBindDefault();

        IIpMessageItemExt ipMsgItem = null;
        if (msgItem != null) {
            ipMsgItem = msgItem.mIpMessageItem;
        }
        mIpMessageListItem.onIpBindDefault(ipMsgItem);

        requestLayout();
    }
    /// @}

    ///M: add for adjust text size
    public void setBodyTextSize(float size) {
        if (mBodyTextView != null && mBodyTextView.getVisibility() == View.VISIBLE) {
            mBodyTextView.setTextSize(size);
        }
        // add for ipmessage
        mIpMessageListItem.onIpSetBodyTextSize(size);
    }

    @Override
    public void setImage(Uri mUri) {
        try {
            Bitmap bitmap = null;
            if (null == mUri) {
                bitmap = BitmapFactory.decodeResource(getResources(),
                        R.drawable.ic_missing_thumbnail_picture);
            } else {
                InputStream mInputStream = null;
                try {
                    mInputStream = this.getContext().getContentResolver().openInputStream(mUri);
                    if (mInputStream != null) {
                        bitmap = BitmapFactory.decodeStream(mInputStream);
                    }
                } catch (FileNotFoundException e) {
                    bitmap = null;
                } finally {
                    if (mInputStream != null) {
                        mInputStream.close();
                    }
                }
                setImage("", bitmap);
            }
        } catch (java.lang.OutOfMemoryError e) {
            Log.e(TAG, "setImage(Uri): out of memory: ", e);
        } catch (IOException e) {
            Log.e(TAG, "mInputStream.close() IOException." + e);
        }
    }

    // Add for vCard begin
    private void hideFileAttachmentViewIfNeeded() {
        if (mFileAttachmentView != null) {
            mFileAttachmentView.setVisibility(View.GONE);
        }
    }

    private void showFileAttachmentView(ArrayList<FileAttachmentModel> files) {
        // There should be one and only one file
        if (files == null || files.size() < 1) {
            Log.e(TAG, "showFileAttachmentView, oops no attachment files found");
            return;
        }
        final int filesize = files.size();
        ArrayList<FileAttachmentModel> mfiles = files;

        if (mFileAttachmentView == null) {
            findView(mms_file_attachment_view_stub).setVisibility(VISIBLE);
            mFileAttachmentView = findView(file_attachment_view);
        }
        mFileAttachmentView.setVisibility(View.VISIBLE);

        final FileAttachmentModel attach = files.get(0);
        mFileAttachmentView.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (mSelectedBox != null && mSelectedBox.getVisibility() == View.VISIBLE) {
                    return;
                }
                if (mOpMessageListItemExt.onFileAttachmentViewClick(
                        attach.isVCard(), attach.getSrc())) {
                    return;
                }
                if (attach.isVCard()) {
                    VCardUtils.importVCard(mContext, attach);
                } else if (attach.isVCalendar()) {
                    try {
                        Intent intent = new Intent(Intent.ACTION_VIEW);
                        intent.setDataAndType(attach.getUri(),
                                attach.getContentType().toLowerCase());
                        if (!(mContext instanceof Activity)) {
                            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION|Intent.FLAG_ACTIVITY_NEW_TASK);
                        } else {
                            intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                        }
                        mContext.startActivity(intent);
                    } catch (ActivityNotFoundException e) {
                        MmsLog.e(TAG, "no activity handle ", e);
                    }
                }
            }
        });

        ImageView thumb = (ImageView) findView(file_attachment_thumbnail, mFileAttachmentView);
        TextView name = (TextView) findView(file_attachment_name_info, mFileAttachmentView);
        TextView name2 = (TextView) findView(file_attachment_name_info2, mFileAttachmentView);
        String nameText = "";
        int thumbResId = -1;
        if (attach.isVCard()) {
            nameText = mContext.getString(R.string.file_attachment_vcard_name, attach.getSrc());
            thumbResId = R.drawable.ic_vcard_attach;
        } else if (attach.isVCalendar()) {
            nameText = mContext.getString(R.string.file_attachment_vcalendar_name,
                    attach.getSrc());
            thumbResId = R.drawable.ic_vcalendar_attach;
        }

        name.setText(nameText);

        if (thumbResId != -1) {
            thumb.setImageDrawable(mContext.getResources().getDrawable(thumbResId));
        }
        final TextView size = (TextView) findView(file_attachment_size_info, mFileAttachmentView);
        size.setText(MessageUtils.getHumanReadableSize(attach.getAttachSize()));

        mOpMessageListItemExt.showFileAttachmentView(
                mFileAttachmentView, name2, name, thumb, size, files,
                new Intent(mContext, MultiSaveActivity.class), mMessageItem.mMsgId, attach,
                attach.mOpFileAttachmentModelExt);
        /// @}
    }
    // Add for vCard end

  private boolean copyPart(PduPart part, String filename) {
      Uri uri = part.getDataUri();
      MmsLog.ipi(TAG, "copyPart, copy part into sdcard uri " + uri);

      InputStream input = null;
      FileOutputStream fout = null;
      try {
          ContentResolver mContentResolver = mContext.getContentResolver();
          input = mContentResolver.openInputStream(uri);
          if (input instanceof FileInputStream) {
              FileInputStream fin = (FileInputStream) input;
              // Depending on the location, there may be an
              // extension already on the name or not
              String dir = "";
              File file = MessageUtils.getStorageFile(filename, mContext.getApplicationContext());
              if (file == null) {
                  return false;
              }
              fout = new FileOutputStream(file);
              byte[] buffer = new byte[8000];
              int size = 0;
              while ((size = fin.read(buffer)) != -1) {
                  fout.write(buffer, 0, size);
              }

              // Notify other applications listening to scanner events
              // that a media file has been added to the sd card
              mContext.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE,
                      Uri.fromFile(file)));
          }
      } catch (IOException e) {
          // Ignore
          MmsLog.e(TAG, "IOException caught while opening or reading stream", e);
          return false;
      } finally {
          if (null != input) {
              try {
                  input.close();
              } catch (IOException e) {
                  // Ignore
                  MmsLog.e(TAG, "IOException caught while closing stream", e);
                  return false;
              }
          }
          if (null != fout) {
              try {
                  fout.close();
              } catch (IOException e) {
                  // Ignore
                  MmsLog.e(TAG, "IOException caught while closing stream", e);
                  return false;
              }
          }
      }
      return true;
  }

    private void bindDividers(MessageItem msgItem, boolean isDeleteMode) {
        if (null != mTimeDivider) {
            if (msgItem.mIsDrawTimeDivider) {
                mTimeDivider.setVisibility(View.VISIBLE);
                mTimeDividerStr.setText(msgItem.mTimeDividerString);
            } else {
                mTimeDivider.setVisibility(View.GONE);
            }
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        MmsLog.d(M_TAG, "onDetachedFromWindow " + this);
        /*if (mContext != null) {
            mContext = mContext.getApplicationContext();
        }*/
        // add for ipmessage
        mIpMessageListItem.onIpDetachedFromWindow();
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        MmsLog.d(M_TAG, "onAttachedToWindow " + this);
    }

    public void setMessageListItemAdapter(MessageListAdapter adapter) {
        mMessageListAdapter = adapter;
        mIpMessageListItem.setIpMessageListItemAdapter(mMessageListAdapter.mIpMessageListAdapter);
    }

    /**
     * Init Plugin
     *
     * @param context
     */
    private void initPlugin(Context context) {
        // add for ipmessage
        mIpMessageListItem = IpMessageUtils.getIpMessagePlugin(context).getIpMessageListItem();
        mOpMessageListItemExt = OpMessageUtils.getOpMessagePlugin().getOpMessageListItemExt();
        mOpMessageListItemExt.init(this, context);
    }

    /**
     * M: BUGFIX: ALPS00515336; Hide all view
     */
    public void hideAllView() {
        MmsLog.d(M_TAG, "hideAllView()");
        mSelectedBox.setVisibility(View.GONE);
        setLongClickable(false);
        setFocusable(false);
        setClickable(false);

        if (mMmsView != null) {
            mMmsView.setVisibility(View.GONE);
        }
        if (mFileAttachmentView != null) {
            mFileAttachmentView.setVisibility(View.GONE);
        }
        /// when we fail to reload item and put in cache during 500 ms
        mBodyTextView.setVisibility(View.GONE);

        if (null != mTimeDivider) {
            mTimeDivider.setVisibility(View.GONE);
        }

        mDateView.setVisibility(View.GONE);
        mSubStatus.setVisibility(View.GONE);
        if (mDownloadButton != null) {
            mDownloadingLabel.setVisibility(View.GONE);
            mDownloadButton.setVisibility(View.GONE);
            mOpMessageListItemExt.hideAllView();
        }
        mLockedIndicator.setVisibility(View.GONE);
        mSubStatus.setVisibility(View.GONE);
        mDeliveredIndicator.setVisibility(View.GONE);

        /// MTK MR1 new feature: Group Mms
        /// set Gone in bindDefault()
        if (mMessageItem != null && !mMessageItem.isMe()) {
            mSenderName = (TextView) findView(sender_name);
            if (mSenderName != null) {
                mSenderName.setVisibility(View.GONE);
            }
            mSendNameSeparator = findView(sender_name_separator);
            if (mSendNameSeparator != null) {
                mSendNameSeparator.setVisibility(View.GONE);
            }
            mSenderPhoto = (QuickContactBadge) findView(sender_photo);
            if (mSenderPhoto != null) {
                mSenderPhoto.setVisibility(View.GONE);
            }
        }
        requestLayout();
    }

    /**
     * M: Use default icon to display
     */
    private Drawable parseAppIcon(Context context, String url) {
        final String telPrefix = "tel:";
        final String smsPrefix = "smsto:";
        final String mailPrefix = "mailto";
        int drawableId;

        if (url.startsWith(telPrefix)) {
            drawableId = R.drawable.common_phone;
        } else if (url.startsWith(smsPrefix)) {
            drawableId = R.drawable.common_message;
        } else if (url.startsWith(mailPrefix)) {
            drawableId = R.drawable.common_email;
        } else {
            drawableId = R.drawable.common_browser;
        }
        return context.getResources().getDrawable(drawableId);
    }

    // Add for IpMessage callback

    public void setSubDateView(String subName) {
        MessageUtils.setSubIconAndLabel(mMessageItem.mSubId, subName, mSubStatus);

        if (mMessageItem.isFailedMessage()
                || (!mMessageItem.isSending() && TextUtils.isEmpty(mMessageItem.mTimestamp))) {
            mDateView.setVisibility(View.GONE);
        } else {
            mDateView.setVisibility(View.VISIBLE);
            // / M: google jb.mr1 patch, group mms
            mDateView.setText(mMessageItem.isSending() ? mContext.getResources().getString(
                    R.string.sending_message) : buildTimestampLine(mMessageItem.mTimestamp));
        }
        drawRightStatusIndicator(mMessageItem);
        requestLayout();
    }

    public void setTextMessage(String body) {
        CharSequence formattedMessage = mMessageItem.getCachedFormattedMessage();
        if (formattedMessage == null) {
            formattedMessage = formatMessage(mMessageItem, body, null, mMessageItem.mHighlight,
                    null);
            mMessageItem.setCachedFormattedMessage(formattedMessage);
        }
        mBodyTextView.setVisibility(View.VISIBLE);
        mBodyTextView.setText(formattedMessage);
    }

    public void updateMessageItemState(long msgId) {
        if (mMessageItem.mIpMessageId <= 0 || mMessageItem.mMsgId != msgId) {
            return;
        }
        final long messageId = msgId;
        ((Activity) mContext).runOnUiThread(new Runnable() {
            @Override
            public void run() {
                boolean deleteMode = mMessageListAdapter == null ? false
                        : mMessageListAdapter.mIsDeleteMode;
                if (deleteMode) {
                    long id = mMessageListAdapter.getKey("sms", messageId);
                    if (mMessageListAdapter.getItemList().get(id) == null) {
                        mMessageListAdapter.getItemList().put(id, false);
                    } else {
                        mMessageItem.setSelectedState(mMessageListAdapter.getItemList().get(id));
                    }
                }
                bind(mMessageItem, false, 0, deleteMode);
            }
        });
    }

    public void showFileAttachmentViewCallback(ArrayList files) {
        showFileAttachmentView((ArrayList<FileAttachmentModel>) files);
    }

    public void hideFileAttachmentViewIfNeededCallback() {
        hideFileAttachmentViewIfNeeded();
    }

    public File getStorageFileCallback(String fileName, Context context) {
        return MessageUtils.getStorageFile(fileName, context);
    }

    public String getHumanReadableSizeCallback(int size) {
        return MessageUtils.getHumanReadableSize(size);
    }

    public boolean copyPartCallback(PduPart part, String filename) {
        return copyPart(part, filename);
    }

    @Override
    public String buildTimestampLineCallback(String time) {
        return buildTimestampLine(time);
    }

    private int getEnabledSubCount() {
        List<SubscriptionInfo> subInfoList = SubscriptionManager.from(mContext).getActiveSubscriptionInfoList();
        int subCount = (subInfoList != null && !subInfoList.isEmpty()) ? subInfoList.size() : 0;
        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();

        if (simOnOffEnabled == false) {
            return subCount;
        }

        if (subInfoList == null || subCount == 0) {
            return subCount;
        }

        int enabledSubCount = 0;
        for (int i = 0; i < subCount; i++) {
            SubscriptionInfo subInfo = subInfoList.get(i);
            if (subInfo != null) {
                int slotId = subInfo.getSimSlotIndex();
                int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    enabledSubCount++;
                }
            }
        }
        return enabledSubCount;
    }
}
