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
package com.mediatek.contacts.list;

import android.view.View;
import android.view.View.MeasureSpec;
import android.widget.CheckBox;
import android.widget.TextView;

import com.android.contacts.list.ContactListItemView;

public class ContactListItemViewEx {
    private ContactListItemView mContactListItemView = null;
    // / Add the check-box support for ContactListItemView.
    private CheckBox mChecktBox = null;

    public ContactListItemViewEx(ContactListItemView contactListItemView) {
        mContactListItemView = contactListItemView;
    }

    /**
     * Enable check-box or disable check-box
     *
     * @param checkable
     *            is true, create check-box and set the visibility.
     */
    public void setCheckable(boolean checkable) {
        if (checkable) {
            if (mChecktBox == null) {
                getCheckBox();
            }
            mChecktBox.setVisibility(View.VISIBLE);
        } else {
            if (mChecktBox != null) {
                mChecktBox.setVisibility(View.GONE);
            }
        }
    }

    /**
     * Retrieve the check box view for changing its state between Checked or
     * Unchecked state
     *
     * @return check-box view
     */
    public CheckBox getCheckBox() {
        if (mChecktBox == null) {
            mChecktBox = new CheckBox(mContactListItemView.getContext());
            mChecktBox.setClickable(false);
            mChecktBox.setFocusable(false);
            mChecktBox.setFocusableInTouchMode(false);
            mContactListItemView.addView(mChecktBox);
        }
        mChecktBox.setVisibility(View.VISIBLE);
        return mChecktBox;
    }

    /**
     * Measure check-box view
     */
    public void measureCheckBox() {
        if (isVisible(mChecktBox)) {
            mChecktBox.measure(0, 0);
        }
    }

    /**
     * Performs layout of check-box view
     *
     * @return new left boundary
     */
    public int layoutLeftCheckBox(int leftBound, int topBound, int bottomBound) {
        if (isVisible(mChecktBox)) {
            int selectBoxWidth = mChecktBox.getMeasuredWidth();
            int selectBoxHeight = mChecktBox.getMeasuredHeight();
            mChecktBox.layout(leftBound, (bottomBound + topBound - selectBoxHeight) / 2, leftBound
                    + selectBoxWidth, (bottomBound + topBound + selectBoxHeight) / 2);
            return leftBound + selectBoxWidth;
        }
        return leftBound;
    }

    /**
     * RTL New Feature
     */
    public int layoutRightCheckBox(int rightBound, int topBound, int bottomBound) {
        if (isVisible(mChecktBox)) {
            int selectBoxWidth = mChecktBox.getMeasuredWidth();
            int selectBoxHeight = mChecktBox.getMeasuredHeight();
            mChecktBox.layout(rightBound - selectBoxWidth,
                    (bottomBound + topBound - selectBoxHeight) / 2, rightBound, (bottomBound
                            + topBound + selectBoxHeight) / 2);
            return rightBound - selectBoxWidth;
        }
        return rightBound;
    }

    private boolean isVisible(View view) {
        return view != null && view.getVisibility() == View.VISIBLE;
    }

    /**
     * fix ALPS01758642,show ... for long string.
     */
    public void measureTextView(TextView textView) {
        if (isVisible(mChecktBox)) {
            int spec1 = MeasureSpec.makeMeasureSpec(textView.getWidth(), MeasureSpec.EXACTLY);
            int spec2 = MeasureSpec.makeMeasureSpec(textView.getHeight(), MeasureSpec.EXACTLY);
            textView.measure(spec1, spec2);
        }
    }

}
