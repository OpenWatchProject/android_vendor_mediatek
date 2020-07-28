/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.op18.mms;

import android.content.Context;
import android.database.Cursor;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Profile;
import android.provider.Telephony.Mms;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup.LayoutParams;

import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.QuickContactBadge;
import android.widget.TextView;

import com.mediatek.contacts.common.ContactPhotoManager;
import com.mediatek.contacts.common.ContactPhotoManager.DefaultImageRequest;
import com.android.mms.MmsApp;
import com.android.mms.data.Contact;
import com.android.mms.ui.MessageUtils;

import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.MultimediaMessagePdu;
import com.google.android.mms.pdu.PduHeaders;
import com.google.android.mms.pdu.PduPersister;
import com.google.android.mms.pdu.RetrieveConf;
import com.google.android.mms.pdu.SendReq;

import com.mediatek.mms.callback.IMessageItemCallback;
import com.mediatek.mms.callback.IMessageListItemCallback;
import com.mediatek.mms.ext.DefaultOpMessageListItemExt;

import com.mediatek.op18.mms.R;


/**
 * Op01MessageListItemExt.
 *
 */
public class Op18MessageListItemExt extends DefaultOpMessageListItemExt {
    private static final String TAG = "Mms/Op18MmsMessageListItemExt";

    /// M: New plugin API @{

    static private Drawable sDefaultContactImage;

    private static final int MSG_RETRIEVE_FAILURE_DEVICE_MEMORY_FULL = 2;
    private Context mContext = null;
    private Context mHostContext = null;
    private IMessageListItemCallback mMessageListItem;
    private QuickContactBadge mSenderPhoto;
    private LinearLayout mlayout;
    private LinearLayout mlistitem;
    private TextView mNetInfo;
    private TextView mDateTime;
    private ImageView mNetImg;
    private String mAddress;
    private Contact mGroupContact;
    private View senderImage = null;
    private LinearLayout mOpLayout;
    private LinearLayout mIconLayout;


    /**
     * Construction.
     * @param context Context
     */
    public Op18MessageListItemExt(Context context) {
        super(context);
        mContext = context;
    }

    @Override
    public void init(IMessageListItemCallback messageListItem, Context context) {
        Log.d(TAG, "init messagelist item plugin");
        mMessageListItem = messageListItem;
        mHostContext = context;
    }

    @Override
    public void onFinishInflateView(TextView expireText, CheckBox selectBox,
                                  TextView bodyTextView) {
        Log.d(TAG, "[viney] add new components");

        // @viney Add changes to add layout for the sent messages:

        // adding the sender picture

        mlayout = (LinearLayout) selectBox.getParent();
        if (mlayout != null) {
            mSenderPhoto = (QuickContactBadge) new QuickContactBadge(mHostContext);
            // set the parameters

            // set the padding for message text as zero
            //bodyTextView.setPadding(0, 0, 0, 0);

            mSenderPhoto.setImageResource(R.drawable.ic_default_contact);
            if (expireText != null) {
                mlayout.addView(mSenderPhoto, 1);
                mlayout.setPadding(35, 0, 0, 0);
                // find the avatar image
                senderImage = (View) mlayout.getChildAt(2);
            } else {
                mlayout.addView(mSenderPhoto);
                mlayout.setPadding(0, 0, 35, 0);
            }
        }

        // adding the sender network name
        LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT,
                                   LayoutParams.WRAP_CONTENT);

        mNetInfo = (TextView) new TextView(mContext);
        mNetInfo.setTextSize(2, 14);
        mNetInfo.setLines(1);
        mNetInfo.setLayoutParams(params);

        mlistitem = (LinearLayout) bodyTextView.getParent();

        // create a new layout to add the new info
        // add image for the network icon

        mNetImg = (ImageView) new ImageView(mContext);

        // add textview for the date time
        mDateTime = (TextView) new TextView(mContext);
        mDateTime.setTextSize(2, 14);
        mDateTime.setLines(1);
        mDateTime.setTextColor(Color.BLACK);
        mDateTime.setLayoutParams(params);

