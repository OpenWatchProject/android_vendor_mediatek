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

package com.mediatek.mms.callback;

import android.content.Intent;
import android.net.Uri;

public interface IDialogModeActivityCallback {

    /**
     * M: get current uri index from host
     * @return uri idx
     */
    public int getCurUriIdx();

    /**
     * M: set current uri index to host
     * @param curUriIdx: curUriIdx
     * @param curUri: curUri
     */
    public void setCurUriIdx(int curUriIdx, Uri curUri);

    /**
     * M: callback mResetMessageRunnable
     */
    public void onPreMessageSent();

    /**
     * M: callback mMessageSentRunnable
     */
    public void onMessageSent();

    /**
     * M: get number from host
     */
    public String getNumber();

    /**
     * M: get name from host
     */
    public String getName();

    /**
     * M: get thread id from host
     */
    public long getIpThreadId();

    /**
     * M: mark as read
     * @param uri: uri
     */
    public void markIpAsRead(final Uri uri);

    /**
     * M: sub selection
     */
    public void onIpSubSelection();

    public int getIpSelectedId();

    void setIpSelectedSubId(int selectedSubId);

    public void onIpConfirmSendMessageIfNeeded();

    public void onIpShowSubSelectedDialog(boolean overridePref , Intent intent);

    public void onIpUpdateSendButtonState();

    /// M: add for op @{

    void opSetSelectedSubId(int subId);

    void opConfirmSendMessageIfNeeded();

    public Uri opGetCurUri();

    public long opGetThreadId();

    public int opGetCurrentSubId();

    /// @}
}














