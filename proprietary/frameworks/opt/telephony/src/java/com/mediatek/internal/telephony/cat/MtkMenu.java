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

import android.graphics.Bitmap;
import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;
import java.util.List;

import com.android.internal.telephony.cat.Menu;

/**
 * Container class for CAT menu (SET UP MENU, SELECT ITEM) parameters.
 *
 */
public class MtkMenu extends Menu {

    /**
       * Add the variable for the tag nextActionIndicator.
       * @internal
       */
    public byte[] nextActionIndicator;
    /* Add the flag to check the command is from modem or DB when the command is SET_UP_MENU */
    public int mFromMD;

    public MtkMenu() {
        super();
        nextActionIndicator = null;
        mFromMD = 0;
    }

    private MtkMenu(Parcel in) {
        super(in);
        mFromMD = in.readInt();
        int naiLen = in.readInt();
        if (naiLen <= 0) {
            nextActionIndicator = null;
        } else {
            nextActionIndicator = new byte[naiLen];
            in.readByteArray(nextActionIndicator);
        }
        MtkCatLog.d("[MtkMenu]", "Menu: " + mFromMD);
    }


    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeInt(mFromMD);

        dest.writeInt(nextActionIndicator == null ? -1 : nextActionIndicator.length);
        if (nextActionIndicator != null && nextActionIndicator.length > 0) {
            dest.writeByteArray(nextActionIndicator);
        }
        MtkCatLog.w("[MtkMenu]", "writeToParcel: " + mFromMD);
    }

    public static final Parcelable.Creator<MtkMenu> CREATOR = new Parcelable.Creator<MtkMenu>() {
        @Override
        public MtkMenu createFromParcel(Parcel in) {
            return new MtkMenu(in);
        }

        @Override
        public MtkMenu[] newArray(int size) {
            return new MtkMenu[size];
        }
    };

    /**
     * Check the command SET_UP_MENU which is from modem or sharepreference.
     * @internal
     */
    public int getSetUpMenuFlag() {
        return mFromMD;
    }

    public void setSetUpMenuFlag(int FromMD) {
        mFromMD = FromMD;
    }
}
