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

package com.mediatek.engineermode.desenseat;

import android.os.SystemClock;

import com.mediatek.engineermode.Elog;

import java.util.ArrayList;

/**
 * Test modes.
 */
public abstract class TestMode implements ClientSocket.OnResponseListener {
    protected static final String TAG = "DesenseAT/testmode";
    private static ArrayList<TestMode> sTestModes;
    ClientSocket mClient = null;
    ATEServerSockets mServerSockets = null;

    private int mTriggerSource = 0;
    public int getmTriggerSource() {
        return mTriggerSource;
    }

    public void setmTriggerSource(int mTriggerSource,ATEServerSockets serverSockets ) {
        mServerSockets = serverSockets;
        this.mTriggerSource = mTriggerSource;
    }



    static ArrayList<TestMode> getTestModes() {
        if (sTestModes == null) {
            sTestModes = new ArrayList<TestMode>();
//            sTestModes.add(new GpsNormalMode());
            sTestModes.add(new GpsSignalMode());
            sTestModes.add(new GpsCwMode());
        }
        return sTestModes;
    }


    boolean setSpec(String specCmd) {
        mClient = new ClientSocket();
        String command = "PMTK828,5," + specCmd;
        Elog.d(TAG, "sendCommand " + command);
        boolean result = mClient.sendSetSpecCmd(command, 10);
        if (mClient != null) {
            mClient.closeClient();
        }
        return result;
    }

    // set selected sv ids
    abstract void setSvIds(int[] svIds);

    /**
     * set parameters.
     *
     * @param testItemIndex
     *              index of current test item
     * @param testState
     *              0 for the first run, 1 for second run
     * @param testCount
     *              test GPS for how many times, currently hard code to 30
     */
    abstract void setParameters(int testItemIndex, int testState, int testCount);

    abstract TestResult run();

    abstract void stop();
}

/**
 * GPS Mode.
 */
abstract class GpsMode extends TestMode {
    int mTestItemIndex;
    int mTestState;
    int mTestCount;
    int mSvCount;
    int[] mSvIds;
    TestResult mResult;
    long mStartTime;
    int mResponseCount = 0;


    public void setParameters(int testItemIndex, int testState, int testCount) {
        mTestItemIndex = testItemIndex;
        mTestState = testState;
        mTestCount = testCount;
    }

    @Override
    public void stop() {
        // TODO: implement it
    }

    @Override
    public String toString() {
        return "GPS";
    }

}

/**
 * Normal Mode.
 */
class GpsNormalMode extends GpsMode {
    void setSvIds(int[] svIds) {
        mSvCount = 1;
        Elog.d(TAG, "GpsNormalMode count " + mSvCount);
    }

    public TestResult run() {
        if (mTestState == 0) {
            mResult = new TestResult();
            mStartTime = SystemClock.elapsedRealtime() / 100;
        }
        // send PMTK command and receive response
        ClientSocket client = new ClientSocket(this);
        String command = "PMTK828,1,3," + mTestItemIndex + "," + mTestState + "," + 30
                + "," + mSvCount;
        Elog.d(TAG, "sendCommand " + command);
        client.sendCommand(command);
        return mResult;
    }

    public void onResponse(String response) {
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
        int command = Integer.parseInt(splited[1]);
        if (command == 2) {
            int index = Integer.parseInt(splited[5]);
            float snr = Float.parseFloat(splited[6]);
            if (index > 0) {
                mResult.addData(0, SystemClock.elapsedRealtime() / 100 - mStartTime);
                mResult.addData(0, snr);
            } else {
                mResult.setAverageCNR(0, snr, (mTestState == 0));
            }
        } else if (command == 3) {
            float snr = Float.parseFloat(splited[4]);
            mResult.setDesenseValue(0, snr);
            mResult.result = Integer.parseInt(splited[5]);
        }
    }

    @Override
    public String toString() {
        return "Normal";
    }
}

/**
 * Signal Mode.
 */
class GpsSignalMode extends GpsMode {
    void setSvIds(int[] svIds) {
        mSvIds = svIds;
        mSvCount = 0;
        for (int i = 0; i < svIds.length; i++) {
            if (svIds[i] !=   Util.INVALID_SV_ID) {
                mSvCount++;
            }
        }

        Elog.d(TAG, "GpsSignalMode count " + mSvCount);
        for (int i = 0; i < mSvIds.length; i++) {
            Elog.d(TAG, "GpsSignalMode " + mSvIds[i]);
        }
    }

