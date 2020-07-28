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

import android.os.Parcel;
import android.os.Parcelable;

import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.CatCmdMessage;
import com.android.internal.telephony.cat.CommandDetails;
import com.android.internal.telephony.cat.CommandParams;
import com.android.internal.telephony.cat.DisplayTextParams;
import com.android.internal.telephony.cat.LaunchBrowserMode;
import com.android.internal.telephony.cat.Menu;
import com.android.internal.telephony.cat.TextMessage;


/**
 * Class used to pass CAT messages from telephony to application. Application
 * should call getXXX() to get commands's specific values.
 */
public class MtkCatCmdMessage extends CatCmdMessage {

    MtkCatCmdMessage(CommandParams cmdParams) {
        super(cmdParams);
    }

    public MtkCatCmdMessage(Parcel in) {
        super(in);
    }

    /* external API to be used by application */
    /**
     * Return command qualifier.
     *
     * @return command qualifier
     * @internal
     */
    public int getCmdQualifier() {
        return mCmdDet.commandQualifier;
    }

    public static final Parcelable.Creator<MtkCatCmdMessage> CREATOR =
            new Parcelable.Creator<MtkCatCmdMessage>() {
        @Override
        public MtkCatCmdMessage createFromParcel(Parcel in) {
            return new MtkCatCmdMessage(in);
        }

        @Override
        public MtkCatCmdMessage[] newArray(int size) {
            return new MtkCatCmdMessage[size];
        }
    };

    // Convert to CatCmdMessage for AOSP APP
    public CatCmdMessage convertToCatCmdMessage(CommandParams cmdParams,
            MtkCatCmdMessage mtkCmdMsg) {
        CatCmdMessage cmdMsg = new CatCmdMessage(cmdParams);
        if (mtkCmdMsg != null) {
            cmdMsg.mCmdDet = mtkCmdMsg.mCmdDet;
            cmdMsg.mTextMsg = mtkCmdMsg.mTextMsg;
            Menu menu = new Menu();
            if (mtkCmdMsg.mMenu != null) {
                menu.items = mtkCmdMsg.mMenu.items;
                menu.titleAttrs = mtkCmdMsg.mMenu.titleAttrs;
                menu.presentationType = mtkCmdMsg.mMenu.presentationType;
                menu.title = mtkCmdMsg.mMenu.title;
                menu.titleIcon = mtkCmdMsg.mMenu.titleIcon;
                menu.defaultItem = mtkCmdMsg.mMenu.defaultItem;
                menu.softKeyPreferred = mtkCmdMsg.mMenu.softKeyPreferred;
                menu.helpAvailable = mtkCmdMsg.mMenu.helpAvailable;
                menu.titleIconSelfExplanatory = mtkCmdMsg.mMenu.titleIconSelfExplanatory;
                menu.itemsIconSelfExplanatory = mtkCmdMsg.mMenu.itemsIconSelfExplanatory;
            }
            cmdMsg.mMenu = menu;
            cmdMsg.mInput = mtkCmdMsg.mInput;
            cmdMsg.mBrowserSettings = mtkCmdMsg.mBrowserSettings;
            cmdMsg.mToneSettings = mtkCmdMsg.mToneSettings;
            cmdMsg.mCallSettings = mtkCmdMsg.mCallSettings;
            cmdMsg.mSetupEventListSettings = mtkCmdMsg.mSetupEventListSettings;
            cmdMsg.mLoadIconFailed = mtkCmdMsg.mLoadIconFailed;
        }
        return cmdMsg;

    }

    public static CatCmdMessage getCmdMsg() {
        /* No use for command type, only to avoid exception */
        CommandDetails cmdDet = new CommandDetails();
        cmdDet.typeOfCommand = 0x70;
        return new CatCmdMessage(new CommandParams(cmdDet));
    }
}
