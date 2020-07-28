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

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.engineermode.bluetooth;



import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.os.RemoteException;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
/**
 * BT jni java part method.
 *
 * @author mtk54040
 *
 */
public class BtTest {
    private static final String TAG = "BtTest";
    private int mPatter;
    private int mChannels;
    private int mPocketType;
    private int mPocketTypeLen;
    private int mFreq;
    private int mPower;

    /**
     * Init BT test mode.
     * @return true if succeed
     */
    public int init() {
        try {
            return EmUtils.getEmHidlService().btInit();
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        }

    }

    /**
     * Check if BLE is supported.
     * @return
     *      -1: error
     *      0: not supported
     *      1: supported
     */
    public int isBLESupport() {
        try {
            return EmUtils.getEmHidlService().btIsBLESupport();
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        }
    }

    /**
     * Check if LE enhanced mode is supported.
     * @return
     *      true: supported
     *      false: not supported
     */
    public boolean isBLEEnhancedSupport() {
        try {
            return EmUtils.getEmHidlService().btIsBLEEnhancedSupport();
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }

    }

    /**
     * Do BT EM test.
     * @param kind Test mode
     * @return result of test
     */
    public int doBtTest(int kind) {
        try {
            return EmUtils.getEmHidlService().btDoTest(kind, mPatter, mChannels, mPocketType,
                    mPocketTypeLen, mFreq, mPower);
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        }

    }

    boolean noSigRxTestStart(int nPatternIdx, int nPocketTypeIdx,
            int nFreq, int nAddress) {
        try {
            return EmUtils.getEmHidlService().btStartNoSigRxTest(
                    nPatternIdx, nPocketTypeIdx, nFreq, nAddress);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }

    }

    int[] noSigRxTestResult() {
        ArrayList<Integer> result = null;
        try {
            result = EmUtils.getEmHidlService().btEndNoSigRxTest();
        } catch (RemoteException e) {
            e.printStackTrace();
            return null;
        }

        int size = result.size();
        int[] data = new int[size];
        for (int i = 0; i < size; i++) {
            data[i] = result.get(i);
        }
        return data;
    }

    /**
     * Un-init BT test mode.
     * @return result of un-init
     */
    public int unInit() {
        try {
            return EmUtils.getEmHidlService().btUninit();
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        }

    }

    char[] hciCommandRun(char[] hciCmd, int cmdLength) {
        ArrayList<Byte> input = new ArrayList<Byte>();
        for (char cmdByte : hciCmd) {
            input.add((byte) cmdByte);
        }
        ArrayList<Byte> result = null;
        try {
            result = EmUtils.getEmHidlService().btHciCommandRun(input);
        } catch (RemoteException e) {
            e.printStackTrace();
            return null;
        }

        int size = result.size();
        char[] data = new char[size];
        for (int i = 0; i < size; i++) {
            Elog.i(TAG, "result.get(i):" + result.get(i));
            Elog.i(TAG, "result.after:" + (result.get(i) & 0xff));
            data[i] = (char) (result.get(i) & 0xff);
        }
        return data;
    }

    int isComboSupport() {
        try {
            return EmUtils.getEmHidlService().btIsComboSupport();
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        }

    }

    /**
     * Start BT test polling.
     * @return result for start polling
     */
    public int pollingStart() {
        try {
            return EmUtils.getEmHidlService().btPollingStart();
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        }

    }

    /**
     * Stop BT test polling.
     * @return result for stop polling
     */
    public int pollingStop() {
        try {
            return EmUtils.getEmHidlService().btPollingStop();
        } catch (RemoteException e) {
            e.printStackTrace();
            return -1;
        }

    }

    /**
     * Construct function.
     */
    public BtTest() {
        mPatter = -1;
        mChannels = -1;
        mPocketType = -1;
        mPocketTypeLen = 0;
        mFreq = 0;
        mPower = 7;
    }


    int getPocketType() {
        return mPocketType;
    }


    int getFreq() {
        return mFreq;
    }

    public void setPatter(int val) {
        mPatter = val;
    }

    public void setChannels(int val) {
        mChannels = val;
    }

    public void setPocketType(int val) {
        mPocketType = val;
    }

    public void setPocketTypeLen(int val) {
        mPocketTypeLen = val;
    }

    void setFreq(int val) {
        mFreq = val;
    }

    void setPower(int val) {
        mPower = val;
    }

    static boolean checkInitState(BluetoothAdapter adapter, Context context) {
        int btState = adapter.getState();
        Elog.i(TAG, "btState:" + btState);
        if (btState != BluetoothAdapter.STATE_OFF) {
            Toast.makeText(context, R.string.BT_turn_bt_off,
                    Toast.LENGTH_LONG).show();
            return false;
        }

        int bleState = adapter.getLeState();
        Elog.i(TAG, "bleState:" + bleState);
        if (bleState != BluetoothAdapter.STATE_OFF) {
            Toast.makeText(context, R.string.BT_turn_ble_off,
                    Toast.LENGTH_LONG).show();
            return false;
        }
        return true;
    }
}