    @Override
    public TestResult run() {
        if (mTestState == 0) {
            mResult = new TestResult();
            mStartTime = SystemClock.elapsedRealtime() / 100;
        }

        mTestCount = 30;
        // send PMTK command and receive response
        mClient = new ClientSocket(this);
        String command = "PMTK828,1,2," + mTestItemIndex + "," + mTestState + "," + mTestCount
                + "," + mSvCount;
        for (int svId : mSvIds) {
            command += "," + svId;
        }
//        command += "*";
        Elog.d(TAG, "sendCommand " + command);
        mClient.sendCommand(command);
        return mResult;
    }

    public void onResponse(String response) {
        Elog.d(TAG, "onResponse " + response);
        if (response == null) {
            mResult.result = TestResult.CONNECTION_FAIL;
            mResult.state =  TestResult.ResultState.STATE_NA;
            if (mClient != null) {
                mClient.closeClient();
            }
            return;
        }

        if (getmTriggerSource() == 1){
            Elog.d(TAG, "triggle from ATE Server");
            mServerSockets.sendCommand("PMTK" + response);
        }

        String[] splited = response.split(",");
        int command = Integer.parseInt(splited[1]);
        if (command == 2) {
            int index = Integer.parseInt(splited[5]);
            for (int i = 0; i < 5; i++) {
                if (mSvIds[i] !=  Util.INVALID_SV_ID) {
                    float snr = Float.parseFloat(splited[i + 6]);
                    if (index > 0) {
                        mResult.addData(i, SystemClock.elapsedRealtime() / 100 - mStartTime);
                        mResult.addData(i, snr);
                    } else {
                        if (mTestState == 0) {
                            mResult.state =  TestResult.ResultState.STATE_REF_DONE;
                            if (mClient != null) {
                                mClient.closeClient();
                            }
                        }
                        mResult.setAverageCNR(i, snr, (mTestState == 0));
                    }
                }
            }
        } else if (command == 3) {

            for (int i = 0; i < 5; i++) {
                if (mSvIds[i] !=  Util.INVALID_SV_ID) {
                    float snr = Float.parseFloat(splited[i + 4]);
                    mResult.setDesenseValue(i, snr);
                }
            }
            mResult.result = Integer.parseInt(splited[9]);
            mResult.state =  TestResult.ResultState.STATE_DONE;
            if (mClient != null) {
                mClient.closeClient();
            }
        }
    }

    @Override
    public String toString() {
        return "Signal";
    }
}

/**
 * CW Mode.
 */
class GpsCwMode extends GpsMode {
    void setSvIds(int[] svIds) {
        mSvCount = 1;
        Elog.d(TAG, "GpsCwMode count " + mSvCount);
    }

    public TestResult run() {
        if (mTestState == 0) {
            mResult = new TestResult();
            mStartTime = SystemClock.elapsedRealtime() / 100;
        }

        mTestCount = 10;
        // send PMTK command and receive response
        mClient = new ClientSocket(this);
        String command = "PMTK828,1,1," + mTestItemIndex + "," + mTestState + "," + mTestCount
                + "," + mSvCount;
        Elog.d(TAG, "sendCommand " + command);
        mClient.sendCommand(command);
        return mResult;
    }

    public void onResponse(String response) {
        Elog.d(TAG, "onResponse " + response);

        if (response == null) {
            mResult.result = TestResult.CONNECTION_FAIL;
            mResult.state =  TestResult.ResultState.STATE_NA;
            if (mClient != null) {
                mClient.closeClient();
            }
            return;
        }

        if(getmTriggerSource() == 1){
            Elog.d(TAG, "triggle from ATE Server");
            mServerSockets.sendCommand("PMTK"+response);
        }

        String[] splited = response.split(",");

        int command = Integer.parseInt(splited[1]);
        if (command == 2) {
            int index = Integer.parseInt(splited[5]);
            float snr = Float.parseFloat(splited[6]);
            if (index > 0) {
                mResult.addData(0, SystemClock.elapsedRealtime() / 100 - mStartTime);
                mResult.addData(0, snr);
            } else {
                if (mTestState == 0) {
                    mResult.state =  TestResult.ResultState.STATE_REF_DONE;
                    if (mClient != null) {
                        mClient.closeClient();
                    }
                }
                mResult.setAverageCNR(0, snr, (mTestState == 0));
            }
        } else if (command == 3) {
            float snr = Float.parseFloat(splited[4]);
            mResult.setDesenseValue(0, snr);
            mResult.result = Integer.parseInt(splited[5]);
            mResult.state =  TestResult.ResultState.STATE_DONE;
            if (mClient != null) {
                mClient.closeClient();
            }
        }
    }

    @Override
    public String toString() {
        return "CW";
    }
}
