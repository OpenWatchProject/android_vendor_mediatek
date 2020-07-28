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

import com.android.internal.telephony.cat.CatService;
import com.android.internal.telephony.cat.CommandParams;
import com.android.internal.telephony.cat.BerTlv;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.ResultException;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;

/**
 * Class used for queuing raw ril messages, decoding them into CommanParams
 * objects and sending the result back to the CAT Service.
 */
class BipRilMessageDecoder extends StateMachine {

    // constants
    private static final int CMD_START = 1;
    private static final int CMD_PARAMS_READY = 2;

    // members
    private BipCommandParamsFactory mBipCmdParamsFactory = null;
    private MtkRilMessage mCurrentRilMessage = null;
    private Handler mCaller = null;
    private int mSlotId;
    private static int mSimCount = 0;
    private static BipRilMessageDecoder[] mInstance = null;

    // States
    private StateStart mStateStart = new StateStart();
    private StateCmdParamsReady mStateCmdParamsReady = new StateCmdParamsReady();

    /**
     * Get the singleton instance, constructing if necessary.
     *
     * @param caller
     * @param fh
     * @return RilMesssageDecoder
     */
    public static synchronized BipRilMessageDecoder getInstance(Handler caller, IccFileHandler fh,
            int slotId) {
        if (null == mInstance) {
            mSimCount = TelephonyManager.getDefault().getSimCount();
            mInstance = new BipRilMessageDecoder[mSimCount];
            for (int i = 0; i < mSimCount; i++) {
                mInstance[i] = null;
            }
        }

        if (slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX && slotId < mSimCount) {
            if (null == mInstance[slotId]) {
                mInstance[slotId] = new BipRilMessageDecoder(caller, fh, slotId);
            }
        } else {
            MtkCatLog.d("BipRilMessageDecoder", "invaild slot id: " + slotId);
            return null;
        }

        return mInstance[slotId];
    }

    /**
     * Start decoding the message parameters,
     * when complete MSG_ID_RIL_MSG_DECODED will be returned to caller.
     *
     * @param rilMsg
     */
    public void sendStartDecodingMessageParams(MtkRilMessage rilMsg) {
        Message msg = obtainMessage(CMD_START);
        msg.obj = rilMsg;
        sendMessage(msg);
    }

    /**
     * The command parameters have been decoded.
     *
     * @param resCode
     * @param cmdParams
     */
    public void sendMsgParamsDecoded(ResultCode resCode, CommandParams cmdParams) {
        Message msg = obtainMessage(BipRilMessageDecoder.CMD_PARAMS_READY);
        msg.arg1 = resCode.value();
        msg.obj = cmdParams;
        sendMessage(msg);
    }

    private void sendCmdForExecution(MtkRilMessage rilMsg) {
        Message msg = mCaller.obtainMessage(BipService.MSG_ID_RIL_MSG_DECODED,
                new MtkRilMessage(rilMsg));
        msg.sendToTarget();
    }

    public int getSlotId() {
        return mSlotId;
    }

    private BipRilMessageDecoder(Handler caller, IccFileHandler fh, int slotId) {
        super("BipRilMessageDecoder");

        addState(mStateStart);
        addState(mStateCmdParamsReady);
        setInitialState(mStateStart);

        mCaller = caller;
        mSlotId = slotId;
        MtkCatLog.d(this, "mCaller is " + mCaller.getClass().getName());
        mBipCmdParamsFactory = BipCommandParamsFactory.getInstance(this, fh);
    }

    private BipRilMessageDecoder() {
        super("BipRilMessageDecoder");
    }

    private class StateStart extends State {
        @Override
        public boolean processMessage(Message msg) {
            if (msg.what == CMD_START) {
                if (decodeMessageParams((MtkRilMessage)msg.obj)) {
                    transitionTo(mStateCmdParamsReady);
                }
            } else {
                MtkCatLog.d(this, "StateStart unexpected expecting START=" +
                         CMD_START + " got " + msg.what);
            }
            return true;
        }
    }

    private class StateCmdParamsReady extends State {
        @Override
        public boolean processMessage(Message msg) {
            if (msg.what == CMD_PARAMS_READY) {
                mCurrentRilMessage.mResCode = ResultCode.fromInt(msg.arg1);
                mCurrentRilMessage.mData = msg.obj;
                sendCmdForExecution(mCurrentRilMessage);
                transitionTo(mStateStart);
            } else {
                MtkCatLog.d(this, "StateCmdParamsReady expecting CMD_PARAMS_READY="
                         + CMD_PARAMS_READY + " got " + msg.what);
                deferMessage(msg);
            }
            return true;
        }
    }

    private boolean decodeMessageParams(MtkRilMessage rilMsg) {
        boolean decodingStarted;

        mCurrentRilMessage = rilMsg;
        switch(rilMsg.mId) {
        case BipService.MSG_ID_BIP_PROACTIVE_COMMAND:
        case BipService.MSG_ID_EVENT_NOTIFY:
            byte[] rawData = null;
            MtkCatLog.d(this, "decodeMessageParams raw: " + (String) rilMsg.mData);
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
                mBipCmdParamsFactory.make(BerTlv.decode(rawData));
                decodingStarted = true;
            } catch (ResultException e) {
                // send to Service for proper RIL communication.
                MtkCatLog.d(this, "decodeMessageParams: caught ResultException e=" + e);
                // Add by Huibin Mao Mtk80229
                // ICS Migration start
                mCurrentRilMessage.mId = CatService.MSG_ID_SESSION_END;
                // ICS Migration end
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
        int i;
        mStateStart = null;
        mStateCmdParamsReady = null;
        mBipCmdParamsFactory.dispose();
        mBipCmdParamsFactory = null;
        mCurrentRilMessage = null;
        mCaller = null;

        if (null != mInstance) {
            if (null != mInstance[mSlotId]) {
                mInstance[mSlotId].quit();
                mInstance[mSlotId] = null;
            }
            // Check if all mInstance[] is null
            for (i = 0 ; i < mSimCount ; i++) {
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
