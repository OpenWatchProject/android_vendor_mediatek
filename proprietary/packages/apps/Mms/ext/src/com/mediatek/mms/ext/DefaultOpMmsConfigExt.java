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

import android.os.Bundle;
import android.util.Log;

public class DefaultOpMmsConfigExt implements IOpMmsConfigExt {
    private static final String TAG = "Mms/MmsConfigImpl";

    private static int sSmsToMmsTextThreshold = 4;

    private static int sMaxTextLimit = 2048;
    private static int sMmsRecipientLimit = 20;                 // default value
/// M:Code analyze 01,For new feature CMCC_Mms in ALPS00325381, MMS easy porting check in JB @{
    private static int sHttpSocketTimeout = 60 * 1000;
/// @}
    /// M: For common case, default retry scheme not change
    private static final int[] DEFAULTRETRYSCHEME = {
        0, 1 * 60 * 1000, 5 * 60 * 1000, 10 * 60 * 1000, 30 * 60 * 1000};
    /// @}

    public int getSmsToMmsTextThreshold() {
        // 0: default plugin don't handle the operation
        return 0;
    }

    public void setSmsToMmsTextThreshold(int value) {
        if (value > -1) {
            sSmsToMmsTextThreshold = value;
        }
        Log.d(TAG, "set SmsToMmsTextThreshold: " + sSmsToMmsTextThreshold);
    }

    public int getMaxTextLimit() {
  //      Log.d(TAG, "get MaxTextLimit: " + sMaxTextLimit);
        return sMaxTextLimit;
    }

    public void setMaxTextLimit(int value) {
        if (value > -1) {
            sMaxTextLimit = value;
        }

        Log.d(TAG, "set MaxTextLimit: " + sMaxTextLimit);
    }

    public int getMmsRecipientLimit() {
        Log.d(TAG, "RecipientLimit: " + sMmsRecipientLimit);
        return sMmsRecipientLimit;
    }

    public void setMmsRecipientLimit(int value) {
        if (value > -1) {
            sMmsRecipientLimit = value;
        }

        Log.d(TAG, "set RecipientLimit: " + sMmsRecipientLimit);
    }

  /// M:Code analyze 02,For new feature CMCC_Mms in ALPS00325381, MMS easy porting check in JB @{
    public int getHttpSocketTimeout() {
        Log.d(TAG, "get default socket timeout: " + sHttpSocketTimeout);
        return sHttpSocketTimeout;
    }

    public void setHttpSocketTimeout(int socketTimeout) {
        Log.d(TAG, "set default socket timeout: " + socketTimeout);
        sHttpSocketTimeout = socketTimeout;
    }
/// @}

    public int getMmsRetryPromptIndex() {
        Log.d(TAG, "getMmsRetryPromptIndex");
        return 1;
    }

    /// M: Add MmsService configure param @{
    public Bundle getMmsServiceConfig() {
        return null;
    }
    /// @}
}

