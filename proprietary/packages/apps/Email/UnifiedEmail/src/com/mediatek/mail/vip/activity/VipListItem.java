/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.mail.vip.activity;

import com.android.mail.R;
import com.android.emailcommon.mail.Address;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.mediatek.mail.ui.utils.UiUtilities;

import android.app.Activity;
import android.app.FragmentManager;
import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * M: The VipListItem class used for display vip item info(include: name, address, avatar...)
 *
 */
public class VipListItem extends LinearLayout implements View.OnClickListener {

    private Uri mQuickContactLookupUri;

    private Activity mActivity;
    private long mVipId;
    private ImageView mVipPhoto;
    private ImageView mRemoveButton;
    private TextView mVipName;
    private TextView mVipEmailAddress;
    private final EmailAsyncTask.Tracker mTaskTracker = new EmailAsyncTask.Tracker();

    public VipListItem(Context context) {
        super(context);
    }

    public VipListItem(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public VipListItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void loadContactAvatar(ImageView view, long photoId) {
        ListPhotoManager.getInstance(mContext).loadThumbnail(view, photoId, false);
    }

    public void setTargetActivity(Activity activity) {
        mActivity = activity;
    }

    public void setVipId(long id) {
        mVipId = id;
    }

    public void setVipName(String vipName) {
        mVipName.setText(vipName);
    }

    public void setVipEmailAddress(String vipEmailAddress) {
        mVipEmailAddress.setText(vipEmailAddress);
    }

    public void setQuickContactLookupUri(Uri uri) {
        mQuickContactLookupUri = uri;
    }

    public void resetViews() {
        mVipPhoto.setClickable(true);
        // In case the last loading was not completed.
        ListPhotoManager.getInstance(mContext).removePhoto(mVipPhoto);
        mVipPhoto.setImageResource(R.drawable.ic_contact_picture);
        mRemoveButton.setClickable(true);
        mQuickContactLookupUri = null;
    }

    @Override
    protected void onFinishInflate() {
        mVipPhoto = (ImageView) findViewById(R.id.contact_icon);
        mVipPhoto.setClickable(true);
        mVipPhoto.setOnClickListener(this);
        mRemoveButton = (ImageView) findViewById(R.id.remove_vip);
        mRemoveButton.setClickable(true);
        mRemoveButton.setOnClickListener(this);
        mVipName = (TextView) findViewById(R.id.contact_name);
        mVipEmailAddress = (TextView) findViewById(R.id.email_address);
    }

    /**
     * Handle clicks on sender, which shows {@link QuickContact} or prompts to add
     * the sender as a contact.
     */
    private void onClickPhoto() {
        final String vipEmail = (String) mVipEmailAddress.getText();
        if (TextUtils.isEmpty(vipEmail)) {
            return;
        }

        Address address = new Address(vipEmail, (String) mVipName.getText());
        UiUtilities.showContacts(mActivity, mVipPhoto, mQuickContactLookupUri, address);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
        case R.id.contact_icon:
            onClickPhoto();
            break;
        case R.id.remove_vip:
            onRemoveVip();
            break;
        }
    }

    private void onRemoveVip() {
        if (mActivity != null) {
            String name = (String) mVipName.getText();
            String address = (String) mVipEmailAddress.getText();
            String message = name + "(" + address + ")";
            ///M: Check whether the dialog is showing first, show it if needed. @{
            final FragmentManager fm = mActivity.getFragmentManager();
            if (fm.findFragmentByTag(VipRemoveDialog.TAG) == null) {
                VipRemoveDialog.newInstance(message, mVipId).show(fm, VipRemoveDialog.TAG);
            }
            /// @}
        }
    }
}
