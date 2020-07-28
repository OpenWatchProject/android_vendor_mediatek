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

package com.mediatek.mms.ext;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.text.SpannableStringBuilder;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.mediatek.mms.callback.IFileAttachmentModelCallback;
import com.mediatek.mms.callback.IMessageItemCallback;
import com.mediatek.mms.callback.IMessageListItemCallback;

import java.util.ArrayList;

public interface IOpMessageListItemExt {

    /**
     * @internal
     */
    boolean showOrHideFileAttachmentView(ArrayList files);

    /**
     * @internal
     */
    void openUrl(Context context, String url);

    /**
     * @internal
     */
    public void bindNotifInd(Context context, String msgSizeText, TextView expireText,
            TextView subStatus, CheckBox selectedBox, boolean isSmsEnabled, TextView sendTime,
            LinearLayout doubleTime);

    /**
     * @internal
     */
    public boolean onDownloadButtonClick(Activity activity, boolean storageFull);

    /**
     * @internal
     */
    public void showDownloadingAttachment(CheckBox selectedBox);

    /**
     * @internal
     */
    public void bindCommonMessage(Button downloadButton, TextView dateView);

    /**
     * @internal
     */
    public SpannableStringBuilder formatMessage(
            SpannableStringBuilder buf, String body, boolean hasSubject);

    /**
     * @internal
     */
    public boolean onMessageListItemClick();

    /**
     * @internal
     */
    public void drawRightStatusIndicator(Context context, LinearLayout statusLayout,
            TextView subStatus, TextView sendTimeText,
            LinearLayout doubleTimeLayout, String stampLine);

    /**
     * @internal
     */
    public void bindDefault(Button downloadButton);

    /**
     * @internal
     */
    public boolean onFileAttachmentViewClick(boolean isVcard, String src);

    /**
     * @internal
     */
    public void showFileAttachmentView(View fileAttachmentView,
            TextView name2, TextView name, ImageView thumb, TextView size,
            ArrayList attachFiles, final Intent intent, final long msgId,
            final IFileAttachmentModelCallback attach,
            final IOpFileAttachmentModelExt opFileAttachmentModelExt);

    /**
     * @internal
     */
    public void inflateDownloadControls(Activity activity, LinearLayout downladBtnLayout,
            TextView expireText);

    /**
     * @internal
     */
    void hideAllView();

    /**
     * @internal
     */
    void init(IMessageListItemCallback messageListItem, Context context);

    /**
     * @internal
     */
    void onFinishInflateView(TextView expireText, CheckBox selectBox, TextView bodyTextView);

    /**
     * @internal
     */
    void bindMessage(IMessageItemCallback msgItem, TextView subStatus, TextView dateView,
                                   String sendingStr);

}
