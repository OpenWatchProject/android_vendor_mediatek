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

import com.android.internal.telephony.cat.IconLoader;
import com.android.internal.telephony.cat.ImageDescriptor;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.util.HexDump;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.telephony.TelephonyManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import java.util.HashMap;

/**
 * Class for loading icons from the SIM card. Has two states: single, for
 * loading one icon. Multi, for loading icons list.
 */
public  class MtkIconLoader extends IconLoader {
    private static final String TAG = "MtkIconLoader";


    public  MtkIconLoader(Looper looper, IccFileHandler fh) {
        super(looper, fh);
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;

        try {
            switch (msg.what) {
                case EVENT_READ_EF_IMG_RECOED_DONE:
                    MtkCatLog.d(TAG, "load EFimg done");
                    if (msg.obj == null) {
                        MtkCatLog.e(TAG, "msg.obj is null.");
                        return;
                    }
                    MtkCatLog.d(TAG, "msg.obj is " + msg.obj.getClass().getName());
                    ar = (AsyncResult) msg.obj;
                    byte[] rawData = ((byte[]) ar.result);
                    MtkCatLog.d(TAG, "EFimg raw data: " + HexDump.toHexString(rawData));
                    if (handleImageDescriptor((byte[]) ar.result)) {
                        readIconData();
                    } else {
                        throw new Exception("Unable to parse image descriptor");
                    }
                    return;
                case EVENT_READ_ICON_DONE:
                    MtkCatLog.d(TAG, "load icon done");
                    ar = (AsyncResult) msg.obj;
                    rawData = ((byte[]) ar.result);
                    MtkCatLog.d(TAG, "icon raw data: " + HexDump.toHexString(rawData));
                    MtkCatLog.d(TAG, "load icon CODING_SCHEME = " + mId.mCodingScheme);
                    if (mId.mCodingScheme == ImageDescriptor.CODING_SCHEME_BASIC) {
                        mCurrentIcon = parseToBnW(rawData, rawData.length);
                        mIconsCache.put(mRecordNumber, mCurrentIcon);
                        postIcon();
                    } else if (mId.mCodingScheme == ImageDescriptor.CODING_SCHEME_COLOUR) {
                        mIconData = rawData;
                        readClut();
                    } else {
                        MtkCatLog.d(TAG, "else  /postIcon ");
                        postIcon();
                    }
                    return;
            }
        } catch (Exception e) {
            MtkCatLog.d(this, "Icon load failed");
            e.printStackTrace();
            // post null icon back to the caller.
            postIcon();
            return;
        }
        super.handleMessage(msg);
    }
}
