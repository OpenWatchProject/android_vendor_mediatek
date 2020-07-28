/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import com.android.internal.telephony.cat.CatCmdMessage;
import com.android.internal.telephony.cat.CatResponseMessage;

public class MtkCatResponseMessage extends CatResponseMessage {

    int mEvent = 0;
    int mSourceId = 0;
    int mDestinationId = 0;
    byte[] mAdditionalInfo = null;
    boolean mOneShot = false;

    public MtkCatResponseMessage(CatCmdMessage cmdMsg) {
        super(cmdMsg);
    }

    public MtkCatResponseMessage(CatCmdMessage cmdMsg, int event) {
        super(cmdMsg);
        mEvent = event;
    }

    public MtkCatResponseMessage(CatCmdMessage cmdMsg, CatResponseMessage rspMsg) {
        super(cmdMsg);
        mCmdDet = rspMsg.mCmdDet;
        mResCode = rspMsg.mResCode;
        mUsersMenuSelection = rspMsg.mUsersMenuSelection;
        mUsersInput = rspMsg.mUsersInput;
        mUsersYesNoSelection = rspMsg.mUsersYesNoSelection;
        mUsersConfirm = rspMsg.mUsersConfirm;
        mIncludeAdditionalInfo = rspMsg.mIncludeAdditionalInfo;
        mEventValue = rspMsg.mEventValue;
        mAddedInfo = rspMsg.mAddedInfo;
    }

    /**
     * Set the source id of the device tag in the terminal response.
     *
     * @internal
     */
    public void setSourceId(int sId) {
        mSourceId = sId;
    }

    public void setEventId(int event) {
        mEvent = event;
    }

    /**
     * Set the destination id of the device tag in the terminal response.
     *
     * @internal
     */
    public void setDestinationId(int dId) {
        mDestinationId = dId;
    }

    /**
     * Set the additional information of result code.
     *
     * @internal
     */
    public void setAdditionalInfo(byte[] additionalInfo) {
        if (additionalInfo != null) {
            mIncludeAdditionalInfo = true;
        }
        mAdditionalInfo = additionalInfo;
        if ((additionalInfo != null) && (additionalInfo.length > 0)) {
            super.mAdditionalInfo = (int) additionalInfo[0];
        }
    }

    /**
     * Set the one shot flag in the terminal response.
     *
     * @internal
     */
    public void setOneShot(boolean shot) {
        mOneShot = shot;
    }
}
