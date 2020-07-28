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

import android.os.Handler;
import android.os.Message;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.cat.BerTlv;
import com.android.internal.telephony.cat.CatService;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.ResultException;
import com.android.internal.telephony.cat.RilMessage;
import com.android.internal.telephony.cat.RilMessageDecoder;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

/**
 * Class used for queuing raw ril messages, decoding them into CommanParams
 * objects and sending the result back to the CAT Service.
 */
public class MtkRilMessageDecoder extends RilMessageDecoder {

    // members
    private int mSlotId;

    public int getSlotId() {
        return mSlotId;
    }

    public MtkRilMessageDecoder(Handler caller, IccFileHandler fh, int slotId) {
        super(caller, fh);

        mSlotId = slotId;
        MtkCatLog.d(this, "mCaller is " + mCaller.getClass().getName());
    }

    public MtkRilMessageDecoder() {
        super();
    }

    @Override
    protected void sendCmdForExecution(RilMessage rilMsg) {
        MtkCatLog.d(this, "sendCmdForExecution");
        if (rilMsg instanceof MtkRilMessage) {
            Message msg = mCaller.obtainMessage(CatService.MSG_ID_RIL_MSG_DECODED,
                    new MtkRilMessage((MtkRilMessage) rilMsg));
            msg.sendToTarget();
        } else {
            super.sendCmdForExecution(rilMsg);
        }
    }

    @Override
    public boolean decodeMessageParams(RilMessage rilMsg) {
        MtkCatLog.d(this, "decodeMessageParams");
        boolean decodingStarted;

        mCurrentRilMessage = rilMsg;
        switch (rilMsg.mId) {
            case CatService.MSG_ID_SESSION_END:
            case CatService.MSG_ID_CALL_SETUP:
                mCurrentRilMessage.mResCode = ResultCode.OK;
                sendCmdForExecution(mCurrentRilMessage);
                decodingStarted = false;
                break;
            case CatService.MSG_ID_PROACTIVE_COMMAND:
            case CatService.MSG_ID_REFRESH:
            case CatService.MSG_ID_EVENT_NOTIFY:
                byte[] rawData = null;

                try {
                    rawData = IccUtils.hexStringToBytes((String) rilMsg.mData);
                } catch (Exception e) {
                    // zombie messages are dropped
                    MtkCatLog.d(this, "decodeMessageParams dropping zombie messages");
                    decodingStarted = false;
                    break;
                }
                try {
                    // Start asynch parsing of the command parameters.
                    if (mCmdParamsFactory != null) {
                        mCmdParamsFactory.make(BerTlv.decode(rawData));
                        decodingStarted = true;
                    } else {
                        decodingStarted = false;
                    }
                } catch (ResultException e) {
                    // send to Service for proper RIL communication.
                    MtkCatLog.d(this, "decodeMessageParams: caught ResultException e=" + e);
                    mCurrentRilMessage.mId = CatService.MSG_ID_SESSION_END;
                    mCurrentRilMessage.mResCode = e.result();
                    sendCmdForExecution(mCurrentRilMessage);
                    decodingStarted = false;
                }
                break;
            default:
                decodingStarted = false;
                break;
        }
        return decodingStarted;
    }

    public void dispose() {
        quitNow();
        mStateStart = null;
        mStateCmdParamsReady = null;
        mCmdParamsFactory.dispose();
        mCmdParamsFactory = null;
        mCurrentRilMessage = null;
        mCaller = null;

        int i;
        if (null != mInstance) {
            if (null != mInstance[mSlotId]) {
                mInstance[mSlotId] = null;
            }
            // Check if all mInstance[] is null
            for (i = 0; i < mSimCount; i++) {
                if (null != mInstance[i]) {
                    break;
                }
            }
            // All mInstance[] has been null, set mInstance as null
            if (i == mSimCount) {
                mInstance = null;
            }
        }
    }
}
