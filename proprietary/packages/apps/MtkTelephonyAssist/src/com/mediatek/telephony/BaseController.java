/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.telephony;

import android.content.Context;
import android.os.Handler;
import android.os.Message;

import com.android.internal.telephony.Phone;

public abstract class BaseController {
    private static final int FACTOR = 10;
    private static final int EVENT_RADIO_STATE_CHANGED = FACTOR * 1;

    protected Context mContext;
    protected Phone[] mPhones;
    protected AssistRIL[] mAssistRIL;

    public BaseController(Context context) {
        mContext = context;
    }

    /**
     * Called when radio changed.
     */
    abstract void onRadioChanged(int phoneId);

    /**
     * Called when radio changed.
     */
    abstract void initialize();

    /**
     * Called by MtkTelephonyService when the service is destroyed.
     */
    void onDestroy() {
        if (mPhones != null) {
            for (int i = 0; i < mPhones.length; i++) {
                mPhones[i].mCi.unregisterForRadioStateChanged(mBaseHandler);
            }
        }
    }

    private Handler mBaseHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int phoneId = msg.what % FACTOR;
            int evendId = msg.what - phoneId;
            switch (evendId) {
                case EVENT_RADIO_STATE_CHANGED:
                    onRadioChanged(phoneId);
                    break;
                default:
                    break;
            }
        }
    };

    /**
     * Called by MtkTelephonyService when it detect Phone is created.
     * Controller may want to send device state to RILD after RadioProxy initialized,
     * this is exactly the place to do such logic.
     */
    public void onPhoneCreated(Phone[] phones, AssistRIL[] assistRIL) {
        mPhones = phones;
        mAssistRIL = assistRIL;

        for (int i = 0; i < mPhones.length; i++) {
            mPhones[i].mCi.registerForRadioStateChanged(mBaseHandler,
                    EVENT_RADIO_STATE_CHANGED + i, null);
        }

        initialize();
    }
}
