/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.clkqualityat;

import android.os.SystemClock;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.desenseat.ATEServerSockets;
/**
 * Test modes.
 */
public class TestMode implements ClientSocket.OnResponseListener {
    protected static final String TAG = "ClkQualityAt/TestMode";
    private static final String CMD_START_PRE = "PMTK856,1,";
    private static final String CHECK_BIT_CMD_POST = ",0";
    private static final String CMD_STOP = "PMTK856,0";
    public static final int MODE_NONE = 0;
    public static final int MODE_CW = 1;
    public static final int MODE_SIGNAL = 2;
    private ClientSocket mClient = null;
    ATEServerSockets mServerSockets = null;
    private int mTriggerSource = 0;
    private static final int FIX_TIME = 60;

    private int mTestItemIndex;
    private int mTestState;
    private boolean mIsEmpty;
    private TestResult mResult;
    private int mHeatingTimes;
    private int mCoolTimes;
    private int mModeValue;
    private long mStartTime;
    private boolean mIsClockTypeCorrect = true;
    private String tag;
    public String getTag() {
        return tag;
    }
    public void setTag(String tag) {
        this.tag = tag;
    }
    private ClkQualityAtActivity.ClockType mClockType;

    TestMode(int heatingTimes, int coolTimes, int mode) {
        mHeatingTimes = heatingTimes;
        mCoolTimes = coolTimes;
        mModeValue = mode;
    }

    public int getmTriggerSource() {
        return mTriggerSource;
    }
    public void setmTriggerSource(int mTriggerSource,ATEServerSockets serverSockets ) {
        mServerSockets = serverSockets;
        this.mTriggerSource = mTriggerSource;
    }

    public int getHeatTestTime() {
        return mHeatingTimes;
    }

    public int getCoolTestTime() {
        return mCoolTimes;
    }

    public boolean isClockTypeCorrect() {
        return mIsClockTypeCorrect;
    }

    boolean checkReadyBit(ClkQualityAtActivity.ClockType clockType) {
        mClockType = clockType;
        mClient = new ClientSocket(null);
        mClient.setClockType(clockType);
        String startCommand = CMD_START_PRE + mModeValue + CHECK_BIT_CMD_POST;
        boolean result = mClient.sendCheckBitCmd(startCommand, CMD_STOP, FIX_TIME);
        mIsClockTypeCorrect = mClient.isClockTypeCorrect();
        Elog.d(TAG, "@checkReadyBit, is clock type correct: " + mIsClockTypeCorrect);
        if (mClient != null) {
            mClient.closeClient();
        }
        return result;
    }

    void setParameters(int testItemIndex, int testState, boolean empty) {
        mTestItemIndex = testItemIndex;
        mTestState = testState;
        mIsEmpty = empty;
    }


    TestResult run() {

        if (mTestState == 0) {
            mResult = new TestResult();
            mStartTime = SystemClock.elapsedRealtime() / 100;
        }
        // send PMTK command and receive response
        mClient = new ClientSocket(this);
        String command = CMD_START_PRE + mModeValue + "," + mTestItemIndex;

        int loopTimes = (mTestState == 0) ? mHeatingTimes : mCoolTimes;
        Elog.d(TAG, "sendCommand " + command + ". test times: " + loopTimes);
        mClient.sendTestCmd(command, CMD_STOP, loopTimes);
        mResult.state = TestResult.ResultState.STATE_DONE;
        if (mClient != null) {
            mClient.closeClient();
        }
        return mResult;
    }

    @Override
    public void onResponse(String response) {
        // TODO Auto-generated method stub

        Elog.d(TAG, "onResponse " + response);
        if (response == null) {
            mResult.result = TestResult.CONNECTION_FAIL;
            mResult.state = TestResult.ResultState.STATE_NA;
            if (mClient != null) {
                mClient.closeClient();
            }
            return;
        }

        String[] splited = response.split(",");

        float clkdriftrate = -1.0f;
        float compclkdriftrate = -1.0f;
        float compclkdrift = -1.0f;
        if (mClockType.equals(ClkQualityAtActivity.ClockType.Co_TMS)) {
            clkdriftrate = Float.parseFloat(splited[2]);
            compclkdriftrate = Float.parseFloat(splited[4]);
            compclkdrift = Float.parseFloat(splited[3]);
        } else if (mClockType.equals(ClkQualityAtActivity.ClockType.TCXO)) {
            clkdriftrate = Float.parseFloat(splited[4]);
        }

        long curTime = SystemClock.elapsedRealtime();
        mResult.addClkdriftrate(curTime / 100 - mStartTime, clkdriftrate);
        mResult.addCompclkdriftrate(curTime / 100 - mStartTime, compclkdriftrate);
        mResult.addCompclkdrift(compclkdrift);

        if(getmTriggerSource() == 1){
            Elog.d(TAG, "triggle from ATE Server");
            mServerSockets.sendCommand("PMTK2027," + getTag() + "," + (curTime / 100 - mStartTime)
                    + "," + clkdriftrate + "," +  compclkdriftrate);
        }
    }

    @Override
    public void onCalculateResult() {
        // TODO Auto-generated method stub

        if (mTestState == TestItem.TEST_STATE_ON) {
            return;
        }
        mResult.calPassResult(mIsEmpty ? TestResult.PassCriterion.EmptyItem : TestResult.PassCriterion.OtherItem);
    }
}