        // create layout to add date time & sim icon
        mIconLayout = (LinearLayout) new LinearLayout(mContext);
        mIconLayout.setOrientation(0);
        mIconLayout.setGravity(Gravity.LEFT);
        mIconLayout.setLayoutParams(params);
        mIconLayout.addView(mNetImg);
        mIconLayout.addView(mDateTime,
                       new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));

        // create second layout to add operator name as well.
        mOpLayout = (LinearLayout) new LinearLayout(mContext);
        mOpLayout.setOrientation(1);
        mOpLayout.setLayoutParams(params);
        mOpLayout.addView(mIconLayout);
        mOpLayout.addView(mNetInfo);
        mOpLayout.setPadding(15, 0, 0, 0);
        mlistitem.addView(mOpLayout, 0);
        mlistitem.setBackgroundColor(0xf0f0f0);

    }


    @Override
    public void bindMessage(IMessageItemCallback msgItem, TextView subStatus, TextView dateView,
                                         String sendingStr) {
        Drawable avatarDrawable;
        Uri photoUri = null;
        Uri msgUri = msgItem.getMessageUri();

        dateView.setVisibility(View.GONE);
        subStatus.setVisibility(View.GONE);
        // add the subtext for operator name
        MessageUtils.setSubIconAndLabel(msgItem.getSubId(), null, mNetInfo);

        // get subsciption info

        int activeSubCount = SubscriptionManager.from(MmsApp.getApplication())
                        .getActiveSubscriptionInfoCount();
        int subId = msgItem.getSubId();
        boolean submsg = msgItem.getIsSubMessage();
        Log.d(TAG, "submsg " + submsg);

        mNetImg.setVisibility(View.GONE);
        if (activeSubCount > 1) {
            SubscriptionInfo subInfo = SubscriptionManager.from(MmsApp.getApplication())
                    .getActiveSubscriptionInfo(subId);
            Log.d(TAG, "subInfo=" + subInfo);
            if (null != subInfo) {
                if ((subInfo.getSimSlotIndex() == SubscriptionManager.SIM_NOT_INSERTED)
                  || (subInfo.getSimSlotIndex() == SubscriptionManager.INVALID_SUBSCRIPTION_ID)) {
                    Log.d(TAG, "current not insert sim card");
                } else {
                    //set icon
                    mNetImg.setBackgroundDrawable(new BitmapDrawable(mContext.getResources(),
                                      subInfo.createIconBitmap(mContext)));
                    mNetImg.setVisibility(View.VISIBLE);
                }
            } else {
                Log.d(TAG, "subInfo is null ");
                mNetImg.setVisibility(View.GONE);
            }
        }

        sDefaultContactImage = mContext.getResources().getDrawable(R.drawable.ic_default_contact);
        avatarDrawable = sDefaultContactImage;

        // set the avatar picture
        mSenderPhoto.setOverlay(null);

        if (msgItem.isReceivedMessage() == false) {
            Log.d(TAG, "[viney] sent message");
            mGroupContact = Contact.getMe(false);
            mSenderPhoto.assignContactUri(Profile.CONTENT_URI);
            photoUri = Uri.withAppendedPath(Profile.CONTENT_URI, Contacts.Photo.CONTENT_DIRECTORY);
            photoUri = mGroupContact.getPhotoUri();
            Log.d(TAG, "photoUri=" + photoUri);
            avatarDrawable = mGroupContact.getAvatar(mContext, sDefaultContactImage, -1);

            if (avatarDrawable != sDefaultContactImage) {
                try {
                    photoUri = Uri.parse("content://com.android.contacts/profile/display_photo");
                    Log.d(TAG, "photoUri=" + photoUri);
                    ContactPhotoManager contactPhotoManager =
                        ContactPhotoManager.getInstance(mContext);
                    final DefaultImageRequest request = new DefaultImageRequest(null, null,
                             true /* isCircular */);
                    Log.d(TAG, "request=" + request);
                    contactPhotoManager.loadPhoto(mSenderPhoto, photoUri,
                                sDefaultContactImage.getIntrinsicHeight(), false, true, request,
                                                               ContactPhotoManager.DEFAULT_AVATAR);
                    Log.d(TAG, "mSenderPhoto=" + mSenderPhoto.getDrawable());
                } catch (NullPointerException e) {
                    Log.d(TAG, "handle exception");
                    mSenderPhoto.setImageDrawable(avatarDrawable);
                }
            } else {
                mSenderPhoto.setImageDrawable(avatarDrawable);
            }


        } else {
            Log.d(TAG, "[viney] recieved message");
            mAddress = msgItem.getAddress();
            if ((!TextUtils.isEmpty(mAddress))) {
                mGroupContact = Contact.get(mAddress, true);
            }
            String number = mGroupContact.getNumber();
            if (Mms.isEmailAddress(number)) {
                mSenderPhoto.assignContactFromEmail(number, true);
            } else {
                if (mGroupContact.existsInDatabase()) {
                    mSenderPhoto.assignContactUri(mGroupContact.getUri());
                } else {
                    mSenderPhoto.assignContactFromPhone(number, true);
                }
            }
            avatarDrawable = mGroupContact.getAvatar(mContext, sDefaultContactImage, -1);
            if (mGroupContact.existsInDatabase() && sDefaultContactImage != avatarDrawable) {
                try {
                    mSenderPhoto.setOverlay(null);
                    photoUri = mGroupContact.getPhotoUri();
                    Log.d(TAG, "photoUri=" + photoUri);
                    ContactPhotoManager contactPhotoManager =
                            ContactPhotoManager.getInstance(mContext);
                    final DefaultImageRequest request = new DefaultImageRequest(null, null,
                            true /* isCircular */);
                    Log.d(TAG, "request=" + request);
                    contactPhotoManager.loadDirectoryPhoto(mSenderPhoto, photoUri,
                            false /* darkTheme */, true /* isCircular */, request);
                    Log.d(TAG, "mSenderPhoto=" + mSenderPhoto.getDrawable());
                } catch (NullPointerException e) {
                    Log.d(TAG, "handle exception");
                    mSenderPhoto.setImageDrawable(sDefaultContactImage);
                }
            } else {
                mSenderPhoto.setImageDrawable(sDefaultContactImage);
            }
        }

        mSenderPhoto.setVisibility(View.VISIBLE);

        if (submsg) {
            // hide the network info for the sim messages
            Log.d(TAG, "sim messages");
            mNetInfo.setVisibility(View.GONE);
            mNetImg.setVisibility(View.GONE);

            // hide the avatar icon for the sub messages
            // get children from checkbox layout
            if (senderImage != null) {
                Log.d(TAG, "sim messages 2");
                senderImage.setVisibility(View.GONE);
            }
        }

        setMessagedate(msgItem, sendingStr, msgUri);
    }


    private void setMessagedate(IMessageItemCallback msgItem, String sendingStr, Uri msgUri) {
        String date_text = null;
        long dateStr = 0L;
        Cursor msgItemCursor;
        Log.d(TAG, "msguri = " + msgUri);

        // no need to set timestamp for sim sent messages
        if (msgItem.getIsSubMessage() && !msgItem.isReceivedMessage()) {
            Log.d(TAG, "no need to set date for sim sent messages");
            // hide the op layout as date & N/w info need to be hidden for the sim sent message
            mOpLayout.setVisibility(View.GONE);
            return;
        }

        // get the timestamp string
        if ("mms".equals(msgItem.getType())) {
            try {
                if (msgItem.getMessageType() == PduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
                    Log.d(TAG, "no need to set date for mms notification type");
                    return;
                } else {
                    PduPersister p = PduPersister.getPduPersister(mContext);
                    MultimediaMessagePdu msg = (MultimediaMessagePdu) p.load(msgUri);

                    if (msgItem.getMessageType() == PduHeaders.MESSAGE_TYPE_RETRIEVE_CONF) {
                        RetrieveConf retrieveConf = (RetrieveConf) msg;
                        dateStr = retrieveConf.getDate() * 1000L;
                    } else {
                        dateStr = ((SendReq) msg).getDate() * 1000L;
                    }
                }
            } catch (MmsException e) {
                Log.d(TAG, "Handel exception");
            }
            Log.d(TAG, "isOutgoingMessage = " + msgItem.isOutgoingMessage());
            Log.d(TAG, "isFailedMessage= " + msgItem.isFailedMessage());
            if (msgItem.isOutgoingMessage() || msgItem.isFailedMessage()) {
                dateStr = 0L;
            }
            Log.d(TAG, "timestamp= " + dateStr);

        } else if ("sms".equals(msgItem.getType())) {
            dateStr = msgItem.getMessageDate();
        }

        Log.d(TAG, "dateStr: = " + dateStr);

        if (dateStr > 0) {
            date_text = MessageUtils.formatDateAndTimeStampString(mContext, 0, dateStr, false);
        }

        if (!TextUtils.isEmpty(date_text)) {
            StringBuilder date = new StringBuilder();
            if (msgItem.isReceivedMessage() == true) {
                date.append(mContext.getResources().getString(R.string.recieved_sms));
            } else {
                date.append(mContext.getResources().getString(R.string.sent_sms));
            }
            date.append(" ");
            date.append(date_text);
            mDateTime.setText(date.toString());
            mDateTime.setVisibility(View.VISIBLE);
            mDateTime.setTextColor(Color.BLACK);
            Log.d(TAG, "[viney] date string set");

        } else {
            if (msgItem.isOutgoingMessage() && msgItem.isFailedMessage()) {
                Log.d(TAG, "[viney] failed message, remove datetime");
                mDateTime.setVisibility(View.GONE);
            } else {
                Log.d(TAG, "[viney] sending message");
                mDateTime.setText(sendingStr);
                mDateTime.setVisibility(View.VISIBLE);
                mDateTime.setTextColor(Color.BLACK);
            }
        }
    }

}
